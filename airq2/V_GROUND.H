// file v_ground.h
// by Bjoern Drabeck


#define DEG2RAD( a ) ( a * 180.0F ) / M_PI
#define RAD2DEG( a ) ( a * 180.0F ) / M_PI


void SteerGround( edict_t *vehicle );
void SetGroundSpeed( edict_t *vehicle );
void GroundSound( edict_t *vehicle );
void ground_touch( edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf );
extern void SetVehicleModel (edict_t *vehicle );
extern void vehicle_pain (edict_t *self, edict_t *other, float kick, int damage);
extern void vehicle_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);

