// header file for V_helo.c
// by Bjoern Drabeck

void SteerHelo( edict_t *vehicle);
void SetHeloSpeed( edict_t *vehicle );
void HeloSound( edict_t *vehicle );
void helo_touch( edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf );
extern void SetVehicleModel (edict_t *vehicle );
extern void vehicle_pain (edict_t *self, edict_t *other, float kick, int damage);
extern void vehicle_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);


