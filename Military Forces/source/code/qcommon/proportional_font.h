#ifndef PROPORTIONAL_FONT_H
#define PROPORTIONAL_FONT_H

#include "../game/q_shared.h"

// The large proportional font used by the loading screen, the banner and a few menu lines.

class ProportionalFont
{
public:
	static void drawBanner( int x, int y, const char* str, int style, vec4_t color );
	static float sizeScale( int style );
	static void drawString( int x, int y, const char* str, int style, vec4_t color );
	static int stringWidth( const char* str );
};

#endif
