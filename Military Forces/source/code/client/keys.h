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
#include "../ui/keycodes.h"

struct qkey_t {
	bool	down;
	int			repeats;		// if > 1, it is autorepeating
	char		*binding;
};

extern	bool	key_overstrikeMode;
extern	qkey_t		keys[MAX_KEYS];

// NOTE TTimo the declaration of EditField and Field_Clear is now in qcommon/qcommon.h
void Field_KeyDownEvent( EditField *edit, int key );
void Field_CharEvent( EditField *edit, int ch );
void Field_Draw( EditField *edit, int x, int y, int width, bool showCursor, bool noColorEscape );
void Field_BigDraw( EditField *edit, int x, int y, int width, bool showCursor, bool noColorEscape );

inline constexpr int COMMAND_HISTORY = 32;
extern	EditField	historyEditLines[COMMAND_HISTORY];

extern	EditField	g_consoleField;
extern	EditField	chatField;
extern	int				anykeydown;
extern	bool	chat_team;
extern	int			chat_playerNum;

