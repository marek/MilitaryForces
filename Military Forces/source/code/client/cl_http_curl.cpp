/*
===========================================================================
Copyright (C) 2006 Tony J. White (tjw@tjw.org)

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#ifdef USE_HTTP

#include "client.h"

#ifdef USE_INTERNAL_CURL_HEADERS
  #include "curl/curl.h"
#else
  #include <curl/curl.h>
#endif

#ifdef __APPLE__
  #define DEFAULT_CURL_LIB "libcurl.dylib"
#else
  #define DEFAULT_CURL_LIB "libcurl.so.4"
  #define ALTERNATE_CURL_LIB "libcurl.so.3"
#endif

#include "../sys/sys_loadlib.h"

cvar_t *cl_cURLLib;

char* (*qcurl_version)();

CURL* (*qcurl_easy_init)();
CURLcode (*qcurl_easy_setopt)(CURL *curl, CURLoption option, ...);
CURLcode (*qcurl_easy_perform)(CURL *curl);
void (*qcurl_easy_cleanup)(CURL *curl);
CURLcode (*qcurl_easy_getinfo)(CURL *curl, CURLINFO info, ...);
CURL* (*qcurl_easy_duphandle)(CURL *curl);
void (*qcurl_easy_reset)(CURL *curl);
const char *(*qcurl_easy_strerror)(CURLcode);

CURLM* (*qcurl_multi_init)();
CURLMcode (*qcurl_multi_add_handle)(CURLM *multi_handle,
                                                CURL *curl_handle);
CURLMcode (*qcurl_multi_remove_handle)(CURLM *multi_handle,
                                                CURL *curl_handle);
CURLMcode (*qcurl_multi_fdset)(CURLM *multi_handle,
                                                fd_set *read_fd_set,
                                                fd_set *write_fd_set,
                                                fd_set *exc_fd_set,
                                                int *max_fd);
CURLMcode (*qcurl_multi_perform)(CURLM *multi_handle,
                                                int *running_handles);
CURLMcode (*qcurl_multi_cleanup)(CURLM *multi_handle);
CURLMsg *(*qcurl_multi_info_read)(CURLM *multi_handle,
                                                int *msgs_in_queue);
const char *(*qcurl_multi_strerror)(CURLMcode);

static void *cURLLib = nullptr;
static bool cURLSymbolLoadFailed = false;

static CURL *downloadCURL = nullptr;
static CURLM *downloadCURLM = nullptr;

/*
=================
GPA
=================
*/
static void *GPA(char *str)
{
	void *rv;

	rv = Sys_LoadFunction(cURLLib, str);
	if(!rv)
	{
		Common::printf("Can't load symbol %s\n", str);
		cURLSymbolLoadFailed = true;
		return nullptr;
	}
	else
	{
		Common::dPrintf("Loaded symbol %s (0x%p)\n", str, rv);
        return rv;
	}
}

/*
=================
CL_HTTP_Init
=================
*/
bool CL_HTTP_Init()
{
	if(cURLLib)
		return true;

	cl_cURLLib = CvarSystem::get("cl_cURLLib", DEFAULT_CURL_LIB, CVAR_ARCHIVE | CVAR_PROTECTED);

	Common::printf("Loading \"%s\"...", cl_cURLLib->string);
	if(!(cURLLib = Sys_LoadDll(cl_cURLLib->string, true)))
	{
#ifdef ALTERNATE_CURL_LIB
		// On some linux distributions there is no libcurl.so.3, but only libcurl.so.4. That one works too.
		if(!(cURLLib = Sys_LoadDll(ALTERNATE_CURL_LIB, true)))
#endif
			return false;
	}

	cURLSymbolLoadFailed = false;

	qcurl_version = reinterpret_cast<decltype(qcurl_version)>(GPA("curl_version"));

	qcurl_easy_init = reinterpret_cast<decltype(qcurl_easy_init)>(GPA("curl_easy_init"));
	qcurl_easy_setopt = reinterpret_cast<decltype(qcurl_easy_setopt)>(GPA("curl_easy_setopt"));
	qcurl_easy_perform = reinterpret_cast<decltype(qcurl_easy_perform)>(GPA("curl_easy_perform"));
	qcurl_easy_cleanup = reinterpret_cast<decltype(qcurl_easy_cleanup)>(GPA("curl_easy_cleanup"));
	qcurl_easy_getinfo = reinterpret_cast<decltype(qcurl_easy_getinfo)>(GPA("curl_easy_getinfo"));
	qcurl_easy_duphandle = reinterpret_cast<decltype(qcurl_easy_duphandle)>(GPA("curl_easy_duphandle"));
	qcurl_easy_reset = reinterpret_cast<decltype(qcurl_easy_reset)>(GPA("curl_easy_reset"));
	qcurl_easy_strerror = reinterpret_cast<decltype(qcurl_easy_strerror)>(GPA("curl_easy_strerror"));
	
	qcurl_multi_init = reinterpret_cast<decltype(qcurl_multi_init)>(GPA("curl_multi_init"));
	qcurl_multi_add_handle = reinterpret_cast<decltype(qcurl_multi_add_handle)>(GPA("curl_multi_add_handle"));
	qcurl_multi_remove_handle = reinterpret_cast<decltype(qcurl_multi_remove_handle)>(GPA("curl_multi_remove_handle"));
	qcurl_multi_fdset = reinterpret_cast<decltype(qcurl_multi_fdset)>(GPA("curl_multi_fdset"));
	qcurl_multi_perform = reinterpret_cast<decltype(qcurl_multi_perform)>(GPA("curl_multi_perform"));
	qcurl_multi_cleanup = reinterpret_cast<decltype(qcurl_multi_cleanup)>(GPA("curl_multi_cleanup"));
	qcurl_multi_info_read = reinterpret_cast<decltype(qcurl_multi_info_read)>(GPA("curl_multi_info_read"));
	qcurl_multi_strerror = reinterpret_cast<decltype(qcurl_multi_strerror)>(GPA("curl_multi_strerror"));

	if(cURLSymbolLoadFailed)
	{
		CL_HTTP_Shutdown();
		Common::printf("FAIL One or more symbols not found\n");
		return false;
	}
	Common::printf("OK\n");

	return true;
}

/*
=================
CL_HTTP_Available
=================
*/
bool CL_HTTP_Available()
{
	return cURLLib != nullptr;
}

static void CL_cURL_Cleanup()
{
	if(downloadCURLM) {
		CURLMcode result;

		if(downloadCURL) {
			result = qcurl_multi_remove_handle(downloadCURLM,
				downloadCURL);
			if(result != CURLM_OK) {
				Common::dPrintf("qcurl_multi_remove_handle failed: %s\n", qcurl_multi_strerror(result));
			}
			qcurl_easy_cleanup(downloadCURL);
		}
		result = qcurl_multi_cleanup(downloadCURLM);
		if(result != CURLM_OK) {
			Common::dPrintf("CL_cURL_Cleanup: qcurl_multi_cleanup failed: %s\n", qcurl_multi_strerror(result));
		}
		downloadCURLM = nullptr;
		downloadCURL = nullptr;
	}
	else if(downloadCURL) {
		qcurl_easy_cleanup(downloadCURL);
		downloadCURL = nullptr;
	}
}

/*
=================
CL_HTTP_Shutdown
=================
*/
void CL_HTTP_Shutdown( )
{
	CL_cURL_Cleanup();

	if(cURLLib)
	{
		Sys_UnloadLibrary(cURLLib);
		cURLLib = nullptr;
	}
	qcurl_easy_init = nullptr;
	qcurl_easy_setopt = nullptr;
	qcurl_easy_perform = nullptr;
	qcurl_easy_cleanup = nullptr;
	qcurl_easy_getinfo = nullptr;
	qcurl_easy_duphandle = nullptr;
	qcurl_easy_reset = nullptr;

	qcurl_multi_init = nullptr;
	qcurl_multi_add_handle = nullptr;
	qcurl_multi_remove_handle = nullptr;
	qcurl_multi_fdset = nullptr;
	qcurl_multi_perform = nullptr;
	qcurl_multi_cleanup = nullptr;
	qcurl_multi_info_read = nullptr;
	qcurl_multi_strerror = nullptr;
}

static int CL_cURL_CallbackProgress(void *clientp, curl_off_t dltotal, curl_off_t dlnow,
    curl_off_t ultotal, curl_off_t ulnow)
{
	clc.downloadSize = static_cast<int>(dltotal);
	CvarSystem::setValue( "cl_downloadSize", clc.downloadSize );
	clc.downloadCount = static_cast<int>(dlnow);
	CvarSystem::setValue( "cl_downloadCount", clc.downloadCount );
	return 0;
}

static size_t CL_cURL_CallbackWrite(void *buffer, size_t size, size_t nmemb,
	void *stream)
{
	FileSystem::write( buffer, size*nmemb, (static_cast<fileHandle_t*>(stream))[0] );
	return size*nmemb;
}

CURLcode qcurl_easy_setopt_warn(CURL *curl, CURLoption option, ...)
{
	CURLcode result;

	va_list argp;
	va_start(argp, option);

	if(option < CURLOPTTYPE_OBJECTPOINT) {
		long longValue = va_arg(argp, long);
		result = qcurl_easy_setopt(curl, option, longValue);
	} else if(option < CURLOPTTYPE_OFF_T) {
		void *pointerValue = va_arg(argp, void *);
		result = qcurl_easy_setopt(curl, option, pointerValue);
	} else {
		curl_off_t offsetValue = va_arg(argp, curl_off_t);
		result = qcurl_easy_setopt(curl, option, offsetValue);
	}

	if(result != CURLE_OK) {
		Common::dPrintf("qcurl_easy_setopt failed: %s\n", qcurl_easy_strerror(result));
	}
	va_end(argp);

	return result;
}

void CL_HTTP_BeginDownload( const char *remoteURL )
{
	CURLMcode result;

	CL_cURL_Cleanup();

	downloadCURL = qcurl_easy_init();
	if(!downloadCURL) {
		Common::error(ERR_DROP, "CL_HTTP_BeginDownload: qcurl_easy_init() "
			"failed");
		return;
	}

	if(com_developer->integer)
		qcurl_easy_setopt_warn(downloadCURL, CURLOPT_VERBOSE, 1);
	qcurl_easy_setopt_warn(downloadCURL, CURLOPT_URL, remoteURL);
	qcurl_easy_setopt_warn(downloadCURL, CURLOPT_TRANSFERTEXT, 0);
	qcurl_easy_setopt_warn(downloadCURL, CURLOPT_REFERER, va("ioQ3://%s",
		Network::adrToString(clc.serverAddress)));
	qcurl_easy_setopt_warn(downloadCURL, CURLOPT_USERAGENT, va("%s %s",
		Q3_VERSION, qcurl_version()));
	qcurl_easy_setopt_warn(downloadCURL, CURLOPT_WRITEFUNCTION,
		CL_cURL_CallbackWrite);
	qcurl_easy_setopt_warn(downloadCURL, CURLOPT_WRITEDATA, &clc.download);
	qcurl_easy_setopt_warn(downloadCURL, CURLOPT_NOPROGRESS, 0);
	qcurl_easy_setopt_warn(downloadCURL, CURLOPT_XFERINFOFUNCTION,
		CL_cURL_CallbackProgress);
	qcurl_easy_setopt_warn(downloadCURL, CURLOPT_PROGRESSDATA, NULL);
	qcurl_easy_setopt_warn(downloadCURL, CURLOPT_FAILONERROR, 1);
	qcurl_easy_setopt_warn(downloadCURL, CURLOPT_FOLLOWLOCATION, 1);
	qcurl_easy_setopt_warn(downloadCURL, CURLOPT_MAXREDIRS, 5);
#if CURL_AT_LEAST_VERSION(7,85,0)
	qcurl_easy_setopt_warn(downloadCURL, CURLOPT_PROTOCOLS_STR, "http,https");
#else
	qcurl_easy_setopt_warn(downloadCURL, CURLOPT_PROTOCOLS, CURLPROTO_HTTP | CURLPROTO_HTTPS);
#endif
	qcurl_easy_setopt_warn(downloadCURL, CURLOPT_BUFFERSIZE, CURL_MAX_READ_SIZE);
	downloadCURLM = qcurl_multi_init();
	if(!downloadCURLM) {
		qcurl_easy_cleanup(downloadCURL);
		downloadCURL = nullptr;
		Common::error(ERR_DROP, "CL_HTTP_BeginDownload: qcurl_multi_init() "
			"failed");
		return;
	}
	result = qcurl_multi_add_handle(downloadCURLM, downloadCURL);
	if(result != CURLM_OK) {
		qcurl_easy_cleanup(downloadCURL);
		downloadCURL = nullptr;
		Common::error(ERR_DROP,"CL_HTTP_BeginDownload: qcurl_multi_add_handle() failed: %s", qcurl_multi_strerror(result));
		return;
	}
}

bool CL_HTTP_PerformDownload()
{
	CURLMcode res;
	CURLMsg *msg;
	int c;
	int i = 0;

	res = qcurl_multi_perform(downloadCURLM, &c);
	while(res == CURLM_CALL_MULTI_PERFORM && i < 100) {
		res = qcurl_multi_perform(downloadCURLM, &c);
		i++;
	}
	if(res == CURLM_CALL_MULTI_PERFORM)
		return false;
	msg = qcurl_multi_info_read(downloadCURLM, &c);
	if(msg == nullptr) {
		return false;
	}
	if(msg->msg != CURLMSG_DONE || msg->data.result != CURLE_OK) {
		long code;

		qcurl_easy_getinfo(msg->easy_handle, CURLINFO_RESPONSE_CODE,
			&code);	
		Common::error(ERR_DROP, "Download Error: %s Code: %ld URL: %s",
			qcurl_easy_strerror(msg->data.result),
			code, clc.downloadURL);
	}

	return true;
}

#endif /* USE_HTTP */
