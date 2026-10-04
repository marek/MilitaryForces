/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2005 Stuart Dalton (badcdev@gmail.com)

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

// Dynamically loads OpenAL

#ifdef USE_OPENAL

#include "qal.h"

#ifdef USE_OPENAL_DLOPEN

#include "../sys/sys_loadlib.h"

LPALENABLE qalEnable;
LPALDISABLE qalDisable;
LPALISENABLED qalIsEnabled;
LPALGETSTRING qalGetString;
LPALGETBOOLEANV qalGetBooleanv;
LPALGETINTEGERV qalGetIntegerv;
LPALGETFLOATV qalGetFloatv;
LPALGETDOUBLEV qalGetDoublev;
LPALGETBOOLEAN qalGetBoolean;
LPALGETINTEGER qalGetInteger;
LPALGETFLOAT qalGetFloat;
LPALGETDOUBLE qalGetDouble;
LPALGETERROR qalGetError;
LPALISEXTENSIONPRESENT qalIsExtensionPresent;
LPALGETPROCADDRESS qalGetProcAddress;
LPALGETENUMVALUE qalGetEnumValue;
LPALLISTENERF qalListenerf;
LPALLISTENER3F qalListener3f;
LPALLISTENERFV qalListenerfv;
LPALLISTENERI qalListeneri;
LPALGETLISTENERF qalGetListenerf;
LPALGETLISTENER3F qalGetListener3f;
LPALGETLISTENERFV qalGetListenerfv;
LPALGETLISTENERI qalGetListeneri;
LPALGENSOURCES qalGenSources;
LPALDELETESOURCES qalDeleteSources;
LPALISSOURCE qalIsSource;
LPALSOURCEF qalSourcef;
LPALSOURCE3F qalSource3f;
LPALSOURCEFV qalSourcefv;
LPALSOURCEI qalSourcei;
LPALGETSOURCEF qalGetSourcef;
LPALGETSOURCE3F qalGetSource3f;
LPALGETSOURCEFV qalGetSourcefv;
LPALGETSOURCEI qalGetSourcei;
LPALSOURCEPLAYV qalSourcePlayv;
LPALSOURCESTOPV qalSourceStopv;
LPALSOURCEREWINDV qalSourceRewindv;
LPALSOURCEPAUSEV qalSourcePausev;
LPALSOURCEPLAY qalSourcePlay;
LPALSOURCESTOP qalSourceStop;
LPALSOURCEREWIND qalSourceRewind;
LPALSOURCEPAUSE qalSourcePause;
LPALSOURCEQUEUEBUFFERS qalSourceQueueBuffers;
LPALSOURCEUNQUEUEBUFFERS qalSourceUnqueueBuffers;
LPALGENBUFFERS qalGenBuffers;
LPALDELETEBUFFERS qalDeleteBuffers;
LPALISBUFFER qalIsBuffer;
LPALBUFFERDATA qalBufferData;
LPALGETBUFFERF qalGetBufferf;
LPALGETBUFFERI qalGetBufferi;
LPALDOPPLERFACTOR qalDopplerFactor;
LPALSPEEDOFSOUND qalSpeedOfSound;
LPALDISTANCEMODEL qalDistanceModel;

LPALCCREATECONTEXT qalcCreateContext;
LPALCMAKECONTEXTCURRENT qalcMakeContextCurrent;
LPALCPROCESSCONTEXT qalcProcessContext;
LPALCSUSPENDCONTEXT qalcSuspendContext;
LPALCDESTROYCONTEXT qalcDestroyContext;
LPALCGETCURRENTCONTEXT qalcGetCurrentContext;
LPALCGETCONTEXTSDEVICE qalcGetContextsDevice;
LPALCOPENDEVICE qalcOpenDevice;
LPALCCLOSEDEVICE qalcCloseDevice;
LPALCGETERROR qalcGetError;
LPALCISEXTENSIONPRESENT qalcIsExtensionPresent;
LPALCGETPROCADDRESS qalcGetProcAddress;
LPALCGETENUMVALUE qalcGetEnumValue;
LPALCGETSTRING qalcGetString;
LPALCGETINTEGERV qalcGetIntegerv;
LPALCCAPTUREOPENDEVICE qalcCaptureOpenDevice;
LPALCCAPTURECLOSEDEVICE qalcCaptureCloseDevice;
LPALCCAPTURESTART qalcCaptureStart;
LPALCCAPTURESTOP qalcCaptureStop;
LPALCCAPTURESAMPLES qalcCaptureSamples;

static void *OpenALLib = nullptr;

static bool alinit_fail = false;

/*
=================
GPA
=================
*/
static void *GPA(char *str)
{
	void *rv;

	rv = Sys_LoadFunction(OpenALLib, str);
	if(!rv)
	{
		Com_Printf( " Can't load symbol %s\n", str);
		alinit_fail = true;
		return nullptr;
	}
	else
	{
		Com_DPrintf( " Loaded symbol %s (%p)\n", str, rv);
        return rv;
	}
}

/*
=================
QAL_Init
=================
*/
bool QAL_Init(const char *libname)
{
	if(OpenALLib)
		return true;

	if(!(OpenALLib = Sys_LoadDll(libname, true)))
		return false;

	alinit_fail = false;

	qalEnable = (decltype(qalEnable))GPA("alEnable");
	qalDisable = (decltype(qalDisable))GPA("alDisable");
	qalIsEnabled = (decltype(qalIsEnabled))GPA("alIsEnabled");
	qalGetString = (decltype(qalGetString))GPA("alGetString");
	qalGetBooleanv = (decltype(qalGetBooleanv))GPA("alGetBooleanv");
	qalGetIntegerv = (decltype(qalGetIntegerv))GPA("alGetIntegerv");
	qalGetFloatv = (decltype(qalGetFloatv))GPA("alGetFloatv");
	qalGetDoublev = (decltype(qalGetDoublev))GPA("alGetDoublev");
	qalGetBoolean = (decltype(qalGetBoolean))GPA("alGetBoolean");
	qalGetInteger = (decltype(qalGetInteger))GPA("alGetInteger");
	qalGetFloat = (decltype(qalGetFloat))GPA("alGetFloat");
	qalGetDouble = (decltype(qalGetDouble))GPA("alGetDouble");
	qalGetError = (decltype(qalGetError))GPA("alGetError");
	qalIsExtensionPresent = (decltype(qalIsExtensionPresent))GPA("alIsExtensionPresent");
	qalGetProcAddress = (decltype(qalGetProcAddress))GPA("alGetProcAddress");
	qalGetEnumValue = (decltype(qalGetEnumValue))GPA("alGetEnumValue");
	qalListenerf = (decltype(qalListenerf))GPA("alListenerf");
	qalListener3f = (decltype(qalListener3f))GPA("alListener3f");
	qalListenerfv = (decltype(qalListenerfv))GPA("alListenerfv");
	qalListeneri = (decltype(qalListeneri))GPA("alListeneri");
	qalGetListenerf = (decltype(qalGetListenerf))GPA("alGetListenerf");
	qalGetListener3f = (decltype(qalGetListener3f))GPA("alGetListener3f");
	qalGetListenerfv = (decltype(qalGetListenerfv))GPA("alGetListenerfv");
	qalGetListeneri = (decltype(qalGetListeneri))GPA("alGetListeneri");
	qalGenSources = (decltype(qalGenSources))GPA("alGenSources");
	qalDeleteSources = (decltype(qalDeleteSources))GPA("alDeleteSources");
	qalIsSource = (decltype(qalIsSource))GPA("alIsSource");
	qalSourcef = (decltype(qalSourcef))GPA("alSourcef");
	qalSource3f = (decltype(qalSource3f))GPA("alSource3f");
	qalSourcefv = (decltype(qalSourcefv))GPA("alSourcefv");
	qalSourcei = (decltype(qalSourcei))GPA("alSourcei");
	qalGetSourcef = (decltype(qalGetSourcef))GPA("alGetSourcef");
	qalGetSource3f = (decltype(qalGetSource3f))GPA("alGetSource3f");
	qalGetSourcefv = (decltype(qalGetSourcefv))GPA("alGetSourcefv");
	qalGetSourcei = (decltype(qalGetSourcei))GPA("alGetSourcei");
	qalSourcePlayv = (decltype(qalSourcePlayv))GPA("alSourcePlayv");
	qalSourceStopv = (decltype(qalSourceStopv))GPA("alSourceStopv");
	qalSourceRewindv = (decltype(qalSourceRewindv))GPA("alSourceRewindv");
	qalSourcePausev = (decltype(qalSourcePausev))GPA("alSourcePausev");
	qalSourcePlay = (decltype(qalSourcePlay))GPA("alSourcePlay");
	qalSourceStop = (decltype(qalSourceStop))GPA("alSourceStop");
	qalSourceRewind = (decltype(qalSourceRewind))GPA("alSourceRewind");
	qalSourcePause = (decltype(qalSourcePause))GPA("alSourcePause");
	qalSourceQueueBuffers = (decltype(qalSourceQueueBuffers))GPA("alSourceQueueBuffers");
	qalSourceUnqueueBuffers = (decltype(qalSourceUnqueueBuffers))GPA("alSourceUnqueueBuffers");
	qalGenBuffers = (decltype(qalGenBuffers))GPA("alGenBuffers");
	qalDeleteBuffers = (decltype(qalDeleteBuffers))GPA("alDeleteBuffers");
	qalIsBuffer = (decltype(qalIsBuffer))GPA("alIsBuffer");
	qalBufferData = (decltype(qalBufferData))GPA("alBufferData");
	qalGetBufferf = (decltype(qalGetBufferf))GPA("alGetBufferf");
	qalGetBufferi = (decltype(qalGetBufferi))GPA("alGetBufferi");
	qalDopplerFactor = (decltype(qalDopplerFactor))GPA("alDopplerFactor");
	qalSpeedOfSound = (decltype(qalSpeedOfSound))GPA("alSpeedOfSound");
	qalDistanceModel = (decltype(qalDistanceModel))GPA("alDistanceModel");

	qalcCreateContext = (decltype(qalcCreateContext))GPA("alcCreateContext");
	qalcMakeContextCurrent = (decltype(qalcMakeContextCurrent))GPA("alcMakeContextCurrent");
	qalcProcessContext = (decltype(qalcProcessContext))GPA("alcProcessContext");
	qalcSuspendContext = (decltype(qalcSuspendContext))GPA("alcSuspendContext");
	qalcDestroyContext = (decltype(qalcDestroyContext))GPA("alcDestroyContext");
	qalcGetCurrentContext = (decltype(qalcGetCurrentContext))GPA("alcGetCurrentContext");
	qalcGetContextsDevice = (decltype(qalcGetContextsDevice))GPA("alcGetContextsDevice");
	qalcOpenDevice = (decltype(qalcOpenDevice))GPA("alcOpenDevice");
	qalcCloseDevice = (decltype(qalcCloseDevice))GPA("alcCloseDevice");
	qalcGetError = (decltype(qalcGetError))GPA("alcGetError");
	qalcIsExtensionPresent = (decltype(qalcIsExtensionPresent))GPA("alcIsExtensionPresent");
	qalcGetProcAddress = (decltype(qalcGetProcAddress))GPA("alcGetProcAddress");
	qalcGetEnumValue = (decltype(qalcGetEnumValue))GPA("alcGetEnumValue");
	qalcGetString = (decltype(qalcGetString))GPA("alcGetString");
	qalcGetIntegerv = (decltype(qalcGetIntegerv))GPA("alcGetIntegerv");
	qalcCaptureOpenDevice = (decltype(qalcCaptureOpenDevice))GPA("alcCaptureOpenDevice");
	qalcCaptureCloseDevice = (decltype(qalcCaptureCloseDevice))GPA("alcCaptureCloseDevice");
	qalcCaptureStart = (decltype(qalcCaptureStart))GPA("alcCaptureStart");
	qalcCaptureStop = (decltype(qalcCaptureStop))GPA("alcCaptureStop");
	qalcCaptureSamples = (decltype(qalcCaptureSamples))GPA("alcCaptureSamples");

	if(alinit_fail)
	{
		QAL_Shutdown();
		Com_Printf( " One or more symbols not found\n");
		return false;
	}

	return true;
}

/*
=================
QAL_Shutdown
=================
*/
void QAL_Shutdown( )
{
	if(OpenALLib)
	{
		Sys_UnloadLibrary(OpenALLib);
		OpenALLib = nullptr;
	}

	qalEnable = nullptr;
	qalDisable = nullptr;
	qalIsEnabled = nullptr;
	qalGetString = nullptr;
	qalGetBooleanv = nullptr;
	qalGetIntegerv = nullptr;
	qalGetFloatv = nullptr;
	qalGetDoublev = nullptr;
	qalGetBoolean = nullptr;
	qalGetInteger = nullptr;
	qalGetFloat = nullptr;
	qalGetDouble = nullptr;
	qalGetError = nullptr;
	qalIsExtensionPresent = nullptr;
	qalGetProcAddress = nullptr;
	qalGetEnumValue = nullptr;
	qalListenerf = nullptr;
	qalListener3f = nullptr;
	qalListenerfv = nullptr;
	qalListeneri = nullptr;
	qalGetListenerf = nullptr;
	qalGetListener3f = nullptr;
	qalGetListenerfv = nullptr;
	qalGetListeneri = nullptr;
	qalGenSources = nullptr;
	qalDeleteSources = nullptr;
	qalIsSource = nullptr;
	qalSourcef = nullptr;
	qalSource3f = nullptr;
	qalSourcefv = nullptr;
	qalSourcei = nullptr;
	qalGetSourcef = nullptr;
	qalGetSource3f = nullptr;
	qalGetSourcefv = nullptr;
	qalGetSourcei = nullptr;
	qalSourcePlayv = nullptr;
	qalSourceStopv = nullptr;
	qalSourceRewindv = nullptr;
	qalSourcePausev = nullptr;
	qalSourcePlay = nullptr;
	qalSourceStop = nullptr;
	qalSourceRewind = nullptr;
	qalSourcePause = nullptr;
	qalSourceQueueBuffers = nullptr;
	qalSourceUnqueueBuffers = nullptr;
	qalGenBuffers = nullptr;
	qalDeleteBuffers = nullptr;
	qalIsBuffer = nullptr;
	qalBufferData = nullptr;
	qalGetBufferf = nullptr;
	qalGetBufferi = nullptr;
	qalDopplerFactor = nullptr;
	qalSpeedOfSound = nullptr;
	qalDistanceModel = nullptr;

	qalcCreateContext = nullptr;
	qalcMakeContextCurrent = nullptr;
	qalcProcessContext = nullptr;
	qalcSuspendContext = nullptr;
	qalcDestroyContext = nullptr;
	qalcGetCurrentContext = nullptr;
	qalcGetContextsDevice = nullptr;
	qalcOpenDevice = nullptr;
	qalcCloseDevice = nullptr;
	qalcGetError = nullptr;
	qalcIsExtensionPresent = nullptr;
	qalcGetProcAddress = nullptr;
	qalcGetEnumValue = nullptr;
	qalcGetString = nullptr;
	qalcGetIntegerv = nullptr;
	qalcCaptureOpenDevice = nullptr;
	qalcCaptureCloseDevice = nullptr;
	qalcCaptureStart = nullptr;
	qalcCaptureStop = nullptr;
	qalcCaptureSamples = nullptr;
}
#else
bool QAL_Init(const char *libname)
{
	return true;
}
void QAL_Shutdown( void )
{
}
#endif
#endif
