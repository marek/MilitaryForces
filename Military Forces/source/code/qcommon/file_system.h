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
};

#endif
