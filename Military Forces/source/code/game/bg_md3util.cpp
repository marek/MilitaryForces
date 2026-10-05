/*
 * $Id: bg_md3util.c,v 1.3 2005-11-21 17:28:20 thebjoern Exp $
*/

#include "q_shared.h"
#include "../qcommon/qfiles.h"
#include "bg_public.h"







/*
=================
VehicleData::findTag

providing the file and tagname this functions returns the taginfo
=================
*/


bool VehicleData::findTag(const char* fileName, const char* tagname, md3Tag_t* tag)
{
	fileHandle_t	f;
	bool		found = false;

	if( const OpenFile file( fileName, FS_READ ); file ) {
		f = file.handle();
		md3Header_t head;
		md3Frame_t	frame;
		int			i;	
		FileSystem::read(&head, sizeof(head), f);
//		CGame::printf("%s has %i tags\n", fileName, head.numTags );
		for( i = 0; i < head.numFrames; ++i ) {
			FileSystem::read(&frame, sizeof(frame), f);
		}
		for( i = 0; i < head.numTags; ++i ) {
			FileSystem::read(tag, sizeof(md3Tag_t), f);
			if( strcmp( tag->name, tagname ) == 0 ) {
				found = true;
				break;
			}
//			CGame::printf("tag %i: %s\n", i, tag.name );
//			CGame::printf("tag %i: %.1f %.1f %.1f\n", i, tag.origin[0], tag.origin[1], tag.origin[2] );
		}
	} else {
		Common::printf( "Unable to read tag from %s\n", fileName );
	}
	return found;
}

/*
=================
VehicleData::getNumberOfTags

providing the file this functions returns the number of tags
=================
*/

bool VehicleData::getNumberOfTags(const char* fileName, int* number)
{
	fileHandle_t	f;
	bool		found = false;

	if( !number ) return found;

	if( const OpenFile file( fileName, FS_READ ); file ) {
		f = file.handle();
		md3Header_t head;
		FileSystem::read(&head, sizeof(head), f);
		*number = head.numTags;
		found = true;
	} else {
		Common::printf( "Unable to open file %s\n", fileName );
	}
	return found;

}

/*
=================
VehicleData::getTagsContaining

providing the file this functions returns the tags containing str in their name
num is the number of tags provided in tags
=================
*/

int VehicleData::getTagsContaining(const char* fileName, const char* str, md3Tag_t* tags, int num)
{
	fileHandle_t	f;
	int				i, number = 0, total, len;

	if( !num || !tags ) return 0;
	if( !str) return 0;
	len = strlen(str);
	if( !len ) return 0;

	if( const OpenFile file( fileName, FS_READ ); file ) {
		f = file.handle();
		md3Header_t head;
		md3Frame_t	frame;
		md3Tag_t	tag;
		FileSystem::read(&head, sizeof(head), f);
		for( i = 0; i < head.numFrames; ++i ) {
			FileSystem::read(&frame, sizeof(frame), f);
		}
		total = head.numTags;
		if( total > num ) total = num;
		for( i = 0; i < total; ++ i ) {
			FileSystem::read(&tag, sizeof(tag), f);
			if( Q_strncmp( tag.name, str, len ) == 0 ) {
				tags[number] = tag;
				number++;
			}
		}
	} else {
		Common::printf( "Unable to open file %s\n", fileName );
	}
	return number;

}


/*
=================
VehicleData::getNumberOfFrames

providing the file this functions returns the number of frames
=================
*/

bool VehicleData::getNumberOfFrames(const char* fileName, int* number)
{
	fileHandle_t	f;
	bool		found = false;

	if( !number ) return found;

	if( const OpenFile file( fileName, FS_READ ); file ) {
		f = file.handle();
		md3Header_t head;
		FileSystem::read(&head, sizeof(head), f);
		*number = head.numFrames;
		found = true;
	} else {
		Common::printf( "Unable to open file %s\n", fileName );
	}
	return found;

}

/*
=================
VehicleData::getDimensions

providing the file and the framenumber this functions returns the dimensions of the model
=================
*/

bool VehicleData::getDimensions(const char* fileName, int frame, vec3_t* maxs, vec3_t* mins)
{
	fileHandle_t	f;
	bool		found = false;
	int				i, number;

	if( const OpenFile file( fileName, FS_READ ); file ) {
		f = file.handle();
		md3Header_t head;
		md3Frame_t frames;
		
		FileSystem::read(&head, sizeof(head), f);
		number = head.numFrames;
		if( frame >= 0 && frame < number ) {
			for( i = 0; i <= frame; ++i ) {
				FileSystem::read(&frames, sizeof(frames), f);
			}
			if( mins ) 
				VectorCopy( frames.bounds[0], *mins );
			if( maxs ) 
				VectorCopy( frames.bounds[1], *maxs );
			found = true;
		}
	} else {
		Common::printf( "Unable to open file %s\n", fileName );
	}
	return found;

}



