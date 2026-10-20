#include "../../g_local.h"
#include "danger.h"
#include "../world/threat.h"

float PathDanger::factor( const Vec3& at ) const
{
	if( team < 0 || caution <= 0.0f )
		return 1.0f;
	vec3_t point;
	at.copyTo( point );
	return 1.0f + weight * caution * ThreatMap::danger( team, point, air );
}

float PathDanger::along( const Vec3& from, const Vec3& to ) const
{
	constexpr float sampleStep = 1000.0f;
	if( team < 0 || caution <= 0.0f )
		return 0.0f;
	const Vec3 way = to - from;
	const float length = way.length();
	float total = 0.0f;
	for( float d = sampleStep; d < length; d += sampleStep )
	{
		vec3_t point;
		(from + way * (d / length)).copyTo( point );
		total = std::max( total, ThreatMap::danger( team, point, air ) );
	}
	return total * caution;
}
