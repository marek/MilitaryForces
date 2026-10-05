#ifndef SYSTEM_H
#define SYSTEM_H

enum cpuFeatures_t : int
{
  CF_RDTSC      = 1 << 0,
  CF_MMX        = 1 << 1,
  CF_MMX_EXT    = 1 << 2,
  CF_3DNOW      = 1 << 3,
  CF_3DNOW_EXT  = 1 << 4,
  CF_SSE        = 1 << 5,
  CF_SSE2       = 1 << 6,
  CF_ALTIVEC    = 1 << 7
};

inline cpuFeatures_t operator|( cpuFeatures_t a, cpuFeatures_t b ) { return (cpuFeatures_t)( (int)a | (int)b ); }
inline cpuFeatures_t &operator|=( cpuFeatures_t &a, cpuFeatures_t b ) { a = a | b; return a; }

enum dialogResult_t
{
	DR_YES = 0,
	DR_NO = 1,
	DR_OK = 0,
	DR_CANCEL = 1
};

enum dialogType_t
{
	DT_INFO,
	DT_WARNING,
	DT_ERROR,
	DT_YES_NO,
	DT_OK_CANCEL
};

struct netadr_t;

class System
{
public:
	static void init ();
	static bool dllExtension( const char *name );
	static char *getCurrentUser( );
	static void QDECL error( const char *error, ...) Q_NO_RETURN Q_PRINTF_FUNC(1, 2);
	static void quit () Q_NO_RETURN;
	static char *getClipboardData( );
	static void print( const char *msg );
	static int milliseconds ();
	static bool randomBytes( byte *string, int len );
	static cpuFeatures_t getProcessorFeatures( );
	static FILE *fileOpen( const char *ospath, const char *mode );
	static bool makeDir( const char *path );
	static FILE *makeFifo( const char *ospath );
	static char *cwd( );
	static void setDefaultInstallPath(const char *path);
	static char *defaultInstallPath();
	static char *steamPath();
	static char *gogPath();
	static char *microsoftStorePath();
	static char *defaultAppPath();
	static char *defaultHomeConfigPath();
	static char *defaultHomeDataPath();
	static char *defaultHomeStatePath();
	static const char *dirName( char *path );
	static const char *baseName( char *path );
	static char *consoleInput();
	static char **listFiles( const char *directory, const char *extension, char *filter, int *numfiles, bool wantsubs );
	static void freeFileList( char **list );
	static void sleepMsec(int msec);
	static bool lowPhysicalMemory( );
	static void setEnv(const char *name, const char *value);
	static dialogResult_t dialog( dialogType_t type, const char *message, const char *title );
	static bool openFolderInFileManager( const char *path, bool create );
	static void removePIDFile( const char *gamedir );
	static void initPIDFile( const char *gamedir );

private:
	static char installPath[ MAX_OSPATH ];
};

#endif
