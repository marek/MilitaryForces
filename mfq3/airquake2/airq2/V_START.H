// file v_start.h by Bjoern Drabeck
// header file

void Start_F16 (edict_t *ent );
void Start_A10 (edict_t *ent );
void Start_Tornado (edict_t *ent );
void Start_SU25 (edict_t *ent );
void Start_SU24 (edict_t *ent );
void Start_F22 (edict_t *ent );
void Start_F14 (edict_t *ent );
void Start_F4 (edict_t *ent );
void Start_SU37 (edict_t *ent );
void Start_SU34 (edict_t *ent );
void Start_MiG21 (edict_t *ent );
void Start_MiG29 (edict_t *ent );
void Start_MiGMFI (edict_t *ent );
void Start_F117 (edict_t *vehicle );
void Start_T72 (edict_t *ent );
void Start_M1 (edict_t *ent );
void Start_2S6 (edict_t *ent );
void Start_Scorpion(edict_t *ent );
void Start_BRDM2(edict_t *ent );
void Start_SA9 (edict_t *ent );
void Start_LAV25 (edict_t *ent );
void Start_Rapier (edict_t *ent );
void Start_AH64(edict_t *ent );
void Start_MI28( edict_t *vehicle );
void Start_RAH66(edict_t *ent );
void Start_KA50( edict_t *vehicle );
void Start_LQM (edict_t *vehicle );
void SetOnGround( edict_t *vehicle );
extern void SteerPlane( edict_t *vehicle );
extern void vehicle_pain (edict_t *self, edict_t *other, float kick, int damage);
extern void vehicle_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);
extern void SteerGround( edict_t *vehicle );
extern void SteerHelo( edict_t *vehicle);
extern void SteerLQM( edict_t *vehicle);
extern void SteerDrone( edict_t *vehicle );


