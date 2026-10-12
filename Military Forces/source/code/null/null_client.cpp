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

void ClientLoop::shutdown(const char *finalmsg, bool disconnect, bool quit)
{
}

void ClientLoop::init() {
	cl_shownet = CvarSystem::get ("cl_shownet", "0", CVAR_TEMP );
}

void ClientLoop::mouseEvent( int dx, int dy, int time ) {
}

void ClientLoop::mousePosition( int x, int y ) {
}

void Keys::writeBindings( fileHandle_t f ) {
}

void ClientLoop::frame ( int msec ) {
}

void ClientLoop::packetEvent( netadr_t from, Message *msg ) {
}

void ClientLoop::charEvent( int key ) {
}

void ClientLoop::disconnect( bool showMainMenu ) {
}

void ClientLoop::mapLoading() {
}

bool ClientLoop::gameCommand() {
  return false;
}

void ClientLoop::keyEvent (int key, bool down, unsigned time) {
}

bool UI_GameCommand() {
	return false;
}

void ClientLoop::forwardCommandToServer( const char *string ) {
}

void ClientLoop::consolePrint( char *txt ) {
}

void ClientLoop::joystickEvent( int axis, int value, int time ) {
}

void ClientLoop::initKeyCommands() {
}

void ClientLoop::cdDialog() {
}

void ClientLoop::flushMemory()
{
}

void ClientLoop::shutdownAll(bool shutdownRef)
{
}

void ClientLoop::startHunkUsers( bool rendererOnly ) {
}

void ClientLoop::initRenderer()
{
}

void ClientLoop::shutdownSound()
{
}

bool ClientLoop::cdKeyValidate( const char *key, const char *checksum ) { return true; }

void SoundSystem::clearSoundBuffer() {
}

void IN_Frame() {
}

void IN_Restart() {
}
