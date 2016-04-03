#ifndef __UI_COMMANDDEF_H__
#define __UI_COMMANDDEF_H__

#include "ui_utils.h"


struct itemDef_t;


struct UI_Utils::Command
{
	virtual					~Command() {}

	static void				setUtils( UI_Utils* utils );

	// interface
	virtual void			handler( itemDef_t *item, char** args ) = 0;

protected:
	//data
	static UI_Utils*		utils_;
};

struct UI_Utils::Command_SetFocus : public Command
{
	virtual void			handler( itemDef_t *item, char** args );
};

struct UI_Utils::Command_FadeIn : public Command
{
	virtual void			handler( itemDef_t *item, char** args );
};

struct UI_Utils::Command_FadeOut : public Command
{
	virtual void			handler( itemDef_t *item, char** args );
};

struct UI_Utils::Command_Show : public Command
{
	virtual void			handler( itemDef_t *item, char** args );
};

struct UI_Utils::Command_Hide : public Command
{
	virtual void			handler( itemDef_t *item, char** args );
};

struct UI_Utils::Command_SetColor : public Command
{
	virtual void			handler( itemDef_t *item, char** args );
};

struct UI_Utils::Command_Open : public Command
{
	virtual void			handler( itemDef_t *item, char** args );
};

struct UI_Utils::Command_Close : public Command
{
	virtual void			handler( itemDef_t *item, char** args );
};

struct UI_Utils::Command_SetAsset : public Command
{
	virtual void			handler( itemDef_t *item, char** args );
};

struct UI_Utils::Command_SetBackground : public Command
{
	virtual void			handler( itemDef_t *item, char** args );
};

struct UI_Utils::Command_SetItemColor : public Command
{
	virtual void			handler( itemDef_t *item, char** args );
};

struct UI_Utils::Command_SetPlayerModel : public Command
{
	virtual void			handler( itemDef_t *item, char** args );
};

struct UI_Utils::Command_SetPlayerHead : public Command
{
	virtual void			handler( itemDef_t *item, char** args );
};

struct UI_Utils::Command_Transition : public Command
{
	virtual void			handler( itemDef_t *item, char** args );
};

struct UI_Utils::Command_SetCvar : public Command
{
	virtual void			handler( itemDef_t *item, char** args );
};

struct UI_Utils::Command_Exec : public Command
{
	virtual void			handler( itemDef_t *item, char** args );
};

struct UI_Utils::Command_Play : public Command
{
	virtual void			handler( itemDef_t *item, char** args );
};

struct UI_Utils::Command_PlayLooped : public Command
{
	virtual void			handler( itemDef_t *item, char** args );
};

struct UI_Utils::Command_Orbit : public Command
{
	virtual void			handler( itemDef_t *item, char** args );
};

#endif // __UI_COMMANDDEF_H__

