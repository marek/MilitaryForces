#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

class FileSystem
{
public:
	static bool initialized( );
	static void initFilesystem ( );
	static void shutdown( bool closemfp );
	static bool conditionalRestart(int checksumFeed, bool disconnect);
	static void restart( int checksumFeed );
	static char **listFiles( const char *directory, const char *extension, int *numfiles );
	static void freeFileList( char **list );
	static bool fileExists_HomeData( const char *file );
	static bool createPath (const char *OSPath);
	static char *baseDir_BuildOSPath( const char *base, const char *qpath );
	static char *buildOSPath( const char *base, const char *game, const char *qpath );
	static bool compareZipChecksum(const char *zipfile);
	static int loadStack( );
	static int getFileList( const char *path, const char *extension, char *listbuf, int bufsize );
	static int getModList( char *listbuf, int bufsize );
	static void getModDescription( const char *modDir, char *description, int descriptionLen );
	static fileHandle_t fOpenFileWrite_HomeConfig( const char *filename );
	static fileHandle_t fOpenFileWrite_HomeData( const char *filename );
	static fileHandle_t fOpenFileWrite_HomeState( const char *filename );
	static fileHandle_t fOpenFileAppend_HomeData( const char *filename );
	static fileHandle_t fCreateOpenPipeFile( const char *filename );
	static fileHandle_t baseDir_FOpenFileWrite_HomeConfig( const char *filename );
	static fileHandle_t baseDir_FOpenFileWrite_HomeData( const char *filename );
	static fileHandle_t baseDir_FOpenFileWrite_HomeState( const char *filename );
	static long baseDir_FOpenFileRead( const char *filename, fileHandle_t *fp );
	static void baseDir_Rename_HomeData( const char *from, const char *to, bool safe );
	static long fOpenFileRead( const char *qpath, fileHandle_t *file, bool uniqueFILE );
	static int fileIsInPAK(const char *filename, int *pChecksum );
	static int write( const void *buffer, int len, fileHandle_t f );
	static int read( void *buffer, int len, fileHandle_t f );
	static void fCloseFile( fileHandle_t f );
	static long readFileDir(const char *qpath, void *searchPath, bool unpure, void **buffer);
	static long readFile(const char *qpath, void **buffer);
	static void forceFlush( fileHandle_t f );
	static void freeFile( void *buffer );
	static void writeFile( const char *qpath, const void *buffer, int size );
	static long filelength(fileHandle_t f);
	static int fTell( fileHandle_t f );
	static void flush( fileHandle_t f );
	static void QDECL printf( fileHandle_t f, const char *fmt, ... ) Q_PRINTF_FUNC(2, 3);
	static int fOpenFileByMode( const char *qpath, fileHandle_t *f, fsMode_t mode );
	static int seek( fileHandle_t f, long offset, int origin );
	static bool filenameCompare( const char *s1, const char *s2 );
	static const char *loadedPakNames( );
	static const char *loadedPakChecksums( );
	static const char *loadedPakPureChecksums( );
	static const char *referencedPakNames( );
	static const char *referencedPakChecksums( );
	static const char *referencedPakPureChecksums( );
	static void clearPakReferences( int flags );
	static void pureServerSetReferencedPaks( const char *pakSums, const char *pakNames );
	static void pureServerSetLoadedPaks( const char *pakSums, const char *pakNames );
	static bool checkDirTraversal(const char *checkdir);
	static bool invalidGameDir(const char *gamedir);
	static bool idPak(char *pak, char *base, int numPaks);
	static bool comparePaks( char *neededpaks, int len, bool dlstring );
	static void remove( const char *osPath );
	static void remove_HomeData( const char *homePath );
	static void filenameCompletion( const char *dir, const char *ext, char *filter, bool stripExt, void(*callback)(const char *s), bool allowNonPureFilesOnDisk );
	static const char *getCurrentGameDir();
	static bool which(const char *filename, void *searchPath);

public:
	static long hashFileName( const char *fname, int hashSize );
	static fileHandle_t handleForFile();
	static FILE * fileForHandle( fileHandle_t f );
	static void replaceSeparators( char *path );
	static void checkFilenameIsMutable( const char *filename, const char *function );
	static bool baseDir_FileExists_HomeData( const char *file );
	static fileHandle_t oSPath_FOpenFileWrite( const char *ospath, const char *filename );
	static int returnPath( const char *zname, char *zpath, int *depth );
	static unsigned int sys_CountFileList( char **list );
	static char** sys_ConcatenateFileLists( char **list0, char **list1 );
	static int QDECL paksort( const void *a, const void *b );
	static void addGameDirectory( const char *path, const char *dir );
	static void addGameDirectories( const char *dir );
	static void reorderPurePaks();
	static void addPathVar( cvar_t *pathVar );
	static void initPathVars();
	static void startup( const char *gameName );
	static void appendUserFriendlyPakList( char *buf, size_t bufsize, int foundPakFiles, int numPakFiles );
	static void checkPak0( void );

private:
	static int fs_numServerReferencedPaks;
	static bool fs_reordered;
	static cvar_t *fs_pathVars[16];
	static int fs_packFiles;
	static int fs_loadStack;
	static int fs_loadCount;
	static int fs_readCount;
	static cvar_t *fs_gamedirvar;
	static cvar_t *fs_basegame;
	static cvar_t *fs_basepath;
	static cvar_t *fs_microsoftstorepath;
	static cvar_t *fs_gogpath;
	static cvar_t *fs_steampath;
	static cvar_t *fs_apppath;
	static cvar_t *fs_homestatepath;
	static cvar_t *fs_homedatapath;
	static cvar_t *fs_homeconfigpath;
	static char fs_gamedir[MAX_OSPATH];
};

#endif
