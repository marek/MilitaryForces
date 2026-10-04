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
	void			handler( itemDef_t *item, char** args ) override;
};

struct UI_Utils::Command_FadeIn : public Command
{
	void			handler( itemDef_t *item, char** args ) override;
};

struct UI_Utils::Command_FadeOut : public Command
{
	void			handler( itemDef_t *item, char** args ) override;
};

struct UI_Utils::Command_Show : public Command
{
	void			handler( itemDef_t *item, char** args ) override;
};

struct UI_Utils::Command_Hide : public Command
{
	void			handler( itemDef_t *item, char** args ) override;
};

struct UI_Utils::Command_SetColor : public Command
{
	void			handler( itemDef_t *item, char** args ) override;
};

struct UI_Utils::Command_Open : public Command
{
	void			handler( itemDef_t *item, char** args ) override;
};

struct UI_Utils::Command_Close : public Command
{
	void			handler( itemDef_t *item, char** args ) override;
};

struct UI_Utils::Command_SetAsset : public Command
{
	void			handler( itemDef_t *item, char** args ) override;
};

struct UI_Utils::Command_SetBackground : public Command
{
	void			handler( itemDef_t *item, char** args ) override;
};

struct UI_Utils::Command_SetItemColor : public Command
{
	void			handler( itemDef_t *item, char** args ) override;
};

struct UI_Utils::Command_SetPlayerModel : public Command
{
	void			handler( itemDef_t *item, char** args ) override;
};

struct UI_Utils::Command_SetPlayerHead : public Command
{
	void			handler( itemDef_t *item, char** args ) override;
};

struct UI_Utils::Command_Transition : public Command
{
	void			handler( itemDef_t *item, char** args ) override;
};

struct UI_Utils::Command_SetCvar : public Command
{
	void			handler( itemDef_t *item, char** args ) override;
};

struct UI_Utils::Command_Exec : public Command
{
	void			handler( itemDef_t *item, char** args ) override;
};

struct UI_Utils::Command_Play : public Command
{
	void			handler( itemDef_t *item, char** args ) override;
};

struct UI_Utils::Command_PlayLooped : public Command
{
	void			handler( itemDef_t *item, char** args ) override;
};

struct UI_Utils::Command_Orbit : public Command
{
	void			handler( itemDef_t *item, char** args ) override;
};

#endif // __UI_COMMANDDEF_H__

