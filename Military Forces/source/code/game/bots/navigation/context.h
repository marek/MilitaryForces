#ifndef __BOT_CONTEXT_H__
#define __BOT_CONTEXT_H__

#include <algorithm>
#include <array>
#include <cmath>

// Context steering: the directions around a vehicle as slots. Each concern writes interest (where it would like to
// go) or danger (where it must not) into the slots, and the resolver picks: a slot whose danger is above its interest
// is masked out, and of the rest the most interesting wins. A new concern is a new writer; the others do not change.
//
// Some dangers cost a trace to find. Those are asked for lazily, in order of interest, so the search stops at the
// first open slot the way a probe that tries the wanted heading first and then either side of it does.
class ContextMap
{
public:
	static constexpr int maxSlots = 16;

	// slots spread evenly round the circle, the first along the given heading
	ContextMap( float baseYaw, int slots ) : baseYaw_( baseYaw ), slots_( std::clamp( slots, 1, maxSlots ) ) {}

	int slots() const { return slots_; }
	float yawOf( int slot ) const { return baseYaw_ + offsetOf( slot ); }
	// the slot's angle off the base heading, from -180 to 180
	float offsetOf( int slot ) const
	{
		const float offset = slot * 360.0f / slots_;
		return offset > 180.0f ? offset - 360.0f : offset;
	}
	// the slot nearest a heading
	int slotOf( float yaw ) const
	{
		float offset = std::fmod( yaw - baseYaw_, 360.0f );
		if( offset < 0 )
			offset += 360.0f;
		return static_cast<int>( std::lround( offset * slots_ / 360.0f ) ) % slots_;
	}

	void addInterest( int slot, float value ) { interest_[slot] = std::max( interest_[slot], value ); }
	void addDanger( int slot, float value ) { danger_[slot] = std::max( danger_[slot], value ); }
	float interest( int slot ) const { return interest_[slot]; }
	float danger( int slot ) const { return danger_[slot]; }

	// interest highest along a heading and falling off with the angle from it, to half at the back. Of two slots
	// equally far round, the one at the greater angle is a hair more interesting, so the order is always the same
	void want( float yaw, float weight )
	{
		constexpr float tieBreak = 0.001f;
		for( int slot = 0; slot < slots_; slot++ )
		{
			float off = std::fmod( yawOf( slot ) - yaw, 360.0f );
			if( off > 180.0f )
				off -= 360.0f;
			else if( off < -180.0f )
				off += 360.0f;
			addInterest( slot, weight * (1.0f - std::fabs( off ) / 360.0f) + (off > 0.0f ? tieBreak : 0.0f) );
		}
	}

	// the most interesting open slot, or -1 when every slot is masked. lazyDanger( slot ) adds the dangers that cost
	// a trace, asked for each slot in turn from the most interesting
	template<typename LazyDanger>
	int resolve( LazyDanger lazyDanger ) const
	{
		std::array<int, maxSlots> order;
		for( int slot = 0; slot < slots_; slot++ )
			order[slot] = slot;
		std::stable_sort( order.begin(), order.begin() + slots_, [this]( int a, int b ) { return interest_[a] > interest_[b]; } );
		for( int i = 0; i < slots_; i++ )
		{
			const int slot = order[i];
			if( interest_[slot] <= 0.0f || danger_[slot] > interest_[slot] )
				continue;
			if( lazyDanger( slot ) > interest_[slot] )
				continue;
			return slot;
		}
		return -1;
	}
	int resolve() const { return resolve( []( int ) { return 0.0f; } ); }

private:
	float							baseYaw_;
	int								slots_;
	std::array<float, maxSlots>		interest_ = {};
	std::array<float, maxSlots>		danger_ = {};
};

#endif
