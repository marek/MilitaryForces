/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
// console.c

#include "client.h"
#include "../ui/ui.h"
#include "../cgame/cg.h"


constexpr int DEFAULT_CONSOLE_WIDTH = 78;
int g_console_field_width = DEFAULT_CONSOLE_WIDTH;
int g_smallchar_width = SMALLCHAR_WIDTH;
int g_smallchar_height = SMALLCHAR_HEIGHT;

constexpr int NUM_CON_TIMES = 17;

constexpr int CON_TEXTSIZE = 32768;
struct console_t {
	bool	initialized;

	short	text[CON_TEXTSIZE];
	int		current;		// line where next message will be printed
	int		x;				// offset in current line for next print
	int		display;		// bottom of console displays this line

	int 	linewidth;		// characters across screen
	int		totallines;		// total lines in console scrollback

	float	xadjust;		// for wide aspect screens

	float	displayFrac;	// aproaches finalFrac at scr_conspeed
	float	finalFrac;		// 0.0 to 1.0 lines of console to display

	int		vislines;		// in scanlines

	int		times[NUM_CON_TIMES];	// cls.realtime time the line was generated
								// for transparent notify lines
	vec4_t	color;
};

console_t	con;

cvar_t		*con_conspeed;
cvar_t		*con_autoclear;
cvar_t		*con_notifytime;
cvar_t		*con_notifylines;
cvar_t		*con_scale;



/*
================
Console::toggleConsole_f
================
*/
void Console::toggleConsole_f () {
	// Can't toggle the console when it's the only thing available
	if ( clc.state == CA_DISCONNECTED && Key_GetCatcher( ) == KEYCATCH_CONSOLE ) {
		return;
	}

	if ( con_autoclear->integer ) {
		g_consoleField.clear();
	}

	g_consoleField.widthInChars = g_console_field_width;

	Console::clearNotify ();
	Key_SetCatcher( Key_GetCatcher( ) ^ KEYCATCH_CONSOLE );
}

/*
===================
Con_ToggleMenu_f
===================
*/
void Con_ToggleMenu_f( ) {
	CL_KeyEvent( K_ESCAPE, true, System::milliseconds() );
	CL_KeyEvent( K_ESCAPE, false, System::milliseconds() );
}

/*
================
Con_MessageMode_f
================
*/
void Con_MessageMode_f () {
	chat_playerNum = -1;
	chat_team = false;
	chatField.clear();
	chatField.widthInChars = 30;

	Key_SetCatcher( Key_GetCatcher( ) ^ KEYCATCH_MESSAGE );
}

/*
================
Con_MessageMode2_f
================
*/
void Con_MessageMode2_f () {
	chat_playerNum = -1;
	chat_team = true;
	chatField.clear();
	chatField.widthInChars = 25;
	Key_SetCatcher( Key_GetCatcher( ) ^ KEYCATCH_MESSAGE );
}

/*
================
Con_MessageMode3_f
================
*/
void Con_MessageMode3_f () {
	chat_playerNum = theCG.crosshairPlayer();
	if ( chat_playerNum < 0 || chat_playerNum >= MAX_CLIENTS ) {
		chat_playerNum = -1;
		return;
	}
	chat_team = false;
	chatField.clear();
	chatField.widthInChars = 30;
	Key_SetCatcher( Key_GetCatcher( ) ^ KEYCATCH_MESSAGE );
}

/*
================
Con_MessageMode4_f
================
*/
void Con_MessageMode4_f () {
	chat_playerNum = theCG.lastAttacker();
	if ( chat_playerNum < 0 || chat_playerNum >= MAX_CLIENTS ) {
		chat_playerNum = -1;
		return;
	}
	chat_team = false;
	chatField.clear();
	chatField.widthInChars = 30;
	Key_SetCatcher( Key_GetCatcher( ) ^ KEYCATCH_MESSAGE );
}

/*
================
Console::clear_f
================
*/
void Console::clear_f () {
	int		i;

	for ( i = 0 ; i < CON_TEXTSIZE ; i++ ) {
		con.text[i] = (ColorIndex(COLOR_WHITE)<<8) | ' ';
	}

	Console::bottom();		// go to end
}

						
/*
================
Con_Dump_f

Save the console contents out to a file
================
*/
void Con_Dump_f ()
{
	int		l, x, i;
	short	*line;
	fileHandle_t	f;
	int		bufferlen;
	char	*buffer;
	char	filename[MAX_QPATH];

	if (CommandSystem::argc() != 2)
	{
		Common::printf ("usage: condump <filename>\n");
		return;
	}

	Q_strncpyz( filename, CommandSystem::argv( 1 ), sizeof( filename ) );
	COM_DefaultExtension( filename, sizeof( filename ), ".txt" );

	if (!COM_CompareExtension(filename, ".txt"))
	{
		Common::printf("Con_Dump_f: Only the \".txt\" extension is supported by this command!\n");
		return;
	}

	f = FileSystem::fOpenFileWrite_HomeData( filename );
	if (!f)
	{
		Common::printf ("ERROR: couldn't open %s.\n", filename);
		return;
	}

	Common::printf ("Dumped console text to %s.\n", filename );

	// skip empty lines
	for (l = con.current - con.totallines + 1 ; l <= con.current ; l++)
	{
		line = con.text + (l%con.totallines)*con.linewidth;
		for (x=0 ; x<con.linewidth ; x++)
			if ((line[x] & 0xff) != ' ')
				break;
		if (x != con.linewidth)
			break;
	}

#ifdef _WIN32
	bufferlen = con.linewidth + 3 * sizeof ( char );
#else
	bufferlen = con.linewidth + 2 * sizeof ( char );
#endif

	buffer = static_cast<char *>(Hunk_AllocateTempMemory( bufferlen ));

	// write the remaining lines
	buffer[bufferlen-1] = 0;
	for ( ; l <= con.current ; l++)
	{
		line = con.text + (l%con.totallines)*con.linewidth;
		for(i=0; i<con.linewidth; i++)
			buffer[i] = line[i] & 0xff;
		for (x=con.linewidth-1 ; x>=0 ; x--)
		{
			if (buffer[x] == ' ')
				buffer[x] = 0;
			else
				break;
		}
#ifdef _WIN32
		Q_strcat(buffer, bufferlen, "\r\n");
#else
		Q_strcat(buffer, bufferlen, "\n");
#endif
		FileSystem::write(buffer, strlen(buffer), f);
	}

	Hunk_FreeTempMemory( buffer );
	FileSystem::fCloseFile( f );
}

						
/*
================
Console::clearNotify
================
*/
void Console::clearNotify( ) {
	int		i;
	
	for ( i = 0 ; i < NUM_CON_TIMES ; i++ ) {
		con.times[i] = 0;
	}
}

						

/*
================
Console::checkResize

If the line width has changed, reformat the buffer.
================
*/
void Console::checkResize ()
{
	int		i, j, width, oldwidth, oldtotallines, numlines, numchars;
	short	tbuf[CON_TEXTSIZE];

	if (con_scale != nullptr)
	{
		g_smallchar_width = static_cast<int>(static_cast<float>(SMALLCHAR_WIDTH) * con_scale->value);
		g_smallchar_height = static_cast<int>(static_cast<float>(SMALLCHAR_HEIGHT) * con_scale->value);
	}

	width = (cls.glconfig.vidWidth / g_smallchar_width) - 2;

	if (width == con.linewidth)
		return;

	if (width < 1)			// video hasn't been initialized yet
	{
		width = DEFAULT_CONSOLE_WIDTH;
		con.linewidth = width;
		con.totallines = CON_TEXTSIZE / con.linewidth;
		for(i=0; i<CON_TEXTSIZE; i++)

			con.text[i] = (ColorIndex(COLOR_WHITE)<<8) | ' ';
	}
	else
	{
		oldwidth = con.linewidth;
		con.linewidth = width;
		oldtotallines = con.totallines;
		con.totallines = CON_TEXTSIZE / con.linewidth;
		numlines = oldtotallines;

		if (con.totallines < numlines)
			numlines = con.totallines;

		numchars = oldwidth;
	
		if (con.linewidth < numchars)
			numchars = con.linewidth;

		Com_Memcpy (tbuf, con.text, CON_TEXTSIZE * sizeof(short));
		for(i=0; i<CON_TEXTSIZE; i++)

			con.text[i] = (ColorIndex(COLOR_WHITE)<<8) | ' ';


		for (i=0 ; i<numlines ; i++)
		{
			for (j=0 ; j<numchars ; j++)
			{
				con.text[(con.totallines - 1 - i) * con.linewidth + j] =
						tbuf[((con.current - i + oldtotallines) %
							  oldtotallines) * oldwidth + j];
			}
		}

		Console::clearNotify ();
	}

	con.current = con.totallines - 1;
	con.display = con.current;
}

/*
==================
Cmd_CompleteTxtName
==================
*/
void Cmd_CompleteTxtName( char *args, int argNum ) {
	if( argNum == 2 ) {
		Field_CompleteFilename( "", "txt", nullptr, false, true );
	}
}


/*
================
Console::init
================
*/
void Console::init () {
	int		i;

	con_notifytime = CvarSystem::get ("con_notifytime", "3", CVAR_ARCHIVE);
	con_notifylines = CvarSystem::get ("con_notifylines", "3", CVAR_ARCHIVE);
	CvarSystem::checkRange(con_notifylines, 1, NUM_CON_TIMES - 1, true);
	con_conspeed = CvarSystem::get ("scr_conspeed", "3", CVAR_ARCHIVE);
	con_autoclear = CvarSystem::get("con_autoclear", "1", CVAR_ARCHIVE);
	con_scale = CvarSystem::get("con_scale", "1", CVAR_ARCHIVE);
	CvarSystem::checkRange(con_scale, 1.0f, 4.0f, false);

	g_consoleField.clear();
	g_consoleField.widthInChars = g_console_field_width;
	for ( i = 0 ; i < COMMAND_HISTORY ; i++ ) {
		historyEditLines[i].clear();
		historyEditLines[i].widthInChars = g_console_field_width;
	}
	CL_LoadConsoleHistory( );

	CommandSystem::addCommand ("toggleconsole", Console::toggleConsole_f);
	CommandSystem::addCommand ("togglemenu", Con_ToggleMenu_f);
	CommandSystem::addCommand ("messagemode", Con_MessageMode_f);
	CommandSystem::addCommand ("messagemode2", Con_MessageMode2_f);
	CommandSystem::addCommand ("messagemode3", Con_MessageMode3_f);
	CommandSystem::addCommand ("messagemode4", Con_MessageMode4_f);
	CommandSystem::addCommand ("clear", Console::clear_f);
	CommandSystem::addCommand ("condump", Con_Dump_f);
	CommandSystem::setCommandCompletionFunc( "condump", Cmd_CompleteTxtName );
}

/*
================
Console::shutdown
================
*/
void Console::shutdown()
{
	CommandSystem::removeCommand("toggleconsole");
	CommandSystem::removeCommand("togglemenu");
	CommandSystem::removeCommand("messagemode");
	CommandSystem::removeCommand("messagemode2");
	CommandSystem::removeCommand("messagemode3");
	CommandSystem::removeCommand("messagemode4");
	CommandSystem::removeCommand("clear");
	CommandSystem::removeCommand("condump");
}

/*
===============
Con_Linefeed
===============
*/
void Con_Linefeed (bool skipnotify)
{
	int		i;

	// mark time for transparent overlay
	if (con.current >= 0)
	{
    if (skipnotify)
		  con.times[con.current % NUM_CON_TIMES] = 0;
    else
		  con.times[con.current % NUM_CON_TIMES] = cls.realtime;
	}

	con.x = 0;
	if (con.display == con.current)
		con.display++;
	con.current++;
	for(i=0; i<con.linewidth; i++)
		con.text[(con.current%con.totallines)*con.linewidth+i] = (ColorIndex(COLOR_WHITE)<<8) | ' ';
}

/*
================
CL_ConsolePrint

Handles cursor positioning, line wrapping, etc
All console printing must go through this in order to be logged to disk
If no console is visible, the text will appear at the top of the game window
================
*/
void CL_ConsolePrint( char *txt ) {
	int		y, l;
	unsigned char	c;
	unsigned short	color;
	bool skipnotify = false;		// NERVE - SMF
	int prev;							// NERVE - SMF

	// TTimo - prefix for text that shows up in console but not in notify
	// backported from RTCW
	if ( !Q_strncmp( txt, "[skipnotify]", 12 ) ) {
		skipnotify = true;
		txt += 12;
	}
	
	// for some demos we don't want to ever show anything on the console
	if ( cl_noprint && cl_noprint->integer ) {
		return;
	}
	
	if (!con.initialized) {
		con.color[0] = 
		con.color[1] = 
		con.color[2] =
		con.color[3] = 1.0f;
		con.linewidth = -1;
		Console::checkResize ();
		con.initialized = true;
	}

	color = ColorIndex(COLOR_WHITE);

	while ( (c = *(reinterpret_cast<unsigned char *>(txt))) != 0 ) {
		if ( Q_IsColorString( txt ) ) {
			color = ColorIndex( *(txt+1) );
			txt += 2;
			continue;
		}

		// count word length
		for (l=0 ; l< con.linewidth ; l++) {
			if ( txt[l] <= ' ') {
				break;
			}

		}

		// word wrap
		if (l != con.linewidth && (con.x + l >= con.linewidth) ) {
			Con_Linefeed(skipnotify);

		}

		txt++;

		switch (c)
		{
		case '\n':
			Con_Linefeed (skipnotify);
			break;
		case '\r':
			con.x = 0;
			break;
		default:	// display character and advance
			y = con.current % con.totallines;
			con.text[y*con.linewidth+con.x] = (color << 8) | c;
			con.x++;
			if(con.x >= con.linewidth)
				Con_Linefeed(skipnotify);
			break;
		}
	}


	// mark time for transparent overlay
	if (con.current >= 0) {
		// NERVE - SMF
		if ( skipnotify ) {
			prev = con.current % NUM_CON_TIMES - 1;
			if ( prev < 0 )
				prev = NUM_CON_TIMES - 1;
			con.times[prev] = 0;
		}
		else
		// -NERVE - SMF
			con.times[con.current % NUM_CON_TIMES] = cls.realtime;
	}
}


/*
==============================================================================

DRAWING

==============================================================================
*/


/*
================
Con_DrawInput

Draw the editline after a ] prompt
================
*/
void Con_DrawInput () {
	int		y;

	if ( clc.state != CA_DISCONNECTED && !(Key_GetCatcher( ) & KEYCATCH_CONSOLE ) ) {
		return;
	}

	y = con.vislines - ( g_smallchar_height * 2 );

	refExport.SetColor( con.color );

	Screen::drawSmallChar( con.xadjust + 1 * g_smallchar_width, y, ']' );

	g_consoleField.draw( con.xadjust + 2 * g_smallchar_width, y, SCREEN_WIDTH - 3 * g_smallchar_width, true, true );
}


/*
================
Console::drawNotify

Draws the last few lines of output transparently over the game top
================
*/
void Console::drawNotify ()
{
	int		x, v;
	short	*text;
	int		i;
	int		time;
	int		skip;
	int		currentColor;

	currentColor = 7;
	refExport.SetColor( g_color_table[currentColor] );

	v = 0;
	for (i= con.current-con_notifylines->integer ; i<=con.current ; i++)
	{
		int linelength = 0;

		if (i < 0)
			continue;
		time = con.times[i % NUM_CON_TIMES];
		if (time == 0)
			continue;
		time = cls.realtime - time;
		if (time > con_notifytime->value*1000)
			continue;
		text = con.text + (i % con.totallines)*con.linewidth;

		if (cl.snap.ps.pm_type != PM_INTERMISSION && Key_GetCatcher( ) & (KEYCATCH_UI | KEYCATCH_CGAME) ) {
			continue;
		}

		for (x = 0 ; x < con.linewidth ; x++) {
			if ( ( text[x] & 0xff ) == ' ' ) {
				continue;
			}
			if ( ColorIndexForNumber( text[x]>>8 ) != currentColor ) {
				currentColor = ColorIndexForNumber( text[x]>>8 );
				refExport.SetColor( g_color_table[currentColor] );
			}
			Screen::drawSmallChar( cl_conXOffset->integer + con.xadjust + (x+1)*g_smallchar_width, v, text[x] & 0xff );
			linelength++;
		}

		if (linelength > 0) {
			v += g_smallchar_height;
		}
	}

	// v is in native coordinates, convert to 640x480 for Screen::drawBigString
	v *= 480.0f / cls.glconfig.vidHeight;
	v += 8;

	refExport.SetColor( nullptr );

	if (Key_GetCatcher( ) & (KEYCATCH_UI | KEYCATCH_CGAME) ) {
		return;
	}

	// draw the chat line
	if ( Key_GetCatcher( ) & KEYCATCH_MESSAGE )
	{
		if (chat_team)
		{
			Screen::drawBigString (8, v, "say_team:", 1.0f, false );
			skip = 10;
		}
		else
		{
			Screen::drawBigString (8, v, "say:", 1.0f, false );
			skip = 5;
		}

		chatField.bigDraw( skip * BIGCHAR_WIDTH, v, SCREEN_WIDTH - ( skip + 1 ) * BIGCHAR_WIDTH, true, true );
	}

}

/*
================
Con_DrawSolidConsole

Draws the console with the solid background
================
*/
void Con_DrawSolidConsole( float frac ) {
	int				i, x, y;
	int				rows;
	short			*text;
	int				row;
	int				lines;
//	qhandle_t		conShader;
	int				currentColor;
	vec4_t			color;

	lines = cls.glconfig.vidHeight * frac;
	if (lines <= 0)
		return;

	if (lines > cls.glconfig.vidHeight )
		lines = cls.glconfig.vidHeight;

	// on wide screens, we will center the text
	con.xadjust = 0;
	Screen::adjustFrom640( &con.xadjust, nullptr, nullptr, nullptr );

	// draw the background
	y = frac * SCREEN_HEIGHT;
	if ( y < 1 ) {
		y = 0;
	}
	else {
		Screen::drawPic( 0, 0, SCREEN_WIDTH, y, cls.consoleShader );
	}

	color[0] = 1;
	color[1] = 0;
	color[2] = 0;
	color[3] = 1;
	Screen::fillRect( 0, y, SCREEN_WIDTH, 2, color );


	// draw the version number

	refExport.SetColor( g_color_table[ColorIndex(COLOR_RED)] );

	i = strlen( Q3_VERSION );

	for (x=0 ; x<i ; x++) {
		Screen::drawSmallChar( cls.glconfig.vidWidth - ( i - x + 1 ) * g_smallchar_width,
			lines - g_smallchar_height, Q3_VERSION[x] );
	}


	// draw the text
	con.vislines = lines;
	rows = (lines-g_smallchar_height)/g_smallchar_height;		// rows of text to draw

	y = lines - (g_smallchar_height*3);

	// draw from the bottom up
	if (con.display != con.current)
	{
	// draw arrows to show the buffer is backscrolled
		refExport.SetColor( g_color_table[ColorIndex(COLOR_RED)] );
		for (x=0 ; x<con.linewidth ; x+=4)
			Screen::drawSmallChar( con.xadjust + (x+1)*g_smallchar_width, y, '^' );
		y -= g_smallchar_height;
		rows--;
	}
	
	row = con.display;

	if ( con.x == 0 ) {
		row--;
	}

	currentColor = 7;
	refExport.SetColor( g_color_table[currentColor] );

	for (i=0 ; i<rows ; i++, y -= g_smallchar_height, row--)
	{
		if (row < 0)
			break;
		if (con.current - row >= con.totallines) {
			// past scrollback wrap point
			continue;	
		}

		text = con.text + (row % con.totallines)*con.linewidth;

		for (x=0 ; x<con.linewidth ; x++) {
			if ( ( text[x] & 0xff ) == ' ' ) {
				continue;
			}

			if ( ColorIndexForNumber( text[x]>>8 ) != currentColor ) {
				currentColor = ColorIndexForNumber( text[x]>>8 );
				refExport.SetColor( g_color_table[currentColor] );
			}
			Screen::drawSmallChar(  con.xadjust + (x+1)*g_smallchar_width, y, text[x] & 0xff );
		}
	}

	// draw the input prompt, user text, and cursor if desired
	Con_DrawInput ();

	refExport.SetColor( nullptr );
}



/*
==================
Console::drawConsole
==================
*/
void Console::drawConsole( ) {
	// check for console width changes from a vid mode change
	Console::checkResize ();

	// if disconnected, render console full screen
	if ( clc.state == CA_DISCONNECTED ) {
		if ( !( Key_GetCatcher( ) & (KEYCATCH_UI | KEYCATCH_CGAME)) ) {
			Con_DrawSolidConsole( 1.0 );
			return;
		}
	}

	if ( con.displayFrac ) {
		Con_DrawSolidConsole( con.displayFrac );
	} else {
		// draw notify lines
		if ( clc.state == CA_ACTIVE ) {
			Console::drawNotify ();
		}
	}
}

//================================================================

/*
==================
Console::runConsole

Scroll it up or down
==================
*/
void Console::runConsole () {
	// decide on the destination height of the console
	if ( Key_GetCatcher( ) & KEYCATCH_CONSOLE )
		con.finalFrac = 0.5;		// half screen
	else
		con.finalFrac = 0;				// none visible
	
	// scroll towards the destination height
	if (con.finalFrac < con.displayFrac)
	{
		con.displayFrac -= con_conspeed->value*cls.realFrametime*0.001;
		if (con.finalFrac > con.displayFrac)
			con.displayFrac = con.finalFrac;

	}
	else if (con.finalFrac > con.displayFrac)
	{
		con.displayFrac += con_conspeed->value*cls.realFrametime*0.001;
		if (con.finalFrac < con.displayFrac)
			con.displayFrac = con.finalFrac;
	}

}


void Console::pageUp( ) {
	con.display -= 2;
	if ( con.current - con.display >= con.totallines ) {
		con.display = con.current - con.totallines + 1;
	}
}

void Console::pageDown( ) {
	con.display += 2;
	if (con.display > con.current) {
		con.display = con.current;
	}
}

void Console::top( ) {
	con.display = con.totallines;
	if ( con.current - con.display >= con.totallines ) {
		con.display = con.current - con.totallines + 1;
	}
}

void Console::bottom( ) {
	con.display = con.current;
}


void Console::close( ) {
	if ( !com_cl_running->integer ) {
		return;
	}
	g_consoleField.clear();
	Console::clearNotify ();
	Key_SetCatcher( Key_GetCatcher( ) & ~KEYCATCH_CONSOLE );
	con.finalFrac = 0;				// none visible
	con.displayFrac = 0;
}
