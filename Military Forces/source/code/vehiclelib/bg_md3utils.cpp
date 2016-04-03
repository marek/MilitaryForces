
#include "../game/q_shared.h"
#include "../qcommon/qfiles.h"
#include "../game/bg_public.h"
#include "bg_md3utils.h"

// decls
int FS_FOpenFileByMode( const char *qpath, fileHandle_t *f, fsMode_t mode );
void FS_FCloseFile( fileHandle_t f );
int FS_Read2( void *buffer, int len, fileHandle_t f );
int FS_Seek( fileHandle_t f, long offset, int origin );


bool
Md3Utils::getModelDimensions( std::string const& filename,
							  vec3_t& mins,
							  vec3_t& maxs,
							  int checkFrame )
{
	bool success = false;
	fileHandle_t file;

	if( FS_FOpenFileByMode(filename.c_str(), &file, FS_READ) >= 0 ) 
		 success = getModelDimensions( file, mins, maxs, checkFrame );
	else
		Com_Error(ERR_FATAL, "Unable to open file %s\n", filename.c_str() );

	FS_FCloseFile(file);

	return success;
}

bool
Md3Utils::getModelDimensions( fileHandle_t const& file,
							  vec3_t& mins,
							  vec3_t& maxs,
							  int checkFrame )
{
	if( !file )
	{
		Com_Error(ERR_FATAL, "Invalid filehandle passed to Md3Utils::getModelDimensions\n" );
		return false;
	}

	// put it back to beginning of file (just in case)
	if( FS_Seek( file, 0, FS_SEEK_SET ) != 0 )
	{
		Com_Error(ERR_FATAL, "Unable to set file pointer in file!\n" );
		return false;
	}

	bool found = false;

	// get body bounding boxes
	md3Header_t header;
	md3Frame_t currentFrame;

	FS_Read2(&header, sizeof(header), file);
	int number = header.numFrames;

	if( checkFrame >= number )
	{
		Com_Error(ERR_FATAL, "File only has %d frames, therefore unable to check frame %d\n", 
					number, checkFrame );
		return false;
	}
	else if( checkFrame == FRAMESIZE_FIRST )
		checkFrame = 0;
	else if( checkFrame == FRAMESIZE_LAST )
		checkFrame = number - 1;

	found = true;

	for( int i = 0; i < number; ++i ) 
	{
		FS_Read2(&currentFrame, sizeof(currentFrame), file);

		if( checkFrame >= 0 && i == checkFrame )
		{
			VectorCopy( currentFrame.bounds[0], mins );
			VectorCopy( currentFrame.bounds[1], maxs );
			break;
		}
		else	// check for all
		{
			if( i == 0 )
			{
				VectorCopy( currentFrame.bounds[0], mins );
				VectorCopy( currentFrame.bounds[1], maxs );
			}
			else
			{
				if( currentFrame.bounds[0][0] < mins[0] )
					mins[0] = currentFrame.bounds[0][0];
				if( currentFrame.bounds[0][1] < mins[1] )
					mins[1] = currentFrame.bounds[0][1];
				if( currentFrame.bounds[0][2] < mins[2] )
					mins[2] = currentFrame.bounds[0][2];

				if( currentFrame.bounds[1][0] > maxs[0] )
					maxs[0] = currentFrame.bounds[1][0];
				if( currentFrame.bounds[1][1] > maxs[1] )
					maxs[1] = currentFrame.bounds[1][1];
				if( currentFrame.bounds[1][2] > maxs[2] )
					maxs[2] = currentFrame.bounds[1][2];
			}
		}
	}
	return found;
}

int
Md3Utils::getTagsContaining( std::string const& filename, 
							 std::string const& str,
							 std::vector<md3Tag_t>& tagList )
{
	int found = 0;
	fileHandle_t file;

	if( FS_FOpenFileByMode(filename.c_str(), &file, FS_READ) >= 0 ) 
		 found = getTagsContaining( file, str, tagList );
	else
		Com_Error(ERR_FATAL, "Unable to open file %s\n", filename.c_str() );

	FS_FCloseFile(file);

	return found;
}

int
Md3Utils::getTagsContaining( fileHandle_t const& file, 
							 std::string const& str,
							 std::vector<md3Tag_t>& tagList )
{
	if( str.empty() || str == "" ) 
		return 0;

	if( !file )
	{
		Com_Error(ERR_FATAL, "Invalid filehandle passed to Md3Utils::getTagsContaining\n" );
		return 0;
	}

	// put it back to beginning of file (just in case)
	if( FS_Seek( file, 0, FS_SEEK_SET ) != 0 )
	{
		Com_Error(ERR_FATAL, "Unable to set file pointer in file!\n" );
		return false;
	}

	tagList.clear();

	md3Header_t head;
	FS_Read2(&head, sizeof(head), file);

	md3Frame_t	frame;
	for( int i = 0; i < head.numFrames; ++i ) 
		FS_Read2(&frame, sizeof(frame), file);

	int total = head.numTags;
	md3Tag_t	tag;
	for( int i = 0; i < total; ++ i ) 
	{
		FS_Read2(&tag, sizeof(tag), file);
		std::string tagName(tag.name);
		// if it contains the string at the first position, then add it
		if( tagName.find(str) == 0 )
			tagList.push_back(tag);
	}

	return tagList.size();
}

bool
Md3Utils::getTagInfo( std::string const& filename,
					  std::string const& tagname,
					  md3Tag_t& tag )
{
	bool success = false;
	fileHandle_t file;

	if( FS_FOpenFileByMode(filename.c_str(), &file, FS_READ) >= 0 ) 
		 success = getTagInfo( file, tagname, tag );
	else
		Com_Error(ERR_FATAL, "Unable to open file %s\n", filename.c_str() );

	FS_FCloseFile(file);

	return success;
}

bool
Md3Utils::getTagInfo( fileHandle_t const& file,
					  std::string const& tagname,
					  md3Tag_t& tag )
{
	if( !file )
	{
		Com_Error(ERR_FATAL, "Invalid filehandle passed to Md3Utils::getTagInfo\n" );
		return false;
	}

	// put it back to beginning of file (just in case)
	if( FS_Seek( file, 0, FS_SEEK_SET ) != 0 )
	{
		Com_Error(ERR_FATAL, "Unable to set file pointer in file!\n" );
		return false;
	}

	bool found = false;

	md3Header_t head;
	md3Frame_t	frame;

	FS_Read2(&head, sizeof(head), file);
	for( int i = 0; i < head.numFrames; ++i ) 
		FS_Read2(&frame, sizeof(frame), file);

	for( int i = 0; i < head.numTags; ++i ) 
	{
		FS_Read2(&tag, sizeof(md3Tag_t), file);
		if( strcmp( tag.name, tagname.c_str() ) == 0 ) 
		{
			found = true;
			break;
		}
	}

	return found;
}

int
Md3Utils::getNumberOfFrames( std::string const& fileName )
{
	fileHandle_t file;
	
	if( FS_FOpenFileByMode( fileName.c_str(), &file, FS_READ ) >= 0 ) 
	{
		int frames = getNumberOfFrames( file );
		FS_FCloseFile( file );
		return frames;
	}

	Com_Printf( "Unable to open file %s\n", fileName.c_str() );

	return 0;
}

int
Md3Utils::getNumberOfFrames( fileHandle_t const& file )
{
	if( !file )
	{
		Com_Error(ERR_FATAL, "Invalid filehandle passed to Md3Utils::getNumberOfFrames\n" );
		return false;
	}

	// put it back to beginning of file (just in case)
	if( FS_Seek( file, 0, FS_SEEK_SET ) != 0 )
	{
		Com_Error(ERR_FATAL, "Unable to set file pointer in file!\n" );
		return false;
	}

	md3Header_t head;
	FS_Read2( &head, sizeof(head), file );
	return head.numFrames;
}



