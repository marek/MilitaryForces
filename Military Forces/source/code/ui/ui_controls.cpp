#include "ui_controls.h"
#include "keycodes.h"
#include "ui_utils.h"


UI_Utils* ControlUtils::utils_ = 0;


ControlUtils::ControlUtils()
{
	bindings_.push_back( new Binding("+scores",			"Show Scoreboard",				ID_SHOWSCORES,		K_TAB,			-1,			-1, -1) );
	bindings_.push_back( new Binding("weapprev",		"Previous Weapon",				ID_WEAPPREV,		'[',			-1,			-1, -1) );
	bindings_.push_back( new Binding("weapnext", 		"Next Weapon",					ID_WEAPNEXT,		']',			-1,			-1, -1) );
	bindings_.push_back( new Binding("messagemode", 	"Chat",							ID_CHAT,			't',			-1,			-1, -1) );
	bindings_.push_back( new Binding("messagemode2", 	"Chat - Team",					ID_CHAT2,			'y',			-1,			-1, -1) );
	bindings_.push_back( new Binding("messagemode3", 	"Chat - Target",				ID_CHAT3,			'u',			-1,			-1, -1) );
	bindings_.push_back( new Binding("messagemode4", 	"Chat - Attacker",				ID_CHAT4,			'i',			-1,			-1, -1) );
	bindings_.push_back( new Binding("cmessagemode", 	"MFQ3 Chat",					ID_CHAT,			't',			-1,			-1, -1) );
	bindings_.push_back( new Binding("cmessagemode2", 	"MFQ3 Chat - Team",				ID_CHAT2,			-1,				-1,			-1, -1) );
	bindings_.push_back( new Binding("cmessagemode3", 	"MFQ3 Chat - Target",			ID_CHAT3,			-1,				-1,			-1, -1) );
	bindings_.push_back( new Binding("cmessagemode4", 	"MFQ3 Chat - Attacker",			ID_CHAT4,			-1,				-1,			-1, -1) );
	bindings_.push_back( new Binding("weapon 0",		"Select MG",					ID_WEAPON_MG,		'0',			-1,			-1, -1) );
	bindings_.push_back( new Binding("weapon 1",		"Select Weapon 1",				ID_WEAPON_1,		'1',			-1,			-1, -1) );
	bindings_.push_back( new Binding("weapon 2",		"Select Weapon 2",				ID_WEAPON_2,		'2',			-1,			-1, -1) );
	bindings_.push_back( new Binding("weapon 3",		"Select Weapon 3",				ID_WEAPON_3,		'3',			-1,			-1, -1) );
	bindings_.push_back( new Binding("weapon 4",		"Select Weapon 4",				ID_WEAPON_4,		'4',			-1,			-1, -1) );
	bindings_.push_back( new Binding("weapon 5",		"Select Weapon 5",				ID_WEAPON_5,		'5',			-1,			-1, -1) );
	bindings_.push_back( new Binding("weapon 6",		"Select Weapon 6",				ID_WEAPON_6,		'6',			-1,			-1, -1) );
	bindings_.push_back( new Binding("+button10",		"Fire Flare",					ID_WEAPON_FLARE,	K_SPACE,		-1,			-1, -1) );
	bindings_.push_back( new Binding("encyclopedia",	"Encyclopedia",					ID_ENCYC,			-1,				-1,			-1, -1) );
	bindings_.push_back( new Binding("teamselect",		"Team Selection",				ID_TEAMSEL,			-1,				-1,			-1, -1) );
	bindings_.push_back( new Binding("vehicleselect",	"Vehicle Selection",			ID_VEHSEL,			'\\',			-1,			-1, -1) );
	bindings_.push_back( new Binding("gps",				"GPS on/off",					ID_GPS,				'f',			-1,			-1, -1) );
	bindings_.push_back( new Binding("toggleview",		"Toggle Camera",				ID_TOGGLEVIEW,		-1,				-1,			-1, -1) );
	bindings_.push_back( new Binding("+cameraup",		"Camera Up",					ID_CAMERAUP,		-1,				-1,			-1, -1) );
	bindings_.push_back( new Binding("+cameradown",		"Camera Down",					ID_CAMERADOWN,		-1,				-1,			-1, -1) );
	bindings_.push_back( new Binding("+zoomin",			"Zoom In",						ID_ZOOMIN,			-1,				-1,			-1, -1) );
	bindings_.push_back( new Binding("+zoomout",		"Zoom Out",						ID_ZOOMOUT,			-1,				-1,			-1, -1) );
	bindings_.push_back( new Binding("contact_tower",	"Contact Tower",				ID_CONTACTTOWER,	K_BACKSPACE,	-1,			-1, -1) );
	bindings_.push_back( new Binding("radar",			"RADAR on/off",					ID_RADAR,			'r',			-1,			-1, -1) );
	bindings_.push_back( new Binding("extinfo",			"Ext.Info on/off",				ID_EXTINFO,			'-',			-1,			-1, -1) );
	bindings_.push_back( new Binding("radarrange",		"Toggle RADAR Range",			ID_RADARRANGE,		'=',			-1,			-1, -1) );
	bindings_.push_back( new Binding("mfd1_mode",		"Cycle MFD1 Mode",				ID_CYCLEMFD1,		-1,				-1,			-1, -1) );
	bindings_.push_back( new Binding("mfd2_mode",		"Cycle MFD2 Mode",				ID_CYCLEMFD2,		-1,				-1,			-1, -1) );
	bindings_.push_back( new Binding("eject",			"Eject",						ID_EJECT,			-1,				'o',		-1,	-1) );
	
	bindings_.push_back( new Binding("+AP_forward", 		"Forward (Max)",			ID_FORWARD,			'w',			-1,			-1, -1) );
	bindings_.push_back( new Binding("+AP_back", 			"Back/Stop (Min)",			ID_BACKPEDAL,		's',			-1,			-1, -1) );
	bindings_.push_back( new Binding("+AP_moveleft", 		"Turn left (tank)",			ID_MOVELEFT,		'a',			-1,			-1, -1) );
	bindings_.push_back( new Binding("+AP_moveright", 		"Turn right (tank)",		ID_MOVERIGHT,		'd',			-1,			-1, -1) );
	bindings_.push_back( new Binding("+AP_moveup",			"up / jump",				ID_MOVEUP,			-1,				-1,			-1, -1) );
	bindings_.push_back( new Binding("+AP_movedown",		"down / crouch",			ID_MOVEDOWN,		-1,				-1,			-1, -1) );
	bindings_.push_back( new Binding("+AP_left", 			"Left (view)",				ID_LEFT,			K_LEFTARROW,	-1,			-1, -1) );
	bindings_.push_back( new Binding("+AP_right", 			"Right (view)", 			ID_RIGHT,			K_RIGHTARROW,	-1,			-1, -1) );
	bindings_.push_back( new Binding("+AP_lookup", 			"Up (view)",				ID_LOOKUP,			K_PGUP,			-1,			-1, -1) );
	bindings_.push_back( new Binding("+AP_lookdown", 		"Down (view)",				ID_LOOKDOWN,		K_PGDN,			-1,			-1, -1) );
	bindings_.push_back( new Binding("+AP_zoom", 			"Zoom View",				ID_ZOOMVIEW,		K_SHIFT,		-1,			-1, -1) );
	bindings_.push_back( new Binding("+AP_attack", 			"Fire MG",					ID_ATTACK,			K_CTRL,			-1,			-1, -1) );
	bindings_.push_back( new Binding("+AP_button2", 		"Fire Main Weapon",			ID_ATTACK2,			K_ENTER,		K_MOUSE4,	-1, -1) );
	bindings_.push_back( new Binding("+AP_button7",			"Landing Gear",				ID_GEAR,			'g',			-1,			-1, -1) );
	bindings_.push_back( new Binding("+AP_button8",	 		"(Speed)Brakes",			ID_BRAKE,			K_ALT,			-1,			-1, -1) );
	bindings_.push_back( new Binding("+AP_button9",			"Freelook Camera",			ID_FREECAM,			-1,				-1,			-1, -1) );
	bindings_.push_back( new Binding("+AP_button5", 		"Throttle Up",				ID_INCREASE,		'q',			-1,			-1, -1) );
	bindings_.push_back( new Binding("+AP_button6", 		"Throttle Down",			ID_DECREASE,		'e',			-1,			-1, -1) );
	bindings_.push_back( new Binding("AP_unlock",			"Unlock Target",			ID_UNLOCK,			K_CTRL,			-1,			-1, -1) );
	bindings_.push_back( new Binding("+AP_bombcam",			"Bomb Camera",				ID_BOMBCAMERA,		-1,				-1,			-1, -1) );
	
	bindings_.push_back( new Binding("+Heli_forward", 		"Forward (Max)",			ID_FORWARD,			'w',			-1,			-1, -1) );
	bindings_.push_back( new Binding("+Heli_back", 			"Back/Stop (Min)",			ID_BACKPEDAL,		's',			-1,			-1, -1) );
	bindings_.push_back( new Binding("+Heli_moveleft", 		"Turn left (tank)",			ID_MOVELEFT,		'a',			-1,			-1, -1) );
	bindings_.push_back( new Binding("+Heli_moveright", 	"Turn right (tank)",		ID_MOVERIGHT,		'd',			-1,			-1, -1) );
	bindings_.push_back( new Binding("+Heli_moveup",		"up / jump",				ID_MOVEUP,			-1,				-1,			-1, -1) );
	bindings_.push_back( new Binding("+Heli_movedown",		"down / crouch",			ID_MOVEDOWN,		-1,				-1,			-1, -1) );
	bindings_.push_back( new Binding("+Heli_left", 			"Left (view)",				ID_LEFT,			K_LEFTARROW,	-1,			-1, -1) );
	bindings_.push_back( new Binding("+Heli_right", 		"Right (view)", 			ID_RIGHT,			K_RIGHTARROW,	-1,			-1, -1) );
	bindings_.push_back( new Binding("+Heli_lookup", 		"Up (view)",				ID_LOOKUP,			K_PGUP,			-1,			-1, -1) );
	bindings_.push_back( new Binding("+Heli_lookdown", 		"Down (view)",				ID_LOOKDOWN,		K_PGDN,			-1,			-1, -1) );
	bindings_.push_back( new Binding("+Heli_zoom", 			"Zoom View",				ID_ZOOMVIEW,		K_SHIFT,		-1,			-1, -1) );
	bindings_.push_back( new Binding("+Heli_attack", 		"Fire MG",					ID_ATTACK,			K_CTRL,			-1,			-1, -1) );
	bindings_.push_back( new Binding("+Heli_button2", 		"Fire Main Weapon",			ID_ATTACK2,			K_ENTER,		K_MOUSE4,	-1, -1) );
	bindings_.push_back( new Binding("+Heli_button7",		"Landing Gear",				ID_GEAR,			'g',			-1,			-1, -1) );
	bindings_.push_back( new Binding("+Heli_button8",	 	"(Speed)Brakes",			ID_BRAKE,			K_ALT,			-1,			-1, -1) );
	bindings_.push_back( new Binding("+Heli_button9",	 	"Freelook Camera",			ID_FREECAM,			-1,				-1,			-1, -1) );
	bindings_.push_back( new Binding("+Heli_button5", 		"Throttle Up",				ID_INCREASE,		'q',			-1,			-1, -1) );
	bindings_.push_back( new Binding("+Heli_button6", 		"Throttle Down",			ID_DECREASE,		'e',			-1,			-1, -1) );
	bindings_.push_back( new Binding("Heli_unlock",			"Unlock Target",			ID_UNLOCK,			K_CTRL,			-1,			-1, -1) );

	bindings_.push_back( new Binding("+GV_forward", 		"Forward (Max)",			ID_FORWARD,			'w',			-1,			-1, -1) );
	bindings_.push_back( new Binding("+GV_back", 			"Back/Stop (Min)",			ID_BACKPEDAL,		's',			-1,			-1, -1) );
	bindings_.push_back( new Binding("+GV_moveleft", 		"Turn left",				ID_MOVELEFT,		'a',			-1,			-1, -1) );
	bindings_.push_back( new Binding("+GV_moveright", 		"Turn right",				ID_MOVERIGHT,		'd',			-1,			-1, -1) );
	bindings_.push_back( new Binding("+GV_moveup",			"up",						ID_MOVEUP,			-1,				-1,			-1, -1) );
	bindings_.push_back( new Binding("+GV_movedown",		"down / crouch",			ID_MOVEDOWN,		-1,				-1,			-1, -1) );
	bindings_.push_back( new Binding("+GV_left", 			"Left (view)",				ID_LEFT,			K_LEFTARROW,	-1,			-1, -1) );
	bindings_.push_back( new Binding("+GV_right", 			"Right (view)", 			ID_RIGHT,			K_RIGHTARROW,	-1,			-1, -1) );
	bindings_.push_back( new Binding("+GV_lookup", 			"Up (view)",				ID_LOOKUP,			K_PGUP,			-1,			-1, -1) );
	bindings_.push_back( new Binding("+GV_lookdown", 		"Down (view)",				ID_LOOKDOWN,		K_PGDN,			-1,			-1, -1) );
	bindings_.push_back( new Binding("+GV_zoom", 			"Zoom View",				ID_ZOOMVIEW,		K_SHIFT,		-1,			-1, -1) );
	bindings_.push_back( new Binding("+GV_attack", 			"Fire MG",					ID_ATTACK,			K_CTRL,			-1,			-1, -1) );
	bindings_.push_back( new Binding("+GV_button2", 		"Fire Main Weapon",			ID_ATTACK2,			K_ENTER,		K_MOUSE4,	-1, -1) );
	bindings_.push_back( new Binding("+GV_button8",	 		"Brakes",					ID_BRAKE,			K_ALT,			-1,			-1, -1) );
	bindings_.push_back( new Binding("+GV_button9",	 		"Freelook Camera",			ID_FREECAM,			-1,				-1,			-1, -1) );
	bindings_.push_back( new Binding("+GV_button5", 		"Throttle Up",				ID_INCREASE,		'q',			-1,			-1, -1) );
	bindings_.push_back( new Binding("+GV_button6", 		"Throttle Down",			ID_DECREASE,		'e',			-1,			-1, -1) );
	bindings_.push_back( new Binding("GV_unlock",			"Unlock Target",			ID_UNLOCK,			K_CTRL,			-1,			-1, -1) );
}



ControlUtils::~ControlUtils()
{
	for( size_t i = 0; i < bindings_.size(); ++i )
	{	
		delete bindings_[i];
		bindings_[i] = 0;
	}
	bindings_.clear();
}

void
ControlUtils::setUtils( UI_Utils* utils )
{
	utils_ = utils;
}

void 
ControlUtils::getKeyAssignment( const char* command, int* twokeys )
{
	int		count;
	int		j;
	char	b[256];

	twokeys[0] = twokeys[1] = -1;
	count = 0;

	for ( j = 0; j < 256; j++ )
	{
		utils_->getDisplayContext()->getBindingBuf( j, b, 256 );
		if ( *b == 0 ) 
			continue;

		if ( !Q_stricmp( b, command ) ) 
		{
			twokeys[count] = j;
			count++;
			if (count == 2) 
				break;
		}
	}
}

void 
ControlUtils::getConfig()
{
	int		i;
	int		twokeys[2];
	size_t	bindCount = bindings_.size();

	// iterate each command, get its numeric binding
	for( i=0; i < bindCount; i++ )
	{
		getKeyAssignment( bindings_[i]->command_, twokeys );

		bindings_[i]->bind1_ = twokeys[0];
		bindings_[i]->bind2_ = twokeys[1];
	}
}

void 
ControlUtils::setConfig( bool restart )
{
	int		i;
	size_t	bindCount = bindings_.size();

	// iterate each command, get its numeric binding
	for( i=0; i < bindCount; i++ )
	{
		if( bindings_[i]->bind1_ != -1 )
		{	
			utils_->getDisplayContext()->setBinding( bindings_[i]->bind1_, bindings_[i]->command_ );

			if( bindings_[i]->bind2_ != -1 )
				utils_->getDisplayContext()->setBinding( bindings_[i]->bind2_, bindings_[i]->command_ );
		}
	}

	utils_->getDisplayContext()->executeText( EXEC_APPEND, "in_restart\n" );
}

void 
ControlUtils::setDefaults()
{
	int	i;
	size_t bindCount = bindings_.size();

	// iterate each command, set its default binding
	for( i=0; i < bindCount; i++ )
	{
		bindings_[i]->bind1_ = bindings_[i]->defaultbind1_;
		bindings_[i]->bind2_ = bindings_[i]->defaultbind2_;
	}
}

int 
ControlUtils::bindingIDFromName( const char *name ) 
{
	int i;
	size_t bindCount = bindings_.size();

	for (i=0; i < bindCount; i++)
	{
		if( Q_stricmp(name, bindings_[i]->command_) == 0 ) 
		{
			return i;
		}
	}
	return -1;
}

void 
ControlUtils::bindingFromName( const char *cvar ) 
{
	int	i, b1, b2;
	size_t bindCount = bindings_.size();

	// iterate each command, set its default binding
	for( i=0; i < bindCount; i++ )
	{
		if (Q_stricmp(cvar, bindings_[i]->command_) == 0) 
		{
			b1 = bindings_[i]->bind1_;
			if (b1 == -1) 
				break;

			utils_->getDisplayContext()->keynumToStringBuf( b1, nameBind1_, 32 );
			Q_strupr( nameBind1_ );

			b2 = bindings_[i]->bind2_;
			if (b2 != -1)
			{
				utils_->getDisplayContext()->keynumToStringBuf( b2, nameBind2_, 32 );
				Q_strupr( nameBind2_ );
				strcat( nameBind1_, " or " );
				strcat( nameBind1_, nameBind2_ );
			}
			return;
		}
	}
	strcpy( nameBind1_, "???" );
}

ControlUtils::Binding::Binding( const char* command,
								const char* label,
								int id,
								int defaultbind1,
								int defaultbind2,
								int bind1,
								int bind2 ) :
	command_(command),
	label_(label),
	id_(id),
	defaultbind1_(defaultbind1),
	defaultbind2_(defaultbind2),
	bind1_(bind1),
	bind2_(bind2)
{
}

ControlUtils::Binding::~Binding()
{
}

