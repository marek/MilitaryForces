/*
 * $Id: ui_gameinfo.c,v 1.7 2005-11-21 17:28:32 thebjoern Exp $
*/
//
// gameinfo.c
//

#include "ui_local.h"


//
// arena and bot info
//


//int				ui_numBots;
//static char		*ui_botInfos[MAX_BOTS];

static int		ui_numArenas;
static char		*ui_arenaInfos[MAX_ARENAS];

/*
===============
UI_ParseInfos
===============
*/
int UI_ParseInfos( char *buf, int max, char *infos[] ) 
{
	char	*token;
	int		count;
	char	key[MAX_TOKEN_CHARS];
	char	info[MAX_INFO_STRING];

	count = 0;

	while ( 1 ) 
	{
		token = COM_Parse( &buf );
		if ( !token[0] ) 
			break;
		
		if ( strcmp( token, "{" ) ) 
		{
			Com_Printf( "Missing { in info file\n" );
			break;
		}

		if ( count == max ) 
		{
			Com_Printf( "Max infos exceeded\n" );
			break;
		}

		info[0] = '\0';
		while ( 1 ) 
		{
			token = COM_ParseExt( &buf, true );
			if ( !token[0] ) 
			{
				Com_Printf( "Unexpected end of info file\n" );
				break;
			}
			if ( !strcmp( token, "}" ) ) 
				break;
			
			Q_strncpyz( key, token, sizeof( key ) );

			token = COM_ParseExt( &buf, false );
			if ( !token[0] ) 
				strcpy( token, "<NULL>" );
			
			Info_SetValueForKey( info, key, token );
		}
		//NOTE: extra space for arena number
		infos[count] = reinterpret_cast<char*>(uiInfo.uiUtils.alloc(strlen(info) + strlen("\\num\\") + 
			strlen(va("%d", MAX_ARENAS)) + 1));
		if (infos[count]) 
		{
			strcpy(infos[count], info);
			count++;
		}
	}
	return count;
}

/*
===============
UI_LoadArenasFromFile
===============
*/
static void UI_LoadArenasFromFile( char *filename ) {
	int				len;
	fileHandle_t	f;
	char			buf[MAX_ARENAS_TEXT];

	len = FS_FOpenFileByMode( filename, &f, FS_READ );
	if ( !f ) {
		Com_Printf( S_COLOR_RED "file not found: %s\n", filename );
		return;
	}
	if ( len >= MAX_ARENAS_TEXT ) {
		Com_Printf( S_COLOR_RED "file too large: %s is %i, max allowed is %i", filename, len, MAX_ARENAS_TEXT );
		FS_FCloseFile( f );
		return;
	}

	FS_Read2( buf, len, f );
	buf[len] = 0;
	FS_FCloseFile( f );

	ui_numArenas += UI_ParseInfos( buf, MAX_ARENAS - ui_numArenas, &ui_arenaInfos[ui_numArenas] );
}

/*
===============
UI_AlphabeticMapNameQsortCompare

Used to sort the maps into alphabetic order
===============
*/
int UI_AlphabeticMapNameQsortCompare( const void * arg1, const void * arg2 )
{
   /* Compare all of both strings: */
   return Q_stricmp( ((mapInfo *) arg1)->mapName, ((mapInfo *) arg2)->mapName );
}

/*
===============
UI_LoadArenas
===============
*/
void UI_LoadArenas( void ) {
	int			numdirs;
	vmCvar_t	arenasFile;
	char		filename[128];
	char		dirlist[1024];
	char*		dirptr;
	int			i, n;
	int			dirlen;
	char		*type, *game;

	ui_numArenas = 0;
	uiInfo.mapCount = 0;

	Cvar_Register( &arenasFile, "g_arenasFile", "", CVAR_INIT|CVAR_ROM );
	if( *arenasFile.string ) {
		UI_LoadArenasFromFile(arenasFile.string);
	}
	else {
		UI_LoadArenasFromFile("scripts/arenas.txt");
	}

	// get all arenas from .arena files
	numdirs = FS_GetFileList("scripts", ".arena", dirlist, 1024 );
	dirptr  = dirlist;
	for (i = 0; i < numdirs; i++, dirptr += dirlen+1) {
		dirlen = strlen(dirptr);
		strcpy(filename, "scripts/");
		strcat(filename, dirptr);
		UI_LoadArenasFromFile(filename);
	}
	Com_Printf( "%i arenas parsed\n", ui_numArenas );
	if (uiInfo.uiUtils.outOfMemory()) {
		Com_Printf(S_COLOR_YELLOW"WARNING: not anough memory in pool to load all arenas\n");
	}

	for( n = 0; n < ui_numArenas; n++ )
	{
		// only allow mfq3 maps
		game = Info_ValueForKey( ui_arenaInfos[n], "game" );
		if( strcmp( game, "mfq3" ) != 0 )
		{
			// not a mfq3 map
			continue;
		}

		// determine type

		uiInfo.mapList[uiInfo.mapCount].cinematic = -1;
		uiInfo.mapList[uiInfo.mapCount].mapLoadName = uiInfo.uiUtils.string_Alloc(Info_ValueForKey(ui_arenaInfos[n], "map"));
		uiInfo.mapList[uiInfo.mapCount].mapName = uiInfo.uiUtils.string_Alloc(Info_ValueForKey(ui_arenaInfos[n], "longname"));
		uiInfo.mapList[uiInfo.mapCount].levelShot = -1;
		uiInfo.mapList[uiInfo.mapCount].imageName = uiInfo.uiUtils.string_Alloc(va("levelshots/%s", uiInfo.mapList[uiInfo.mapCount].mapLoadName));
		uiInfo.mapList[uiInfo.mapCount].typeBits = 0;

		type = Info_ValueForKey( ui_arenaInfos[n], "type" );
		
		// if no type specified, it will be treated as "dm"
		if( *type )
		{
			// deathmatch (Free For All) ('ffa' supported for backwards compatibility)
			if( strstr( type, "dm" ) || strstr( type, "ffa" ) ) {
				uiInfo.mapList[uiInfo.mapCount].typeBits |= (1 << GT_FFA);
			}
			// team deathmatch ('team' supported for backwards compatibility)
			if( strstr( type, "tdm" ) || strstr( type, "team" ) ) {
				uiInfo.mapList[uiInfo.mapCount].typeBits |= (1 << GT_TEAM);
			}
			// capture the flag
			if( strstr( type, "ctf" ) ) {
				uiInfo.mapList[uiInfo.mapCount].typeBits |= (1 << GT_CTF);
			}
		}
		else
		{
			// default
			uiInfo.mapList[uiInfo.mapCount].typeBits |= (1 << GT_FFA);
		}

		uiInfo.mapCount++;
		if (uiInfo.mapCount >= MAX_MAPS) {
			break;
		}
	}

	// perform an alphabetic sort on the map names
	qsort( &uiInfo.mapList[0], uiInfo.mapCount, sizeof( mapInfo ), UI_AlphabeticMapNameQsortCompare );
}


