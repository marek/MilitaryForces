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

cvar_t		*CvarSystem::vars_ = nullptr;
cvar_t		*CvarSystem::cheats_;
int			CvarSystem::modifiedFlags_;

cvar_t		CvarSystem::indexes_[CvarSystem::MAX_CVARS];
int			CvarSystem::numIndexes_;

cvar_t		*CvarSystem::hashTable_[CvarSystem::FILE_HASH_SIZE];

/*
================
return a hash value for the filename
================
*/
long CvarSystem::generateHashValue( const char *fname ) {
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
bool CvarSystem::validateString( const char *s ) {
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
cvar_t * CvarSystem::findVar( const char *var_name ) {
	cvar_t	*var;
	long hash;

	hash = generateHashValue(var_name);
	
	for (var=hashTable_[hash] ; var ; var=var->hashNext_) {
		if (!Q_stricmp(var_name, var->name_)) {
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
	
	var = findVar (var_name);
	if (!var)
		return 0;
	return var->value_;
}


/*
============
CvarSystem::variableIntegerValue
============
*/
int CvarSystem::variableIntegerValue( const char *var_name ) {
	cvar_t	*var;
	
	var = findVar (var_name);
	if (!var)
		return 0;
	return var->integer_;
}


/*
============
CvarSystem::variableString
============
*/
char *CvarSystem::variableString( const char *var_name ) {
	cvar_t *var;
	
	var = findVar (var_name);
	if (!var)
		return "";
	return var->string_;
}


/*
============
CvarSystem::variableStringBuffer
============
*/
void CvarSystem::variableStringBuffer( const char *var_name, char *buffer, int bufsize ) {
	cvar_t *var;
	
	var = findVar (var_name);
	if (!var) {
		*buffer = 0;
	}
	else {
		Q_strncpyz( buffer, var->string_, bufsize );
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
	
	if(!(var = findVar(var_name)))
		return CVAR_NONEXISTENT;
	else
	{
		if(var->modified_)
			return var->flags_ | CVAR_MODIFIED;
		else
			return var->flags_;
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
	
	for(cvar = vars_; cvar; cvar = cvar->next_)
	{
		if(cvar->name_)
			callback(cvar->name_);
	}
}

/*
============
Cvar_Validate
============
*/
const char * CvarSystem::validate( cvar_t *var,
    const char *value, bool warn )
{
	static char s[ MAX_CVAR_VALUE_STRING ];
	float valuef;
	bool changed = false;

	if( !var->validate_ )
		return value;

	if( !value )
		return value;

	if( Q_isanumber( value ) )
	{
		valuef = atof( value );

		if( var->integral_ )
		{
			if( !Q_isintegral( valuef ) )
			{
				if( warn )
					Common::printf( "WARNING: cvar '%s' must be integral", var->name_ );

				valuef = static_cast<int>(valuef);
				changed = true;
			}
		}
	}
	else
	{
		if( warn )
			Common::printf( "WARNING: cvar '%s' must be numeric", var->name_ );

		valuef = atof( var->resetString_ );
		changed = true;
	}

	if( valuef < var->min_ )
	{
		if( warn )
		{
			if( changed )
				Common::printf( " and is" );
			else
				Common::printf( "WARNING: cvar '%s'", var->name_ );

			if( Q_isintegral( var->min_ ) )
				Common::printf( " out of range (min %d)", static_cast<int>(var->min_) );
			else
				Common::printf( " out of range (min %f)", var->min_ );
		}

		valuef = var->min_;
		changed = true;
	}
	else if( valuef > var->max_ )
	{
		if( warn )
		{
			if( changed )
				Common::printf( " and is" );
			else
				Common::printf( "WARNING: cvar '%s'", var->name_ );

			if( Q_isintegral( var->max_ ) )
				Common::printf( " out of range (max %d)", static_cast<int>(var->max_) );
			else
				Common::printf( " out of range (max %f)", var->max_ );
		}

		valuef = var->max_;
		changed = true;
	}

	if( changed )
	{
		if( Q_isintegral( valuef ) )
		{
			Com_sprintf( s, sizeof( s ), "%d", static_cast<int>(valuef) );

			if( warn )
				Common::printf( ", setting to %d\n", static_cast<int>(valuef) );
		}
		else
		{
			Com_sprintf( s, sizeof( s ), "%f", valuef );

			if( warn )
				Common::printf( ", setting to %f\n", valuef );
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
		Common::error( ERR_FATAL, "CvarSystem::get: NULL parameter" );
	}

	if ( !validateString( var_name ) ) {
		Common::printf("invalid cvar name string: %s\n", var_name );
		var_name = "BADNAME";
	}

#if 0		// FIXME: values with backslash happen
	if ( !validateString( var_value ) ) {
		Common::printf("invalid cvar value string: %s\n", var_value );
		var_value = "BADVALUE";
	}
#endif

	var = findVar (var_name);
	
	if(var)
	{
		var_value = validate(var, var_value, false);

		// Make sure the game code cannot mark engine-added variables as gamecode vars
		if(var->flags_ & CVAR_VM_CREATED)
		{
			if(!(flags & CVAR_VM_CREATED))
				var->flags_ &= ~CVAR_VM_CREATED;
		}
		else if (!(var->flags_ & CVAR_USER_CREATED))
		{
			if(flags & CVAR_VM_CREATED)
				flags &= ~CVAR_VM_CREATED;
		}

		// if the C code is now specifying a variable that the user already
		// set a value for, take the new value as the reset value
		if(var->flags_ & CVAR_USER_CREATED)
		{
			var->flags_ &= ~CVAR_USER_CREATED;
			Zone::free( var->resetString_ );
			var->resetString_ = CopyString( var_value );

			if(flags & CVAR_ROM)
			{
				// this variable was set by the user,
				// so force it to value given by the engine.

				if(var->latchedString_)
					Zone::free(var->latchedString_);
				
				var->latchedString_ = CopyString(var_value);
			}
		}
		
		// Make sure servers cannot mark engine-added variables as SERVER_CREATED
		if(var->flags_ & CVAR_SERVER_CREATED)
		{
			if(!(flags & CVAR_SERVER_CREATED))
				var->flags_ &= ~CVAR_SERVER_CREATED;
		}
		else
		{
			if(flags & CVAR_SERVER_CREATED)
				flags &= ~CVAR_SERVER_CREATED;
		}
		
		var->flags_ |= flags;

		// only allow one non-empty reset string without a warning
		if ( !var->resetString_[0] ) {
			// we don't have a reset string yet
			Zone::free( var->resetString_ );
			var->resetString_ = CopyString( var_value );
		} else if ( var_value[0] && strcmp( var->resetString_, var_value ) ) {
			Common::dPrintf( "Warning: cvar \"%s\" given initial values: \"%s\" and \"%s\"\n",
				var_name, var->resetString_, var_value );
		}
		// if we have a latched string, take that value now
		if ( var->latchedString_ ) {
			char *s;

			s = var->latchedString_;
			var->latchedString_ = nullptr;	// otherwise cvar_set2 would free it
			CvarSystem::set2( var_name, s, true );
			Zone::free( s );
		}

		// ZOID--needs to be set so that cvars the game sets as 
		// SERVERINFO get sent to clients
		modifiedFlags_ |= flags;

		return var;
	}

	//
	// allocate a new cvar
	//

	// find a free cvar
	for(index = 0; index < MAX_CVARS; index++)
	{
		if(!indexes_[index].name_)
			break;
	}

	if(index >= MAX_CVARS)
	{
		if(!com_errorEntered)
			Common::error(ERR_FATAL, "Error: Too many cvars, cannot create a new one!");

		return nullptr;
	}
	
	var = &indexes_[index];
	
	if(index >= numIndexes_)
		numIndexes_ = index + 1;
		
	var->name_ = CopyString (var_name);
	var->string_ = CopyString (var_value);
	var->modified_ = true;
	var->modificationCount_ = 1;
	var->value_ = atof (var->string_);
	var->integer_ = atoi(var->string_);
	var->resetString_ = CopyString( var_value );
	var->validate_ = false;
	var->description_ = nullptr;

	// link the variable in
	var->next_ = vars_;
	if(vars_)
		vars_->prev_ = var;

	var->prev_ = nullptr;
	vars_ = var;

	var->flags_ = flags;
	// note what types of cvars have been modified (userinfo, archive, serverinfo, systeminfo)
	modifiedFlags_ |= var->flags_;

	hash = generateHashValue(var_name);
	var->hashIndex_ = hash;

	var->hashNext_ = hashTable_[hash];
	if(hashTable_[hash])
		hashTable_[hash]->hashPrev_ = var;

	var->hashPrev_ = nullptr;
	hashTable_[hash] = var;

	return var;
}

/*
============
CvarSystem::print

Prints the value, default, and latched string of the given variable
============
*/
void CvarSystem::print( cvar_t *v ) {
	Common::printf ("\"%s\" is:\"%s" S_COLOR_WHITE "\"",
			v->name_, v->string_ );

	if ( !( v->flags_ & CVAR_ROM ) ) {
		if ( !Q_stricmp( v->string_, v->resetString_ ) ) {
			Common::printf (", the default" );
		} else {
			Common::printf (" default:\"%s" S_COLOR_WHITE "\"",
					v->resetString_ );
		}
	}

	Common::printf ("\n");

	if ( v->latchedString_ ) {
		Common::printf( "latched: \"%s\"\n", v->latchedString_ );
	}

	if ( v->description_ ) {
		Common::printf( "%s\n", v->description_ );
	}
}

/*
============
CvarSystem::set2
============
*/
cvar_t *CvarSystem::set2( const char *var_name, const char *value, bool force ) {
	cvar_t	*var;

//	Common::dPrintf( "CvarSystem::set2: %s %s\n", var_name, value );

	if ( !validateString( var_name ) ) {
		Common::printf("invalid cvar name string: %s\n", var_name );
		var_name = "BADNAME";
	}

#if 0	// FIXME
	if ( value && !validateString( value ) ) {
		Common::printf("invalid cvar value string: %s\n", value );
		var_value = "BADVALUE";
	}
#endif

	var = findVar (var_name);
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
		value = var->resetString_;
	}

	value = validate(var, value, true);

	if((var->flags_ & CVAR_LATCH) && var->latchedString_)
	{
		if(!strcmp(value, var->string_))
		{
			Zone::free(var->latchedString_);
			var->latchedString_ = nullptr;
			return var;
		}

		if(!strcmp(value, var->latchedString_))
			return var;
	}
	else if(!strcmp(value, var->string_))
		return var;

	// note what types of cvars have been modified (userinfo, archive, serverinfo, systeminfo)
	modifiedFlags_ |= var->flags_;

	if (!force)
	{
		if (var->flags_ & CVAR_ROM)
		{
			Common::printf ("%s is read only.\n", var_name);
			return var;
		}

		if (var->flags_ & CVAR_INIT)
		{
			Common::printf ("%s is write protected.\n", var_name);
			return var;
		}

		if ((var->flags_ & CVAR_CHEAT) && !cheats_->integer_)
		{
			Common::printf ("%s is cheat protected.\n", var_name);
			return var;
		}
		
		if (var->flags_ & CVAR_LATCH)
		{
			if (var->latchedString_)
			{
				if (strcmp(value, var->latchedString_) == 0)
					return var;
				Zone::free (var->latchedString_);
			}
			else
			{
				if (strcmp(value, var->string_) == 0)
					return var;
			}

			Common::printf ("%s will be changed upon restarting.\n", var_name);
			var->latchedString_ = CopyString(value);
			var->modified_ = true;
			var->modificationCount_++;
			return var;
		}
	}
	else
	{
		if (var->latchedString_)
		{
			Zone::free (var->latchedString_);
			var->latchedString_ = nullptr;
		}
	}

	if (!strcmp(value, var->string_))
		return var;		// not changed

	var->modified_ = true;
	var->modificationCount_++;
	
	Zone::free (var->string_);	// free the old value string
	
	var->string_ = CopyString(value);
	var->value_ = atof (var->string_);
	var->integer_ = atoi (var->string_);

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
			Common::error( ERR_DROP, "Restricted source tried to set "
				"\"%s\" to \"%s\"", var_name, value );
		else
			Common::error( ERR_DROP, "Restricted source tried to "
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
	for(var = vars_; var ; var = var->next_)
	{
		if(var->flags_ & CVAR_CHEAT)
		{
			// the CVAR_LATCHED|CVAR_CHEAT vars might escape the reset here 
			// because of a different var->latchedString
			if (var->latchedString_)
			{
				Zone::free(var->latchedString_);
				var->latchedString_ = nullptr;
			}
			if (strcmp(var->resetString_,var->string_))
				CvarSystem::set(var->name_, var->resetString_);
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
	v = findVar (CommandSystem::argv(0));
	if (!v) {
		return false;
	}

	// perform a variable print or set
	if ( CommandSystem::argc() == 1 ) {
		CvarSystem::print( v );
		return true;
	}

	// set the value if forcing isn't required
	CvarSystem::set2 (v->name_, CommandSystem::args(), false);
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
		Common::printf ("usage: print <variable>\n");
		return;
	}

	name = CommandSystem::argv(1);

	cv = findVar(name);
	
	if(cv)
		CvarSystem::print(cv);
	else
		Common::printf ("Cvar %s does not exist.\n", name);
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
		Common::printf("usage: toggle <variable> [value1, value2, ...]\n");
		return;
	}

	if(c == 2) {
		CvarSystem::set2(CommandSystem::argv(1), va("%d", 
			!CvarSystem::variableValue(CommandSystem::argv(1))), 
			false);
		return;
	}

	if(c == 3) {
		Common::printf("toggle: nothing to toggle to\n");
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
		Common::printf ("usage: %s <variable> <value>\n", cmd);
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
			if( !( v->flags_ & CVAR_ARCHIVE ) ) {
				v->flags_ |= CVAR_ARCHIVE;
				modifiedFlags_ |= CVAR_ARCHIVE;
			}
			break;
		case 'u':
			if( !( v->flags_ & CVAR_USERINFO ) ) {
				v->flags_ |= CVAR_USERINFO;
				modifiedFlags_ |= CVAR_USERINFO;
			}
			break;
		case 's':
			if( !( v->flags_ & CVAR_SERVERINFO ) ) {
				v->flags_ |= CVAR_SERVERINFO;
				modifiedFlags_ |= CVAR_SERVERINFO;
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
		Common::printf ("usage: reset <variable>\n");
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

	for (var = vars_; var; var = var->next_)
	{
		if(!var->name_ || Q_stricmp( var->name_, "cl_cdkey" ) == 0)
			continue;

		if( var->flags_ & CVAR_ARCHIVE ) {
			// write the latched value, even if it hasn't taken effect yet
			if ( var->latchedString_ ) {
				if( strlen( var->name_ ) + strlen( var->latchedString_ ) + 10 > sizeof( buffer ) ) {
					Common::printf( S_COLOR_YELLOW "WARNING: value of variable "
							"\"%s\" too long to write to file\n", var->name_ );
					continue;
				}
				Com_sprintf (buffer, sizeof(buffer), "seta %s \"%s\"\n", var->name_, var->latchedString_);
			} else {
				if( strlen( var->name_ ) + strlen( var->string_ ) + 10 > sizeof( buffer ) ) {
					Common::printf( S_COLOR_YELLOW "WARNING: value of variable "
							"\"%s\" too long to write to file\n", var->name_ );
					continue;
				}
				Com_sprintf (buffer, sizeof(buffer), "seta %s \"%s\"\n", var->name_, var->string_);
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
	for (var = vars_ ; var ; var = var->next_, i++)
	{
		if(!var->name_ || (match && !Common::filter(match, var->name_, false)))
			continue;

		if (var->flags_ & CVAR_SERVERINFO) {
			Common::printf("S");
		} else {
			Common::printf(" ");
		}
		if (var->flags_ & CVAR_SYSTEMINFO) {
			Common::printf("s");
		} else {
			Common::printf(" ");
		}
		if (var->flags_ & CVAR_USERINFO) {
			Common::printf("U");
		} else {
			Common::printf(" ");
		}
		if (var->flags_ & CVAR_ROM) {
			Common::printf("R");
		} else {
			Common::printf(" ");
		}
		if (var->flags_ & CVAR_INIT) {
			Common::printf("I");
		} else {
			Common::printf(" ");
		}
		if (var->flags_ & CVAR_ARCHIVE) {
			Common::printf("A");
		} else {
			Common::printf(" ");
		}
		if (var->flags_ & CVAR_LATCH) {
			Common::printf("L");
		} else {
			Common::printf(" ");
		}
		if (var->flags_ & CVAR_CHEAT) {
			Common::printf("C");
		} else {
			Common::printf(" ");
		}
		if (var->flags_ & CVAR_USER_CREATED) {
			Common::printf("?");
		} else {
			Common::printf(" ");
		}

		Common::printf (" %s \"%s\"\n", var->name_, var->string_);
	}

	Common::printf ("\n%i total cvars\n", i);
	Common::printf ("%i cvar indexes\n", numIndexes_);
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
	for (var = vars_ ; var ; var = var->next_)
	{
		if ( !var->name_ || !var->modificationCount_ )
			continue;

		value = var->latchedString_ ? var->latchedString_ : var->string_;
		if ( !strcmp( value, var->resetString_ ) )
			continue;

		totalModified++;

		if (match && !Common::filter(match, var->name_, false))
			continue;

		if (var->flags_ & CVAR_SERVERINFO) {
			Common::printf("S");
		} else {
			Common::printf(" ");
		}
		if (var->flags_ & CVAR_SYSTEMINFO) {
			Common::printf("s");
		} else {
			Common::printf(" ");
		}
		if (var->flags_ & CVAR_USERINFO) {
			Common::printf("U");
		} else {
			Common::printf(" ");
		}
		if (var->flags_ & CVAR_ROM) {
			Common::printf("R");
		} else {
			Common::printf(" ");
		}
		if (var->flags_ & CVAR_INIT) {
			Common::printf("I");
		} else {
			Common::printf(" ");
		}
		if (var->flags_ & CVAR_ARCHIVE) {
			Common::printf("A");
		} else {
			Common::printf(" ");
		}
		if (var->flags_ & CVAR_LATCH) {
			Common::printf("L");
		} else {
			Common::printf(" ");
		}
		if (var->flags_ & CVAR_CHEAT) {
			Common::printf("C");
		} else {
			Common::printf(" ");
		}
		if (var->flags_ & CVAR_USER_CREATED) {
			Common::printf("?");
		} else {
			Common::printf(" ");
		}

		Common::printf (" %s \"%s\", default \"%s\"\n", var->name_, value, var->resetString_);
	}

	Common::printf ("\n%i total modified cvars\n", totalModified);
}

/*
============
CvarSystem::unset

Unsets a cvar
============
*/

cvar_t *CvarSystem::unset(cvar_t *cv)
{
	cvar_t *next = cv->next_;

	// note what types of cvars have been modified (userinfo, archive, serverinfo, systeminfo)
	modifiedFlags_ |= cv->flags_;

	if(cv->name_)
		Zone::free(cv->name_);
	if(cv->string_)
		Zone::free(cv->string_);
	if(cv->latchedString_)
		Zone::free(cv->latchedString_);
	if(cv->resetString_)
		Zone::free(cv->resetString_);
	if(cv->description_)
		Zone::free(cv->description_);

	if(cv->prev_)
		cv->prev_->next_ = cv->next_;
	else
		vars_ = cv->next_;
	if(cv->next_)
		cv->next_->prev_ = cv->prev_;

	if(cv->hashPrev_)
		cv->hashPrev_->hashNext_ = cv->hashNext_;
	else
		hashTable_[cv->hashIndex_] = cv->hashNext_;
	if(cv->hashNext_)
		cv->hashNext_->hashPrev_ = cv->hashPrev_;

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
		Common::printf("Usage: %s <varname>\n", CommandSystem::argv(0));
		return;
	}
	
	cv = findVar(CommandSystem::argv(1));

	if(!cv)
		return;
	
	if(cv->flags_ & CVAR_USER_CREATED)
		CvarSystem::unset(cv);
	else
		Common::printf("Error: %s: Variable %s is not user created.\n", CommandSystem::argv(0), cv->name_);
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

	curvar = vars_;

	while(curvar)
	{
		if((curvar->flags_ & CVAR_USER_CREATED) ||
			(unsetVM && (curvar->flags_ & CVAR_VM_CREATED)))
		{
			// throw out any variables the user/vm created
			curvar = CvarSystem::unset(curvar);
			continue;
		}
		
		if(!(curvar->flags_ & (CVAR_ROM | CVAR_INIT | CVAR_NORESTART)))
		{
			// Just reset the rest to their default values.
			CvarSystem::set2(curvar->name_, curvar->resetString_, false);
		}
		
		curvar = curvar->next_;
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

	for(var = vars_; var; var = var->next_)
	{
		if(var->name_ && (var->flags_ & bit))
			Info::setValueForKey (info, var->name_, var->string_);
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

	for (var = vars_; var; var = var->next_)
	{
		if(var->name_ && (var->flags_ & bit))
			Info::setValueForKey_Big (info, var->name_, var->string_);
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
	var->validate_ = true;
	var->min_ = min;
	var->max_ = max;
	var->integral_ = integral;

	// Force an initial range check
	CvarSystem::set( var->name_, var->string_ );
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
		if( var->description_ != nullptr )
		{
			Zone::free( var->description_ );
		}
		var->description_ = CopyString( var_description );
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
		Common::dPrintf( S_COLOR_YELLOW "WARNING: Unsetting CVAR_ROM from cvar '%s', "
			"since it is also CVAR_ARCHIVE\n", varName );
		flags &= ~CVAR_ROM;
	}

	// Don't allow VM to specific a different creator or other internal flags.
	if ( flags & CVAR_USER_CREATED ) {
		Common::dPrintf( S_COLOR_YELLOW "WARNING: VM tried to set CVAR_USER_CREATED on cvar '%s'\n", varName );
		flags &= ~CVAR_USER_CREATED;
	}
	if ( flags & CVAR_SERVER_CREATED ) {
		Common::dPrintf( S_COLOR_YELLOW "WARNING: VM tried to set CVAR_SERVER_CREATED on cvar '%s'\n", varName );
		flags &= ~CVAR_SERVER_CREATED;
	}
	if ( flags & CVAR_PROTECTED ) {
		Common::dPrintf( S_COLOR_YELLOW "WARNING: VM tried to set CVAR_PROTECTED on cvar '%s'\n", varName );
		flags &= ~CVAR_PROTECTED;
	}
	if ( flags & CVAR_MODIFIED ) {
		Common::dPrintf( S_COLOR_YELLOW "WARNING: VM tried to set CVAR_MODIFIED on cvar '%s'\n", varName );
		flags &= ~CVAR_MODIFIED;
	}
	if ( flags & CVAR_NONEXISTENT ) {
		Common::dPrintf( S_COLOR_YELLOW "WARNING: VM tried to set CVAR_NONEXISTENT on cvar '%s'\n", varName );
		flags &= ~CVAR_NONEXISTENT;
	}

	cv = findVar(varName);

	// Don't modify cvar if it's protected.
	if ( cv && ( cv->flags_ & CVAR_PROTECTED ) ) {
		Common::dPrintf( S_COLOR_YELLOW "WARNING: VM tried to register protected cvar '%s' with value '%s'%s\n",
			varName, defaultValue, ( flags & ~cv->flags_ ) != 0 ? " and new flags" : "" );
	} else {
		cv = CvarSystem::get(varName, defaultValue, flags | CVAR_VM_CREATED);
	}

	if (!vmCvar)
		return;

	vmCvar->handle = cv - indexes_;
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

	if ( static_cast<unsigned>(vmCvar->handle) >= numIndexes_ ) {
		Common::error( ERR_DROP, "CvarSystem::update: handle out of range" );
	}

	cv = indexes_ + vmCvar->handle;

	if ( cv->modificationCount_ == vmCvar->modificationCount ) {
		return;
	}
	if ( !cv->string_ ) {
		return;		// variable might have been cleared by a cvar_restart
	}
	vmCvar->modificationCount = cv->modificationCount_;
	if ( strlen(cv->string_)+1 > MAX_CVAR_VALUE_STRING ) 
	  Common::error( ERR_DROP, "CvarSystem::update: src %s length %u exceeds MAX_CVAR_VALUE_STRING",
		     cv->string_, 
		     static_cast<unsigned int>(strlen(cv->string_)));
	Q_strncpyz( vmCvar->string, cv->string_,  MAX_CVAR_VALUE_STRING ); 

	vmCvar->value = cv->value_;
	vmCvar->integer = cv->integer_;
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
	Com_Memset(indexes_, '\0', sizeof(indexes_));
	Com_Memset(hashTable_, '\0', sizeof(hashTable_));

	cheats_ = CvarSystem::get("sv_cheats", "1", CVAR_ROM | CVAR_SYSTEMINFO );

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
