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
		Common::printf( " Can't load symbol %s\n", str);
		alinit_fail = true;
		return nullptr;
	}
	else
	{
		Common::dPrintf( " Loaded symbol %s (%p)\n", str, rv);
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

	qalEnable = reinterpret_cast<decltype(qalEnable)>(GPA("alEnable"));
	qalDisable = reinterpret_cast<decltype(qalDisable)>(GPA("alDisable"));
	qalIsEnabled = reinterpret_cast<decltype(qalIsEnabled)>(GPA("alIsEnabled"));
	qalGetString = reinterpret_cast<decltype(qalGetString)>(GPA("alGetString"));
	qalGetBooleanv = reinterpret_cast<decltype(qalGetBooleanv)>(GPA("alGetBooleanv"));
	qalGetIntegerv = reinterpret_cast<decltype(qalGetIntegerv)>(GPA("alGetIntegerv"));
	qalGetFloatv = reinterpret_cast<decltype(qalGetFloatv)>(GPA("alGetFloatv"));
	qalGetDoublev = reinterpret_cast<decltype(qalGetDoublev)>(GPA("alGetDoublev"));
	qalGetBoolean = reinterpret_cast<decltype(qalGetBoolean)>(GPA("alGetBoolean"));
	qalGetInteger = reinterpret_cast<decltype(qalGetInteger)>(GPA("alGetInteger"));
	qalGetFloat = reinterpret_cast<decltype(qalGetFloat)>(GPA("alGetFloat"));
	qalGetDouble = reinterpret_cast<decltype(qalGetDouble)>(GPA("alGetDouble"));
	qalGetError = reinterpret_cast<decltype(qalGetError)>(GPA("alGetError"));
	qalIsExtensionPresent = reinterpret_cast<decltype(qalIsExtensionPresent)>(GPA("alIsExtensionPresent"));
	qalGetProcAddress = reinterpret_cast<decltype(qalGetProcAddress)>(GPA("alGetProcAddress"));
	qalGetEnumValue = reinterpret_cast<decltype(qalGetEnumValue)>(GPA("alGetEnumValue"));
	qalListenerf = reinterpret_cast<decltype(qalListenerf)>(GPA("alListenerf"));
	qalListener3f = reinterpret_cast<decltype(qalListener3f)>(GPA("alListener3f"));
	qalListenerfv = reinterpret_cast<decltype(qalListenerfv)>(GPA("alListenerfv"));
	qalListeneri = reinterpret_cast<decltype(qalListeneri)>(GPA("alListeneri"));
	qalGetListenerf = reinterpret_cast<decltype(qalGetListenerf)>(GPA("alGetListenerf"));
	qalGetListener3f = reinterpret_cast<decltype(qalGetListener3f)>(GPA("alGetListener3f"));
	qalGetListenerfv = reinterpret_cast<decltype(qalGetListenerfv)>(GPA("alGetListenerfv"));
	qalGetListeneri = reinterpret_cast<decltype(qalGetListeneri)>(GPA("alGetListeneri"));
	qalGenSources = reinterpret_cast<decltype(qalGenSources)>(GPA("alGenSources"));
	qalDeleteSources = reinterpret_cast<decltype(qalDeleteSources)>(GPA("alDeleteSources"));
	qalIsSource = reinterpret_cast<decltype(qalIsSource)>(GPA("alIsSource"));
	qalSourcef = reinterpret_cast<decltype(qalSourcef)>(GPA("alSourcef"));
	qalSource3f = reinterpret_cast<decltype(qalSource3f)>(GPA("alSource3f"));
	qalSourcefv = reinterpret_cast<decltype(qalSourcefv)>(GPA("alSourcefv"));
	qalSourcei = reinterpret_cast<decltype(qalSourcei)>(GPA("alSourcei"));
	qalGetSourcef = reinterpret_cast<decltype(qalGetSourcef)>(GPA("alGetSourcef"));
	qalGetSource3f = reinterpret_cast<decltype(qalGetSource3f)>(GPA("alGetSource3f"));
	qalGetSourcefv = reinterpret_cast<decltype(qalGetSourcefv)>(GPA("alGetSourcefv"));
	qalGetSourcei = reinterpret_cast<decltype(qalGetSourcei)>(GPA("alGetSourcei"));
	qalSourcePlayv = reinterpret_cast<decltype(qalSourcePlayv)>(GPA("alSourcePlayv"));
	qalSourceStopv = reinterpret_cast<decltype(qalSourceStopv)>(GPA("alSourceStopv"));
	qalSourceRewindv = reinterpret_cast<decltype(qalSourceRewindv)>(GPA("alSourceRewindv"));
	qalSourcePausev = reinterpret_cast<decltype(qalSourcePausev)>(GPA("alSourcePausev"));
	qalSourcePlay = reinterpret_cast<decltype(qalSourcePlay)>(GPA("alSourcePlay"));
	qalSourceStop = reinterpret_cast<decltype(qalSourceStop)>(GPA("alSourceStop"));
	qalSourceRewind = reinterpret_cast<decltype(qalSourceRewind)>(GPA("alSourceRewind"));
	qalSourcePause = reinterpret_cast<decltype(qalSourcePause)>(GPA("alSourcePause"));
	qalSourceQueueBuffers = reinterpret_cast<decltype(qalSourceQueueBuffers)>(GPA("alSourceQueueBuffers"));
	qalSourceUnqueueBuffers = reinterpret_cast<decltype(qalSourceUnqueueBuffers)>(GPA("alSourceUnqueueBuffers"));
	qalGenBuffers = reinterpret_cast<decltype(qalGenBuffers)>(GPA("alGenBuffers"));
	qalDeleteBuffers = reinterpret_cast<decltype(qalDeleteBuffers)>(GPA("alDeleteBuffers"));
	qalIsBuffer = reinterpret_cast<decltype(qalIsBuffer)>(GPA("alIsBuffer"));
	qalBufferData = reinterpret_cast<decltype(qalBufferData)>(GPA("alBufferData"));
	qalGetBufferf = reinterpret_cast<decltype(qalGetBufferf)>(GPA("alGetBufferf"));
	qalGetBufferi = reinterpret_cast<decltype(qalGetBufferi)>(GPA("alGetBufferi"));
	qalDopplerFactor = reinterpret_cast<decltype(qalDopplerFactor)>(GPA("alDopplerFactor"));
	qalSpeedOfSound = reinterpret_cast<decltype(qalSpeedOfSound)>(GPA("alSpeedOfSound"));
	qalDistanceModel = reinterpret_cast<decltype(qalDistanceModel)>(GPA("alDistanceModel"));

	qalcCreateContext = reinterpret_cast<decltype(qalcCreateContext)>(GPA("alcCreateContext"));
	qalcMakeContextCurrent = reinterpret_cast<decltype(qalcMakeContextCurrent)>(GPA("alcMakeContextCurrent"));
	qalcProcessContext = reinterpret_cast<decltype(qalcProcessContext)>(GPA("alcProcessContext"));
	qalcSuspendContext = reinterpret_cast<decltype(qalcSuspendContext)>(GPA("alcSuspendContext"));
	qalcDestroyContext = reinterpret_cast<decltype(qalcDestroyContext)>(GPA("alcDestroyContext"));
	qalcGetCurrentContext = reinterpret_cast<decltype(qalcGetCurrentContext)>(GPA("alcGetCurrentContext"));
	qalcGetContextsDevice = reinterpret_cast<decltype(qalcGetContextsDevice)>(GPA("alcGetContextsDevice"));
	qalcOpenDevice = reinterpret_cast<decltype(qalcOpenDevice)>(GPA("alcOpenDevice"));
	qalcCloseDevice = reinterpret_cast<decltype(qalcCloseDevice)>(GPA("alcCloseDevice"));
	qalcGetError = reinterpret_cast<decltype(qalcGetError)>(GPA("alcGetError"));
	qalcIsExtensionPresent = reinterpret_cast<decltype(qalcIsExtensionPresent)>(GPA("alcIsExtensionPresent"));
	qalcGetProcAddress = reinterpret_cast<decltype(qalcGetProcAddress)>(GPA("alcGetProcAddress"));
	qalcGetEnumValue = reinterpret_cast<decltype(qalcGetEnumValue)>(GPA("alcGetEnumValue"));
	qalcGetString = reinterpret_cast<decltype(qalcGetString)>(GPA("alcGetString"));
	qalcGetIntegerv = reinterpret_cast<decltype(qalcGetIntegerv)>(GPA("alcGetIntegerv"));
	qalcCaptureOpenDevice = reinterpret_cast<decltype(qalcCaptureOpenDevice)>(GPA("alcCaptureOpenDevice"));
	qalcCaptureCloseDevice = reinterpret_cast<decltype(qalcCaptureCloseDevice)>(GPA("alcCaptureCloseDevice"));
	qalcCaptureStart = reinterpret_cast<decltype(qalcCaptureStart)>(GPA("alcCaptureStart"));
	qalcCaptureStop = reinterpret_cast<decltype(qalcCaptureStop)>(GPA("alcCaptureStop"));
	qalcCaptureSamples = reinterpret_cast<decltype(qalcCaptureSamples)>(GPA("alcCaptureSamples"));

	if(alinit_fail)
	{
		QAL_Shutdown();
		Common::printf( " One or more symbols not found\n");
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
