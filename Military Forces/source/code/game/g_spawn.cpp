/*
 * $Id: g_spawn.c,v 1.5 2006-01-29 14:03:41 thebjoern Exp $
*/

// Copyright (C) 1999-2000 Id Software, Inc.
//


#include "g_local.h"
#include "g_level.h"
#include "g_world.h"


bool G_SpawnString( const char *key, const char *defaultString, char **out ) 
{
	int		i;

	if ( !theLevel.spawning_ ) 
	{
		*out = (char *)defaultString;
//		G_Error( "G_SpawnString() called while not spawning" );
	}

	for ( i = 0 ; i < theLevel.numSpawnVars_ ; i++ ) 
	{
		if ( !Q_stricmp( key, theLevel.spawnVars_[i][0] ) ) 
		{
			*out = theLevel.spawnVars_[i][1];
			return true;
		}
	}

	*out = (char *)defaultString;
	return false;
}

bool G_SpawnFloat( const char *key, const char *defaultString, float *out )
{
	char		*s;
	bool	present;

	present = G_SpawnString( key, defaultString, &s );
	*out = atof( s );
	return present;
}

bool G_SpawnInt( const char *key, const char *defaultString, int *out ) 
{
	char		*s;
	bool	present;

	present = G_SpawnString( key, defaultString, &s );
	*out = atoi( s );
	return present;
}

bool G_SpawnVector( const char *key, const char *defaultString, float *out ) 
{
	char		*s;
	bool	present;

	present = G_SpawnString( key, defaultString, &s );
	sscanf( s, "%f %f %f", &out[0], &out[1], &out[2] );
	return present;
}



//
// fields are needed for spawning from the entity string
//
typedef enum 
{
	F_INT, 
	F_FLOAT,
	F_LSTRING,			// string on disk, pointer in memory, TAG_LEVEL
	F_GSTRING,			// string on disk, pointer in memory, TAG_GAME
	F_VECTOR,
	F_ANGLEHACK,
	F_ENTITY,			// index on disk, pointer in memory
	F_ITEM,				// index on disk, pointer in memory
	F_CLIENT,			// index on disk, pointer in memory
	F_IGNORE
} fieldtype_t;

typedef struct
{
	char	*name;
	int		ofs;
	fieldtype_t	type;
	int		flags;
} field_t;

//field_t fields[] = {
//	{"classname", FOFS(classname_), F_LSTRING},
//	{"origin", FOFS(s.origin), F_VECTOR},
//	{"model", FOFS(model_), F_LSTRING},
//	{"model2", FOFS(model2_), F_LSTRING},
//	{"spawnflags", FOFS(spawnflags_), F_INT},
//	{"speed", FOFS(speed_), F_FLOAT},
//	{"target", FOFS(target_), F_LSTRING},
//	{"targetname", FOFS(targetname_), F_LSTRING},
//	{"message", FOFS(message_), F_LSTRING},
//	{"team", FOFS(team_), F_LSTRING},
//	{"wait", FOFS(wait_), F_FLOAT},
//	{"random", FOFS(random_), F_FLOAT},
//	{"count", FOFS(count_), F_INT},
//	{"health", FOFS(health_), F_INT},
//	{"light", 0, F_IGNORE},
//	{"dmg", FOFS(damage_), F_INT},
//	{"angles", FOFS(s.angles), F_VECTOR},
//	{"angle", FOFS(s.angles), F_ANGLEHACK},
//	{"targetShaderName", FOFS(targetShaderName_), F_LSTRING},
//	{"targetShaderNewName", FOFS(targetShaderNewName_), F_LSTRING},
//	{"category", FOFS(ent_category_), F_INT},
//	{"score", FOFS(score_), F_INT},
//
//	{NULL}
//};


field_t fields[] = {
	{"classname", FOFS2(classname_), F_LSTRING},
	{"origin", FOFS2(origin_), F_VECTOR},
	{"model", FOFS2(model_), F_LSTRING},
	{"model2", FOFS2(model2_), F_LSTRING},
	{"spawnflags", FOFS2(spawnflags_), F_INT},
	{"speed", FOFS2(speed_), F_FLOAT},
	{"target", FOFS2(target_), F_LSTRING},
	{"targetname", FOFS2(targetname_), F_LSTRING},
	{"message", FOFS2(message_), F_LSTRING},
	{"team", FOFS2(team_), F_LSTRING},
	{"wait", FOFS2(wait_), F_FLOAT},
	{"random", FOFS2(random_), F_FLOAT},
	{"count", FOFS2(count_), F_INT},
	{"health", FOFS2(health_), F_INT},
	{"light", 0, F_IGNORE},
	{"dmg", FOFS2(damage_), F_INT},
	{"angles", FOFS2(angles_), F_VECTOR},
	{"angle", FOFS2(angles_), F_ANGLEHACK},
	{"targetShaderName", FOFS2(targetShaderName_), F_LSTRING},
	{"targetShaderNewName", FOFS2(targetShaderNewName_), F_LSTRING},
	{"category", FOFS2(ent_category_), F_INT},
	{"score", FOFS2(score_), F_INT},

	{NULL}
};




void SP_info_player_start (GameEntity *ent);
void SP_info_player_deathmatch (GameEntity *ent);
void SP_info_player_intermission (GameEntity *ent);

void SP_func_plat (GameEntity *ent);
void SP_func_static (GameEntity *ent);
void SP_func_rotating (GameEntity *ent);
void SP_func_bobbing (GameEntity *ent);
void SP_func_pendulum( GameEntity *ent );
void SP_func_button (GameEntity *ent);
void SP_func_door (GameEntity *ent);
void SP_func_train (GameEntity *ent);
void SP_func_timer (GameEntity *self);

// MFQ3
//void SP_misc_waypoint (gentity_t *ent);
void SP_func_runway (GameEntity *ent);
void SP_func_explosive (GameEntity *ent);
void SP_trigger_recharge (GameEntity *ent);
void SP_trigger_radio (GameEntity *ent);

void SP_func_catapult (GameEntity *ent);
void SP_func_wires (GameEntity *ent);

void SP_ai_radar (GameEntity* ent);
void SP_ai_sam (GameEntity* ent);
void SP_ai_flak (GameEntity* ent);

void SP_trigger_always (GameEntity *ent);
void SP_trigger_multiple (GameEntity *ent);

void SP_target_delay (GameEntity *ent);
void SP_target_speaker (GameEntity *ent);
void SP_target_print (GameEntity *ent);
void SP_target_character (GameEntity *ent);
void SP_target_score( GameEntity *ent );
void SP_target_relay (GameEntity *ent);
void SP_target_kill (GameEntity *ent);
void SP_target_position (GameEntity *ent);
void SP_target_location (GameEntity *ent);

void SP_light (GameEntity *self);
void SP_info_null (GameEntity *self);
void SP_info_notnull (GameEntity *self);
void SP_info_camp (GameEntity *self);
void SP_path_corner (GameEntity *self);

void SP_misc_model(GameEntity *ent);
void SP_misc_portal_camera(GameEntity *ent);
void SP_misc_portal_surface(GameEntity *ent);

void SP_team_CTF_redplayer( GameEntity *ent );
void SP_team_CTF_blueplayer( GameEntity *ent );

void SP_team_CTF_redspawn( GameEntity *ent );
void SP_team_CTF_bluespawn( GameEntity *ent );

void SP_item_botroam( GameEntity *ent ) {};

spawn_t	spawns[] = {
	// info entities don't do anything at all, but provide positional
	// information for things controlled by other processes
	{"info_player_start", SP_info_player_start},
	{"info_player_deathmatch", SP_info_player_deathmatch},
	{"info_player_intermission", SP_info_player_intermission},
	{"info_null", SP_info_null},
	{"info_notnull", SP_info_notnull},		// use target_position instead
	{"info_camp", SP_info_camp},

	{"func_plat", SP_func_plat},
	{"func_button", SP_func_button},
	{"func_door", SP_func_door},
	{"func_static", SP_func_static},
	{"func_rotating", SP_func_rotating},
	{"func_bobbing", SP_func_bobbing},
	{"func_pendulum", SP_func_pendulum},
	{"func_train", SP_func_train},
	{"func_group", SP_info_null},
	{"func_timer", SP_func_timer},			// rename trigger_timer?

	// MFQ3
	{"func_runway", SP_func_runway},
//	{"misc_waypoint", SP_misc_waypoint},
//	{"misc_vehicle", SP_misc_vehicle},
	{"func_explosive", SP_func_explosive},
	{"trigger_recharge", SP_trigger_recharge},
	{"trigger_radio", SP_trigger_radio},

	{"func_catapult", SP_func_catapult},
	{"func_explosive", SP_func_wires},

	{"ai_radar", SP_ai_radar},
	{"ai_sam", SP_ai_sam},
	{"ai_flak", SP_ai_flak},
	
	// Triggers are brush objects that cause an effect when contacted
	// by a living player, usually involving firing targets.
	// While almost everything could be done with
	// a single trigger class and different targets, triggered effects
	// could not be client side predicted (push and teleport).
	{"trigger_always", SP_trigger_always},
	{"trigger_multiple", SP_trigger_multiple},

	// targets perform no action by themselves, but must be triggered
	// by another entity
	{"target_delay", SP_target_delay},
	{"target_speaker", SP_target_speaker},
	{"target_print", SP_target_print},
	{"target_score", SP_target_score},
	{"target_relay", SP_target_relay},
	{"target_kill", SP_target_kill},
	{"target_position", SP_target_position},
	{"target_location", SP_target_location},

	{"light", SP_light},
	{"path_corner", SP_path_corner},

	{"misc_model", SP_misc_model},
	{"misc_portal_surface", SP_misc_portal_surface},
	{"misc_portal_camera", SP_misc_portal_camera},

	{"team_CTF_redplayer", SP_team_CTF_redplayer},
	{"team_CTF_blueplayer", SP_team_CTF_blueplayer},

	{"team_CTF_redspawn", SP_team_CTF_redspawn},
	{"team_CTF_bluespawn", SP_team_CTF_bluespawn},

	{"item_botroam", SP_item_botroam},

	{0, 0}
};

// MFQ3 moved this here from g_items.c 
/*
============
G_ItemDisabled
============
*/
int G_ItemDisabled( gitem_t *item ) 
{

	char name[128];

	Com_sprintf(name, sizeof(name), "disable_%s", item->classname);
	return Cvar_VariableIntegerValue( name );
}

/*
===============
G_CallSpawn

Finds the spawn function for the entity and calls it,
returning false if not found
===============
*/
//bool G_CallSpawn( GameEntity *ent )
bool G_CallSpawn( SpawnFieldHolder *sfh )
{
	spawn_t	*s;
	gitem_t	*item;

	if ( !sfh->classname_ )
	{
		Com_Printf ("G_CallSpawn: NULL classname\n");
		return false;
	}

	// check item spawn functions
	for ( item=bg_itemlist+1 ; item->classname ; item++ ) 
	{
		if ( !strcmp(item->classname, sfh->classname_) ) 
		{
			if ( G_ItemDisabled(item) )	// MFQ3
				return false;			// MFQ3
			theLevel.createItemFromSpawnFields(sfh, item);
			//G_SpawnItem( sfh, item );
			return true;
		}
	}

	// check normal spawn functions
	for ( s=spawns ; s->name ; s++ ) 
	{
		if ( !strcmp(s->name, sfh->classname_) ) 
		{
			theLevel.createEntityFromSpawnFields(sfh, s);
			return true;
		}
	}
	Com_Printf ("%s doesn't have a spawn function\n", sfh->classname_);
	return false;
}

/*
=============
G_NewString

Builds a copy of the string, translating \n to real linefeeds
so message texts can be multi-line
=============
*/
char *G_NewString( const char *string ) 
{
	char	*newb, *new_p;
	int		i,l;
	
	l = strlen(string) + 1;

	newb = reinterpret_cast<char*>(G_Alloc( l ));

	new_p = newb;

	// turn \n into a real linefeed
	for ( i=0 ; i< l ; i++ ) {
		if (string[i] == '\\' && i < l-1) {
			i++;
			if (string[i] == 'n') {
				*new_p++ = '\n';
			} else {
				*new_p++ = '\\';
			}
		} else {
			*new_p++ = string[i];
		}
	}
	
	return newb;
}




/*
===============
G_ParseField

Takes a key/value pair and sets the binary values
in a gentity
===============
*/
//void G_ParseField( const char *key, const char *value, GameEntity *ent ) 
void G_ParseField( const char *key, const char *value, SpawnFieldHolder *ent ) 
{
	field_t	*f;
	byte	*b;
	float	v;
	vec3_t	vec;

	for ( f=fields ; f->name ; f++ ) 
	{
		if ( !Q_stricmp(f->name, key) ) 
		{
			// found it
			b = (byte *)ent;

			switch( f->type ) 
			{
			case F_LSTRING:
				*(char **)(b+f->ofs) = G_NewString (value);
				break;
			case F_VECTOR:
				sscanf (value, "%f %f %f", &vec[0], &vec[1], &vec[2]);
				((float *)(b+f->ofs))[0] = vec[0];
				((float *)(b+f->ofs))[1] = vec[1];
				((float *)(b+f->ofs))[2] = vec[2];
				break;
			case F_INT:
				*(int *)(b+f->ofs) = atoi(value);
				break;
			case F_FLOAT:
				*(float *)(b+f->ofs) = atof(value);
				break;
			case F_ANGLEHACK:
				v = atof(value);
				((float *)(b+f->ofs))[0] = 0;
				((float *)(b+f->ofs))[1] = v;
				((float *)(b+f->ofs))[2] = 0;
				break;
			default:
			case F_IGNORE:
				break;
			}
			return;
		}
	}
}




/*
===================
G_SpawnGEntityFromSpawnVars

Spawn an entity and fill in all of the level fields from
level.spawnVars[], then call the class specfic spawn function
===================
*/
void G_SpawnGEntityFromSpawnVars() 
{
	int			i;
//	GameEntity	*ent;
	SpawnFieldHolder sfh;
	memset( &sfh, 0, sizeof(sfh) );

	char		*s, *value, *gametypeName;
	static char *gametypeNames[] = {"ffa", "tournament", "single", "missioneditor", "team", "ctf", "oneflag", "obelisk", "harvester", "teamtournament"};

	// get the next free entity
//	ent = G_Spawn();

	for ( i = 0 ; i < theLevel.numSpawnVars_ ; i++ ) 
	{
//		G_ParseField( theLevel.spawnVars_[i][0], theLevel.spawnVars_[i][1], ent );
		G_ParseField( theLevel.spawnVars_[i][0], theLevel.spawnVars_[i][1], &sfh );
	}

	// check for "notsingle" flag
	if ( g_gametype.integer == GT_SINGLE_PLAYER ) 
	{
		G_SpawnInt( "notsingle", "0", &i );
		if ( i ) 
		{
			//ent->free();
			return;
		}
	}
	// check for "notteam" flag (GT_FFA, GT_TOURNAMENT, GT_SINGLE_PLAYER)
	if ( g_gametype.integer >= GT_TEAM )
	{
		G_SpawnInt( "notteam", "0", &i );
		// MFQ3 special case for flags
		if( (g_gametype.integer != GT_CTF) &&
			(Q_stricmp( sfh.classname_, "team_CTF_redflag" ) == 0 ||
			Q_stricmp( sfh.classname_, "team_CTF_blueflag" ) == 0 ) )
		{
			//ent->free();
			return;
		}
		if ( i ) 
		{
			//ent->free();
			return;
		}
	} 
	else 
	{
		G_SpawnInt( "notfree", "0", &i );
		if ( i )
		{
			//ent->free();
			return;
		}
	}

	if( G_SpawnString( "gametype", NULL, &value ) ) 
	{
		if( g_gametype.integer >= GT_FFA && g_gametype.integer < GT_MAX_GAME_TYPE ) 
		{
			gametypeName = gametypeNames[g_gametype.integer];

			s = strstr( value, gametypeName );
			if( !s ) {
				//ent->free();
				return;
			}
		}
	}

	// move editor origin to pos
	//VectorCopy( ent->s.origin, ent->s.pos.trBase );
	//VectorCopy( ent->s.origin, ent->r.currentOrigin );

	// if we didn't get a classname, don't bother spawning anything
	if ( !G_CallSpawn( &sfh ) ) 
	{
		//ent->free();
	}
}



/*
====================
G_AddSpawnVarToken
====================
*/
char *G_AddSpawnVarToken( const char *string ) 
{
	int		l;
	char	*dest;

	l = strlen( string );
	if ( theLevel.numSpawnVarChars_ + l + 1 > LevelLocals::k_MAX_SPAWN_VARS_CHARS ) 
		Com_Error( ERR_DROP, "G_AddSpawnVarToken: MAX_SPAWN_VARS" );

	dest = theLevel.spawnVarChars_ + theLevel.numSpawnVarChars_;
	memcpy( dest, string, l+1 );

	theLevel.numSpawnVarChars_ += l + 1;

	return dest;
}

/*
====================
G_ParseSpawnVars

Parses a brace bounded set of key / value pairs out of the
level's entity strings into level.spawnVars[]

This does not actually spawn an entity.
====================
*/
bool G_ParseSpawnVars() 
{
	char		keyname[MAX_TOKEN_CHARS];
	char		com_token[MAX_TOKEN_CHARS];

	theLevel.numSpawnVars_ = 0;
	theLevel.numSpawnVarChars_ = 0;

	// parse the opening brace
	if ( !SV_GetEntityToken( com_token, sizeof( com_token ) ) ) 
	{
		// end of spawn string
		return false;
	}
	if ( com_token[0] != '{' ) 
		Com_Error( ERR_DROP, "G_ParseSpawnVars: found %s when expecting {",com_token );

	// go through all the key / value pairs
	while ( 1 )
	{	
		// parse key
		if ( !SV_GetEntityToken( keyname, sizeof( keyname ) ) ) 
			Com_Error( ERR_DROP, "G_ParseSpawnVars: EOF without closing brace" );

		if ( keyname[0] == '}' ) 
			break;
		
		// parse value	
		if ( !SV_GetEntityToken( com_token, sizeof( com_token ) ) ) 
			Com_Error( ERR_DROP, "G_ParseSpawnVars: EOF without closing brace" );

		if ( com_token[0] == '}' ) 
			Com_Error( ERR_DROP, "G_ParseSpawnVars: closing brace without data" );
		if ( theLevel.numSpawnVars_ == LevelLocals::k_MAX_SPAWN_VARS ) 
			Com_Error( ERR_DROP, "G_ParseSpawnVars: MAX_SPAWN_VARS" );

		theLevel.spawnVars_[ theLevel.numSpawnVars_ ][0] = G_AddSpawnVarToken( keyname );
		theLevel.spawnVars_[ theLevel.numSpawnVars_ ][1] = G_AddSpawnVarToken( com_token );
		theLevel.numSpawnVars_++;
	}

	return true;
}



/*QUAKED worldspawn (0 0 0) ?

Every map should have exactly one worldspawn.
"music"		music wav file
"gravity"	800 is default gravity
"message"	Text to print during connection process
*/
void SP_worldspawn() 
{
	char	*s;

	G_SpawnString( "classname", "", &s );
	if ( Q_stricmp( s, "worldspawn" ) ) 
		Com_Error( ERR_DROP, "SP_worldspawn: The first entity isn't 'worldspawn'" );

	// make some data visible to connecting client
	SV_SetConfigstring( CS_GAME_VERSION, MF_VERSION );

	SV_SetConfigstring( CS_LEVEL_START_TIME, va("%i", theLevel.startTime_ ) );

	G_SpawnString( "music", "", &s );
	SV_SetConfigstring( CS_MUSIC, s );

	G_SpawnString( "message", "", &s );
	SV_SetConfigstring( CS_MESSAGE, s );				// map specific message

	SV_SetConfigstring( CS_MOTD, g_motd.string );		// message of the day

	G_SpawnString( "gravity", "800", &s );
	Cvar_Set( "g_gravity", s );

	G_SpawnString( "enableDust", "0", &s );
	Cvar_Set( "g_enableDust", s );

	G_SpawnString( "enableBreath", "0", &s );
	Cvar_Set( "g_enableBreath", s );

	// the following is now handled in g_world.cpp
	//theLevel.gameEntities_.at(ENTITYNUM_WORLD)->s.number = ENTITYNUM_WORLD;// g_entities[ENTITYNUM_WORLD].s.number = ENTITYNUM_WORLD;
	//reinterpret_cast<GameEntity*>(theLevel.gameEntities_.at(ENTITYNUM_WORLD))->classname_ = "worldspawn";// g_entities[ENTITYNUM_WORLD].classname = "worldspawn";
	// instead we just need to add the new EntityWorld
	EntityWorld* world = new EntityWorld;
	world->init();
	theLevel.addWorld(world);

	// see if we want a warmup time
	SV_SetConfigstring( CS_WARMUP, "" );
	if ( g_restarted.integer ) 
	{
		Cvar_Set( "g_restarted", "0" );
		theLevel.warmupTime_ = 0;
	} 
	else if ( g_doWarmup.integer ) 
	{ 
		// Turn it on
		theLevel.warmupTime_ = -1;
		SV_SetConfigstring( CS_WARMUP, va("%i", theLevel.warmupTime_) );
		G_LogPrintf( "Warmup:\n" );
	}

}


/*
==============
G_SpawnEntitiesFromString

Parses textual entity definitions out of an entstring and spawns gentities.
==============
*/
void G_SpawnEntitiesFromString()
{
	// allow calls to G_Spawn*()
	theLevel.spawning_ = true;
	theLevel.numSpawnVars_ = 0;

	// the worldspawn is not an actual entity, but it still
	// has a "spawn" function to perform any global setup
	// needed by a level (setting configstrings or cvars, etc)
	if ( !G_ParseSpawnVars() ) 
		Com_Error( ERR_DROP, "SpawnEntities: no entities" );

	SP_worldspawn();

	// parse ents
	while( G_ParseSpawnVars() ) 
		G_SpawnGEntityFromSpawnVars();

	theLevel.spawning_ = false;			// any future calls to G_Spawn*() will be errors
}

