// file: drone.h by Bjoern Drabeck
// header file for drone.c

void Cmd_Spawn_Drone(edict_t *ent);
void drone_pain (edict_t *self, edict_t *other, float kick, int damage);
void drone_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);
void SteerDrone( edict_t *vehicle );
void drone_touch( edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf );
extern void SetVehicleModel (edict_t *vehicle );
extern edict_t *SelectDeathmatchSpawnPoint (void);
edict_t *finddronestart(int tmnr);
void DroneSidewinderLockOn( edict_t *vehicle );
void SpawnTeamDrone( );






