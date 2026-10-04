#ifndef SOUND_SYSTEM_H
#define SOUND_SYSTEM_H

class SoundSystem
{
public:
	static void init( );
	static void shutdown( );
	static void startSound( vec3_t origin, int entnum, int entchannel, sfxHandle_t sfx );
	static void startLocalSound( sfxHandle_t sfx, int channelNum );
	static void startBackgroundTrack( const char *intro, const char *loop );
	static void stopBackgroundTrack( );
	static void rawSamples(int stream, int samples, int rate, int width, int channels, const byte *data, float volume, int entityNum);
	static void stopAllSounds( );
	static void clearLoopingSounds( bool killall );
	static void addLoopingSound( int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx );
	static void addRealLoopingSound( int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx );
	static void stopLoopingSound(int entityNum );
	static void respatialize( int entityNum, const vec3_t origin, vec3_t axis[3], int inwater );
	static void updateEntityPosition( int entityNum, const vec3_t origin );
	static void update( );
	static void disableSounds( );
	static void beginRegistration( );
	static sfxHandle_t registerSound( const char *sample, bool compressed );
	static void clearSoundBuffer( );
	static void startCapture( );
	static int availableCaptureSamples( );
	static void capture( int samples, byte *data );
	static void stopCapture( );
	static void masterGain( float gain );
};

#endif
