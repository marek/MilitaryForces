// file g_aq2cmds.h
// by Bjoern Drabeck

void Cmd_VehicleSelect_f( edict_t *ent );
void Cmd_VehicleUp_f( edict_t *ent );
void Cmd_VehicleDown_f (edict_t *ent );
void Cmd_KillVehicle_f( edict_t *ent );
void Cmd_Thrustup_f (edict_t *ent );
void Cmd_Thrustdown_f (edict_t *ent );
void Cmd_Speedbrake_f (edict_t *ent );
void Cmd_Gear_f (edict_t *ent);
void Cmd_Flare_f (edict_t *ent);
void Cmd_Coord_f( edict_t *ent );
void Cmd_Roll_f( edict_t *ent );
void Cmd_FixRoll_f (edict_t *ent );
void Cmd_Ripple_f(edict_t *ent);
void Cmd_GPS_f(edict_t *ent);
void Cmd_Autopilot_f(edict_t *ent);
void Cmd_Light_f( edict_t *ent );
void Cmd_Zoom_f( edict_t *ent );
void Cmd_Tower_f( edict_t *ent );
void Cmd_Pickle_f( edict_t *ent );
void Cmd_VehicleInfo_f(edict_t *ent);
void Cmd_Eject_f(edict_t *ent);
void ClearClient( edict_t *ent );
//void Cmd_Autoaim_f(edict_t *ent);
void Cmd_PickleToggle_f(edict_t *ent);
void Cmd_OpenBay_f(edict_t *ent);
void Cmd_Change_f(edict_t *ent);

extern void SteerLQM( edict_t *vehicle);
extern void SteerGround( edict_t *vehicle);
extern void SteerHelo( edict_t *vehicle);
extern void SteerPlane( edict_t *vehicle);
extern void vehicle_pain (edict_t *self, edict_t *other, float kick, int damage);
extern void vehicle_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);
extern void plane_touch( edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
extern void helo_touch( edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
extern void ground_touch( edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);

// number of vehicles defined for game
#define PLANE_FIGHTER 8
#define PLANE_GROUNDATTACK 6
#define PLANE_TRANSPORT 0

#define HELO_ATTACK 2
#define HELO_SCOUT 2
#define HELO_TRANSPORT 0

#define GROUND_MBT 2
#define GROUND_APC 0
#define GROUND_SCOUT 2
#define GROUND_SPECIAL 4

#define INF_PILOT 0
#define INF_TEAM 0
