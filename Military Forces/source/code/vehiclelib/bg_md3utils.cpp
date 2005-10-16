
#include "../game/q_shared.h"
#include "../qcommon/qfiles.h"
#include "../game/bg_public.h"
#include "bg_md3utils.h"

// decls
int		trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode );
void	trap_FS_Read( void *buffer, int len, fileHandle_t f );
void	trap_FS_FCloseFile( fileHandle_t f );




int
Md3Utils::getTagsContaining( std::string const& filename, 
							 std::string const& str,
							 std::vector<md3Tag_t>& tagList )
{
	if( str.empty() || str == "" ) 
		return 0;

	tagList.clear();
	fileHandle_t	f;
	if( trap_FS_FOpenFile(filename.c_str(), &f, FS_READ) >= 0 ) 
	{
		md3Header_t head;
		md3Frame_t	frame;
		md3Tag_t	tag;
		trap_FS_Read(&head, sizeof(head), f);
		for( int i = 0; i < head.numFrames; ++i ) 
			trap_FS_Read(&frame, sizeof(frame), f);
		int total = head.numTags;
		for( int i = 0; i < total; ++ i ) 
		{
			trap_FS_Read(&tag, sizeof(tag), f);
			std::string tagName(tag.name);
			// if it contains the string at the first position, then add it
			if( tagName.find(str) == 0 )
				tagList.push_back(tag);
		}
		trap_FS_FCloseFile(f);
	} 
	else 
	{
		Com_Printf( "Unable to open file %s\n", filename.c_str() );
	}
	return tagList.size();
}

bool
Md3Utils::getModelDimensions( std::string const& filename,
							  vec3_t& mins,
							  vec3_t& maxs,
							  int checkFrame )
{
	fileHandle_t		file;
	bool				found = false;

	// get body bounding boxes
	if( trap_FS_FOpenFile(filename.c_str(), &file, FS_READ) >= 0 ) 
	{
		md3Header_t header;
		md3Frame_t currentFrame;

		trap_FS_Read(&header, sizeof(header), file);
		int number = header.numFrames;

		if( checkFrame >= number )
		{
			Com_Error(ERR_FATAL, "File %s only has %d frames, therefore unable to check frame %d\n", 
						filename.c_str(), number, checkFrame );
			trap_FS_FCloseFile(file);
			return false;
		}
		else if( checkFrame == FRAMESIZE_FIRST )
			checkFrame = 0;
		else if( checkFrame == FRAMESIZE_LAST )
			checkFrame = number - 1;

		found = true;

		for( int i = 0; i < number; ++i ) 
		{
			trap_FS_Read(&currentFrame, sizeof(currentFrame), file);

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
		trap_FS_FCloseFile(file);
	} 
	else 
	{
		Com_Error(ERR_FATAL, "Unable to open file %s\n", filename.c_str() );
	}
	return found;
	
}

