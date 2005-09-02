#include "cg_local.h"


static void CG_EjectParticle(particleWorld_t * w, particleEmitter_t * e)
{
	int i;
	for(i = 0; i < sizeof(w->particles)/sizeof(w->particles[0]); i++)
	{
		if(w->particles[i].dead)
			break;
	}

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
		w->particles[i].birthtime = cg.time;
	}
}

static int CG_AddSystem(particleWorld_t * w, particleSystem_t * s)
{
	int i;
	for(i = 0; i < sizeof(w->systems)/sizeof(w->systems[0]); i++)
	{
		if(w->systems[i].dead)
			break;
	}
	memcpy(&w->systems[i],s,sizeof(particleEmitter_t));
	w->systems[i].dead = false;
	w->systems[i].birthtime = cg.time;
	w->systems[i].lastthink = 0;

	return i;
}

static void CG_KillSystem(particleWorld_t * w, int index)
{
	w->systems[index].dead = true;
}

static int CG_AddEmitter(particleWorld_t * w, particleEmitter_t * e)
{
	int i;
	for(i = 0; i < sizeof(w->emitters)/sizeof(w->emitters[0]); i++)
	{
		if(w->emitters[i].dead)
			break;
	}
	memcpy(&w->emitters[i],e,sizeof(particleEmitter_t));
	w->emitters[i].dead = false;
	w->emitters[i].birthtime = cg.time;
	w->emitters[i].lastthink = 0;

	return i;
}

static void CG_KillEmitter(particleWorld_t * w, int index)
{
	w->emitters[index].dead = true;
}

// attract particle a to particle b
static void CG_AttractParticles(particle_t * a, particle_t * b)
{
	vec3_t vtemp;
	float ftemp;
	long dtime = (cg.time - a->lastthink)*0.001;
	
	VectorSubtract(b->trBase, a->trBase, vtemp);
	ftemp = VectorNormalize(vtemp);
	ftemp = a->attractionEmitter*ftemp;
	if(ftemp > 0.0001)
		ftemp = 1/ftemp;
	else
		ftemp = 0;
	VectorScale(vtemp, dtime *ftemp,vtemp);
	VectorAdd(vtemp, a->trDelta, a->trDelta);
}

//attract particle a to entity b
static void CG_AttractParticleToEntity(particle_t * a, centity_t * b)
{
	vec3_t vtemp;
	float distance;
	float ftemp;
	long dtime = (cg.time - a->lastthink)*0.001;
	
	VectorSubtract(b->currentState.pos.trBase, a->trBase, vtemp);
	distance = VectorNormalize(vtemp);
	ftemp = a->attractionEmitter*distance;
	if(ftemp > 0.0001)
		ftemp = 1/ftemp;
	else
		ftemp = 0;
	VectorScale(vtemp, dtime*ftemp,vtemp);
	VectorAdd(vtemp, a->trDelta, a->trDelta);
	
	// If close enough, inherhit speed
	if(distance <= 100 && distance > 0)
	{
		VectorCopy(b->currentState.pos.trBase, vtemp);
		VectorNormalize(vtemp);
		VectorScale(vtemp, dtime*VectorLength(b->currentState.pos.trBase)*(1/distance),vtemp);
		VectorAdd(vtemp, a->trDelta, a->trDelta);
	}
}

static void CG_OnParticleDeath(particle_t * p)
{
	
}

static void CG_OnParticleCollision(particle_t * p)
{
	
}

static void CG_ParticleChildSystem(particle_t * p)
{
	
}

static void CG_OnParticleSystemDeath(particleSystem_t * s)
{
	
}

void CG_ParticleEmitterThink(particleWorld_t * w, particleEmitter_t * e)
{
	long		ptime = (cg.time - e->birthtime)*0.001;
	long		dtime = cg.time - e->lastthink;

	// *
	// Emitter Life
	// *
	if(ptime >= e->lifetime)
	{
		e->dead = true;
		return;
	}	
}

void CG_ParticleSystemThink(particleWorld_t * w, particleSystem_t * s)
{
	long		ptime = (cg.time - s->birthtime)*0.001;
	long		dtime = cg.time - s->lastthink;

	// *
	// System Life
	// *
	if(ptime >= s->lifetime)
	{
		CG_OnParticleSystemDeath(s);
		s->dead = true;
		return;
	}	
}

void CG_ParticleThink(particleWorld_t * w, particle_t * p)
{
	trace_t		trace;
	vec3_t		mins, maxs, result;
	long		ptime = (cg.time - p->birthtime)*0.001;
	long		dtime = cg.time - p->lastthink;
	

	// *
	// Particle Life
	// *
	if(ptime >= p->lifetime)
	{
		CG_OnParticleDeath(p);
		p->dead = true;
		return;
	}

	// *
	// Do Particle growing
	// *
	if(ptime >= p->growstart && ptime <= p->growend)
		p->radius += (cg.time - p->lastthink) * p->growrate;
	if(p->radius <= 0) {
		CG_OnParticleDeath(p);
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
		p->alpha = p->alphaDecayEndValue - (p->alphaDecayEndValue - p->alphaDecayStartValue)*(p->alphaDecayEndTime - ptime);
	else if (ptime < p->alphaDecayStartTime) 
		p->alpha = p->alphaDecayStartValue;
	else
		p->alpha = p->alphaDecayEndValue;
	
	
	// *
	// Do particle attraction
	// *
	if(p->attractionEmitter != 0 && p->attractionSystem != 0 && p->attractionWorld != 0)
	{
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
	}
	if(p->attractionEntity != 0)
	{
		for(int i = 0; i < MAX_GENTITIES; i++)
		{
			if(cg_entities[i].currentValid)
				CG_AttractParticleToEntity(p, &cg_entities[i]);
		}
	}


	// *
	// Do particle movement
	// *
	
	if(ptime < p->acceltime)
		VectorMA (p->trBase, (p->accel/2)*dtime*dtime, p->trDelta, result);		// acceleration
	VectorMA( result, dtime, p->trDelta, result );								// basic movement
	result[2] -= 0.5 * p->gravity * dtime * dtime; 								// gravity
	VectorCopy(result, p->trBase);												// copy final result
	

	// *
	// Check particle bouncing
	// *
	
	// Set collision boxes
	VectorSet( mins, -p->radius, -p->radius, -p->radius );
	VectorSet( maxs, p->radius, p->radius, p->radius );
	
	CG_Trace( &trace, p->trBase, mins, maxs, result, -1, CONTENTS_SOLID );
	
	// bounce/die if hit
	if( trace.fraction == 1.0f || p->bounce == 0.0f )
	{
		CG_OnParticleCollision(p);
		if(p->dieOnCollision) {
			CG_OnParticleDeath(p);
			p->dead = true;
			return;
		}
		VectorCopy( result, p->trBase );
	} else {
		//reflect the particle
		VectorMA( p->trDelta, -2.0f * DotProduct( p->trDelta, trace.plane.normal ), trace.plane.normal, p->trDelta );
		VectorScale( p->trDelta, p->bounce * VectorLength(p->trDelta), p->trDelta );
		VectorCopy( trace.endpos, p->trBase );	
	}
	
	// Do work for children (if any)
	CG_ParticleChildSystem(p);

	p->lastthink = cg.time;
}

void CG_RenderParticle(particle_t * p)
{
	refEntity_t		re;
	int				index;
	vec3_t          alight, dlight, lightdir;
	int             i;
	long			t;
	
	memset( &re, 0, sizeof( refEntity_t ) );

	t = p->lifetime - (cg.time - p->birthtime);
	
	for(i = 0; i < 4; re.shaderRGBA[ i++ ] = 0xFF);
	re.reType = RT_SPRITE;
	re.shaderTime = cg.time;
	re.shaderRGBA[ 3 ] = 0xFF * p->alpha;
	re.radius = p->radius;
	re.rotation = p->rotation;
	
	// world lighting
	if(p->reallight)
	{
		trap_R_LightForPoint( p->trBase, alight, dlight, lightdir );
		for( i = 0; i < 3; i++ )
			re.shaderRGBA[ i ] = (int)alight[ i ];
	}
	
	// shader animation
	if(p->shaderFPS == 0) {
		// sync animation time to lifeTime of particle
		index = (int)(t *(p->numShaders + 1));
		
		if( index >= p->numShaders )
			index = p->numShaders - 1;
		
		re.customShader = p->shaders[index];
	} else {
		// looping animation
		index = (int)(p->shaderFPS * t * 0.001) % p->numShaders;
		re.customShader = p->shaders[index];
	}
	
	VectorCopy( p->trBase, re.origin );
	trap_R_AddRefEntityToScene( &re );
}


void CG_ParticleWorldThink(particleWorld_t * w)
{
	int i;

	// Think particles
	for(i = 0; i < sizeof(w->particles)/sizeof(w->particles[0]); i++)
	{
		if(!w->particles[i].dead)
			CG_ParticleThink(w, &w->particles[i]);
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