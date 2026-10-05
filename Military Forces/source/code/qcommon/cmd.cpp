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
// cmd.c -- Quake script command processing module

#include "../game/q_shared.h"
#include "qcommon.h"

constexpr int MAX_CMD_BUFFER = 128*1024;
constexpr int MAX_CMD_LINE = 1024;

struct cmd_t {
	byte	*data;
	int		maxsize;
	int		cursize;
};

int			cmd_wait;
cmd_t		cmd_text;
byte		cmd_text_buf[MAX_CMD_BUFFER];


//=============================================================================

/*
============
CommandSystem::wait_f

Causes execution of the remainder of the command buffer to be delayed until
next frame.  This allows commands like:
bind g "cmd use rocket ; +attack ; wait ; -attack ; cmd use blaster"
============
*/
void CommandSystem::wait_f( ) {
	if ( CommandSystem::argc() == 2 ) {
		cmd_wait = atoi( CommandSystem::argv( 1 ) );
		if ( cmd_wait < 0 )
			cmd_wait = 1; // ignore the argument
	} else {
		cmd_wait = 1;
	}
}


/*
=============================================================================

						COMMAND BUFFER

=============================================================================
*/

/*
============
CommandBuffer::init
============
*/
void CommandBuffer::init ()
{
	cmd_text.data = cmd_text_buf;
	cmd_text.maxsize = MAX_CMD_BUFFER;
	cmd_text.cursize = 0;
}

/*
============
CommandBuffer::addText

Adds command text at the end of the buffer, does NOT add a final \n
============
*/
void CommandBuffer::addText( const char *text ) {
	int		l;
	
	l = strlen (text);

	if (cmd_text.cursize + l >= cmd_text.maxsize)
	{
		Common::printf ("CommandBuffer::addText: overflow\n");
		return;
	}
	Com_Memcpy(&cmd_text.data[cmd_text.cursize], text, l);
	cmd_text.cursize += l;
}


/*
============
CommandBuffer::insertText

Adds command text immediately after the current command
Adds a \n to the text
============
*/
void CommandBuffer::insertText( const char *text ) {
	int		len;
	int		i;

	len = strlen( text ) + 1;
	if ( len + cmd_text.cursize > cmd_text.maxsize ) {
		Common::printf( "CommandBuffer::insertText overflowed\n" );
		return;
	}

	// move the existing command text
	for ( i = cmd_text.cursize - 1 ; i >= 0 ; i-- ) {
		cmd_text.data[ i + len ] = cmd_text.data[ i ];
	}

	// copy the new text in
	Com_Memcpy( cmd_text.data, text, len - 1 );

	// add a \n
	cmd_text.data[ len - 1 ] = '\n';

	cmd_text.cursize += len;
}


/*
============
CommandBuffer::executeText
============
*/
void CommandBuffer::executeText (int exec_when, const char *text)
{
	switch (exec_when)
	{
	case EXEC_NOW:
		if (text && strlen(text) > 0) {
			Common::dPrintf(S_COLOR_YELLOW "EXEC_NOW %s\n", text);
			CommandSystem::executeString (text);
		} else {
			CommandBuffer::execute();
			Common::dPrintf(S_COLOR_YELLOW "EXEC_NOW %s\n", cmd_text.data);
		}
		break;
	case EXEC_INSERT:
		CommandBuffer::insertText (text);
		break;
	case EXEC_APPEND:
		CommandBuffer::addText (text);
		break;
	default:
		Common::error (ERR_FATAL, "CommandBuffer::executeText: bad exec_when");
	}
}

/*
============
CommandBuffer::execute
============
*/
void CommandBuffer::execute ()
{
	int		i;
	char	*text;
	char	line[MAX_CMD_LINE];
	int		quotes;

	// This will keep // style comments all on one line by not breaking on
	// a semicolon.  It will keep /* ... */ style comments all on one line by not
	// breaking it for semicolon or newline.
	bool in_star_comment = false;
	bool in_slash_comment = false;
	while (cmd_text.cursize)
	{
		if ( cmd_wait > 0 ) {
			// skip out while text still remains in buffer, leaving it
			// for next frame
			cmd_wait--;
			break;
		}

		// find a \n or ; line break or comment: // or /* */
		text = reinterpret_cast<char *>(cmd_text.data);

		quotes = 0;
		for (i=0 ; i< cmd_text.cursize ; i++)
		{
			if (text[i] == '"')
				quotes++;

			if ( !(quotes&1)) {
				if (i < cmd_text.cursize - 1) {
					if (! in_star_comment && text[i] == '/' && text[i+1] == '/')
						in_slash_comment = true;
					else if (! in_slash_comment && text[i] == '/' && text[i+1] == '*')
						in_star_comment = true;
					else if (in_star_comment && text[i] == '*' && text[i+1] == '/') {
						in_star_comment = false;
						// If we are in a star comment, then the part after it is valid
						// Note: This will cause it to NUL out the terminating '/'
						// but ExecuteString doesn't require it anyway.
						i++;
						break;
					}
				}
				if (! in_slash_comment && ! in_star_comment && text[i] == ';')
					break;
			}
			if (! in_star_comment && (text[i] == '\n' || text[i] == '\r')) {
				in_slash_comment = false;
				break;
			}
		}

		if( i >= (MAX_CMD_LINE - 1)) {
			i = MAX_CMD_LINE - 1;
		}
				
		Com_Memcpy (line, text, i);
		line[i] = 0;
		
// delete the text from the command buffer and move remaining commands down
// this is necessary because commands (exec) can insert data at the
// beginning of the text buffer

		if (i == cmd_text.cursize)
			cmd_text.cursize = 0;
		else
		{
			i++;
			cmd_text.cursize -= i;
			memmove (text, text+i, cmd_text.cursize);
		}

// execute the command line

		CommandSystem::executeString (line);		
	}
}


/*
==============================================================================

						SCRIPT COMMANDS

==============================================================================
*/


/*
===============
CommandSystem::exec_f
===============
*/
void CommandSystem::exec_f( ) {
	bool quiet;
	union {
		char	*c;
		void	*v;
	} f;
	char	filename[MAX_QPATH];

	quiet = !Q_stricmp(CommandSystem::argv(0), "execq");

	if (CommandSystem::argc () != 2) {
		Common::printf ("exec%s <filename> : execute a script file%s\n",
		            quiet ? "q" : "", quiet ? " without notification" : "");
		return;
	}

	Q_strncpyz( filename, CommandSystem::argv(1), sizeof( filename ) );
	COM_DefaultExtension( filename, sizeof( filename ), ".cfg" );
	FileSystem::readFile( filename, &f.v);
	if (!f.c) {
		Common::printf ("couldn't exec %s\n", filename);
		return;
	}
	if (!quiet)
		Common::printf ("execing %s\n", filename);
	
	CommandBuffer::insertText (f.c);

	FileSystem::freeFile (f.v);
}


/*
===============
CommandSystem::vstr_f

Inserts the current value of a variable as command text
===============
*/
void CommandSystem::vstr_f( ) {
	char	*v;

	if (CommandSystem::argc () != 2) {
		Common::printf ("vstr <variablename> : execute a variable command\n");
		return;
	}

	v = CvarSystem::variableString( CommandSystem::argv( 1 ) );
	CommandBuffer::insertText( va("%s\n", v ) );
}


/*
===============
CommandSystem::echo_f

Just prints the rest of the line to the console
===============
*/
void CommandSystem::echo_f ()
{
	Common::printf ("%s\n", CommandSystem::args());
}


/*
=============================================================================

					COMMAND EXECUTION

=============================================================================
*/

struct cmd_function_s
{
	struct cmd_function_s	*next;
	char					*name;
	xcommand_t				function;
	completionFunc_t	complete;
};
using cmd_function_t = cmd_function_s;


int CommandSystem::cmd_argc;
char *CommandSystem::cmd_argv[MAX_STRING_TOKENS];		// points into cmd_tokenized
char CommandSystem::cmd_tokenized[BIG_INFO_STRING+MAX_STRING_TOKENS];	// will have 0 bytes inserted
char CommandSystem::cmd_cmd[BIG_INFO_STRING]; // the original command we received (no token processing)

static	cmd_function_t	*cmd_functions;		// possible commands to execute

/*
============
CommandSystem::argc
============
*/
int		CommandSystem::argc( ) {
	return cmd_argc;
}

/*
============
CommandSystem::argv
============
*/
char	*CommandSystem::argv( int arg ) {
	if ( static_cast<unsigned>(arg) >= cmd_argc ) {
		return "";
	}
	return cmd_argv[arg];	
}

/*
============
CommandSystem::argvBuffer

The interpreted versions use this because
they can't have pointers returned to them
============
*/
void	CommandSystem::argvBuffer( int arg, char *buffer, int bufferLength ) {
	Q_strncpyz( buffer, CommandSystem::argv( arg ), bufferLength );
}


/*
============
CommandSystem::args

Returns a single string containing argv(1) to argv(argc()-1)
============
*/
char	*CommandSystem::args( ) {
	static	char		cmd_args[MAX_STRING_CHARS];
	int		i;

	cmd_args[0] = 0;
	for ( i = 1 ; i < cmd_argc ; i++ ) {
		strcat( cmd_args, cmd_argv[i] );
		if ( i != cmd_argc-1 ) {
			strcat( cmd_args, " " );
		}
	}

	return cmd_args;
}

/*
============
CommandSystem::args

Returns a single string containing argv(arg) to argv(argc()-1)
============
*/
char *CommandSystem::argsFrom( int arg ) {
	static	char		cmd_args[BIG_INFO_STRING];
	int		i;

	cmd_args[0] = 0;
	if (arg < 0)
		arg = 0;
	for ( i = arg ; i < cmd_argc ; i++ ) {
		strcat( cmd_args, cmd_argv[i] );
		if ( i != cmd_argc-1 ) {
			strcat( cmd_args, " " );
		}
	}

	return cmd_args;
}

/*
============
CommandSystem::argsBuffer

The interpreted versions use this because
they can't have pointers returned to them
============
*/
void	CommandSystem::argsBuffer( char *buffer, int bufferLength ) {
	Q_strncpyz( buffer, CommandSystem::args(), bufferLength );
}

/*
============
CommandSystem::cmd

Retrieve the unmodified command string
For rcon use when you want to transmit without altering quoting
https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=543
============
*/
char *CommandSystem::cmd()
{
	return cmd_cmd;
}

/*
   Replace command separators with space to prevent interpretation
   This is a hack to protect buggy qvms
   https://bugzilla.icculus.org/show_bug.cgi?id=3593
   https://bugzilla.icculus.org/show_bug.cgi?id=4769
*/

void CommandSystem::args_Sanitize()
{
	int i;

	for(i = 1; i < cmd_argc; i++)
	{
		char *c = cmd_argv[i];
		
		if(strlen(c) > MAX_CVAR_VALUE_STRING - 1)
			c[MAX_CVAR_VALUE_STRING - 1] = '\0';
		
		while ((c = strpbrk(c, "\n\r;"))) {
			*c = ' ';
			++c;
		}
	}
}

/*
============
CommandSystem::tokenizeString

Parses the given string into command line tokens.
The text is copied to a separate buffer and 0 characters
are inserted in the appropriate place, The argv array
will point into this temporary buffer.
============
*/
// NOTE TTimo define that to track tokenization issues
//#define TKN_DBG
void CommandSystem::tokenizeString2( const char *text_in, bool ignoreQuotes ) {
	const char	*text;
	char	*textOut;

#ifdef TKN_DBG
  // FIXME TTimo blunt hook to try to find the tokenization of userinfo
  Common::dPrintf("CommandSystem::tokenizeString: %s\n", text_in);
#endif

	// clear previous args
	cmd_argc = 0;

	if ( !text_in ) {
		return;
	}
	
	Q_strncpyz( cmd_cmd, text_in, sizeof(cmd_cmd) );

	text = text_in;
	textOut = cmd_tokenized;

	while ( true ) {
		if ( cmd_argc == MAX_STRING_TOKENS ) {
			return;			// this is usually something malicious
		}

		while ( true ) {
			// skip whitespace
			while ( *text && *text <= ' ' ) {
				text++;
			}
			if ( !*text ) {
				return;			// all tokens parsed
			}

			// skip // comments
			if ( text[0] == '/' && text[1] == '/' ) {
				return;			// all tokens parsed
			}

			// skip /* */ comments
			if ( text[0] == '/' && text[1] =='*' ) {
				while ( *text && ( text[0] != '*' || text[1] != '/' ) ) {
					text++;
				}
				if ( !*text ) {
					return;		// all tokens parsed
				}
				text += 2;
			} else {
				break;			// we are ready to parse a token
			}
		}

		// handle quoted strings
    // NOTE TTimo this doesn't handle \" escaping
		if ( !ignoreQuotes && *text == '"' ) {
			cmd_argv[cmd_argc] = textOut;
			cmd_argc++;
			text++;
			while ( *text && *text != '"' ) {
				*textOut++ = *text++;
			}
			*textOut++ = 0;
			if ( !*text ) {
				return;		// all tokens parsed
			}
			text++;
			continue;
		}

		// regular token
		cmd_argv[cmd_argc] = textOut;
		cmd_argc++;

		// skip until whitespace, quote, or command
		while ( *text > ' ' ) {
			if ( !ignoreQuotes && text[0] == '"' ) {
				break;
			}

			if ( text[0] == '/' && text[1] == '/' ) {
				break;
			}

			// skip /* */ comments
			if ( text[0] == '/' && text[1] =='*' ) {
				break;
			}

			*textOut++ = *text++;
		}

		*textOut++ = 0;

		if ( !*text ) {
			return;		// all tokens parsed
		}
	}
	
}

/*
============
CommandSystem::tokenizeString
============
*/
void CommandSystem::tokenizeString( const char *text_in ) {
	tokenizeString2( text_in, false );
}

/*
============
CommandSystem::tokenizeStringIgnoreQuotes
============
*/
void CommandSystem::tokenizeStringIgnoreQuotes( const char *text_in ) {
	tokenizeString2( text_in, true );
}

/*
============
CommandSystem::findCommand
============
*/
cmd_function_t *CommandSystem::findCommand( const char *cmd_name )
{
	cmd_function_t *cmd;
	for( cmd = cmd_functions; cmd; cmd = cmd->next )
		if( !Q_stricmp( cmd_name, cmd->name ) )
			return cmd;
	return nullptr;
}

/*
============
CommandSystem::addCommand
============
*/
void	CommandSystem::addCommand( const char *cmd_name, xcommand_t function ) {
	cmd_function_t	*cmd;
	
	// fail if the command already exists
	if( CommandSystem::findCommand( cmd_name ) )
	{
		// allow completion-only commands to be silently doubled
		if( function != nullptr )
			Common::printf( "CommandSystem::addCommand: %s already defined\n", cmd_name );
		return;
	}

	// use a small malloc to avoid zone fragmentation
	cmd = static_cast<cmd_function_t *>(Zone::smallMalloc(sizeof(cmd_function_t)));
	cmd->name = CopyString( cmd_name );
	cmd->function = function;
	cmd->complete = nullptr;
	cmd->next = cmd_functions;
	cmd_functions = cmd;
}

/*
============
CommandSystem::setCommandCompletionFunc
============
*/
void CommandSystem::setCommandCompletionFunc( const char *command, completionFunc_t complete ) {
	cmd_function_t	*cmd;

	for( cmd = cmd_functions; cmd; cmd = cmd->next ) {
		if( !Q_stricmp( command, cmd->name ) ) {
			cmd->complete = complete;
			return;
		}
	}
}

/*
============
CommandSystem::removeCommand
============
*/
void	CommandSystem::removeCommand( const char *cmd_name ) {
	cmd_function_t	*cmd, **back;

	back = &cmd_functions;
	while( true ) {
		cmd = *back;
		if ( !cmd ) {
			// command wasn't active
			return;
		}
		if ( !strcmp( cmd_name, cmd->name ) ) {
			*back = cmd->next;
			Zone::free (cmd->name);
			Zone::free (cmd);
			return;
		}
		back = &cmd->next;
	}
}

/*
============
CommandSystem::removeCommandSafe

Only remove commands with no associated function
============
*/
void CommandSystem::removeCommandSafe( const char *cmd_name )
{
	cmd_function_t *cmd = CommandSystem::findCommand( cmd_name );

	if( !cmd )
		return;
	if( cmd->function )
	{
		Common::error( ERR_DROP, "Restricted source tried to remove "
			"system command \"%s\"", cmd_name );
		return;
	}

	CommandSystem::removeCommand( cmd_name );
}

/*
============
CommandSystem::commandCompletion
============
*/
void	CommandSystem::commandCompletion( void(*callback)(const char *s) ) {
	cmd_function_t	*cmd;
	
	for (cmd=cmd_functions ; cmd ; cmd=cmd->next) {
		callback( cmd->name );
	}
}

/*
============
CommandSystem::completeArgument
============
*/
void CommandSystem::completeArgument( const char *command, char *args, int argNum ) {
	cmd_function_t	*cmd;

	for( cmd = cmd_functions; cmd; cmd = cmd->next ) {
		if( !Q_stricmp( command, cmd->name ) ) {
			if ( cmd->complete ) {
				cmd->complete( args, argNum );
			}
			return;
		}
	}
}


/*
============
CommandSystem::executeString

A complete command line has been parsed, so try to execute it
============
*/
void	CommandSystem::executeString( const char *text ) {	
	cmd_function_t	*cmd, **prev;

	// execute the command line
	CommandSystem::tokenizeString( text );		
	if ( !CommandSystem::argc() ) {
		return;		// no tokens
	}

	// check registered command functions	
	for ( prev = &cmd_functions ; *prev ; prev = &cmd->next ) {
		cmd = *prev;
		if ( !Q_stricmp( cmd_argv[0],cmd->name ) ) {
			// rearrange the links so that the command will be
			// near the head of the list next time it is used
			*prev = cmd->next;
			cmd->next = cmd_functions;
			cmd_functions = cmd;

			// perform the action
			if ( !cmd->function ) {
				// let the cgame or game handle it
				break;
			} else {
				cmd->function ();
			}
			return;
		}
	}
	
	// check cvars
	if ( CvarSystem::command() ) {
		return;
	}

	// check client game commands
	if ( com_cl_running && com_cl_running->integer && ClientLoop::gameCommand() ) {
		return;
	}

	// check server game commands
	if ( com_sv_running && com_sv_running->integer && ServerLoop::gameCommand() ) {
		return;
	}

	// check ui commands
	if ( com_cl_running && com_cl_running->integer && UI_GameCommand() ) {
		return;
	}

	// send it as a server command if we are connected
	// this will usually result in a chat message
	ClientLoop::forwardCommandToServer ( text );
}

/*
============
CommandSystem::list_f
============
*/
void CommandSystem::list_f ()
{
	cmd_function_t	*cmd;
	int				i;
	char			*match;

	if ( CommandSystem::argc() > 1 ) {
		match = CommandSystem::argv( 1 );
	} else {
		match = nullptr;
	}

	i = 0;
	for (cmd=cmd_functions ; cmd ; cmd=cmd->next) {
		if (match && !Common::filter(match, cmd->name, false)) continue;

		Common::printf ("%s\n", cmd->name);
		i++;
	}
	Common::printf ("%i commands\n", i);
}

/*
==================
CommandSystem::completeCfgName
==================
*/
void CommandSystem::completeCfgName( char *args, int argNum ) {
	if( argNum == 2 ) {
		Field_CompleteFilename( "", "cfg", nullptr, false, true );
	}
}

/*
============
CommandSystem::init
============
*/
void CommandSystem::init () {
	CommandSystem::addCommand ("cmdlist",CommandSystem::list_f);
	CommandSystem::addCommand ("exec",CommandSystem::exec_f);
	CommandSystem::addCommand ("execq",CommandSystem::exec_f);
	CommandSystem::setCommandCompletionFunc( "exec", CommandSystem::completeCfgName );
	CommandSystem::setCommandCompletionFunc( "execq", CommandSystem::completeCfgName );
	CommandSystem::addCommand ("vstr",CommandSystem::vstr_f);
	CommandSystem::setCommandCompletionFunc( "vstr", CvarSystem::completeCvarName );
	CommandSystem::addCommand ("echo",CommandSystem::echo_f);
	CommandSystem::addCommand ("wait", CommandSystem::wait_f);
}

