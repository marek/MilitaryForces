/*
 * $Id: bg_md3util.c,v 1.1 2002-02-25 13:16:54 thebjoern Exp $
*/

#include "q_shared.h"
#include "bg_public.h"







/*
=================
MF_findTag

providing the file and tagname this functions returns the taginfo
=================
*/
int		trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode );
void	trap_FS_Read( void *buffer, int len, fileHandle_t f );
void	trap_FS_FCloseFile( fileHandle_t f );


qboolean MF_findTag(const char* fileName, const char* tagname, md3Tag_t* tag)
{
	fileHandle_t	f;
	qboolean		found = qfalse;

	if( trap_FS_FOpenFile(fileName, &f, FS_READ) >= 0 ) {
		md3Header_t head;
		md3Frame_t	frame;
		int			i;	
		trap_FS_Read(&head, sizeof(head), f);
//		CG_Printf("%s has %i tags\n", fileName, head.numTags );
		for( i = 0; i < head.numFrames; ++i ) {
			trap_FS_Read(&frame, sizeof(frame), f);
		}
		for( i = 0; i < head.numTags; ++i ) {
			trap_FS_Read(tag, sizeof(md3Tag_t), f);
			if( strcmp( tag->name, tagname ) == 0 ) {
				found = qtrue;
				break;
			}
//			CG_Printf("tag %i: %s\n", i, tag.name );
//			CG_Printf("tag %i: %.1f %.1f %.1f\n", i, tag.origin[0], tag.origin[1], tag.origin[2] );
		}
		trap_FS_FCloseFile(f);
	} else {
		Com_Printf( "Unable to read tag from &s\n", fileName );
	}
	return found;
}






