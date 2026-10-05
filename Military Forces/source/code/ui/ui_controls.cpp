#include "ui_controls.h"
#include "keycodes.h"
#include "ui_utils.h"


UI_Utils* ControlUtils::utils_ = nullptr;


ControlUtils::ControlUtils()
{
	bindings_.emplace_back("+scores",			"Show Scoreboard",				ID_SHOWSCORES,		K_TAB,			-1,			-1, -1 );
	bindings_.emplace_back("weapprev",		"Previous Weapon",				ID_WEAPPREV,		'[',			-1,			-1, -1 );
	bindings_.emplace_back("weapnext", 		"Next Weapon",					ID_WEAPNEXT,		']',			-1,			-1, -1 );
	bindings_.emplace_back("messagemode", 	"Chat",							ID_CHAT,			't',			-1,			-1, -1 );
	bindings_.emplace_back("messagemode2", 	"Chat - Team",					ID_CHAT2,			'y',			-1,			-1, -1 );
	bindings_.emplace_back("messagemode3", 	"Chat - Target",				ID_CHAT3,			'u',			-1,			-1, -1 );
	bindings_.emplace_back("messagemode4", 	"Chat - Attacker",				ID_CHAT4,			'i',			-1,			-1, -1 );
	bindings_.emplace_back("cmessagemode", 	"MFQ3 Chat",					ID_CHAT,			't',			-1,			-1, -1 );
	bindings_.emplace_back("cmessagemode2", 	"MFQ3 Chat - Team",				ID_CHAT2,			-1,				-1,			-1, -1 );
	bindings_.emplace_back("cmessagemode3", 	"MFQ3 Chat - Target",			ID_CHAT3,			-1,				-1,			-1, -1 );
	bindings_.emplace_back("cmessagemode4", 	"MFQ3 Chat - Attacker",			ID_CHAT4,			-1,				-1,			-1, -1 );
	bindings_.emplace_back("weapon 0",		"Select MG",					ID_WEAPON_MG,		'0',			-1,			-1, -1 );
	bindings_.emplace_back("weapon 1",		"Select Weapon 1",				ID_WEAPON_1,		'1',			-1,			-1, -1 );
	bindings_.emplace_back("weapon 2",		"Select Weapon 2",				ID_WEAPON_2,		'2',			-1,			-1, -1 );
	bindings_.emplace_back("weapon 3",		"Select Weapon 3",				ID_WEAPON_3,		'3',			-1,			-1, -1 );
	bindings_.emplace_back("weapon 4",		"Select Weapon 4",				ID_WEAPON_4,		'4',			-1,			-1, -1 );
	bindings_.emplace_back("weapon 5",		"Select Weapon 5",				ID_WEAPON_5,		'5',			-1,			-1, -1 );
	bindings_.emplace_back("weapon 6",		"Select Weapon 6",				ID_WEAPON_6,		'6',			-1,			-1, -1 );
	bindings_.emplace_back("+button10",		"Fire Flare",					ID_WEAPON_FLARE,	K_SPACE,		-1,			-1, -1 );
	bindings_.emplace_back("encyclopedia",	"Encyclopedia",					ID_ENCYC,			-1,				-1,			-1, -1 );
	bindings_.emplace_back("teamselect",		"Team Selection",				ID_TEAMSEL,			-1,				-1,			-1, -1 );
	bindings_.emplace_back("vehicleselect",	"Vehicle Selection",			ID_VEHSEL,			'\\',			-1,			-1, -1 );
	bindings_.emplace_back("gps",				"GPS on/off",					ID_GPS,				'f',			-1,			-1, -1 );
	bindings_.emplace_back("toggleview",		"Toggle Camera",				ID_TOGGLEVIEW,		-1,				-1,			-1, -1 );
	bindings_.emplace_back("+cameraup",		"Camera Up",					ID_CAMERAUP,		-1,				-1,			-1, -1 );
	bindings_.emplace_back("+cameradown",		"Camera Down",					ID_CAMERADOWN,		-1,				-1,			-1, -1 );
	bindings_.emplace_back("+zoomin",			"Zoom In",						ID_ZOOMIN,			-1,				-1,			-1, -1 );
	bindings_.emplace_back("+zoomout",		"Zoom Out",						ID_ZOOMOUT,			-1,				-1,			-1, -1 );
	bindings_.emplace_back("contact_tower",	"Contact Tower",				ID_CONTACTTOWER,	K_BACKSPACE,	-1,			-1, -1 );
	bindings_.emplace_back("radar",			"RADAR on/off",					ID_RADAR,			-1,				-1,			-1, -1, 'r' );
	bindings_.emplace_back("extinfo",			"Ext.Info on/off",				ID_EXTINFO,			'-',			-1,			-1, -1 );
	// one key for both radar jobs: a tap steps the range, holding it switches the radar
	bindings_.emplace_back("radarrange",		"RADAR range",					ID_RADARRANGE,		'r',			-1,			-1, -1 );
	bindings_.emplace_back("mfd1_mode",		"Cycle MFD1 Mode",				ID_CYCLEMFD1,		-1,				-1,			-1, -1 );
	bindings_.emplace_back("mfd2_mode",		"Cycle MFD2 Mode",				ID_CYCLEMFD2,		-1,				-1,			-1, -1 );
	bindings_.emplace_back("eject",			"Eject",						ID_EJECT,			-1,				'o',		-1,	-1 );

	bindings_.emplace_back("+forward", 		"Forward (Max)",				ID_FORWARD,			'w',			-1,			-1, -1 );
	bindings_.emplace_back("+back", 			"Back/Stop (Min)",				ID_BACKPEDAL,		's',			-1,			-1, -1 );
	bindings_.emplace_back("+moveleft", 		"Turn left (tank)",				ID_MOVELEFT,		'a',			-1,			-1, -1 );
	bindings_.emplace_back("+moveright", 		"Turn right (tank)",			ID_MOVERIGHT,		'd',			-1,			-1, -1 );
	bindings_.emplace_back("+moveup",			"up / jump",					ID_MOVEUP,			-1,				-1,			-1, -1 );
	bindings_.emplace_back("+movedown",		"down / crouch",				ID_MOVEDOWN,		-1,				-1,			-1, -1 );
	bindings_.emplace_back("+left", 			"Left (view)",					ID_LEFT,			K_LEFTARROW,	-1,			-1, -1 );
	bindings_.emplace_back("+right", 			"Right (view)", 				ID_RIGHT,			K_RIGHTARROW,	-1,			-1, -1 );
	bindings_.emplace_back("+lookup", 		"Up (view)",					ID_LOOKUP,			K_PGUP,			-1,			-1, -1 );
	bindings_.emplace_back("+lookdown", 		"Down (view)",					ID_LOOKDOWN,		K_PGDN,			-1,			-1, -1 );
	bindings_.emplace_back("+zoom", 			"Zoom View",					ID_ZOOMVIEW,		K_SHIFT,		-1,			-1, -1 );
	bindings_.emplace_back("+attack", 		"Fire MG",						ID_ATTACK,			K_CTRL,			-1,			-1, -1 );
	bindings_.emplace_back("+button2", 		"Fire Main Weapon",				ID_ATTACK2,			K_ENTER,		K_MOUSE4,	-1, -1 );
	bindings_.emplace_back("+button7",		"Landing Gear",					ID_GEAR,			'g',			-1,			-1, -1 );
	bindings_.emplace_back("+button8",	 	"(Speed)Brakes",				ID_BRAKE,			K_ALT,			-1,			-1, -1 );
	bindings_.emplace_back("+button9",	 	"Freelook Camera",				ID_FREECAM,			-1,				-1,			-1, -1 );
	bindings_.emplace_back("+button5", 		"Throttle Up",					ID_INCREASE,		'q',			-1,			-1, -1 );
	bindings_.emplace_back("+button6", 		"Throttle Down",				ID_DECREASE,		'e',			-1,			-1, -1 );
	bindings_.emplace_back("unlock",			"Unlock Target",				ID_UNLOCK,			K_CTRL,			-1,			-1, -1 );
	bindings_.emplace_back("+bombcam",		"Bomb Camera",					ID_BOMBCAMERA,		-1,				-1,			-1, -1 );
	

	// until this is finished and works it will be disabled
/*
	bindings_.emplace_back("+AP_forward", 		"Forward (Max)",			ID_FORWARD,			'w',			-1,			-1, -1 );
	bindings_.emplace_back("+AP_back", 			"Back/Stop (Min)",			ID_BACKPEDAL,		's',			-1,			-1, -1 );
	bindings_.emplace_back("+AP_moveleft", 		"Turn left (tank)",			ID_MOVELEFT,		'a',			-1,			-1, -1 );
	bindings_.emplace_back("+AP_moveright", 		"Turn right (tank)",		ID_MOVERIGHT,		'd',			-1,			-1, -1 );
	bindings_.emplace_back("+AP_moveup",			"up / jump",				ID_MOVEUP,			-1,				-1,			-1, -1 );
	bindings_.emplace_back("+AP_movedown",		"down / crouch",			ID_MOVEDOWN,		-1,				-1,			-1, -1 );
	bindings_.emplace_back("+AP_left", 			"Left (view)",				ID_LEFT,			K_LEFTARROW,	-1,			-1, -1 );
	bindings_.emplace_back("+AP_right", 			"Right (view)", 			ID_RIGHT,			K_RIGHTARROW,	-1,			-1, -1 );
	bindings_.emplace_back("+AP_lookup", 			"Up (view)",				ID_LOOKUP,			K_PGUP,			-1,			-1, -1 );
	bindings_.emplace_back("+AP_lookdown", 		"Down (view)",				ID_LOOKDOWN,		K_PGDN,			-1,			-1, -1 );
	bindings_.emplace_back("+AP_zoom", 			"Zoom View",				ID_ZOOMVIEW,		K_SHIFT,		-1,			-1, -1 );
	bindings_.emplace_back("+AP_attack", 			"Fire MG",					ID_ATTACK,			K_CTRL,			-1,			-1, -1 );
	bindings_.emplace_back("+AP_button2", 		"Fire Main Weapon",			ID_ATTACK2,			K_ENTER,		K_MOUSE4,	-1, -1 );
	bindings_.emplace_back("+AP_button7",			"Landing Gear",				ID_GEAR,			'g',			-1,			-1, -1 );
	bindings_.emplace_back("+AP_button8",	 		"(Speed)Brakes",			ID_BRAKE,			K_ALT,			-1,			-1, -1 );
	bindings_.emplace_back("+AP_button9",			"Freelook Camera",			ID_FREECAM,			-1,				-1,			-1, -1 );
	bindings_.emplace_back("+AP_button5", 		"Throttle Up",				ID_INCREASE,		'q',			-1,			-1, -1 );
	bindings_.emplace_back("+AP_button6", 		"Throttle Down",			ID_DECREASE,		'e',			-1,			-1, -1 );
	bindings_.emplace_back("AP_unlock",			"Unlock Target",			ID_UNLOCK,			K_CTRL,			-1,			-1, -1 );
	bindings_.emplace_back("+AP_bombcam",			"Bomb Camera",				ID_BOMBCAMERA,		-1,				-1,			-1, -1 );
	
	bindings_.emplace_back("+Heli_forward", 		"Forward (Max)",			ID_FORWARD,			'w',			-1,			-1, -1 );
	bindings_.emplace_back("+Heli_back", 			"Back/Stop (Min)",			ID_BACKPEDAL,		's',			-1,			-1, -1 );
	bindings_.emplace_back("+Heli_moveleft", 		"Turn left (tank)",			ID_MOVELEFT,		'a',			-1,			-1, -1 );
	bindings_.emplace_back("+Heli_moveright", 	"Turn right (tank)",		ID_MOVERIGHT,		'd',			-1,			-1, -1 );
	bindings_.emplace_back("+Heli_moveup",		"up / jump",				ID_MOVEUP,			-1,				-1,			-1, -1 );
	bindings_.emplace_back("+Heli_movedown",		"down / crouch",			ID_MOVEDOWN,		-1,				-1,			-1, -1 );
	bindings_.emplace_back("+Heli_left", 			"Left (view)",				ID_LEFT,			K_LEFTARROW,	-1,			-1, -1 );
	bindings_.emplace_back("+Heli_right", 		"Right (view)", 			ID_RIGHT,			K_RIGHTARROW,	-1,			-1, -1 );
	bindings_.emplace_back("+Heli_lookup", 		"Up (view)",				ID_LOOKUP,			K_PGUP,			-1,			-1, -1 );
	bindings_.emplace_back("+Heli_lookdown", 		"Down (view)",				ID_LOOKDOWN,		K_PGDN,			-1,			-1, -1 );
	bindings_.emplace_back("+Heli_zoom", 			"Zoom View",				ID_ZOOMVIEW,		K_SHIFT,		-1,			-1, -1 );
	bindings_.emplace_back("+Heli_attack", 		"Fire MG",					ID_ATTACK,			K_CTRL,			-1,			-1, -1 );
	bindings_.emplace_back("+Heli_button2", 		"Fire Main Weapon",			ID_ATTACK2,			K_ENTER,		K_MOUSE4,	-1, -1 );
	bindings_.emplace_back("+Heli_button7",		"Landing Gear",				ID_GEAR,			'g',			-1,			-1, -1 );
	bindings_.emplace_back("+Heli_button8",	 	"(Speed)Brakes",			ID_BRAKE,			K_ALT,			-1,			-1, -1 );
	bindings_.emplace_back("+Heli_button9",	 	"Freelook Camera",			ID_FREECAM,			-1,				-1,			-1, -1 );
	bindings_.emplace_back("+Heli_button5", 		"Throttle Up",				ID_INCREASE,		'q',			-1,			-1, -1 );
	bindings_.emplace_back("+Heli_button6", 		"Throttle Down",			ID_DECREASE,		'e',			-1,			-1, -1 );
	bindings_.emplace_back("Heli_unlock",			"Unlock Target",			ID_UNLOCK,			K_CTRL,			-1,			-1, -1 );

	bindings_.emplace_back("+GV_forward", 		"Forward (Max)",			ID_FORWARD,			'w',			-1,			-1, -1 );
	bindings_.emplace_back("+GV_back", 			"Back/Stop (Min)",			ID_BACKPEDAL,		's',			-1,			-1, -1 );
	bindings_.emplace_back("+GV_moveleft", 		"Turn left",				ID_MOVELEFT,		'a',			-1,			-1, -1 );
	bindings_.emplace_back("+GV_moveright", 		"Turn right",				ID_MOVERIGHT,		'd',			-1,			-1, -1 );
	bindings_.emplace_back("+GV_moveup",			"up",						ID_MOVEUP,			-1,				-1,			-1, -1 );
	bindings_.emplace_back("+GV_movedown",		"down / crouch",			ID_MOVEDOWN,		-1,				-1,			-1, -1 );
	bindings_.emplace_back("+GV_left", 			"Left (view)",				ID_LEFT,			K_LEFTARROW,	-1,			-1, -1 );
	bindings_.emplace_back("+GV_right", 			"Right (view)", 			ID_RIGHT,			K_RIGHTARROW,	-1,			-1, -1 );
	bindings_.emplace_back("+GV_lookup", 			"Up (view)",				ID_LOOKUP,			K_PGUP,			-1,			-1, -1 );
	bindings_.emplace_back("+GV_lookdown", 		"Down (view)",				ID_LOOKDOWN,		K_PGDN,			-1,			-1, -1 );
	bindings_.emplace_back("+GV_zoom", 			"Zoom View",				ID_ZOOMVIEW,		K_SHIFT,		-1,			-1, -1 );
	bindings_.emplace_back("+GV_attack", 			"Fire MG",					ID_ATTACK,			K_CTRL,			-1,			-1, -1 );
	bindings_.emplace_back("+GV_button2", 		"Fire Main Weapon",			ID_ATTACK2,			K_ENTER,		K_MOUSE4,	-1, -1 );
	bindings_.emplace_back("+GV_button8",	 		"Brakes",					ID_BRAKE,			K_ALT,			-1,			-1, -1 );
	bindings_.emplace_back("+GV_button9",	 		"Freelook Camera",			ID_FREECAM,			-1,				-1,			-1, -1 );
	bindings_.emplace_back("+GV_button5", 		"Throttle Up",				ID_INCREASE,		'q',			-1,			-1, -1 );
	bindings_.emplace_back("+GV_button6", 		"Throttle Down",			ID_DECREASE,		'e',			-1,			-1, -1 );
	bindings_.emplace_back("GV_unlock",			"Unlock Target",			ID_UNLOCK,			K_CTRL,			-1,			-1, -1 );
*/
}



ControlUtils::~ControlUtils()
= default;

void
ControlUtils::setUtils( UI_Utils* utils )
{
	utils_ = utils;
}

// a key bound to "+hold <tap> | <hold>" runs one command on a quick tap and another when kept down
static bool splitHold( const char* binding, char* tap, char* hold, int size )
{
	if( Q_stricmpn( binding, "+hold ", 6 ) ) 
		return false;

	const char* text = binding + 6;
	const char* bar = strchr( text, '|' );
	const char* space = strchr( text, ' ' );
	const char* split = bar ? bar : space ? space : text + strlen( text );
	Q_strncpyz( tap, text, std::min( size, static_cast<int>( split - text ) + 1 ) );
	Q_strncpyz( hold, *split ? split + 1 : "", size );
	for( char* s : { tap, hold } ) 
	{
		while( *s == ' ' ) 
			memmove( s, s + 1, strlen( s ) );
		for( size_t n = strlen( s ); n && s[n - 1] == ' '; n-- ) 
			s[n - 1] = 0;
	}
	return true;
}

void 
ControlUtils::getConfig()
{
	char	b[256], tap[256], hold[256];

	for( Binding& binding : bindings_ )
		binding.bind1_ = binding.bind2_ = binding.hold_ = -1;

	for( int key = 0; key < K_LAST_KEY; key++ )
	{
		utils_->dc_->getBindingBuf( key, b, 256 );
		if( *b == 0 ) 
			continue;

		const bool held = splitHold( b, tap, hold, 256 );
		for( Binding& binding : bindings_ )
		{
			if( !Q_stricmp( held ? tap : b, binding.command_ ) )
			{
				if( binding.bind1_ == -1 ) 
					binding.bind1_ = key;
				else if( binding.bind2_ == -1 ) 
					binding.bind2_ = key;
			}
			if( held && binding.hold_ == -1 && !Q_stricmp( hold, binding.command_ ) )
				binding.hold_ = key;
		}
	}
}

void 
ControlUtils::setConfig( bool restart )
{
	char	b[256], tap[256], hold[256];

	for( int key = 0; key < K_LAST_KEY; key++ )
	{
		const Binding* tapped = nullptr;
		const Binding* held = nullptr;
		for( const Binding& binding : bindings_ )
		{
			if( !tapped && ( binding.bind1_ == key || binding.bind2_ == key ) ) 
				tapped = &binding;
			if( !held && binding.hold_ == key ) 
				held = &binding;
		}

		if( held )
			utils_->dc_->setBinding( key, va( "+hold %s | %s", tapped ? tapped->command_ : "", held->command_ ) );
		else if( tapped )
			utils_->dc_->setBinding( key, tapped->command_ );
		else
		{
			// a key no row claims any more loses the command this menu gave it
			utils_->dc_->getBindingBuf( key, b, 256 );
			if( *b && ( splitHold( b, tap, hold, 256 ) || bindingIDFromName( b ) != -1 ) )
				utils_->dc_->setBinding( key, "" );
		}
	}

	utils_->dc_->executeText( EXEC_APPEND, "in_restart\n" );
}

void 
ControlUtils::setDefaults()
{
	int	i;
	size_t bindCount = bindings_.size();

	// iterate each command, set its default binding
	for( i=0; i < bindCount; i++ )
	{
		bindings_[i].bind1_ = bindings_[i].defaultbind1_;
		bindings_[i].bind2_ = bindings_[i].defaultbind2_;
		bindings_[i].hold_ = bindings_[i].defaulthold_;
	}
}

int 
ControlUtils::bindingIDFromName( const char *name ) 
{
	int i;
	size_t bindCount = bindings_.size();

	for (i=0; i < bindCount; i++)
	{
		if( Q_stricmp(name, bindings_[i].command_) == 0 ) 
		{
			return i;
		}
	}
	return -1;
}

void 
ControlUtils::bindingFromName( const char *cvar ) 
{
	char	name[32];

	nameBind1_[0] = 0;
	const int id = bindingIDFromName( cvar );
	if( id != -1 )
	{
		const Binding& binding = bindings_[id];
		const struct { int key; const char* prefix; } keys[] = { { binding.bind1_, "" }, { binding.bind2_, "" }, { binding.hold_, "hold " } };
		for( const auto& entry : keys )
		{
			if( entry.key == -1 ) 
				continue;

			utils_->dc_->keynumToStringBuf( entry.key, name, sizeof(name) );
			Q_strupr( name );
			if( nameBind1_[0] ) 
				Q_strcat( nameBind1_, sizeof(nameBind1_), " or " );
			Q_strcat( nameBind1_, sizeof(nameBind1_), entry.prefix );
			Q_strcat( nameBind1_, sizeof(nameBind1_), name );
		}
	}
	if( !nameBind1_[0] ) 
		strcpy( nameBind1_, "???" );
}

ControlUtils::Binding::Binding( const char* command,
								const char* label,
								int id,
								int defaultbind1,
								int defaultbind2,
								int bind1,
								int bind2,
								int defaulthold ) :
	command_(command),
	label_(label),
	id_(id),
	defaultbind1_(defaultbind1),
	defaultbind2_(defaultbind2),
	bind1_(bind1),
	bind2_(bind2),
	defaulthold_(defaulthold),
	hold_(-1)
{
}


