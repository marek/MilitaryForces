// p_vehicle.h by Bjoern Drabeck

void Get_Vehicle_Names_f( edict_t *ent );
void ShowCrosshair( edict_t *ent );
//void AQ2LoadVars( gclient_t *client );
//void AQ2SaveVars( gclient_t *client);
void vehicle_pain (edict_t *self, edict_t *other, float kick, int damage);
void vehicle_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);
void checkifonrunway( edict_t *ent );
void checkiflanded( edict_t *ent );
void SetVehicleModel( edict_t *ent );
void CountFrags( edict_t *ent );
int SelectVehicleStart (edict_t *ent);
int SelectAQ2SpawnPoint (edict_t *ent, vec3_t origin, vec3_t angles);
void VehicleExplosion (edict_t *vehicle);
edict_t *SelectAQ2DMSpawnPoint (int cat, int tmnr);
void MakeDamage( edict_t *targ );


