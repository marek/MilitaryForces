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



// if origin is NULL, the sound will be dynamically sourced from the entity


// cinematics and voice-over-network will send raw samples
// 1.0 volume will be direct output of source samples

// stop all sounds and the background track

// all continuous looping sounds must be added before calling SoundSystem::update

// recompute the relative volumes for all running sounds
// relative to the given entityNum / orientation

// let the sound system know where an entity currently is




// RegisterSound will allways return a valid sample, even if it
// has to create a placeholder.  This prevents continuous filesystem
// checks for missing files

void S_DisplayFreeMemory();


void SNDDMA_Activate( );

void S_UpdateBackgroundTrack( );


#ifdef USE_VOIP
#endif

