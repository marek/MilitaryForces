#include "ui_commanddef.h"
#include "ui_local.h"




UI_Utils* UI_Utils::Command::utils_ = 0;

void
UI_Utils::Command::setUtils( UI_Utils* utils )
{
	utils_ = utils;
}



void
UI_Utils::Command_SetFocus::handler( itemDef_t *item, char** args )
{
	const char *name;
	itemDef_t *focusItem;

	if (utils_->string_Parse(args, &name)) 
	{
		focusItem = utils_->menu_FindItemByName( reinterpret_cast<menuDef_t*>(item->parent), name );
		if (focusItem && 
			!(focusItem->window.flags & WINDOW_DECORATION) && 
			!(focusItem->window.flags & WINDOW_HASFOCUS)) 
		{
			utils_->menu_ClearFocus(reinterpret_cast<menuDef_t*>(item->parent));
			focusItem->window.flags |= WINDOW_HASFOCUS;
			if (focusItem->onFocus) 
				utils_->item_RunScript(focusItem, focusItem->onFocus);
			if (utils_->dc_->assets_.itemFocusSound) 
				utils_->dc_->startLocalSound( utils_->dc_->assets_.itemFocusSound, CHAN_LOCAL_SOUND );
		}
	}
}

void
UI_Utils::Command_FadeIn::handler( itemDef_t *item, char** args )
{
	const char *name;
	if (utils_->string_Parse(args, &name)) 
		utils_->menu_FadeItemByName(reinterpret_cast<menuDef_t*>(item->parent), name, false);
}

void
UI_Utils::Command_FadeOut::handler( itemDef_t *item, char** args )
{
	const char *name;
	if (utils_->string_Parse(args, &name)) 
		utils_->menu_FadeItemByName(reinterpret_cast<menuDef_t*>(item->parent), name, true);
}

void
UI_Utils::Command_Show::handler( itemDef_t *item, char** args )
{
	const char *name;
	if (utils_->string_Parse(args, &name)) 
		utils_->menu_ShowItemByName(reinterpret_cast<menuDef_t*>(item->parent), name, true);
}

void
UI_Utils::Command_Hide::handler( itemDef_t *item, char** args )
{
	const char *name;
	if (utils_->string_Parse(args, &name)) 
		utils_->menu_ShowItemByName(reinterpret_cast<menuDef_t*>(item->parent), name, false);
}

void
UI_Utils::Command_SetColor::handler( itemDef_t *item, char** args )
{
	const char *name;
	int i;
	float f;
	vec4_t *out;
	// expecting type of color to set and 4 args for the color
	if (utils_->string_Parse(args, &name)) {
		out = NULL;
		if (Q_stricmp(name, "backcolor") == 0) 
		{
			out = &item->window.backColor;
			item->window.flags |= WINDOW_BACKCOLORSET;
		} 
		else if (Q_stricmp(name, "forecolor") == 0) 
		{
			out = &item->window.foreColor;
			item->window.flags |= WINDOW_FORECOLORSET;
		} 
		else if (Q_stricmp(name, "bordercolor") == 0)
		{
			out = &item->window.borderColor;
		}

		if (out) 
		{
			for (i = 0; i < 4; i++) 
			{
				if (!utils_->float_Parse(args, &f)) 
				{
					return;
				}
				(*out)[i] = f;
			}
		}
	}
}

void
UI_Utils::Command_Open::handler( itemDef_t *item, char** args )
{
	const char *name;
	if (utils_->string_Parse(args, &name))
		utils_->menu_OpenByName(name);
}

void
UI_Utils::Command_Close::handler( itemDef_t *item, char** args )
{
	const char *name;
	if (utils_->string_Parse(args, &name))
		utils_->menu_CloseByName(name);
}

void
UI_Utils::Command_SetAsset::handler( itemDef_t *item, char** args )
{
	const char *name;
	// expecting name to set asset to
	if (utils_->string_Parse(args, &name)) 
	{
		// check for a model 
		if (item->type == ITEM_TYPE_MODEL) 
		{
		}
	}
}

void
UI_Utils::Command_SetBackground::handler( itemDef_t *item, char** args )
{
	const char *name;
	// expecting name to set asset to
	if (utils_->string_Parse(args, &name)) 
	{
		item->window.background = utils_->dc_->registerShaderNoMip(name);
	}
}

void
UI_Utils::Command_SetItemColor::handler( itemDef_t *item, char** args )
{
	const char *itemname;
	const char *name;
	vec4_t color;
	int i;
	vec4_t *out;

	// expecting type of color to set and 4 args for the color
	if (utils_->string_Parse(args, &itemname) && utils_->string_Parse(args, &name)) 
	{
		itemDef_t *item2;
		int j;
		int count = utils_->menu_ItemsMatchingGroup(reinterpret_cast<menuDef_t*>(item->parent), itemname);

		if (!utils_->color_Parse(args, &color)) 
		{
			return;
		}

		for (j = 0; j < count; j++) 
		{
			item2 = utils_->menu_GetMatchingItemByNumber(reinterpret_cast<menuDef_t*>(item->parent), j, itemname);
			if (item2 != NULL) 
			{
				out = NULL;
				if (Q_stricmp(name, "backcolor") == 0) 
				{
					out = &item2->window.backColor;
				}
				else if (Q_stricmp(name, "forecolor") == 0) 
				{
					out = &item2->window.foreColor;
					item2->window.flags |= WINDOW_FORECOLORSET;
				} 
				else if (Q_stricmp(name, "bordercolor") == 0) 
				{
					out = &item2->window.borderColor;
				}

				if (out) 
				{
					for (i = 0; i < 4; i++) 
					{
						(*out)[i] = color[i];
					}
				}
			}
		}
	}
}

void
UI_Utils::Command_SetPlayerModel::handler( itemDef_t *item, char** args )
{
	const char *name;
	if (utils_->string_Parse(args, &name)) 
	{
		utils_->dc_->setCVar("team_model", name);
	}
}

void
UI_Utils::Command_SetPlayerHead::handler( itemDef_t *item, char** args )
{
	const char *name;
	if (utils_->string_Parse(args, &name)) 
		utils_->dc_->setCVar("team_headmodel", name);
}

void
UI_Utils::Command_Transition::handler( itemDef_t *item, char** args )
{
	const char *name;
	rectDef_t	rectFrom, rectTo;
	int			time;
	float		amt;

	if (utils_->string_Parse(args, &name)) 
	{
		if ( utils_->rect_Parse(args, &rectFrom) && 
			 utils_->rect_Parse(args, &rectTo) && 
			 utils_->int_Parse(args, &time) && 
			 utils_->float_Parse(args, &amt) ) 
		{
			utils_->menu_TransitionItemByName(reinterpret_cast<menuDef_t*>(item->parent), name, rectFrom, rectTo, time, amt);
		}
	}
}

void
UI_Utils::Command_SetCvar::handler( itemDef_t *item, char** args )
{
	const char *cvar, *val;
	if (utils_->string_Parse(args, &cvar) && utils_->string_Parse(args, &val)) 
		utils_->dc_->setCVar(cvar, val);
}

void
UI_Utils::Command_Exec::handler( itemDef_t *item, char** args )
{
	const char *val;
	if (utils_->string_Parse(args, &val)) 
	{
		utils_->dc_->executeText(EXEC_APPEND, va("%s ; ", val));
	}
}

void
UI_Utils::Command_Play::handler( itemDef_t *item, char** args )
{
	const char *val;
	if (utils_->string_Parse(args, &val)) 
		utils_->dc_->startLocalSound(utils_->dc_->registerSound(val, false), CHAN_LOCAL_SOUND);
}

void
UI_Utils::Command_PlayLooped::handler( itemDef_t *item, char** args )
{
	const char *val;
	if (utils_->string_Parse(args, &val)) 
	{
		utils_->dc_->stopBackgroundTrack();
		utils_->dc_->startBackgroundTrack(val, val);
	}
}

void
UI_Utils::Command_Orbit::handler( itemDef_t *item, char** args )
{
	const char *name;
	float cx, cy, x, y;
	int time;

	if( utils_->string_Parse(args, &name) ) 
	{
		if ( utils_->float_Parse(args, &x) && 
			 utils_->float_Parse(args, &y) && 
			 utils_->float_Parse(args, &cx) && 
			 utils_->float_Parse(args, &cy) && 
			 utils_->int_Parse(args, &time) ) 
		{
			utils_->menu_OrbitItemByName(reinterpret_cast<menuDef_t*>(item->parent), name, x, y, cx, cy, time);
		}
	}
}



