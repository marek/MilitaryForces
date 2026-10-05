
#include "../game/q_shared.h"
#include "../qcommon/qfiles.h"
#include "../game/bg_public.h"
#include "bg_md3utils.h"

// decls


bool
Md3Utils::getModelDimensions( std::string const& filename,
							  vec3_t& mins,
							  vec3_t& maxs,
							  int checkFrame )
{
	bool success = false;
	fileHandle_t file;

	if( FileSystem::fOpenFileByMode(filename.c_str(), &file, FS_READ) >= 0 ) 
		 success = getModelDimensions( file, mins, maxs, checkFrame );
	else
		Common::error(ERR_FATAL, "Unable to open file %s\n", filename.c_str() );

	FileSystem::fCloseFile(file);

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
		Common::error(ERR_FATAL, "Invalid filehandle passed to Md3Utils::getModelDimensions\n" );
		return false;
	}

	// put it back to beginning of file (just in case)
	if( FileSystem::seek( file, 0, FS_SEEK_SET ) != 0 )
	{
		Common::error(ERR_FATAL, "Unable to set file pointer in file!\n" );
		return false;
	}

	bool found = false;

	// get body bounding boxes
	md3Header_t header;
	md3Frame_t currentFrame;

	FileSystem::read(&header, sizeof(header), file);
	int number = header.numFrames;

	if( checkFrame >= number )
	{
		Common::error(ERR_FATAL, "File only has %d frames, therefore unable to check frame %d\n", 
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
		FileSystem::read(&currentFrame, sizeof(currentFrame), file);

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
							 std::string_view str,
							 std::vector<md3Tag_t>& tagList )
{
	int found = 0;
	fileHandle_t file;

	if( FileSystem::fOpenFileByMode(filename.c_str(), &file, FS_READ) >= 0 ) 
		 found = getTagsContaining( file, str, tagList );
	else
		Common::error(ERR_FATAL, "Unable to open file %s\n", filename.c_str() );

	FileSystem::fCloseFile(file);

	return found;
}

int
Md3Utils::getTagsContaining( fileHandle_t const& file, 
							 std::string_view str,
							 std::vector<md3Tag_t>& tagList )
{
	if( str.empty() ) 
		return 0;

	if( !file )
	{
		Common::error(ERR_FATAL, "Invalid filehandle passed to Md3Utils::getTagsContaining\n" );
		return 0;
	}

	// put it back to beginning of file (just in case)
	if( FileSystem::seek( file, 0, FS_SEEK_SET ) != 0 )
	{
		Common::error(ERR_FATAL, "Unable to set file pointer in file!\n" );
		return false;
	}

	tagList.clear();

	md3Header_t head;
	FileSystem::read(&head, sizeof(head), file);

	md3Frame_t	frame;
	for( int i = 0; i < head.numFrames; ++i ) 
		FileSystem::read(&frame, sizeof(frame), file);

	int total = head.numTags;
	md3Tag_t	tag;
	for( int i = 0; i < total; ++ i ) 
	{
		FileSystem::read(&tag, sizeof(tag), file);
		// a tag whose name starts with the string
		if( std::string_view( tag.name ).starts_with( str ) )
			tagList.push_back(tag);
	}

	return tagList.size();
}

bool
Md3Utils::getTagInfo( std::string const& filename,
					  std::string_view tagname,
					  md3Tag_t& tag )
{
	bool success = false;
	fileHandle_t file;

	if( FileSystem::fOpenFileByMode(filename.c_str(), &file, FS_READ) >= 0 ) 
		 success = getTagInfo( file, tagname, tag );
	else
		Common::error(ERR_FATAL, "Unable to open file %s\n", filename.c_str() );

	FileSystem::fCloseFile(file);

	return success;
}

bool
Md3Utils::getTagInfo( fileHandle_t const& file,
					  std::string_view tagname,
					  md3Tag_t& tag )
{
	if( !file )
	{
		Common::error(ERR_FATAL, "Invalid filehandle passed to Md3Utils::getTagInfo\n" );
		return false;
	}

	// put it back to beginning of file (just in case)
	if( FileSystem::seek( file, 0, FS_SEEK_SET ) != 0 )
	{
		Common::error(ERR_FATAL, "Unable to set file pointer in file!\n" );
		return false;
	}

	bool found = false;

	md3Header_t head;
	md3Frame_t	frame;

	FileSystem::read(&head, sizeof(head), file);
	for( int i = 0; i < head.numFrames; ++i ) 
		FileSystem::read(&frame, sizeof(frame), file);

	for( int i = 0; i < head.numTags; ++i ) 
	{
		FileSystem::read(&tag, sizeof(md3Tag_t), file);
		if( tagname == tag.name ) 
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
	
	if( FileSystem::fOpenFileByMode( fileName.c_str(), &file, FS_READ ) >= 0 ) 
	{
		int frames = getNumberOfFrames( file );
		FileSystem::fCloseFile( file );
		return frames;
	}

	Common::printf( "Unable to open file %s\n", fileName.c_str() );

	return 0;
}

int
Md3Utils::getNumberOfFrames( fileHandle_t const& file )
{
	if( !file )
	{
		Common::error(ERR_FATAL, "Invalid filehandle passed to Md3Utils::getNumberOfFrames\n" );
		return false;
	}

	// put it back to beginning of file (just in case)
	if( FileSystem::seek( file, 0, FS_SEEK_SET ) != 0 )
	{
		Common::error(ERR_FATAL, "Unable to set file pointer in file!\n" );
		return false;
	}

	md3Header_t head;
	FileSystem::read( &head, sizeof(head), file );
	return head.numFrames;
}



