// file v_lqm.h



void SteerLQM( edict_t *vehicle);
void LQM_touch( edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf );
void LQM_pain (edict_t *self, edict_t *other, float kick, int damage);
void LQM_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);

