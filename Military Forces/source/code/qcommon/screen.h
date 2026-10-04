#ifndef SCREEN_H
#define SCREEN_H

class Screen
{
public:
	static void init ();
	static void updateScreen ();
	static void debugGraph (float value);
	static int getBigStringWidth( const char *str );
	static void adjustFrom640( float *x, float *y, float *w, float *h );
	static void fillRect( float x, float y, float width, float height, const float *color );
	static void drawPic( float x, float y, float width, float height, qhandle_t hShader );
	static void drawNamedPic( float x, float y, float width, float height, const char *picname );
	static void drawBigString( int x, int y, const char *s, float alpha, bool noColorEscape );
	static void drawBigStringColor( int x, int y, const char *s, vec4_t color, bool noColorEscape );
	static void drawSmallStringExt( int x, int y, const char *string, float *setColor, bool forceColor, bool noColorEscape );
	static void drawSmallChar( int x, int y, int ch );
	static void drawCinematic ();
	static void runCinematic ();
	static void stopCinematic ();
};

#endif
