/*
 * $Id: bg_md3util.c,v 1.3 2005-11-21 17:28:20 thebjoern Exp $
*/

#include "q_shared.h"
#include "../qcommon/qfiles.h"
#include "bg_public.h"







/*
=================
MF_findTag

providing the file and tagname this functions returns the taginfo
=================
*/


bool MF_findTag(const char* fileName, const char* tagname, md3Tag_t* tag)
{
	fileHandle_t	f;
	bool		found = false;

	if( FileSystem::fOpenFileByMode(fileName, &f, FS_READ) >= 0 ) {
		md3Header_t head;
		md3Frame_t	frame;
		int			i;	
		FileSystem::read(&head, sizeof(head), f);
//		CG_Printf("%s has %i tags\n", fileName, head.numTags );
		for( i = 0; i < head.numFrames; ++i ) {
			FileSystem::read(&frame, sizeof(frame), f);
		}
		for( i = 0; i < head.numTags; ++i ) {
			FileSystem::read(tag, sizeof(md3Tag_t), f);
			if( strcmp( tag->name, tagname ) == 0 ) {
				found = true;
				break;
			}
//			CG_Printf("tag %i: %s\n", i, tag.name );
//			CG_Printf("tag %i: %.1f %.1f %.1f\n", i, tag.origin[0], tag.origin[1], tag.origin[2] );
		}
		FileSystem::fCloseFile(f);
	} else {
		Com_Printf( "Unable to read tag from %s\n", fileName );
	}
	return found;
}

/*
=================
MF_getNumberOfTags

providing the file this functions returns the number of tags
=================
*/

bool MF_getNumberOfTags(const char* fileName, int* number)
{
	fileHandle_t	f;
	bool		found = false;

	if( !number ) return found;

	if( FileSystem::fOpenFileByMode(fileName, &f, FS_READ) >= 0 ) {
		md3Header_t head;
		FileSystem::read(&head, sizeof(head), f);
		*number = head.numTags;
		found = true;
		FileSystem::fCloseFile(f);
	} else {
		Com_Printf( "Unable to open file %s\n", fileName );
	}
	return found;

}

/*
=================
MF_getTagsContaining

providing the file this functions returns the tags containing str in their name
num is the number of tags provided in tags
=================
*/

int MF_getTagsContaining(const char* fileName, const char* str, md3Tag_t* tags, int num)
{
	fileHandle_t	f;
	int				i, number = 0, total, len;

	if( !num || !tags ) return 0;
	if( !str) return 0;
	len = strlen(str);
	if( !len ) return 0;

	if( FileSystem::fOpenFileByMode(fileName, &f, FS_READ) >= 0 ) {
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
				memcpy( &tags[number], &tag, sizeof(tag) );
				number++;
			}
		}
		FileSystem::fCloseFile(f);
	} else {
		Com_Printf( "Unable to open file %s\n", fileName );
	}
	return number;

}


/*
=================
MF_getNumberOfFrames

providing the file this functions returns the number of frames
=================
*/

bool MF_getNumberOfFrames(const char* fileName, int* number)
{
	fileHandle_t	f;
	bool		found = false;

	if( !number ) return found;

	if( FileSystem::fOpenFileByMode(fileName, &f, FS_READ) >= 0 ) {
		md3Header_t head;
		FileSystem::read(&head, sizeof(head), f);
		*number = head.numFrames;
		found = true;
		FileSystem::fCloseFile(f);
	} else {
		Com_Printf( "Unable to open file %s\n", fileName );
	}
	return found;

}

/*
=================
MF_getDimensions

providing the file and the framenumber this functions returns the dimensions of the model
=================
*/

bool MF_getDimensions(const char* fileName, int frame, vec3_t* maxs, vec3_t* mins)
{
	fileHandle_t	f;
	bool		found = false;
	int				i, number;

	if( FileSystem::fOpenFileByMode(fileName, &f, FS_READ) >= 0 ) {
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
		FileSystem::fCloseFile(f);
	} else {
		Com_Printf( "Unable to open file %s\n", fileName );
	}
	return found;

}



