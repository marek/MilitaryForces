// file e_drone.h by Bjoern Drabeck

void SP_enemy_drone( edict_t *self );
void enemy_plane_fly( edict_t *drone );
void enemy_plane_touch( edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf );
void enemy_plane_pain (edict_t *self, edict_t *other, float kick, int damage);
void enemy_plane_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);



