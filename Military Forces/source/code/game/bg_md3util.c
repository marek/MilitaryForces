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
int FS_FOpenFileByMode( const char *qpath, fileHandle_t *f, fsMode_t mode );
int FS_Read2( void *buffer, int len, fileHandle_t f );
void FS_FCloseFile( fileHandle_t f );


bool MF_findTag(const char* fileName, const char* tagname, md3Tag_t* tag)
{
	fileHandle_t	f;
	bool		found = false;

	if( FS_FOpenFileByMode(fileName, &f, FS_READ) >= 0 ) {
		md3Header_t head;
		md3Frame_t	frame;
		int			i;	
		FS_Read2(&head, sizeof(head), f);
//		CG_Printf("%s has %i tags\n", fileName, head.numTags );
		for( i = 0; i < head.numFrames; ++i ) {
			FS_Read2(&frame, sizeof(frame), f);
		}
		for( i = 0; i < head.numTags; ++i ) {
			FS_Read2(tag, sizeof(md3Tag_t), f);
			if( strcmp( tag->name, tagname ) == 0 ) {
				found = true;
				break;
			}
//			CG_Printf("tag %i: %s\n", i, tag.name );
//			CG_Printf("tag %i: %.1f %.1f %.1f\n", i, tag.origin[0], tag.origin[1], tag.origin[2] );
		}
		FS_FCloseFile(f);
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

	if( FS_FOpenFileByMode(fileName, &f, FS_READ) >= 0 ) {
		md3Header_t head;
		FS_Read2(&head, sizeof(head), f);
		*number = head.numTags;
		found = true;
		FS_FCloseFile(f);
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

	if( FS_FOpenFileByMode(fileName, &f, FS_READ) >= 0 ) {
		md3Header_t head;
		md3Frame_t	frame;
		md3Tag_t	tag;
		FS_Read2(&head, sizeof(head), f);
		for( i = 0; i < head.numFrames; ++i ) {
			FS_Read2(&frame, sizeof(frame), f);
		}
		total = head.numTags;
		if( total > num ) total = num;
		for( i = 0; i < total; ++ i ) {
			FS_Read2(&tag, sizeof(tag), f);
			if( Q_strncmp( tag.name, str, len ) == 0 ) {
				memcpy( &tags[number], &tag, sizeof(tag) );
				number++;
			}
		}
		FS_FCloseFile(f);
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

	if( FS_FOpenFileByMode(fileName, &f, FS_READ) >= 0 ) {
		md3Header_t head;
		FS_Read2(&head, sizeof(head), f);
		*number = head.numFrames;
		found = true;
		FS_FCloseFile(f);
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

	if( FS_FOpenFileByMode(fileName, &f, FS_READ) >= 0 ) {
		md3Header_t head;
		md3Frame_t frames;
		
		FS_Read2(&head, sizeof(head), f);
		number = head.numFrames;
		if( frame >= 0 && frame < number ) {
			for( i = 0; i <= frame; ++i ) {
				FS_Read2(&frames, sizeof(frames), f);
			}
			if( mins ) 
				VectorCopy( frames.bounds[0], *mins );
			if( maxs ) 
				VectorCopy( frames.bounds[1], *maxs );
			found = true;
		}
		FS_FCloseFile(f);
	} else {
		Com_Printf( "Unable to open file %s\n", fileName );
	}
	return found;

}



