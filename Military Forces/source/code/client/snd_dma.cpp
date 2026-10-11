/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

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

/*****************************************************************************
 * name:		snd_dma.c
 *
 * desc:		main control for any streaming sound output device
 *
 * $Archive: /MissionPack/code/client/snd_dma.c $
 *
 *****************************************************************************/

#include "snd_local.h"
#include <algorithm>
#include "snd_codec.h"
#include "client.h"

static void S_Update_( );
static void S_Base_StopAllSounds();
static void S_Base_StopBackgroundTrack( );

SoundStream	*s_backgroundStream = nullptr;
static char		s_backgroundLoop[MAX_QPATH];
//static char		s_backgroundMusic[MAX_QPATH]; //TTimo: unused


// =======================================================================
// Internal sound data & structures
// =======================================================================

// only begin attenuating sound volumes when outside the FULLVOLUME range
constexpr int SOUND_FULLVOLUME = 80;

constexpr float SOUND_ATTENUATE = 0.0008f;

channel_t   s_channels[MAX_CHANNELS];
channel_t   loop_channels[MAX_CHANNELS];
int			numLoopChannels;

static int	s_soundStarted;
static		bool	s_soundMuted;

dma_t		dma;

static int			listener_number;
static vec3_t		listener_origin;
static vec3_t		listener_axis[3];

int			s_soundtime;		// sample PAIRS
int   		s_paintedtime; 		// sample PAIRS

// MAX_SFX may be larger than MAX_SOUNDS because
// of custom player sounds
#define		MAX_SFX			4096
SoundEffect		s_knownSfx[MAX_SFX];
static int			s_numSfx = 0;

constexpr int LOOP_HASH = 128;
static	SoundEffect		*sfxHash[LOOP_HASH];

cvar_t		*s_testsound;
static cvar_t		*s_show;
static cvar_t		*s_mixahead;
static cvar_t		*s_mixPreStep;

static loopSound_t		loopSounds[MAX_GENTITIES];
static	channel_t		*freelist = nullptr;

int						s_rawend[MAX_RAW_STREAMS];
portable_samplepair_t s_rawsamples[MAX_RAW_STREAMS][MAX_RAW_SAMPLES];


// ====================================================================
// User-setable variables
// ====================================================================


static void S_Base_SoundInfo() {	
	Common::printf("----- Sound Info -----\n" );
	if (!s_soundStarted) {
		Common::printf ("sound system not started\n");
	} else {
		Common::printf("%5d channels\n", dma.channels);
		Common::printf("%5d samples\n", dma.samples);
		Common::printf("%5d samplebits (%s)\n", dma.samplebits, dma.isfloat ? "float" : "int");
		Common::printf("%5d submission_chunk\n", dma.submission_chunk);
		Common::printf("%5d speed\n", dma.speed);
		Common::printf("%p dma buffer\n", dma.buffer);
		if ( s_backgroundStream ) {
			Common::printf("Background file: %s\n", s_backgroundLoop );
		} else {
			Common::printf("No background file.\n" );
		}

	}
	Common::printf("----------------------\n" );
}


#ifdef USE_VOIP
static
static void S_Base_StartCapture( )
{
	SNDDMA_StartCapture();
}

static
static int S_Base_AvailableCaptureSamples( )
{
	return SNDDMA_AvailableCaptureSamples();
}

static
static void S_Base_Capture( int samples, byte *data )
{
	SNDDMA_Capture(samples, data);
}

static
static void S_Base_StopCapture( )
{
	SNDDMA_StopCapture();
}

static
static void S_Base_MasterGain( float val )
{
	SNDDMA_MasterGain(val);
}
#endif



/*
=================
S_Base_SoundList
=================
*/
static void S_Base_SoundList( ) {
	int		i;
	SoundEffect	*sfx;
	int		size, total;
	char	type[4][16];
	char	mem[2][16];

	strcpy(type[0], "16bit");
	strcpy(type[1], "adpcm");
	strcpy(type[2], "daub4");
	strcpy(type[3], "mulaw");
	strcpy(mem[0], "paged out");
	strcpy(mem[1], "resident ");
	total = 0;
	for (sfx=s_knownSfx, i=0 ; i<s_numSfx ; i++, sfx++) {
		size = sfx->soundLength;
		total += size;
		Common::printf("%6i[%s] : %s[%s]\n", size, type[sfx->soundCompressionMethod],
				sfx->soundName, mem[sfx->inMemory] );
	}
	Common::printf ("Total resident: %i\n", total);
	S_DisplayFreeMemory();
}



static void S_ChannelFree(channel_t *v) {
	v->thesfx = nullptr;
	*reinterpret_cast<channel_t **>(v) = freelist;
	freelist = v;
}

static channel_t*	S_ChannelMalloc( ) {
	channel_t *v;
	if (freelist == nullptr) {
		return nullptr;
	}
	v = freelist;
	freelist = *reinterpret_cast<channel_t **>(freelist);
	v->allocTime = Common::milliseconds();
	return v;
}

static void S_ChannelSetup( ) {
	channel_t *p, *q;

	// clear all the sounds so they don't
	Com_Memset( s_channels, 0, sizeof( s_channels ) );

	p = s_channels;;
	q = p + MAX_CHANNELS;
	while (--q > p) {
		*reinterpret_cast<channel_t **>(q) = q-1;
	}
	
	*reinterpret_cast<channel_t **>(q) = nullptr;
	freelist = p + MAX_CHANNELS - 1;
	Common::dPrintf("Channel memory manager started\n");
}



// =======================================================================
// Load a sound
// =======================================================================

/*
================
return a hash value for the sfx name
================
*/
static long S_HashSFXName(const char *name) {
	int		i;
	long	hash;
	char	letter;

	hash = 0;
	i = 0;
	while (name[i] != '\0') {
		letter = tolower(name[i]);
		if (letter =='.') break;				// don't include extension
		if (letter =='\\') letter = '/';		// damn path names
		hash+=static_cast<long>(letter)*(i+119);
		i++;
	}
	hash &= (LOOP_HASH-1);
	return hash;
}

/*
==================
S_FindName

Will allocate a new sfx if it isn't found
==================
*/
static SoundEffect *S_FindName( const char *name ) {
	int		i;
	int		hash;

	SoundEffect	*sfx;

	if (!name) {
		Common::error(ERR_FATAL, "Sound name is NULL");
	}

	if (!name[0]) {
		Common::printf( S_COLOR_YELLOW "WARNING: Sound name is empty\n" );
		return nullptr;
	}

	if (strlen(name) >= MAX_QPATH) {
		Common::printf( S_COLOR_YELLOW "WARNING: Sound name is too long: %s\n", name );
		return nullptr;
	}

	if (name[0] == '*') {
		Common::printf( S_COLOR_YELLOW "WARNING: Tried to load player sound directly: %s\n", name );
		return nullptr;
	}

	hash = S_HashSFXName(name);

	sfx = sfxHash[hash];
	// see if already loaded
	while (sfx) {
		if (!Q_stricmp(sfx->soundName, name) ) {
			return sfx;
		}
		sfx = sfx->next;
	}

	// find a free sfx
	for (i=0 ; i < s_numSfx ; i++) {
		if (!s_knownSfx[i].soundName[0]) {
			break;
		}
	}

	if (i == s_numSfx) {
		if (s_numSfx == MAX_SFX) {
			Common::error (ERR_FATAL, "S_FindName: out of SoundEffect");
		}
		s_numSfx++;
	}
	
	sfx = &s_knownSfx[i];
	Com_Memset (sfx, 0, sizeof(*sfx));
	strcpy (sfx->soundName, name);

	sfx->next = sfxHash[hash];
	sfxHash[hash] = sfx;

	return sfx;
}

/*
=================
S_DefaultSound
=================
*/
void SoundEffect::loadDefault() {
	
	int		i;

	this->soundLength = 512;
	this->soundData = SND_malloc();
	this->soundData->next = nullptr;


	for ( i = 0 ; i < this->soundLength ; i++ ) {
		this->soundData->sndChunk[i] = i;
	}
}

/*
===================
SoundSystem::disableSounds

Disables sounds until the next SoundSystem::beginRegistration.
This is called when the hunk is cleared and the sounds
are no longer valid.
===================
*/
static void S_Base_DisableSounds( ) {
	S_Base_StopAllSounds();
	s_soundMuted = true;
}

/*
==================
SoundSystem::registerSound

Creates a default buzz sound if the file can't be loaded
==================
*/
static sfxHandle_t	S_Base_RegisterSound( const char *name, bool compressed ) {
	SoundEffect	*sfx;

	compressed = false;
	if (!s_soundStarted) {
		return 0;
	}

	sfx = S_FindName( name );
	if ( !sfx ) {
		return 0;
	}

	if ( sfx->soundData ) {
		if ( sfx->defaultSound ) {
			Common::printf( "MISSING sound %s\n", sfx->soundName );
			return 0;
		}
		return sfx - s_knownSfx;
	}

	sfx->inMemory = false;
	sfx->soundCompressed = compressed;

  sfx->memoryLoad();

	if ( sfx->defaultSound ) {
		Common::printf( "MISSING sound %s\n", sfx->soundName );
		return 0;
	}

	return sfx - s_knownSfx;
}

/*
=====================
SoundSystem::beginRegistration

=====================
*/
static void S_Base_BeginRegistration( ) {
	s_soundMuted = false;		// we can play again

	if (s_numSfx == 0) {
		SND_setup();

		Com_Memset(s_knownSfx, '\0', sizeof(s_knownSfx));
		Com_Memset(sfxHash, '\0', sizeof(SoundEffect *) * LOOP_HASH);

		S_Base_RegisterSound("sound/feedback/hit.wav", false);		// changed to a sound in baseq3
	}
}

void SoundEffect::memoryLoad() {
	// load the sound file
	if ( !this->loadSound() ) {
//		Common::printf( S_COLOR_YELLOW "WARNING: couldn't load sound: %s\n", this->soundName );
		this->defaultSound = true;
	}
	this->inMemory = true;
}

//=============================================================================

/*
=================
S_SpatializeOrigin

Used for spatializing s_channels
=================
*/
static void S_SpatializeOrigin (vec3_t origin, int master_vol, int *left_vol, int *right_vol)
{
    vec_t		dot;
    vec_t		dist;
    vec_t		lscale, rscale, scale;
    vec3_t		source_vec;
    vec3_t		vec;

	const float dist_mult = SOUND_ATTENUATE;
	
	// calculate stereo separation and distance attenuation
	VectorSubtract(origin, listener_origin, source_vec);

	dist = VectorNormalize(source_vec);
	dist -= SOUND_FULLVOLUME;
	if (dist < 0)
		dist = 0;			// close enough to be at full volume
	dist *= dist_mult;		// different attenuation levels
	
	VectorRotate( source_vec, listener_axis, vec );

	dot = -vec[1];

	if (dma.channels == 1)
	{ // no attenuation = no spatialization
		rscale = 1.0;
		lscale = 1.0;
	}
	else
	{
		rscale = 0.5 * (1.0 + dot);
		lscale = 0.5 * (1.0 - dot);
		if ( rscale < 0 ) {
			rscale = 0;
		}
		if ( lscale < 0 ) {
			lscale = 0;
		}
	}

	// add in distance effect
	scale = (1.0 - dist) * rscale;
	*right_vol = (master_vol * scale);
	if (*right_vol < 0)
		*right_vol = 0;

	scale = (1.0 - dist) * lscale;
	*left_vol = (master_vol * scale);
	if (*left_vol < 0)
		*left_vol = 0;
}

// =======================================================================
// Start a sound effect
// =======================================================================

/*
=================
S_Base_HearingThroughEntity

Also see S_AL_HearingThroughEntity
=================
*/
static bool S_Base_HearingThroughEntity( int entityNum, vec3_t origin )
{
	float	distanceSq;
	vec3_t	sorigin;

	if (origin)
		VectorCopy(origin, sorigin);
	else
		VectorCopy(loopSounds[entityNum].origin, sorigin);

	if( listener_number == entityNum )
	{
		// This is an outrageous hack to detect
		// whether or not the player is rendering in third person or not. We can't
		// ask the renderer because the renderer has no notion of entities and we
		// can't ask cgame since that would involve changing the API and hence mod
		// compatibility. I don't think there is any way around this, but I'll leave
		// the FIXME just in case anyone has a bright idea.
		distanceSq = DistanceSquared(
				sorigin,
				listener_origin );

		if( distanceSq > THIRD_PERSON_THRESHOLD_SQ )
			return false; //we're the player, but third person
		else
			return true;  //we're the player
	}
	else
		return false; //not the player
}

/*
====================
S_Base_StartSoundEx

Validates the parms and ques the sound up
if origin is NULL, the sound will be dynamically sourced from the entity
Entchannel 0 will never override a playing sound
====================
*/
static void S_Base_StartSoundEx( vec3_t origin, int entityNum, int entchannel, sfxHandle_t sfxHandle, bool localSound ) {
	channel_t	*ch;
	SoundEffect		*sfx;
  int i, oldest, chosen, time;
  int	inplay, allowed;
	bool	fullVolume;

	if ( !s_soundStarted || s_soundMuted ) {
		return;
	}

	if ( !origin && ( entityNum < 0 || entityNum >= MAX_GENTITIES ) ) {
		Common::error( ERR_DROP, "SoundSystem::startSound: bad entitynum %i", entityNum );
	}

	if ( sfxHandle < 0 || sfxHandle >= s_numSfx ) {
		Common::printf( S_COLOR_YELLOW "SoundSystem::startSound: handle %i out of range\n", sfxHandle );
		return;
	}

	sfx = &s_knownSfx[ sfxHandle ];

	if (sfx->inMemory == false) {
		sfx->memoryLoad();
	}

	if ( s_show->asInt() == 1 ) {
		Common::printf( "%i : %s\n", s_paintedtime, sfx->soundName );
	}

	time = Common::milliseconds();

//	Common::printf("playing %s\n", sfx->soundName);
	// pick a channel to play on

	allowed = 4;
	if (entityNum == listener_number) {
		allowed = 8;
	}

	fullVolume = false;
	if (localSound || S_Base_HearingThroughEntity(entityNum, origin)) {
		fullVolume = true;
	}

	ch = s_channels;
	inplay = 0;
	for ( i = 0; i < MAX_CHANNELS ; i++, ch++ ) {		
		if (ch->entnum == entityNum && ch->thesfx == sfx) {
			if (time - ch->allocTime < 50) {
//				if (CvarSystem::variableValue( "cg_showmiss" )) {
//					Common::printf("double sound start\n");
//				}
				return;
			}
			inplay++;
		}
	}

	if (inplay>allowed) {
		return;
	}

	sfx->lastTimeUsed = time;

	ch = S_ChannelMalloc();	// entityNum, entchannel);
	if (!ch) {
		ch = s_channels;

		oldest = sfx->lastTimeUsed;
		chosen = -1;
		for ( i = 0 ; i < MAX_CHANNELS ; i++, ch++ ) {
			if (ch->entnum != listener_number && ch->entnum == entityNum && ch->allocTime<oldest && ch->entchannel != CHAN_ANNOUNCER) {
				oldest = ch->allocTime;
				chosen = i;
			}
		}
		if (chosen == -1) {
			ch = s_channels;
			for ( i = 0 ; i < MAX_CHANNELS ; i++, ch++ ) {
				if (ch->entnum != listener_number && ch->allocTime<oldest && ch->entchannel != CHAN_ANNOUNCER) {
					oldest = ch->allocTime;
					chosen = i;
				}
			}
			if (chosen == -1) {
				ch = s_channels;
				if (ch->entnum == listener_number) {
					for ( i = 0 ; i < MAX_CHANNELS ; i++, ch++ ) {
						if (ch->allocTime<oldest) {
							oldest = ch->allocTime;
							chosen = i;
						}
					}
				}
				if (chosen == -1) {
					Common::dPrintf("dropping sound\n");
					return;
				}
			}
		}
		ch = &s_channels[chosen];
		ch->allocTime = sfx->lastTimeUsed;
	}

	if (origin) {
		VectorCopy (origin, ch->origin);
		ch->fixed_origin = true;
	} else {
		ch->fixed_origin = false;
	}

	// the level of the sound's bus is part of its own volume here; a change shows on the next sound started
	ch->master_vol = static_cast<int>( 127 * std::min( 2.0f, S_BusLevel( sfx->soundName ) ) );
	ch->entnum = entityNum;
	ch->thesfx = sfx;
	ch->startSample = START_SAMPLE_IMMEDIATE;
	ch->entchannel = entchannel;
	ch->leftvol = ch->master_vol;		// these will get calced at next spatialize
	ch->rightvol = ch->master_vol;		// unless the game isn't running
	ch->doppler = false;
	ch->fullVolume = fullVolume;
}

/*
====================
SoundSystem::startSound

if origin is NULL, the sound will be dynamically sourced from the entity
====================
*/
static void S_Base_StartSound( vec3_t origin, int entityNum, int entchannel, sfxHandle_t sfxHandle ) {
	S_Base_StartSoundEx( origin, entityNum, entchannel, sfxHandle, false );
}

/*
==================
SoundSystem::startLocalSound
==================
*/
static void S_Base_StartLocalSound( sfxHandle_t sfxHandle, int channelNum ) {
	if ( !s_soundStarted || s_soundMuted ) {
		return;
	}

	if ( sfxHandle < 0 || sfxHandle >= s_numSfx ) {
		Common::printf( S_COLOR_YELLOW "SoundSystem::startLocalSound: handle %i out of range\n", sfxHandle );
		return;
	}

	S_Base_StartSoundEx( nullptr, listener_number, channelNum, sfxHandle, true );
}


/*
==================
SoundSystem::clearSoundBuffer

If we are about to perform file access, clear the buffer
so sound doesn't stutter.
==================
*/
static void S_Base_ClearSoundBuffer( ) {
	int		clear;
		
	if (!s_soundStarted)
		return;

	// stop looping sounds
	Com_Memset(loopSounds, 0, MAX_GENTITIES*sizeof(loopSound_t));
	Com_Memset(loop_channels, 0, MAX_CHANNELS*sizeof(channel_t));
	numLoopChannels = 0;

	S_ChannelSetup();

	Com_Memset(s_rawend, '\0', sizeof (s_rawend));

	if (dma.samplebits == 8)
		clear = 0x80;
	else
		clear = 0;

	SNDDMA_BeginPainting ();
	if (dma.buffer)
		Com_Memset(dma.buffer, clear, dma.samples * dma.samplebits/8);
	SNDDMA_Submit ();
}

/*
==================
SoundSystem::stopAllSounds
==================
*/
static void S_Base_StopAllSounds() {
	if ( !s_soundStarted ) {
		return;
	}

	// stop the background music
	S_Base_StopBackgroundTrack();

	S_Base_ClearSoundBuffer ();
}

/*
==============================================================

continuous looping sounds are added each frame

==============================================================
*/

static void S_Base_StopLoopingSound(int entityNum) {
	loopSounds[entityNum].active = false;
//	loopSounds[entityNum].sfx = 0;
	loopSounds[entityNum].kill = false;
}

/*
==================
SoundSystem::clearLoopingSounds

==================
*/
static void S_Base_ClearLoopingSounds( bool killall ) {
	int i;
	for ( i = 0 ; i < MAX_GENTITIES ; i++) {
		if (killall || loopSounds[i].kill == true || (loopSounds[i].sfx && loopSounds[i].sfx->soundLength == 0)) {
			S_Base_StopLoopingSound(i);
		}
	}
	numLoopChannels = 0;
}

/*
==================
SoundSystem::addLoopingSound

Called during entity generation for a frame
Include velocity in case I get around to doing doppler...
==================
*/
static void S_Base_AddLoopingSound( int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfxHandle ) {
	SoundEffect *sfx;

	if ( !s_soundStarted || s_soundMuted ) {
		return;
	}

	if ( sfxHandle < 0 || sfxHandle >= s_numSfx ) {
		Common::printf( S_COLOR_YELLOW "SoundSystem::addLoopingSound: handle %i out of range\n", sfxHandle );
		return;
	}

	sfx = &s_knownSfx[ sfxHandle ];

	if (sfx->inMemory == false) {
		sfx->memoryLoad();
	}

	if ( !sfx->soundLength ) {
		Common::error( ERR_DROP, "%s has length 0", sfx->soundName );
	}

	VectorCopy( origin, loopSounds[entityNum].origin );
	VectorCopy( velocity, loopSounds[entityNum].velocity );
	loopSounds[entityNum].active = true;
	loopSounds[entityNum].kill = true;
	loopSounds[entityNum].doppler = false;
	loopSounds[entityNum].oldDopplerScale = 1.0;
	loopSounds[entityNum].dopplerScale = 1.0;
	loopSounds[entityNum].sfx = sfx;

	if (s_doppler->asInt() && VectorLengthSquared(velocity)>0.0) {
		vec3_t	out;
		float	lena, lenb;

		loopSounds[entityNum].doppler = true;
		lena = DistanceSquared(loopSounds[listener_number].origin, loopSounds[entityNum].origin);
		VectorAdd(loopSounds[entityNum].origin, loopSounds[entityNum].velocity, out);
		lenb = DistanceSquared(loopSounds[listener_number].origin, out);
		if ((loopSounds[entityNum].framenum+1) != cls.framecount) {
			loopSounds[entityNum].oldDopplerScale = 1.0;
		} else {
			loopSounds[entityNum].oldDopplerScale = loopSounds[entityNum].dopplerScale;
		}
		loopSounds[entityNum].dopplerScale = lenb/(lena*100);
		if (loopSounds[entityNum].dopplerScale<=1.0) {
			loopSounds[entityNum].doppler = false;			// don't bother doing the math
		} else if (loopSounds[entityNum].dopplerScale>MAX_DOPPLER_SCALE) {
			loopSounds[entityNum].dopplerScale = MAX_DOPPLER_SCALE;
		}
	}

	loopSounds[entityNum].framenum = cls.framecount;
}

/*
==================
SoundSystem::addLoopingSound

Called during entity generation for a frame
Include velocity in case I get around to doing doppler...
==================
*/
static void S_Base_AddRealLoopingSound( int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfxHandle ) {
	SoundEffect *sfx;

	if ( !s_soundStarted || s_soundMuted ) {
		return;
	}

	if ( sfxHandle < 0 || sfxHandle >= s_numSfx ) {
		Common::printf( S_COLOR_YELLOW "SoundSystem::addRealLoopingSound: handle %i out of range\n", sfxHandle );
		return;
	}

	sfx = &s_knownSfx[ sfxHandle ];

	if (sfx->inMemory == false) {
		sfx->memoryLoad();
	}

	if ( !sfx->soundLength ) {
		Common::error( ERR_DROP, "%s has length 0", sfx->soundName );
	}
	VectorCopy( origin, loopSounds[entityNum].origin );
	VectorCopy( velocity, loopSounds[entityNum].velocity );
	loopSounds[entityNum].sfx = sfx;
	loopSounds[entityNum].active = true;
	loopSounds[entityNum].kill = false;
	loopSounds[entityNum].doppler = false;
}



/*
==================
S_AddLoopSounds

Spatialize all of the looping sounds.
All sounds are on the same cycle, so any duplicates can just
sum up the channel multipliers.
==================
*/
static void S_AddLoopSounds () {
	int			i, j, time;
	int			left_total, right_total, left, right;
	channel_t	*ch;
	loopSound_t	*loop, *loop2;
	static int	loopFrame;


	numLoopChannels = 0;

	time = Common::milliseconds();

	loopFrame++;
	for ( i = 0 ; i < MAX_GENTITIES ; i++) {
		loop = &loopSounds[i];
		if ( !loop->active || loop->mergeFrame == loopFrame ) {
			continue;	// already merged into an earlier sound
		}

		if (loop->kill) {
			S_SpatializeOrigin( loop->origin, 127, &left_total, &right_total);			// 3d
		} else {
			S_SpatializeOrigin( loop->origin, 90,  &left_total, &right_total);			// sphere
		}

		loop->sfx->lastTimeUsed = time;

		for (j=(i+1); j< MAX_GENTITIES ; j++) {
			loop2 = &loopSounds[j];
			if ( !loop2->active || loop2->doppler || loop2->sfx != loop->sfx) {
				continue;
			}
			loop2->mergeFrame = loopFrame;

			if (loop2->kill) {
				S_SpatializeOrigin( loop2->origin, 127, &left, &right);				// 3d
			} else {
				S_SpatializeOrigin( loop2->origin, 90,  &left, &right);				// sphere
			}

			loop2->sfx->lastTimeUsed = time;
			left_total += left;
			right_total += right;
		}
		if (left_total == 0 && right_total == 0) {
			continue;		// not audible
		}

		// allocate a channel
		ch = &loop_channels[numLoopChannels];
		
		// the loop's bus level
		left_total = static_cast<int>( left_total * S_BusLevel( loop->sfx->soundName ) );
		right_total = static_cast<int>( right_total * S_BusLevel( loop->sfx->soundName ) );
		if (left_total > 255) {
			left_total = 255;
		}
		if (right_total > 255) {
			right_total = 255;
		}
		
		ch->master_vol = 127;
		ch->leftvol = left_total;
		ch->rightvol = right_total;
		ch->thesfx = loop->sfx;
		ch->doppler = loop->doppler;
		ch->dopplerScale = loop->dopplerScale;
		ch->oldDopplerScale = loop->oldDopplerScale;
		ch->fullVolume = false;
		numLoopChannels++;
		if (numLoopChannels == MAX_CHANNELS) {
			return;
		}
	}
}

//=============================================================================

/*
=================
S_ByteSwapRawSamples

If raw data has been loaded in little endien binary form, this must be done.
If raw data was calculated, as with ADPCM, this should not be called.
=================
*/
void S_ByteSwapRawSamples( int samples, int width, int numChannels, const byte *data ) {
	int		i;

	if ( width != 2 ) {
		return;
	}
	if ( LittleShort( 256 ) == 256 ) {
		return;
	}

	if ( numChannels == 2 ) {
		samples <<= 1;
	}
	for ( i = 0 ; i < samples ; i++ ) {
		((short *)data)[i] = LittleShort( ((short *)data)[i] );
	}
}

/*
============
S_Base_RawSamples

Music streaming
============
*/
static void S_Base_RawSamples( int stream, int samples, int rate, int width, int numChannels, const byte *data, float volume, int entityNum)
{
	int		i;
	int		src, dst;
	float	scale;
	int		intVolumeLeft, intVolumeRight;
	portable_samplepair_t *rawsamples;

	if ( !s_soundStarted || s_soundMuted ) {
		return;
	}

	if ( (stream < 0) || (stream >= MAX_RAW_STREAMS) ) {
		return;
	}

	rawsamples = s_rawsamples[stream];

	if ( s_muted->asInt() ) {
		intVolumeLeft = intVolumeRight = 0;
	} else {
		int leftvol, rightvol;

		if ( entityNum >= 0 && entityNum < MAX_GENTITIES ) {
			// support spatialized raw streams, e.g. for VoIP
			S_SpatializeOrigin( loopSounds[ entityNum ].origin, 256, &leftvol, &rightvol );
		} else {
			leftvol = rightvol = 256;
		}

		intVolumeLeft = leftvol * volume * s_volume->asFloat();
		intVolumeRight = rightvol * volume * s_volume->asFloat();
	}

	if ( s_rawend[stream] < s_soundtime ) {
		Common::dPrintf( "S_Base_RawSamples: resetting minimum: %i < %i\n", s_rawend[stream], s_soundtime );
		s_rawend[stream] = s_soundtime;
	}

	scale = static_cast<float>(rate) / dma.speed;

//Common::printf ("%i < %i < %i\n", s_soundtime, s_paintedtime, s_rawend[stream]);
	if (numChannels == 2 && width == 2)
	{
		if (scale == 1.0)
		{	// optimized case
			for (i=0 ; i<samples ; i++)
			{
				dst = s_rawend[stream]&(MAX_RAW_SAMPLES-1);
				s_rawend[stream]++;
				rawsamples[dst].left = ((short *)data)[i*2] * intVolumeLeft;
				rawsamples[dst].right = ((short *)data)[i*2+1] * intVolumeRight;
			}
		}
		else
		{
			for (i=0 ; ; i++)
			{
				src = i*scale;
				if (src >= samples)
					break;
				dst = s_rawend[stream]&(MAX_RAW_SAMPLES-1);
				s_rawend[stream]++;
				rawsamples[dst].left = ((short *)data)[src*2] * intVolumeLeft;
				rawsamples[dst].right = ((short *)data)[src*2+1] * intVolumeRight;
			}
		}
	}
	else if (numChannels == 1 && width == 2)
	{
		for (i=0 ; ; i++)
		{
			src = i*scale;
			if (src >= samples)
				break;
			dst = s_rawend[stream]&(MAX_RAW_SAMPLES-1);
			s_rawend[stream]++;
			rawsamples[dst].left = ((short *)data)[src] * intVolumeLeft;
			rawsamples[dst].right = ((short *)data)[src] * intVolumeRight;
		}
	}
	else if (numChannels == 2 && width == 1)
	{
		intVolumeLeft *= 256;
		intVolumeRight *= 256;

		for (i=0 ; ; i++)
		{
			src = i*scale;
			if (src >= samples)
				break;
			dst = s_rawend[stream]&(MAX_RAW_SAMPLES-1);
			s_rawend[stream]++;
			rawsamples[dst].left = ((char *)data)[src*2] * intVolumeLeft;
			rawsamples[dst].right = ((char *)data)[src*2+1] * intVolumeRight;
		}
	}
	else if (numChannels == 1 && width == 1)
	{
		intVolumeLeft *= 256;
		intVolumeRight *= 256;

		for (i=0 ; ; i++)
		{
			src = i*scale;
			if (src >= samples)
				break;
			dst = s_rawend[stream]&(MAX_RAW_SAMPLES-1);
			s_rawend[stream]++;
			rawsamples[dst].left = ((const_cast<byte *>(data))[src]-128) * intVolumeLeft;
			rawsamples[dst].right = ((const_cast<byte *>(data))[src]-128) * intVolumeRight;
		}
	}

	if ( s_rawend[stream] > s_soundtime + MAX_RAW_SAMPLES ) {
		Common::dPrintf( "S_Base_RawSamples: overflowed %i > %i\n", s_rawend[stream], s_soundtime );
	}
}

//=============================================================================

/*
=====================
SoundSystem::updateEntityPosition

let the sound system know where an entity currently is
======================
*/
static void S_Base_UpdateEntityPosition( int entityNum, const vec3_t origin ) {
	if ( entityNum < 0 || entityNum >= MAX_GENTITIES ) {
		Common::error( ERR_DROP, "SoundSystem::updateEntityPosition: bad entitynum %i", entityNum );
	}
	VectorCopy( origin, loopSounds[entityNum].origin );
}


/*
============
SoundSystem::respatialize

Change the volumes of all the playing sounds for changes in their positions
============
*/
static void S_Base_Respatialize( int entityNum, const vec3_t head, vec3_t axis[3], int inwater ) {
	int			i;
	channel_t	*ch;
	vec3_t		origin;

	if ( !s_soundStarted || s_soundMuted ) {
		return;
	}

	listener_number = entityNum;
	VectorCopy(head, listener_origin);
	VectorCopy(axis[0], listener_axis[0]);
	VectorCopy(axis[1], listener_axis[1]);
	VectorCopy(axis[2], listener_axis[2]);

	// update spatialization for dynamic sounds	
	ch = s_channels;
	for ( i = 0 ; i < MAX_CHANNELS ; i++, ch++ ) {
		if ( !ch->thesfx ) {
			continue;
		}
		// local and first person sounds will always be full volume
		if (ch->fullVolume) {
			ch->leftvol = ch->master_vol;
			ch->rightvol = ch->master_vol;
		} else {
			if (ch->fixed_origin) {
				VectorCopy( ch->origin, origin );
			} else {
				VectorCopy( loopSounds[ ch->entnum ].origin, origin );
			}

			S_SpatializeOrigin (origin, ch->master_vol, &ch->leftvol, &ch->rightvol);
		}
	}

	// add loopsounds
	S_AddLoopSounds ();
}


/*
========================
S_ScanChannelStarts

Returns true if any new sounds were started since the last mix
========================
*/
static bool S_ScanChannelStarts( ) {
	channel_t		*ch;
	int				i;
	bool		newSamples;

	newSamples = false;
	ch = s_channels;

	for (i=0; i<MAX_CHANNELS ; i++, ch++) {
		if ( !ch->thesfx ) {
			continue;
		}
		// if this channel was just started this frame,
		// set the sample count to it begins mixing
		// into the very first sample
		if ( ch->startSample == START_SAMPLE_IMMEDIATE ) {
			ch->startSample = s_paintedtime;
			newSamples = true;
			continue;
		}

		// if it is completely finished by now, clear it
		if ( ch->startSample + (ch->thesfx->soundLength) <= s_paintedtime ) {
			S_ChannelFree(ch);
		}
	}

	return newSamples;
}

/*
============
SoundSystem::update

Called once each time through the main loop
============
*/
static void S_Base_Update( ) {
	int			i;
	int			total;
	channel_t	*ch;

	if ( !s_soundStarted || s_soundMuted ) {
//		Common::dPrintf ("not started or muted\n");
		return;
	}

	//
	// debugging output
	//
	if ( s_show->asInt() == 2 ) {
		total = 0;
		ch = s_channels;
		for (i=0 ; i<MAX_CHANNELS; i++, ch++) {
			if (ch->thesfx && (ch->leftvol || ch->rightvol) ) {
				Common::printf ("%d %d %s\n", ch->leftvol, ch->rightvol, ch->thesfx->soundName);
				total++;
			}
		}
		
		Common::printf ("----(%i)---- painted: %i\n", total, s_paintedtime);
	}

	// add raw data from streamed samples
	S_UpdateBackgroundTrack();

	// mix some sound
	S_Update_();
}

static void S_GetSoundtime()
{
	int		samplepos;
	static	int		buffers;
	static	int		oldsamplepos;

	if( Client::videoRecording( ) )
	{
		float fps = std::min(cl_aviFrameRate->asFloat(), 1000.0f);
		float frameDuration = std::max(dma.speed / fps, 1.0f) + clc.aviSoundFrameRemainder;

		int msec = static_cast<int>(frameDuration);
		s_soundtime += msec;
		clc.aviSoundFrameRemainder = frameDuration - msec;

		return;
	}

	// it is possible to miscount buffers if it has wrapped twice between
	// calls to SoundSystem::update.  Oh well.
	samplepos = SNDDMA_GetDMAPos();
	if (samplepos < oldsamplepos)
	{
		buffers++;					// buffer wrapped
		
		if (s_paintedtime > 0x40000000)
		{	// time to chop things off to avoid 32 bit limits
			buffers = 0;
			s_paintedtime = dma.fullsamples;
			S_Base_StopAllSounds ();
		}
	}
	oldsamplepos = samplepos;

	s_soundtime = buffers*dma.fullsamples + samplepos/dma.channels;

#if 0
// check to make sure that we haven't overshot
	if (s_paintedtime < s_soundtime)
	{
		Common::dPrintf ("S_Update_ : overflow\n");
		s_paintedtime = s_soundtime;
	}
#endif

	if ( dma.submission_chunk < 256 ) {
		s_paintedtime = s_soundtime + s_mixPreStep->asFloat() * dma.speed;
	} else {
		s_paintedtime = s_soundtime + dma.submission_chunk;
	}
}


static void S_Update_() {
	unsigned        endtime;
	static			float	lastTime = 0.0f;
	float			ma, op;
	float			thisTime, sane;
	static			int ot = -1;

	if ( !s_soundStarted || s_soundMuted ) {
		return;
	}

	thisTime = Common::milliseconds();

	// Updates s_soundtime
	S_GetSoundtime();

	if (s_soundtime == ot) {
		return;
	}
	ot = s_soundtime;

	// clear any sound effects that end before the current time,
	// and start any new sounds
	S_ScanChannelStarts();

	sane = thisTime - lastTime;
	if (sane<11) {
		sane = 11;			// 85hz
	}

	ma = s_mixahead->asFloat() * dma.speed;
	op = s_mixPreStep->asFloat() + sane*dma.speed*0.01;

	if (op < ma) {
		ma = op;
	}

	// mix ahead of current position
	endtime = s_soundtime + ma;

	// mix to an even submission block size
	endtime = (endtime + dma.submission_chunk-1)
		& ~(dma.submission_chunk-1);

	// never mix more than the complete buffer
	if (endtime - s_soundtime > dma.fullsamples)
		endtime = s_soundtime + dma.fullsamples;



	SNDDMA_BeginPainting ();

	S_PaintChannels (endtime);

	SNDDMA_Submit ();

	lastTime = thisTime;
}



/*
===============================================================================

background music functions

===============================================================================
*/

/*
======================
SoundSystem::stopBackgroundTrack
======================
*/
static void S_Base_StopBackgroundTrack( ) {
	if(!s_backgroundStream)
		return;
	s_backgroundStream->closeStream();
	s_backgroundStream = nullptr;
	s_rawend[0] = 0;
}

/*
======================
S_OpenBackgroundStream
======================
*/
static void S_OpenBackgroundStream( const char *filename ) {
	// close the background track, but DON'T reset s_rawend
	// if restarting the same back ground track
	if(s_backgroundStream)
	{
		s_backgroundStream->closeStream();
		s_backgroundStream = nullptr;
	}

	// Open stream
	s_backgroundStream = S_CodecOpenStream(filename);
	if(!s_backgroundStream) {
		Common::printf( S_COLOR_YELLOW "WARNING: couldn't open music file %s\n", filename );
		return;
	}

	if(s_backgroundStream->info.channels != 2 || s_backgroundStream->info.rate != 22050) {
		Common::printf(S_COLOR_YELLOW "WARNING: music file %s is not 22k stereo\n", filename );
	}
}

/*
======================
SoundSystem::startBackgroundTrack
======================
*/
static void S_Base_StartBackgroundTrack( const char *intro, const char *loop ){
	if ( !intro ) {
		intro = "";
	}
	if ( !loop || !loop[0] ) {
		loop = intro;
	}
	Common::dPrintf( "SoundSystem::startBackgroundTrack( %s, %s )\n", intro, loop );

	if(!*intro)
	{
		S_Base_StopBackgroundTrack();
		return;
	}

	Q_strncpyz( s_backgroundLoop, loop, sizeof( s_backgroundLoop ) );

	S_OpenBackgroundStream( intro );
}

/*
======================
S_UpdateBackgroundTrack
======================
*/
void S_UpdateBackgroundTrack( ) {
	int		bufferSamples;
	int		fileSamples;
	byte	raw[30000];		// just enough to fit in a mac stack frame
	int		fileBytes;
	int		r;

	if(!s_backgroundStream) {
		return;
	}

	// don't bother playing anything if musicvolume is 0
	if ( s_musicVolume->asFloat() <= 0 ) {
		return;
	}

	// see how many samples should be copied into the raw buffer
	if ( s_rawend[0] < s_soundtime ) {
		s_rawend[0] = s_soundtime;
	}

	while ( s_rawend[0] < s_soundtime + MAX_RAW_SAMPLES ) {
		bufferSamples = MAX_RAW_SAMPLES - (s_rawend[0] - s_soundtime);

		// decide how much data needs to be read from the file
		fileSamples = bufferSamples * s_backgroundStream->info.rate / dma.speed;

		if (!fileSamples)
			return;

		// our max buffer size
		fileBytes = fileSamples * (s_backgroundStream->info.width * s_backgroundStream->info.channels);
		if ( fileBytes > sizeof(raw) ) {
			fileBytes = sizeof(raw);
			fileSamples = fileBytes / (s_backgroundStream->info.width * s_backgroundStream->info.channels);
		}

		// Read
		r = s_backgroundStream->readStream( fileBytes, raw );
		if(r < fileBytes)
		{
			fileSamples = r / (s_backgroundStream->info.width * s_backgroundStream->info.channels);
		}

		if(r > 0)
		{
			// add to raw buffer
			S_Base_RawSamples(0, fileSamples, s_backgroundStream->info.rate,
				s_backgroundStream->info.width, s_backgroundStream->info.channels, raw, s_musicVolume->asFloat(), -1);
		}
		else
		{
			// loop
			if(s_backgroundLoop[0])
			{
				S_OpenBackgroundStream( s_backgroundLoop );
				if(!s_backgroundStream)
					return;
			}
			else
			{
				S_Base_StopBackgroundTrack();
				return;
			}
		}

	}
}


/*
======================
S_FreeOldestSound
======================
*/

void S_FreeOldestSound( ) {
	int	i, oldest, used;
	SoundEffect	*sfx;
	sndBuffer	*buffer, *nbuffer;

	oldest = Common::milliseconds();
	used = 0;

	for (i=1 ; i < s_numSfx ; i++) {
		sfx = &s_knownSfx[i];
		if (sfx->inMemory && sfx->lastTimeUsed<oldest) {
			used = i;
			oldest = sfx->lastTimeUsed;
		}
	}

	sfx = &s_knownSfx[used];

	Common::dPrintf("S_FreeOldestSound: freeing sound %s\n", sfx->soundName);

	buffer = sfx->soundData;
	while(buffer != nullptr) {
		nbuffer = buffer->next;
		SND_free(buffer);
		buffer = nbuffer;
	}
	sfx->inMemory = false;
	sfx->soundData = nullptr;
}

// =======================================================================
// Shutdown sound engine
// =======================================================================

static void S_Base_Shutdown( ) {
	if ( !s_soundStarted ) {
		return;
	}

	SNDDMA_Shutdown();
	SND_shutdown();

	s_soundStarted = 0;
	s_numSfx = 0;

	CommandSystem::removeCommand("s_info");
}

/*
================
SoundSystem::init
================
*/
bool S_Base_Init( soundInterface_t *si ) {
	bool	r;

	if( !si ) {
		return false;
	}

	s_mixahead = CvarSystem::get ("s_mixahead", "0.2", CVAR_ARCHIVE);
	s_mixPreStep = CvarSystem::get ("s_mixPreStep", "0.05", CVAR_ARCHIVE);
	s_show = CvarSystem::get ("s_show", "0", CVAR_CHEAT);
	s_testsound = CvarSystem::get ("s_testsound", "0", CVAR_CHEAT);

	r = SNDDMA_Init();

	if ( r ) {
		s_soundStarted = 1;
		s_soundMuted = true;
//		s_numSfx = 0;

		Com_Memset(sfxHash, 0, sizeof(SoundEffect *)*LOOP_HASH);

		s_soundtime = 0;
		s_paintedtime = 0;

		S_Base_StopAllSounds( );
	} else {
		return false;
	}

	si->Shutdown = S_Base_Shutdown;
	si->StartSound = S_Base_StartSound;
	si->StartLocalSound = S_Base_StartLocalSound;
	si->StartBackgroundTrack = S_Base_StartBackgroundTrack;
	si->StopBackgroundTrack = S_Base_StopBackgroundTrack;
	si->RawSamples = S_Base_RawSamples;
	si->StopAllSounds = S_Base_StopAllSounds;
	si->ClearLoopingSounds = S_Base_ClearLoopingSounds;
	si->AddLoopingSound = S_Base_AddLoopingSound;
	si->AddRealLoopingSound = S_Base_AddRealLoopingSound;
	si->StopLoopingSound = S_Base_StopLoopingSound;
	si->Respatialize = S_Base_Respatialize;
	si->UpdateEntityPosition = S_Base_UpdateEntityPosition;
	si->Update = S_Base_Update;
	si->DisableSounds = S_Base_DisableSounds;
	si->BeginRegistration = S_Base_BeginRegistration;
	si->RegisterSound = S_Base_RegisterSound;
	si->ClearSoundBuffer = S_Base_ClearSoundBuffer;
	si->SoundInfo = S_Base_SoundInfo;
	si->SoundList = S_Base_SoundList;

#ifdef USE_VOIP
	si->StartCapture = S_Base_StartCapture;
	si->AvailableCaptureSamples = S_Base_AvailableCaptureSamples;
	si->Capture = S_Base_Capture;
	si->StopCapture = S_Base_StopCapture;
	si->MasterGain = S_Base_MasterGain;
#endif

	return true;
}
