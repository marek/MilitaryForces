/*
 * $Id: g_missioneditor.c,v 1.1 2002-06-09 20:09:41 thebjoern Exp $
*/

#include "g_local.h"


void Cmd_ME_Spawn_f( gentity_t* editor ) {
	char	arg1[MAX_STRING_TOKENS];
	int		vehnum;
	gentity_t* ent = 0;
	gitem_t* item = 0;

	trap_Argv( 1, arg1, sizeof( arg1 ) );

	vehnum = atoi(arg1);
	if( vehnum < 0 || vehnum >= bg_numberOfVehicles ) return;

	ent = G_Spawn();

	VectorCopy( editor->s.origin, ent->s.origin );
	VectorCopy(availableVehicles[vehnum].mins, ent->r.mins);
	VectorCopy(availableVehicles[vehnum].maxs, ent->r.maxs);
	G_SetOrigin( ent, ent->s.origin );
	VectorCopy( ent->s.angles, ent->s.apos.trBase );
	VectorCopy( ent->s.origin, ent->s.pos.trBase );
	VectorCopy( ent->s.origin, ent->r.currentOrigin );

	ent->classname = "Mission Entity";
	ent->s.groundEntityNum = ENTITYNUM_NONE;
	ent->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	ent->s.pos.trType = TR_STATIONARY;
	ent->s.apos.trType = TR_STATIONARY;
	ent->s.eType = ET_MISC_VEHICLE;
	ent->s.modelindex = vehnum;
	ent->takedamage = qfalse;
	ent->inuse = qtrue;
	ent->r.contents = CONTENTS_SOLID;
	ent->clipmask = MASK_PLAYERSOLID;

	trap_LinkEntity (ent);

}

void ME_Deselect(gentity_t* editor)
{
	if( editor->selector ) {
		G_FreeEntity( editor->selector );
		editor->selector = 0;
	}
}

void ME_Select(gentity_t* editor, int entnum)
{
	gentity_t* test;

	if( entnum == ENTITYNUM_NONE ||
		entnum == ENTITYNUM_WORLD ) {
		ME_Deselect(editor);
		return;
	}

	test = &g_entities[entnum];
	if( test->s.eType == ET_MISC_VEHICLE ) {
		// deselect
		if( editor->selector && editor->selector->target_ent == test ) {
			ME_Deselect(editor);
			return;
		} else {
			// selector
			if( !editor->selector ) {
				gitem_t* item;
				editor->selector = G_Spawn();

				for ( item=bg_itemlist+1 ; item->classname ; item++ ) {
					if ( !strcmp(item->classname, "ME_Selector") ) {
						G_SpawnItem( editor->selector, item );
					}
				}
				editor->selector->spawnflags |= 1;
				editor->selector->classname = "Selector";
				editor->selector->s.groundEntityNum = ENTITYNUM_NONE;
				editor->selector->r.svFlags = SVF_USE_CURRENT_ORIGIN;
				editor->selector->s.pos.trType = TR_STATIONARY;
				editor->selector->s.apos.trType = TR_STATIONARY;
				editor->selector->s.eType = ET_ITEM;
				editor->selector->s.modelindex = editor->selector->item - bg_itemlist;		// store item number in modelindex
 				editor->selector->takedamage = qfalse;
				editor->selector->inuse = qtrue;
//				editor->selector->r.contents = CONTENTS_SOLID;
//				editor->selector->clipmask = MASK_PLAYERSOLID;
			}
			editor->selector->s.modelindex2 = test->s.modelindex;
			editor->selector->target_ent = test;
			VectorCopy( test->s.origin, editor->selector->s.origin );
			VectorCopy(availableVehicles[test->s.modelindex].mins, editor->selector->r.mins);
			VectorCopy(availableVehicles[test->s.modelindex].maxs, editor->selector->r.maxs);
			G_SetOrigin( editor->selector, editor->selector->s.origin );
			VectorCopy( editor->selector->s.angles, editor->selector->s.apos.trBase );
			VectorCopy( editor->selector->s.origin, editor->selector->s.pos.trBase );
			VectorCopy( editor->selector->s.origin, editor->selector->r.currentOrigin );

			trap_LinkEntity (editor->selector);
		}
	}

}

void ME_Find(gentity_t* ent)
{
	trace_t	trace;
	vec3_t	startpos;
	vec3_t	endpos, forward;

	VectorCopy( ent->s.origin, startpos );
	AngleVectors( ent->client->ps.viewangles, forward, 0, 0 );
	VectorMA( startpos, 2000, forward, endpos );
	trap_Trace (&trace, startpos, NULL, NULL, endpos, ent->s.number, MASK_ALL );
	if( trace.fraction < 1.0f ) {
		ME_Select( ent, trace.entityNum );
	} else {
		ME_Deselect( ent );
	}
	ent->client->ps.timers[TIMER_WEAPON] = level.time + 300;
}

void ME_MoveObject(gentity_t* editor, usercmd_t *ucmd ) 
{
	trace_t	trace;
	vec3_t	startpos;
	vec3_t	endpos, forward;

	if( !editor->selector->last_move_time ) {
		editor->selector->last_move_time = level.time;
		VectorCopy( ucmd->angles, editor->meangles );
		return;
	}

	if( !editor->selector || !editor->selector->target_ent ) {
		return;
	}

	VectorCopy( editor->s.origin, startpos );
	AngleVectors( editor->client->ps.viewangles, forward, 0, 0 );
	VectorMA( startpos, 2000, forward, endpos );
	trap_Trace (&trace, startpos, NULL, NULL, endpos, editor->s.number, MASK_ALL );
	// make sure we are resting over our entity and that our view has changed
	if( trace.fraction < 1.0f && trace.entityNum != ENTITYNUM_NONE 
		&& trace.entityNum != ENTITYNUM_WORLD && 
		&g_entities[trace.entityNum] == editor->selector->target_ent ) {
		// freeze
		editor->client->ps.pm_flags |= PMF_ME_FREEZE;
		// use view change for calculating dragging
		if( editor->meangles[0] != ucmd->angles[0] || editor->meangles[1] != ucmd->angles[1] || 
			editor->meangles[2] != ucmd->angles[2] ) {
			// then drag it along with you
			vec3_t	oldangles, newangles, diff, forward, right, pos;
			int i;
			for( i = 0; i < 3; ++i ) {
				oldangles[i] = SHORT2ANGLE(editor->meangles[i]);
				newangles[i] = SHORT2ANGLE(ucmd->angles[i]);
				diff[i] = newangles[i] - oldangles[i];
			}	
			VectorSet( newangles, 0, editor->client->ps.viewangles[1], 0 );
			AngleVectors( newangles, forward, right, 0 );
			VectorMA( editor->selector->s.pos.trBase, -diff[0], forward, pos );
			VectorMA( pos, -diff[1], right, pos );
			VectorCopy( pos, editor->selector->s.pos.trBase );
			VectorCopy( pos, editor->selector->s.origin );
			VectorCopy( pos, editor->selector->r.currentOrigin );
			editor->selector->s.pos.trTime = level.time;
			editor->selector->s.pos.trDuration = 0;
		}
	}	
	VectorCopy( ucmd->angles, editor->meangles );
	trap_LinkEntity(editor->selector);
}

