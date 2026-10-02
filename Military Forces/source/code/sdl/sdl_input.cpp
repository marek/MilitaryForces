/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include <SDL.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "../client/client.h"
#include "../sys/sys_local.h"

#define CTRL(a) ((a)-'a'+1)

extern SDL_Window *SDL_window;

static cvar_t *in_keyboardDebug     = NULL;

static SDL_Joystick *stick = NULL;

static bool mouseAvailable = false;
static bool mouseActive = false;

static int vidRestartTime = 0;

static cvar_t *in_mouse             = NULL;
static cvar_t *in_nograb;

static cvar_t *in_joystick          = NULL;
static cvar_t *in_joystickNo        = NULL;
static cvar_t *in_joyBallScale      = NULL;
static cvar_t *in_debugJoystick     = NULL;
static cvar_t *joy_threshold        = NULL;

static int in_eventTime = 0;

/*
===============
IN_PrintKey
===============
*/
static void IN_PrintKey( const SDL_Keysym *keysym, int key, bool down )
{
	if( down )
		Com_Printf( "+ " );
	else
		Com_Printf( "  " );

	Com_Printf( "Scancode: 0x%02x(%s) Sym: 0x%02x(%s)",
			keysym->scancode, SDL_GetScancodeName( keysym->scancode ),
			keysym->sym, SDL_GetKeyName( keysym->sym ) );

	Com_Printf( " Q:0x%02x(%s)\n", key, Key_KeynumToString( key ) );
}

/*
===============
IN_TranslateSDLToQ3Key
===============
*/
static int IN_TranslateSDLToQ3Key( SDL_Keysym *keysym, bool down )
{
	int key = 0;

	if( keysym->scancode >= SDL_SCANCODE_1 && keysym->scancode <= SDL_SCANCODE_0 )
	{
		// Always map the number keys as such even if they actually map
		// to other characters (eg, "1" is "&" on an AZERTY keyboard).
		// This is required for SDL before 2.0.6, except on Windows
		// which already had this behavior.
		if( keysym->scancode == SDL_SCANCODE_0 )
			key = '0';
		else
			key = '1' + keysym->scancode - SDL_SCANCODE_1;
	}
	else if( keysym->scancode == SDL_SCANCODE_GRAVE )
	{
		key = '`';
	}
	else if( keysym->sym >= SDLK_SPACE && keysym->sym < SDLK_DELETE )
	{
		// These happen to match the ASCII chars
		key = (int)keysym->sym;
	}
	else
	{
		switch( keysym->sym )
		{
			case SDLK_PAGEUP:       key = K_PGUP;          break;
			case SDLK_KP_9:         key = K_KP_PGUP;       break;
			case SDLK_PAGEDOWN:     key = K_PGDN;          break;
			case SDLK_KP_3:         key = K_KP_PGDN;       break;
			case SDLK_KP_7:         key = K_KP_HOME;       break;
			case SDLK_HOME:         key = K_HOME;          break;
			case SDLK_KP_1:         key = K_KP_END;        break;
			case SDLK_END:          key = K_END;           break;
			case SDLK_KP_4:         key = K_KP_LEFTARROW;  break;
			case SDLK_LEFT:         key = K_LEFTARROW;     break;
			case SDLK_KP_6:         key = K_KP_RIGHTARROW; break;
			case SDLK_RIGHT:        key = K_RIGHTARROW;    break;
			case SDLK_KP_2:         key = K_KP_DOWNARROW;  break;
			case SDLK_DOWN:         key = K_DOWNARROW;     break;
			case SDLK_KP_8:         key = K_KP_UPARROW;    break;
			case SDLK_UP:           key = K_UPARROW;       break;
			case SDLK_ESCAPE:       key = K_ESCAPE;        break;
			case SDLK_KP_ENTER:     key = K_KP_ENTER;      break;
			case SDLK_RETURN:       key = K_ENTER;         break;
			case SDLK_TAB:          key = K_TAB;           break;
			case SDLK_F1:           key = K_F1;            break;
			case SDLK_F2:           key = K_F2;            break;
			case SDLK_F3:           key = K_F3;            break;
			case SDLK_F4:           key = K_F4;            break;
			case SDLK_F5:           key = K_F5;            break;
			case SDLK_F6:           key = K_F6;            break;
			case SDLK_F7:           key = K_F7;            break;
			case SDLK_F8:           key = K_F8;            break;
			case SDLK_F9:           key = K_F9;            break;
			case SDLK_F10:          key = K_F10;           break;
			case SDLK_F11:          key = K_F11;           break;
			case SDLK_F12:          key = K_F12;           break;
			case SDLK_F13:          key = K_F13;           break;
			case SDLK_F14:          key = K_F14;           break;
			case SDLK_F15:          key = K_F15;           break;

			case SDLK_BACKSPACE:    key = K_BACKSPACE;     break;
			case SDLK_KP_PERIOD:    key = K_KP_DEL;        break;
			case SDLK_DELETE:       key = K_DEL;           break;
			case SDLK_PAUSE:        key = K_PAUSE;         break;

			case SDLK_LSHIFT:
			case SDLK_RSHIFT:       key = K_SHIFT;         break;

			case SDLK_LCTRL:
			case SDLK_RCTRL:        key = K_CTRL;          break;

			case SDLK_RGUI:
			case SDLK_LGUI:         key = K_COMMAND;       break;

			case SDLK_RALT:
			case SDLK_LALT:         key = K_ALT;           break;

			case SDLK_KP_5:         key = K_KP_5;          break;
			case SDLK_INSERT:       key = K_INS;           break;
			case SDLK_KP_0:         key = K_KP_INS;        break;
			case SDLK_KP_MULTIPLY:  key = K_KP_STAR;       break;
			case SDLK_KP_PLUS:      key = K_KP_PLUS;       break;
			case SDLK_KP_MINUS:     key = K_KP_MINUS;      break;
			case SDLK_KP_DIVIDE:    key = K_KP_SLASH;      break;
			case SDLK_KP_EQUALS:    key = K_KP_EQUALS;     break;

			case SDLK_POWER:        key = K_POWER;         break;
			case SDLK_NUMLOCKCLEAR: key = K_KP_NUMLOCK;    break;
			case SDLK_CAPSLOCK:     key = K_CAPSLOCK;      break;

			default:
				break;
		}
	}

	if( in_keyboardDebug->integer )
		IN_PrintKey( keysym, key, down );

	return key;
}

/*
===============
IN_GobbleMotionEvents
===============
*/
static void IN_GobbleMotionEvents( void )
{
	SDL_Event dummy[ 1 ];
	int val = 0;

	// Gobble any mouse motion events
	SDL_PumpEvents( );
	while( ( val = SDL_PeepEvents( dummy, 1, SDL_GETEVENT,
		SDL_MOUSEMOTION, SDL_MOUSEMOTION ) ) > 0 ) { }

	if ( val < 0 )
		Com_Printf( "IN_GobbleMotionEvents failed: %s\n", SDL_GetError( ) );
}

/*
===============
IN_ActivateMouse
===============
*/
static void IN_ActivateMouse( bool isFullscreen )
{
	if (!mouseAvailable || !SDL_WasInit( SDL_INIT_VIDEO ) || !SDL_window )
		return;

	if( !mouseActive )
	{
		SDL_SetRelativeMouseMode( SDL_TRUE );
		SDL_SetWindowGrab( SDL_window, SDL_TRUE );

		IN_GobbleMotionEvents( );
	}

	// in_nograb makes no sense in fullscreen mode
	if( !isFullscreen )
	{
		if( in_nograb->modified || !mouseActive )
		{
			if( in_nograb->integer ) {
				SDL_SetRelativeMouseMode( SDL_FALSE );
				SDL_SetWindowGrab( SDL_window, SDL_FALSE );
			} else {
				SDL_SetRelativeMouseMode( SDL_TRUE );
				SDL_SetWindowGrab( SDL_window, SDL_TRUE );
			}

			in_nograb->modified = false;
		}
	}

	mouseActive = true;
}

/*
===============
IN_DeactivateMouse
===============
*/
static void IN_DeactivateMouse( bool isFullscreen )
{
	if( !SDL_WasInit( SDL_INIT_VIDEO ) || !SDL_window )
		return;

	// Always show the cursor when the mouse is disabled,
	// but not when fullscreen
	if( !isFullscreen )
		SDL_ShowCursor( SDL_TRUE );

	if( !mouseAvailable )
		return;

	if( mouseActive )
	{
		IN_GobbleMotionEvents( );

		SDL_SetWindowGrab( SDL_window, SDL_FALSE );
		SDL_SetRelativeMouseMode( SDL_FALSE );

		// Don't warp the mouse unless the cursor is within the window
		if( SDL_GetWindowFlags( SDL_window ) & SDL_WINDOW_MOUSE_FOCUS )
			SDL_WarpMouseInWindow( SDL_window, cls.glconfig.vidWidth / 2, cls.glconfig.vidHeight / 2 );

		mouseActive = false;
	}
}

// We translate axes movement into keypresses
static int joyDirectionKeys[16] = {
	K_LEFTARROW, K_RIGHTARROW,
	K_UPARROW, K_DOWNARROW,
	K_JOY16, K_JOY17,
	K_JOY18, K_JOY19,
	K_JOY20, K_JOY21,
	K_JOY22, K_JOY23,

	K_JOY24, K_JOY25,
	K_JOY26, K_JOY27
};

static struct
{
	bool buttons[32];
	unsigned int oldpovstate;
} stick_state;

/*
===============
IN_InitJoystick
===============
*/
static void IN_InitJoystick( void )
{
	int i = 0;
	int total = 0;
	char buf[16384] = "";

	if (stick != NULL)
		SDL_JoystickClose(stick);

	stick = NULL;
	memset(&stick_state, '\0', sizeof (stick_state));

	if (!SDL_WasInit(SDL_INIT_JOYSTICK))
	{
		Com_DPrintf("Calling SDL_Init(SDL_INIT_JOYSTICK)...\n");
		if (SDL_Init(SDL_INIT_JOYSTICK) != 0)
		{
			Com_DPrintf("SDL_Init(SDL_INIT_JOYSTICK) failed: %s\n", SDL_GetError());
			return;
		}
		Com_DPrintf("SDL_Init(SDL_INIT_JOYSTICK) worked\n");
	}

	total = SDL_NumJoysticks();
	Com_DPrintf("%d possible joysticks\n", total);

	// Print list and build cvar to allow ui to select joystick.
	for (i = 0; i < total; i++)
	{
		Q_strcat(buf, sizeof(buf), SDL_JoystickNameForIndex(i));
		Q_strcat(buf, sizeof(buf), "\n");
	}

	Cvar_Get( "in_availableJoysticks", "", CVAR_ROM );

	// Update cvar on in_restart or controller add/remove.
	Cvar_Set( "in_availableJoysticks", buf );

	if( !in_joystick->integer ) {
		Com_DPrintf( "Joystick is not active.\n" );
		SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
		return;
	}

	in_joystickNo = Cvar_Get( "in_joystickNo", "0", CVAR_ARCHIVE );
	if( in_joystickNo->integer < 0 || in_joystickNo->integer >= total )
		Cvar_Set( "in_joystickNo", "0" );

	stick = SDL_JoystickOpen( in_joystickNo->integer );

	if (stick == NULL) {
		Com_DPrintf( "No joystick opened: %s\n", SDL_GetError() );
		return;
	}

	Com_DPrintf( "Joystick %d opened\n", in_joystickNo->integer );
	Com_DPrintf( "Name:       %s\n", SDL_JoystickNameForIndex(in_joystickNo->integer) );
	Com_DPrintf( "Axes:       %d\n", SDL_JoystickNumAxes(stick) );
	Com_DPrintf( "Hats:       %d\n", SDL_JoystickNumHats(stick) );
	Com_DPrintf( "Buttons:    %d\n", SDL_JoystickNumButtons(stick) );
	Com_DPrintf( "Balls:      %d\n", SDL_JoystickNumBalls(stick) );

	SDL_JoystickEventState(SDL_QUERY);
}

/*
===============
IN_ShutdownJoystick
===============
*/
static void IN_ShutdownJoystick( void )
{
	if ( !SDL_WasInit( SDL_INIT_JOYSTICK ) )
		return;

	if (stick)
	{
		SDL_JoystickClose(stick);
		stick = NULL;
	}

	SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
}

/*
===============
IN_JoyMove
===============
*/
static void IN_JoyMove( void )
{
	unsigned int povstate = 0;
	int total = 0;
	int i = 0;

	if (!stick)
		return;

	SDL_JoystickUpdate();

	// update the ball state.
	total = SDL_JoystickNumBalls(stick);
	if (total > 0)
	{
		int balldx = 0;
		int balldy = 0;
		for (i = 0; i < total; i++)
		{
			int dx = 0;
			int dy = 0;
			SDL_JoystickGetBall(stick, i, &dx, &dy);
			balldx += dx;
			balldy += dy;
		}
		balldx *= in_joyBallScale->value;
		balldy *= in_joyBallScale->value;
		if (balldx || balldy)
		{
			Sys_QueEvent( in_eventTime, SE_MOUSE, balldx, balldy, 0, NULL );
		}
	}

	// now query the stick buttons...
	total = SDL_JoystickNumButtons(stick);
	if (total > 0)
	{
		if (total > (int)( sizeof( stick_state.buttons ) / sizeof( stick_state.buttons[0] ) ))
			total = sizeof( stick_state.buttons ) / sizeof( stick_state.buttons[0] );
		for (i = 0; i < total; i++)
		{
			bool pressed = (SDL_JoystickGetButton(stick, i) != 0);
			if (pressed != stick_state.buttons[i])
			{
				Sys_QueEvent( in_eventTime, SE_KEY, K_JOY1 + i, pressed, 0, NULL );
				stick_state.buttons[i] = pressed;
			}
		}
	}

	// convert main joystick motion into 6 direction button bits
	total = SDL_JoystickNumAxes(stick);
	for (i = 0; i < total && i < 4; i++)
	{
		float f = ( (float) SDL_JoystickGetAxis(stick, i) ) / 32767.0f;

		if( in_debugJoystick->integer )
			Com_Printf( "axis %d: %5.2f\n", i, f );

		if( f < -joy_threshold->value ) {
			povstate |= (1<<(i*2));
		} else if( f > joy_threshold->value ) {
			povstate |= (1<<(i*2+1));
		}
	}

	// convert POV information from a direction into 4 button bits
	if (SDL_JoystickNumHats(stick) > 0)
	{
		Uint8 hat = SDL_JoystickGetHat(stick, 0);

		if (hat & SDL_HAT_UP)
			povstate |= 1<<12;
		if (hat & SDL_HAT_DOWN)
			povstate |= 1<<13;
		if (hat & SDL_HAT_RIGHT)
			povstate |= 1<<14;
		if (hat & SDL_HAT_LEFT)
			povstate |= 1<<15;
	}

	// determine which bits have changed and key an auxillary event for each change
	for (i = 0; i < 16; i++) {
		if ( (povstate & (1<<i)) && !(stick_state.oldpovstate & (1<<i)) ) {
			Sys_QueEvent( in_eventTime, SE_KEY, joyDirectionKeys[i], true, 0, NULL );
		}

		if ( !(povstate & (1<<i)) && (stick_state.oldpovstate & (1<<i)) ) {
			Sys_QueEvent( in_eventTime, SE_KEY, joyDirectionKeys[i], false, 0, NULL );
		}
	}
	stick_state.oldpovstate = povstate;
}

/*
===============
IN_ProcessEvents
===============
*/
static void IN_ProcessEvents( void )
{
	SDL_Event e;
	int key = 0;

	if( !SDL_WasInit( SDL_INIT_VIDEO ) )
			return;

	while( SDL_PollEvent( &e ) )
	{
		switch( e.type )
		{
			case SDL_KEYDOWN:
				if ( e.key.repeat && cls.keyCatchers == 0 )
					break;

				if( ( key = IN_TranslateSDLToQ3Key( &e.key.keysym, true ) ) )
					Sys_QueEvent( in_eventTime, SE_KEY, key, true, 0, NULL );

				if( key == K_BACKSPACE )
					Sys_QueEvent( in_eventTime, SE_CHAR, CTRL('h'), 0, 0, NULL );
				else if( keys[K_CTRL].down && key >= 'a' && key <= 'z' )
					Sys_QueEvent( in_eventTime, SE_CHAR, CTRL(key), 0, 0, NULL );
				break;

			case SDL_KEYUP:
				if( ( key = IN_TranslateSDLToQ3Key( &e.key.keysym, false ) ) )
					Sys_QueEvent( in_eventTime, SE_KEY, key, false, 0, NULL );
				break;

			case SDL_TEXTINPUT:
				{
					char *c = e.text.text;

					while( *c )
					{
						int ch = 0;

						if( ( *c & 0x80 ) == 0 )
							ch = *c++;
						else
						{
							c++;
							while( ( *c & 0xC0 ) == 0x80 )
								c++;
						}

						if( ch != 0 )
							Sys_QueEvent( in_eventTime, SE_CHAR, ch, 0, 0, NULL );
					}
				}
				break;

			case SDL_MOUSEMOTION:
				if( mouseActive )
				{
					if( !e.motion.xrel && !e.motion.yrel )
						break;
					Sys_QueEvent( in_eventTime, SE_MOUSE, e.motion.xrel, e.motion.yrel, 0, NULL );
				}
				break;

			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				{
					int b;
					switch( e.button.button )
					{
						case SDL_BUTTON_LEFT:   b = K_MOUSE1;     break;
						case SDL_BUTTON_MIDDLE: b = K_MOUSE3;     break;
						case SDL_BUTTON_RIGHT:  b = K_MOUSE2;     break;
						case SDL_BUTTON_X1:     b = K_MOUSE4;     break;
						case SDL_BUTTON_X2:     b = K_MOUSE5;     break;
						default:                b = K_AUX1 + ( e.button.button - SDL_BUTTON_X2 + 1 ) % 16; break;
					}
					Sys_QueEvent( in_eventTime, SE_KEY, b,
						( e.type == SDL_MOUSEBUTTONDOWN ? true : false ), 0, NULL );
				}
				break;

			case SDL_MOUSEWHEEL:
				if( e.wheel.y > 0 )
				{
					Sys_QueEvent( in_eventTime, SE_KEY, K_MWHEELUP, true, 0, NULL );
					Sys_QueEvent( in_eventTime, SE_KEY, K_MWHEELUP, false, 0, NULL );
				}
				else if( e.wheel.y < 0 )
				{
					Sys_QueEvent( in_eventTime, SE_KEY, K_MWHEELDOWN, true, 0, NULL );
					Sys_QueEvent( in_eventTime, SE_KEY, K_MWHEELDOWN, false, 0, NULL );
				}
				break;

			case SDL_WINDOWEVENT:
				switch( e.window.event )
				{
					case SDL_WINDOWEVENT_RESIZED:
						{
							int width, height;

							width = e.window.data1;
							height = e.window.data2;

							// ignore this event on fullscreen
							if( cls.glconfig.isFullscreen )
							{
								break;
							}

							// check if size actually changed
							if( cls.glconfig.vidWidth == width && cls.glconfig.vidHeight == height )
							{
								break;
							}

							Cvar_SetValue( "r_customwidth", width );
							Cvar_SetValue( "r_customheight", height );
							Cvar_Set( "r_mode", "-1" );

							// Wait until user stops dragging for 1 second, so
							// we aren't constantly recreating the GL context while
							// he tries to drag...
							vidRestartTime = Sys_Milliseconds( ) + 1000;
						}
						break;

					case SDL_WINDOWEVENT_MINIMIZED:    Cvar_SetValue( "com_minimized", 1 ); break;
					case SDL_WINDOWEVENT_RESTORED:
					case SDL_WINDOWEVENT_MAXIMIZED:    Cvar_SetValue( "com_minimized", 0 ); break;
					case SDL_WINDOWEVENT_FOCUS_LOST:   Cvar_SetValue( "com_unfocused", 1 ); break;
					case SDL_WINDOWEVENT_FOCUS_GAINED: Cvar_SetValue( "com_unfocused", 0 ); break;
				}
				break;

			case SDL_QUIT:
				Cbuf_ExecuteText(EXEC_NOW, "quit Closed window\n");
				break;

			default:
				break;
		}
	}
}

/*
===============
IN_Frame
===============
*/
void IN_Frame( void )
{
	bool loading;
	bool fullscreen;

	if( !SDL_window )
		return;

	IN_JoyMove( );

	// If not DISCONNECTED (main menu) or ACTIVE (in game), we're loading
	loading = ( cls.state != CA_DISCONNECTED && cls.state != CA_ACTIVE );

	fullscreen = Cvar_VariableIntegerValue( "r_fullscreen" ) != 0;

	if( !fullscreen && ( cls.keyCatchers & KEYCATCH_CONSOLE ) )
	{
		// Console is down in windowed mode
		IN_DeactivateMouse( fullscreen );
	}
	else if( !fullscreen && loading )
	{
		// Loading in windowed mode
		IN_DeactivateMouse( fullscreen );
	}
	else if( !( SDL_GetWindowFlags( SDL_window ) & SDL_WINDOW_INPUT_FOCUS ) )
	{
		// Window not got focus
		IN_DeactivateMouse( fullscreen );
	}
	else
		IN_ActivateMouse( fullscreen );

	IN_ProcessEvents( );

	// Set event time for next frame to earliest possible time an event could happen
	in_eventTime = Sys_Milliseconds( );

	// In case we had to delay actual restart of video system
	if( ( vidRestartTime != 0 ) && ( vidRestartTime < Sys_Milliseconds( ) ) )
	{
		vidRestartTime = 0;
		Cbuf_AddText( "vid_restart\n" );
	}
}

/*
===============
IN_Init
===============
*/
void IN_Init( void )
{
	if( !SDL_WasInit( SDL_INIT_VIDEO ) || !SDL_window )
	{
		Com_Error( ERR_FATAL, "IN_Init called before SDL_Init( SDL_INIT_VIDEO )" );
		return;
	}

	Com_DPrintf( "\n------- Input Initialization -------\n" );

	in_keyboardDebug = Cvar_Get( "in_keyboardDebug", "0", CVAR_ARCHIVE );

	// mouse variables
	in_mouse = Cvar_Get( "in_mouse", "1", CVAR_ARCHIVE|CVAR_LATCH );
	in_nograb = Cvar_Get( "in_nograb", "0", CVAR_ARCHIVE );

	in_joystick = Cvar_Get( "in_joystick", "0", CVAR_ARCHIVE|CVAR_LATCH );
	in_joyBallScale = Cvar_Get( "in_joyBallScale", "0.02", CVAR_ARCHIVE );
	in_debugJoystick = Cvar_Get( "in_debugjoystick", "0", CVAR_TEMP );
	joy_threshold = Cvar_Get( "joy_threshold", "0.15", CVAR_ARCHIVE );

	SDL_StartTextInput( );

	mouseAvailable = ( in_mouse->value != 0 );
	IN_DeactivateMouse( Cvar_VariableIntegerValue( "r_fullscreen" ) != 0 );

	IN_InitJoystick( );
	Com_DPrintf( "------------------------------------\n" );
}

/*
===============
IN_Shutdown
===============
*/
void IN_Shutdown( void )
{
	SDL_StopTextInput( );

	IN_DeactivateMouse( Cvar_VariableIntegerValue( "r_fullscreen" ) != 0 );
	mouseAvailable = false;

	IN_ShutdownJoystick( );
}
