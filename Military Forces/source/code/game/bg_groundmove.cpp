/*
 * $Id: bg_groundmove.c,v 1.6 2006-09-24 17:01:13 minkis Exp $
*/

#include "q_shared.h"
#include <algorithm>
#include <cmath>
#include "../qcommon/qfiles.h"
#include "bg_public.h"
#include "bg_local.h"


namespace
{
	// a vehicle moves over the ground at twice the speed its table entry and its instruments give; every vehicle has
	// always done so, and the entries are tuned to it
	constexpr float GROUND_SPEED_SCALE = 2.0f;
	// what pulls a ground vehicle down, in units a second each second
	constexpr float GROUND_GRAVITY = 200.0f;
	// a vehicle with no weight in its table entry is taken for a medium one, in tonnes
	constexpr float DEFAULT_MASS = 20.0f;

	constexpr int MAX_CONTACTS = 8;

	// How a vehicle's weight and running gear make it behave. A light wheeled car rides soft, bounces and slides; a
	// tank sits planted on its tracks, settles at once and holds its line
	struct Handling
	{
		float	mass;			// tonnes
		bool	tracked;
		float	damping;		// of the suspension, 1 is no bounce at all
		float	sag;			// how far it sits down on its springs at rest
		float	travel;			// how much further the springs go before the hull is on the ground
		float	clearance;		// how far above the ground its underside rests
		float	lateralGrip;	// grip across the hull, as a share of its weight
		float	driveGrip;		// grip along the hull, for driving and braking
		float	rollingDrag;	// what slows it when it coasts
		float	climb;			// the steepest slope, in degrees, its engine can hold it on at low speed
		float	standNormal;	// the steepest ground it stands and drives on, as the upward part of that ground's normal
		float	bounce;			// how much of an impact comes back off a wall
		float	settleTime;		// seconds the hull takes to come most of the way round to lie on the ground
	};

	Handling handlingOf( int vehicle )
	{
		const completeVehicleData_t& veh = availableVehicles[vehicle];
		Handling h;
		h.mass = veh.mass > 0 ? veh.mass : DEFAULT_MASS;
		h.tracked = !(veh.caps & HC_WHEELS);
		// from a one tonne buggy to a twenty tonne hull and beyond
		const float heavy = std::clamp( ( h.mass - 1.0f ) / 19.0f, 0.0f, 1.0f );
		// Springs in proportion to the vehicle: a Humvee stands eight units tall and its suspension moves about a
		// quarter of a metre, one unit. A light car sits half a unit down on its springs and has a unit of travel
		// left; a tank's suspension hardly shows. Bounce and air come from the ground, not from long springs
		h.damping = 0.6f + 0.4f * heavy;
		h.sag = 0.6f - 0.3f * heavy;
		h.travel = 1.2f - 0.7f * heavy;
		// Wheels stand on the ground: a wheeled vehicle's underside is the bottom of its wheels. A tracked hull is
		// kept a unit clear, or its tracks cut into uneven ground
		h.clearance = availableVehicles[vehicle].wheelRadius > 0 && !h.tracked ? 0.0f : 1.0f;
		h.lateralGrip = h.tracked ? 3.0f : 1.3f + 0.5f * heavy;
		h.driveGrip = h.tracked ? 1.6f : 1.25f;
		// tracks and their drive train hold a standing tank on a fair slope; wheels roll more freely
		h.rollingDrag = h.tracked ? 0.25f : 0.08f;
		// tracks bite: a tank claws its way up anything short of a wall. Wheels need ground under them
		h.climb = h.tracked ? 80.0f : 40.0f;
		h.standNormal = h.tracked ? 0.15f : 0.5f;
		h.bounce = 0.35f - 0.3f * heavy;
		h.settleTime = 0.12f + 0.16f * heavy;
		return h;
	}

	// The ground under a vehicle, from the points it stands on: wheels at the four corners, or four points along
	// each track
	struct Footing
	{
		int		points = 0;
		int		touching = 0;
		bool	front = false, back = false, left = false, right = false;	// which sides have a point on the ground
		float	slopeForward = 0;	// rise of the ground along the hull, and across it to the right
		float	slopeRight = 0;
		float	height = 0;			// of that ground under the vehicle's middle
		vec3_t	normal = { 0, 0, 1 };
		float	steepness = 1;		// the upward part of the ground's own normal under the points, on average
		// where the points that touch lie, on average, from the vehicle's middle: 0 with all of them down, towards
		// 1 as only those at the front (or the right) are left, towards -1 for the back (or the left)
		float	leanForward = 0, leanRight = 0;
	};

	// what the vehicle ran into during this frame's move
	struct MoveHit
	{
		bool	hit = false;
		vec3_t	normal = { 0, 0, 0 };
		int		entity = ENTITYNUM_NONE;
	};
	MoveHit moveHit;
}

// The box a ground vehicle is moved through the world with: a small core in the upper middle of the hull. A box
// the size of the hull stays level while the hull tilts, so on any slope its leading bottom edge would dig into
// the ground and drag. The ground is the footing's business; what the hull's ends and sides run into is found by
// probes that tilt with it
static void PM_GroundCoreBox( int vehicle, vec3_t mins, vec3_t maxs )
{
	const completeVehicleData_t& veh = availableVehicles[vehicle];
	const float half = 0.3f * std::min( veh.maxs[0] - veh.mins[0], veh.maxs[1] - veh.mins[1] );
	VectorSet( mins, -half, -half, veh.mins[2] + ( veh.maxs[2] - veh.mins[2] ) * 0.55f );
	VectorSet( maxs, half, half, veh.maxs[2] );
}

// The points a vehicle stands on, along the hull and across it to the right: the wheels its model has, where it
// has them, or four points along each track
static int PM_ContactPoints( int vehicle, bool tracked, float* along, float* across )
{
	constexpr float inset = 0.85f;
	const completeVehicleData_t& veh = availableVehicles[vehicle];
	int points = 0;
	if( !tracked && veh.wheelRadius > 0 )
	{
		// a model's second axis points left
		for( int i = 0; i < veh.wheels && i < MAX_CONTACTS; i++ )
		{
			along[points] = veh.wheelPos[i][0]; across[points++] = -veh.wheelPos[i][1];
		}
		return points;
	}
	const float front = veh.maxs[0] * inset, back = veh.mins[0] * inset, side = veh.maxs[1] * inset;
	const int perSide = tracked ? 4 : 2;
	for( int i = 0; i < perSide; i++ )
	{
		const float f = back + ( front - back ) * i / ( perSide - 1 );
		along[points] = f; across[points++] = side;
		along[points] = f; across[points++] = -side;
	}
	return points;
}

// Finds the ground under each point the vehicle stands on and lays one surface through those that touch
static Footing PM_GroundFooting( pmove_t* pm, int vehicle, const Handling& handling )
{
	const bool tracked = handling.tracked;
	// How far above and below the hull's underside the ground is looked for. A wheel rides up over what is no
	// higher than this; anything taller is an obstacle for the hull, not ground to stand on
	// A track climbs what a wheel cannot: the ground is looked for as high as the face of a slope it can still
	// stand on reaches over its length
	constexpr float below = 20.0f;
	const float above = tracked ? 30.0f : 6.0f;
	const completeVehicleData_t& veh = availableVehicles[vehicle];
	const vec3_t yawOnly = { 0, pm->ps->vehicleAngles[YAW], 0 };
	vec3_t forward, right;
	AngleVectors( yawOnly, forward, right, nullptr );

	Footing footing;
	float along[MAX_CONTACTS], across[MAX_CONTACTS];
	footing.points = PM_ContactPoints( vehicle, tracked, along, across );

	// what each point found: ground at all, how high, how steep, and whether the point bears on it
	bool found[MAX_CONTACTS] = {}, bears[MAX_CONTACTS] = {};
	float height[MAX_CONTACTS] = {}, slant[MAX_CONTACTS] = {};
	int bearing = 0, reached = 0;
	// the hull's tilt, as far as it tells where its wheels are: one standing on its nose after a bad landing has
	// its wheels nowhere useful, and is treated as lying no steeper than this so that it finds the ground again
	constexpr float tiltCounted = 75.0f;
	const float pitchNow = DEG2RAD( std::clamp( AngleNormalize180( pm->ps->vehicleAngles[PITCH] ), -tiltCounted, tiltCounted ) );
	const float rollNow = DEG2RAD( std::clamp( AngleNormalize180( pm->ps->vehicleAngles[ROLL] ), -tiltCounted, tiltCounted ) );
	for( int i = 0; i < footing.points; i++ )
	{
		vec3_t start, end;
		VectorMA( pm->ps->origin, along[i], forward, start );
		VectorMA( start, across[i], right, start );
		VectorCopy( start, end );
		// From the top of the hull down. Much of the ground in these maps is a surface with nothing behind it: a
		// look that starts under it, as one from wheel height does on a face that rises ahead, sees nothing at
		// all, and the vehicle drives into the hill. What a wheel or track cannot ride up onto still counts as
		// ground under that point, at the most it can ride up
		// where the tilted hull holds this wheel, and the most it can ride up from there: on a slope the uphill
		// wheels are already higher than the hull's middle, and are measured from where they are
		const float wheelAt = pm->ps->origin[2] + veh.mins[2] - along[i] * tanf( pitchNow ) - across[i] * tanf( rollNow );
		// Never less than it could ride up with the hull level, though: a hull that comes down nose first has its
		// front wheels "held" far below the ground it is landing on, and ground that is right there under it must
		// not be taken for lower than it is - the vehicle would sink into it
		const float ride = std::max( wheelAt, pm->ps->origin[2] + veh.mins[2] ) + above;
		start[2] = std::max( pm->ps->origin[2] + veh.maxs[2], ride + 1.0f );
		end[2] = pm->ps->origin[2] + veh.mins[2] - below;
		trace_t tr;
		pm->trace( &tr, start, vec3_origin, vec3_origin, end, pm->ps->clientNum, MASK_SOLID, false );
		if( tr.fraction >= 1.0f )
			continue;
		// a point that starts inside something, or finds ground higher than it can ride up, stands at that most
		if( tr.startsolid || tr.endpos[2] > ride )
			tr.endpos[2] = ride;
		found[i] = true;
		reached++;
		height[i] = tr.endpos[2];
		slant[i] = tr.startsolid ? 1.0f : tr.plane.normal[2];
		// Does this point bear on that ground, or only hang over it? It bears if the ground is within a wheel's
		// droop of where the tilted hull holds the wheel
		constexpr float droopReach = 2.0f;
		bears[i] = height[i] >= wheelAt - droopReach - handling.sag;
		if( bears[i] )
			bearing++;
	}
	if( !reached )
		return footing;

	// The vehicle stands on the points that bear. A wheel that has gone over an edge and has lower ground somewhere
	// beneath it says nothing about where the vehicle is: counted in, it would lower the ground under the middle
	// and the vehicle would be taken for airborne the moment its nose was over. With no point bearing - coming
	// down onto the ground - all that found ground are used, so the landing is seen coming
	float sumF[2] = { 0, 0 }, sumZF[2] = { 0, 0 }, sumR[2] = { 0, 0 }, sumZR[2] = { 0, 0 }, sumZ = 0, meanF = 0, meanR = 0;
	int countF[2] = { 0, 0 }, countR[2] = { 0, 0 };
	float steep = 0, bearF = 0, bearR = 0;
	for( int i = 0; i < footing.points; i++ )
	{
		if( !found[i] || ( bearing && !bears[i] ) )
			continue;
		footing.touching++;
		steep += slant[i];
		const int f = along[i] > 0 ? 1 : 0, r = across[i] > 0 ? 1 : 0;
		sumF[f] += along[i]; sumZF[f] += height[i]; countF[f]++;
		sumR[r] += across[i]; sumZR[r] += height[i]; countR[r]++;
		sumZ += height[i]; meanF += along[i]; meanR += across[i];
		if( bears[i] )
		{
			bearF += along[i]; bearR += across[i];
		}
	}

	footing.back = countF[0] > 0; footing.front = countF[1] > 0;
	footing.left = countR[0] > 0; footing.right = countR[1] > 0;
	// With only one end or one side bearing there is no rise to measure between them. The ground is then taken to
	// run on as the hull lies - a vehicle with its nose off the top of a ramp is still on the ramp, not on level
	// ground at the height of its back wheels
	footing.slopeForward = -tanf( pitchNow );
	footing.slopeRight = -tanf( rollNow );
	// the rise between the points behind the middle and those ahead of it, and between the two sides
	if( footing.front && footing.back )
		footing.slopeForward = ( sumZF[1] / countF[1] - sumZF[0] / countF[0] ) / ( sumF[1] / countF[1] - sumF[0] / countF[0] );
	if( footing.left && footing.right )
		footing.slopeRight = ( sumZR[1] / countR[1] - sumZR[0] / countR[0] ) / ( sumR[1] / countR[1] - sumR[0] / countR[0] );
	// an edge under one end is not a slope to lie on: the surface through the points is never steeper than a
	// vehicle can stand on
	// (as steep as 79 degrees: a vehicle carried up a face by its speed still lies on that face)
	const float steepest = 5.0f;
	footing.slopeForward = std::clamp( footing.slopeForward, -steepest, steepest );
	footing.slopeRight = std::clamp( footing.slopeRight, -steepest, steepest );
	footing.steepness = steep / footing.touching;
	{
		float reachF = 0.01f, reachR = 0.01f, allF = 0, allR = 0;
		for( int i = 0; i < footing.points; i++ )
		{
			reachF = std::max( reachF, fabsf( along[i] ) ); reachR = std::max( reachR, fabsf( across[i] ) );
			allF += along[i]; allR += across[i];
		}
		// against where they would lie with every point down, so a model with its wheels off centre is not leaning
		if( bearing )
		{
			footing.leanForward = ( bearF / bearing - allF / footing.points ) / reachF;
			footing.leanRight = ( bearR / bearing - allR / footing.points ) / reachR;
		}
	}
	footing.height = sumZ / footing.touching - footing.slopeForward * meanF / footing.touching - footing.slopeRight * meanR / footing.touching;
	for( int i = 0; i < 3; i++ )
		footing.normal[i] = -footing.slopeForward * forward[i] - footing.slopeRight * right[i];
	footing.normal[2] = 1.0f;
	VectorNormalize( footing.normal );
	return footing;
}

/*
===================
groundVehicle_Brakes

===================
*/

void PlayerMove::groundVehicle_Brakes()
{
	if( pm->cmd.serverTime < pm->ps->timers[TIMER_BRAKE] ) return;

	if( pm->ps->ONOFF & OO_SPEEDBRAKE ) {
		pm->ps->ONOFF &= ~OO_SPEEDBRAKE;
	}
	else {
		pm->ps->ONOFF |= OO_SPEEDBRAKE;
	}

	pm->ps->timers[TIMER_BRAKE] = pm->cmd.serverTime + 500;

}


/*
===================
PlayerMove::groundVehicle_FuelFlow

===================
*/

void PlayerMove::groundVehicle_FuelFlow( int throttle )
{
	int fuelflow = (20 - throttle) * 1000;

	if( pm->cmd.serverTime < pm->ps->timers[TIMER_FUEL] + fuelflow ) {
		return;
	}	

	pm->ps->stats[STAT_FUEL]--;
	if( pm->ps->stats[STAT_FUEL] <= 0 ) {
		pm->ps->throttle = 0;
		return;
	}

	pm->ps->timers[TIMER_FUEL] = pm->cmd.serverTime;
}

/*
===================
PlayerMove::groundVehicleAccelerate

What the engine, the brakes and the ground do to the vehicle's velocity over one frame. The vehicle is a weight
with a velocity: gravity always pulls on it, its springs hold it off the ground, and its wheels or tracks drive,
brake and grip only while they touch.
===================
*/
void PlayerMove::groundVehicleAccelerate()
{
	const completeVehicleData_t& veh = availableVehicles[pm->vehicle];
	const Handling handling = handlingOf( pm->vehicle );
	const float dt = pml.frametime;
	const float maxthrottle = veh.maxthrottle, minthrottle = veh.minthrottle;
	float throttle = pm->ps->fixed_throttle;

	// a held key overrides the set throttle
	if( pm->cmd.forwardmove > 0 ) throttle = maxthrottle;
	else if( pm->cmd.forwardmove < 0 ) throttle = maxthrottle - minthrottle;
	pm->ps->throttle = throttle;
	// past the top of the range is reverse
	if( throttle > maxthrottle )
		throttle = -( throttle - maxthrottle );
	if( pm->ps->stats[STAT_FUEL] <= 0 )
		throttle = 0;

	// An amphibious vehicle floats once the water is up to its middle; with only its wheels wet it is still
	// driving on the bottom
	// An amphibious vehicle is afloat when it is in the water and its wheels are not standing on the bottom, or
	// when it has gone right under. Where the bottom comes up to its wheels - a shore, a ramp - it is driving again
	Footing footing = PM_GroundFooting( pm, pm->vehicle, handling );
	const bool onBottom = footing.touching > 0 && footing.height - veh.mins[2] + handling.clearance + handling.sag - pm->ps->origin[2] > 0;
	const bool afloat = (veh.caps & HC_AMPHIBIOUS) && ( pm->waterlevel >= 3 || ( pm->waterlevel >= 1 && !onBottom ) );
	if( afloat )
		footing = Footing{};
	pml.afloat = afloat;
	const vec3_t yawOnly = { 0, pm->ps->vehicleAngles[YAW], 0 };
	vec3_t hullForward;
	AngleVectors( yawOnly, hullForward, nullptr, nullptr );

	// how far the springs are pressed: at rest, by their sag
	const float restHeight = footing.height - veh.mins[2] + handling.clearance;
	// springs press no further than their travel: past that the hull itself is on the ground
	const float travel = handling.travel;
	float pressed = afloat ? handling.sag : restHeight + handling.sag - pm->ps->origin[2];
	const bool touching = footing.touching > 0 && pressed > 0;
	const bool standing = afloat || ( touching && footing.steepness >= handling.standNormal );
	// Ground is ground whether or not it can be driven on: a vehicle on a face too steep for it slides down that
	// face, it does not sink into it
	if( ( standing || touching ) && pressed > handling.sag + travel )
	{
		// The springs are right down: the ground itself holds the hull up. It comes up out of the ground no
		// faster than it is moving, so a face it drives at lifts it instead of throwing it
		const float rise = std::max( 60.0f, VectorLength( pm->ps->velocity ) * 1.5f ) * dt;
		pm->ps->origin[2] += std::min( pressed - ( handling.sag + travel ), rise );
		pressed = handling.sag + travel;
		// What speed it had into the ground is gone, and none of it comes back: ground that steepens under a fast
		// vehicle turns it up the slope, it does not throw it off
		const float sinking = DotProduct( pm->ps->velocity, footing.normal );
		if( sinking < 0 )
			VectorMA( pm->ps->velocity, -sinking, footing.normal, pm->ps->velocity );
	}
	const float support = afloat ? 1.0f : static_cast<float>( footing.touching ) / std::max( footing.points, 1 );

	pm->ground.valid = true;
	pm->ground.touching = footing.touching;
	pm->ground.points = footing.points;
	pm->ground.pressed = pressed;
	pm->ground.steepness = footing.steepness;
	pm->ground.normalZ = footing.normal[2];
	pm->ground.slopeForward = footing.slopeForward;
	pm->ground.slopeRight = footing.slopeRight;
	pm->ground.standing = standing;
	pm->ground.topSpeed = veh.maxspeed * GROUND_SPEED_SCALE / ( pm->waterlevel + 1 );
	if( !afloat && !footing.touching )
	{
		// Nothing under any point. If the ground's surface is just overhead, the vehicle has got under it - such
		// a surface has no inside to stop anything - and is put back on top
		vec3_t over, here;
		VectorCopy( pm->ps->origin, here );
		here[2] += veh.mins[2];
		VectorCopy( pm->ps->origin, over );
		over[2] += ( veh.maxs[2] - veh.mins[2] ) * 6.0f;
		trace_t surface;
		pm->trace( &surface, over, vec3_origin, vec3_origin, here, pm->ps->clientNum, MASK_SOLID, false );
		if( surface.fraction < 1.0f && !surface.startsolid && surface.plane.normal[2] > 0.1f && surface.endpos[2] > here[2] + 1.0f )
		{
			pm->ps->origin[2] = surface.endpos[2] - veh.mins[2] + handling.clearance;
			if( pm->ps->velocity[2] < 0 )
				pm->ps->velocity[2] = 0;
		}
	}
	if( !standing )
	{
		// in the air it is a thrown weight: what it had, and gravity
		pm->ps->velocity[2] -= GROUND_GRAVITY * dt;
		if( touching )
		{
			// Slipping down ground too steep to stand on: it scrapes, and the faster it slides the more it
			// scrapes, so it comes down at a slide and not at the speed of a fall
			constexpr float slideFriction = 0.8f, slideDrag = 1.5f;
			float into = DotProduct( pm->ps->velocity, footing.normal );
			// and it does not move into the face
			if( into < 0 )
			{
				VectorMA( pm->ps->velocity, -into, footing.normal, pm->ps->velocity );
				into = 0;
			}
			Vec3 along;
			VectorMA( pm->ps->velocity, -into, footing.normal, along );
			const float speed = VectorLength( along );
			if( speed > 0.01f )
			{
				const float loss = std::min( speed, ( slideFriction * GROUND_GRAVITY * std::max( footing.normal[2], 0.2f ) + slideDrag * speed ) * dt );
				VectorMA( pm->ps->velocity, -loss / speed, along, pm->ps->velocity );
			}
		}
		pm->ps->ONOFF &= ~OO_LANDED;
		pm->ps->speed = sqrtf( pm->ps->velocity[0] * pm->ps->velocity[0] + pm->ps->velocity[1] * pm->ps->velocity[1] ) / GROUND_SPEED_SCALE * 10;
		groundVehicle_FuelFlow( fabs( throttle ) );
		return;
	}
	if( !(pm->ps->ONOFF & OO_LANDED) )
	{
		// came down: how hard is the speed into the ground, less what its springs take up. Driving down a slope
		// is not falling
		pm->ps->ONOFF |= OO_VAPOR;
		pm->landingSpeed = std::max( 0.0f, -DotProduct( pm->ps->velocity, footing.normal ) - 10.0f * handling.travel );
	}
	pm->ps->ONOFF |= OO_LANDED;

	const float* normal = footing.normal;
	// along the hull and across it, on the ground
	Vec3 forward, side;
	for( int i = 0; i < 3; i++ )
		forward[i] = hullForward[i] - DotProduct( hullForward, normal ) * normal[i];
	VectorNormalize( forward );
	CrossProduct( normal, forward, side );

	float speedUp = DotProduct( pm->ps->velocity, normal );
	float speedForward = DotProduct( pm->ps->velocity, forward );
	float speedSide = DotProduct( pm->ps->velocity, side );
	// what presses the vehicle onto the ground. Tracks dig in: on a steep face they still hold as on the level
	const float weightOnGround = GROUND_GRAVITY * ( handling.tracked ? 1.0f : normal[2] ) * support;

	if( afloat )
	{
		// it rides at the surface, and comes up to it when it has gone under
		constexpr float surfacing = 70.0f, settling = 40.0f;
		speedUp = 0;
		if( pm->waterlevel >= 3 )
			speedUp = surfacing;
		else if( pm->waterlevel == 1 )
		{
			// sitting high, on its wheels: it settles until the water is at its middle
			vec3_t under;
			VectorCopy( pm->ps->origin, under );
			under[2] += veh.mins[2];
			trace_t water;
			pm->trace( &water, pm->ps->origin, vec3_origin, vec3_origin, under, pm->ps->clientNum, MASK_WATER, false );
			const float high = water.fraction * -veh.mins[2];
			if( high > 0.3f )
				speedUp = -std::min( settling, high / dt );
		}
	}
	else
	{
		// springs and shock absorbers, in short steps so a stiff suspension stays calm at any frame rate
		const float stiffness = GROUND_GRAVITY / handling.sag;
		const float shock = 2.0f * handling.damping * sqrtf( stiffness );
		const int steps = std::clamp( static_cast<int>( ceilf( dt / 0.008f ) ), 1, 12 );
		const float step = dt / steps;
		float moved = 0;
		for( int i = 0; i < steps; i++ )
		{
			const float push = pressed > 0 ? std::max( 0.0f, ( stiffness * pressed - shock * speedUp ) * support ) : 0.0f;
			speedUp += ( push - GROUND_GRAVITY * normal[2] ) * step;
			pressed -= speedUp * step;
			moved += speedUp * step;
		}
		// the move this frame goes as far as the springs carried it; the speed it ends with is kept for the next
		pml.groundRise = moved / dt - speedUp;
	}

	// gravity along the slope
	const vec3_t down = { 0, 0, -GROUND_GRAVITY };
	if( !afloat )
	{
		speedForward += DotProduct( down, forward ) * dt;
		speedSide += DotProduct( down, side ) * dt;
	}

	const float topSpeed = veh.maxspeed * GROUND_SPEED_SCALE / ( pm->waterlevel + 1 );
	const float engine = veh.accel * GROUND_SPEED_SCALE;
	const float grip = handling.driveGrip * weightOnGround;
	const auto slow = [&]( float by ) {
		speedForward = speedForward > 0 ? std::max( 0.0f, speedForward - by ) : std::min( 0.0f, speedForward + by );
	};

	if( pm->ps->ONOFF & OO_SPEEDBRAKE )
		slow( grip * dt );								// brakes on: it stops, and holds on any slope they can grip
	else
	{
		const float target = topSpeed * throttle / maxthrottle;
		const float wanted = target - speedForward;
		if( throttle != 0 && wanted * target > 0 )
		{
			// The engine pulls hardest at low speed, which is what takes a vehicle up a hill, and no harder than
			// its wheels or tracks grip
			const float low = std::min( 4.0f, topSpeed / std::max( fabsf( speedForward ), topSpeed * 0.25f ) );
			const float pull = std::min( engine * low, grip ) * dt;
			speedForward += std::clamp( wanted, -pull, pull );
			// On a hill the engine first carries the vehicle's weight up it: all of it on a slope it can climb
			// at low speed, less as it gathers speed, so a steep hill is taken slowly and a gentle one hardly
			// costs anything
			// how hard gravity pulls against the way it is being driven: along the hull when driving forwards,
			// against it in reverse
			const float uphill = -DotProduct( down, forward ) * ( target > 0 ? 1.0f : -1.0f );
			if( uphill > 0 )
			{
				const float power = std::clamp( 1.6f - fabsf( speedForward ) / std::max( topSpeed, 1.0f ), 0.6f, 1.0f );
				const float carried = std::min( { uphill, GROUND_GRAVITY * sinf( DEG2RAD( handling.climb ) ) * power, grip } ) * dt;
				speedForward += target > 0 ? carried : -carried;
			}
		}
		else if( throttle != 0 && speedForward * target < 0 )
			slow( grip * dt );							// driving the other way: braked first
		else
		{
			// Faster than asked for, or coasting: the ground slows it, the engine holds it back, and the further
			// over its speed it runs the harder the driver brakes. A slope can carry it past its top speed, not
			// away with it, and a gentle slope lets it come to rest
			const float over = std::clamp( ( fabsf( speedForward ) - fabsf( target ) ) / std::max( topSpeed, 1.0f ) - ( throttle != 0 ? 0.0f : 0.5f ), 0.0f, 1.0f );
			const float drag = ( handling.rollingDrag * weightOnGround + engine * 0.6f + grip * over ) * dt;
			if( throttle != 0 && fabsf( wanted ) < drag )
				speedForward = target;
			else
				slow( drag );
		}
	}

	// what the wheels or tracks will not let slide sideways
	const float held = handling.lateralGrip * weightOnGround * dt;
	speedSide = speedSide > 0 ? std::max( 0.0f, speedSide - held ) : std::min( 0.0f, speedSide + held );

	for( int i = 0; i < 3; i++ )
		pm->ps->velocity[i] = forward[i] * speedForward + side[i] * speedSide + normal[i] * speedUp;

	pm->ground.speedForward = speedForward;
	// the speed shown is the speed along the hull; backwards is carried as a flag
	pm->ps->speed = fabsf( speedForward ) / GROUND_SPEED_SCALE * 10;
	if( speedForward < -1.0f ) pm->ps->ONOFF |= OO_STALLED;
	else if( speedForward > 1.0f || throttle >= 0 ) pm->ps->ONOFF &= ~OO_STALLED;

	groundVehicle_FuelFlow( fabs( throttle ) );
}

/*
===================
PlayerMove::adjustToTerrain

Lays the hull on the ground it stands on. With nothing under one end it tips that way.
===================
*/
void PlayerMove::adjustToTerrain( ) 
{
	const completeVehicleData_t& veh = availableVehicles[pm->vehicle];
	if( pml.afloat )
	{
		// afloat it rights itself and lies level on the water, whatever way up it went in
		constexpr float rightingTime = 0.5f;
		const float share = 1.0f - expf( -pml.frametime / rightingTime );
		float pitch = AngleNormalize180( pm->ps->vehicleAngles[PITCH] ), roll = AngleNormalize180( pm->ps->vehicleAngles[ROLL] );
		pm->ps->vehicleAngles[PITCH] = pitch - pitch * share;
		pm->ps->vehicleAngles[ROLL] = roll - roll * share;
		pm->ps->vehicleSpin[PITCH] = pm->ps->vehicleSpin[ROLL] = 0;
		return;
	}

	const Handling handling = handlingOf( pm->vehicle );
	const Footing footing = PM_GroundFooting( pm, pm->vehicle, handling );
	float pitch = AngleNormalize180( pm->ps->vehicleAngles[PITCH] ), roll = AngleNormalize180( pm->ps->vehicleAngles[ROLL] );
	// The hull lies on the ground only while its wheels or tracks reach it. Ground that is merely near underneath
	// - the far side of a crest it has just flown off - does not turn it
	constexpr float droop = 1.0f;
	const float above = pm->ps->origin[2] - ( footing.height - veh.mins[2] + handling.clearance + handling.sag );
	// The hull has a spin of its own, about its pitch and its roll, that it keeps: what it was turning at when its
	// wheels left the ground it goes on turning at in the air
	float& pitchSpin = pm->ps->vehicleSpin[PITCH];
	float& rollSpin = pm->ps->vehicleSpin[ROLL];
	const float dt = pml.frametime;
	if( !footing.touching || above > droop )
	{
		// In the air nothing turns it but the spin it has, which dies away, and it does not go on turning past
		// standing on its nose or its side: a flight ends with the vehicle somewhere near the right way up
		constexpr float airDrag = 1.0f, airSpin = 140.0f, airTilt = 60.0f;
		pitchSpin = std::clamp( pitchSpin, -airSpin, airSpin );
		rollSpin = std::clamp( rollSpin, -airSpin, airSpin );
		pitch += pitchSpin * dt;
		roll += rollSpin * dt;
		if( fabsf( pitch ) > airTilt ) { pitch = std::clamp( pitch, -airTilt, airTilt ); pitchSpin = 0; }
		if( fabsf( roll ) > airTilt ) { roll = std::clamp( roll, -airTilt, airTilt ); rollSpin = 0; }
		pitchSpin *= expf( -airDrag * dt );
		rollSpin *= expf( -airDrag * dt );
	}
	else
	{
		// On the ground the hull eases round to it: most of what is left of the way in each settling time, so it
		// rolls over an edge with its weight instead of snapping onto the next face, and that turning is its spin.
		// With some of its points out over nothing its weight turns it towards them: a nose off a ledge drops,
		// a corner off it drops that corner, and the more of it is unsupported the harder
		constexpr float tipping = 1200.0f, leaning = 0.08f;		// degrees a second each second, with one end wholly off
		const float share = 1.0f - expf( -dt / handling.settleTime );
		const float pitchBefore = pitch, rollBefore = roll;
		if( fabsf( footing.leanForward ) < leaning )
		{
			pitch += AngleSubtract( -RAD2DEG( atanf( footing.slopeForward ) ), pitch ) * share;
			pitchSpin = dt > 0 ? ( pitch - pitchBefore ) / dt : 0.0f;
		}
		else
		{
			// supported at the back means the nose goes down, which is pitch going up
			pitchSpin += -footing.leanForward * tipping * dt;
			pitch += pitchSpin * dt;
		}
		if( fabsf( footing.leanRight ) < leaning )
		{
			roll += AngleSubtract( -RAD2DEG( atanf( footing.slopeRight ) ), roll ) * share;
			rollSpin = dt > 0 ? ( roll - rollBefore ) / dt : 0.0f;
		}
		else
		{
			// supported on the left means the right side goes down, which is roll going up
			rollSpin += -footing.leanRight * tipping * dt;
			roll += rollSpin * dt;
		}
	}
	constexpr float fastest = 360.0f;
	pitchSpin = std::clamp( pitchSpin, -fastest, fastest );
	rollSpin = std::clamp( rollSpin, -fastest, fastest );
	pm->ps->vehicleAngles[PITCH] = std::clamp( pitch, -80.0f, 80.0f );
	pm->ps->vehicleAngles[ROLL] = std::clamp( roll, -80.0f, 80.0f );
}

/*
===================
PlayerMove::groundVehicleMove

===================
*/

// how high a ground vehicle can step: a good part of its own height
float VehicleData::stepHeight( int vehicle )
{
	constexpr float least = 6.0f, shareOfHeight = 0.6f;
	return std::max( least, ( availableVehicles[vehicle].maxs[2] - availableVehicles[vehicle].mins[2] ) * shareOfHeight );
}

float VehicleData::climbAngle( int vehicle )
{
	return handlingOf( vehicle ).climb;
}

/*
===================
stepSlideMove_GV

A ground vehicle that is stopped by an edge in the terrain tries again from a little higher up, so it
climbs onto ground it could drive on instead of sliding along the edge. The step is only taken when it
ends on ground that is not too steep and gets the vehicle further than sliding did.
===================
*/
void PlayerMove::stepSlideMove_GV()
{
	Vec3	startOrigin, startVelocity, slideOrigin, slideVelocity, up, down, mins;
	trace_t	trace;

	VectorCopy( pm->ps->origin, startOrigin );
	VectorCopy( pm->ps->velocity, startVelocity );
	if( !slideMove_GV() )
		return;		// nothing in the way

	VectorCopy( pm->ps->origin, slideOrigin );
	VectorCopy( pm->ps->velocity, slideVelocity );

	const float stepSize = VehicleData::stepHeight( pm->vehicle );
	Vec3 maxs;
	PM_GroundCoreBox( pm->vehicle, mins, maxs );

	// room above to lift the vehicle
	up = startOrigin;
	up[2] += stepSize;
	pm->trace( &trace, startOrigin, mins, maxs, up, pm->ps->clientNum, pm->tracemask, false );
	if( trace.allsolid )
		return;

	// the same move from up there
	VectorCopy( trace.endpos, pm->ps->origin );
	VectorCopy( startVelocity, pm->ps->velocity );
	slideMove_GV();

	// and back down onto whatever is underneath
	VectorCopy( pm->ps->origin, down );
	down[2] -= stepSize;
	pm->trace( &trace, pm->ps->origin, mins, maxs, down, pm->ps->clientNum, pm->tracemask, false );
	if( !trace.allsolid )
		VectorCopy( trace.endpos, pm->ps->origin );

	const float slideDistance = ( slideOrigin[0] - startOrigin[0] ) * ( slideOrigin[0] - startOrigin[0] ) + ( slideOrigin[1] - startOrigin[1] ) * ( slideOrigin[1] - startOrigin[1] );
	const float stepDistance = ( pm->ps->origin[0] - startOrigin[0] ) * ( pm->ps->origin[0] - startOrigin[0] ) + ( pm->ps->origin[1] - startOrigin[1] ) * ( pm->ps->origin[1] - startOrigin[1] );
	// A step is taken onto the ground and onto what a map builds its ground from (a runway is an entity of its own,
	// with a lip where it meets the terrain), never onto another player's vehicle
	const bool drivable = trace.fraction < 1.0f && trace.plane.normal[2] >= handlingOf( pm->vehicle ).standNormal && trace.entityNum > MAX_CLIENTS;
	if( !drivable || stepDistance <= slideDistance )
	{
		VectorCopy( slideOrigin, pm->ps->origin );
		VectorCopy( slideVelocity, pm->ps->velocity );
	}
}

void PlayerMove::groundVehicleMove( ) 
{
	// like in the q3tank mod and airq2 the left/right keys turn the hull of the tank
	// while the mouse turns the turret/gunbarrel; and also like in airq2 we will later
	// also introduce a feature to lock the turret/gun to the hull which is more 
	// convenient in some situations. Usually when you turn the body of the tank the
	// turret keeps facing the previous direction, which is very useful in combat, because
	// you can easier keep tracking the enemy while driving zik-zak to avoid enemy hits,
	// sometimes however you might wanna keep the turret facing the current direction
	// (often forward) while driving so you don't have to use both controls. Also another
	// feature that will be introduced later will be the "auto-return" for the turret 
	// which makes it turn back to look in the direction the tank is driving after the
	// user hasn't moved it for 2 secs (or whatever value, might be set in a cvar as well)

    vec3_t		viewdir;
    vec3_t		vehdir;
	Vec3		turretdir;
    vec3_t		diff;
    vec3_t		turnspeed;
	Vec3		forward, up;
	Vec3		temp;
    bool	dead = (pm->ps->stats[STAT_HEALTH] <= 0);
	int			i;
	float		smove = pm->cmd.rightmove;
	float		turret_yaw = pm->ps->turretAngle;
	float		gun_pitch = pm->ps->gunAngle;
    float		topSpeed, currSpeed, turnModifier;
	int anim = 0;

	// brake
	if( pm->cmd.buttons & BUTTON_BRAKE ) {
		groundVehicle_Brakes();
	}

	// engine, brakes, springs, grip and gravity on the velocity
	pml.groundRise = 0;
	PlayerMove::groundVehicleAccelerate();

	// The move. Over the frame the vehicle rises or sinks as far as its springs carried it, which is not what its
	// speed is at the end of the frame
	const Handling handling = handlingOf( pm->vehicle );
	Vec3 before, carried;
	VectorCopy( pm->ps->velocity, before );
	if( pm->ps->ONOFF & OO_LANDED )
		pm->ps->velocity[2] += pml.groundRise;
	VectorCopy( pm->ps->velocity, carried );
	moveHit = MoveHit{};
	Vec3 startOrigin;
	VectorCopy( pm->ps->origin, startOrigin );
	// No wheel or track goes through a face it cannot ride up. Each point the vehicle stands on is carried along
	// this frame's move at axle height; where one meets such a face, the whole vehicle stops short of it. Much of
	// the ground is a surface with nothing behind it, so once a point is through there is nothing to stop the rest
	{
		const completeVehicleData_t& veh = availableVehicles[pm->vehicle];
		float along[MAX_CONTACTS], across[MAX_CONTACTS];
		const int points = PM_ContactPoints( pm->vehicle, handling.tracked, along, across );
		// the points are where the tilted hull has them: on the side of a hill a point placed level would be
		// inside the hill
		vec3_t hull[3];
		AnglesToAxis( pm->ps->vehicleAngles, hull );
		const float axle = veh.mins[2] + std::max( veh.wheelRadius, 2.0f );
		float allowed = 1.0f;
		vec3_t faceNormal = { 0, 0, 0 };
		for( int i = 0; i < points; i++ )
		{
			vec3_t from, to;
			VectorMA( pm->ps->origin, along[i], hull[0], from );
			VectorMA( from, -across[i], hull[1], from );
			VectorMA( from, axle, hull[2], from );
			VectorMA( from, pml.frametime, pm->ps->velocity, to );
			// a little further than the move, so it stops clear of the face and not on it
			vec3_t step;
			VectorSubtract( to, from, step );
			const float length = VectorNormalize( step );
			if( length < 0.01f )
				continue;
			constexpr float clear = 2.0f;
			VectorMA( to, clear, step, to );
			trace_t face;
			pm->trace( &face, from, vec3_origin, vec3_origin, to, pm->ps->clientNum, MASK_SOLID | CONTENTS_PLAYERCLIP, false );
			// a face is in the way only if it faces the point coming at it, and is no overhang seen from underneath
			// A face the vehicle is already on is not in its way, however steep: one carried up a steep face by its
			// speed goes on up it and over the top. A wall is a face that stands up from the surface the hull lies on
			constexpr float standsUp = 0.5f;
			if( face.fraction >= 1.0f || face.startsolid || face.plane.normal[2] >= handling.standNormal || face.plane.normal[2] < -0.1f ||
				DotProduct( step, face.plane.normal ) >= 0 || DotProduct( face.plane.normal, hull[2] ) >= standsUp )
				continue;
			// a face no higher than the vehicle can step is a kerb, not a wall: the same stretch one step up is clear,
			// and the step move takes it from here
			{
				const float stepHeight = VehicleData::stepHeight( pm->vehicle );
				vec3_t fromUp, toUp;
				VectorCopy( from, fromUp );
				VectorCopy( to, toUp );
				fromUp[2] += stepHeight;
				toUp[2] += stepHeight;
				trace_t over;
				pm->trace( &over, fromUp, vec3_origin, vec3_origin, toUp, pm->ps->clientNum, MASK_SOLID | CONTENTS_PLAYERCLIP, false );
				if( over.fraction >= 1.0f && !over.startsolid )
					continue;
			}
			const float share = std::max( 0.0f, ( face.fraction * ( length + clear ) - clear ) / length );
			if( share < allowed )
			{
				allowed = share;
				VectorCopy( face.plane.normal, faceNormal );
			}
		}
		if( allowed < 1.0f )
		{
			// the move goes only that far; what the vehicle does about the face is settled with the other hits
			moveHit.hit = true;
			VectorCopy( faceNormal, moveHit.normal );
			moveHit.entity = ENTITYNUM_WORLD;
			pm->ground.hitBy = 1;
			VectorScale( pm->ps->velocity, allowed, pm->ps->velocity );
			VectorScale( carried, allowed, carried );
		}
	}
	// through the world with the core box; other vehicles and people are met with the whole hull below
	const int fullMask = pm->tracemask;
	pm->tracemask &= ~CONTENTS_BODY;
	stepSlideMove_GV();
	pm->tracemask = fullMask;
	if( pm->ps->ONOFF & OO_LANDED )
	{
		// only what the move changed is kept of the adjustment
		for( int i = 0; i < 3; i++ )
			pm->ps->velocity[i] = before[i] + ( pm->ps->velocity[i] - carried[i] );
	}

	{
		const completeVehicleData_t& veh = availableVehicles[pm->vehicle];
		// another vehicle or a person in the way of the hull stops it there
		if( fullMask & CONTENTS_BODY )
		{
			vec3_t hullMins;
			VectorCopy( veh.mins, hullMins );
			hullMins[2] += ( veh.maxs[2] - veh.mins[2] ) * 0.3f;
			trace_t body;
			pm->trace( &body, startOrigin, hullMins, pm->maxs, pm->ps->origin, pm->ps->clientNum, CONTENTS_BODY, false );
			if( body.fraction < 1.0f && !body.allsolid && body.entityNum != ENTITYNUM_WORLD && body.entityNum != ENTITYNUM_NONE )
			{
				VectorCopy( body.endpos, pm->ps->origin );
				PlayerMove::addTouchEnt_GV( body.entityNum );
				if( !moveHit.hit )
				{
					moveHit.hit = true;
					VectorCopy( body.plane.normal, moveHit.normal );
					moveHit.entity = body.entityNum;
					pm->ground.hitBy = 3;
				}
			}
		}
		// The hull's nose, tail and sides, tilted as the hull is. What one of them reaches that is no ground to
		// drive on is a wall: the vehicle stops with that end against it
		vec3_t axis[3];
		AnglesToAxis( pm->ps->vehicleAngles, axis );
		const float middle = ( veh.mins[2] + veh.maxs[2] ) * 0.5f;
		const struct { int along; float reach; } ends[4] = { { 0, veh.maxs[0] }, { 0, veh.mins[0] }, { 1, veh.maxs[1] }, { 1, veh.mins[1] } };
		for( const auto& end : ends )
		{
			vec3_t from, to;
			VectorMA( pm->ps->origin, middle, axis[2], from );
			VectorMA( from, end.reach, axis[end.along], to );
			trace_t wall;
			pm->trace( &wall, from, vec3_origin, vec3_origin, to, pm->ps->clientNum, MASK_SOLID | CONTENTS_PLAYERCLIP, false );
			vec3_t probing;
			VectorSubtract( to, from, probing );
			// For the hull's ends only what is close to upright is a wall. Where the ground steepens ahead the hull
			// is still coming round to it and its nose reaches into the slope; that slope is the wheels' business
			constexpr float upright = 0.3f;
			if( wall.fraction >= 1.0f || wall.startsolid || wall.plane.normal[2] >= std::min( handling.standNormal, upright ) || wall.plane.normal[2] < -0.1f ||
				DotProduct( probing, wall.plane.normal ) >= 0 || DotProduct( wall.plane.normal, axis[2] ) >= 0.5f )
				continue;
			// what that end has met is no higher than the vehicle steps: a kerb, which the step move takes
			{
				const float lift = VehicleData::stepHeight( pm->vehicle ) + veh.mins[2] + 1.0f - middle;
				vec3_t fromUp, toUp;
				VectorMA( from, lift, axis[2], fromUp );
				VectorMA( to, lift, axis[2], toUp );
				trace_t over;
				pm->trace( &over, fromUp, vec3_origin, vec3_origin, toUp, pm->ps->clientNum, MASK_SOLID | CONTENTS_PLAYERCLIP, false );
				if( lift > 0.0f && over.fraction >= 1.0f && !over.startsolid )
					continue;
			}
			// back off, level, by as far as that end is into it
			vec3_t out = { wall.plane.normal[0], wall.plane.normal[1], 0 };
			if( VectorNormalize( out ) < 0.01f )
				continue;
			const float into = ( 1.0f - wall.fraction ) * fabsf( end.reach );
			vec3_t reach;
			VectorSubtract( to, from, reach );
			VectorNormalize( reach );
			VectorMA( pm->ps->origin, into * fabsf( DotProduct( reach, out ) ), out, pm->ps->origin );
			if( !moveHit.hit )
			{
				moveHit.hit = true;
				VectorCopy( out, moveHit.normal );
				moveHit.entity = ENTITYNUM_WORLD;
				pm->ground.hitBy = 4;
			}
		}
	}

	pm->ground.hit = moveHit.hit;
	pm->ground.hitEntity = moveHit.entity;
	pm->ground.hitNormalZ = moveHit.normal[2];
	// Ran into something it cannot drive over: part of the blow comes back, and it scrapes along. Against
	// another vehicle the weights decide who gives way
	if( moveHit.hit )
	{
		const float into = -DotProduct( before, moveHit.normal );
		if( into > 0 )
		{
			const float other = pm->entityMass && moveHit.entity != ENTITYNUM_WORLD ? pm->entityMass( moveHit.entity ) : 0.0f;
			// the share of its speed into the obstacle that it has afterwards: negative is a bounce back
			const float kept = other > 0 ? ( handling.mass - handling.bounce * other ) / ( handling.mass + other ) : -handling.bounce;
			const float normalNow = DotProduct( pm->ps->velocity, moveHit.normal );
			VectorMA( pm->ps->velocity, -kept * into - normalNow, moveHit.normal, pm->ps->velocity );
			// scraping takes speed along the obstacle in proportion to the blow
			constexpr float scrape = 0.3f;
			Vec3 sliding;
			VectorMA( pm->ps->velocity, -DotProduct( pm->ps->velocity, moveHit.normal ), moveHit.normal, sliding );
			const float along = VectorLength( sliding );
			if( along > 0.01f && other <= 0 )
				VectorMA( pm->ps->velocity, -std::min( along, scrape * into * ( 1.0f + handling.bounce ) ) / along, sliding, pm->ps->velocity );
			if( other > 0 )
			{
				pm->pushEntity = moveHit.entity;
				pm->pushSpeed = into;
				VectorScale( moveHit.normal, -1.0f, pm->pushDir );
			}
		}
	}

	// put the vehicle into the landscape with proper angles
	PlayerMove::adjustToTerrain();

	// get the actual turret angles
	AngleVectors( pm->ps->vehicleAngles, forward, nullptr, up );
	RotatePointAroundVector( temp, up, forward, turret_yaw );
	vectoangles( temp, turretdir );
	turretdir[PITCH] += gun_pitch;

//	Common::printf( "vehd = %.1f %.1f %.1f (%.1f %.1f)\n", pm->ps->vehicleAngles[0], pm->ps->vehicleAngles[1],
//			pm->ps->vehicleAngles[2], turret_yaw, gun_pitch );
//	Common::printf( "view = %.1f %.1f %.1f\n", pm->ps->viewangles[0], pm->ps->viewangles[1],
//			pm->ps->viewangles[2] );
//	Common::printf( "turr = %.1f %.1f %.1f\n", turretdir[0], turretdir[1], turretdir[2] );

	// local vectors
    VectorCopy( pm->ps->vehicleAngles, vehdir );
	if( (pm->cmd.buttons & BUTTON_FREELOOK) && !dead ) { // freelook
		VectorCopy( turretdir, viewdir );
	}
	else { // normal - turret follows camera
		VectorCopy( pm->gunnerAngles ? pm->gunnerAngles : pm->ps->viewangles, viewdir );
	}

	// set to 0<=x<=360
	viewdir[YAW] = AngleMod( viewdir[YAW] );
	vehdir[YAW] = AngleMod( vehdir[YAW] );
	turretdir[YAW] = AngleMod( turretdir[YAW] );
	viewdir[PITCH] = AngleMod( viewdir[PITCH] );
	vehdir[PITCH] = AngleMod( vehdir[PITCH] );
	turretdir[PITCH] = AngleMod( turretdir[PITCH] );


	// get the turnspeeds
	for( i = HULL_YAW; i <= GUN_PITCH; i++ ) {
	    turnspeed[i] = availableVehicles[pm->vehicle].turnspeed[i] * pml.frametime;
	}
	
	// non-track ground-vehicle?
	if( availableVehicles[pm->vehicle].caps & HC_WHEELS )
	{
		// create a turning modifier for normal wheeled vehicles
		topSpeed = availableVehicles[pm->vehicle].maxspeed;
		currSpeed = static_cast<float>(pm->ps->speed)/10;

		// maxium turn is allowed at a percentage of the top speed (0.5f behind 50% of top speed)
		turnModifier = currSpeed/(topSpeed * 0.2f);
		VehicleData::limitFloat( &turnModifier, 0.0f, 1.0f );
	}
	else
	{
		// n/a
		turnModifier = 1.0f;
	}

	// turn the hull
	if( pm->ps->ONOFF & OO_LANDED ) {
		if(pm->ps->ONOFF & OO_STALLED)
			turnModifier *= -1;

		if( smove > 0 ) {
			vehdir[YAW] -= turnspeed[HULL_YAW] * turnModifier;
			anim = 1;
		} else if( smove < 0 ) {
			vehdir[YAW] += turnspeed[HULL_YAW] * turnModifier;
			anim = 2;
		}
		pm->ps->vehicleAnim = anim;
	}

	if( !(pm->cmd.buttons & BUTTON_FREELOOK) ) {
		// get the angle difference
		for( i = PITCH; i <= YAW; i++ ) {
			diff[i] = viewdir[i] - turretdir[i];
			if( diff[i] > 180 ) diff[i] -= 360;
			else if( diff[i] < -180 ) diff[i] += 360;
		}	
		// turn the turret
		if( diff[YAW] < -turnspeed[TURRET_YAW] ) turret_yaw -= turnspeed[TURRET_YAW];
		else if( diff[YAW] > turnspeed[TURRET_YAW] ) turret_yaw += turnspeed[TURRET_YAW];
		else turret_yaw += diff[YAW];
		if( turret_yaw < 0 ) turret_yaw += 360;
		else if( turret_yaw > 360 ) turret_yaw -= 360;

		// turn the gun
		if( diff[PITCH] < -turnspeed[GUN_PITCH] ) gun_pitch -= turnspeed[GUN_PITCH];
		else if( diff[PITCH] > turnspeed[GUN_PITCH] ) gun_pitch += turnspeed[GUN_PITCH];
		else gun_pitch += diff[PITCH];
		if( gun_pitch < 0 ) gun_pitch += 360;
		else if( gun_pitch > 360 ) gun_pitch -= 360;
		if( gun_pitch < 90 && gun_pitch > availableVehicles[pm->vehicle].tailangle ) {
			gun_pitch = availableVehicles[pm->vehicle].tailangle;
		} else if ( gun_pitch > 90 && gun_pitch < availableVehicles[pm->vehicle].gearheight ) {
			gun_pitch = availableVehicles[pm->vehicle].gearheight;
		}
//		Common::printf( "gun %.1f (d %.1f)\n", gun_pitch, diff[PITCH] );
	}

	// return angles
	VectorCopy( vehdir, pm->ps->vehicleAngles );
	pm->ps->turretAngle = turret_yaw;
	pm->ps->gunAngle = gun_pitch;

}


/*
===============
PlayerMove::addTouchEnt_GV
===============
*/
void PlayerMove::addTouchEnt_GV( int entityNum ) {
	int		i;

	if ( entityNum == ENTITYNUM_WORLD ) {
		if( pm->ps->pm_type != PM_VEHICLE )
		    return;
	}
	if ( pm->numtouch == MAXTOUCH ) {
		return;
	}

	// see if it is already added
	for ( i = 0 ; i < pm->numtouch ; i++ ) {
		if ( pm->touchents[ i ] == entityNum ) {
			return;
		}
	}

	// add it
	pm->touchents[pm->numtouch] = entityNum;
	pm->numtouch++;
}


/*
==================
slideMove_GV

Returns true if the velocity was clipped in some way
==================
*/
constexpr int MAX_CLIP_PLANES = 5;
bool PlayerMove::slideMove_GV() {
	int			bumpcount, numbumps;
	Vec3		dir;
	float		d;
	int			numplanes;
	vec3_t		planes[MAX_CLIP_PLANES];
	Vec3		primal_velocity;
	Vec3		clipVelocity;
	int			i, j, k;
	trace_t	trace;
	Vec3		end;
	float		time_left;
	float		into;
	Vec3		endVelocity;
	Vec3		endClipVelocity;
	Vec3		mins;
	
	Vec3 maxs;
	PM_GroundCoreBox( pm->vehicle, mins, maxs );
	
	numbumps = 4;

	VectorCopy (pm->ps->velocity, primal_velocity);

	time_left = pml.frametime;

	// never turn against the ground plane
	if ( pml.groundPlane ) {
		numplanes = 1;
		VectorCopy( pml.groundTrace.plane.normal, planes[0] );
	} else {
		numplanes = 0;
	}

	// never turn against original velocity
	VectorNormalize2( pm->ps->velocity, planes[numplanes] );
	numplanes++;

	for ( bumpcount=0 ; bumpcount < numbumps ; bumpcount++ ) {

		// calculate position we are trying to move to
		VectorMA( pm->ps->origin, time_left, pm->ps->velocity, end );

		// see if we can make it there
		pm->trace ( &trace, 
					pm->ps->origin, 
					mins, 
					maxs, 
					end, 
					pm->ps->clientNum, 
					pm->tracemask,
					false);

		if (trace.allsolid) {

			pm->trace ( &trace, 
					pm->ps->origin, 
					nullptr, 
					nullptr, 
					end, 
					pm->ps->clientNum, 
					pm->tracemask,
					false);

			if( trace.allsolid ) {
				// entity is completely trapped in another solid
				pm->ps->velocity[2] = 0;	// don't build up falling damage, but allow sideways acceleration
				return true;
			}
		}

		if (trace.fraction > 0) {
			// actually covered some distance
			VectorCopy (trace.endpos, pm->ps->origin);
		}

		if (trace.fraction == 1) {
			 break;		// moved the entire distance
		}

		// save entity for contact
		if(	trace.entityNum != ENTITYNUM_WORLD ||
			pm->ps->stats[STAT_HEALTH] <= 0 ||
			(trace.entityNum == ENTITYNUM_WORLD &&
			!(trace.surfaceFlags & SURF_SKY)) ) {
			PlayerMove::addTouchEnt_GV( trace.entityNum );
		}
		// the first thing in the way that is no ground to drive on
		if( !moveHit.hit && trace.plane.normal[2] < handlingOf( pm->vehicle ).standNormal )
		{
			moveHit.hit = true;
			VectorCopy( trace.plane.normal, moveHit.normal );
			moveHit.entity = trace.entityNum;
			pm->ground.hitBy = 2;
		}

		time_left -= time_left * trace.fraction;

		if (numplanes >= MAX_CLIP_PLANES) {
			// this shouldn't really happen
			VectorClear( pm->ps->velocity );
			return true;
		}

		//
		// if this is the same plane we hit before, nudge velocity
		// out along it, which fixes some epsilon issues with
		// non-axial planes
		//
		for ( i = 0 ; i < numplanes ; i++ ) {
			if ( DotProduct( trace.plane.normal, planes[i] ) > 0.99 ) {
				VectorAdd( trace.plane.normal, pm->ps->velocity, pm->ps->velocity );
				break;
			}
		}
		if ( i < numplanes ) {
			continue;
		}
		VectorCopy (trace.plane.normal, planes[numplanes]);
		numplanes++;

		//
		// modify velocity so it parallels all of the clip planes
		//

		// find a plane that it enters
		for ( i = 0 ; i < numplanes ; i++ ) {
			into = DotProduct( pm->ps->velocity, planes[i] );
			if ( into >= 0.1 ) {
				continue;		// move doesn't interact with the plane
			}

			// see how hard we are hitting things
			if ( -into > pml.impactSpeed ) {
				pml.impactSpeed = -into;
			}

			// slide along the plane
			PlayerMove::clipVelocity (pm->ps->velocity, planes[i], clipVelocity, OVERCLIP );

			// slide along the plane
			PlayerMove::clipVelocity (endVelocity, planes[i], endClipVelocity, OVERCLIP );

			// see if there is a second plane that the New move enters
			for ( j = 0 ; j < numplanes ; j++ ) {
				if ( j == i ) {
					continue;
				}
				if ( DotProduct( clipVelocity, planes[j] ) >= 0.1 ) {
					continue;		// move doesn't interact with the plane
				}

				// try clipping the move to the plane
				PlayerMove::clipVelocity( clipVelocity, planes[j], clipVelocity, OVERCLIP );
				PlayerMove::clipVelocity( endClipVelocity, planes[j], endClipVelocity, OVERCLIP );

				// see if it goes back into the first clip plane
				if ( DotProduct( clipVelocity, planes[i] ) >= 0 ) {
					continue;
				}

				// slide the original velocity along the crease
				CrossProduct (planes[i], planes[j], dir);
				VectorNormalize( dir );
				d = DotProduct( dir, pm->ps->velocity );
				clipVelocity = dir * ( d );

				CrossProduct (planes[i], planes[j], dir);
				VectorNormalize( dir );
				d = DotProduct( dir, endVelocity );
				endClipVelocity = dir * ( d );

				// see if there is a third plane the the New move enters
				for ( k = 0 ; k < numplanes ; k++ ) {
					if ( k == i || k == j ) {
						continue;
					}
					if ( DotProduct( clipVelocity, planes[k] ) >= 0.1 ) {
						continue;		// move doesn't interact with the plane
					}

					// stop dead at a tripple plane interaction
					VectorClear( pm->ps->velocity );
					return true;
				}
			}

			// if we have fixed all interactions, try another move
			VectorCopy( clipVelocity, pm->ps->velocity );
			endVelocity = endClipVelocity;
			break;
		}
	}

	// don't change velocity if in a timer (FIXME: is this correct?)
	if ( pm->ps->pm_time ) {
		VectorCopy( primal_velocity, pm->ps->velocity );
	}

	return ( bumpcount != 0 );
}


