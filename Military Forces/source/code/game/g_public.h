/*
 * $Id: g_public.h,v 1.5 2005-11-12 21:26:17 thebjoern Exp $
*/

// Copyright (C) 1999-2000 Id Software, Inc.
//

// g_public.h -- game module information visible to server

#ifndef __G_PUBLIC_H__
#define __G_PUBLIC_H__

#define	GAME_API_VERSION	8

// entity->svFlags
// the server does not know how to interpret most of the values
// in entityStates (level eType), so the game must explicitly flag
// special server behaviors
#define	SVF_NOCLIENT			0x00000001	// don't send entity to clients, even if it has effects

// TTimo
// https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=551
#define SVF_CLIENTMASK			0x00000002

#define SVF_BOT					0x00000008	// set if the entity is a bot
#define	SVF_BROADCAST			0x00000020	// send to all connected clients
#define	SVF_PORTAL				0x00000040	// merge a second pvs at origin2 into snapshots
#define	SVF_USE_CURRENT_ORIGIN	0x00000080	// entity->r.currentOrigin instead of entity->s.origin
											// for link position (missiles and movers)
#define SVF_SINGLECLIENT		0x00000100	// only send to a single client
#define SVF_NOSERVERINFO		0x00000200	// don't send CS_SERVERINFO updates to this client
											// so that it can be updated for ping tools without
											// lagging clients
#define SVF_CAPSULE				0x00000400	// use capsule for collision detection instead of bbox
#define SVF_NOTSINGLECLIENT		0x00000800	// send entity to everyone but one client
											// (entityShared_t->singleClient)

//===============================================================


typedef struct 
{
	entityState_t	s;				// communicated by server to clients

	bool	linked;				// false if not in any good cluster
	int			linkcount;

	int			svFlags;			// SVF_NOCLIENT, SVF_BROADCAST, etc
	int			singleClient;		// only send to this client when SVF_SINGLECLIENT is set

	bool	bmodel;				// if false, assume an explicit mins / maxs bounding box
									// only set by trap_SetBrushModel
	vec3_t		mins, maxs;
	int			contents;			// CONTENTS_TRIGGER, CONTENTS_SOLID, CONTENTS_BODY, etc
									// a non-solid entity should set to 0

	vec3_t		absmin, absmax;		// derived from mins/maxs and origin + rotation

	// currentOrigin will be used for all collision detection and world linking.
	// it will not necessarily be the same as the trajectory evaluation for the current
	// time, because each entity must be moved one at a time after time is advanced
	// to avoid simultanious collision issues
	vec3_t		currentOrigin;
	vec3_t		currentAngles;

	// when a trace call is made and passEntityNum != ENTITYNUM_NONE,
	// an ent will be excluded from testing if:
	// ent->s.number == passEntityNum	(don't interact with self)
	// ent->s.ownerNum = passEntityNum	(don't interact with your own missiles)
	// entity[ent->s.ownerNum].ownerNum = passEntityNum	(don't interact with other missiles from owner)
	int			ownerNum;
} entityShared_t;



// the server looks at a sharedEntity, which is the start of the game's gentity_t structure
typedef struct {
	entityState_t	s;				// communicated by server to clients
	entityShared_t	r;				// shared by both the server system and game
} sharedEntity_t;




#endif // __G_PUBLIC_H__
