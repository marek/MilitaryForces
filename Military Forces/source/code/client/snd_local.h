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
// snd_local.h -- private sound definations


#include "../game/q_shared.h"
#include "../qcommon/qcommon.h"
#include "snd_public.h"

inline constexpr int PAINTBUFFER_SIZE = 4096; // this is in samples

inline constexpr int SND_CHUNK_SIZE = 1024; // samples
#define SND_CHUNK_SIZE_FLOAT	(SND_CHUNK_SIZE/2)		// floats
#define SND_CHUNK_SIZE_BYTE		(SND_CHUNK_SIZE*2)		// floats

struct portable_samplepair_t {
	int			left;	// the final values will be clamped to +/- 0x00ffff00 and shifted down
	int			right;
};

struct adpcm_state {
    short	sample;		/* Previous output value */
    char	index;		/* Index into stepsize table */
};
using adpcm_state_t = adpcm_state;

struct sndBuffer_s {
	short					sndChunk[SND_CHUNK_SIZE];
	struct sndBuffer_s		*next;
    int						size;
	adpcm_state_t			adpcm;
};
using sndBuffer = sndBuffer_s;

class SoundEffect
{
public:
	int resampleSfx( int channels, int inrate, int inwidth, int samples, byte *data, bool compressed );
	bool loadSound();
	void loadDefault();
	void memoryLoad();
	void adpcmEncodeSound( short *samples );
	void encodeWavelet( short *packets );
	void encodeMuLaw( short *packets );

	sndBuffer		*soundData;
	bool		defaultSound;			// couldn't be loaded, so use buzz
	bool		inMemory;				// not in Memory
	bool		soundCompressed;		// not in Memory
	int				soundCompressionMethod;	
	int 			soundLength;
	int			soundChannels;
	char 			soundName[MAX_QPATH];
	int				lastTimeUsed;
	SoundEffect	*next;

private:
	static bool madeTable;
};

struct dma_t {
	int			channels;
	int			samples;				// mono samples in buffer
	int			fullsamples;			// samples with all channels in buffer (samples divided by channels)
	int			submission_chunk;		// don't mix less than this #
	int			samplebits;
	int			isfloat;
	int			speed;
	byte		*buffer;
};

inline constexpr int START_SAMPLE_IMMEDIATE = 0x7fffffff;

inline constexpr float MAX_DOPPLER_SCALE = 50.0f; //arbitrary

inline constexpr float THIRD_PERSON_THRESHOLD_SQ = (48.0f*48.0f);

struct loopSound_s {
	vec3_t		origin;
	vec3_t		velocity;
	SoundEffect		*sfx;
	int			mergeFrame;
	bool	active;
	bool	kill;
	bool	doppler;
	float		dopplerScale;
	float		oldDopplerScale;
	int			framenum;
};
using loopSound_t = loopSound_s;

struct channel_t
{
	int			allocTime;
	int			startSample;	// START_SAMPLE_IMMEDIATE = set immediately on next mix
	int			entnum;			// to allow overriding a specific sound
	int			entchannel;		// to allow overriding a specific sound
	int			leftvol;		// 0-255 volume after spatialization
	int			rightvol;		// 0-255 volume after spatialization
	int			master_vol;		// 0-255 volume before spatialization
	float		dopplerScale;
	float		oldDopplerScale;
	vec3_t		origin;			// only use if fixed_origin is set
	bool	fixed_origin;	// use origin instead of fetching entnum's origin
	SoundEffect		*thesfx;		// sfx structure
	bool	doppler;
	bool	fullVolume;
};


inline constexpr int WAV_FORMAT_PCM = 1;


struct wavinfo_t {
	int			format;
	int			rate;
	int			width;
	int			channels;
	int			samples;
	int			dataofs;		// chunk starts this many bytes from file start
};

// Interface between Q3 sound "api" and the sound backend
struct soundInterface_t
{
	void (*Shutdown)();
	void (*StartSound)( vec3_t origin, int entnum, int entchannel, sfxHandle_t sfx );
	void (*StartLocalSound)( sfxHandle_t sfx, int channelNum );
	void (*StartBackgroundTrack)( const char *intro, const char *loop );
	void (*StopBackgroundTrack)( );
	void (*RawSamples)(int stream, int samples, int rate, int width, int channels, const byte *data, float volume, int entityNum);
	void (*StopAllSounds)( );
	void (*ClearLoopingSounds)( bool killall );
	void (*AddLoopingSound)( int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx );
	void (*AddRealLoopingSound)( int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx );
	void (*StopLoopingSound)(int entityNum );
	void (*Respatialize)( int entityNum, const vec3_t origin, vec3_t axis[3], int inwater );
	void (*UpdateEntityPosition)( int entityNum, const vec3_t origin );
	void (*Update)( );
	void (*DisableSounds)( );
	void (*BeginRegistration)( );
	sfxHandle_t (*RegisterSound)( const char *sample, bool compressed );
	void (*ClearSoundBuffer)( );
	void (*SoundInfo)( );
	void (*SoundList)( );
#ifdef USE_VOIP
	void (*StartCapture)( );
	int (*AvailableCaptureSamples)( );
	void (*Capture)( int samples, byte *data );
	void (*StopCapture)( );
	void (*MasterGain)( float gain );
#endif
};


/*
====================================================================

  SYSTEM SPECIFIC FUNCTIONS

====================================================================
*/

// initializes cycling through a DMA buffer and returns information on it
bool SNDDMA_Init();

// gets the current DMA position
int		SNDDMA_GetDMAPos();

// shutdown the DMA xfer.
void	SNDDMA_Shutdown();

void	SNDDMA_BeginPainting ();

void	SNDDMA_Submit();

#ifdef USE_VOIP
void SNDDMA_StartCapture();
int SNDDMA_AvailableCaptureSamples();
void SNDDMA_Capture(int samples, byte *data);
void SNDDMA_StopCapture();
void SNDDMA_MasterGain(float val);
#endif


//====================================================================

inline constexpr int MAX_CHANNELS = 96;

extern	channel_t   s_channels[MAX_CHANNELS];
extern	channel_t   loop_channels[MAX_CHANNELS];
extern	int		numLoopChannels;

extern	int		s_paintedtime;
extern	dma_t	dma;

inline constexpr int MAX_RAW_SAMPLES = 16384;
#define MAX_RAW_STREAMS (MAX_CLIENTS * 2 + 1)
extern	portable_samplepair_t s_rawsamples[MAX_RAW_STREAMS][MAX_RAW_SAMPLES];
extern	int		s_rawend[MAX_RAW_STREAMS];

extern cvar_t *s_volume;
extern cvar_t *s_musicVolume;
extern cvar_t *s_muted;
extern cvar_t *s_doppler;

extern cvar_t *s_testsound;


void		SND_free(sndBuffer *v);
sndBuffer*	SND_malloc( );
void		SND_setup( );
void		SND_shutdown();

void S_PaintChannels(int endtime);


// spatializes a channel
void S_Spatialize(channel_t *ch);

// adpcm functions
int  S_AdpcmMemoryNeeded( const wavinfo_t *info );
void S_AdpcmGetSamples(sndBuffer *chunk, short *to);

// wavelet function

inline constexpr int SENTINEL_MULAW_ZERO_RUN = 127;
inline constexpr int SENTINEL_MULAW_FOUR_BIT_RUN = 126;

void S_FreeOldestSound( );

#define	NXStream byte

void decodeWavelet( sndBuffer *stream, short *packets);

extern short mulawToShort[256];

extern short *sfxScratchBuffer;
extern SoundEffect *sfxScratchPointer;
extern int	   sfxScratchIndex;

bool S_Base_Init( soundInterface_t *si );

// OpenAL stuff
enum alSrcPriority_t
{
	SRCPRI_AMBIENT = 0,	// Ambient sound effects
	SRCPRI_ENTITY,			// Entity sound effects
	SRCPRI_ONESHOT,			// One-shot sounds
	SRCPRI_LOCAL,				// Local sounds
	SRCPRI_STREAM				// Streams (music, cutscenes)
};

typedef int srcHandle_t;

bool S_AL_Init( soundInterface_t *si );

#ifdef idppc_altivec
void S_PaintChannelFrom16_altivec( portable_samplepair_t paintbuffer[PAINTBUFFER_SIZE], int snd_vol, channel_t *ch, const SoundEffect *sc, int count, int sampleOffset, int bufferOffset );
#endif
