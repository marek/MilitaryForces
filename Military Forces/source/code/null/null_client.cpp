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

#include "../game/q_shared.h"
#include "../qcommon/qcommon.h"

cvar_t *cl_shownet;

void CL_Shutdown(const char *finalmsg, bool disconnect, bool quit)
{
}

void CL_Init() {
	cl_shownet = CvarSystem::get ("cl_shownet", "0", CVAR_TEMP );
}

void CL_MouseEvent( int dx, int dy, int time ) {
}

void Keys::writeBindings( fileHandle_t f ) {
}

void CL_Frame ( int msec ) {
}

void CL_PacketEvent( netadr_t from, Message *msg ) {
}

void CL_CharEvent( int key ) {
}

void CL_Disconnect( bool showMainMenu ) {
}

void CL_MapLoading() {
}

bool CL_GameCommand() {
  return false;
}

void CL_KeyEvent (int key, bool down, unsigned time) {
}

bool UI_GameCommand() {
	return false;
}

void CL_ForwardCommandToServer( const char *string ) {
}

void CL_ConsolePrint( char *txt ) {
}

void CL_JoystickEvent( int axis, int value, int time ) {
}

void CL_InitKeyCommands() {
}

void CL_CDDialog() {
}

void CL_FlushMemory()
{
}

void CL_ShutdownAll(bool shutdownRef)
{
}

void CL_StartHunkUsers( bool rendererOnly ) {
}

void CL_InitRef()
{
}

void CL_Snd_Shutdown()
{
}

bool CL_CDKeyValidate( const char *key, const char *checksum ) { return true; }

void SoundSystem::clearSoundBuffer() {
}

void IN_Frame() {
}

void IN_Restart() {
}
