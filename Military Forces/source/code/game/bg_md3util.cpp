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


std::optional<md3Tag_t> VehicleData::findTag(const char* fileName, const char* tagname)
{
	const OpenFile file( fileName, FS_READ );
	if( !file ) {
		Common::printf( "Unable to read tag from %s\n", fileName );
		return std::nullopt;
	}
	const fileHandle_t f = file.handle();
	md3Header_t head;
	md3Frame_t	frame;
	FileSystem::read(&head, sizeof(head), f);
	for( int i = 0; i < head.numFrames; ++i ) {
		FileSystem::read(&frame, sizeof(frame), f);
	}
	for( int i = 0; i < head.numTags; ++i ) {
		md3Tag_t tag;
		FileSystem::read(&tag, sizeof(tag), f);
		if( strcmp( tag.name, tagname ) == 0 ) {
			return tag;
		}
	}
	return std::nullopt;
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

the number of frames in a model file, or nothing when the file is not there (quiet: part files are optional)
=================
*/

std::optional<int> VehicleData::getNumberOfFrames(const char* fileName)
{
	const OpenFile file( fileName, FS_READ );
	if( !file )
		return std::nullopt;

	md3Header_t head;
	FileSystem::read(&head, sizeof(head), file.handle());
	return head.numFrames;
}

/*
=================
VehicleData::getDimensions

providing the file and the framenumber this functions returns the dimensions of the model
=================
*/

std::optional<ModelBounds> VehicleData::getDimensions(const char* fileName, int frame)
{
	const OpenFile file( fileName, FS_READ );
	if( !file ) {
		Common::printf( "Unable to open file %s\n", fileName );
		return std::nullopt;
	}
	const fileHandle_t f = file.handle();
	md3Header_t head;
	md3Frame_t frames;

	FileSystem::read(&head, sizeof(head), f);
	if( frame < 0 || frame >= head.numFrames ) {
		return std::nullopt;
	}
	for( int i = 0; i <= frame; ++i ) {
		FileSystem::read(&frames, sizeof(frames), f);
	}
	ModelBounds bounds;
	VectorCopy( frames.bounds[0], bounds.mins );
	VectorCopy( frames.bounds[1], bounds.maxs );
	return bounds;
}



