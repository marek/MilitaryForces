#include "ui_utils.h"
#include "ui_commanddef.h"
#include "ui_hashutils.h"
#include "ui_keywordhash.h"
#include "ui_controls.h"
#include "ui_itemcapture.h"
#include "ui_precomp.h"




UI_PrecompilerTools	UI_Utils::precomp_;

UI_Utils::UI_Utils() : 
	menuCount_(0),
	openMenuCount_(0),
	editingField_(false),
	bindItem_(0),
	editItem_(0),
	itemCapture_(0),
	waitingForKey_(false),
	lastListBoxClickTime_(0),
	captureTool_(0),
	debugMode_(false),
	allocPoint_(0),
	outOfMemory_(0),
	strPoolIndex_(0),
	strHandleCount_(0)
{
	UI_CaptureBase::setUtils(this);
	UI_HashUtils::setUtils(this);
	hashUtils_ = new UI_HashUtils;
	ControlUtils::setUtils(this);
	controlUtils_ = new ControlUtils;
	Command::setUtils(this);
	commands_.insert(std::make_pair("setfocus", new Command_SetFocus));
	commands_.insert(std::make_pair("fadein", new Command_FadeIn));
	commands_.insert(std::make_pair("fadeout", new Command_FadeOut));
	commands_.insert(std::make_pair("show", new Command_Show));
	commands_.insert(std::make_pair("hide", new Command_Hide));
	commands_.insert(std::make_pair("setcolor", new Command_SetColor));
	commands_.insert(std::make_pair("open", new Command_Open));
	commands_.insert(std::make_pair("close", new Command_Close));
	commands_.insert(std::make_pair("setasset", new Command_SetAsset));
	commands_.insert(std::make_pair("setbackground", new Command_SetBackground));
	commands_.insert(std::make_pair("setitemcolor", new Command_SetItemColor));
	commands_.insert(std::make_pair("setplayermodel", new Command_SetPlayerModel));
	commands_.insert(std::make_pair("setplayerhead", new Command_SetPlayerHead));
	commands_.insert(std::make_pair("transition", new Command_Transition));
	commands_.insert(std::make_pair("setcvar", new Command_SetCvar));
	commands_.insert(std::make_pair("exec", new Command_Exec));
	commands_.insert(std::make_pair("play", new Command_Play));
	commands_.insert(std::make_pair("playlooped", new Command_PlayLooped));
	commands_.insert(std::make_pair("orbit", new Command_Orbit));

	Vector4Set( colourVector_, 1, 1, 1, 1 );
	colourCharArray_[0] = colourCharArray_[1] = colourCharArray_[2] = colourCharArray_[3] = 255;

	//precomp_ = new UI_PrecompilerTools;
}

UI_Utils::~UI_Utils() 
{
	for( CommandMapIter it = commands_.begin(); it != commands_.end(); ++it )
	{
		delete ((*it).second);
		(*it).second = 0;
	}
	commands_.clear();

	delete hashUtils_;

	//delete precomp_;
}

//void 
//UI_Utils::init_Display( UI_DisplayContext* dc ) 
//{
//	setDisplayContext( dc );
//}

void 
UI_Utils::window_Init( Window *w ) 
{
	memset(w, 0, sizeof(windowDef_t));
	w->borderSize = 1;
	w->foreColor[0] = w->foreColor[1] = w->foreColor[2] = w->foreColor[3] = 1.0;
	w->cinematic = -1;
}


void 
UI_Utils::window_Paint( Window *w, float fadeAmount, float fadeClamp, float fadeCycle ) 
{
	//float bordersize = 0;
	vec4_t color;
	rectDef_t fillRect = w->rect;


	if (debugMode_) 
	{
		color[0] = color[1] = color[2] = color[3] = 1;
		dc_->drawRect(w->rect.x, w->rect.y, w->rect.w, w->rect.h, 1, color);
	}

	if (w == NULL || (w->style == 0 && w->border == 0)) 
		return;

	if (w->border != 0) 
	{
		fillRect.x += w->borderSize;
		fillRect.y += w->borderSize;
		fillRect.w -= w->borderSize + 1;
		fillRect.h -= w->borderSize + 1;
	}

	if (w->style == WINDOW_STYLE_FILLED) 
	{
		// box, but possible a shader that needs filled
		if (w->background) 
		{
			fade(&w->flags, &w->backColor[3], fadeClamp, &w->nextTime, fadeCycle, true, fadeAmount);
			dc_->setColor(w->backColor);
			dc_->drawHandlePic(fillRect.x, fillRect.y, fillRect.w, fillRect.h, w->background);
			dc_->setColor(NULL);
		} 
		else 
		{
			dc_->fillRect(fillRect.x, fillRect.y, fillRect.w, fillRect.h, w->backColor);
		}
	} 
	else if (w->style == WINDOW_STYLE_GRADIENT) 
	{
		gradientBar_Paint(&fillRect, w->backColor);
	} 
    // gradient bar
	else if (w->style == WINDOW_STYLE_SHADER) 
	{
		if (w->flags & WINDOW_FORECOLORSET) 
			dc_->setColor(w->foreColor);
		
		dc_->drawHandlePic(fillRect.x, fillRect.y, fillRect.w, fillRect.h, w->background);
		dc_->setColor(NULL);
	} 
	else if (w->style == WINDOW_STYLE_CINEMATIC) 
	{
		if (w->cinematic == -1) 
		{
			w->cinematic = dc_->playCinematic(w->cinematicName, fillRect.x, fillRect.y, fillRect.w, fillRect.h);
			if (w->cinematic == -1) 
				w->cinematic = -2;
		} 
		if (w->cinematic >= 0) 
		{
			dc_->runCinematicFrame(w->cinematic);
			dc_->drawCinematic(w->cinematic, fillRect.x, fillRect.y, fillRect.w, fillRect.h);
		}
	}

	if (w->border == WINDOW_BORDER_FULL) 
	{
		dc_->drawRect(w->rect.x, w->rect.y, w->rect.w, w->rect.h, w->borderSize, w->borderColor);
	} 
	else if (w->border == WINDOW_BORDER_HORZ) 
	{
		// top/bottom
		dc_->setColor(w->borderColor);
		dc_->drawTopBottom(w->rect.x, w->rect.y, w->rect.w, w->rect.h, w->borderSize);
  		dc_->setColor( NULL );
	} 
	else if (w->border == WINDOW_BORDER_VERT) 
	{
		// left right
		dc_->setColor(w->borderColor);
		dc_->drawSides(w->rect.x, w->rect.y, w->rect.w, w->rect.h, w->borderSize);
  		dc_->setColor( NULL );
	} 
	else if (w->border == WINDOW_BORDER_KCGRADIENT) 
	{
		// this is just two gradient bars along each horz edge
		rectDef_t r = w->rect;
		r.h = w->borderSize;
		gradientBar_Paint(&r, w->borderColor);
		r.y = w->rect.y + w->rect.h - 1;
		gradientBar_Paint(&r, w->borderColor);
	}
}

void
UI_Utils::window_CloseCinematic( windowDef_t *window ) 
{
	if (window->style == WINDOW_STYLE_CINEMATIC && window->cinematic >= 0) 
	{
		dc_->stopCinematic(window->cinematic);
		window->cinematic = -1;
	}
}

void 
UI_Utils::window_CacheContents( windowDef_t *window ) 
{
	if (window) 
	{
		if (window->cinematicName) 
		{
			int cin = dc_->playCinematic(window->cinematicName, 0, 0, 0, 0);
			dc_->stopCinematic(cin);
		}
	}
}

void 
UI_Utils::gradientBar_Paint( rectDef_t *rect, vec4_t color ) 
{
	// gradient bar takes two paints
	dc_->setColor( color );
	dc_->drawHandlePic(rect->x, rect->y, rect->w, rect->h, dc_->assets_.gradientBar);
	dc_->setColor( NULL );
}

void 
UI_Utils::item_TextColor(itemDef_t *item, vec4_t *newColor) 
{
	vec4_t lowLight;
	menuDef_t *parent = (menuDef_t*)item->parent;

	fade( &item->window.flags, &item->window.foreColor[3], parent->fadeClamp, &item->window.nextTime, 
		  parent->fadeCycle, true, parent->fadeAmount );

	if ( item->window.flags & WINDOW_HASFOCUS ) 
	{
		lowLight[0] = 0.8 * parent->focusColor[0]; 
		lowLight[1] = 0.8 * parent->focusColor[1]; 
		lowLight[2] = 0.8 * parent->focusColor[2]; 
		lowLight[3] = 0.8 * parent->focusColor[3]; 
		lerpColor(parent->focusColor,lowLight,*newColor,0.5f+0.5f*sinf(dc_->realTime_ / PULSE_DIVISOR));
	} 
	else if ( item->textStyle == ITEM_TEXTSTYLE_BLINK && !((dc_->realTime_/BLINK_DIVISOR) & 1) ) 
	{
		lowLight[0] = 0.8 * item->window.foreColor[0]; 
		lowLight[1] = 0.8 * item->window.foreColor[1]; 
		lowLight[2] = 0.8 * item->window.foreColor[2]; 
		lowLight[3] = 0.8 * item->window.foreColor[3]; 
		lerpColor(item->window.foreColor,lowLight,*newColor,0.5f+0.5f*sinf(dc_->realTime_ / PULSE_DIVISOR));
	} 
	else 
	{
		memcpy(newColor, &item->window.foreColor, sizeof(vec4_t));
		// items can be enabled and disabled based on cvars
	}

	if ( item->enableCvar && *item->enableCvar && item->cvarTest && *item->cvarTest ) 
	{
		if (item->cvarFlags & (CVAR_ENABLE | CVAR_DISABLE) && !item_EnableShowViaCvar(item, CVAR_ENABLE)) 
			memcpy(newColor, &parent->disableColor, sizeof(vec4_t));
	}
}

bool 
UI_Utils::item_EnableShowViaCvar(itemDef_t *item, int flag) 
{
	char script[1024], *p;
	memset(script, 0, sizeof(script));
	if (item && item->enableCvar && *item->enableCvar && item->cvarTest && *item->cvarTest) 
	{
		char buff[1024];
		dc_->getCVarString(item->cvarTest, buff, sizeof(buff));

		Q_strcat(script, 1024, item->enableCvar);
		p = script;
		while (1) 
		{
			const char *val;
			// expect value then ; or NULL, NULL ends list
			if (!string_Parse(&p, &val)) 
			{
				return (item->cvarFlags & flag) ? false : true;
			}

			if (val[0] == ';' && val[1] == '\0')
				continue;
			
			// enable it if any of the values are true
			if (item->cvarFlags & flag) 
			{
				if (Q_stricmp(buff, val) == 0) 
				{
					return true;
				}
			} 
			else 
			{
				// disable it if any of the values are true
				if (Q_stricmp(buff, val) == 0) 
				{
					return false;
				}
			}
	    }
		return (item->cvarFlags & flag) ? false : true;
	}
	return true;
}

// will optionaly set focus to this item 
bool 
UI_Utils::item_SetFocus(itemDef_t *item, float x, float y) 
{
	int i;
	itemDef_t *oldFocus;
	sfxHandle_t *sfx = &dc_->assets_.itemFocusSound;
	bool playSound = false;
	menuDef_t *parent; // bk001206: = (menuDef_t*)item->parent;

	// sanity check, non-null, not a decoration and does not already have the focus
	if ( item == NULL || (item->window.flags & WINDOW_DECORATION) || (item->window.flags & WINDOW_HASFOCUS) ||
		!(item->window.flags & WINDOW_VISIBLE)) 
	{
		return false;
	}

	// bk001206 - this can be NULL.
	parent = (menuDef_t*)item->parent; 
      
	// items can be enabled and disabled based on cvars
	if (item->cvarFlags & (CVAR_ENABLE | CVAR_DISABLE) && !item_EnableShowViaCvar(item, CVAR_ENABLE))
		return false;

	if (item->cvarFlags & (CVAR_SHOW | CVAR_HIDE) && !item_EnableShowViaCvar(item, CVAR_SHOW)) 
		return false;

	oldFocus = menu_ClearFocus(reinterpret_cast<menuDef_t*>(item->parent));

	if ( item->type == ITEM_TYPE_TEXT ) 
	{
		rectDef_t r;
		r = item->textRect;
		r.y -= r.h;
		if( rect_ContainsPoint(&r, x, y) ) 
		{
			item->window.flags |= WINDOW_HASFOCUS;
			if (item->focusSound) 
				sfx = &item->focusSound;

			playSound = true;
		} 
		else 
		{
			if (oldFocus) 
			{
				oldFocus->window.flags |= WINDOW_HASFOCUS;
				if (oldFocus->onFocus) 
					item_RunScript(oldFocus, oldFocus->onFocus);
			}
		}
	} 
	else 
	{
	    item->window.flags |= WINDOW_HASFOCUS;
		if (item->onFocus) 
			item_RunScript(item, item->onFocus);

		if (item->focusSound) 
			sfx = &item->focusSound;

		playSound = true;
	}

	if (playSound && sfx) 
		dc_->startLocalSound( *sfx, CHAN_LOCAL_SOUND );

	for (i = 0; i < parent->itemCount; i++) 
	{
		if (parent->items[i] == item) 
		{
			parent->cursorItem = i;
			break;
		}
	}

	return true;
}

void 
UI_Utils::item_RunScript(itemDef_t *item, const char *s) 
{
	char script[1024], *p;
	bool bRan;
	memset(script, 0, sizeof(script));
	if (item && s && s[0]) 
	{
		Q_strcat(script, 1024, s);
		p = script;
		while (1) 
		{
			const char *command;
			// expect command then arguments, ; ends command, NULL ends script
			if (!string_Parse(&p, &command)) 
				return;
      
			if (command[0] == ';' && command[1] == '\0')
				continue;

			bRan = false;
//			for (i = 0; i < scriptCommandCount; i++) 
//			{
//				if (Q_stricmp(command, commandList[i].name) == 0) 
//				{
//					(commandList[i].handler(item, &p));
//					bRan = true;
//					break;
//				}
//			}
			CommandMapIter it = commands_.find(command);
			if( it != commands_.end() )
			{
				it->second->handler(item, &p);
				bRan = true;
			}

			// not in our auto list, pass to handler
			if (!bRan) 
			{
				dc_->runScript(&p);
			}
		}
	}
}

void 
UI_Utils::item_UpdatePosition( itemDef_t *item ) 
{
	float x, y;
	menuDef_t *menu;
  
	if (item == NULL || item->parent == NULL) 
	    return;

	menu = reinterpret_cast<menuDef_t*>(item->parent);

	x = menu->window.rect.x;
	y = menu->window.rect.y;
  
	if (menu->window.border != 0) 
	{
		x += menu->window.borderSize;
		y += menu->window.borderSize;
	}

	item_SetScreenCoords(item, x, y);
}


void 
UI_Utils::item_SetScreenCoords( itemDef_t *item, float x, float y )
{
	if (item == NULL) 
		return;

	if (item->window.border != 0) 
	{
		x += item->window.borderSize;
		y += item->window.borderSize;
	}

	item->window.rect.x = x + item->window.rectClient.x;
	item->window.rect.y = y + item->window.rectClient.y;
	item->window.rect.w = item->window.rectClient.w;
	item->window.rect.h = item->window.rectClient.h;

	// force the text rects to recompute
	item->textRect.w = 0;
	item->textRect.h = 0;
}

rectDef_t*
UI_Utils::item_CorrectedTextRect(itemDef_t *item) 
{
	static rectDef_t rect;
	memset(&rect, 0, sizeof(rectDef_t));
	if (item) 
	{
		rect = item->textRect;
		if (rect.w) 
		{
			rect.y -= rect.h;
		}
	}
	return &rect;
}

bool 
UI_Utils::item_Parse( int handle, itemDef_t *item ) 
{
	UI_PrecompilerTools::PC_Token token;
//	keywordHash_t *key;

	if (!precomp_.readTokenHandle(handle, &token))
		return false;

	if (*token.string_ != '{')
		return false;

	while ( 1 ) 
	{
		if (!precomp_.readTokenHandle(handle, &token)) 
		{
			pc_SourceError(handle, "end of file inside menu item\n");
			return false;
		}

		if (*token.string_ == '}')
			return true;

		KeywordHash* key = hashUtils_->findItemKeyword( token.string_ );
		if (!key) 
		{
			pc_SourceError(handle, "unknown menu item keyword %s", token.string_);
			continue;
		}
		if ( !key->func(item, handle) ) 
		{
			pc_SourceError(handle, "couldn't parse menu item keyword %s", token.string_);
			return false;
		}
	}
	return false; 	// bk001205 - LCC missing return value
}

void 
UI_Utils::item_InitControls( itemDef_t *item ) 
{
	if (item == NULL) {
		return;
	}
	if (item->type == ITEM_TYPE_LISTBOX) {
		listBoxDef_t *listPtr = (listBoxDef_t*)item->typeData;
		item->cursorPos = 0;
		if (listPtr) {
			listPtr->cursorPos = 0;
			listPtr->startPos = 0;
			listPtr->endPos = 0;
			listPtr->cursorPos = 0;
		}
	}
}

void UI_Utils::item_ValidateTypeData(itemDef_t *item) 
{
	if (item->typeData)
		return;

	if( item->type == ITEM_TYPE_LISTBOX ) 
	{
		item->typeData = alloc(sizeof(listBoxDef_t));
		memset( item->typeData, 0, sizeof(listBoxDef_t) );
	} 
	else if( item->type == ITEM_TYPE_EDITFIELD || item->type == ITEM_TYPE_NUMERICFIELD || 
			 item->type == ITEM_TYPE_YESNO || item->type == ITEM_TYPE_BIND || 
			 item->type == ITEM_TYPE_SLIDER || item->type == ITEM_TYPE_TEXT ) 
	{
		item->typeData = alloc(sizeof(editFieldDef_t));
		memset(item->typeData, 0, sizeof(editFieldDef_t));
		if( item->type == ITEM_TYPE_EDITFIELD ) 
		{
			if (!((editFieldDef_t *) item->typeData)->maxPaintChars) 
			{
				((editFieldDef_t *) item->typeData)->maxPaintChars = MAX_EDITFIELD;
			}
		}
	} 
	else if (item->type == ITEM_TYPE_MULTI) 
	{
		item->typeData = alloc(sizeof(multiDef_t));
	} 
	else if (item->type == ITEM_TYPE_MODEL) 
	{
		item->typeData = alloc(sizeof(modelDef_t));
	}
}


void 
UI_Utils::item_Paint(itemDef_t *item) 
{
	vec4_t red;
	menuDef_t *parent = (menuDef_t*)item->parent;
	red[0] = red[3] = 1;
	red[1] = red[2] = 0;

	if (item == NULL) 
		return;
 
	if (item->window.flags & WINDOW_ORBITING) 
	{
		if (dc_->realTime_ > item->window.nextTime) 
		{
			float rx, ry, a, c, s, w, h;
      
			item->window.nextTime = dc_->realTime_ + item->window.offsetTime;
			// translate
			w = item->window.rectClient.w / 2;
			h = item->window.rectClient.h / 2;
			rx = item->window.rectClient.x + w - item->window.rectEffects.x;
			ry = item->window.rectClient.y + h - item->window.rectEffects.y;
			a = 3 * M_PI / 180;
  			c = cos(a);
			s = sin(a);
			item->window.rectClient.x = (rx * c - ry * s) + item->window.rectEffects.x - w;
			item->window.rectClient.y = (rx * s + ry * c) + item->window.rectEffects.y - h;
			item_UpdatePosition(item);
		}
	}

	if (item->window.flags & WINDOW_INTRANSITION) 
	{
		if (dc_->realTime_ > item->window.nextTime) 
		{
			int done = 0;
			item->window.nextTime = dc_->realTime_ + item->window.offsetTime;
			// transition the x,y
			if (item->window.rectClient.x == item->window.rectEffects.x) 
			{
				done++;
			} 
			else 
			{
				if (item->window.rectClient.x < item->window.rectEffects.x) 
				{
					item->window.rectClient.x += item->window.rectEffects2.x;
					if (item->window.rectClient.x > item->window.rectEffects.x) 
					{
						item->window.rectClient.x = item->window.rectEffects.x;
						done++;
					}
				} 
				else 
				{
					item->window.rectClient.x -= item->window.rectEffects2.x;
					if (item->window.rectClient.x < item->window.rectEffects.x) 
					{
						item->window.rectClient.x = item->window.rectEffects.x;
						done++;
					}
				}
			}
			if (item->window.rectClient.y == item->window.rectEffects.y) 
			{
				done++;
			}
			else 
			{
				if (item->window.rectClient.y < item->window.rectEffects.y) 
				{
					item->window.rectClient.y += item->window.rectEffects2.y;
					if (item->window.rectClient.y > item->window.rectEffects.y) 
					{
						item->window.rectClient.y = item->window.rectEffects.y;
						done++;
					}
				} 
				else 
				{
					item->window.rectClient.y -= item->window.rectEffects2.y;
					if (item->window.rectClient.y < item->window.rectEffects.y) 
					{
						item->window.rectClient.y = item->window.rectEffects.y;
						done++;
					}
				}
			}
			if (item->window.rectClient.w == item->window.rectEffects.w) {
				done++;
			} 
			else 
			{
				if (item->window.rectClient.w < item->window.rectEffects.w) 
				{
					item->window.rectClient.w += item->window.rectEffects2.w;
					if (item->window.rectClient.w > item->window.rectEffects.w) 
					{
						item->window.rectClient.w = item->window.rectEffects.w;
						done++;
					}
				} 
				else 
				{
					item->window.rectClient.w -= item->window.rectEffects2.w;
					if (item->window.rectClient.w < item->window.rectEffects.w) 
					{
						item->window.rectClient.w = item->window.rectEffects.w;
						done++;
					}
				}
			}
			if (item->window.rectClient.h == item->window.rectEffects.h) 
			{
				done++;
			} 
			else 
			{
				if (item->window.rectClient.h < item->window.rectEffects.h) 
				{
					item->window.rectClient.h += item->window.rectEffects2.h;
					if (item->window.rectClient.h > item->window.rectEffects.h) 
					{
						item->window.rectClient.h = item->window.rectEffects.h;
						done++;
					}
				} 
				else 
				{
					item->window.rectClient.h -= item->window.rectEffects2.h;
					if (item->window.rectClient.h < item->window.rectEffects.h) 
					{
						item->window.rectClient.h = item->window.rectEffects.h;
						done++;
					}
				}
			}

			item_UpdatePosition(item);

			if (done == 4) 
			{
				item->window.flags &= ~WINDOW_INTRANSITION;
			}
		}
	}

	if (item->window.ownerDrawFlags )//&& gDC->ownerDrawVisible) 
	{
		if (!dc_->ownerDrawVisible(item->window.ownerDrawFlags)) 
		{
			item->window.flags &= ~WINDOW_VISIBLE;
		} 
		else 
		{
			item->window.flags |= WINDOW_VISIBLE;
		}
	}

	if (item->cvarFlags & (CVAR_SHOW | CVAR_HIDE)) 
	{
		if (!item_EnableShowViaCvar(item, CVAR_SHOW)) 
		{
			return;
		}
	}

	if (item->window.flags & WINDOW_TIMEDVISIBLE) 
	{
	}

	if (!(item->window.flags & WINDOW_VISIBLE)) 
	{
		return;
	}

	// paint the rect first.. 
	window_Paint(&item->window, parent->fadeAmount , parent->fadeClamp, parent->fadeCycle);

	if (debugMode_) 
	{
		vec4_t color;
		rectDef_t *r = item_CorrectedTextRect(item);
		color[1] = color[3] = 1;
		color[0] = color[2] = 0;
		dc_->drawRect(r->x, r->y, r->w, r->h, 1, color);
	}

	//DC->drawRect(item->window.rect.x, item->window.rect.y, item->window.rect.w, item->window.rect.h, 1, red);

	switch (item->type) 
	{
    case ITEM_TYPE_OWNERDRAW:
		item_OwnerDraw_Paint(item);
		break;
    case ITEM_TYPE_TEXT:
    case ITEM_TYPE_BUTTON:
		item_Text_Paint(item);
		break;
    case ITEM_TYPE_RADIOBUTTON:
		break;
    case ITEM_TYPE_CHECKBOX:
		break;
    case ITEM_TYPE_EDITFIELD:
    case ITEM_TYPE_NUMERICFIELD:
		item_TextField_Paint(item);
		break;
    case ITEM_TYPE_COMBO:
		break;
    case ITEM_TYPE_LISTBOX:
		item_ListBox_Paint(item);
		break;
    //case ITEM_TYPE_IMAGE:
    //  Item_Image_Paint(item);
    //  break;
    case ITEM_TYPE_MODEL:
		item_Model_Paint(item);
		break;
    case ITEM_TYPE_YESNO:
		item_YesNo_Paint(item);
		break;
    case ITEM_TYPE_MULTI:
		item_Multi_Paint(item);
		break;
    case ITEM_TYPE_BIND:
		item_Bind_Paint(item);
		break;
    case ITEM_TYPE_SLIDER:
		item_Slider_Paint(item);
		break;
    default:
		break;
	}
}

void
UI_Utils::item_OwnerDraw_Paint( itemDef_t *item ) 
{
	 menuDef_t *parent;

	if (item == NULL) 
		return;
	
	parent = (menuDef_t*)item->parent;

	//if (gDC->ownerDrawItem) 
	//{
		vec4_t color, lowLight;
		//menuDef_t *parent = (menuDef_t*)item->parent;
		fade(&item->window.flags, &item->window.foreColor[3], parent->fadeClamp, &item->window.nextTime, 
			parent->fadeCycle, true, parent->fadeAmount);
		memcpy(&color, &item->window.foreColor, sizeof(color));
		//if (item->numColors > 0 && gDC->getValue) 
		//{
		//	// if the value is within one of the ranges then set color to that, otherwise leave at default
		//	int i;
		//	float f = gDC->getValue(item->window.ownerDraw);
		//	for (i = 0; i < item->numColors; i++) 
		//	{
		//		if (f >= item->colorRanges[i].low && f <= item->colorRanges[i].high) 
		//		{
		//			memcpy(&color, &item->colorRanges[i].color, sizeof(color));
		//			break;
		//		}
		//	}
		//}

		if (item->window.flags & WINDOW_HASFOCUS) 
		{
			lowLight[0] = 0.8 * parent->focusColor[0]; 
			lowLight[1] = 0.8 * parent->focusColor[1]; 
			lowLight[2] = 0.8 * parent->focusColor[2]; 
			lowLight[3] = 0.8 * parent->focusColor[3]; 
			lerpColor(parent->focusColor,lowLight,color,0.5f+0.5f*sinf(dc_->realTime_ / PULSE_DIVISOR));
		} 
		else if (item->textStyle == ITEM_TEXTSTYLE_BLINK && !((dc_->realTime_/BLINK_DIVISOR) & 1)) 
		{
			lowLight[0] = 0.8 * item->window.foreColor[0]; 
			lowLight[1] = 0.8 * item->window.foreColor[1]; 
			lowLight[2] = 0.8 * item->window.foreColor[2]; 
			lowLight[3] = 0.8 * item->window.foreColor[3]; 
			lerpColor(item->window.foreColor,lowLight,color,0.5f+0.5f*sinf(dc_->realTime_ / PULSE_DIVISOR));
		}

		if (item->cvarFlags & (CVAR_ENABLE | CVAR_DISABLE) && !item_EnableShowViaCvar(item, CVAR_ENABLE)) 
		{
			memcpy(color, parent->disableColor, sizeof(vec4_t)); // bk001207 - FIXME: Com_Memcpy
		}
	
		if (item->text)
		{
			item_Text_Paint(item);
			if( item->text[0] )
			{
				// +8 is an offset kludge to properly align owner draw items that have text combined with them
				dc_->ownerDrawItem(item->textRect.x + item->textRect.w + 8, item->window.rect.y, item->window.rect.w, 
					item->window.rect.h, 0, item->textaligny, item->window.ownerDraw, item->window.ownerDrawFlags, 
					item->alignment, item->special, item->textscale, color, item->window.background, 
					item->textStyle, item );
			}
			else
			{
				dc_->ownerDrawItem(item->textRect.x + item->textRect.w, item->window.rect.y, item->window.rect.w, 
					item->window.rect.h, 0, item->textaligny, item->window.ownerDraw, item->window.ownerDrawFlags, 
					item->alignment, item->special, item->textscale, color, item->window.background, 
					item->textStyle, item );
			}
		}
		else
		{
			dc_->ownerDrawItem(item->window.rect.x, item->window.rect.y, item->window.rect.w, item->window.rect.h, 
				item->textalignx, item->textaligny, item->window.ownerDraw, item->window.ownerDrawFlags, 
				item->alignment, item->special, item->textscale, color, item->window.background, 
				item->textStyle, item );
		}
	//}
}

void 
UI_Utils::item_Text_Paint(itemDef_t *item) 
{
	char text[1024];
	const char *textPtr;
	int height, width;
	vec4_t color;

	if (item->window.flags & WINDOW_WRAPPED) 
	{
		item_Text_Wrapped_Paint(item);
		return;
	}
	if (item->window.flags & WINDOW_AUTOWRAPPED) 
	{
		item_Text_AutoWrapped_Paint(item);
		return;
	}

	if (item->text == NULL) 
	{
		if (item->cvar == NULL)
		{
			return;
		}
		else 
		{
			dc_->getCVarString(item->cvar, text, sizeof(text));
			textPtr = text;
		}
	}
	else 
	{
		textPtr = item->text;
	}

	// this needs to go here as it sets extents for cvar types as well
	item_SetTextExtents(item, &width, &height, textPtr);

	if (*textPtr == '\0') {
		return;
	}


	item_TextColor(item, &color);

	dc_->drawText(item->textRect.x, item->textRect.y, item->textscale, color, textPtr, 0, 0, item->textStyle);
}


void 
UI_Utils::item_Text_AutoWrapped_Paint( itemDef_t *item ) 
{
	char text[1024];
	const char *p, *textPtr, *newLinePtr;
	char buff[1024];
	int width, height, len, textWidth, newLine, newLineWidth;
	float y;
	vec4_t color;

	textWidth = 0;
	newLinePtr = NULL;

	if (item->text == NULL) 
	{
		if (item->cvar == NULL) 
			return;
		else 
		{
			dc_->getCVarString(item->cvar, text, sizeof(text));
			textPtr = text;
		}
	}
	else 
	{
		textPtr = item->text;
	}
	if (*textPtr == '\0') 
		return;

	item_TextColor(item, &color);
	item_SetTextExtents(item, &width, &height, textPtr);

	y = item->textaligny;
	len = 0;
	buff[0] = '\0';
	newLine = 0;
	newLineWidth = 0;
	p = textPtr;
	while (p) 
	{
		if (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\0') 
		{
			newLine = len;
			newLinePtr = p+1;
			newLineWidth = textWidth;
		}
		textWidth = dc_->textWidth(buff, item->textscale, 0);
		if ( (newLine && textWidth > item->window.rect.w) || *p == '\n' || *p == '\0') 
		{
			if (len) 
			{
				if (item->textalignment == ITEM_ALIGN_LEFT) 
					item->textRect.x = item->textalignx;
				else if (item->textalignment == ITEM_ALIGN_RIGHT) 
					item->textRect.x = item->textalignx - newLineWidth;
				else if (item->textalignment == ITEM_ALIGN_CENTER) 
					item->textRect.x = item->textalignx - newLineWidth / 2;

				item->textRect.y = y;
				toWindowCoords(&item->textRect.x, &item->textRect.y, &item->window);
				//
				buff[newLine] = '\0';
				dc_->drawText(item->textRect.x, item->textRect.y, item->textscale, color, buff, 0, 0, item->textStyle);
			}
			if (*p == '\0') 
				break;

			//
			y += height + 5;
			p = newLinePtr;
			len = 0;
			newLine = 0;
			newLineWidth = 0;
			continue;
		}
		buff[len++] = *p++;
		buff[len] = '\0';
	}
}

void
UI_Utils::item_TextField_Paint( itemDef_t *item ) 
{
	char buff[1024];
	vec4_t newColor, lowLight;
	int offset;
	menuDef_t *parent = (menuDef_t*)item->parent;
	editFieldDef_t *editPtr = (editFieldDef_t*)item->typeData;

	item_Text_Paint(item);

	buff[0] = '\0';

	if (item->cvar) 
	{
		dc_->getCVarString(item->cvar, buff, sizeof(buff));
	} 

	parent = (menuDef_t*)item->parent;

	if (item->window.flags & WINDOW_HASFOCUS) 
	{
		lowLight[0] = 0.8 * parent->focusColor[0]; 
		lowLight[1] = 0.8 * parent->focusColor[1]; 
		lowLight[2] = 0.8 * parent->focusColor[2]; 
		lowLight[3] = 0.8 * parent->focusColor[3]; 
		lerpColor(parent->focusColor,lowLight,newColor,0.5f+0.5f*sinf(dc_->realTime_ / PULSE_DIVISOR));
	} 
	else 
	{
		memcpy(&newColor, &item->window.foreColor, sizeof(vec4_t));
	}

	offset = (item->text && *item->text) ? 8 : 0;
	if ((item->window.flags & WINDOW_HASFOCUS) && editingField_) 
	{
		char cursor = dc_->getOverstrikeMode() ? '_' : '|';
		dc_->drawTextWithCursor(item->textRect.x + item->textRect.w + offset, item->textRect.y, item->textscale, 
			newColor, buff + editPtr->paintOffset, item->cursorPos - editPtr->paintOffset , cursor, 
			editPtr->maxPaintChars, item->textStyle);
	} 
	else 
	{
		dc_->drawText(item->textRect.x + item->textRect.w + offset, item->textRect.y, item->textscale, newColor, 
			buff + editPtr->paintOffset, 0, editPtr->maxPaintChars, item->textStyle);
	}

}

void 
UI_Utils::item_ListBox_Paint( itemDef_t *item ) 
{
	float x, y, size, count, i, thumb;
	qhandle_t image;
	qhandle_t optionalImage;
	listBoxDef_t *listPtr = (listBoxDef_t*)item->typeData;

	// the listbox is horizontal or vertical and has a fixed size scroll bar going either direction
	// elements are enumerated from the DC and either text or image handles are acquired from the DC as well
	// textscale is used to size the text, textalignx and textaligny are used to size image elements
	// there is no clipping available so only the last completely visible item is painted
	count = dc_->feederCount(item->special);
	// default is vertical if horizontal flag is not here
	if (item->window.flags & WINDOW_HORIZONTAL) 
	{
		// draw scrollbar in bottom of the window
		// bar
		x = item->window.rect.x + 1;
		y = item->window.rect.y + item->window.rect.h - SCROLLBAR_SIZE - 1;
		dc_->drawHandlePic(x, y, SCROLLBAR_SIZE, SCROLLBAR_SIZE, dc_->assets_.scrollBarArrowLeft);
		x += SCROLLBAR_SIZE - 1;
		size = item->window.rect.w - (SCROLLBAR_SIZE * 2);
		dc_->drawHandlePic(x, y, size+1, SCROLLBAR_SIZE, dc_->assets_.scrollBar);
		x += size - 1;
		dc_->drawHandlePic(x, y, SCROLLBAR_SIZE, SCROLLBAR_SIZE, dc_->assets_.scrollBarArrowRight);
		// thumb
		thumb = item_ListBox_ThumbDrawPosition(item);//Item_ListBox_ThumbPosition(item);
		if (thumb > x - SCROLLBAR_SIZE - 1) 
		{
			thumb = x - SCROLLBAR_SIZE - 1;
		}
		dc_->drawHandlePic(thumb, y, SCROLLBAR_SIZE, SCROLLBAR_SIZE, dc_->assets_.scrollBarThumb);
		//
		listPtr->endPos = listPtr->startPos;
		size = item->window.rect.w - 2;
		// items
		// size contains max available space
		if (listPtr->elementStyle == LISTBOX_IMAGE) 
		{
			// fit = 0;
			x = item->window.rect.x + 1;
			y = item->window.rect.y + 1;
			for (i = listPtr->startPos; i < count; i++) 
			{
				// always draw at least one
				// which may overdraw the box if it is too small for the element
				image = dc_->feederItemImage(item->special, i);
				if (image) 
					dc_->drawHandlePic(x+1, y+1, listPtr->elementWidth - 2, listPtr->elementHeight - 2, image);

				if (i == item->cursorPos) 
					dc_->drawRect(x, y, listPtr->elementWidth-1, listPtr->elementHeight-1, item->window.borderSize, item->window.borderColor);

				size -= listPtr->elementWidth;
				if (size < listPtr->elementWidth) 
				{
					listPtr->drawPadding = size; //listPtr->elementWidth - size;
					break;
				}
				x += listPtr->elementWidth;
				listPtr->endPos++;
				// fit++;
			}
		} 
		//else 
		//{
		//	//
		//}
	} 
	else 
	{
		// draw scrollbar to right side of the window
		x = item->window.rect.x + item->window.rect.w - SCROLLBAR_SIZE - 1;
		y = item->window.rect.y + 1;
		dc_->drawHandlePic(x, y, SCROLLBAR_SIZE, SCROLLBAR_SIZE, dc_->assets_.scrollBarArrowUp);
		y += SCROLLBAR_SIZE - 1;

		listPtr->endPos = listPtr->startPos;
		size = item->window.rect.h - (SCROLLBAR_SIZE * 2);
		dc_->drawHandlePic(x, y, SCROLLBAR_SIZE, size+1, dc_->assets_.scrollBar);
		y += size - 1;
		dc_->drawHandlePic(x, y, SCROLLBAR_SIZE, SCROLLBAR_SIZE, dc_->assets_.scrollBarArrowDown);
		// thumb
		thumb = item_ListBox_ThumbDrawPosition(item);//Item_ListBox_ThumbPosition(item);
		if (thumb > y - SCROLLBAR_SIZE - 1) {
			thumb = y - SCROLLBAR_SIZE - 1;
		}
		dc_->drawHandlePic(x, thumb, SCROLLBAR_SIZE, SCROLLBAR_SIZE, dc_->assets_.scrollBarThumb);

		// adjust size for item painting
		size = item->window.rect.h - 2;
		if (listPtr->elementStyle == LISTBOX_IMAGE) 
		{
			// fit = 0;
			x = item->window.rect.x + 1;
			y = item->window.rect.y + 1;
			for (i = listPtr->startPos; i < count; i++) 
			{
				// always draw at least one
				// which may overdraw the box if it is too small for the element
				image = dc_->feederItemImage(item->special, i);
				if (image) 
				{
					dc_->drawHandlePic(x+1, y+1, listPtr->elementWidth - 2, listPtr->elementHeight - 2, image);
				}

				if (i == item->cursorPos) 
				{
					dc_->drawRect(x, y, listPtr->elementWidth - 1, listPtr->elementHeight - 1, item->window.borderSize, 
								item->window.borderColor);
				}

				listPtr->endPos++;
				size -= listPtr->elementWidth;
				if (size < listPtr->elementHeight) 
				{
					listPtr->drawPadding = listPtr->elementHeight - size;
					break;
				}
				y += listPtr->elementHeight;
				// fit++;
			}
		} 
		else 
		{
			x = item->window.rect.x + 1;
			y = item->window.rect.y + 1;
			for (i = listPtr->startPos; i < count; i++) 
			{
				const char *text;
				// always draw at least one
				// which may overdraw the box if it is too small for the element

				if (listPtr->numColumns > 0) 
				{
					int j;
					for (j = 0; j < listPtr->numColumns; j++) 
					{
						text = dc_->feederItemText(item->special, i, j, &optionalImage);
						if (optionalImage >= 0) 
						{
							dc_->drawHandlePic(x + 4 + listPtr->columnInfo[j].pos, y - 1 + listPtr->elementHeight / 2, 
								listPtr->columnInfo[j].width, listPtr->columnInfo[j].width, optionalImage);
						} 
						else if (text) 
						{
							dc_->drawText(x + 4 + listPtr->columnInfo[j].pos, y + listPtr->elementHeight, item->textscale, 
								item->window.foreColor, text, 0, listPtr->columnInfo[j].maxChars, item->textStyle);
						}
					}
				} 
				else 
				{
					text = dc_->feederItemText(item->special, i, 0, &optionalImage);
					if (optionalImage >= 0) 
					{
						//DC->drawHandlePic(x + 4 + listPtr->elementHeight, y, listPtr->columnInfo[j].width, listPtr->columnInfo[j].width, optionalImage);
					} 
					else if (text) 
					{
						dc_->drawText(x + 4, y + listPtr->elementHeight, item->textscale, item->window.foreColor, text,
									0, 0, item->textStyle);
					}
				}

				if (i == item->cursorPos) 
				{
					dc_->fillRect(x + 2, y + 2, item->window.rect.w - SCROLLBAR_SIZE - 4, listPtr->elementHeight, 
									item->window.outlineColor);
				}

				size -= listPtr->elementHeight;
				if (size < listPtr->elementHeight) 
				{
					listPtr->drawPadding = listPtr->elementHeight - size;
					break;
				}
				listPtr->endPos++;
				y += listPtr->elementHeight;
				// fit++;
			}
		}
	}
}

void 
UI_Utils::item_Model_Paint( itemDef_t *item ) 
{
	float x, y, w, h;
	refdef_t refdef;
	refEntity_t		ent;
	vec3_t			mins, maxs, origin;
	vec3_t			angles;
	modelDef_t *modelPtr = (modelDef_t*)item->typeData;

	if (modelPtr == NULL) 
		return;

	// setup the refdef
	memset( &refdef, 0, sizeof( refdef ) );
	refdef.rdflags = RDF_NOWORLDMODEL;
	AxisClear( refdef.viewaxis );
	x = item->window.rect.x+1;
	y = item->window.rect.y+1;
	w = item->window.rect.w-2;
	h = item->window.rect.h-2;

	adjustFrom640( &x, &y, &w, &h );

	refdef.x = x;
	refdef.y = y;
	refdef.width = w;
	refdef.height = h;

	dc_->modelBounds( item->asset, mins, maxs );

	origin[2] = -0.5 * ( mins[2] + maxs[2] );
	origin[1] = 0.5 * ( mins[1] + maxs[1] );

	// calculate distance so the model nearly fills the box
	//if (true) 
	//{
		float len = 0.5 * ( maxs[2] - mins[2] );		
		origin[0] = len / 0.268;	// len / tan( fov/2 )
		//origin[0] = len / tan(w/2);
	//} else {
	//	origin[0] = item->textscale;
	//}
	refdef.fov_x = (modelPtr->fov_x) ? modelPtr->fov_x : w;
	refdef.fov_y = (modelPtr->fov_y) ? modelPtr->fov_y : h;

	//refdef.fov_x = (int)((float)refdef.width / 640.0f * 90.0f);
	//xx = refdef.width / tan( refdef.fov_x / 360 * M_PI );
	//refdef.fov_y = atan2( refdef.height, xx );
	//refdef.fov_y *= ( 360 / M_PI );

	dc_->clearScene();

	refdef.time = dc_->realTime_;

	// add the model

	memset( &ent, 0, sizeof(ent) );

	//adjust = 5.0 * sin( (float)uis.realtime / 500 );
	//adjust = 360 % (int)((float)uis.realtime / 1000);
	//VectorSet( angles, 0, 0, 1 );

	// use item storage to track
	if (modelPtr->rotationSpeed) 
	{
		if (dc_->realTime_ > item->window.nextTime) 
		{
			item->window.nextTime = dc_->realTime_ + modelPtr->rotationSpeed;
			modelPtr->angle = (int)(modelPtr->angle + 1) % 360;
		}
	}
	VectorSet( angles, 0, modelPtr->angle, 0 );
	AnglesToAxis( angles, ent.axis );

	ent.hModel = item->asset;
	VectorCopy( origin, ent.origin );
	VectorCopy( origin, ent.lightingOrigin );
	ent.renderfx = RF_LIGHTING_ORIGIN | RF_NOSHADOW;
	VectorCopy( ent.origin, ent.oldorigin );

	dc_->addRefEntityToScene( &ent );
	dc_->renderScene( &refdef );

}

void 
UI_Utils::item_YesNo_Paint( itemDef_t *item ) 
{
	vec4_t newColor, lowLight;
	float value;
	menuDef_t *parent = (menuDef_t*)item->parent;

	value = (item->cvar) ? dc_->getCVarValue(item->cvar) : 0;

	if (item->window.flags & WINDOW_HASFOCUS) 
	{
		lowLight[0] = 0.8 * parent->focusColor[0]; 
		lowLight[1] = 0.8 * parent->focusColor[1]; 
		lowLight[2] = 0.8 * parent->focusColor[2]; 
		lowLight[3] = 0.8 * parent->focusColor[3]; 
		lerpColor(parent->focusColor,lowLight,newColor,0.5f+0.5f*sinf(dc_->realTime_ / PULSE_DIVISOR));
	} 
	else 
	{
		memcpy(&newColor, &item->window.foreColor, sizeof(vec4_t));
	}

	if (item->text) 
	{
		item_Text_Paint(item);
		dc_->drawText(item->textRect.x + item->textRect.w + 8, item->textRect.y, item->textscale, newColor, 
				(value != 0) ? "Yes" : "No", 0, 0, item->textStyle);
	} 
	else 
	{
		dc_->drawText(item->textRect.x, item->textRect.y, item->textscale, newColor, (value != 0) ? "Yes" : "No", 0, 0, 
			item->textStyle);
	}
}


void 
UI_Utils::item_Multi_Paint( itemDef_t *item ) 
{
	vec4_t newColor, lowLight;
	const char *text = "";
	menuDef_t *parent = (menuDef_t*)item->parent;

	if (item->window.flags & WINDOW_HASFOCUS) 
	{
		lowLight[0] = 0.8 * parent->focusColor[0]; 
		lowLight[1] = 0.8 * parent->focusColor[1]; 
		lowLight[2] = 0.8 * parent->focusColor[2]; 
		lowLight[3] = 0.8 * parent->focusColor[3]; 
		lerpColor(parent->focusColor,lowLight,newColor,0.5f+0.5f*sinf(dc_->realTime_ / PULSE_DIVISOR));
	} 
	else 
	{
		memcpy(&newColor, &item->window.foreColor, sizeof(vec4_t));
	}

	text = item_Multi_Setting(item);

	if (item->text) 
	{
		item_Text_Paint(item);
		dc_->drawText(item->textRect.x + item->textRect.w + 8, item->textRect.y, item->textscale, newColor, 
			text, 0, 0, item->textStyle);
	} 
	else 
	{
		dc_->drawText(item->textRect.x, item->textRect.y, item->textscale, newColor, text, 0, 0, item->textStyle);
	}
}

void 
UI_Utils::item_Bind_Paint( itemDef_t *item ) 
{
	vec4_t newColor, lowLight;
	float value;
	int maxChars = 0;
	menuDef_t *parent = (menuDef_t*)item->parent;
	editFieldDef_t *editPtr = (editFieldDef_t*)item->typeData;
	if (editPtr) 
		maxChars = editPtr->maxPaintChars;

	value = (item->cvar) ? dc_->getCVarValue(item->cvar) : 0;

	if (item->window.flags & WINDOW_HASFOCUS) 
	{
		if (bindItem_ == item) 
		{
			lowLight[0] = 0.8f * 1.0f;
			lowLight[1] = 0.8f * 0.0f;
			lowLight[2] = 0.8f * 0.0f;
			lowLight[3] = 0.8f * 1.0f;
		} 
		else 
		{
			lowLight[0] = 0.8f * parent->focusColor[0]; 
			lowLight[1] = 0.8f * parent->focusColor[1]; 
			lowLight[2] = 0.8f * parent->focusColor[2]; 
			lowLight[3] = 0.8f * parent->focusColor[3]; 
		}
		lerpColor(parent->focusColor,lowLight,newColor,0.5f+0.5f*sinf(dc_->realTime_ / PULSE_DIVISOR));
	} 
	else 
	{
		memcpy(&newColor, &item->window.foreColor, sizeof(vec4_t));
	}

	if (item->text) 
	{
		item_Text_Paint(item);
		controlUtils_->bindingFromName(item->cvar);
		dc_->drawText(item->textRect.x + item->textRect.w + 8, item->textRect.y, item->textscale, newColor,
					controlUtils_->nameBind1_, 0, maxChars, item->textStyle);
	} 
	else 
	{
		dc_->drawText(item->textRect.x, item->textRect.y, item->textscale, newColor, (value != 0) ? "FIXME" : "FIXME", 
			0, maxChars, item->textStyle);
	}
}

void 
UI_Utils::item_Slider_Paint(itemDef_t *item) 
{
	vec4_t newColor, lowLight;
	float x, y, value;
	menuDef_t *parent = (menuDef_t*)item->parent;

	value = (item->cvar) ? dc_->getCVarValue(item->cvar) : 0;

	if (item->window.flags & WINDOW_HASFOCUS) 
	{
		lowLight[0] = 0.8 * parent->focusColor[0]; 
		lowLight[1] = 0.8 * parent->focusColor[1]; 
		lowLight[2] = 0.8 * parent->focusColor[2]; 
		lowLight[3] = 0.8 * parent->focusColor[3]; 
		lerpColor(parent->focusColor,lowLight,newColor,0.5f+0.5f*sinf(dc_->realTime_ / PULSE_DIVISOR));
	} 
	else 
	{
		memcpy(&newColor, &item->window.foreColor, sizeof(vec4_t));
	}

	y = item->window.rect.y;
	if (item->text)
	{
		item_Text_Paint(item);
		x = item->textRect.x + item->textRect.w + 8;
	} 
	else 
	{
		x = item->window.rect.x;
	}
	dc_->setColor(newColor);
	dc_->drawHandlePic( x, y, SLIDER_WIDTH, SLIDER_HEIGHT, dc_->assets_.sliderBar );

	x = item_Slider_ThumbPosition(item);
	dc_->drawHandlePic( x - (SLIDER_THUMB_WIDTH / 2), y - 2, SLIDER_THUMB_WIDTH, SLIDER_THUMB_HEIGHT, 
		dc_->assets_.sliderThumb );

}

void 
UI_Utils::item_Text_Wrapped_Paint( itemDef_t *item ) 
{
	char text[1024];
	const char *p, *start, *textPtr;
	char buff[1024];
	int width, height;
	float x, y;
	vec4_t color;

	// now paint the text and/or any optional images
	// default to left

	if (item->text == NULL) 
	{
		if (item->cvar == NULL) 
			return;
		else 
		{
			dc_->getCVarString(item->cvar, text, sizeof(text));
			textPtr = text;
		}
	}
	else 
	{
		textPtr = item->text;
	}
	if (*textPtr == '\0') {
		return;
	}

	item_TextColor(item, &color);
	item_SetTextExtents(item, &width, &height, textPtr);

	x = item->textRect.x;
	y = item->textRect.y;
	start = textPtr;
	p = strchr(textPtr, '\r');
	while (p && *p) 
	{
		strncpy(buff, start, p-start+1);
		buff[p-start] = '\0';
		dc_->drawText(x, y, item->textscale, color, buff, 0, 0, item->textStyle);
		y += height + 5;
		start += p - start + 1;
		p = strchr(p+1, '\r');
	}
	dc_->drawText(x, y, item->textscale, color, start, 0, 0, item->textStyle);
}

void 
UI_Utils::item_SetTextExtents( itemDef_t *item, int *width, int *height, const char *text ) 
{
	const char *textPtr = (text) ? text : item->text;

	if (textPtr == NULL ) 
		return;

	*width = item->textRect.w;
	*height = item->textRect.h;

	// keeps us from computing the widths and heights more than once
	if (*width == 0 || (item->type == ITEM_TYPE_OWNERDRAW && item->textalignment == ITEM_ALIGN_CENTER)) 
	{
		int originalWidth = dc_->textWidth(item->text, item->textscale, 0);

		if (item->type == ITEM_TYPE_OWNERDRAW && (item->textalignment == ITEM_ALIGN_CENTER || 
			item->textalignment == ITEM_ALIGN_RIGHT)) 
		{
			originalWidth += dc_->ownerDrawWidth(item->window.ownerDraw, item->textscale);
		}
		else if (item->type == ITEM_TYPE_EDITFIELD && item->textalignment == ITEM_ALIGN_CENTER && item->cvar) 
		{
			char buff[256];
			dc_->getCVarString(item->cvar, buff, 256);
			originalWidth += dc_->textWidth(buff, item->textscale, 0);
		}

		*width = dc_->textWidth(textPtr, item->textscale, 0);
		*height = dc_->textHeight(textPtr, item->textscale, 0);
		item->textRect.w = *width;
		item->textRect.h = *height;
		item->textRect.x = item->textalignx;
		item->textRect.y = item->textaligny;
		if (item->textalignment == ITEM_ALIGN_RIGHT) 
		{
			item->textRect.x = item->textalignx - originalWidth;
		} 
		else if (item->textalignment == ITEM_ALIGN_CENTER) 
		{
			item->textRect.x = item->textalignx - originalWidth / 2;
		}

		toWindowCoords(&item->textRect.x, &item->textRect.y, &item->window);
	}
}


int
UI_Utils::item_ListBox_ThumbDrawPosition( itemDef_t *item ) 
{
	int min, max;

	if (itemCapture_ == item) 
	{
		if (item->window.flags & WINDOW_HORIZONTAL) 
		{
			min = item->window.rect.x + SCROLLBAR_SIZE + 1;
			max = item->window.rect.x + item->window.rect.w - 2*SCROLLBAR_SIZE - 1;
			if (dc_->cursorX_ >= min + SCROLLBAR_SIZE/2 && dc_->cursorX_ <= max + SCROLLBAR_SIZE/2) 
			{
				return dc_->cursorX_ - SCROLLBAR_SIZE/2;
			}
			else 
			{
				return item_ListBox_ThumbPosition(item);
			}
		}
		else {
			min = item->window.rect.y + SCROLLBAR_SIZE + 1;
			max = item->window.rect.y + item->window.rect.h - 2*SCROLLBAR_SIZE - 1;
			if (dc_->cursorY_ >= min + SCROLLBAR_SIZE/2 && dc_->cursorY_ <= max + SCROLLBAR_SIZE/2) 
			{
				return dc_->cursorY_ - SCROLLBAR_SIZE/2;
			}
			else 
			{
				return item_ListBox_ThumbPosition(item);
			}
		}
	}
	else 
	{
		return item_ListBox_ThumbPosition(item);
	}
}


int 
UI_Utils::item_ListBox_ThumbPosition( itemDef_t *item ) 
{
	float max, pos, size;
	listBoxDef_t *listPtr = (listBoxDef_t*)item->typeData;

	max = item_ListBox_MaxScroll(item);
	if (item->window.flags & WINDOW_HORIZONTAL) {
		size = item->window.rect.w - (SCROLLBAR_SIZE * 2) - 2;
		if (max > 0) {
			pos = (size-SCROLLBAR_SIZE) / (float) max;
		} else {
			pos = 0;
		}
		pos *= listPtr->startPos;
		return item->window.rect.x + 1 + SCROLLBAR_SIZE + pos;
	}
	else {
		size = item->window.rect.h - (SCROLLBAR_SIZE * 2) - 2;
		if (max > 0) {
			pos = (size-SCROLLBAR_SIZE) / (float) max;
		} else {
			pos = 0;
		}
		pos *= listPtr->startPos;
		return item->window.rect.y + 1 + SCROLLBAR_SIZE + pos;
	}
}

const char* 
UI_Utils::item_Multi_Setting( itemDef_t *item ) 
{
	char buff[1024];
	float value = 0;
	int i;
	multiDef_t *multiPtr = (multiDef_t*)item->typeData;
	if (multiPtr) 
	{
		if (multiPtr->strDef) 
		{
			dc_->getCVarString(item->cvar, buff, sizeof(buff));
		} 
		else 
		{
			value = dc_->getCVarValue(item->cvar);
		}
		for (i = 0; i < multiPtr->count; i++) 
		{
			if (multiPtr->strDef) 
			{
				if (Q_stricmp(buff, multiPtr->cvarStr[i]) == 0) 
				{
					return multiPtr->cvarList[i];
				}
			} 
			else 
			{
 				if (multiPtr->cvarValue[i] == value) 
				{
					return multiPtr->cvarList[i];
 				}
 			}
 		}
	}
	return "";
}


float 
UI_Utils::item_Slider_ThumbPosition( itemDef_t *item ) 
{
	float value, range, x;
	editFieldDef_t *editDef = reinterpret_cast<editFieldDef_t*>(item->typeData);

	if (item->text) 
		x = item->textRect.x + item->textRect.w + 8;
	else 
		x = item->window.rect.x;

	if (editDef == NULL && item->cvar) 
		return x;

	value = dc_->getCVarValue(item->cvar);

	if (value < editDef->minVal) 
		value = editDef->minVal;
	else if (value > editDef->maxVal) 
		value = editDef->maxVal;

	range = editDef->maxVal - editDef->minVal;
	value -= editDef->minVal;
	value /= range;
	//value /= (editDef->maxVal - editDef->minVal);
	value *= SLIDER_WIDTH;
	x += value;
	// vm fuckage
	//x = x + (((float)value / editDef->maxVal) * SLIDER_WIDTH);
	return x;
}


int 
UI_Utils::item_ListBox_MaxScroll( itemDef_t *item ) 
{
	listBoxDef_t *listPtr = (listBoxDef_t*)item->typeData;
	int count = dc_->feederCount(item->special);
	int max;

	if (item->window.flags & WINDOW_HORIZONTAL) 
	{
		max = count - (item->window.rect.w / listPtr->elementWidth) + 1;
	}
	else 
	{
		max = count - (item->window.rect.h / listPtr->elementHeight) + 1;
	}
	if (max < 0) 
	{
		return 0;
	}
	return max;
}

void 
UI_Utils::item_CacheContents( itemDef_t *item ) 
{
	if (item) 
	{
		window_CacheContents(&item->window);
	}

}


void 
UI_Utils::item_MouseEnter( itemDef_t *item, float x, float y ) 
{
	rectDef_t r;
	if (item) 
	{
		r = item->textRect;
		r.y -= r.h;
		// in the text rect?

		// items can be enabled and disabled based on cvars
		if (item->cvarFlags & (CVAR_ENABLE | CVAR_DISABLE) && !item_EnableShowViaCvar(item, CVAR_ENABLE))
			return;

		if (item->cvarFlags & (CVAR_SHOW | CVAR_HIDE) && !item_EnableShowViaCvar(item, CVAR_SHOW))
			return;

		if (rect_ContainsPoint(&r, x, y)) 
		{
			if (!(item->window.flags & WINDOW_MOUSEOVERTEXT)) 
			{
				item_RunScript(item, item->mouseEnterText);
				item->window.flags |= WINDOW_MOUSEOVERTEXT;
			}
			if (!(item->window.flags & WINDOW_MOUSEOVER)) 
			{
				item_RunScript(item, item->mouseEnter);
				item->window.flags |= WINDOW_MOUSEOVER;
			}

		} 
		else 
		{
			// not in the text rect
			if (item->window.flags & WINDOW_MOUSEOVERTEXT) 
			{
				// if we were
				item_RunScript(item, item->mouseExitText);
				item->window.flags &= ~WINDOW_MOUSEOVERTEXT;
			}
			if (!(item->window.flags & WINDOW_MOUSEOVER)) 
			{
				item_RunScript(item, item->mouseEnter);
				item->window.flags |= WINDOW_MOUSEOVER;
			}

			if (item->type == ITEM_TYPE_LISTBOX) 
			{
				item_ListBox_MouseEnter(item, x, y);
			}
		}
	}
}

void 
UI_Utils::item_MouseLeave( itemDef_t *item ) 
{
	if (item) 
	{
		if (item->window.flags & WINDOW_MOUSEOVERTEXT) 
		{
			item_RunScript(item, item->mouseExitText);
			item->window.flags &= ~WINDOW_MOUSEOVERTEXT;
		}
		item_RunScript(item, item->mouseExit);
		item->window.flags &= ~(WINDOW_LB_RIGHTARROW | WINDOW_LB_LEFTARROW);
	}
}


void 
UI_Utils::item_ListBox_MouseEnter(itemDef_t *item, float x, float y) 
{
	rectDef_t r;
	listBoxDef_t *listPtr = (listBoxDef_t*)item->typeData;
        
	item->window.flags &= ~(WINDOW_LB_LEFTARROW | WINDOW_LB_RIGHTARROW | WINDOW_LB_THUMB | WINDOW_LB_PGUP | WINDOW_LB_PGDN);
	item->window.flags |= item_ListBox_OverLB(item, x, y);

	if (item->window.flags & WINDOW_HORIZONTAL) 
	{
		if (!(item->window.flags & (WINDOW_LB_LEFTARROW | WINDOW_LB_RIGHTARROW | WINDOW_LB_THUMB | WINDOW_LB_PGUP | WINDOW_LB_PGDN))) 
		{
			// check for selection hit as we have exausted buttons and thumb
			if (listPtr->elementStyle == LISTBOX_IMAGE) 
			{
				r.x = item->window.rect.x;
				r.y = item->window.rect.y;
				r.h = item->window.rect.h - SCROLLBAR_SIZE;
				r.w = item->window.rect.w - listPtr->drawPadding;
				if (rect_ContainsPoint(&r, x, y)) 
				{
					listPtr->cursorPos =  (int)((x - r.x) / listPtr->elementWidth)  + listPtr->startPos;
					if (listPtr->cursorPos >= listPtr->endPos) 
						listPtr->cursorPos = listPtr->endPos;
				}
			} 
			else 
			{
				// text hit.. 
			}
		}
	} 
	else if (!(item->window.flags & (WINDOW_LB_LEFTARROW | WINDOW_LB_RIGHTARROW | WINDOW_LB_THUMB | WINDOW_LB_PGUP | WINDOW_LB_PGDN))) 
	{
		r.x = item->window.rect.x;
		r.y = item->window.rect.y;
		r.w = item->window.rect.w - SCROLLBAR_SIZE;
		r.h = item->window.rect.h - listPtr->drawPadding;
		if (rect_ContainsPoint(&r, x, y)) 
		{
			listPtr->cursorPos =  (int)((y - 2 - r.y) / listPtr->elementHeight)  + listPtr->startPos;
			if (listPtr->cursorPos > listPtr->endPos) 
				listPtr->cursorPos = listPtr->endPos;
		}
	}
}

int 
UI_Utils::item_ListBox_OverLB( itemDef_t *item, float x, float y ) 
{
	rectDef_t r;
	listBoxDef_t *listPtr;
	int thumbstart;
	int count;

	count = dc_->feederCount(item->special);
	listPtr = (listBoxDef_t*)item->typeData;
	if (item->window.flags & WINDOW_HORIZONTAL) 
	{
		// check if on left arrow
		r.x = item->window.rect.x;
		r.y = item->window.rect.y + item->window.rect.h - SCROLLBAR_SIZE;
		r.h = r.w = SCROLLBAR_SIZE;
		if (rect_ContainsPoint(&r, x, y)) 
			return WINDOW_LB_LEFTARROW;

		// check if on right arrow
		r.x = item->window.rect.x + item->window.rect.w - SCROLLBAR_SIZE;
		if (rect_ContainsPoint(&r, x, y)) 
			return WINDOW_LB_RIGHTARROW;

		// check if on thumb
		thumbstart = item_ListBox_ThumbPosition(item);
		r.x = thumbstart;
		if (rect_ContainsPoint(&r, x, y))
			return WINDOW_LB_THUMB;

		r.x = item->window.rect.x + SCROLLBAR_SIZE;
		r.w = thumbstart - r.x;
		if (rect_ContainsPoint(&r, x, y))
			return WINDOW_LB_PGUP;

		r.x = thumbstart + SCROLLBAR_SIZE;
		r.w = item->window.rect.x + item->window.rect.w - SCROLLBAR_SIZE;
		if (rect_ContainsPoint(&r, x, y)) 
			return WINDOW_LB_PGDN;

	} 
	else 
	{
		r.x = item->window.rect.x + item->window.rect.w - SCROLLBAR_SIZE;
		r.y = item->window.rect.y;
		r.h = r.w = SCROLLBAR_SIZE;
		if (rect_ContainsPoint(&r, x, y)) 
			return WINDOW_LB_LEFTARROW;

		r.y = item->window.rect.y + item->window.rect.h - SCROLLBAR_SIZE;
		if (rect_ContainsPoint(&r, x, y)) 
			return WINDOW_LB_RIGHTARROW;

		thumbstart = item_ListBox_ThumbPosition(item);
		r.y = thumbstart;
		if (rect_ContainsPoint(&r, x, y)) 
			return WINDOW_LB_THUMB;

		r.y = item->window.rect.y + SCROLLBAR_SIZE;
		r.h = thumbstart - r.y;
		if (rect_ContainsPoint(&r, x, y)) 
			return WINDOW_LB_PGUP;

		r.y = thumbstart + SCROLLBAR_SIZE;
		r.h = item->window.rect.y + item->window.rect.h - SCROLLBAR_SIZE;
		if (rect_ContainsPoint(&r, x, y))
			return WINDOW_LB_PGDN;
	}
	return 0;
}

void 
UI_Utils::item_SetMouseOver( itemDef_t *item, bool focus ) 
{
	if (item) 
	{
		if (focus) 
			item->window.flags |= WINDOW_MOUSEOVER;
		else	
			item->window.flags &= ~WINDOW_MOUSEOVER;
	}
}

bool
UI_Utils::item_Bind_HandleKey( itemDef_t *item, int key, int down ) 
{
	int			id;
	int			i;

	if (rect_ContainsPoint(&item->window.rect, dc_->cursorX_, dc_->cursorY_) && 
		!waitingForKey_)
	{
		if (down && (key == K_MOUSE1 || key == K_ENTER)) 
		{
			waitingForKey_ = true;
			bindItem_ = item;
		}
		return true;
	}
	else
	{
		if (!waitingForKey_ || bindItem_ == NULL) 
			return true;

		if (key & K_CHAR_FLAG) 
			return true;

		switch (key)
		{
			case K_ESCAPE:
				waitingForKey_ = false;
				return true;
	
			case K_BACKSPACE:
				id = controlUtils_->bindingIDFromName(item->cvar);
				if (id != -1) 
				{
					controlUtils_->bindings_[id]->bind1_ = -1;
					controlUtils_->bindings_[id]->bind2_ = -1;
				}
				controlUtils_->setConfig(true);
				waitingForKey_ = false;
				bindItem_ = NULL;
				return true;

			case '`':
				return true;
		}
	}

	if (key != -1)
	{
		size_t bindCount = controlUtils_->bindings_.size();
		for (i=0; i < bindCount; i++)
		{

			if (controlUtils_->bindings_[i]->bind2_ == key) 
				controlUtils_->bindings_[i]->bind2_ = -1;

			if (controlUtils_->bindings_[i]->bind1_ == key)
			{
				controlUtils_->bindings_[i]->bind1_ = controlUtils_->bindings_[i]->bind2_;
				controlUtils_->bindings_[i]->bind2_ = -1;
			}
		}
	}


	id = controlUtils_->bindingIDFromName(item->cvar);

	if (id != -1) 
	{
		if (key == -1) 
		{
			if( controlUtils_->bindings_[id]->bind1_ != -1 ) 
			{
				dc_->setBinding( controlUtils_->bindings_[id]->bind1_, "" );
				controlUtils_->bindings_[id]->bind1_ = -1;
			}
			if( controlUtils_->bindings_[id]->bind2_ != -1 ) 
			{
				dc_->setBinding( controlUtils_->bindings_[id]->bind2_, "" );
				controlUtils_->bindings_[id]->bind2_ = -1;
			}
		}
		else if (controlUtils_->bindings_[id]->bind1_ == -1) 
		{
			controlUtils_->bindings_[id]->bind1_ = key;
		}
		else if (controlUtils_->bindings_[id]->bind1_ != key && controlUtils_->bindings_[id]->bind2_ == -1) 
		{
			controlUtils_->bindings_[id]->bind2_ = key;
		}
		else 
		{
			dc_->setBinding( controlUtils_->bindings_[id]->bind1_, "" );
			dc_->setBinding( controlUtils_->bindings_[id]->bind2_, "" );
			controlUtils_->bindings_[id]->bind1_ = key;
			controlUtils_->bindings_[id]->bind2_ = -1;
		}						
	}

	controlUtils_->setConfig(true);	
	waitingForKey_ = false;

	return true;
}

bool 
UI_Utils::item_TextField_HandleKey( itemDef_t *item, int key ) 
{
	char buff[1024];
	int len;
	itemDef_t *newItem = NULL;
	editFieldDef_t *editPtr = static_cast<editFieldDef_t*>(item->typeData);

	if (item->cvar) 
	{
		memset(buff, 0, sizeof(buff));
		dc_->getCVarString(item->cvar, buff, sizeof(buff));
		len = strlen(buff);
		if (editPtr->maxChars && len > editPtr->maxChars) 
			len = editPtr->maxChars;
		if ( key & K_CHAR_FLAG ) 
		{
			key &= ~K_CHAR_FLAG;


			if (key == 'h' - 'a' + 1 )	
			{	// ctrl-h is backspace
				if ( item->cursorPos > 0 ) 
				{
					memmove( &buff[item->cursorPos - 1], &buff[item->cursorPos], len + 1 - item->cursorPos);
					item->cursorPos--;
					if (item->cursorPos < editPtr->paintOffset)
						editPtr->paintOffset--;
				}
				dc_->setCVar(item->cvar, buff);
	    		return true;
			}


			//
			// ignore any non printable chars
			//
			if ( key < 32 || !item->cvar) 
			    return true;

			if (item->type == ITEM_TYPE_NUMERICFIELD) 
			{
				if (key < '0' || key > '9') 
					return false;
			}

			if (!dc_->getOverstrikeMode()) 
			{
				if (( len == MAX_EDITFIELD - 1 ) || (editPtr->maxChars && len >= editPtr->maxChars)) 
					return true;
				memmove( &buff[item->cursorPos + 1], &buff[item->cursorPos], len + 1 - item->cursorPos );
			} 
			else 
			{
				if (editPtr->maxChars && item->cursorPos >= editPtr->maxChars) 
					return true;
			}

			buff[item->cursorPos] = key;

			dc_->setCVar(item->cvar, buff);

			if (item->cursorPos < len + 1) 
			{
				item->cursorPos++;
				if (editPtr->maxPaintChars && item->cursorPos > editPtr->maxPaintChars) 
					editPtr->paintOffset++;
			}

		} 
		else 
		{

			if ( key == K_DEL || key == K_KP_DEL ) 
			{
				if ( item->cursorPos < len ) 
				{
					memmove( buff + item->cursorPos, buff + item->cursorPos + 1, len - item->cursorPos);
					dc_->setCVar(item->cvar, buff);
				}
				return true;
			}

			if ( key == K_RIGHTARROW || key == K_KP_RIGHTARROW ) 
			{
				if (editPtr->maxPaintChars && item->cursorPos >= editPtr->maxPaintChars && item->cursorPos < len) {
					item->cursorPos++;
					editPtr->paintOffset++;
					return true;
				}
				if (item->cursorPos < len) 
					item->cursorPos++;
				return true;
			}

			if ( key == K_LEFTARROW || key == K_KP_LEFTARROW ) 
			{
				if ( item->cursorPos > 0 ) 
					item->cursorPos--;
				if (item->cursorPos < editPtr->paintOffset) 
					editPtr->paintOffset--;
				return true;
			}

			if ( key == K_HOME || key == K_KP_HOME) 
			{	
				// || ( tolower(key) == 'a' && trap_Key_IsDown( K_CTRL ) ) ) {
				item->cursorPos = 0;
				editPtr->paintOffset = 0;
				return true;
			}

			if ( key == K_END || key == K_KP_END)  
			{
				// ( tolower(key) == 'e' && trap_Key_IsDown( K_CTRL ) ) ) {
				item->cursorPos = len;
				if(item->cursorPos > editPtr->maxPaintChars) 
					editPtr->paintOffset = len - editPtr->maxPaintChars;
				return true;
			}

			if ( key == K_INS || key == K_KP_INS ) 
			{
				dc_->setOverstrikeMode(!dc_->getOverstrikeMode());
				return true;
			}
		}

		if (key == K_TAB || key == K_DOWNARROW || key == K_KP_DOWNARROW) 
		{
			newItem = menu_SetNextCursorItem(reinterpret_cast<menuDef_t*>(item->parent));
			if (newItem && (newItem->type == ITEM_TYPE_EDITFIELD || newItem->type == ITEM_TYPE_NUMERICFIELD)) 
				editItem_ = newItem;
		}

		if (key == K_UPARROW || key == K_KP_UPARROW) 
		{
			newItem = menu_SetPrevCursorItem(reinterpret_cast<menuDef_t*>(item->parent));
			if (newItem && (newItem->type == ITEM_TYPE_EDITFIELD || newItem->type == ITEM_TYPE_NUMERICFIELD)) 
				editItem_ = newItem;
		}

		if ( key == K_ENTER || key == K_KP_ENTER || key == K_ESCAPE)  
			return false;

		return true;
	}
	return false;

}

void 
UI_Utils::item_Action( itemDef_t *item ) 
{
	if (item) 
	{
		item_RunScript(item, item->action);
	}
}


bool 
UI_Utils::item_HandleKey( itemDef_t *item, int key, int down ) 
{
	if (itemCapture_) 
	{
		item_StopCapture(itemCapture_);
		itemCapture_ = 0;
		delete captureTool_;
		captureTool_ = 0;
	} 
	else 
	{
	  // bk001206 - parentheses
		if ( down && ( key == K_MOUSE1 || key == K_MOUSE2 || key == K_MOUSE3 ) ) 
			item_StartCapture(item, key);
	}

	if (!down)
		return false;

  switch (item->type)
  {
  case ITEM_TYPE_BUTTON:
      return false;
      break;
  case ITEM_TYPE_RADIOBUTTON:
      return false;
      break;
  case ITEM_TYPE_CHECKBOX:
      return false;
      break;
  case ITEM_TYPE_EDITFIELD:
  case ITEM_TYPE_NUMERICFIELD:
      //return Item_TextField_HandleKey(item, key);
      return false;
      break;
  case ITEM_TYPE_COMBO:
      return false;
      break;
  case ITEM_TYPE_LISTBOX:
      return item_ListBox_HandleKey(item, key, down, false);
      break;
  case ITEM_TYPE_YESNO:
      return item_YesNo_HandleKey(item, key);
      break;
  case ITEM_TYPE_MULTI:
      return item_Multi_HandleKey(item, key);
      break;
  case ITEM_TYPE_OWNERDRAW:
      return item_OwnerDraw_HandleKey(item, key);
      break;
  case ITEM_TYPE_BIND:
	return item_Bind_HandleKey(item, key, down);
      break;
  case ITEM_TYPE_SLIDER:
      return item_Slider_HandleKey(item, key, down);
      break;
  //case ITEM_TYPE_IMAGE:
    //  Item_Image_Paint(item);
    //  break;
  default:
      return false;
      break;
	}
	//return false;
}

void 
UI_Utils::item_StartCapture( itemDef_t *item, int key ) 
{
	int flags;
	switch (item->type) 
	{
    case ITEM_TYPE_EDITFIELD:
    case ITEM_TYPE_NUMERICFIELD:

		case ITEM_TYPE_LISTBOX:
		{
			flags = item_ListBox_OverLB(item, dc_->cursorX_, dc_->cursorY_);
			if (flags & (WINDOW_LB_LEFTARROW | WINDOW_LB_RIGHTARROW)) 
			{
				scrollInfo_.nextScrollTime_ = dc_->realTime_ + SCROLL_TIME_START;
				scrollInfo_.nextAdjustTime_ = dc_->realTime_ + SCROLL_TIME_ADJUST;
				scrollInfo_.adjustValue_ = SCROLL_TIME_START;
				scrollInfo_.scrollKey_ = key;
				scrollInfo_.scrollDir_ = (flags & WINDOW_LB_LEFTARROW) ? true : false;
				scrollInfo_.item_ = item;
				captureTool_ = new UI_Capture_ListBoxAuto( &scrollInfo_ );
				itemCapture_ = item;
			} 
			else if (flags & WINDOW_LB_THUMB) 
			{
				scrollInfo_.scrollKey_ = key;
				scrollInfo_.item_ = item;
				scrollInfo_.xStart_ = dc_->cursorX_;
				scrollInfo_.yStart_ = dc_->cursorY_;
				captureTool_ = new UI_Capture_ListBoxThumb( &scrollInfo_ );
				itemCapture_ = item;
			}
			break;
		}
		case ITEM_TYPE_SLIDER:
		{
			flags = item_Slider_OverSlider(item, dc_->cursorX_, dc_->cursorY_);
			if (flags & WINDOW_LB_THUMB) 
			{
				scrollInfo_.scrollKey_ = key;
				scrollInfo_.item_ = item;
				scrollInfo_.xStart_ = dc_->cursorX_;
				scrollInfo_.yStart_ = dc_->cursorY_;
				captureTool_ = new UI_Capture_SliderThumb( &scrollInfo_ );
				itemCapture_ = item;
			}
			break;
		}
	}
}

void 
UI_Utils::item_StopCapture( itemDef_t *item ) 
{
}


bool 
UI_Utils::item_OwnerDraw_HandleKey( itemDef_t *item, int key ) 
{
	if (item )//&& gDC->ownerDrawHandleKey) 
		return dc_->ownerDrawHandleKey(item->window.ownerDraw, item->window.ownerDrawFlags, &item->special, key);
  
	return false;
}

bool 
UI_Utils::item_ListBox_HandleKey( itemDef_t *item, int key, int down, bool force ) 
{
	listBoxDef_t *listPtr = (listBoxDef_t*)item->typeData;
	int count = dc_->feederCount(item->special);
	int max, viewmax;

	if (force || (rect_ContainsPoint(&item->window.rect, dc_->cursorX_, dc_->cursorY_) && 
		(item->window.flags & WINDOW_HASFOCUS))) 
	{
		max = item_ListBox_MaxScroll(item);
		if (item->window.flags & WINDOW_HORIZONTAL) 
		{
			viewmax = (item->window.rect.w / listPtr->elementWidth);
			if ( key == K_LEFTARROW || key == K_KP_LEFTARROW ) 
			{
				if (!listPtr->notselectable) 
				{
					listPtr->cursorPos--;
					if (listPtr->cursorPos < 0) 
						listPtr->cursorPos = 0;
					
					if (listPtr->cursorPos < listPtr->startPos) 
						listPtr->startPos = listPtr->cursorPos;
					
					if (listPtr->cursorPos >= listPtr->startPos + viewmax) 
						listPtr->startPos = listPtr->cursorPos - viewmax + 1;
					
					item->cursorPos = listPtr->cursorPos;
					dc_->feederSelection(item->special, item->cursorPos);
				}
				else 
				{
					listPtr->startPos--;
					if (listPtr->startPos < 0)
						listPtr->startPos = 0;
				}
				return true;
			}
			if ( key == K_RIGHTARROW || key == K_KP_RIGHTARROW ) 
			{
				if (!listPtr->notselectable) {
					listPtr->cursorPos++;
					if (listPtr->cursorPos < listPtr->startPos) 
						listPtr->startPos = listPtr->cursorPos;
					
					if (listPtr->cursorPos >= count) 
						listPtr->cursorPos = count-1;
					
					if (listPtr->cursorPos >= listPtr->startPos + viewmax) 
						listPtr->startPos = listPtr->cursorPos - viewmax + 1;
					
					item->cursorPos = listPtr->cursorPos;
					dc_->feederSelection(item->special, item->cursorPos);
				}
				else 
				{
					listPtr->startPos++;
					if (listPtr->startPos >= count)
						listPtr->startPos = count-1;
				}
				return true;
			}
		}
		else 
		{
			viewmax = (item->window.rect.h / listPtr->elementHeight);
			if ( key == K_UPARROW || key == K_KP_UPARROW ) 
			{
				if (!listPtr->notselectable) 
				{
					listPtr->cursorPos--;
					if (listPtr->cursorPos < 0) 
						listPtr->cursorPos = 0;
					
					if (listPtr->cursorPos < listPtr->startPos) 
						listPtr->startPos = listPtr->cursorPos;
					
					if (listPtr->cursorPos >= listPtr->startPos + viewmax) 
						listPtr->startPos = listPtr->cursorPos - viewmax + 1;
					
					item->cursorPos = listPtr->cursorPos;
					dc_->feederSelection(item->special, item->cursorPos);
				}
				else 
				{
					listPtr->startPos--;
					if (listPtr->startPos < 0)
						listPtr->startPos = 0;
				}
				return true;
			}
			if ( key == K_DOWNARROW || key == K_KP_DOWNARROW ) 
			{
				if (!listPtr->notselectable) 
				{
					listPtr->cursorPos++;
					if (listPtr->cursorPos < listPtr->startPos) 
						listPtr->startPos = listPtr->cursorPos;
					
					if (listPtr->cursorPos >= count) 
						listPtr->cursorPos = count-1;
					
					if (listPtr->cursorPos >= listPtr->startPos + viewmax) 
						listPtr->startPos = listPtr->cursorPos - viewmax + 1;
					
					item->cursorPos = listPtr->cursorPos;
					dc_->feederSelection(item->special, item->cursorPos);
				}
				else 
				{
					listPtr->startPos++;
					if (listPtr->startPos > max)
						listPtr->startPos = max;
				}
				return true;
			}
		}
		// mouse hit
		if (key == K_MOUSE1 || key == K_MOUSE2) 
		{
			if (item->window.flags & WINDOW_LB_LEFTARROW) 
			{
				listPtr->startPos--;
				if (listPtr->startPos < 0) 
					listPtr->startPos = 0;
	
			} 
			else if (item->window.flags & WINDOW_LB_RIGHTARROW) 
			{
				// one down
				listPtr->startPos++;
				if (listPtr->startPos > max) 
					listPtr->startPos = max;
				
			} 
			else if (item->window.flags & WINDOW_LB_PGUP) 
			{
				// page up
				listPtr->startPos -= viewmax;
				if (listPtr->startPos < 0) 
					listPtr->startPos = 0;
				
			} 
			else if (item->window.flags & WINDOW_LB_PGDN) 
			{
				// page down
				listPtr->startPos += viewmax;
				if (listPtr->startPos > max) 
					listPtr->startPos = max;
				
			} 
			else if (item->window.flags & WINDOW_LB_THUMB) 
			{
				// Display_SetCaptureItem(item);
			} 
			else 
			{
				// select an item
				if (dc_->realTime_ < lastListBoxClickTime_ && listPtr->doubleClick) 
					item_RunScript(item, listPtr->doubleClick);
				
				lastListBoxClickTime_ = dc_->realTime_ + DOUBLE_CLICK_DELAY;
				if (item->cursorPos != listPtr->cursorPos) 
				{
					item->cursorPos = listPtr->cursorPos;
					dc_->feederSelection(item->special, item->cursorPos);
				}
			}
			return true;
		}
		if ( key == K_HOME || key == K_KP_HOME) 
		{
			// home
			listPtr->startPos = 0;
			return true;
		}
		if ( key == K_END || key == K_KP_END) 
		{
			// end
			listPtr->startPos = max;
			return true;
		}
		if (key == K_PGUP || key == K_KP_PGUP ) 
		{
			// page up
			if (!listPtr->notselectable) 
			{
				listPtr->cursorPos -= viewmax;
				if (listPtr->cursorPos < 0) 
					listPtr->cursorPos = 0;
				
				if (listPtr->cursorPos < listPtr->startPos) 
					listPtr->startPos = listPtr->cursorPos;
				
				if (listPtr->cursorPos >= listPtr->startPos + viewmax) 
					listPtr->startPos = listPtr->cursorPos - viewmax + 1;
				
				item->cursorPos = listPtr->cursorPos;
				dc_->feederSelection(item->special, item->cursorPos);
			}
			else 
			{
				listPtr->startPos -= viewmax;
				if (listPtr->startPos < 0) 
					listPtr->startPos = 0;
				
			}
			return true;
		}
		if ( key == K_PGDN || key == K_KP_PGDN ) 
		{
			// page down
			if (!listPtr->notselectable) 
			{
				listPtr->cursorPos += viewmax;
				if (listPtr->cursorPos < listPtr->startPos) 
					listPtr->startPos = listPtr->cursorPos;
				
				if (listPtr->cursorPos >= count) 
					listPtr->cursorPos = count-1;
				
				if (listPtr->cursorPos >= listPtr->startPos + viewmax) 
					listPtr->startPos = listPtr->cursorPos - viewmax + 1;
				
				item->cursorPos = listPtr->cursorPos;
				dc_->feederSelection(item->special, item->cursorPos);
			}
			else 
			{
				listPtr->startPos += viewmax;
				if (listPtr->startPos > max) 
					listPtr->startPos = max;
				
			}
			return true;
		}
	}
	return false;
}

bool
UI_Utils::item_YesNo_HandleKey( itemDef_t *item, int key )  
{

  if (rect_ContainsPoint(&item->window.rect, dc_->cursorX_, dc_->cursorY_) && 
	  (item->window.flags & WINDOW_HASFOCUS) && item->cvar) 
  {
		if (key == K_MOUSE1 || key == K_ENTER || key == K_MOUSE2 || key == K_MOUSE3) 
		{
			dc_->setCVar(item->cvar, va("%i", !dc_->getCVarValue(item->cvar)));
			return true;
		}
  }
  return false;
}

bool 
UI_Utils::item_Multi_HandleKey( itemDef_t *item, int key ) 
{
	multiDef_t *multiPtr = (multiDef_t*)item->typeData;
	if (multiPtr) 
	{
	  if (rect_ContainsPoint(&item->window.rect, dc_->cursorX_, dc_->cursorY_) && 
		  (item->window.flags & WINDOW_HASFOCUS) && item->cvar) {
			if (key == K_MOUSE1 || key == K_ENTER || key == K_MOUSE2 || key == K_MOUSE3) {
				int current = item_Multi_FindCvarByValue(item) + 1;
				int max = item_Multi_CountSettings(item);
				if ( current < 0 || current >= max ) {
					current = 0;
				}
				if (multiPtr->strDef) 
				{
					dc_->setCVar(item->cvar, multiPtr->cvarStr[current]);
				} 
				else 
				{
					float value = multiPtr->cvarValue[current];
					if (((float)((int) value)) == value) 
					{
						dc_->setCVar(item->cvar, va("%i", (int) value ));
					}
					else 
					{
						dc_->setCVar(item->cvar, va("%f", value ));
					}
				}
				return true;
			}
		}
	}
	return false;
}

bool 
UI_Utils::item_Slider_HandleKey( itemDef_t *item, int key, int down ) 
{
	float x, value, width, work;

	//DC->Print("slider handle key\n");
	if (item->window.flags & WINDOW_HASFOCUS && 
		item->cvar && rect_ContainsPoint(&item->window.rect, dc_->cursorX_, dc_->cursorY_)) 
	{
		if (key == K_MOUSE1 || key == K_ENTER || key == K_MOUSE2 || key == K_MOUSE3) 
		{
			editFieldDef_t *editDef = reinterpret_cast<editFieldDef_t*>(item->typeData);
			if (editDef) 
			{
				rectDef_t testRect;
				width = SLIDER_WIDTH;
				if (item->text) 
					x = item->textRect.x + item->textRect.w + 8;
				else 
					x = item->window.rect.x;

				testRect = item->window.rect;
				testRect.x = x;
				value = (float)SLIDER_THUMB_WIDTH / 2;
				testRect.x -= value;
				//DC->Print("slider x: %f\n", testRect.x);
				testRect.w = (SLIDER_WIDTH + (float)SLIDER_THUMB_WIDTH / 2);
				//DC->Print("slider w: %f\n", testRect.w);
				if (rect_ContainsPoint(&testRect, dc_->cursorX_, dc_->cursorY_)) 
				{
					work = dc_->cursorX_ - x;
					value = work / width;
					value *= (editDef->maxVal - editDef->minVal);
					// vm fuckage
					// value = (((float)(DC->cursorx - x)/ SLIDER_WIDTH) * (editDef->maxVal - editDef->minVal));
					value += editDef->minVal;
					dc_->setCVar(item->cvar, va("%f", value));
					return true;
				}
			}
		}
	}
	Com_Printf("slider handle key exit\n");
	return false;
}

int 
UI_Utils::item_Multi_CountSettings( itemDef_t *item ) 
{
	multiDef_t *multiPtr = (multiDef_t*)item->typeData;
	if (multiPtr == NULL) 
	{
		return 0;
	}
	return multiPtr->count;
}

int 
UI_Utils::item_Multi_FindCvarByValue( itemDef_t *item ) 
{
	char buff[1024];
	float value = 0;
	int i;
	multiDef_t *multiPtr = (multiDef_t*)item->typeData;
	if (multiPtr) 
	{
		if (multiPtr->strDef) 
		{
			dc_->getCVarString(item->cvar, buff, sizeof(buff));
		} 
		else 
		{
			value = dc_->getCVarValue(item->cvar);
		}
		for (i = 0; i < multiPtr->count; i++) 
		{
			if (multiPtr->strDef) 
			{
				if (Q_stricmp(buff, multiPtr->cvarStr[i]) == 0) 
				{
					return i;
				}
			} 
			else 
			{
 				if (multiPtr->cvarValue[i] == value) 
				{
 					return i;
 				}
 			}
 		}
	}
	return 0;
}

int 
UI_Utils::item_Slider_OverSlider( itemDef_t *item, float x, float y ) 
{
	rectDef_t r;

	r.x = item_Slider_ThumbPosition(item) - (SLIDER_THUMB_WIDTH / 2);
	r.y = item->window.rect.y - 2;
	r.w = SLIDER_THUMB_WIDTH;
	r.h = SLIDER_THUMB_HEIGHT;

	if (rect_ContainsPoint(&r, x, y)) {
		return WINDOW_LB_THUMB;
	}
	return 0;
}

void
UI_Utils::item_Image_Paint( itemDef_t *item ) 
{
	if (item == NULL) 
		return;

	dc_->drawHandlePic(item->window.rect.x+1, item->window.rect.y+1, item->window.rect.w-2, 
						item->window.rect.h-2, item->asset);
}

void
UI_Utils::item_Init( itemDef_t *item ) 
{
	memset(item, 0, sizeof(itemDef_t));
	item->textscale = 0.55f;
	window_Init(&item->window);
}

itemDef_t*
UI_Utils::menu_FindItemByName( menuDef_t *menu, const char *p )
{
	int i;
	if (menu == NULL || p == NULL) 
		return NULL;
  
	for (i = 0; i < menu->itemCount; i++) 
	{
		if (Q_stricmp(p, menu->items[i]->window.name) == 0) 
		{
			return menu->items[i];
		}
	}

	return NULL;
}

itemDef_t*
UI_Utils::menu_ClearFocus(menuDef_t *menu) 
{
	int i;
	itemDef_t *ret = NULL;

	if (menu == NULL)
		return NULL;
 
	for( i = 0; i < menu->itemCount; i++ ) 
	{
		if (menu->items[i]->window.flags & WINDOW_HASFOCUS)
			ret = menu->items[i];
 
		menu->items[i]->window.flags &= ~WINDOW_HASFOCUS;
		if (menu->items[i]->leaveFocus) 
			item_RunScript(menu->items[i], menu->items[i]->leaveFocus);
	}
	return ret;
}

void 
UI_Utils::menu_FadeItemByName( menuDef_t *menu, const char *p, bool fadeOut ) 
{
	itemDef_t *item;
	int i;
	int count = menu_ItemsMatchingGroup(menu, p);
	for (i = 0; i < count; i++) 
	{
		item = menu_GetMatchingItemByNumber(menu, i, p);
		if (item != NULL) 
		{
			if (fadeOut) 
			{
				item->window.flags |= (WINDOW_FADINGOUT | WINDOW_VISIBLE);
				item->window.flags &= ~WINDOW_FADINGIN;
			} 
			else 
			{
				item->window.flags |= (WINDOW_VISIBLE | WINDOW_FADINGIN);
				item->window.flags &= ~WINDOW_FADINGOUT;
			}
		}
	}
}

void 
UI_Utils::menu_ShowItemByName(menuDef_t *menu, const char *p, bool bShow) 
{
	itemDef_t *item;
	int i;
	int count = menu_ItemsMatchingGroup(menu, p);
	for (i = 0; i < count; i++) 
	{
		item = menu_GetMatchingItemByNumber(menu, i, p);
		if (item != NULL) 
		{
			if (bShow) 
			{
				item->window.flags |= WINDOW_VISIBLE;
			} 
			else 
			{
				item->window.flags &= ~WINDOW_VISIBLE;
				// stop cinematics playing in the window
				if (item->window.cinematic >= 0) 
				{
					dc_->stopCinematic(item->window.cinematic);
					item->window.cinematic = -1;
				}
			}
		}
	}
}

itemDef_t*
UI_Utils::menu_GetMatchingItemByNumber(menuDef_t *menu, int index, const char *name) 
{
	int i;
	int count = 0;
	for (i = 0; i < menu->itemCount; i++) 
	{
		if( Q_stricmp(menu->items[i]->window.name, name) == 0 || 
			(menu->items[i]->window.group && Q_stricmp(menu->items[i]->window.group, name) == 0) ) 
		{
			if (count == index) 
			{
				return menu->items[i];
			}
			count++;
		} 
	}
	return NULL;
}

void 
UI_Utils::menu_OpenByName(const char *p) 
{
	menu_ActivateByName(p);
}

void 
UI_Utils::menu_CloseByName(const char *p) 
{
	menuDef_t *menu = menu_FindByName(p);
	if (menu != NULL) 
	{
		menu_RunCloseScript(menu);
		menu->window.flags &= ~(WINDOW_VISIBLE | WINDOW_HASFOCUS);
	}
}

void 
UI_Utils::menu_RunCloseScript(menuDef_t *menu) 
{
	if (menu && (menu->window.flags & WINDOW_VISIBLE) && menu->onClose) 
	{
		itemDef_t item;
		item.parent = menu;
		item_RunScript(&item, menu->onClose);
	}
}

int 
UI_Utils::menu_ItemsMatchingGroup(menuDef_t *menu, const char *name) 
{
	int i;
	int count = 0;
	for (i = 0; i < menu->itemCount; i++) 
	{
		if( Q_stricmp(menu->items[i]->window.name, name) == 0 || 
			(menu->items[i]->window.group && Q_stricmp(menu->items[i]->window.group, name) == 0) ) 
		{
			count++;
		} 
	}
	return count;
}

void 
UI_Utils::menu_TransitionItemByName(menuDef_t *menu, const char *p, rectDef_t rectFrom, rectDef_t rectTo, 
									int time, float amt) 
{
	itemDef_t *item;
	int i;
	int count = menu_ItemsMatchingGroup(menu, p);
	for (i = 0; i < count; i++) 
	{
		item = menu_GetMatchingItemByNumber(menu, i, p);
		if (item != NULL) 
		{
			item->window.flags |= (WINDOW_INTRANSITION | WINDOW_VISIBLE);
			item->window.offsetTime = time;
			memcpy(&item->window.rectClient, &rectFrom, sizeof(rectDef_t));
			memcpy(&item->window.rectEffects, &rectTo, sizeof(rectDef_t));
			item->window.rectEffects2.x = abs(rectTo.x - rectFrom.x) / amt;
			item->window.rectEffects2.y = abs(rectTo.y - rectFrom.y) / amt;
			item->window.rectEffects2.w = abs(rectTo.w - rectFrom.w) / amt;
			item->window.rectEffects2.h = abs(rectTo.h - rectFrom.h) / amt;
			item_UpdatePosition(item);
		}
	}
}

void 
UI_Utils::menu_OrbitItemByName(menuDef_t *menu, const char *p, float x, float y, float cx, float cy, int time) 
{
	itemDef_t *item;
	int i;
	int count = menu_ItemsMatchingGroup(menu, p);
	for (i = 0; i < count; i++) 
	{
		item = menu_GetMatchingItemByNumber(menu, i, p);
		if (item != NULL) 
		{
			item->window.flags |= (WINDOW_ORBITING | WINDOW_VISIBLE);
			item->window.offsetTime = time;
			item->window.rectEffects.x = cx;
			item->window.rectEffects.y = cy;
			item->window.rectClient.x = x;
			item->window.rectClient.y = y;
			item_UpdatePosition(item);
		}
	}
}

// menus
void 
UI_Utils::menu_UpdatePosition( menuDef_t *menu ) 
{
	int i;
	float x, y;

	if (menu == NULL)
		return;
	  
	x = menu->window.rect.x;
	y = menu->window.rect.y;
	if (menu->window.border != 0) 
	{
		x += menu->window.borderSize;
		y += menu->window.borderSize;
	}

	for (i = 0; i < menu->itemCount; i++) 
	{
		item_SetScreenCoords(menu->items[i], x, y);
	}
}

void 
UI_Utils::menu_PostParse( menuDef_t *menu) 
{
	if (menu == NULL) 
		return;

	if (menu->fullScreen) 
	{
		menu->window.rect.x = 0;
		menu->window.rect.y = 0;
		menu->window.rect.w = 640;
		menu->window.rect.h = 480;
	}
	menu_UpdatePosition(menu);
}


void
UI_Utils::menu_CloseAll()
{
	int i;
	for (i = 0; i < menuCount_; i++) 
	{
			menu_RunCloseScript(&menus_[i]);
			menus_[i].window.flags &= ~(WINDOW_HASFOCUS | WINDOW_VISIBLE);
	}
}

menuDef_t*
UI_Utils::menu_FindByName( const char *p ) 
{
	int i;
	for (i = 0; i < menuCount_; i++) 
	{
		if (Q_stricmp(menus_[i].window.name, p) == 0) 
		{
			return &menus_[i];
		} 
	}
	return NULL;
}

void 
UI_Utils::menu_ShowByName( const char *p ) 
{
	menuDef_t *menu = menu_FindByName(p);
	if (menu) 
		menu_Activate(menu);
}

void 
UI_Utils::menu_CloseCinematics( menuDef_t *menu ) 
{
	if( menu ) 
	{
		int i;
		window_CloseCinematic(&menu->window);
		for (i = 0; i < menu->itemCount; i++) 
		{
			window_CloseCinematic(&menu->items[i]->window);
			if (menu->items[i]->type == ITEM_TYPE_OWNERDRAW) 
			{
				dc_->stopCinematic(0-menu->items[i]->window.ownerDraw);
			}
		}
	}
}


void 
UI_Utils::menu_HandleOOBClick( menuDef_t *menu, int key, int down ) 
{
	if (menu) 
	{
		int i;
		// basically the behaviour we are looking for is if there are windows in the stack.. see if 
		// the cursor is within any of them.. if not close them otherwise activate them and pass the 
		// key on.. force a mouse move to activate focus and script stuff 
		if (down && menu->window.flags & WINDOW_OOB_CLICK) 
		{
			menu_RunCloseScript(menu);
			menu->window.flags &= ~(WINDOW_HASFOCUS | WINDOW_VISIBLE);
		}

		for (i = 0; i < menuCount_; i++) 
		{
			if (menu_OverActiveItem(&menus_[i], dc_->cursorX_, dc_->cursorY_)) 
			{
				menu_RunCloseScript(menu);
				menu->window.flags &= ~(WINDOW_HASFOCUS | WINDOW_VISIBLE);
				menu_Activate(&menus_[i]);
				menu_HandleMouseMove(&menus_[i], dc_->cursorX_, dc_->cursorY_);
				menu_HandleKey(&menus_[i], key, down);
			}
		}

		if (display_VisibleMenuCount() == 0) 
		{
//			if (gDC->Pause) {
				dc_->pause(false);
//			}
		}
		display_CloseCinematics();
	}
}

bool 
UI_Utils::menu_OverActiveItem( menuDef_t *menu, float x, float y )
{
 	if (menu && menu->window.flags & (WINDOW_VISIBLE | WINDOW_FORCED)) 
	{
		if (rect_ContainsPoint(&menu->window.rect, x, y)) 
		{
			int i;
			for (i = 0; i < menu->itemCount; i++) 
			{
				// turn off focus each item
				// menu->items[i].window.flags &= ~WINDOW_HASFOCUS;

				if (!(menu->items[i]->window.flags & (WINDOW_VISIBLE | WINDOW_FORCED))) 
					continue;

				if (menu->items[i]->window.flags & WINDOW_DECORATION) 
					continue;

				if (rect_ContainsPoint(&menu->items[i]->window.rect, x, y))
				{
					itemDef_t *overItem = menu->items[i];
					if (overItem->type == ITEM_TYPE_TEXT && overItem->text)
					{
						if (rect_ContainsPoint(item_CorrectedTextRect(overItem), x, y)) 
							return true;
						else 
							continue;
					} 
					else 
						return true;
				}
			}

		}
	}
	return false;
}

menuDef_t*
UI_Utils::menu_GetFocused() 
{
	int i;
	for (i = 0; i < menuCount_; i++) 
	{
	    if( (menus_[i].window.flags & WINDOW_HASFOCUS) && menus_[i].window.flags & WINDOW_VISIBLE) {
		return &menus_[i];
		}
	}
	return NULL;
}

bool
UI_Utils::menu_AnyFullScreenVisible() 
{
	int i;
	for (i = 0; i < menuCount_; i++) 
	{
		if( (menus_[i].window.flags & WINDOW_VISIBLE) && menus_[i].fullScreen) 
		{
			return true;
		}
	}
	return false;
}

menuDef_t*
UI_Utils::menu_ActivateByName( const char *p )
{
	int i;
	menuDef_t *m = NULL;
	menuDef_t *focus = menu_GetFocused();
	for (i = 0; i < menuCount_; i++) 
	{
		if (Q_stricmp(menus_[i].window.name, p) == 0) 
		{
			m = &menus_[i];
			menu_Activate(m);
			if (openMenuCount_ < MAX_OPEN_MENUS && focus != NULL) 
			{
				menuStack_[openMenuCount_++] = focus;
			}
		} 
		else 
		{
			menus_[i].window.flags &= ~WINDOW_HASFOCUS;
		}
	}
	display_CloseCinematics();
	return m;
}

void 
UI_Utils::menu_Activate( menuDef_t *menu ) 
{
	menu->window.flags |= (WINDOW_HASFOCUS | WINDOW_VISIBLE);
	if (menu->onOpen) 
	{
		itemDef_t item;
		item.parent = menu;
		item_RunScript(&item, menu->onOpen);
	}

	if (menu->soundName && *menu->soundName) 
	{
//		DC->stopBackgroundTrack();					// you don't want to do this since it will reset s_rawend
		dc_->startBackgroundTrack(menu->soundName, menu->soundName);
	}

	display_CloseCinematics();
}

void 
UI_Utils::menu_New( int handle ) 
{
	menuDef_t *menu = &menus_[menuCount_];

	if( menuCount_ < MAX_MENUS ) 
	{
		menu_Init(menu);
		if( menu_Parse(handle, menu) ) 
		{
			menu_PostParse(menu);
			menuCount_++;
		}
	}
}

void 
UI_Utils::menu_Init(menuDef_t *menu) 
{
	memset(menu, 0, sizeof(menuDef_t));
	menu->cursorItem = -1;
	menu->fadeAmount = dc_->assets_.fadeAmount;
	menu->fadeClamp = dc_->assets_.fadeClamp;
	menu->fadeCycle = dc_->assets_.fadeCycle;
	window_Init(&menu->window);
}

bool 
UI_Utils::menu_Parse( int handle, menuDef_t *menu ) 
{
	UI_PrecompilerTools::PC_Token  token;
//	keywordHash_t *key;

	if (!precomp_.readTokenHandle(handle, &token))
		return false;
	if (*token.string_ != '{')
		return false;
    
	while ( 1 ) 
	{
		memset(&token, 0, sizeof(UI_PrecompilerTools::PC_Token ));
		if (!precomp_.readTokenHandle(handle, &token)) 
		{
			pc_SourceError(handle, "end of file inside menu\n");
			return false;
		}

		if (*token.string_ == '}') 
			return true;

		KeywordHash* key = hashUtils_->findMenuKeyword( token.string_ );

		if (!key)
		{
			pc_SourceError(handle, "unknown menu keyword %s", token.string_);
			continue;
		}
		if ( !key->func((itemDef_t*)menu, handle) ) 
		{
			pc_SourceError(handle, "couldn't parse menu keyword %s", token.string_);
			return false;
		}
	}
	return false; 	// bk001205 - LCC missing return value
}


void 
UI_Utils::menu_Paint( menuDef_t *menu, bool forcePaint ) 
{
	int i;

	if (menu == NULL) 
		return;

	if (!(menu->window.flags & WINDOW_VISIBLE) &&  !forcePaint) 
		return;

	if (menu->window.ownerDrawFlags /*&& gDC->ownerDrawVisible*/ && !dc_->ownerDrawVisible(menu->window.ownerDrawFlags)) 
		return;
	
	if (forcePaint) 
	{
		menu->window.flags |= WINDOW_FORCED;
	}

	// draw the background if necessary
	if (menu->fullScreen) 
	{
		// implies a background shader
		// FIXME: make sure we have a default shader if fullscreen is set with no background
		dc_->drawHandlePic( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, menu->window.background );
	} 
	else if (menu->window.background) 
	{
		// this allows a background shader without being full screen
		//UI_DrawHandlePic(menu->window.rect.x, menu->window.rect.y, menu->window.rect.w, menu->window.rect.h, menu->backgroundShader);
	}

	// paint the background and or border
	window_Paint(&menu->window, menu->fadeAmount, menu->fadeClamp, menu->fadeCycle );

	for( i = 0; i < menu->itemCount; i++ )
	{
		item_Paint(menu->items[i]);
	}

	if (debugMode_) 
	{
		vec4_t color;
		color[0] = color[2] = color[3] = 1;
		color[1] = 0;
		dc_->drawRect(menu->window.rect.x, menu->window.rect.y, menu->window.rect.w, menu->window.rect.h, 1, color);
	}
}

void
UI_Utils::menu_CacheContents( menuDef_t *menu ) 
{
	if( menu ) 
	{
		int i;
		window_CacheContents( &menu->window );
		for( i = 0; i < menu->itemCount; i++ ) 
		{
			item_CacheContents(menu->items[i]);
		}

		if( menu->soundName && *menu->soundName ) 
		{
			dc_->registerSound(menu->soundName, false);
		}
	}
}


void 
UI_Utils::menu_HandleMouseMove( menuDef_t *menu, float x, float y ) 
{
	if (menu == NULL) 
		return;
	
	if (!(menu->window.flags & (WINDOW_VISIBLE | WINDOW_FORCED)))
		return;
	
	if (itemCapture_) 
	{
		//Item_MouseMove(itemCapture, x, y);
		return;
	}

	if (waitingForKey_ || editingField_) {
		return;
	}

	int i, pass;
	bool focusSet = false;
	itemDef_t *overItem;

	// FIXME: this is the whole issue of focus vs. mouse over.. 
	// need a better overall solution as i don't like going through everything twice
	for (pass = 0; pass < 2; pass++) 
	{
		for (i = 0; i < menu->itemCount; i++) 
		{
			// turn off focus each item
			// menu->items[i].window.flags &= ~WINDOW_HASFOCUS;

			if (!(menu->items[i]->window.flags & (WINDOW_VISIBLE | WINDOW_FORCED))) 
				continue;

			// items can be enabled and disabled based on cvars
			if (menu->items[i]->cvarFlags & (CVAR_ENABLE | CVAR_DISABLE) && 
				!item_EnableShowViaCvar(menu->items[i], CVAR_ENABLE)) 
			{
				continue;
			}

			if (menu->items[i]->cvarFlags & (CVAR_SHOW | CVAR_HIDE) && 
				!item_EnableShowViaCvar(menu->items[i], CVAR_SHOW)) 
			{
				continue;
			}

			if (rect_ContainsPoint(&menu->items[i]->window.rect, x, y)) 
			{
				if (pass == 1) 
				{
					overItem = menu->items[i];
					if (overItem->type == ITEM_TYPE_TEXT && overItem->text) 
					{
						if (!rect_ContainsPoint(item_CorrectedTextRect(overItem), x, y)) 
							continue;
					}
					// if we are over an item
					if (isVisible(overItem->window.flags)) 
					{
						// different one
						item_MouseEnter(overItem, x, y);
						// Item_SetMouseOver(overItem, true);

						// if item is not a decoration see if it can take focus
						if (!focusSet)
							focusSet = item_SetFocus(overItem, x, y);
					}
				}
			} 
			else if (menu->items[i]->window.flags & WINDOW_MOUSEOVER) 
			{
				item_MouseLeave(menu->items[i]);
				item_SetMouseOver(menu->items[i], false);
			}
		}
	}
}

void 
UI_Utils::menu_HandleKey( menuDef_t *menu, int key, int down ) 
{
	int i;
	itemDef_t *item = NULL;
	bool inHandler = false;

	if (inHandler) 
		return;

	inHandler = true;
	if (waitingForKey_ && down) 
	{
		item_Bind_HandleKey(bindItem_, key, down);
		inHandler = false;
		return;
	}

	if (editingField_ && down) 
	{
		if (!item_TextField_HandleKey(editItem_, key)) 
		{
			editingField_ = false;
			editItem_ = NULL;
			inHandler = false;
			return;
		} 
		else if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_MOUSE3) 
		{
			editingField_ = false;
			editItem_ = NULL;
			display_MouseMove(NULL, dc_->cursorX_, dc_->cursorY_);
		} 
		else if (key == K_TAB || key == K_UPARROW || key == K_DOWNARROW) 
		{
			return;
		}
	}

	if (menu == NULL) 
	{
		inHandler = false;
		return;
	}

		// see if the mouse is within the window bounds and if so is this a mouse click
	if (down && !(menu->window.flags & WINDOW_POPUP) && !rect_ContainsPoint(&menu->window.rect, dc_->cursorX_, dc_->cursorY_)) 
	{
		static bool inHandleKey = false;
		// bk001206 - parentheses
		if (!inHandleKey && ( key == K_MOUSE1 || key == K_MOUSE2 || key == K_MOUSE3 ) ) 
		{
			inHandleKey = true;
			menu_HandleOOBClick(menu, key, down);
			inHandleKey = false;
			inHandler = false;
			return;
		}
	}

	// get the item with focus
	for (i = 0; i < menu->itemCount; i++) 
	{
		if (menu->items[i]->window.flags & WINDOW_HASFOCUS) 
		{
			item = menu->items[i];
		}
	}

	if (item != NULL) 
	{
		if (item_HandleKey(item, key, down)) 
		{
			item_Action(item);
			inHandler = false;
			return;
		}
	}

	if (!down) 
	{
		inHandler = false;
		return;
	}

	// default handling
	switch ( key ) 
	{
		case K_F11:
			if (dc_->getCVarValue("developer")) 
			{
				debugMode_ ^= 1;
			}
			break;

		case K_F12:
			if (dc_->getCVarValue("developer")) 
				dc_->executeText(EXEC_APPEND, "screenshot\n");
			break;
		case K_KP_UPARROW:
		case K_UPARROW:
			menu_SetPrevCursorItem(menu);
			break;

		case K_ESCAPE:
			if (!waitingForKey_ && menu->onESC) 
			{
				itemDef_t it;
				it.parent = menu;
				item_RunScript(&it, menu->onESC);
			}
			break;
		case K_TAB:
		case K_KP_DOWNARROW:
		case K_DOWNARROW:
			menu_SetNextCursorItem(menu);
			break;

		case K_MOUSE1:
		case K_MOUSE2:
			if (item) 
			{
				if (item->type == ITEM_TYPE_TEXT) 
				{
					if (rect_ContainsPoint(item_CorrectedTextRect(item), dc_->cursorX_, dc_->cursorY_)) 
					{
						item_Action(item);
					}
				} 
				else if (item->type == ITEM_TYPE_EDITFIELD || item->type == ITEM_TYPE_NUMERICFIELD) 
				{
					if (rect_ContainsPoint(&item->window.rect, dc_->cursorX_, dc_->cursorY_)) {
						item->cursorPos = 0;
						editingField_ = true;
						editItem_ = item;
						dc_->setOverstrikeMode(true);
					}
				} 
				else 
				{
					if (rect_ContainsPoint(&item->window.rect, dc_->cursorX_, dc_->cursorY_)) 
					{
						item_Action(item);
					}
				}
			}
			break;

		case K_JOY1:
		case K_JOY2:
		case K_JOY3:
		case K_JOY4:
		case K_AUX1:
		case K_AUX2:
		case K_AUX3:
		case K_AUX4:
		case K_AUX5:
		case K_AUX6:
		case K_AUX7:
		case K_AUX8:
		case K_AUX9:
		case K_AUX10:
		case K_AUX11:
		case K_AUX12:
		case K_AUX13:
		case K_AUX14:
		case K_AUX15:
		case K_AUX16:
			break;
		case K_KP_ENTER:
		case K_ENTER:
			if (item) 
			{
				if (item->type == ITEM_TYPE_EDITFIELD || item->type == ITEM_TYPE_NUMERICFIELD) 
				{
					item->cursorPos = 0;
					editingField_ = true;
					editItem_ = item;
					dc_->setOverstrikeMode(true);
				} 
				else 
				{
					item_Action(item);
				}
			}
			break;
	}
	inHandler = false;
}

itemDef_t* 
UI_Utils::menu_SetNextCursorItem( menuDef_t *menu ) 
{

	bool wrapped = false;
	int oldCursor = menu->cursorItem;


	if (menu->cursorItem == -1) 
	{
		menu->cursorItem = 0;
		wrapped = true;
	}

	while (menu->cursorItem < menu->itemCount) 
	{
	    menu->cursorItem++;
		if (menu->cursorItem >= menu->itemCount && !wrapped) 
		{
			wrapped = true;
			menu->cursorItem = 0;
		}
		if (item_SetFocus(menu->items[menu->cursorItem], dc_->cursorX_, dc_->cursorY_)) 
		{
			menu_HandleMouseMove(menu, menu->items[menu->cursorItem]->window.rect.x + 1, menu->items[menu->cursorItem]->window.rect.y + 1);
			return menu->items[menu->cursorItem];
		}
	}
	menu->cursorItem = oldCursor;
	return NULL;
}


itemDef_t* 
UI_Utils::menu_SetPrevCursorItem( menuDef_t *menu ) 
{
	bool wrapped = false;
	int oldCursor = menu->cursorItem;
  
	if (menu->cursorItem < 0) 
	{
		menu->cursorItem = menu->itemCount-1;
		wrapped = true;
	} 

	while (menu->cursorItem > -1) 
	{
        menu->cursorItem--;
		if (menu->cursorItem < 0 && !wrapped) 
		{
			wrapped = true;
			menu->cursorItem = menu->itemCount -1;
		}

		if (item_SetFocus(menu->items[menu->cursorItem], dc_->cursorX_, dc_->cursorY_)) 
		{
			menu_HandleMouseMove(menu, menu->items[menu->cursorItem]->window.rect.x + 1, 
								 menu->items[menu->cursorItem]->window.rect.y + 1);
			return menu->items[menu->cursorItem];
		}
	}
	menu->cursorItem = oldCursor;
	return NULL;
}

int
UI_Utils::menu_Count() const	
{
	return menuCount_;
}

void 
UI_Utils::menu_Reset() 
{
	menuCount_ = 0;
}

void 
UI_Utils::menu_PaintAll() 
{
	int i;
	if (captureTool_) 
	{
		captureTool_->capture();
	}

	for (i = 0; i < menuCount_; i++) 
	{
		menu_Paint(&menus_[i], false);
	}

	if (debugMode_) 
	{
		vec4_t v = {1, 1, 1, 1};
		dc_->drawText(5, 25, .5, v, va("fps: %f", dc_->fps_), 0, 0, 0);
	}
}

itemDef_t* 
UI_Utils::menu_GetFocusedItem( menuDef_t *menu ) 
{
	int i;
	if (menu) 
	{
		for (i = 0; i < menu->itemCount; i++) 
		{
			if (menu->items[i]->window.flags & WINDOW_HASFOCUS) 
			{
				return menu->items[i];
			}
		}
	}
	return NULL;
}

void 
UI_Utils::menu_ScrollFeeder( menuDef_t *menu, int feeder, int down ) 
{
	if (menu) 
	{
		int i;
		for (i = 0; i < menu->itemCount; i++) 
		{
			if (menu->items[i]->special == feeder) 
			{
				item_ListBox_HandleKey(menu->items[i], (down) ? K_DOWNARROW : K_UPARROW, 1, true);
				return;
			}
		}
	}
}

void 
UI_Utils::menu_SetFeederSelection( menuDef_t *menu, int feeder, int index, const char *name ) 
{
	if (menu == NULL) 
	{
		if (name == NULL) 
		{
			menu = menu_GetFocused();
		} 
		else 
		{
			menu = menu_FindByName(name);
		}
	}

	if (menu) 
	{
		for (int i = 0; i < menu->itemCount; i++) 
		{
			if (menu->items[i]->special == feeder) 
			{
				if (index == 0) 
				{
					listBoxDef_t *listPtr = (listBoxDef_t*)menu->items[i]->typeData;
					listPtr->cursorPos = 0;
					listPtr->startPos = 0;
				}
				menu->items[i]->cursorPos = index;
				dc_->feederSelection(menu->items[i]->special, menu->items[i]->cursorPos);
				return;
			}
		}
	}
}

itemDef_t* 
UI_Utils::menu_HitTest( menuDef_t *menu, float x, float y )
{
	int i;
	for (i = 0; i < menu->itemCount; i++) 
	{
	    if (rect_ContainsPoint(&menu->items[i]->window.rect, x, y)) 
			return menu->items[i];
	}
	return NULL;
}

void 
UI_Utils::display_CloseCinematics() 
{
	int i;
	for (i = 0; i < menuCount_; i++) 
	{
		menu_CloseCinematics(&menus_[i]);
	}
}

int 
UI_Utils::display_VisibleMenuCount() 
{
	int i, count;
	count = 0;
	for (i = 0; i < menuCount_; i++) 
	{
		if (menus_[i].window.flags & (WINDOW_FORCED | WINDOW_VISIBLE)) 
		{
			count++;
		}
	}
	return count;
}

void 
UI_Utils::display_CacheAll() 
{
	int i;
	for (i = 0; i < menuCount_; i++) 
	{
		menu_CacheContents(&menus_[i]);
	}
}

void 
UI_Utils::display_HandleKey( int key, bool down, int x, int y ) 
{
	menuDef_t *menu = reinterpret_cast<menuDef_t*>(display_CaptureItem(x, y));
	if (menu == NULL)  
		menu = menu_GetFocused();

	if (menu) 
		menu_HandleKey(menu, key, down );
}

void* 
UI_Utils::display_CaptureItem( int x, int y ) 
{
	int i;

	for (i = 0; i < menuCount_; i++) 
	{
		// turn off focus each item
		// menu->items[i].window.flags &= ~WINDOW_HASFOCUS;
		if (rect_ContainsPoint(&menus_[i].window.rect, x, y)) 
			return &menus_[i];
	}
	return NULL;
}

UI_DisplayContext* 
UI_Utils::display_GetContext() 
{
	return dc_;
}

int 
UI_Utils::display_CursorType(int x, int y) 
{
	int i;
	for (i = 0; i < menuCount_; i++) 
	{
		rectDef_t r2;
		r2.x = menus_[i].window.rect.x - 3;
		r2.y = menus_[i].window.rect.y - 3;
		r2.w = r2.h = 7;
		if (rect_ContainsPoint(&r2, x, y))
			return CURSOR_SIZER;
	}
	return CURSOR_ARROW;
}

// FIXME: 
bool 
UI_Utils::display_MouseMove( void *p, int x, int y ) 
{
	int i;
	menuDef_t *menu = reinterpret_cast<menuDef_t*>(p);

	if (menu == NULL) 
	{
		menu = menu_GetFocused();
		if (menu) 
		{
			if (menu->window.flags & WINDOW_POPUP) 
			{
				menu_HandleMouseMove(menu, x, y);
				return true;
			}
		}
		for (i = 0; i < menuCount_; i++) 
			menu_HandleMouseMove(&menus_[i], x, y);
	} 
	else 
	{
		menu->window.rect.x += x;
		menu->window.rect.y += y;
		menu_UpdatePosition(menu);
	}
 	return true;
}

bool 
UI_Utils::display_KeyBindPending() const
{
	return waitingForKey_;
}

void 
UI_Utils::pc_SourceError(int handle, char *format, ...) 
{
	int line;
	char filename[128];
	va_list argptr;
	static char string[4096];

	va_start (argptr, format);
	vsprintf (string, format, argptr);
	va_end (argptr);

	filename[0] = '\0';
	line = 0;
	precomp_.sourceFileAndLine(handle, filename, &line);

	Com_Printf(S_COLOR_RED "ERROR: %s, line %d: %s\n", filename, line, string);
}

bool 
UI_Utils::pc_Float_Parse( int handle, float *f ) 
{
	UI_PrecompilerTools::PC_Token  token;
	int negative = false;

	if (!precomp_.readTokenHandle(handle, &token))
		return false;
	if (token.string_[0] == '-') 
	{
		if (!precomp_.readTokenHandle(handle, &token))
			return false;
		negative = true;
	}
	if (token.type_ != UI_PrecompilerTools::TT_NUMBER) 
	{
		pc_SourceError(handle, "expected float but found %s\n", token.string_);
		return false;
	}
	if (negative)
		*f = -token.floatvalue_;
	else
		*f = token.floatvalue_;
	return true;
}

bool 
UI_Utils::pc_Color_Parse( int handle, vec4_t *c ) 
{
	int i;
	float f;

	for (i = 0; i < 4; i++) 
	{
		if (!pc_Float_Parse(handle, &f)) 
		{
			return false;
		}
		(*c)[i] = f;
	}
	return true;
}

bool 
UI_Utils::pc_Int_Parse( int handle, int *i )
{
	UI_PrecompilerTools::PC_Token  token;
	int negative = false;

	if (!precomp_.readTokenHandle(handle, &token))
		return false;
	if (token.string_[0] == '-') 
	{
		if (!precomp_.readTokenHandle(handle, &token))
			return false;
		negative = true;
	}
	if (token.type_ != UI_PrecompilerTools::TT_NUMBER) 
	{
		pc_SourceError(handle, "expected integer but found %s\n", token.string_);
		return false;
	}
	*i = token.intvalue_;
	if (negative)
		*i = - *i;
	return true;
}

bool 
UI_Utils::pc_Rect_Parse( int handle, rectDef_t *r ) 
{
	if (pc_Float_Parse(handle, &r->x)) 
	{
		if (pc_Float_Parse(handle, &r->y)) 
		{
			if (pc_Float_Parse(handle, &r->w)) 
			{
				if (pc_Float_Parse(handle, &r->h)) 
					return true;
			}
		}
	}
	return false;
}

bool 
UI_Utils::pc_String_Parse(int handle, const char **out) 
{
	UI_PrecompilerTools::PC_Token  token;

	if (!precomp_.readTokenHandle(handle, &token))
		return false;
	
	*(out) = string_Alloc(token.string_);
    return true;
}

bool 
UI_Utils::pc_Script_Parse(int handle, const char **out) 
{
	char script[1024];
	UI_PrecompilerTools::PC_Token  token;

	memset(script, 0, sizeof(script));
	// scripts start with { and have ; separated command lists.. commands are command, arg.. 
	// basically we want everything between the { } as it will be interpreted at run time
  
	if (!precomp_.readTokenHandle(handle, &token))
		return false;
	if (Q_stricmp(token.string_, "{") != 0) {
	    return false;
	}

	while ( 1 ) {
		if (!precomp_.readTokenHandle(handle, &token))
			return false;

		if (Q_stricmp(token.string_, "}") == 0) {
			*out = string_Alloc(script);
			return true;
		}

		if (token.string_[1] != '\0') {
			Q_strcat(script, 1024, va("\"%s\"", token.string_));
		} else {
			Q_strcat(script, 1024, token.string_);
		}
		Q_strcat(script, 1024, " ");
	}
	return false; 	// bk001105 - LCC   missing return value
}

bool 
UI_Utils::float_Parse( char **p, float *f ) 
{
	char	*token;
	token = COM_ParseExt(p, false);
	if (token && token[0] != 0) 
	{
		*f = atof(token);
		return true;
	} 
	else
		return false;
}

bool 
UI_Utils::color_Parse( char **p, vec4_t *c ) 
{
	int i;
	float f;

	for (i = 0; i < 4; i++) 
	{
		if (!float_Parse(p, &f)) 
			return false;

		(*c)[i] = f;
	}
	return true;
}

bool 
UI_Utils::int_Parse( char **p, int *i ) 
{
	char	*token;
	token = COM_ParseExt(p, false);

	if (token && token[0] != 0) 
	{
		*i = atoi(token);
		return true;
	} 
	else
		return false;
}

bool 
UI_Utils::rect_Parse( char **p, rectDef_t *r ) 
{
	if (float_Parse(p, &r->x)) 
	{
		if (float_Parse(p, &r->y)) 
		{
			if (float_Parse(p, &r->w)) 
			{
				if (float_Parse(p, &r->h))
					return true;
			}
		}
	}
	return false;
}

bool 
UI_Utils::string_Parse(char **p, const char **out) 
{
	char *token;

	token = COM_ParseExt(p, false);
	if (token && token[0] != 0) 
	{
		*(out) = string_Alloc(token);
		return true;
	}
	return false;
}


void 
UI_Utils::fade(int *flags, float *f, float clamp, int *nextTime, int offsetTime, bool bFlags, float fadeAmount) 
{
	if (*flags & (WINDOW_FADINGOUT | WINDOW_FADINGIN)) 
	{
		if (dc_->realTime_ > *nextTime) 
		{
			*nextTime = dc_->realTime_ + offsetTime;
			if (*flags & WINDOW_FADINGOUT) 
			{
				*f -= fadeAmount;
				if (bFlags && *f <= 0.0) 
				{
					*flags &= ~(WINDOW_FADINGOUT | WINDOW_VISIBLE);
				}
			} 
			else 
			{
				*f += fadeAmount;
				if (*f >= clamp) 
				{
					*f = clamp;
					if (bFlags) 
					{
						*flags &= ~WINDOW_FADINGIN;
					}
				}
			}
		}
	}
}

bool 
UI_Utils::rect_ContainsPoint( rectDef_t *rect, float x, float y ) 
{
	if (rect) 
	{
		if (x > rect->x && x < rect->x + rect->w && y > rect->y && y < rect->y + rect->h)
			return true;
	}
	return false;
}

void 
UI_Utils::toWindowCoords( float *x, float *y, windowDef_t *window ) 
{
	if (window->border != 0) 
	{
		*x += window->borderSize;
		*y += window->borderSize;
	} 
	*x += window->rect.x;
	*y += window->rect.y;
}

void 
UI_Utils::rect_ToWindowCoords( rectDef_t *rect, windowDef_t *window ) 
{
	toWindowCoords(&rect->x, &rect->y, window);
}

void 
UI_Utils::adjustFrom640( float *x, float *y, float *w, float *h ) 
{
	//*x = *x * DC->scale + DC->bias;
	*x *= dc_->xScale_;
	*y *= dc_->yScale_;
	*w *= dc_->xScale_;
	*h *= dc_->yScale_;
}

void 
UI_Utils::lerpColor( const vec4_t a, const vec4_t b, vec4_t c, float t )
{
	int i;

	// lerp and clamp each component
	for (i=0; i<4; i++)
	{
		c[i] = a[i] + t*(b[i]-a[i]);
		if (c[i] < 0)
			c[i] = 0;
		else if (c[i] > 1.0)
			c[i] = 1.0;
	}
}

bool
UI_Utils::isVisible( int flags ) 
{
	return (flags & WINDOW_VISIBLE && !(flags & WINDOW_FADINGOUT));
}

vec4_t* 
UI_Utils::createColourVector( float r, float g, float b, float a, vec4_t* pVector )
{
	// assign
	colourVector_[0] = r;
	colourVector_[1] = g;
	colourVector_[2] = b;
	colourVector_[3] = a;

	// also copy into provided char array pointer?
	if( pVector )
	{
		// duplicate
		memcpy( pVector, colourVector_, sizeof( colourVector_ ) );
	}

	// return common ptr
	return &colourVector_;
}


unsigned char* 
UI_Utils::createColourChar( unsigned char r, unsigned char g, unsigned char b, unsigned char a, unsigned char* pArray )
{
	// assign
	colourCharArray_[0] = r;
	colourCharArray_[1] = g;
	colourCharArray_[2] = b;
	colourCharArray_[3] = a;

	// also copy into provided char array pointer?
	if( pArray )
	{
		// duplicate
		memcpy( pArray, colourCharArray_, sizeof( colourCharArray_ ) );
	}

	// return common ptr
	return &colourCharArray_[0];
}

void
UI_Utils::verticalGradient_Paint(rectDef_t *rect, vec4_t color) 
{
	// vertical gradient takes two paints
	dc_->setColor( color );
	dc_->drawHandlePic( rect->x, rect->y, rect->w, rect->h, dc_->assets_.verticalGradient );
	dc_->setColor( 0 );
}

void 
UI_Utils::drawStringNewAlpha( int x, int y, const char * pText, float alpha, textJustify_t formatting )
{
	// just use default extra parameters
	drawStringNew( x, y, 0.5f, *createColourVector(1,1,1,alpha,NULL), pText, 0, 0, 3, formatting );
}

void 
UI_Utils::drawStringNewColour( int x, int y, const char * pText, vec4_t colour, textJustify_t formatting )
{
	// just use default extra parameters
	drawStringNew( x, y, 0.5f, colour, pText, 0, 0, 3, formatting );
}

void 
UI_Utils::drawStringNewBasic( int x, int y, float scale, const char * pText )
{
	// fill in a few parameters for us
	drawStringNew( x, y, scale, *createColourVector(1,1,1,1,NULL), pText, 0, 0, ITEM_TEXTSTYLE_SHADOWED, LEFT_JUSTIFY );
}


void 
UI_Utils::drawStringNew( int x, int y, float scale, vec4_t colour, const char * pText, int hSpacing, int numChars, 
						 int style, textJustify_t formatting )
{
	int xPos = x;
	int yPos = y;
	int textWidth = 0;
	int textHeight = 0;

	// can't do anything without a device
	if( !dc_ )
		return;

	// get the width and height of this text
	textWidth = dc_->textWidth( pText, scale, 0 );
	textHeight = dc_->textHeight( pText, scale, 0 );

	// ->drawText() is draw from the "bottom up"
	yPos += textHeight;

	// reformat?
	switch( formatting )
	{
	case LEFT_JUSTIFY:
		break;

	case CENTRE_JUSTIFY:
		xPos -= (textWidth/2);
		break;

	case RIGHT_JUSTIFY:
		xPos -= textWidth;
		break;
	}

	// draw
	dc_->drawText( xPos, yPos, scale, colour, pText, hSpacing, numChars, style );
}

void*
UI_Utils::alloc( int size ) 
{
	char	*p; 

	if ( allocPoint_ + size > MEM_POOL_SIZE ) 
	{
		outOfMemory_ = true;
		Com_Printf("UI_Alloc: Failure. Out of memory!\n");
		//DC->trap_Print(S_COLOR_YELLOW"WARNING: UI Out of Memory!\n");
		return NULL;
	}

	p = &memoryPool_[allocPoint_];

	allocPoint_ += ( size + 15 ) & ~15;

	return p;
}

void 
UI_Utils::initMemory() 
{
	allocPoint_ = 0;
	outOfMemory_ = false;
}

int 
UI_Utils::outOfMemory() 
{
	return outOfMemory_;
}

void 
UI_Utils::string_Init() 
{
	int i;
	for( i = 0; i < HASH_TABLE_SIZE; i++ ) 
		strHandle_[i] = 0;

	strHandleCount_ = 0;
	strPoolIndex_ = 0;
	menuCount_ = 0;
	openMenuCount_ = 0;
	initMemory();
	//Item_SetupKeywordHash();
	//Menu_SetupKeywordHash();
	//Controls_GetConfig();
	controlUtils_->getConfig();
}

const char*
UI_Utils::string_Alloc( const char *p ) 
{
	int len;
	long hash;
	StringDef *str, *last;
	static const char *staticNULL = "";

	if (p == NULL) 
		return NULL;

	if (*p == 0) 
		return staticNULL;

	hash = hashForString(p);

	str = strHandle_[hash];
	while (str) 
	{
		if (strcmp(p, str->str) == 0) 
			return str->str;

		str = str->next;
	}

	len = strlen(p);
	if (len + strPoolIndex_ + 1 < STRING_POOL_SIZE) 
	{
		int ph = strPoolIndex_;
		strcpy(&strPool_[strPoolIndex_], p);
		strPoolIndex_ += len + 1;

		str = strHandle_[hash];
		last = str;
		while (str && str->next) 
		{
			last = str;
			str = str->next;
		}

		str  = reinterpret_cast<StringDef*>(alloc(sizeof(StringDef)));
		str->next = NULL;
		str->str = &strPool_[ph];
		if (last) 
			last->next = str;
		else 
			strHandle_[hash] = str;
		
		return &strPool_[ph];
	}
	return NULL;
}

void 
UI_Utils::string_Report() 
{
	float f;
	Com_Printf("Memory/String Pool Info\n");
	Com_Printf("----------------\n");
	f = strPoolIndex_;
	f /= STRING_POOL_SIZE;
	f *= 100;
	Com_Printf("String Pool is %.1f%% full, %i bytes out of %i used.\n", f, strPoolIndex_, STRING_POOL_SIZE);
	f = allocPoint_;
	f /= MEM_POOL_SIZE;
	f *= 100;
	Com_Printf("Memory Pool is %.1f%% full, %i bytes out of %i used.\n", f, allocPoint_, MEM_POOL_SIZE);
}

long 
UI_Utils::hashForString( const char *str ) 
{
	int		i;
	long	hash;
	char	letter;

	hash = 0;
	i = 0;
	while (str[i] != '\0') {
		letter = tolower(str[i]);
		hash+=(long)(letter)*(i+119);
		i++;
	}
	hash &= (HASH_TABLE_SIZE-1);
	return hash;
}


