// file v_plane.h
// by Bjoern Drabeck


void SetPlaneSpeed (edict_t *vehicle);
void SteerPlane( edict_t *vehicle );
void PlaneSound( edict_t *vehicle );
void plane_touch( edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
extern void SetVehicleModel (edict_t *vehicle );
extern void vehicle_pain (edict_t *self, edict_t *other, float kick, int damage);
extern void vehicle_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);

