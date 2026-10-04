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
// cvar.c -- dynamic variable tracking

#include "../game/q_shared.h"
#include "qcommon.h"

cvar_t		*cvar_vars = nullptr;
cvar_t		*cvar_cheats;
int			cvar_modifiedFlags;

constexpr int MAX_CVARS = 2048;
cvar_t		cvar_indexes[MAX_CVARS];
int			cvar_numIndexes;

#define FILE_HASH_SIZE		256
static	cvar_t	*hashTable[FILE_HASH_SIZE];

/*
================
return a hash value for the filename
================
*/
static long generateHashValue( const char *fname ) {
	int		i;
	long	hash;
	char	letter;

	hash = 0;
	i = 0;
	while (fname[i] != '\0') {
		letter = tolower(fname[i]);
		hash+=static_cast<long>(letter)*(i+119);
		i++;
	}
	hash &= (FILE_HASH_SIZE-1);
	return hash;
}

/*
============
Cvar_ValidateString
============
*/
static bool Cvar_ValidateString( const char *s ) {
	if ( !s ) {
		return false;
	}
	if ( strchr( s, '\\' ) ) {
		return false;
	}
	if ( strchr( s, '\"' ) ) {
		return false;
	}
	if ( strchr( s, ';' ) ) {
		return false;
	}
	return true;
}

/*
============
Cvar_FindVar
============
*/
static cvar_t *Cvar_FindVar( const char *var_name ) {
	cvar_t	*var;
	long hash;

	hash = generateHashValue(var_name);
	
	for (var=hashTable[hash] ; var ; var=var->hashNext) {
		if (!Q_stricmp(var_name, var->name)) {
			return var;
		}
	}

	return nullptr;
}

/*
============
CvarSystem::variableValue
============
*/
float CvarSystem::variableValue( const char *var_name ) {
	cvar_t	*var;
	
	var = Cvar_FindVar (var_name);
	if (!var)
		return 0;
	return var->value;
}


/*
============
CvarSystem::variableIntegerValue
============
*/
int CvarSystem::variableIntegerValue( const char *var_name ) {
	cvar_t	*var;
	
	var = Cvar_FindVar (var_name);
	if (!var)
		return 0;
	return var->integer;
}


/*
============
CvarSystem::variableString
============
*/
char *CvarSystem::variableString( const char *var_name ) {
	cvar_t *var;
	
	var = Cvar_FindVar (var_name);
	if (!var)
		return "";
	return var->string;
}


/*
============
CvarSystem::variableStringBuffer
============
*/
void CvarSystem::variableStringBuffer( const char *var_name, char *buffer, int bufsize ) {
	cvar_t *var;
	
	var = Cvar_FindVar (var_name);
	if (!var) {
		*buffer = 0;
	}
	else {
		Q_strncpyz( buffer, var->string, bufsize );
	}
}

/*
============
CvarSystem::flags
============
*/
int CvarSystem::flags(const char *var_name)
{
	cvar_t *var;
	
	if(!(var = Cvar_FindVar(var_name)))
		return CVAR_NONEXISTENT;
	else
	{
		if(var->modified)
			return var->flags | CVAR_MODIFIED;
		else
			return var->flags;
	}
}

/*
============
CvarSystem::commandCompletion
============
*/
void CvarSystem::commandCompletion(void (*callback)(const char *s))
{
	cvar_t		*cvar;
	
	for(cvar = cvar_vars; cvar; cvar = cvar->next)
	{
		if(cvar->name)
			callback(cvar->name);
	}
}

/*
============
Cvar_Validate
============
*/
static const char *Cvar_Validate( cvar_t *var,
    const char *value, bool warn )
{
	static char s[ MAX_CVAR_VALUE_STRING ];
	float valuef;
	bool changed = false;

	if( !var->validate )
		return value;

	if( !value )
		return value;

	if( Q_isanumber( value ) )
	{
		valuef = atof( value );

		if( var->integral )
		{
			if( !Q_isintegral( valuef ) )
			{
				if( warn )
					Com_Printf( "WARNING: cvar '%s' must be integral", var->name );

				valuef = static_cast<int>(valuef);
				changed = true;
			}
		}
	}
	else
	{
		if( warn )
			Com_Printf( "WARNING: cvar '%s' must be numeric", var->name );

		valuef = atof( var->resetString );
		changed = true;
	}

	if( valuef < var->min )
	{
		if( warn )
		{
			if( changed )
				Com_Printf( " and is" );
			else
				Com_Printf( "WARNING: cvar '%s'", var->name );

			if( Q_isintegral( var->min ) )
				Com_Printf( " out of range (min %d)", static_cast<int>(var->min) );
			else
				Com_Printf( " out of range (min %f)", var->min );
		}

		valuef = var->min;
		changed = true;
	}
	else if( valuef > var->max )
	{
		if( warn )
		{
			if( changed )
				Com_Printf( " and is" );
			else
				Com_Printf( "WARNING: cvar '%s'", var->name );

			if( Q_isintegral( var->max ) )
				Com_Printf( " out of range (max %d)", static_cast<int>(var->max) );
			else
				Com_Printf( " out of range (max %f)", var->max );
		}

		valuef = var->max;
		changed = true;
	}

	if( changed )
	{
		if( Q_isintegral( valuef ) )
		{
			Com_sprintf( s, sizeof( s ), "%d", static_cast<int>(valuef) );

			if( warn )
				Com_Printf( ", setting to %d\n", static_cast<int>(valuef) );
		}
		else
		{
			Com_sprintf( s, sizeof( s ), "%f", valuef );

			if( warn )
				Com_Printf( ", setting to %f\n", valuef );
		}

		return s;
	}
	else
		return value;
}


/*
============
CvarSystem::get

If the variable already exists, the value will not be set unless CVAR_ROM
The flags will be or'ed in if the variable exists.
============
*/
cvar_t *CvarSystem::get( const char *var_name, const char *var_value, int flags ) {
	cvar_t	*var;
	long	hash;
	int	index;

	if ( !var_name || ! var_value ) {
		Com_Error( ERR_FATAL, "CvarSystem::get: NULL parameter" );
	}

	if ( !Cvar_ValidateString( var_name ) ) {
		Com_Printf("invalid cvar name string: %s\n", var_name );
		var_name = "BADNAME";
	}

#if 0		// FIXME: values with backslash happen
	if ( !Cvar_ValidateString( var_value ) ) {
		Com_Printf("invalid cvar value string: %s\n", var_value );
		var_value = "BADVALUE";
	}
#endif

	var = Cvar_FindVar (var_name);
	
	if(var)
	{
		var_value = Cvar_Validate(var, var_value, false);

		// Make sure the game code cannot mark engine-added variables as gamecode vars
		if(var->flags & CVAR_VM_CREATED)
		{
			if(!(flags & CVAR_VM_CREATED))
				var->flags &= ~CVAR_VM_CREATED;
		}
		else if (!(var->flags & CVAR_USER_CREATED))
		{
			if(flags & CVAR_VM_CREATED)
				flags &= ~CVAR_VM_CREATED;
		}

		// if the C code is now specifying a variable that the user already
		// set a value for, take the new value as the reset value
		if(var->flags & CVAR_USER_CREATED)
		{
			var->flags &= ~CVAR_USER_CREATED;
			Z_Free( var->resetString );
			var->resetString = CopyString( var_value );

			if(flags & CVAR_ROM)
			{
				// this variable was set by the user,
				// so force it to value given by the engine.

				if(var->latchedString)
					Z_Free(var->latchedString);
				
				var->latchedString = CopyString(var_value);
			}
		}
		
		// Make sure servers cannot mark engine-added variables as SERVER_CREATED
		if(var->flags & CVAR_SERVER_CREATED)
		{
			if(!(flags & CVAR_SERVER_CREATED))
				var->flags &= ~CVAR_SERVER_CREATED;
		}
		else
		{
			if(flags & CVAR_SERVER_CREATED)
				flags &= ~CVAR_SERVER_CREATED;
		}
		
		var->flags |= flags;

		// only allow one non-empty reset string without a warning
		if ( !var->resetString[0] ) {
			// we don't have a reset string yet
			Z_Free( var->resetString );
			var->resetString = CopyString( var_value );
		} else if ( var_value[0] && strcmp( var->resetString, var_value ) ) {
			Com_DPrintf( "Warning: cvar \"%s\" given initial values: \"%s\" and \"%s\"\n",
				var_name, var->resetString, var_value );
		}
		// if we have a latched string, take that value now
		if ( var->latchedString ) {
			char *s;

			s = var->latchedString;
			var->latchedString = nullptr;	// otherwise cvar_set2 would free it
			CvarSystem::set2( var_name, s, true );
			Z_Free( s );
		}

		// ZOID--needs to be set so that cvars the game sets as 
		// SERVERINFO get sent to clients
		cvar_modifiedFlags |= flags;

		return var;
	}

	//
	// allocate a new cvar
	//

	// find a free cvar
	for(index = 0; index < MAX_CVARS; index++)
	{
		if(!cvar_indexes[index].name)
			break;
	}

	if(index >= MAX_CVARS)
	{
		if(!com_errorEntered)
			Com_Error(ERR_FATAL, "Error: Too many cvars, cannot create a new one!");

		return nullptr;
	}
	
	var = &cvar_indexes[index];
	
	if(index >= cvar_numIndexes)
		cvar_numIndexes = index + 1;
		
	var->name = CopyString (var_name);
	var->string = CopyString (var_value);
	var->modified = true;
	var->modificationCount = 1;
	var->value = atof (var->string);
	var->integer = atoi(var->string);
	var->resetString = CopyString( var_value );
	var->validate = false;
	var->description = nullptr;

	// link the variable in
	var->next = cvar_vars;
	if(cvar_vars)
		cvar_vars->prev = var;

	var->prev = nullptr;
	cvar_vars = var;

	var->flags = flags;
	// note what types of cvars have been modified (userinfo, archive, serverinfo, systeminfo)
	cvar_modifiedFlags |= var->flags;

	hash = generateHashValue(var_name);
	var->hashIndex = hash;

	var->hashNext = hashTable[hash];
	if(hashTable[hash])
		hashTable[hash]->hashPrev = var;

	var->hashPrev = nullptr;
	hashTable[hash] = var;

	return var;
}

/*
============
CvarSystem::print

Prints the value, default, and latched string of the given variable
============
*/
void CvarSystem::print( cvar_t *v ) {
	Com_Printf ("\"%s\" is:\"%s" S_COLOR_WHITE "\"",
			v->name, v->string );

	if ( !( v->flags & CVAR_ROM ) ) {
		if ( !Q_stricmp( v->string, v->resetString ) ) {
			Com_Printf (", the default" );
		} else {
			Com_Printf (" default:\"%s" S_COLOR_WHITE "\"",
					v->resetString );
		}
	}

	Com_Printf ("\n");

	if ( v->latchedString ) {
		Com_Printf( "latched: \"%s\"\n", v->latchedString );
	}

	if ( v->description ) {
		Com_Printf( "%s\n", v->description );
	}
}

/*
============
CvarSystem::set2
============
*/
cvar_t *CvarSystem::set2( const char *var_name, const char *value, bool force ) {
	cvar_t	*var;

//	Com_DPrintf( "CvarSystem::set2: %s %s\n", var_name, value );

	if ( !Cvar_ValidateString( var_name ) ) {
		Com_Printf("invalid cvar name string: %s\n", var_name );
		var_name = "BADNAME";
	}

#if 0	// FIXME
	if ( value && !Cvar_ValidateString( value ) ) {
		Com_Printf("invalid cvar value string: %s\n", value );
		var_value = "BADVALUE";
	}
#endif

	var = Cvar_FindVar (var_name);
	if (!var) {
		if ( !value ) {
			return nullptr;
		}
		// create it
		if ( !force ) {
			return CvarSystem::get( var_name, value, CVAR_USER_CREATED );
		} else {
			return CvarSystem::get (var_name, value, 0);
		}
	}

	if (!value ) {
		value = var->resetString;
	}

	value = Cvar_Validate(var, value, true);

	if((var->flags & CVAR_LATCH) && var->latchedString)
	{
		if(!strcmp(value, var->string))
		{
			Z_Free(var->latchedString);
			var->latchedString = nullptr;
			return var;
		}

		if(!strcmp(value, var->latchedString))
			return var;
	}
	else if(!strcmp(value, var->string))
		return var;

	// note what types of cvars have been modified (userinfo, archive, serverinfo, systeminfo)
	cvar_modifiedFlags |= var->flags;

	if (!force)
	{
		if (var->flags & CVAR_ROM)
		{
			Com_Printf ("%s is read only.\n", var_name);
			return var;
		}

		if (var->flags & CVAR_INIT)
		{
			Com_Printf ("%s is write protected.\n", var_name);
			return var;
		}

		if ((var->flags & CVAR_CHEAT) && !cvar_cheats->integer)
		{
			Com_Printf ("%s is cheat protected.\n", var_name);
			return var;
		}
		
		if (var->flags & CVAR_LATCH)
		{
			if (var->latchedString)
			{
				if (strcmp(value, var->latchedString) == 0)
					return var;
				Z_Free (var->latchedString);
			}
			else
			{
				if (strcmp(value, var->string) == 0)
					return var;
			}

			Com_Printf ("%s will be changed upon restarting.\n", var_name);
			var->latchedString = CopyString(value);
			var->modified = true;
			var->modificationCount++;
			return var;
		}
	}
	else
	{
		if (var->latchedString)
		{
			Z_Free (var->latchedString);
			var->latchedString = nullptr;
		}
	}

	if (!strcmp(value, var->string))
		return var;		// not changed

	var->modified = true;
	var->modificationCount++;
	
	Z_Free (var->string);	// free the old value string
	
	var->string = CopyString(value);
	var->value = atof (var->string);
	var->integer = atoi (var->string);

	return var;
}

/*
============
CvarSystem::set
============
*/
void CvarSystem::set( const char *var_name, const char *value) {
	CvarSystem::set2 (var_name, value, true);
}

/*
============
CvarSystem::setSafe
============
*/
void CvarSystem::setSafe( const char *var_name, const char *value )
{
	int flags = CvarSystem::flags( var_name );

	if((flags != CVAR_NONEXISTENT) && (flags & CVAR_PROTECTED))
	{
		if( value )
			Com_Error( ERR_DROP, "Restricted source tried to set "
				"\"%s\" to \"%s\"", var_name, value );
		else
			Com_Error( ERR_DROP, "Restricted source tried to "
				"modify \"%s\"", var_name );
		return;
	}
	CvarSystem::set( var_name, value );
}

/*
============
CvarSystem::setLatched
============
*/
void CvarSystem::setLatched( const char *var_name, const char *value) {
	CvarSystem::set2 (var_name, value, false);
}

/*
============
CvarSystem::setValue
============
*/
void CvarSystem::setValue( const char *var_name, float value) {
	char	val[32];

	if ( value == static_cast<int>(value) ) {
		Com_sprintf (val, sizeof(val), "%i",static_cast<int>(value));
	} else {
		Com_sprintf (val, sizeof(val), "%f",value);
	}
	CvarSystem::set (var_name, val);
}

/*
============
CvarSystem::setValueSafe
============
*/
void CvarSystem::setValueSafe( const char *var_name, float value )
{
	char val[32];

	if( Q_isintegral( value ) )
		Com_sprintf( val, sizeof(val), "%i", static_cast<int>(value) );
	else
		Com_sprintf( val, sizeof(val), "%f", value );
	CvarSystem::setSafe( var_name, val );
}

/*
============
CvarSystem::reset
============
*/
void CvarSystem::reset( const char *var_name ) {
	CvarSystem::set2( var_name, nullptr, false );
}

/*
============
CvarSystem::forceReset
============
*/
void CvarSystem::forceReset(const char *var_name)
{
	CvarSystem::set2(var_name, nullptr, true);
}

/*
============
CvarSystem::setCheatState

Any testing variables will be reset to the safe values
============
*/
void CvarSystem::setCheatState()
{
	cvar_t	*var;

	// set all default vars to the safe value
	for(var = cvar_vars; var ; var = var->next)
	{
		if(var->flags & CVAR_CHEAT)
		{
			// the CVAR_LATCHED|CVAR_CHEAT vars might escape the reset here 
			// because of a different var->latchedString
			if (var->latchedString)
			{
				Z_Free(var->latchedString);
				var->latchedString = nullptr;
			}
			if (strcmp(var->resetString,var->string))
				CvarSystem::set(var->name, var->resetString);
		}
	}
}

/*
============
CvarSystem::command

Handles variable inspection and changing from the console
============
*/
bool CvarSystem::command( ) {
	cvar_t	*v;

	// check variables
	v = Cvar_FindVar (CommandSystem::argv(0));
	if (!v) {
		return false;
	}

	// perform a variable print or set
	if ( CommandSystem::argc() == 1 ) {
		CvarSystem::print( v );
		return true;
	}

	// set the value if forcing isn't required
	CvarSystem::set2 (v->name, CommandSystem::args(), false);
	return true;
}


/*
============
CvarSystem::print_f

Prints the contents of a cvar 
(preferred over CvarSystem::command where cvar names and commands conflict)
============
*/
void CvarSystem::print_f()
{
	char *name;
	cvar_t *cv;
	
	if(CommandSystem::argc() != 2)
	{
		Com_Printf ("usage: print <variable>\n");
		return;
	}

	name = CommandSystem::argv(1);

	cv = Cvar_FindVar(name);
	
	if(cv)
		CvarSystem::print(cv);
	else
		Com_Printf ("Cvar %s does not exist.\n", name);
}

/*
============
CvarSystem::toggle_f

Toggles a cvar for easy single key binding, optionally through a list of
given values
============
*/
void CvarSystem::toggle_f( ) {
	int		i, c = CommandSystem::argc();
	char		*curval;

	if(c < 2) {
		Com_Printf("usage: toggle <variable> [value1, value2, ...]\n");
		return;
	}

	if(c == 2) {
		CvarSystem::set2(CommandSystem::argv(1), va("%d", 
			!CvarSystem::variableValue(CommandSystem::argv(1))), 
			false);
		return;
	}

	if(c == 3) {
		Com_Printf("toggle: nothing to toggle to\n");
		return;
	}

	curval = CvarSystem::variableString(CommandSystem::argv(1));

	// don't bother checking the last arg for a match since the desired
	// behaviour is the same as no match (set to the first argument)
	for(i = 2; i + 1 < c; i++) {
		if(strcmp(curval, CommandSystem::argv(i)) == 0) {
			CvarSystem::set2(CommandSystem::argv(1), CommandSystem::argv(i + 1), false);
			return;
		}
	}

	// fallback
	CvarSystem::set2(CommandSystem::argv(1), CommandSystem::argv(2), false);
}

/*
============
CvarSystem::set_f

Allows setting and defining of arbitrary cvars from console, even if they
weren't declared in C code.
============
*/
void CvarSystem::set_f( ) {
	int		c;
	char	*cmd;
	cvar_t	*v;

	c = CommandSystem::argc();
	cmd = CommandSystem::argv(0);

	if ( c < 2 ) {
		Com_Printf ("usage: %s <variable> <value>\n", cmd);
		return;
	}
	if ( c == 2 ) {
		CvarSystem::print_f();
		return;
	}

	v = CvarSystem::set2 (CommandSystem::argv(1), CommandSystem::argsFrom(2), false);
	if( !v ) {
		return;
	}
	switch( cmd[3] ) {
		case 'a':
			if( !( v->flags & CVAR_ARCHIVE ) ) {
				v->flags |= CVAR_ARCHIVE;
				cvar_modifiedFlags |= CVAR_ARCHIVE;
			}
			break;
		case 'u':
			if( !( v->flags & CVAR_USERINFO ) ) {
				v->flags |= CVAR_USERINFO;
				cvar_modifiedFlags |= CVAR_USERINFO;
			}
			break;
		case 's':
			if( !( v->flags & CVAR_SERVERINFO ) ) {
				v->flags |= CVAR_SERVERINFO;
				cvar_modifiedFlags |= CVAR_SERVERINFO;
			}
			break;
	}
}

/*
============
CvarSystem::reset_f
============
*/
void CvarSystem::reset_f( ) {
	if ( CommandSystem::argc() != 2 ) {
		Com_Printf ("usage: reset <variable>\n");
		return;
	}
	CvarSystem::reset( CommandSystem::argv( 1 ) );
}

/*
============
CvarSystem::writeVariables

Appends lines containing "set variable value" for all variables
with the archive flag set to true.
============
*/
void CvarSystem::writeVariables(fileHandle_t f)
{
	cvar_t	*var;
	char	buffer[1024];

	for (var = cvar_vars; var; var = var->next)
	{
		if(!var->name || Q_stricmp( var->name, "cl_cdkey" ) == 0)
			continue;

		if( var->flags & CVAR_ARCHIVE ) {
			// write the latched value, even if it hasn't taken effect yet
			if ( var->latchedString ) {
				if( strlen( var->name ) + strlen( var->latchedString ) + 10 > sizeof( buffer ) ) {
					Com_Printf( S_COLOR_YELLOW "WARNING: value of variable "
							"\"%s\" too long to write to file\n", var->name );
					continue;
				}
				Com_sprintf (buffer, sizeof(buffer), "seta %s \"%s\"\n", var->name, var->latchedString);
			} else {
				if( strlen( var->name ) + strlen( var->string ) + 10 > sizeof( buffer ) ) {
					Com_Printf( S_COLOR_YELLOW "WARNING: value of variable "
							"\"%s\" too long to write to file\n", var->name );
					continue;
				}
				Com_sprintf (buffer, sizeof(buffer), "seta %s \"%s\"\n", var->name, var->string);
			}
			FileSystem::write( buffer, strlen( buffer ), f );
		}
	}
}

/*
============
CvarSystem::list_f
============
*/
void CvarSystem::list_f( ) {
	cvar_t	*var;
	int		i;
	char	*match;

	if ( CommandSystem::argc() > 1 ) {
		match = CommandSystem::argv( 1 );
	} else {
		match = nullptr;
	}

	i = 0;
	for (var = cvar_vars ; var ; var = var->next, i++)
	{
		if(!var->name || (match && !Com_Filter(match, var->name, false)))
			continue;

		if (var->flags & CVAR_SERVERINFO) {
			Com_Printf("S");
		} else {
			Com_Printf(" ");
		}
		if (var->flags & CVAR_SYSTEMINFO) {
			Com_Printf("s");
		} else {
			Com_Printf(" ");
		}
		if (var->flags & CVAR_USERINFO) {
			Com_Printf("U");
		} else {
			Com_Printf(" ");
		}
		if (var->flags & CVAR_ROM) {
			Com_Printf("R");
		} else {
			Com_Printf(" ");
		}
		if (var->flags & CVAR_INIT) {
			Com_Printf("I");
		} else {
			Com_Printf(" ");
		}
		if (var->flags & CVAR_ARCHIVE) {
			Com_Printf("A");
		} else {
			Com_Printf(" ");
		}
		if (var->flags & CVAR_LATCH) {
			Com_Printf("L");
		} else {
			Com_Printf(" ");
		}
		if (var->flags & CVAR_CHEAT) {
			Com_Printf("C");
		} else {
			Com_Printf(" ");
		}
		if (var->flags & CVAR_USER_CREATED) {
			Com_Printf("?");
		} else {
			Com_Printf(" ");
		}

		Com_Printf (" %s \"%s\"\n", var->name, var->string);
	}

	Com_Printf ("\n%i total cvars\n", i);
	Com_Printf ("%i cvar indexes\n", cvar_numIndexes);
}

/*
============
CvarSystem::listModified_f
============
*/
void CvarSystem::listModified_f( ) {
	cvar_t	*var;
	int		totalModified;
	char	*value;
	char	*match;

	if ( CommandSystem::argc() > 1 ) {
		match = CommandSystem::argv( 1 );
	} else {
		match = nullptr;
	}

	totalModified = 0;
	for (var = cvar_vars ; var ; var = var->next)
	{
		if ( !var->name || !var->modificationCount )
			continue;

		value = var->latchedString ? var->latchedString : var->string;
		if ( !strcmp( value, var->resetString ) )
			continue;

		totalModified++;

		if (match && !Com_Filter(match, var->name, false))
			continue;

		if (var->flags & CVAR_SERVERINFO) {
			Com_Printf("S");
		} else {
			Com_Printf(" ");
		}
		if (var->flags & CVAR_SYSTEMINFO) {
			Com_Printf("s");
		} else {
			Com_Printf(" ");
		}
		if (var->flags & CVAR_USERINFO) {
			Com_Printf("U");
		} else {
			Com_Printf(" ");
		}
		if (var->flags & CVAR_ROM) {
			Com_Printf("R");
		} else {
			Com_Printf(" ");
		}
		if (var->flags & CVAR_INIT) {
			Com_Printf("I");
		} else {
			Com_Printf(" ");
		}
		if (var->flags & CVAR_ARCHIVE) {
			Com_Printf("A");
		} else {
			Com_Printf(" ");
		}
		if (var->flags & CVAR_LATCH) {
			Com_Printf("L");
		} else {
			Com_Printf(" ");
		}
		if (var->flags & CVAR_CHEAT) {
			Com_Printf("C");
		} else {
			Com_Printf(" ");
		}
		if (var->flags & CVAR_USER_CREATED) {
			Com_Printf("?");
		} else {
			Com_Printf(" ");
		}

		Com_Printf (" %s \"%s\", default \"%s\"\n", var->name, value, var->resetString);
	}

	Com_Printf ("\n%i total modified cvars\n", totalModified);
}

/*
============
CvarSystem::unset

Unsets a cvar
============
*/

cvar_t *CvarSystem::unset(cvar_t *cv)
{
	cvar_t *next = cv->next;

	// note what types of cvars have been modified (userinfo, archive, serverinfo, systeminfo)
	cvar_modifiedFlags |= cv->flags;

	if(cv->name)
		Z_Free(cv->name);
	if(cv->string)
		Z_Free(cv->string);
	if(cv->latchedString)
		Z_Free(cv->latchedString);
	if(cv->resetString)
		Z_Free(cv->resetString);
	if(cv->description)
		Z_Free(cv->description);

	if(cv->prev)
		cv->prev->next = cv->next;
	else
		cvar_vars = cv->next;
	if(cv->next)
		cv->next->prev = cv->prev;

	if(cv->hashPrev)
		cv->hashPrev->hashNext = cv->hashNext;
	else
		hashTable[cv->hashIndex] = cv->hashNext;
	if(cv->hashNext)
		cv->hashNext->hashPrev = cv->hashPrev;

	Com_Memset(cv, '\0', sizeof(*cv));
	
	return next;
}

/*
============
CvarSystem::unset_f

Unsets a userdefined cvar
============
*/

void CvarSystem::unset_f()
{
	cvar_t *cv;
	
	if(CommandSystem::argc() != 2)
	{
		Com_Printf("Usage: %s <varname>\n", CommandSystem::argv(0));
		return;
	}
	
	cv = Cvar_FindVar(CommandSystem::argv(1));

	if(!cv)
		return;
	
	if(cv->flags & CVAR_USER_CREATED)
		CvarSystem::unset(cv);
	else
		Com_Printf("Error: %s: Variable %s is not user created.\n", CommandSystem::argv(0), cv->name);
}



/*
============
CvarSystem::restart

Resets all cvars to their hardcoded values and removes userdefined variables
and variables added via the VMs if requested.
============
*/

void CvarSystem::restart(bool unsetVM)
{
	cvar_t	*curvar;

	curvar = cvar_vars;

	while(curvar)
	{
		if((curvar->flags & CVAR_USER_CREATED) ||
			(unsetVM && (curvar->flags & CVAR_VM_CREATED)))
		{
			// throw out any variables the user/vm created
			curvar = CvarSystem::unset(curvar);
			continue;
		}
		
		if(!(curvar->flags & (CVAR_ROM | CVAR_INIT | CVAR_NORESTART)))
		{
			// Just reset the rest to their default values.
			CvarSystem::set2(curvar->name, curvar->resetString, false);
		}
		
		curvar = curvar->next;
	}
}


/*
============
CvarSystem::restart_f

Resets all cvars to their hardcoded values
============
*/
void CvarSystem::restart_f()
{
	CvarSystem::restart(false);
}

/*
=====================
CvarSystem::infoString
=====================
*/
char *CvarSystem::infoString(int bit)
{
	static char	info[MAX_INFO_STRING];
	cvar_t	*var;

	info[0] = 0;

	for(var = cvar_vars; var; var = var->next)
	{
		if(var->name && (var->flags & bit))
			Info::setValueForKey (info, var->name, var->string);
	}

	return info;
}

/*
=====================
CvarSystem::infoString_Big

  handles large info strings ( CS_SYSTEMINFO )
=====================
*/
char *CvarSystem::infoString_Big(int bit)
{
	static char	info[BIG_INFO_STRING];
	cvar_t	*var;

	info[0] = 0;

	for (var = cvar_vars; var; var = var->next)
	{
		if(var->name && (var->flags & bit))
			Info::setValueForKey_Big (info, var->name, var->string);
	}
	return info;
}



/*
=====================
CvarSystem::infoStringBuffer
=====================
*/
void CvarSystem::infoStringBuffer( int bit, char* buff, int buffsize ) {
	Q_strncpyz(buff,CvarSystem::infoString(bit),buffsize);
}

/*
=====================
CvarSystem::checkRange
=====================
*/
void CvarSystem::checkRange( cvar_t *var, float min, float max, bool integral )
{
	var->validate = true;
	var->min = min;
	var->max = max;
	var->integral = integral;

	// Force an initial range check
	CvarSystem::set( var->name, var->string );
}

/*
=====================
CvarSystem::setDescription
=====================
*/
void CvarSystem::setDescription( cvar_t *var, const char *var_description )
{
	if( var_description && var_description[0] != '\0' )
	{
		if( var->description != nullptr )
		{
			Z_Free( var->description );
		}
		var->description = CopyString( var_description );
	}
}

/*
=====================
CvarSystem::register_

basically a slightly modified CvarSystem::get for the interpreted modules
=====================
*/
void CvarSystem::register_(vmCvar_t *vmCvar, const char *varName, const char *defaultValue, int flags)
{
	cvar_t	*cv;

	// There is code in CvarSystem::get to prevent CVAR_ROM cvars being changed by the
	// user. In other words CVAR_ARCHIVE and CVAR_ROM are mutually exclusive
	// flags. Unfortunately some historical game code (including single player
	// baseq3) sets both flags. We unset CVAR_ROM for such cvars.
	if ((flags & (CVAR_ARCHIVE | CVAR_ROM)) == (CVAR_ARCHIVE | CVAR_ROM)) {
		Com_DPrintf( S_COLOR_YELLOW "WARNING: Unsetting CVAR_ROM from cvar '%s', "
			"since it is also CVAR_ARCHIVE\n", varName );
		flags &= ~CVAR_ROM;
	}

	// Don't allow VM to specific a different creator or other internal flags.
	if ( flags & CVAR_USER_CREATED ) {
		Com_DPrintf( S_COLOR_YELLOW "WARNING: VM tried to set CVAR_USER_CREATED on cvar '%s'\n", varName );
		flags &= ~CVAR_USER_CREATED;
	}
	if ( flags & CVAR_SERVER_CREATED ) {
		Com_DPrintf( S_COLOR_YELLOW "WARNING: VM tried to set CVAR_SERVER_CREATED on cvar '%s'\n", varName );
		flags &= ~CVAR_SERVER_CREATED;
	}
	if ( flags & CVAR_PROTECTED ) {
		Com_DPrintf( S_COLOR_YELLOW "WARNING: VM tried to set CVAR_PROTECTED on cvar '%s'\n", varName );
		flags &= ~CVAR_PROTECTED;
	}
	if ( flags & CVAR_MODIFIED ) {
		Com_DPrintf( S_COLOR_YELLOW "WARNING: VM tried to set CVAR_MODIFIED on cvar '%s'\n", varName );
		flags &= ~CVAR_MODIFIED;
	}
	if ( flags & CVAR_NONEXISTENT ) {
		Com_DPrintf( S_COLOR_YELLOW "WARNING: VM tried to set CVAR_NONEXISTENT on cvar '%s'\n", varName );
		flags &= ~CVAR_NONEXISTENT;
	}

	cv = Cvar_FindVar(varName);

	// Don't modify cvar if it's protected.
	if ( cv && ( cv->flags & CVAR_PROTECTED ) ) {
		Com_DPrintf( S_COLOR_YELLOW "WARNING: VM tried to register protected cvar '%s' with value '%s'%s\n",
			varName, defaultValue, ( flags & ~cv->flags ) != 0 ? " and new flags" : "" );
	} else {
		cv = CvarSystem::get(varName, defaultValue, flags | CVAR_VM_CREATED);
	}

	if (!vmCvar)
		return;

	vmCvar->handle = cv - cvar_indexes;
	vmCvar->modificationCount = -1;
	CvarSystem::update( vmCvar );
}


/*
=====================
CvarSystem::update

updates an interpreted modules' version of a cvar
=====================
*/
void	CvarSystem::update( vmCvar_t *vmCvar ) {
	cvar_t	*cv = nullptr;
	assert(vmCvar);

	if ( static_cast<unsigned>(vmCvar->handle) >= cvar_numIndexes ) {
		Com_Error( ERR_DROP, "CvarSystem::update: handle out of range" );
	}

	cv = cvar_indexes + vmCvar->handle;

	if ( cv->modificationCount == vmCvar->modificationCount ) {
		return;
	}
	if ( !cv->string ) {
		return;		// variable might have been cleared by a cvar_restart
	}
	vmCvar->modificationCount = cv->modificationCount;
	if ( strlen(cv->string)+1 > MAX_CVAR_VALUE_STRING ) 
	  Com_Error( ERR_DROP, "CvarSystem::update: src %s length %u exceeds MAX_CVAR_VALUE_STRING",
		     cv->string, 
		     static_cast<unsigned int>(strlen(cv->string)));
	Q_strncpyz( vmCvar->string, cv->string,  MAX_CVAR_VALUE_STRING ); 

	vmCvar->value = cv->value;
	vmCvar->integer = cv->integer;
}

/*
==================
CvarSystem::completeCvarName
==================
*/
void CvarSystem::completeCvarName( char *args, int argNum )
{
	if( argNum == 2 )
	{
		// Skip "<cmd> "
		char *p = Com_SkipTokens( args, 1, " " );

		if( p > args )
			Field_CompleteCommand( p, false, true );
	}
}

/*
============
CvarSystem::init

Reads in all archived cvars
============
*/
void CvarSystem::init ()
{
	Com_Memset(cvar_indexes, '\0', sizeof(cvar_indexes));
	Com_Memset(hashTable, '\0', sizeof(hashTable));

	cvar_cheats = CvarSystem::get("sv_cheats", "1", CVAR_ROM | CVAR_SYSTEMINFO );

	CommandSystem::addCommand ("print", CvarSystem::print_f);
	CommandSystem::addCommand ("toggle", CvarSystem::toggle_f);
	CommandSystem::setCommandCompletionFunc( "toggle", CvarSystem::completeCvarName );
	CommandSystem::addCommand ("set", CvarSystem::set_f);
	CommandSystem::setCommandCompletionFunc( "set", CvarSystem::completeCvarName );
	CommandSystem::addCommand ("sets", CvarSystem::set_f);
	CommandSystem::setCommandCompletionFunc( "sets", CvarSystem::completeCvarName );
	CommandSystem::addCommand ("setu", CvarSystem::set_f);
	CommandSystem::setCommandCompletionFunc( "setu", CvarSystem::completeCvarName );
	CommandSystem::addCommand ("seta", CvarSystem::set_f);
	CommandSystem::setCommandCompletionFunc( "seta", CvarSystem::completeCvarName );
	CommandSystem::addCommand ("reset", CvarSystem::reset_f);
	CommandSystem::setCommandCompletionFunc( "reset", CvarSystem::completeCvarName );
	CommandSystem::addCommand ("unset", CvarSystem::unset_f);
	CommandSystem::setCommandCompletionFunc("unset", CvarSystem::completeCvarName);

	CommandSystem::addCommand ("cvarlist", CvarSystem::list_f);
	CommandSystem::addCommand ("cvar_modified", CvarSystem::listModified_f);
	CommandSystem::addCommand ("cvar_restart", CvarSystem::restart_f);
}
