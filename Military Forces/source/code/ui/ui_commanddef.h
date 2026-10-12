#ifndef __UI_COMMANDDEF_H__
#define __UI_COMMANDDEF_H__

#include "ui_utils.h"


struct itemDef_t;


struct UI_Utils::Command
{
	explicit				Command( UI_Utils* utils ) : utils_( utils ) {}
	virtual					~Command() {}

	// interface
	virtual void			handler( itemDef_t *item, char** args ) = 0;

protected:
	UI_Utils*				utils_;
};

struct UI_Utils::Command_SetFocus : public Command
{
	using Command::Command;
	void			handler( itemDef_t *item, char** args ) override;
};

struct UI_Utils::Command_FadeIn : public Command
{
	using Command::Command;
	void			handler( itemDef_t *item, char** args ) override;
};

struct UI_Utils::Command_FadeOut : public Command
{
	using Command::Command;
	void			handler( itemDef_t *item, char** args ) override;
};

struct UI_Utils::Command_Show : public Command
{
	using Command::Command;
	void			handler( itemDef_t *item, char** args ) override;
};

struct UI_Utils::Command_Hide : public Command
{
	using Command::Command;
	void			handler( itemDef_t *item, char** args ) override;
};

struct UI_Utils::Command_SetColor : public Command
{
	using Command::Command;
	void			handler( itemDef_t *item, char** args ) override;
};

struct UI_Utils::Command_Open : public Command
{
	using Command::Command;
	void			handler( itemDef_t *item, char** args ) override;
};

struct UI_Utils::Command_Close : public Command
{
	using Command::Command;
	void			handler( itemDef_t *item, char** args ) override;
};

struct UI_Utils::Command_SetAsset : public Command
{
	using Command::Command;
	void			handler( itemDef_t *item, char** args ) override;
};

struct UI_Utils::Command_SetBackground : public Command
{
	using Command::Command;
	void			handler( itemDef_t *item, char** args ) override;
};

struct UI_Utils::Command_SetItemColor : public Command
{
	using Command::Command;
	void			handler( itemDef_t *item, char** args ) override;
};

struct UI_Utils::Command_SetPlayerModel : public Command
{
	using Command::Command;
	void			handler( itemDef_t *item, char** args ) override;
};

struct UI_Utils::Command_SetPlayerHead : public Command
{
	using Command::Command;
	void			handler( itemDef_t *item, char** args ) override;
};

struct UI_Utils::Command_Transition : public Command
{
	using Command::Command;
	void			handler( itemDef_t *item, char** args ) override;
};

struct UI_Utils::Command_SetCvar : public Command
{
	using Command::Command;
	void			handler( itemDef_t *item, char** args ) override;
};

struct UI_Utils::Command_Exec : public Command
{
	using Command::Command;
	void			handler( itemDef_t *item, char** args ) override;
};

struct UI_Utils::Command_Play : public Command
{
	using Command::Command;
	void			handler( itemDef_t *item, char** args ) override;
};

struct UI_Utils::Command_PlayLooped : public Command
{
	using Command::Command;
	void			handler( itemDef_t *item, char** args ) override;
};

struct UI_Utils::Command_Orbit : public Command
{
	using Command::Command;
	void			handler( itemDef_t *item, char** args ) override;
};

#endif // __UI_COMMANDDEF_H__
