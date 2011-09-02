// Header file for flares
// by Bjoern Drabeck



void Flare_Fire (edict_t *ent, vec3_t g_offset, int damage, qboolean hyper, int effect);
void flare_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
void fire_flare (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect, qboolean hyper);





