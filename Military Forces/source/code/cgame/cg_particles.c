#include "cg_local.h"


void CG_EjectParticle(particleWorld_t * w, particleEmitter_t * e)
{
	int i, j;
	for(i = 0; i < sizeof(w->particles)/sizeof(w->particles[0]); i++)
	{
		if(w->particles[i].dead)
			break;
	}
	if(i >= MAX_WORLD_PARTICLES)
		return;

	memcpy(&w->particles[i], &e->particleTemplate, sizeof(particle_t));
	
	if(w->systems[e->parentSytem].cent != NULL)
	{
		if(e->inheritVelocity == -1)
		{
			VectorNegate(w->systems[e->parentSytem].cent->currentState.pos.trDelta, w->particles[i].trDelta );
		} 
		else if(e->inheritVelocity == 1)
		{
			VectorCopy(w->systems[e->parentSytem].cent->currentState.pos.trDelta, w->particles[i].trDelta );
		}
		else
		{
			memset(w->particles[i].trDelta, 0, sizeof(w->particles[i].trDelta));
		}
		VectorAdd(e->ejectVelocity, w->particles[i].trDelta, w->particles[i].trDelta);
		VectorCopy(w->systems[e->parentSytem].cent->currentState.pos.trBase, w->particles[i].trBase );
	}
	else
	{
		VectorCopy(e->ejectVelocity, w->particles[i].trDelta);
		VectorCopy(w->systems[e->parentSytem].pos, w->particles[i].trBase);
		
	}
	for(j = 0; j < 3; j++)
	{
		w->particles[i].trDelta[j] += RandomFloat( e->accelVarianceMin[j], e->accelVarianceMax[j] );
	}
	w->particles[i].lastthink = w->particles[i].lastAttractionThink = w->particles[i].birthtime = cg.time;
	w->particles[i].dead = false;
	CG_ParticleThink(w, &w->particles[i]);
	w->numParticles++;
}

int CG_AddParticleSystem(particleWorld_t * w, particleSystem_t * s)
{
	int i;
	for(i = 0; i < sizeof(w->systems)/sizeof(w->systems[0]); i++)
	{
		if(w->systems[i].dead)
			break;
	}
	if(i >= MAX_WORLD_SYSTEMS)
		return -1;

	memcpy(&w->systems[i],s,sizeof(particleEmitter_t));
	w->systems[i].dead = false;
	w->systems[i].birthtime = cg.time;
	w->systems[i].lastthink = 0;

	return i;
}

void CG_KillParticleSystem(particleWorld_t * w, int index)
{
	w->systems[index].dead = true;
}

int CG_AddParticleEmitter(particleWorld_t * w, particleEmitter_t * e)
{
	int i;
	for(i = 0; i < sizeof(w->emitters)/sizeof(w->emitters[0]); i++)
	{
		if(w->emitters[i].dead)
			break;
	}
	if(i >= MAX_WORLD_EMITTERS)
		return -1;
	memcpy(&w->emitters[i],e,sizeof(particleEmitter_t));
	w->emitters[i].dead = false;
	w->emitters[i].birthtime = cg.time;
	w->emitters[i].lastthink = 0;

	return i;
}

void CG_KillParticleEmitter(particleWorld_t * w, int index)
{
	w->emitters[index].dead = true;
}

// attract particle a to particle b
void CG_AttractParticles(particle_t * a, particle_t * b)
{
	vec3_t vtemp;
	float ftemp;
	float dtime = (cg.time - a->lastthink)*0.001;
	
	VectorSubtract(b->trBase, a->trBase, vtemp);
	ftemp = VectorNormalize(vtemp);
	if(ftemp <= 0)
		return;
	if(fabs(ftemp) > 1000.0f)
		ftemp = 0;
	else
	{
		ftemp = 1/ftemp;
		if(ftemp > 1.0f)
			ftemp = 1.0f;
	}
	VectorScale(vtemp, a->attractionEmitter*ftemp, vtemp);
	VectorAdd(vtemp,a->trAttraction,a->trAttraction);
}

void CG_RemoveParticleSystemFromLocalEntity(centity_t * b)
{

}

void CG_RemoveParticleSystemFromCEntity(centity_t * b)
{

}

//attract particle a to entity b
void CG_AttractParticleToEntity(particle_t * a, centity_t * b)
{
	vec3_t vtemp;
	float distance;
	float ftemp;
	float dtime = (cg.time - a->lastthink)*0.001;
	
	VectorSubtract(b->currentState.pos.trBase, a->trBase, vtemp);
	distance = VectorNormalize(vtemp);
	if(distance <= 0)
		return;
	if(fabs(distance) > 1000.0f)
		ftemp = 0;
	else
	{
		ftemp = 1/distance;
		if(ftemp < 1.0f)
			ftemp = 1.0f;
	}
	VectorScale(vtemp, a->attractionEmitter*ftemp, vtemp);
	VectorAdd(vtemp,a->trAttraction,a->trAttraction);
	
	// If close enough, inherhit speed
	if(distance <= 100 && distance > 0)
	{
		VectorCopy(b->currentState.pos.trBase, vtemp);
		VectorNormalize(vtemp);
		VectorScale(vtemp, dtime*VectorLength(b->currentState.pos.trBase)*(1/distance),vtemp);
		VectorAdd(vtemp, a->trDelta, a->trDelta);
	}
}

void CG_OnParticleDeath(particleWorld_t * w, particle_t * p)
{
	w->numParticles--;
}

void CG_OnParticleCollision(particle_t * p)
{
	
}

void CG_ParticleChildSystem(particle_t * p)
{
	
}

void CG_OnParticleSystemDeath(particleSystem_t * s)
{
	
}

void CG_ParticleEmitterThink(particleWorld_t * w, particleEmitter_t * e)
{
	int			i;
	long		ptime = cg.time - e->birthtime;
	float		dtime = (cg.time - e->lastthink)*0.001;

	// *
	// Emitter Life
	// *
	if(e->lifetime != 0 && ptime >= e->lifetime)
	{
		e->dead = true;
		return;
	}	

	// *
	// Eject Particles
	// *
	if(cg.time >= e->lastEjectTime + e->ejectInterval)
	{
		for(i = 0; i < e->numEjectPerInterval; i++)
		{
			CG_EjectParticle(w, e);
		}
		e->lastEjectTime = cg.time;
	}
}

void CG_ParticleSystemThink(particleWorld_t * w, particleSystem_t * s)
{
	long		ptime = cg.time - s->birthtime;
	float		dtime = (cg.time - s->lastthink)*0.001;

	// *
	// System Life
	// *
	if(s->lifetime != 0 && ptime >= s->lifetime)
	{
		CG_OnParticleSystemDeath(s);
		s->dead = true;
		return;
	}	
}

void CG_ParticleThink(particleWorld_t * w, particle_t * p)
{
	trace_t		trace;
	vec3_t		mins, maxs, result, temp;
	long		ptime = cg.time - p->birthtime;
	float		dtime = (cg.time - p->lastthink)*0.001;

	if(cg.time <= p->lastthink + 100)
		return;

	// *
	// Particle Life
	// *
	if(p->lifetime != 0 && ptime >= p->lifetime)
	{
		CG_OnParticleDeath(w, p);
		p->dead = true;
		return;
	}

	// *
	// Do Particle growing
	// *
	if(ptime >= p->growstart && ptime <= p->growend)
		p->radius += dtime * p->growrate;
	if(p->radius <= 0) {
		CG_OnParticleDeath(w, p);
		p->dead = true;
		return;
	}
	
	// *
	// Do Particle rotation
	// *
	p->rotation = p->rotationvel * dtime;
	
	// *
	// Do Particle alpha
	// *
	if(ptime >= p->alphaDecayStartTime && dtime <= p->alphaDecayEndTime)
		p->alpha = p->alphaDecayEndValue - (p->alphaDecayEndValue - p->alphaDecayStartValue)*(p->alphaDecayEndTime - p->alphaDecayStartTime)/(p->alphaDecayEndTime - ptime);
	else if (ptime < p->alphaDecayStartTime) 
		p->alpha = p->alphaDecayStartValue;
	else
		p->alpha = p->alphaDecayEndValue;
	
	// *
	// Do particle attraction
	// *
	if((p->lastAttractionThink + 300 < cg.time) && (p->attractionEmitter != 0 || p->attractionSystem != 0 || p->attractionWorld != 0))
	{
		VectorClear(p->trAttraction);
		for(int i = 0; i < sizeof(w->particles)/sizeof(w->particles[0]); i++)
		{
			if(!w->particles[i].dead)
			{
				if(	p->attractionEmitter != 0 && 
					w->particles[i].parentEmitter == p->parentEmitter)
				{
						CG_AttractParticles(p, &w->particles[i]);
				}
				if(	p->attractionSystem != 0 && 
					w->emitters[w->particles[i].parentEmitter].parentSytem == w->emitters[p->parentEmitter].parentSytem && 
					w->particles[i].parentEmitter != p->parentEmitter)
				{
					CG_AttractParticles(p, &w->particles[i]);
				}
				if( p->attractionWorld != 0 &&
					w->emitters[w->particles[i].parentEmitter].parentSytem != w->emitters[p->parentEmitter].parentSytem && 
					w->particles[i].parentEmitter != p->parentEmitter)
				{
					CG_AttractParticles(p, &w->particles[i]);
				}
			}
		}
		p->lastAttractionThink = cg.time;
	}
	if(p->attractionEntity != 0)
	{
		CG_AttractParticleToEntity(p, &cg_entities[0]);
		/*
		for(int i = 0; i < MAX_GENTITIES; i++)
		{
			if(cg_entities[i].currentValid)
				CG_AttractParticleToEntity(p, &cg_entities[i]);
		}*/
	}

	// *
	// Do particle movement
	// *
	
	if(ptime < p->acceltime)
		VectorMA (p->trBase, (p->accel/2)*dtime*dtime, p->trDelta, result);		// acceleration
	else
		VectorCopy(p->trBase, result);
	p->trDelta[2] -= 0.5 * p->gravity * dtime * dtime; 								// gravity
	VectorAdd(p->trDelta, p->trAttraction, temp);
	VectorMA( result, dtime, temp, result );								// basic movement
											// copy final result

	// *
	// Check particle bouncing
	// *
	
	// Set collision boxes
	VectorSet( mins, -p->radius, -p->radius, -p->radius );
	VectorSet( maxs, p->radius, p->radius, p->radius );
	
	CG_Trace( &trace, p->trBase, mins, maxs, result, ENTITYNUM_NONE, MASK_SOLID );
	
	// bounce/die if hit
	if( trace.fraction == 1.0f)
	{
		VectorCopy(result, p->trBase);	
	} else {
		CG_OnParticleCollision(p);
		if(p->dieOnCollision) {
			CG_OnParticleDeath(w, p);
			p->dead = true;
			return;
		} else {	//reflect the particle
			float	len = VectorNormalize(p->trDelta);
			VectorMA( p->trDelta, -2.0f * DotProduct( p->trDelta, trace.plane.normal ), trace.plane.normal, p->trDelta );
			VectorScale( p->trDelta, p->bounce*len, p->trDelta );
			VectorCopy( trace.endpos, p->trBase );
		}
	}
	
	// Do work for children (if any)
	CG_ParticleChildSystem(p);

	p->lastthink = cg.time;
}

void CG_RenderParticle(particle_t * p)
{
	int				index;
	vec3_t          alight, dlight, lightdir, origin, point;
	int             i, j;
	long			t;
	polyVert_t		verts[4];
	float			dtime = (cg.time - p->lastthink)*0.001;
	vec3_t			rr, ru;
	vec3_t			rotate_ang;
	byte			shaderRGBA[4];

	VectorSubtract(p->trBase, cg.refdef.vieworg, origin);
	
	if(VectorLength(origin) > 5000)
		return;

	t = p->lifetime - (cg.time - p->birthtime);
	
	for(i = 0; i < 4; shaderRGBA[ i++ ] = 0xFF);
	shaderRGBA[ 3 ] = 0xFF * p->alpha;

	for(i = 0; i < 4; i++)
	{
		for(j = 0; j < 4; j++)
			verts[i].modulate[j] = shaderRGBA[j];
	}

	// world lighting
	if(p->reallight)
	{
		trap_R_LightForPoint( p->trBase, alight, dlight, lightdir );
		for( i = 0; i < 3; i++ )
			shaderRGBA[ i ] = (int)alight[ i ];
	}
	
	// shader animation
	if(p->shaderFPS == 0) {
		// sync animation time to lifeTime of particle
		index = (int)(t *(p->numShaders + 1));
		
		if( index >= p->numShaders )
			index = p->numShaders - 1;
	} else {
		// looping animation
		index = (int)(p->shaderFPS * t * 0.001) % p->numShaders;
	}
	
	VectorAdd(p->trAttraction, p->trDelta, origin);
	VectorMA( p->trBase, dtime, origin, origin );
													
	vectoangles( cg.refdef.viewaxis[0], rotate_ang );
	rotate_ang[ROLL] += p->rotation;
	AngleVectors ( rotate_ang, NULL, rr, ru);

	VectorMA (origin, -p->radius*2, ru, point);	
	VectorMA (point, -p->radius*2, rr, point);	
	VectorCopy (point, verts[0].xyz);
	verts[0].st[0] = 0;
	verts[0].st[1] = 0;

	VectorMA (point, 4*p->radius, ru, point);	
	VectorCopy (point, verts[1].xyz);
	verts[1].st[0] = 0;
	verts[1].st[1] = 1;

	VectorMA (point, 4*p->radius, rr, point);	
	VectorCopy (point, verts[2].xyz);
	verts[2].st[0] = 1;
	verts[2].st[1] = 1;

	VectorMA (point, -4*p->radius, ru, point);	
	VectorCopy (point, verts[3].xyz);
	verts[3].st[0] = 1;
	verts[3].st[1] = 0;

	trap_R_AddPolyToScene( p->shaders[0], 4, verts );

}


void CG_ParticleWorldThink(particleWorld_t * w)
{
	int i;

	// Think particles
	for(i = 0; i < sizeof(w->particles)/sizeof(w->particles[0]); i++)
	{
		if(!w->particles[i].dead)
		{
			CG_ParticleThink(w, &w->particles[i]);
		}
	}

	// Think systems
	for(i = 0; i < sizeof(w->systems)/sizeof(w->systems[0]); i++)
	{
		if(!w->systems[i].dead)
			CG_ParticleSystemThink(w, &w->systems[i]);
	}

	// Think emitters
	for(i = 0; i < sizeof(w->emitters)/sizeof(w->emitters[0]); i++)
	{
		if(!w->emitters[i].dead )
		{
			if(w->systems[w->emitters[i].parentSytem].dead)
				w->emitters[i].dead = true;
			else
				CG_ParticleEmitterThink(w, &w->emitters[i]);
		}
	}
}
void CG_ParticleWorldRender(particleWorld_t * w)
{
	int i, j;

	// Render particles
	for(i = 0, j = 0; i < sizeof(w->particles)/sizeof(w->particles[0]) && j < w->numParticles; i++)
	{
		if(!w->particles[i].dead)
		{
			CG_RenderParticle(&w->particles[i]);
			j++;
		}
	}
}

void CG_ParticleWorldInit(particleWorld_t * w)
{
int i;

	// Think particles
	for(i = 0; i < sizeof(w->particles)/sizeof(w->particles[0]); i++)
	{
		w->particles[i].dead = true;
	}

	// Think systems
	for(i = 0; i < sizeof(w->systems)/sizeof(w->systems[0]); i++)
	{
		w->systems[i].dead = true;
	}

	// Think emitters
	for(i = 0; i < sizeof(w->emitters)/sizeof(w->emitters[0]); i++)
	{
		w->emitters[i].dead = true;
	}
}

