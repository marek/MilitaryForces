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
// cl.input.c  -- builds an intended movement command to send to the server

#include "client.h"
#include "../ui/ui.h"
#include "../cgame/cg.h"

unsigned	frame_msec;
static int			old_com_frameTime;

/*
===============================================================================

KEY BUTTONS

Continuous button event tracking is complicated by the fact that two different
input sources (say, mouse button 1 and the control key) can both press the
same button, but the button should only be released when both of the
pressing key have been released.

When a key event issues a button command (+forward, +attack, etc), it appends
its key number as argv(1) so it can be matched up with the release.

argv(2) will be set to the time the event happened, which allows exact
control even at low framerates when the down and up events may both get qued
at the same time.

===============================================================================
*/


kbutton_t	in_left, in_right, in_forward, in_back;
kbutton_t	in_lookup, in_lookdown, in_moveleft, in_moveright;
kbutton_t	in_strafe, in_speed;
kbutton_t	in_up, in_down;

#ifdef USE_VOIP
kbutton_t	in_voiprecord;
#endif

kbutton_t	in_buttons[16];


static bool	in_mlooking;


static void IN_MLookDown( ) {
	in_mlooking = true;
}

static void IN_MLookUp( ) {
	in_mlooking = false;
	if ( !cl_freelook->integer ) {
		IN_CenterView ();
	}
}

static void IN_KeyDown( kbutton_t *b ) {
	int		k;
	char	*c;
	
	c = CommandSystem::argv(1);
	if ( c[0] ) {
		k = atoi(c);
	} else {
		k = -1;		// typed manually at the console for continuous down
	}

	if ( k == b->down[0] || k == b->down[1] ) {
		return;		// repeating key
	}
	
	if ( !b->down[0] ) {
		b->down[0] = k;
	} else if ( !b->down[1] ) {
		b->down[1] = k;
	} else {
		Common::printf ("Three keys down for a button!\n");
		return;
	}
	
	if ( b->active ) {
		return;		// still down
	}

	// save timestamp for partial frame summing
	c = CommandSystem::argv(2);
	b->downtime = atoi(c);

	b->active = true;
	b->wasPressed = true;
}

static void IN_KeyUp( kbutton_t *b ) {
	int		k;
	char	*c;
	unsigned	uptime;

	c = CommandSystem::argv(1);
	if ( c[0] ) {
		k = atoi(c);
	} else {
		// typed manually at the console, assume for unsticking, so clear all
		b->down[0] = b->down[1] = 0;
		b->active = false;
		return;
	}

	if ( b->down[0] == k ) {
		b->down[0] = 0;
	} else if ( b->down[1] == k ) {
		b->down[1] = 0;
	} else {
		return;		// key up without coresponding down (menu pass through)
	}
	if ( b->down[0] || b->down[1] ) {
		return;		// some other key is still holding it down
	}

	b->active = false;

	// save timestamp for partial frame summing
	c = CommandSystem::argv(2);
	uptime = atoi(c);
	if ( uptime ) {
		b->msec += uptime - b->downtime;
	} else {
		b->msec += frame_msec / 2;
	}

	b->active = false;
}



/*
===============
Client::keyState

Returns the fraction of the frame that the key was down
===============
*/
float Client::keyState( kbutton_t *key ) {
	float		val;
	int			msec;

	msec = key->msec;
	key->msec = 0;

	if ( key->active ) {
		// still down
		if ( !key->downtime ) {
			msec = com_frameTime;
		} else {
			msec += com_frameTime - key->downtime;
		}
		key->downtime = com_frameTime;
	}

#if 0
	if (msec) {
		Common::printf ("%i ", msec);
	}
#endif

	val = static_cast<float>(msec) / frame_msec;
	if ( val < 0 ) {
		val = 0;
	}
	if ( val > 1 ) {
		val = 1;
	}

	return val;
}



static void IN_UpDown() {IN_KeyDown(&in_up);}
static void IN_UpUp() {IN_KeyUp(&in_up);}
static void IN_DownDown() {IN_KeyDown(&in_down);}
static void IN_DownUp() {IN_KeyUp(&in_down);}
static void IN_LeftDown() {IN_KeyDown(&in_left);}
static void IN_LeftUp() {IN_KeyUp(&in_left);}
static void IN_RightDown() {IN_KeyDown(&in_right);}
static void IN_RightUp() {IN_KeyUp(&in_right);}
static void IN_ForwardDown() {IN_KeyDown(&in_forward);}
static void IN_ForwardUp() {IN_KeyUp(&in_forward);}
static void IN_BackDown() {IN_KeyDown(&in_back);}
static void IN_BackUp() {IN_KeyUp(&in_back);}
static void IN_LookupDown() {IN_KeyDown(&in_lookup);}
static void IN_LookupUp() {IN_KeyUp(&in_lookup);}
static void IN_LookdownDown() {IN_KeyDown(&in_lookdown);}
static void IN_LookdownUp() {IN_KeyUp(&in_lookdown);}
static void IN_MoveleftDown() {IN_KeyDown(&in_moveleft);}
static void IN_MoveleftUp() {IN_KeyUp(&in_moveleft);}
static void IN_MoverightDown() {IN_KeyDown(&in_moveright);}
static void IN_MoverightUp() {IN_KeyUp(&in_moveright);}

static void IN_SpeedDown() {IN_KeyDown(&in_speed);}
static void IN_SpeedUp() {IN_KeyUp(&in_speed);}
static void IN_StrafeDown() {IN_KeyDown(&in_strafe);}
static void IN_StrafeUp() {IN_KeyUp(&in_strafe);}

#ifdef USE_VOIP
static void IN_VoipRecordDown()
{
	IN_KeyDown(&in_voiprecord);
	CvarSystem::set("cl_voipSend", "1");
}

static void IN_VoipRecordUp()
{
	IN_KeyUp(&in_voiprecord);
	CvarSystem::set("cl_voipSend", "0");
}
#endif

static void IN_Button0Down() {IN_KeyDown(&in_buttons[0]);}
static void IN_Button0Up() {IN_KeyUp(&in_buttons[0]);}
static void IN_Button1Down() {IN_KeyDown(&in_buttons[1]);}
static void IN_Button1Up() {IN_KeyUp(&in_buttons[1]);}
static void IN_Button2Down() {IN_KeyDown(&in_buttons[2]);}
static void IN_Button2Up() {IN_KeyUp(&in_buttons[2]);}
static void IN_Button3Down() {IN_KeyDown(&in_buttons[3]);}
static void IN_Button3Up() {IN_KeyUp(&in_buttons[3]);}
static void IN_Button4Down() {IN_KeyDown(&in_buttons[4]);}
static void IN_Button4Up() {IN_KeyUp(&in_buttons[4]);}
static void IN_Button5Down() {IN_KeyDown(&in_buttons[5]);}
static void IN_Button5Up() {IN_KeyUp(&in_buttons[5]);}
static void IN_Button6Down() {IN_KeyDown(&in_buttons[6]);}
static void IN_Button6Up() {IN_KeyUp(&in_buttons[6]);}
static void IN_Button7Down() {IN_KeyDown(&in_buttons[7]);}
static void IN_Button7Up() {IN_KeyUp(&in_buttons[7]);}
static void IN_Button8Down() {IN_KeyDown(&in_buttons[8]);}
static void IN_Button8Up() {IN_KeyUp(&in_buttons[8]);}
static void IN_Button9Down() {IN_KeyDown(&in_buttons[9]);}
static void IN_Button9Up() {IN_KeyUp(&in_buttons[9]);}
static void IN_Button10Down() {IN_KeyDown(&in_buttons[10]);}
static void IN_Button10Up() {IN_KeyUp(&in_buttons[10]);}
static void IN_Button11Down() {IN_KeyDown(&in_buttons[11]);}
static void IN_Button11Up() {IN_KeyUp(&in_buttons[11]);}
static void IN_Button12Down() {IN_KeyDown(&in_buttons[12]);}
static void IN_Button12Up() {IN_KeyUp(&in_buttons[12]);}
static void IN_Button13Down() {IN_KeyDown(&in_buttons[13]);}
static void IN_Button13Up() {IN_KeyUp(&in_buttons[13]);}
static void IN_Button14Down() {IN_KeyDown(&in_buttons[14]);}
static void IN_Button14Up() {IN_KeyUp(&in_buttons[14]);}


void IN_CenterView () {
	cl.viewangles[PITCH] = -SHORT2ANGLE(cl.snap.ps.delta_angles[PITCH]);
}


//==========================================================================

cvar_t	*cl_yawspeed;
cvar_t	*cl_pitchspeed;

cvar_t	*cl_run;

cvar_t	*cl_anglespeedkey;


/*
================
CL_AdjustAngles

Moves the local angle positions
================
*/
static void CL_AdjustAngles( ) {
	float	speed;
	
	if ( in_speed.active ) {
		speed = 0.001 * cls.frametime * cl_anglespeedkey->value;
	} else {
		speed = 0.001 * cls.frametime;
	}

	if ( !in_strafe.active ) {
		cl.viewangles[YAW] -= speed*cl_yawspeed->value*Client::keyState (&in_right);
		cl.viewangles[YAW] += speed*cl_yawspeed->value*Client::keyState (&in_left);
	}

	cl.viewangles[PITCH] -= speed*cl_pitchspeed->value * Client::keyState (&in_lookup);
	cl.viewangles[PITCH] += speed*cl_pitchspeed->value * Client::keyState (&in_lookdown);
}

/*
================
CL_KeyMove

Sets the usercmd_t based on key states
================
*/
static void CL_KeyMove( usercmd_t *cmd ) {
	int		movespeed;
	int		forward, side, up;

	//
	// adjust for speed key / running
	// the walking flag is to keep animations consistent
	// even during acceleration and develeration
	//
	if ( in_speed.active ^ cl_run->integer ) {
		movespeed = 127;
		cmd->buttons &= ~BUTTON_XXX2;
	} else {
		cmd->buttons |= BUTTON_XXX2;
		movespeed = 64;
	}

	forward = 0;
	side = 0;
	up = 0;
	if ( in_strafe.active ) {
		side += movespeed * Client::keyState (&in_right);
		side -= movespeed * Client::keyState (&in_left);
	}

	side += movespeed * Client::keyState (&in_moveright);
	side -= movespeed * Client::keyState (&in_moveleft);


	up += movespeed * Client::keyState (&in_up);
	up -= movespeed * Client::keyState (&in_down);

	forward += movespeed * Client::keyState (&in_forward);
	forward -= movespeed * Client::keyState (&in_back);

	cmd->forwardmove = ClampChar( forward );
	cmd->rightmove = ClampChar( side );
	cmd->upmove = ClampChar( up );
}

/*
=================
ClientLoop::mouseEvent
=================
*/
void ClientLoop::mouseEvent( int dx, int dy, int time ) {
	if ( Key_GetCatcher( ) & KEYCATCH_UI ) {
		theUI.mouseEvent( dx, dy );
	} else if (Key_GetCatcher( ) & KEYCATCH_CGAME) {
		theCG.mouseEvent( dx, dy );
	} else {
		cl.mouseDx[cl.mouseIndex] += dx;
		cl.mouseDy[cl.mouseIndex] += dy;
	}
}

/*
=================
ClientLoop::joystickEvent

Joystick values stay set until changed
=================
*/
void ClientLoop::joystickEvent( int axis, int value, int time ) {
	if ( axis < 0 || axis >= MAX_JOYSTICK_AXIS ) {
		Common::error( ERR_DROP, "CL_JoystickEvent: bad axis %i", axis );
	}
	cl.joystickAxis[axis] = value;
}

/*
=================
CL_JoystickMove
=================
*/
static void CL_JoystickMove( usercmd_t *cmd ) {
	float	anglespeed;

	float yaw     = j_yaw->value     * cl.joystickAxis[j_yaw_axis->integer];
	float right   = j_side->value    * cl.joystickAxis[j_side_axis->integer];
	float forward = j_forward->value * cl.joystickAxis[j_forward_axis->integer];
	float pitch   = j_pitch->value   * cl.joystickAxis[j_pitch_axis->integer];
	float up      = j_up->value      * cl.joystickAxis[j_up_axis->integer];

	if ( !(in_speed.active ^ cl_run->integer) ) {
		cmd->buttons |= BUTTON_XXX2;
	}

	if ( in_speed.active ) {
		anglespeed = 0.001 * cls.frametime * cl_anglespeedkey->value;
	} else {
		anglespeed = 0.001 * cls.frametime;
	}

	if ( !in_strafe.active ) {
		cl.viewangles[YAW] += anglespeed * yaw;
		cmd->rightmove = ClampChar( cmd->rightmove + static_cast<int>(right) );
	} else {
		cl.viewangles[YAW] += anglespeed * right;
		cmd->rightmove = ClampChar( cmd->rightmove + static_cast<int>(yaw) );
	}

	if ( in_mlooking ) {
		cl.viewangles[PITCH] += anglespeed * forward;
		cmd->forwardmove = ClampChar( cmd->forwardmove + static_cast<int>(pitch) );
	} else {
		cl.viewangles[PITCH] += anglespeed * pitch;
		cmd->forwardmove = ClampChar( cmd->forwardmove + static_cast<int>(forward) );
	}

	cmd->upmove = ClampChar( cmd->upmove + static_cast<int>(up) );
}

/*
=================
CL_MouseMove
=================
*/

static void CL_MouseMove(usercmd_t *cmd)
{
	float mx, my;

	// allow mouse smoothing
	if (m_filter->integer)
	{
		mx = (cl.mouseDx[0] + cl.mouseDx[1]) * 0.5f;
		my = (cl.mouseDy[0] + cl.mouseDy[1]) * 0.5f;
	}
	else
	{
		mx = cl.mouseDx[cl.mouseIndex];
		my = cl.mouseDy[cl.mouseIndex];
	}
	
	cl.mouseIndex ^= 1;
	cl.mouseDx[cl.mouseIndex] = 0;
	cl.mouseDy[cl.mouseIndex] = 0;

	if (mx == 0.0f && my == 0.0f)
		return;
	
	if (cl_mouseAccel->value != 0.0f)
	{
		if(cl_mouseAccelStyle->integer == 0)
		{
			float accelSensitivity;
			float rate;
			
			rate = sqrt(mx * mx + my * my) / static_cast<float>(frame_msec);

			accelSensitivity = cl_sensitivity->value + rate * cl_mouseAccel->value;
			mx *= accelSensitivity;
			my *= accelSensitivity;
			
			if(cl_showMouseRate->integer)
				Common::printf("rate: %f, accelSensitivity: %f\n", rate, accelSensitivity);
		}
		else
		{
			float rate[2];
			float power[2];

			// sensitivity remains pretty much unchanged at low speeds
			// cl_mouseAccel is a power value to how the acceleration is shaped
			// cl_mouseAccelOffset is the rate for which the acceleration will have doubled the non accelerated amplification
			// NOTE: decouple the config cvars for independent acceleration setup along X and Y?

			rate[0] = fabs(mx) / static_cast<float>(frame_msec);
			rate[1] = fabs(my) / static_cast<float>(frame_msec);
			power[0] = powf(rate[0] / cl_mouseAccelOffset->value, cl_mouseAccel->value);
			power[1] = powf(rate[1] / cl_mouseAccelOffset->value, cl_mouseAccel->value);

			mx = cl_sensitivity->value * (mx + ((mx < 0) ? -power[0] : power[0]) * cl_mouseAccelOffset->value);
			my = cl_sensitivity->value * (my + ((my < 0) ? -power[1] : power[1]) * cl_mouseAccelOffset->value);

			if(cl_showMouseRate->integer)
				Common::printf("ratex: %f, ratey: %f, powx: %f, powy: %f\n", rate[0], rate[1], power[0], power[1]);
		}
	}
	else
	{
		mx *= cl_sensitivity->value;
		my *= cl_sensitivity->value;
	}

	// ingame FOV
	mx *= cl.cgameSensitivity;
	my *= cl.cgameSensitivity;

	// add mouse X/Y movement to cmd
	if(in_strafe.active)
		cmd->rightmove = ClampChar(cmd->rightmove + m_side->value * mx);
	else
		cl.viewangles[YAW] -= m_yaw->value * mx;

	if ((in_mlooking || cl_freelook->integer) && !in_strafe.active)
		cl.viewangles[PITCH] += m_pitch->value * my;
	else
		cmd->forwardmove = ClampChar(cmd->forwardmove - m_forward->value * my);
}


/*
==============
CL_CmdButtons
==============
*/
static void CL_CmdButtons( usercmd_t *cmd ) {
	int		i;

	//
	// figure button bits
	// send a button bit even if the key was pressed and released in
	// less than a frame
	//	
	for (i = 0 ; i < 15 ; i++) {
		if ( in_buttons[i].active || in_buttons[i].wasPressed ) {
			cmd->buttons |= 1 << i;
		}
		in_buttons[i].wasPressed = false;
	}

	if ( Key_GetCatcher( ) ) {
		cmd->buttons |= BUTTON_TALK;
	}

	// allow the game to know if any key at all is
	// currently pressed, even if it isn't bound to anything
	if ( anykeydown && Key_GetCatcher( ) == 0 ) {
		cmd->buttons |= BUTTON_ANY;
	}
}


/*
==============
CL_FinishMove
==============
*/
static void CL_FinishMove( usercmd_t *cmd ) {
	int		i;

	// copy the state that the cgame is currently sending
	cmd->weapon = cl.cgameUserCmdValue;

	// send the current server time so the amount of movement
	// can be determined without allowing cheating
	cmd->serverTime = cl.serverTime;

	for (i=0 ; i<3 ; i++) {
		cmd->angles[i] = ANGLE2SHORT(cl.viewangles[i]);
	}
}


/*
=================
CL_CreateCmd
=================
*/
static usercmd_t CL_CreateCmd( ) {
	usercmd_t	cmd;
	vec3_t		oldAngles;

	VectorCopy( cl.viewangles, oldAngles );

	// keyboard angle adjustment
	CL_AdjustAngles ();
	
	Com_Memset( &cmd, 0, sizeof( cmd ) );

	CL_CmdButtons( &cmd );

	// get basic movement from keyboard
	CL_KeyMove( &cmd );

	// get basic movement from mouse
	CL_MouseMove( &cmd );

	// get basic movement from joystick
	CL_JoystickMove( &cmd );

	// check to make sure the angles haven't wrapped
	if ( cl.viewangles[PITCH] - oldAngles[PITCH] > 90 ) {
		cl.viewangles[PITCH] = oldAngles[PITCH] + 90;
	} else if ( oldAngles[PITCH] - cl.viewangles[PITCH] > 90 ) {
		cl.viewangles[PITCH] = oldAngles[PITCH] - 90;
	} 

	// store out the final values
	CL_FinishMove( &cmd );

	// draw debug graphs of turning for mouse testing
	if ( cl_debugMove->integer ) {
		if ( cl_debugMove->integer == 1 ) {
			Screen::debugGraph( fabs(cl.viewangles[YAW] - oldAngles[YAW]) );
		}
		if ( cl_debugMove->integer == 2 ) {
			Screen::debugGraph( fabs(cl.viewangles[PITCH] - oldAngles[PITCH]) );
		}
	}

	return cmd;
}


/*
=================
CL_CreateNewCommands

Create a new usercmd_t structure for this frame
=================
*/
static void CL_CreateNewCommands( ) {
	int			cmdNum;

	// no need to create usercmds until we have a gamestate
	if ( clc.state < CA_PRIMED ) {
		return;
	}

	frame_msec = com_frameTime - old_com_frameTime;

	// if running over 1000fps, act as if each frame is 1ms
	// prevents divisions by zero
	if ( frame_msec < 1 ) {
		frame_msec = 1;
	}

	// if running less than 5fps, truncate the extra time to prevent
	// unexpected moves after a hitch
	if ( frame_msec > 200 ) {
		frame_msec = 200;
	}
	old_com_frameTime = com_frameTime;


	// generate a command for this frame
	cl.cmdNumber++;
	cmdNum = cl.cmdNumber & CMD_MASK;
	cl.cmds[cmdNum] = CL_CreateCmd ();
}

/*
=================
CL_ReadyToSendPacket

Returns false if we are over the maxpackets limit
and should choke back the bandwidth a bit by not sending
a packet this frame.  All the commands will still get
delivered in the next packet, but saving a header and
getting more delta compression will reduce total bandwidth.
=================
*/
static bool CL_ReadyToSendPacket( ) {
	int		oldPacketNum;
	int		delta;

	// don't send anything if playing back a demo
	if ( clc.demoplaying || clc.state == CA_CINEMATIC ) {
		return false;
	}

	// If we are downloading, we send no less than 50ms between packets
	if ( *clc.downloadTempName &&
		cls.realtime - clc.lastPacketSentTime < 50 ) {
		return false;
	}

	// if we don't have a valid gamestate yet, only send
	// one packet a second
	if ( clc.state != CA_ACTIVE && 
		clc.state != CA_PRIMED && 
		!*clc.downloadTempName &&
		cls.realtime - clc.lastPacketSentTime < 1000 ) {
		return false;
	}

	// send every frame for loopbacks
	if ( clc.netchan.remoteAddress().type == NA_LOOPBACK ) {
		return true;
	}

	// send every frame for LAN
	if ( cl_lanForcePackets->integer && Sys_IsLANAddress( clc.netchan.remoteAddress() ) ) {
		return true;
	}

	// check for exceeding cl_maxpackets
	if ( cl_maxpackets->integer < 15 ) {
		CvarSystem::set( "cl_maxpackets", "15" );
	} else if ( cl_maxpackets->integer > 125 ) {
		CvarSystem::set( "cl_maxpackets", "125" );
	}
	oldPacketNum = (clc.netchan.outgoingSequence() - 1) & PACKET_MASK;
	delta = cls.realtime -  cl.outPackets[ oldPacketNum ].p_realtime;
	if ( delta < 1000 / cl_maxpackets->integer ) {
		// the accumulated commands will go out in the next packet
		return false;
	}

	return true;
}

/*
===================
Client::writePacket

Create and send the command packet to the server
Including both the reliable commands and the usercmds

During normal gameplay, a client packet will contain something like:

4	sequence number
2	qport
4	serverid
4	acknowledged sequence number
4	clc.serverCommandSequence
<optional reliable commands>
1	clc_move or clc_moveNoDelta
1	command count
<count * usercmds>

===================
*/
void Client::writePacket( ) {
	Message		buf;
	byte		data[MAX_MSGLEN];
	int			i, j;
	usercmd_t	*cmd, *oldcmd;
	usercmd_t	nullcmd;
	int			packetNum;
	int			oldPacketNum;
	int			count, key;

	// don't send anything if playing back a demo
	if ( clc.demoplaying || clc.state == CA_CINEMATIC ) {
		return;
	}

	Com_Memset( &nullcmd, 0, sizeof(nullcmd) );
	oldcmd = &nullcmd;

	buf = Message( data, sizeof(data) );

	buf.bitstream();
	// write the current serverId so the server
	// can tell if this is from the current gameState
	buf.writeLong( cl.serverId );

	// write the last message we received, which can
	// be used for delta compression, and is also used
	// to tell if we dropped a gamestate
	buf.writeLong( clc.serverMessageSequence );

	// write the last reliable message we received
	buf.writeLong( clc.serverCommandSequence );

	buf.writeLong( cl.snap.valid ? cl.snap.messageNum : 0 );
	buf.writeLong( cl.entityAckBits );

	// write any unacknowledged clientCommands
	for ( i = clc.reliableAcknowledge + 1 ; i <= clc.reliableSequence ; i++ ) {
		buf.writeByte( clc_clientCommand );
		buf.writeLong( i );
		buf.writeString( clc.reliableCommands[ i & (MAX_RELIABLE_COMMANDS-1) ] );
	}

	// we want to send all the usercmds that were generated in the last
	// few packet, so even if a couple packets are dropped in a row,
	// all the cmds will make it to the server
	if ( cl_packetdup->integer < 0 ) {
		CvarSystem::set( "cl_packetdup", "0" );
	} else if ( cl_packetdup->integer > 5 ) {
		CvarSystem::set( "cl_packetdup", "5" );
	}
	oldPacketNum = (clc.netchan.outgoingSequence() - 1 - cl_packetdup->integer) & PACKET_MASK;
	count = cl.cmdNumber - cl.outPackets[ oldPacketNum ].p_cmdNumber;
	if ( count > MAX_PACKET_USERCMDS ) {
		count = MAX_PACKET_USERCMDS;
		Common::printf("MAX_PACKET_USERCMDS\n");
	}

#ifdef USE_VOIP
	if (clc.voipOutgoingDataSize > 0)
	{
		if((clc.voipFlags & VOIP_SPATIAL) || Common::isVoipTarget(clc.voipTargets, sizeof(clc.voipTargets), -1))
		{
			buf.writeByte( clc_voipOpus);
			buf.writeByte( clc.voipOutgoingGeneration);
			buf.writeLong( clc.voipOutgoingSequence);
			buf.writeByte( clc.voipOutgoingDataFrames);
			buf.writeData( clc.voipTargets, sizeof(clc.voipTargets));
			buf.writeByte( clc.voipFlags);
			buf.writeShort( clc.voipOutgoingDataSize);
			buf.writeData( clc.voipOutgoingData, clc.voipOutgoingDataSize);

			// If we're recording a demo, we have to fake a server packet with
			//  this VoIP data so it gets to disk; the server doesn't send it
			//  back to us, and we might as well eliminate concerns about dropped
			//  and misordered packets here.
			if(clc.demorecording && !clc.demowaiting)
			{
				const int voipSize = clc.voipOutgoingDataSize;
				Message fakemsg;
				byte fakedata[MAX_MSGLEN];
				fakemsg = Message( fakedata, sizeof (fakedata) );
				fakemsg.bitstream();
				fakemsg.writeLong( clc.reliableAcknowledge);
				fakemsg.writeByte( svc_voipOpus);
				fakemsg.writeShort( clc.clientNum);
				fakemsg.writeByte( clc.voipOutgoingGeneration);
				fakemsg.writeLong( clc.voipOutgoingSequence);
				fakemsg.writeByte( clc.voipOutgoingDataFrames);
				fakemsg.writeShort( clc.voipOutgoingDataSize );
				fakemsg.writeBits( clc.voipFlags, VOIP_FLAGCNT);
				fakemsg.writeData( clc.voipOutgoingData, voipSize);
				fakemsg.writeByte( svc_EOF);
				Client::writeDemoMessage (&fakemsg, 0);
			}

			clc.voipOutgoingSequence += clc.voipOutgoingDataFrames;
			clc.voipOutgoingDataSize = 0;
			clc.voipOutgoingDataFrames = 0;
		}
		else
		{
			// We have data, but no targets. Silently discard all data
			clc.voipOutgoingDataSize = 0;
			clc.voipOutgoingDataFrames = 0;
		}
	}
#endif

	if ( count >= 1 ) {
		if ( cl_showSend->integer ) {
			Common::printf( "(%i)", count );
		}

		// begin a client move command
		if ( cl_nodelta->integer || !cl.snap.valid || clc.demowaiting
			|| clc.serverMessageSequence != cl.snap.messageNum ) {
			buf.writeByte( clc_moveNoDelta);
		} else {
			buf.writeByte( clc_move);
		}

		// write the command count
		buf.writeByte( count );

		// use the checksum feed in the key
		key = clc.checksumFeed;
		// also use the message acknowledge
		key ^= clc.serverMessageSequence;
		// also use the last acknowledged server command in the key
		key ^= Message::hashKey(clc.serverCommands[ clc.serverCommandSequence & (MAX_RELIABLE_COMMANDS-1) ], 32);

		// write all the commands, including the predicted command
		for ( i = 0 ; i < count ; i++ ) {
			j = (cl.cmdNumber - count + i + 1) & CMD_MASK;
			cmd = &cl.cmds[j];
			buf.writeDeltaUsercmdKey( key, oldcmd, cmd);
			oldcmd = cmd;
		}
	}

	//
	// deliver the message
	//
	packetNum = clc.netchan.outgoingSequence() & PACKET_MASK;
	cl.outPackets[ packetNum ].p_realtime = cls.realtime;
	cl.outPackets[ packetNum ].p_serverTime = oldcmd->serverTime;
	cl.outPackets[ packetNum ].p_cmdNumber = cl.cmdNumber;
	clc.lastPacketSentTime = cls.realtime;

	if ( cl_showSend->integer ) {
		Common::printf( "%i ", buf.size() );
	}

	Client::netchan_Transmit (&clc.netchan, &buf);	
}

/*
=================
Client::sendCmd

Called every frame to builds and sends a command packet to the server.
=================
*/
void Client::sendCmd( ) {
	// don't send any message if not connected
	if ( clc.state < CA_CONNECTED ) {
		return;
	}

	// don't send commands if paused
	if ( com_sv_running->integer && sv_paused->integer && cl_paused->integer ) {
		return;
	}

	// we create commands even if a demo is playing,
	CL_CreateNewCommands();

	// don't send a packet if the last packet was sent too recently
	if ( !CL_ReadyToSendPacket() ) {
		if ( cl_showSend->integer ) {
			Common::printf( ". " );
		}
		return;
	}

	Client::writePacket();
}

/*
============
Client::initInput
============
*/
void Client::initInput( ) {
	CommandSystem::addCommand ("centerview",IN_CenterView);

	CommandSystem::addCommand ("+moveup",IN_UpDown);
	CommandSystem::addCommand ("-moveup",IN_UpUp);
	CommandSystem::addCommand ("+movedown",IN_DownDown);
	CommandSystem::addCommand ("-movedown",IN_DownUp);
	CommandSystem::addCommand ("+left",IN_LeftDown);
	CommandSystem::addCommand ("-left",IN_LeftUp);
	CommandSystem::addCommand ("+right",IN_RightDown);
	CommandSystem::addCommand ("-right",IN_RightUp);
	CommandSystem::addCommand ("+forward",IN_ForwardDown);
	CommandSystem::addCommand ("-forward",IN_ForwardUp);
	CommandSystem::addCommand ("+back",IN_BackDown);
	CommandSystem::addCommand ("-back",IN_BackUp);
	CommandSystem::addCommand ("+lookup", IN_LookupDown);
	CommandSystem::addCommand ("-lookup", IN_LookupUp);
	CommandSystem::addCommand ("+lookdown", IN_LookdownDown);
	CommandSystem::addCommand ("-lookdown", IN_LookdownUp);
	CommandSystem::addCommand ("+strafe", IN_StrafeDown);
	CommandSystem::addCommand ("-strafe", IN_StrafeUp);
	CommandSystem::addCommand ("+moveleft", IN_MoveleftDown);
	CommandSystem::addCommand ("-moveleft", IN_MoveleftUp);
	CommandSystem::addCommand ("+moveright", IN_MoverightDown);
	CommandSystem::addCommand ("-moveright", IN_MoverightUp);
	CommandSystem::addCommand ("+speed", IN_SpeedDown);
	CommandSystem::addCommand ("-speed", IN_SpeedUp);
	CommandSystem::addCommand ("+attack", IN_Button0Down);
	CommandSystem::addCommand ("-attack", IN_Button0Up);
	CommandSystem::addCommand ("+button0", IN_Button0Down);
	CommandSystem::addCommand ("-button0", IN_Button0Up);
	CommandSystem::addCommand ("+button1", IN_Button1Down);
	CommandSystem::addCommand ("-button1", IN_Button1Up);
	CommandSystem::addCommand ("+button2", IN_Button2Down);
	CommandSystem::addCommand ("-button2", IN_Button2Up);
	CommandSystem::addCommand ("+button3", IN_Button3Down);
	CommandSystem::addCommand ("-button3", IN_Button3Up);
	CommandSystem::addCommand ("+button4", IN_Button4Down);
	CommandSystem::addCommand ("-button4", IN_Button4Up);
	CommandSystem::addCommand ("+button5", IN_Button5Down);
	CommandSystem::addCommand ("-button5", IN_Button5Up);
	CommandSystem::addCommand ("+button6", IN_Button6Down);
	CommandSystem::addCommand ("-button6", IN_Button6Up);
	CommandSystem::addCommand ("+button7", IN_Button7Down);
	CommandSystem::addCommand ("-button7", IN_Button7Up);
	CommandSystem::addCommand ("+button8", IN_Button8Down);
	CommandSystem::addCommand ("-button8", IN_Button8Up);
	CommandSystem::addCommand ("+button9", IN_Button9Down);
	CommandSystem::addCommand ("-button9", IN_Button9Up);
	CommandSystem::addCommand ("+button10", IN_Button10Down);
	CommandSystem::addCommand ("-button10", IN_Button10Up);
	CommandSystem::addCommand ("+button11", IN_Button11Down);
	CommandSystem::addCommand ("-button11", IN_Button11Up);
	CommandSystem::addCommand ("+button12", IN_Button12Down);
	CommandSystem::addCommand ("-button12", IN_Button12Up);
	CommandSystem::addCommand ("+button13", IN_Button13Down);
	CommandSystem::addCommand ("-button13", IN_Button13Up);
	CommandSystem::addCommand ("+button14", IN_Button14Down);
	CommandSystem::addCommand ("-button14", IN_Button14Up);
	CommandSystem::addCommand ("+mlook", IN_MLookDown);
	CommandSystem::addCommand ("-mlook", IN_MLookUp);

#ifdef USE_VOIP
	CommandSystem::addCommand ("+voiprecord", IN_VoipRecordDown);
	CommandSystem::addCommand ("-voiprecord", IN_VoipRecordUp);
#endif

	cl_nodelta = CvarSystem::get ("cl_nodelta", "0", 0);
	cl_debugMove = CvarSystem::get ("cl_debugMove", "0", 0);
}

/*
============
Client::shutdownInput
============
*/
void Client::shutdownInput()
{
	CommandSystem::removeCommand("centerview");

	CommandSystem::removeCommand("+moveup");
	CommandSystem::removeCommand("-moveup");
	CommandSystem::removeCommand("+movedown");
	CommandSystem::removeCommand("-movedown");
	CommandSystem::removeCommand("+left");
	CommandSystem::removeCommand("-left");
	CommandSystem::removeCommand("+right");
	CommandSystem::removeCommand("-right");
	CommandSystem::removeCommand("+forward");
	CommandSystem::removeCommand("-forward");
	CommandSystem::removeCommand("+back");
	CommandSystem::removeCommand("-back");
	CommandSystem::removeCommand("+lookup");
	CommandSystem::removeCommand("-lookup");
	CommandSystem::removeCommand("+lookdown");
	CommandSystem::removeCommand("-lookdown");
	CommandSystem::removeCommand("+strafe");
	CommandSystem::removeCommand("-strafe");
	CommandSystem::removeCommand("+moveleft");
	CommandSystem::removeCommand("-moveleft");
	CommandSystem::removeCommand("+moveright");
	CommandSystem::removeCommand("-moveright");
	CommandSystem::removeCommand("+speed");
	CommandSystem::removeCommand("-speed");
	CommandSystem::removeCommand("+attack");
	CommandSystem::removeCommand("-attack");
	CommandSystem::removeCommand("+button0");
	CommandSystem::removeCommand("-button0");
	CommandSystem::removeCommand("+button1");
	CommandSystem::removeCommand("-button1");
	CommandSystem::removeCommand("+button2");
	CommandSystem::removeCommand("-button2");
	CommandSystem::removeCommand("+button3");
	CommandSystem::removeCommand("-button3");
	CommandSystem::removeCommand("+button4");
	CommandSystem::removeCommand("-button4");
	CommandSystem::removeCommand("+button5");
	CommandSystem::removeCommand("-button5");
	CommandSystem::removeCommand("+button6");
	CommandSystem::removeCommand("-button6");
	CommandSystem::removeCommand("+button7");
	CommandSystem::removeCommand("-button7");
	CommandSystem::removeCommand("+button8");
	CommandSystem::removeCommand("-button8");
	CommandSystem::removeCommand("+button9");
	CommandSystem::removeCommand("-button9");
	CommandSystem::removeCommand("+button10");
	CommandSystem::removeCommand("-button10");
	CommandSystem::removeCommand("+button11");
	CommandSystem::removeCommand("-button11");
	CommandSystem::removeCommand("+button12");
	CommandSystem::removeCommand("-button12");
	CommandSystem::removeCommand("+button13");
	CommandSystem::removeCommand("-button13");
	CommandSystem::removeCommand("+button14");
	CommandSystem::removeCommand("-button14");
	CommandSystem::removeCommand("+mlook");
	CommandSystem::removeCommand("-mlook");

#ifdef USE_VOIP
	CommandSystem::removeCommand("+voiprecord");
	CommandSystem::removeCommand("-voiprecord");
#endif
}
