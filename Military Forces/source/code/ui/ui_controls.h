#ifndef __UI_CONTROLS_H__
#define __UI_CONTROLS_H__



#include <vector>

struct UI_Utils;
struct itemDef_t;

struct ControlUtils
{
	struct Binding;
						
	enum
	{
		// bindable actions
		ID_SHOWSCORES		= 0,
		ID_FORWARD			= 1,
		ID_BACKPEDAL		= 2,
		ID_MOVELEFT			= 3,
		ID_MOVERIGHT		= 4,
		ID_MOVEUP			= 5,	
		ID_MOVEDOWN			= 6,
		ID_LEFT				= 7,	
		ID_RIGHT			= 8,	
		ID_LOOKUP			= 9,	
		ID_LOOKDOWN			= 10,
		ID_ZOOMVIEW			= 11,
		ID_ATTACK			= 12,
		ID_ATTACK2			= 13,
		ID_WEAPPREV			= 14,
		ID_WEAPNEXT			= 15,
		ID_CHAT				= 16,
		ID_CHAT2			= 17,
		ID_CHAT3			= 18,
		ID_CHAT4			= 19,
		// mfq3 specific bindable actions
		ID_GEAR				= 20,
		ID_BRAKE			= 21,
		ID_FREECAM			= 22,
		ID_INCREASE			= 23,
		ID_DECREASE			= 24,
		ID_WEAPON_MG		= 25,
		ID_WEAPON_1			= 26,
		ID_WEAPON_2			= 27,
		ID_WEAPON_3			= 28,
		ID_WEAPON_4			= 29,
		ID_WEAPON_5			= 30,
		ID_WEAPON_6			= 31,
		ID_WEAPON_FLARE		= 32,
		ID_ENCYC			= 33,
		ID_VEHSEL			= 34,
		ID_GPS				= 35,
		ID_TOGGLEVIEW		= 36,
		ID_CAMERAUP			= 37,
		ID_CAMERADOWN		= 38,
		ID_ZOOMIN			= 39,
		ID_ZOOMOUT			= 40,
		ID_CONTACTTOWER 	= 41,
		ID_RADAR			= 42,
		ID_EXTINFO			= 43,
		ID_RADARRANGE		= 44,
		ID_UNLOCK			= 45,
		ID_TEAMSEL			= 46,
		ID_SPEEDUP			= 47,
		ID_BOMBCAMERA		= 48,
		ID_CYCLEMFD1		= 49,
		ID_CYCLEMFD2		= 50,
		ID_EJECT			= 51
	};

							ControlUtils();
							~ControlUtils();
	
	void					getKeyAssignment( const char *command, int* twokeys );
	void					getConfig();
	void					setConfig( bool restart );
	void					setDefaults();
	int						bindingIDFromName( const char *name );
	void					bindingFromName( const char *cvar );

	static void				setUtils( UI_Utils* utils );

	// data
	std::vector<Binding*>	bindings_;

	char					nameBind1_[32];
	char					nameBind2_[32];

	static UI_Utils*		utils_;

};

struct ControlUtils::Binding
{
					Binding( const char* command,
							const char* label,
							int id,
							int defaultbind1,
							int defaultbind2,
							int bind1,
							int bind2 );
					~Binding();

	const char*     command_;
	const char*		label_;	
	int				id_;
	int				defaultbind1_;
	int				defaultbind2_;
	int				bind1_;
	int				bind2_;

private:
	// disabled
					Binding( Binding const& );
	Binding&		operator=( Binding const& );
};








#endif // __UI_CONTROLS_H__



