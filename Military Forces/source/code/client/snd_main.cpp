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

#include "client.h"
#include "snd_codec.h"
#include "snd_local.h"
#include "snd_public.h"

cvar_t *s_volume;
cvar_t *s_muted;
cvar_t *s_musicVolume;
cvar_t *s_doppler;
cvar_t *s_backend;
cvar_t *s_muteWhenMinimized;
cvar_t *s_muteWhenUnfocused;

static soundInterface_t si;

/*
=================
S_ValidateInterface
=================
*/
static bool S_ValidSoundInterface( soundInterface_t *pSi )
{
	if( !pSi->Shutdown ) return false;
	if( !pSi->StartSound ) return false;
	if( !pSi->StartLocalSound ) return false;
	if( !pSi->StartBackgroundTrack ) return false;
	if( !pSi->StopBackgroundTrack ) return false;
	if( !pSi->RawSamples ) return false;
	if( !pSi->StopAllSounds ) return false;
	if( !pSi->ClearLoopingSounds ) return false;
	if( !pSi->AddLoopingSound ) return false;
	if( !pSi->AddRealLoopingSound ) return false;
	if( !pSi->StopLoopingSound ) return false;
	if( !pSi->Respatialize ) return false;
	if( !pSi->UpdateEntityPosition ) return false;
	if( !pSi->Update ) return false;
	if( !pSi->DisableSounds ) return false;
	if( !pSi->BeginRegistration ) return false;
	if( !pSi->RegisterSound ) return false;
	if( !pSi->ClearSoundBuffer ) return false;
	if( !pSi->SoundInfo ) return false;
	if( !pSi->SoundList ) return false;

#ifdef USE_VOIP
	if( !pSi->StartCapture ) return false;
	if( !pSi->AvailableCaptureSamples ) return false;
	if( !pSi->Capture ) return false;
	if( !pSi->StopCapture ) return false;
	if( !pSi->MasterGain ) return false;
#endif

	return true;
}

/*
=================
SoundSystem::startSound
=================
*/
void SoundSystem::startSound( vec3_t origin, int entnum, int entchannel, sfxHandle_t sfx )
{
	if( si.StartSound ) {
		si.StartSound( origin, entnum, entchannel, sfx );
	}
}

/*
=================
SoundSystem::startLocalSound
=================
*/
void SoundSystem::startLocalSound( sfxHandle_t sfx, int channelNum )
{
	if( si.StartLocalSound ) {
		si.StartLocalSound( sfx, channelNum );
	}
}

/*
=================
SoundSystem::startBackgroundTrack
=================
*/
void SoundSystem::startBackgroundTrack( const char *intro, const char *loop )
{
	if( si.StartBackgroundTrack ) {
		si.StartBackgroundTrack( intro, loop );
	}
}

/*
=================
SoundSystem::stopBackgroundTrack
=================
*/
void SoundSystem::stopBackgroundTrack( )
{
	if( si.StopBackgroundTrack ) {
		si.StopBackgroundTrack( );
	}
}

/*
=================
SoundSystem::rawSamples
=================
*/
void SoundSystem::rawSamples (int stream, int samples, int rate, int width, int channels,
		   const byte *data, float volume, int entityNum)
{
	if(si.RawSamples)
		si.RawSamples(stream, samples, rate, width, channels, data, volume, entityNum);
}

/*
=================
SoundSystem::stopAllSounds
=================
*/
void SoundSystem::stopAllSounds( )
{
	if( si.StopAllSounds ) {
		si.StopAllSounds( );
	}
}

/*
=================
SoundSystem::clearLoopingSounds
=================
*/
void SoundSystem::clearLoopingSounds( bool killall )
{
	if( si.ClearLoopingSounds ) {
		si.ClearLoopingSounds( killall );
	}
}

/*
=================
SoundSystem::addLoopingSound
=================
*/
void SoundSystem::addLoopingSound( int entityNum, const vec3_t origin,
		const vec3_t velocity, sfxHandle_t sfx )
{
	if( si.AddLoopingSound ) {
		si.AddLoopingSound( entityNum, origin, velocity, sfx );
	}
}

/*
=================
SoundSystem::addRealLoopingSound
=================
*/
void SoundSystem::addRealLoopingSound( int entityNum, const vec3_t origin,
		const vec3_t velocity, sfxHandle_t sfx )
{
	if( si.AddRealLoopingSound ) {
		si.AddRealLoopingSound( entityNum, origin, velocity, sfx );
	}
}

/*
=================
SoundSystem::stopLoopingSound
=================
*/
void SoundSystem::stopLoopingSound( int entityNum )
{
	if( si.StopLoopingSound ) {
		si.StopLoopingSound( entityNum );
	}
}

/*
=================
SoundSystem::respatialize
=================
*/
void SoundSystem::respatialize( int entityNum, const vec3_t origin,
		vec3_t axis[3], int inwater )
{
	if( si.Respatialize ) {
		si.Respatialize( entityNum, origin, axis, inwater );
	}
}

/*
=================
SoundSystem::updateEntityPosition
=================
*/
void SoundSystem::updateEntityPosition( int entityNum, const vec3_t origin )
{
	if( si.UpdateEntityPosition ) {
		si.UpdateEntityPosition( entityNum, origin );
	}
}

/*
=================
SoundSystem::update
=================
*/
void SoundSystem::update( )
{
	if(s_muted->integer)
	{
		if(!(s_muteWhenMinimized->integer && com_minimized->integer) &&
		   !(s_muteWhenUnfocused->integer && com_unfocused->integer))
		{
			s_muted->integer = false;
			s_muted->modified = true;
		}
	}
	else
	{
		if((s_muteWhenMinimized->integer && com_minimized->integer) ||
		   (s_muteWhenUnfocused->integer && com_unfocused->integer))
		{
			s_muted->integer = true;
			s_muted->modified = true;
		}
	}
	
	if( si.Update ) {
		si.Update( );
	}
}

/*
=================
SoundSystem::disableSounds
=================
*/
void SoundSystem::disableSounds( )
{
	if( si.DisableSounds ) {
		si.DisableSounds( );
	}
}

/*
=================
SoundSystem::beginRegistration
=================
*/
void SoundSystem::beginRegistration( )
{
	if( si.BeginRegistration ) {
		si.BeginRegistration( );
	}
}

/*
=================
SoundSystem::registerSound
=================
*/
sfxHandle_t	SoundSystem::registerSound( const char *sample, bool compressed )
{
	if( si.RegisterSound ) {
		return si.RegisterSound( sample, compressed );
	} else {
		return 0;
	}
}

/*
=================
SoundSystem::clearSoundBuffer
=================
*/
void SoundSystem::clearSoundBuffer( )
{
	if( si.ClearSoundBuffer ) {
		si.ClearSoundBuffer( );
	}
}

/*
=================
S_SoundInfo
=================
*/
void S_SoundInfo( )
{
	if( si.SoundInfo ) {
		si.SoundInfo( );
	}
}

/*
=================
S_SoundList
=================
*/
void S_SoundList( )
{
	if( si.SoundList ) {
		si.SoundList( );
	}
}


#ifdef USE_VOIP
/*
=================
SoundSystem::startCapture
=================
*/
void SoundSystem::startCapture( )
{
	if( si.StartCapture ) {
		si.StartCapture( );
	}
}

/*
=================
SoundSystem::availableCaptureSamples
=================
*/
int SoundSystem::availableCaptureSamples( )
{
	if( si.AvailableCaptureSamples ) {
		return si.AvailableCaptureSamples( );
	}
	return 0;
}

/*
=================
SoundSystem::capture
=================
*/
void SoundSystem::capture( int samples, byte *data )
{
	if( si.Capture ) {
		si.Capture( samples, data );
	}
}

/*
=================
SoundSystem::stopCapture
=================
*/
void SoundSystem::stopCapture( )
{
	if( si.StopCapture ) {
		si.StopCapture( );
	}
}

/*
=================
SoundSystem::masterGain
=================
*/
void SoundSystem::masterGain( float gain )
{
	if( si.MasterGain ) {
		si.MasterGain( gain );
	}
}
#endif

//=============================================================================

/*
=================
S_Play_f
=================
*/
void S_Play_f( ) {
	int 		i;
	int			c;
	sfxHandle_t	h;

	if( !si.RegisterSound || !si.StartLocalSound ) {
		return;
	}

	c = CommandSystem::argc();

	if( c < 2 ) {
		Com_Printf ("Usage: play <sound filename> [sound filename] [sound filename] ...\n");
		return;
	}

	for( i = 1; i < c; i++ ) {
		h = si.RegisterSound( CommandSystem::argv(i), false );

		if( h ) {
			si.StartLocalSound( h, CHAN_LOCAL_SOUND );
		}
	}
}

/*
=================
S_Music_f
=================
*/
void S_Music_f( ) {
	int		c;

	if( !si.StartBackgroundTrack ) {
		return;
	}

	c = CommandSystem::argc();

	if ( c == 2 ) {
		si.StartBackgroundTrack( CommandSystem::argv(1), nullptr );
	} else if ( c == 3 ) {
		si.StartBackgroundTrack( CommandSystem::argv(1), CommandSystem::argv(2) );
	} else {
		Com_Printf ("Usage: music <musicfile> [loopfile]\n");
		return;
	}

}

/*
=================
S_Music_f
=================
*/
void S_StopMusic_f( )
{
	if(!si.StopBackgroundTrack)
		return;

	si.StopBackgroundTrack();
}


//=============================================================================

/*
=================
SoundSystem::init
=================
*/
void SoundSystem::init( )
{
	cvar_t		*cv;
	bool	started = false;

	Com_Printf( "------ Initializing Sound ------\n" );

	s_volume = CvarSystem::get( "s_volume", "0.8", CVAR_ARCHIVE );
	s_musicVolume = CvarSystem::get( "s_musicvolume", "0.25", CVAR_ARCHIVE );
	s_muted = CvarSystem::get("s_muted", "0", CVAR_ROM);
	s_doppler = CvarSystem::get( "s_doppler", "1", CVAR_ARCHIVE );
	s_backend = CvarSystem::get( "s_backend", "", CVAR_ROM );
	s_muteWhenMinimized = CvarSystem::get( "s_muteWhenMinimized", "0", CVAR_ARCHIVE );
	s_muteWhenUnfocused = CvarSystem::get( "s_muteWhenUnfocused", "0", CVAR_ARCHIVE );

	cv = CvarSystem::get( "s_initsound", "1", 0 );
	if( !cv->integer ) {
		Com_Printf( "Sound disabled.\n" );
	} else {

		S_CodecInit( );

		CommandSystem::addCommand( "play", S_Play_f );
		CommandSystem::addCommand( "music", S_Music_f );
		CommandSystem::addCommand( "stopmusic", S_StopMusic_f );
		CommandSystem::addCommand( "s_list", S_SoundList );
		CommandSystem::addCommand( "s_stop", SoundSystem::stopAllSounds );
		CommandSystem::addCommand( "s_info", S_SoundInfo );

		cv = CvarSystem::get( "s_useOpenAL", "1", CVAR_ARCHIVE | CVAR_LATCH );
		if( cv->integer ) {
			//OpenAL
			started = S_AL_Init( &si );
			CvarSystem::set( "s_backend", "OpenAL" );
		}

		if( !started ) {
			started = S_Base_Init( &si );
			CvarSystem::set( "s_backend", "base" );
		}

		if( started ) {
			if( !S_ValidSoundInterface( &si ) ) {
				Com_Error( ERR_FATAL, "Sound interface invalid" );
			}

			S_SoundInfo( );
			Com_Printf( "Sound initialization successful.\n" );
		} else {
			Com_Printf( "Sound initialization failed.\n" );
		}
	}

	Com_Printf( "--------------------------------\n");
}

/*
=================
SoundSystem::shutdown
=================
*/
void SoundSystem::shutdown( )
{
	if( si.Shutdown ) {
		si.Shutdown( );
	}

	Com_Memset( &si, 0, sizeof( soundInterface_t ) );

	CommandSystem::removeCommand( "play" );
	CommandSystem::removeCommand( "music");
	CommandSystem::removeCommand( "stopmusic");
	CommandSystem::removeCommand( "s_list" );
	CommandSystem::removeCommand( "s_stop" );
	CommandSystem::removeCommand( "s_info" );

	S_CodecShutdown( );
}

