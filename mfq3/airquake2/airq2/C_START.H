// file c_start.h
// by Bjoern Drabeck

void Start_Client(edict_t *ent, int bullets, int shells, int sidewinders, int stingers,
		int amraams, int phoenix, int ATGMs, int mavericks, int rockets, int bombs,
	    int lgbs, int flares, int jammer );
extern void SteerPlane( edict_t *vehicle );
extern void SteerLQM( edict_t *vehicle );
extern void SteerHelo( edict_t *vehicle );
extern void SteerGround( edict_t *vehicle );
extern void vehicle_pain (edict_t *self, edict_t *other, float kick, int damage);
extern void vehicle_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);
extern void helo_touch( edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf );
extern void plane_touch( edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
extern void ground_touch( edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);



