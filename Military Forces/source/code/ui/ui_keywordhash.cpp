#include "ui_keywordhash.h"
#include "ui_utils.h"
#include "ui_precomp.h"



UI_Utils* KeywordHash::utils_ = 0;

KeywordHash::KeywordHash( std::string const& keyword ) : 
	keyword_(keyword) 
{
}

void
KeywordHash::setUtils( UI_Utils* utils )
{
	utils_ = utils;
}



ItemKeyword_Name::ItemKeyword_Name() : KeywordHash( "name" )
{}

bool
ItemKeyword_Name::func( itemDef_t *item, int handle )
{
	if (!utils_->pc_String_Parse(handle, &item->window.name)) 
		return false;
	return true;
}

ItemKeyword_Text::ItemKeyword_Text() : KeywordHash( "text" )
{}

bool
ItemKeyword_Text::func( itemDef_t *item, int handle )
{
	if (!utils_->pc_String_Parse(handle, &item->text)) 
		return false;
	return true;
}

ItemKeyword_Group::ItemKeyword_Group() : KeywordHash( "group" )
{}

bool
ItemKeyword_Group::func( itemDef_t *item, int handle )
{
	if (!utils_->pc_String_Parse(handle, &item->window.group)) 
		return false;
	return true;
}

ItemKeyword_Asset_Model::ItemKeyword_Asset_Model() : KeywordHash( "asset_model" )
{}

bool
ItemKeyword_Asset_Model::func( itemDef_t *item, int handle )
{
	const char *temp;
	modelDef_t *modelPtr;
	utils_->item_ValidateTypeData(item);
	modelPtr = (modelDef_t*)item->typeData;

	if (!utils_->pc_String_Parse(handle, &temp)) 
		return false;

	item->asset = utils_->dc_->registerModel(temp);
	modelPtr->angle = rand() % 360;
	return true;
}

ItemKeyword_Asset_Shader::ItemKeyword_Asset_Shader() : KeywordHash( "asset_shader" )
{}

bool
ItemKeyword_Asset_Shader::func( itemDef_t *item, int handle )
{
	const char *temp;

	if (!utils_->pc_String_Parse(handle, &temp)) 
		return false;

	item->asset = utils_->dc_->registerShaderNoMip(temp);
	return true;
}

ItemKeyword_Model_Origin::ItemKeyword_Model_Origin() : KeywordHash( "model_origin" )
{}

bool
ItemKeyword_Model_Origin::func( itemDef_t *item, int handle )
{
	modelDef_t *modelPtr;
	utils_->item_ValidateTypeData(item);
	modelPtr = (modelDef_t*)item->typeData;

	if (utils_->pc_Float_Parse(handle, &modelPtr->origin[0])) 
	{
		if (utils_->pc_Float_Parse(handle, &modelPtr->origin[1])) 
		{
			if (utils_->pc_Float_Parse(handle, &modelPtr->origin[2])) 
				return true;
		}
	}
	return false;
}

ItemKeyword_Model_fovX::ItemKeyword_Model_fovX() : KeywordHash( "model_fovx" )
{}

bool
ItemKeyword_Model_fovX::func( itemDef_t *item, int handle )
{
	modelDef_t *modelPtr;
	utils_->item_ValidateTypeData(item);
	modelPtr = (modelDef_t*)item->typeData;

	if (!utils_->pc_Float_Parse(handle, &modelPtr->fov_x)) 
		return false;
	return true;
}

ItemKeyword_Model_fovY::ItemKeyword_Model_fovY() : KeywordHash( "model_fovy" )
{}

bool
ItemKeyword_Model_fovY::func( itemDef_t *item, int handle )
{
	modelDef_t *modelPtr;
	utils_->item_ValidateTypeData(item);
	modelPtr = (modelDef_t*)item->typeData;

	if (!utils_->pc_Float_Parse(handle, &modelPtr->fov_y)) 
		return false;
	return true;
}

ItemKeyword_Model_Rotation::ItemKeyword_Model_Rotation() : KeywordHash( "model_rotation" )
{}

bool
ItemKeyword_Model_Rotation::func( itemDef_t *item, int handle )
{
	modelDef_t *modelPtr;
	utils_->item_ValidateTypeData(item);
	modelPtr = (modelDef_t*)item->typeData;

	if (!utils_->pc_Int_Parse(handle, &modelPtr->rotationSpeed)) 
		return false;
	return true;
}

ItemKeyword_Model_Angle::ItemKeyword_Model_Angle() : KeywordHash( "model_angle" )
{}

bool
ItemKeyword_Model_Angle::func( itemDef_t *item, int handle )
{
	modelDef_t *modelPtr;
	utils_->item_ValidateTypeData(item);
	modelPtr = (modelDef_t*)item->typeData;

	if (!utils_->pc_Int_Parse(handle, &modelPtr->angle)) 
		return false;
	return true;
}

ItemKeyword_Rect::ItemKeyword_Rect() : KeywordHash( "rect" )
{}

bool
ItemKeyword_Rect::func( itemDef_t *item, int handle )
{
	if (!utils_->pc_Rect_Parse(handle, &item->window.rectClient)) 
	{
		return false;
	}
	return true;
}

ItemKeyword_Style::ItemKeyword_Style() : KeywordHash( "style" )
{}

bool
ItemKeyword_Style::func( itemDef_t *item, int handle )
{
	if (!utils_->pc_Int_Parse(handle, &item->window.style)) 
	{
		return false;
	}
	return true;
}

ItemKeyword_Decoration::ItemKeyword_Decoration() : KeywordHash( "decoration" )
{}

bool
ItemKeyword_Decoration::func( itemDef_t *item, int handle )
{
	item->window.flags |= WINDOW_DECORATION;
	return true;
}

ItemKeyword_NotSelectable::ItemKeyword_NotSelectable() : KeywordHash( "notselectable" )
{}

bool
ItemKeyword_NotSelectable::func( itemDef_t *item, int handle )
{
	listBoxDef_t *listPtr;
	utils_->item_ValidateTypeData(item);
	listPtr = (listBoxDef_t*)item->typeData;
	if (item->type == ITEM_TYPE_LISTBOX && listPtr) 
		listPtr->notselectable = true;
	return true;
}

ItemKeyword_Wrapped::ItemKeyword_Wrapped() : KeywordHash( "wrapped" )
{}

bool
ItemKeyword_Wrapped::func( itemDef_t *item, int handle )
{
	item->window.flags |= WINDOW_WRAPPED;
	return true;
}

ItemKeyword_AutoWrapped::ItemKeyword_AutoWrapped() : KeywordHash( "autowrapped" )
{}

bool
ItemKeyword_AutoWrapped::func( itemDef_t *item, int handle )
{
	item->window.flags |= WINDOW_AUTOWRAPPED;
	return true;
}

ItemKeyword_HorizontalScroll::ItemKeyword_HorizontalScroll() : KeywordHash( "horizontalscroll" )
{}

bool
ItemKeyword_HorizontalScroll::func( itemDef_t *item, int handle )
{
	item->window.flags |= WINDOW_HORIZONTAL;
	return true;
}

ItemKeyword_Type::ItemKeyword_Type() : KeywordHash( "type" )
{}

bool
ItemKeyword_Type::func( itemDef_t *item, int handle )
{
	if (!utils_->pc_Int_Parse(handle, &item->type)) 
		return false;
	utils_->item_ValidateTypeData(item);
	return true;
}

ItemKeyword_ElementWidth::ItemKeyword_ElementWidth() : KeywordHash( "elementwidth" )
{}

bool
ItemKeyword_ElementWidth::func( itemDef_t *item, int handle )
{
	listBoxDef_t *listPtr;

	utils_->item_ValidateTypeData(item);
	listPtr = (listBoxDef_t*)item->typeData;
	if (!utils_->pc_Float_Parse(handle, &listPtr->elementWidth))
		return false;
	return true;
}

ItemKeyword_ElementHeight::ItemKeyword_ElementHeight() : KeywordHash( "elementheight" )
{}

bool
ItemKeyword_ElementHeight::func( itemDef_t *item, int handle )
{
	listBoxDef_t *listPtr;

	utils_->item_ValidateTypeData(item);
	listPtr = (listBoxDef_t*)item->typeData;
	if (!utils_->pc_Float_Parse(handle, &listPtr->elementHeight)) 
		return false;
	return true;
}

ItemKeyword_Feeder::ItemKeyword_Feeder() : KeywordHash( "feeder" )
{}

bool
ItemKeyword_Feeder::func( itemDef_t *item, int handle )
{
	if (!utils_->pc_Float_Parse(handle, &item->special)) 
		return false;
	return true;
}

ItemKeyword_ElementType::ItemKeyword_ElementType() : KeywordHash( "elementtype" )
{}

bool
ItemKeyword_ElementType::func( itemDef_t *item, int handle )
{
	listBoxDef_t *listPtr;

	utils_->item_ValidateTypeData(item);
	if (!item->typeData)
		return false;

	listPtr = (listBoxDef_t*)item->typeData;
	if (!utils_->pc_Int_Parse(handle, &listPtr->elementStyle)) 
		return false;

	return true;
}

ItemKeyword_Columns::ItemKeyword_Columns() : KeywordHash( "columns" )
{}

bool
ItemKeyword_Columns::func( itemDef_t *item, int handle )
{
	int num, i;
	listBoxDef_t *listPtr;

	utils_->item_ValidateTypeData(item);
	if (!item->typeData)
		return false;
	listPtr = (listBoxDef_t*)item->typeData;
	if (utils_->pc_Int_Parse(handle, &num)) 
	{
		if (num > MAX_LB_COLUMNS) 
			num = MAX_LB_COLUMNS;
		listPtr->numColumns = num;
		for (i = 0; i < num; i++) 
		{
			int pos, width, maxChars;

			if (utils_->pc_Int_Parse(handle, &pos) && 
				utils_->pc_Int_Parse(handle, &width) && 
				utils_->pc_Int_Parse(handle, &maxChars)) 
			{
				listPtr->columnInfo[i].pos = pos;
				listPtr->columnInfo[i].width = width;
				listPtr->columnInfo[i].maxChars = maxChars;
			} 
			else 
				return false;
		}
	}
	else 
		return false;
	return true;
}

ItemKeyword_Border::ItemKeyword_Border() : KeywordHash( "border" )
{}

bool
ItemKeyword_Border::func( itemDef_t *item, int handle )
{
	if (!utils_->pc_Int_Parse(handle, &item->window.border)) 
		return false;
	return true;
}

ItemKeyword_BorderSize::ItemKeyword_BorderSize() : KeywordHash( "bordersize" )
{}

bool
ItemKeyword_BorderSize::func( itemDef_t *item, int handle )
{
	if (!utils_->pc_Float_Parse(handle, &item->window.borderSize)) 
		return false;
	return true;
}

ItemKeyword_Visible::ItemKeyword_Visible() : KeywordHash( "visible" )
{}

bool
ItemKeyword_Visible::func( itemDef_t *item, int handle )
{
	int i;

	if (!utils_->pc_Int_Parse(handle, &i)) 
		return false;
	if (i) 
		item->window.flags |= WINDOW_VISIBLE;
	return true;
}

ItemKeyword_Ownerdraw::ItemKeyword_Ownerdraw() : KeywordHash( "ownerdraw" )
{}

bool
ItemKeyword_Ownerdraw::func( itemDef_t *item, int handle )
{
	if (!utils_->pc_Int_Parse(handle, &item->window.ownerDraw)) 
		return false;
	item->type = ITEM_TYPE_OWNERDRAW;
	return true;
}

ItemKeyword_Align::ItemKeyword_Align() : KeywordHash( "align" )
{}

bool
ItemKeyword_Align::func( itemDef_t *item, int handle )
{
	if (!utils_->pc_Int_Parse(handle, &item->alignment))
		return false;
	return true;
}

ItemKeyword_TextAlign::ItemKeyword_TextAlign() : KeywordHash( "textalign" )
{}

bool
ItemKeyword_TextAlign::func( itemDef_t *item, int handle )
{
	if (!utils_->pc_Int_Parse(handle, &item->textalignment)) 
		return false;
	return true;
}

ItemKeyword_TextAlignX::ItemKeyword_TextAlignX() : KeywordHash( "textalignx" )
{}

bool
ItemKeyword_TextAlignX::func( itemDef_t *item, int handle )
{
	if (!utils_->pc_Float_Parse(handle, &item->textalignx)) 
		return false;
	return true;
}

ItemKeyword_TextAlignY::ItemKeyword_TextAlignY() : KeywordHash( "textaligny" )
{}

bool
ItemKeyword_TextAlignY::func( itemDef_t *item, int handle )
{
	if (!utils_->pc_Float_Parse(handle, &item->textaligny)) 
		return false;
	return true;
}

ItemKeyword_TextScale::ItemKeyword_TextScale() : KeywordHash( "textscale" )
{}

bool
ItemKeyword_TextScale::func( itemDef_t *item, int handle )
{
	if (!utils_->pc_Float_Parse(handle, &item->textscale)) 
		return false;
	return true;
}

ItemKeyword_TextStyle::ItemKeyword_TextStyle() : KeywordHash( "textstyle" )
{}

bool
ItemKeyword_TextStyle::func( itemDef_t *item, int handle )
{
	if (!utils_->pc_Int_Parse(handle, &item->textStyle))
	{
		return false;
	}
	return true;
}

ItemKeyword_BackColor::ItemKeyword_BackColor() : KeywordHash( "backcolor" )
{}

bool
ItemKeyword_BackColor::func( itemDef_t *item, int handle )
{
	int i;
	float f;

	for (i = 0; i < 4; i++) 
	{
		if (!utils_->pc_Float_Parse(handle, &f)) 
			return false;
		item->window.backColor[i]  = f;
	}
	return true;
}

ItemKeyword_ForeColor::ItemKeyword_ForeColor() : KeywordHash( "forecolor" )
{}

bool
ItemKeyword_ForeColor::func( itemDef_t *item, int handle )
{
	int i;
	float f;

	for (i = 0; i < 4; i++) {
		if (!utils_->pc_Float_Parse(handle, &f)) 
			return false;
		item->window.foreColor[i]  = f;
		item->window.flags |= WINDOW_FORECOLORSET;
	}
	return true;
}

ItemKeyword_BorderColor::ItemKeyword_BorderColor() : KeywordHash( "bordercolor" )
{}

bool
ItemKeyword_BorderColor::func( itemDef_t *item, int handle )
{
	int i;
	float f;

	for (i = 0; i < 4; i++) {
		if (!utils_->pc_Float_Parse(handle, &f)) 
			return false;
		item->window.borderColor[i]  = f;
	}
	return true;
}

ItemKeyword_OutlineColor::ItemKeyword_OutlineColor() : KeywordHash( "outlinecolor" )
{}

bool
ItemKeyword_OutlineColor::func( itemDef_t *item, int handle )
{
	if (!utils_->pc_Color_Parse(handle, &item->window.outlineColor))
		return false;

	return true;
}

ItemKeyword_Background::ItemKeyword_Background() : KeywordHash( "background" )
{}

bool
ItemKeyword_Background::func( itemDef_t *item, int handle )
{
	const char *temp;

	if (!utils_->pc_String_Parse(handle, &temp)) 
		return false;
	item->window.background = utils_->dc_->registerShaderNoMip(temp);
	return true;
}

ItemKeyword_OnFocus::ItemKeyword_OnFocus() : KeywordHash( "onFocus" )
{}

bool
ItemKeyword_OnFocus::func( itemDef_t *item, int handle )
{
	if (!utils_->pc_Script_Parse(handle, &item->onFocus)) 
		return false;

	return true;
}

ItemKeyword_LeaveFocus::ItemKeyword_LeaveFocus() : KeywordHash( "leaveFocus" )
{}

bool
ItemKeyword_LeaveFocus::func( itemDef_t *item, int handle )
{
	if (!utils_->pc_Script_Parse(handle, &item->leaveFocus)) 
		return false;

	return true;
}

ItemKeyword_MouseEnter::ItemKeyword_MouseEnter() : KeywordHash( "mouseEnter" )
{}

bool
ItemKeyword_MouseEnter::func( itemDef_t *item, int handle )
{
	if (!utils_->pc_Script_Parse(handle, &item->mouseEnter))
		return false;

	return true;
}

ItemKeyword_MouseExit::ItemKeyword_MouseExit() : KeywordHash( "mouseExit" )
{}

bool
ItemKeyword_MouseExit::func( itemDef_t *item, int handle )
{
	if (!utils_->pc_Script_Parse(handle, &item->mouseExit)) 
		return false;

	return true;
}

ItemKeyword_MouseEnterText::ItemKeyword_MouseEnterText() : KeywordHash( "mouseEnterText" )
{}

bool
ItemKeyword_MouseEnterText::func( itemDef_t *item, int handle )
{
	if (!utils_->pc_Script_Parse(handle, &item->mouseEnterText)) 
		return false;

	return true;
}

ItemKeyword_MouseExitText::ItemKeyword_MouseExitText() : KeywordHash( "mouseExitText" )
{}

bool
ItemKeyword_MouseExitText::func( itemDef_t *item, int handle )
{
	if (!utils_->pc_Script_Parse(handle, &item->mouseExitText)) 
		return false;

	return true;
}

ItemKeyword_Action::ItemKeyword_Action() : KeywordHash( "action" )
{}

bool
ItemKeyword_Action::func( itemDef_t *item, int handle )
{
	if (!utils_->pc_Script_Parse(handle, &item->action)) 
		return false;

	return true;
}

ItemKeyword_Special::ItemKeyword_Special() : KeywordHash( "special" )
{}

bool
ItemKeyword_Special::func( itemDef_t *item, int handle )
{
	if (!utils_->pc_Float_Parse(handle, &item->special)) 
		return false;

	return true;
}

ItemKeyword_Cvar::ItemKeyword_Cvar() : KeywordHash( "cvar" )
{}

bool
ItemKeyword_Cvar::func( itemDef_t *item, int handle )
{
	editFieldDef_t *editPtr;

	utils_->item_ValidateTypeData(item);
	if (!utils_->pc_String_Parse(handle, &item->cvar)) 
		return false;

	if (item->typeData) 
	{
		editPtr = (editFieldDef_t*)item->typeData;
		editPtr->minVal = -1;
		editPtr->maxVal = -1;
		editPtr->defVal = -1;
	}
	return true;
}

ItemKeyword_MaxChars::ItemKeyword_MaxChars() : KeywordHash( "maxChars" )
{}

bool
ItemKeyword_MaxChars::func( itemDef_t *item, int handle )
{
	editFieldDef_t *editPtr;
	int maxChars;

	utils_->item_ValidateTypeData(item);
	if (!item->typeData)
		return false;

	if (!utils_->pc_Int_Parse(handle, &maxChars)) {
		return false;
	}
	editPtr = (editFieldDef_t*)item->typeData;
	editPtr->maxChars = maxChars;
	return true;
}

ItemKeyword_MaxPaintChars::ItemKeyword_MaxPaintChars() : KeywordHash( "maxPaintChars" )
{}

bool
ItemKeyword_MaxPaintChars::func( itemDef_t *item, int handle )
{
	editFieldDef_t *editPtr;
	int maxChars;

	utils_->item_ValidateTypeData(item);
	if (!item->typeData)
		return false;

	if (!utils_->pc_Int_Parse(handle, &maxChars)) 
		return false;

	editPtr = (editFieldDef_t*)item->typeData;
	editPtr->maxPaintChars = maxChars;
	return true;
}

ItemKeyword_FocusSound::ItemKeyword_FocusSound() : KeywordHash( "focusSound" )
{}

bool
ItemKeyword_FocusSound::func( itemDef_t *item, int handle )
{
	const char *temp;
	if (!utils_->pc_String_Parse(handle, &temp)) 
		return false;

	item->focusSound = utils_->dc_->registerSound(temp, false);
	return true;
}

ItemKeyword_CvarFloat::ItemKeyword_CvarFloat() : KeywordHash( "cvarFloat" )
{}

bool
ItemKeyword_CvarFloat::func( itemDef_t *item, int handle )
{
	editFieldDef_t *editPtr;

	utils_->item_ValidateTypeData(item);
	if (!item->typeData)
		return false;
	editPtr = (editFieldDef_t*)item->typeData;
	if (utils_->pc_String_Parse(handle, &item->cvar) &&
		utils_->pc_Float_Parse(handle, &editPtr->defVal) &&
		utils_->pc_Float_Parse(handle, &editPtr->minVal) &&
		utils_->pc_Float_Parse(handle, &editPtr->maxVal)) 
	{
		return true;
	}
	return false;
}

ItemKeyword_CvarStrList::ItemKeyword_CvarStrList() : KeywordHash( "cvarStrList" )
{}

bool
ItemKeyword_CvarStrList::func( itemDef_t *item, int handle )
{
	UI_PrecompilerTools::PC_Token token;
	multiDef_t *multiPtr;
	int pass;
	
	utils_->item_ValidateTypeData(item);
	if (!item->typeData)
		return false;
	multiPtr = (multiDef_t*)item->typeData;
	multiPtr->count = 0;
	multiPtr->strDef = true;

	if (!utils_->getPrecompilerTools()->readTokenHandle(handle, &token))
		return false;
	if (*token.string_ != '{')
		return false;

	pass = 0;
	while ( 1 ) 
	{
		if (!utils_->getPrecompilerTools()->readTokenHandle(handle, &token)) 
		{
			utils_->pc_SourceError(handle, "end of file inside menu item\n");
			return false;
		}

		if (*token.string_ == '}')
			return true;

		if (*token.string_ == ',' || *token.string_ == ';') 
			continue;

		if (pass == 0) 
		{
			multiPtr->cvarList[multiPtr->count] = utils_->string_Alloc(token.string_);
			pass = 1;
		} 
		else 
		{
			multiPtr->cvarStr[multiPtr->count] = utils_->string_Alloc(token.string_);
			pass = 0;
			multiPtr->count++;
			if (multiPtr->count >= MAX_MULTI_CVARS)
				return false;
		}
	}
	return false; 	// bk001205 - LCC missing return value
}

ItemKeyword_CvarFloatList::ItemKeyword_CvarFloatList() : KeywordHash( "cvarFloatList" )
{}

bool
ItemKeyword_CvarFloatList::func( itemDef_t *item, int handle )
{
	UI_PrecompilerTools::PC_Token  token;
	multiDef_t *multiPtr;
	
	utils_->item_ValidateTypeData(item);
	if (!item->typeData)
		return false;
	multiPtr = (multiDef_t*)item->typeData;
	multiPtr->count = 0;
	multiPtr->strDef = false;

	if (!utils_->getPrecompilerTools()->readTokenHandle(handle, &token))
		return false;
	if (*token.string_ != '{')
		return false;

	while ( 1 ) 
	{
		if (!utils_->getPrecompilerTools()->readTokenHandle(handle, &token)) 
		{
			utils_->pc_SourceError(handle, "end of file inside menu item\n");
			return false;
		}

		if (*token.string_ == '}')
			return true;

		if (*token.string_ == ',' || *token.string_ == ';')
			continue;

		multiPtr->cvarList[multiPtr->count] = utils_->string_Alloc(token.string_);
		if (!utils_->pc_Float_Parse(handle, &multiPtr->cvarValue[multiPtr->count]))
			return false;

		multiPtr->count++;
		if (multiPtr->count >= MAX_MULTI_CVARS)
			return false;
	}
	return false; 	// bk001205 - LCC missing return value
}

ItemKeyword_AddColorRange::ItemKeyword_AddColorRange() : KeywordHash( "addColorRange" )
{}

bool
ItemKeyword_AddColorRange::func( itemDef_t *item, int handle )
{
	colorRangeDef_t color;

	if (utils_->pc_Float_Parse(handle, &color.low) &&
		utils_->pc_Float_Parse(handle, &color.high) &&
		utils_->pc_Color_Parse(handle, &color.color) ) 
	{
		if (item->numColors < MAX_COLOR_RANGES) 
		{
			memcpy(&item->colorRanges[item->numColors], &color, sizeof(color));
			item->numColors++;
		}
		return true;
	}
	return false;
}

ItemKeyword_OwnerdrawFlag::ItemKeyword_OwnerdrawFlag() : KeywordHash( "ownerdrawFlag" )
{}

bool
ItemKeyword_OwnerdrawFlag::func( itemDef_t *item, int handle )
{
	int i;
	if (!utils_->pc_Int_Parse(handle, &i)) 
		return false;

	item->window.ownerDrawFlags |= i;
	return true;
}

ItemKeyword_EnableCvar::ItemKeyword_EnableCvar() : KeywordHash( "enableCvar" )
{}

bool
ItemKeyword_EnableCvar::func( itemDef_t *item, int handle )
{
	if (utils_->pc_Script_Parse(handle, &item->enableCvar)) 
	{
		item->cvarFlags = CVAR_ENABLE;
		return true;
	}
	return false;
}

ItemKeyword_CvarTest::ItemKeyword_CvarTest() : KeywordHash( "cvarTest" )
{}

bool
ItemKeyword_CvarTest::func( itemDef_t *item, int handle )
{
	if (!utils_->pc_String_Parse(handle, &item->cvarTest))
		return false;

	return true;
}

ItemKeyword_DisableCvar::ItemKeyword_DisableCvar() : KeywordHash( "disableCvar" )
{}

bool
ItemKeyword_DisableCvar::func( itemDef_t *item, int handle )
{
	if (utils_->pc_Script_Parse(handle, &item->enableCvar)) 
	{
		item->cvarFlags = CVAR_DISABLE;
		return true;
	}
	return false;
}

ItemKeyword_ShowCvar::ItemKeyword_ShowCvar() : KeywordHash( "showCvar" )
{}

bool
ItemKeyword_ShowCvar::func( itemDef_t *item, int handle )
{
	if (utils_->pc_Script_Parse(handle, &item->enableCvar)) 
	{
		item->cvarFlags = CVAR_SHOW;
		return true;
	}
	return false;
}

ItemKeyword_HideCvar::ItemKeyword_HideCvar() : KeywordHash( "hideCvar" )
{}

bool
ItemKeyword_HideCvar::func( itemDef_t *item, int handle )
{
	if (utils_->pc_Script_Parse(handle, &item->enableCvar)) 
	{
		item->cvarFlags = CVAR_HIDE;
		return true;
	}
	return false;
}

ItemKeyword_Cinematic::ItemKeyword_Cinematic() : KeywordHash( "cinematic" )
{}

bool
ItemKeyword_Cinematic::func( itemDef_t *item, int handle )
{
	if (!utils_->pc_String_Parse(handle, &item->window.cinematicName)) 
	{
		return false;
	}
	return true;
}

ItemKeyword_DoubleClick::ItemKeyword_DoubleClick() : KeywordHash( "doubleclick" )
{}

bool
ItemKeyword_DoubleClick::func( itemDef_t *item, int handle )
{
	listBoxDef_t *listPtr;

	utils_->item_ValidateTypeData(item);
	if (!item->typeData) 
	{
		return false;
	}

	listPtr = (listBoxDef_t*)item->typeData;

	if (!utils_->pc_Script_Parse(handle, &listPtr->doubleClick)) 
	{
		return false;
	}
	return true;
}

// ----------------

MenuKeyword_Font::MenuKeyword_Font() : KeywordHash( "font" )
{}

bool
MenuKeyword_Font::func( itemDef_t *item, int handle )
{
	menuDef_t *menu = reinterpret_cast<menuDef_t*>(item);
	if (!utils_->pc_String_Parse(handle, &menu->font)) 
		return false;
	
	if (!utils_->dc_->assets_.fontRegistered) 
	{
		utils_->dc_->registerFont(menu->font, 48, &utils_->dc_->assets_.textFont);
		utils_->dc_->assets_.fontRegistered = true;
	}
	return true;
}

MenuKeyword_Name::MenuKeyword_Name() : KeywordHash( "name" )
{}

bool
MenuKeyword_Name::func( itemDef_t *item, int handle )
{
	menuDef_t *menu = reinterpret_cast<menuDef_t*>(item);
	if (!utils_->pc_String_Parse(handle, &menu->window.name)) 
		return false;
	
	return true;
}

MenuKeyword_Fullscreen::MenuKeyword_Fullscreen() : KeywordHash( "fullScreen" )
{}

bool
MenuKeyword_Fullscreen::func( itemDef_t *item, int handle )
{
	menuDef_t *menu = reinterpret_cast<menuDef_t*>(item);
	if (!utils_->pc_Int_Parse(handle, (int*) &menu->fullScreen))  // bk001206 - cast bool
		return false;
	
	return true;
}

MenuKeyword_Rect::MenuKeyword_Rect() : KeywordHash( "rect" )
{}

bool
MenuKeyword_Rect::func( itemDef_t *item, int handle )
{
	menuDef_t *menu = reinterpret_cast<menuDef_t*>(item);
	if (!utils_->pc_Rect_Parse(handle, &menu->window.rect)) 
		return false;
	
	return true;
}

MenuKeyword_Style::MenuKeyword_Style() : KeywordHash( "style" )
{}

bool
MenuKeyword_Style::func( itemDef_t *item, int handle )
{
	menuDef_t *menu = reinterpret_cast<menuDef_t*>(item);
	if (!utils_->pc_Int_Parse(handle, &menu->window.style)) 
		return false;
	
	return true;
}

MenuKeyword_Visible::MenuKeyword_Visible() : KeywordHash( "visible" )
{}

bool
MenuKeyword_Visible::func( itemDef_t *item, int handle )
{
	int i;
	menuDef_t *menu = reinterpret_cast<menuDef_t*>(item);

	if (!utils_->pc_Int_Parse(handle, &i)) 
		return false;
	
	if (i) 
		menu->window.flags |= WINDOW_VISIBLE;
	
	return true;
}

MenuKeyword_OnOpen::MenuKeyword_OnOpen() : KeywordHash( "onOpen" )
{}

bool
MenuKeyword_OnOpen::func( itemDef_t *item, int handle )
{
	menuDef_t *menu = reinterpret_cast<menuDef_t*>(item);
	if (!utils_->pc_Script_Parse(handle, &menu->onOpen)) 
		return false;
	
	return true;
}

MenuKeyword_OnClose::MenuKeyword_OnClose() : KeywordHash( "onClose" )
{}

bool
MenuKeyword_OnClose::func( itemDef_t *item, int handle )
{
	menuDef_t *menu = reinterpret_cast<menuDef_t*>(item);
	if (!utils_->pc_Script_Parse(handle, &menu->onClose)) 
		return false;
	
	return true;
}

MenuKeyword_OnESC::MenuKeyword_OnESC() : KeywordHash( "onESC" )
{}

bool
MenuKeyword_OnESC::func( itemDef_t *item, int handle )
{
	menuDef_t *menu = reinterpret_cast<menuDef_t*>(item);
	if (!utils_->pc_Script_Parse(handle, &menu->onESC)) 
		return false;
	
	return true;
}

MenuKeyword_Border::MenuKeyword_Border() : KeywordHash( "border" )
{}

bool
MenuKeyword_Border::func( itemDef_t *item, int handle )
{
	menuDef_t *menu = reinterpret_cast<menuDef_t*>(item);
	if (!utils_->pc_Int_Parse(handle, &menu->window.border)) 
		return false;
	
	return true;
}

MenuKeyword_BorderSize::MenuKeyword_BorderSize() : KeywordHash( "borderSize" )
{}

bool
MenuKeyword_BorderSize::func( itemDef_t *item, int handle )
{
	menuDef_t *menu = reinterpret_cast<menuDef_t*>(item);
	if (!utils_->pc_Float_Parse(handle, &menu->window.borderSize)) 
		return false;
	
	return true;
}

MenuKeyword_BackColor::MenuKeyword_BackColor() : KeywordHash( "backcolor" )
{}

bool
MenuKeyword_BackColor::func( itemDef_t *item, int handle )
{
	int i;
	float f;
	menuDef_t *menu = reinterpret_cast<menuDef_t*>(item);

	for (i = 0; i < 4; i++) 
	{
		if (!utils_->pc_Float_Parse(handle, &f)) 
			return false;
		
		menu->window.backColor[i]  = f;
	}
	return true;
}

MenuKeyword_ForeColor::MenuKeyword_ForeColor() : KeywordHash( "forecolor" )
{}

bool
MenuKeyword_ForeColor::func( itemDef_t *item, int handle )
{
	int i;
	float f;
	menuDef_t *menu = reinterpret_cast<menuDef_t*>(item);

	for (i = 0; i < 4; i++) 
	{
		if (!utils_->pc_Float_Parse(handle, &f)) 
			return false;
		
		menu->window.foreColor[i]  = f;
		menu->window.flags |= WINDOW_FORECOLORSET;
	}
	return true;
}

MenuKeyword_BorderColor::MenuKeyword_BorderColor() : KeywordHash( "bordercolor" )
{}

bool
MenuKeyword_BorderColor::func( itemDef_t *item, int handle )
{
	int i;
	float f;
	menuDef_t *menu = reinterpret_cast<menuDef_t*>(item);

	for (i = 0; i < 4; i++) 
	{
		if (!utils_->pc_Float_Parse(handle, &f)) 
			return false;
		
		menu->window.borderColor[i]  = f;
	}
	return true;
}

MenuKeyword_FocusColor::MenuKeyword_FocusColor() : KeywordHash( "focuscolor" )
{}

bool
MenuKeyword_FocusColor::func( itemDef_t *item, int handle )
{
	int i;
	float f;
	menuDef_t *menu = reinterpret_cast<menuDef_t*>(item);

	for (i = 0; i < 4; i++) {
		if (!utils_->pc_Float_Parse(handle, &f)) 
			return false;
		
		menu->focusColor[i]  = f;
	}
	return true;
}

MenuKeyword_DisableColor::MenuKeyword_DisableColor() : KeywordHash( "disablecolor" )
{}

bool
MenuKeyword_DisableColor::func( itemDef_t *item, int handle )
{
	int i;
	float f;
	menuDef_t *menu = reinterpret_cast<menuDef_t*>(item);
	for (i = 0; i < 4; i++) 
	{
		if (!utils_->pc_Float_Parse(handle, &f)) 
			return false;
		menu->disableColor[i]  = f;
	}
	return true;
}

MenuKeyword_OutlineColor::MenuKeyword_OutlineColor() : KeywordHash( "outlinecolor" )
{}

bool
MenuKeyword_OutlineColor::func( itemDef_t *item, int handle )
{
	menuDef_t *menu = reinterpret_cast<menuDef_t*>(item);
	if (!utils_->pc_Color_Parse(handle, &menu->window.outlineColor))
		return false;
	
	return true;
}

MenuKeyword_Background::MenuKeyword_Background() : KeywordHash( "background" )
{}

bool
MenuKeyword_Background::func( itemDef_t *item, int handle )
{
	const char *buff;
	menuDef_t *menu = reinterpret_cast<menuDef_t*>(item);

	if (!utils_->pc_String_Parse(handle, &buff)) 
		return false;
	
	menu->window.background = utils_->dc_->registerShaderNoMip(buff);
	return true;
}

MenuKeyword_Ownerdraw::MenuKeyword_Ownerdraw() : KeywordHash( "ownerdraw" )
{}

bool
MenuKeyword_Ownerdraw::func( itemDef_t *item, int handle )
{
	menuDef_t *menu = reinterpret_cast<menuDef_t*>(item);

	if (!utils_->pc_Int_Parse(handle, &menu->window.ownerDraw)) 
		return false;
	
	return true;
}

MenuKeyword_OwnerdrawFlag::MenuKeyword_OwnerdrawFlag() : KeywordHash( "ownerdrawFlag" )
{}

bool
MenuKeyword_OwnerdrawFlag::func( itemDef_t *item, int handle )
{
	int i;
	menuDef_t *menu = reinterpret_cast<menuDef_t*>(item);

	if (!utils_->pc_Int_Parse(handle, &i)) 
		return false;
	
	menu->window.ownerDrawFlags |= i;
	return true;
}

MenuKeyword_OutOfBoundsClick::MenuKeyword_OutOfBoundsClick() : KeywordHash( "outOfBoundsClick" )
{}

bool
MenuKeyword_OutOfBoundsClick::func( itemDef_t *item, int handle )
{
	menuDef_t *menu = reinterpret_cast<menuDef_t*>(item);

	menu->window.flags |= WINDOW_OOB_CLICK;
	return true;
}

MenuKeyword_SoundLoop::MenuKeyword_SoundLoop() : KeywordHash( "soundLoop" )
{}

bool
MenuKeyword_SoundLoop::func( itemDef_t *item, int handle )
{
	menuDef_t *menu = reinterpret_cast<menuDef_t*>(item);

	if (!utils_->pc_String_Parse(handle, &menu->soundName)) 
		return false;
	
	return true;
}

MenuKeyword_ItemDef::MenuKeyword_ItemDef() : KeywordHash( "itemDef" )
{}

bool
MenuKeyword_ItemDef::func( itemDef_t *item, int handle )
{
	menuDef_t *menu = reinterpret_cast<menuDef_t*>(item);

	if (menu->itemCount < MAX_MENUITEMS) 
	{
		menu->items[menu->itemCount] = reinterpret_cast<itemDef_t*>(utils_->alloc(sizeof(itemDef_t)));
		utils_->item_Init(menu->items[menu->itemCount]);
		if (!utils_->item_Parse(handle, menu->items[menu->itemCount])) 
			return false;
		utils_->item_InitControls(menu->items[menu->itemCount]);
		menu->items[menu->itemCount++]->parent = menu;
	}
	return true;
}

MenuKeyword_Cinematic::MenuKeyword_Cinematic() : KeywordHash( "cinematic" )
{}

bool
MenuKeyword_Cinematic::func( itemDef_t *item, int handle )
{
	menuDef_t *menu = reinterpret_cast<menuDef_t*>(item);

	if (!utils_->pc_String_Parse(handle, &menu->window.cinematicName)) 
		return false;
	
	return true;
}

MenuKeyword_Popup::MenuKeyword_Popup() : KeywordHash( "popup" )
{}

bool
MenuKeyword_Popup::func( itemDef_t *item, int handle )
{
	menuDef_t *menu = reinterpret_cast<menuDef_t*>(item);
	menu->window.flags |= WINDOW_POPUP;
	return true;
}

MenuKeyword_FadeClamp::MenuKeyword_FadeClamp() : KeywordHash( "fadeClamp" )
{}

bool
MenuKeyword_FadeClamp::func( itemDef_t *item, int handle )
{
	menuDef_t *menu = reinterpret_cast<menuDef_t*>(item);

	if (!utils_->pc_Float_Parse(handle, &menu->fadeClamp)) 
		return false;
	
	return true;
}

MenuKeyword_FadeCycle::MenuKeyword_FadeCycle() : KeywordHash( "fadeCycle" )
{}

bool
MenuKeyword_FadeCycle::func( itemDef_t *item, int handle )
{
	menuDef_t *menu = reinterpret_cast<menuDef_t*>(item);

	if (!utils_->pc_Int_Parse(handle, &menu->fadeCycle)) 
		return false;
	
	return true;
}

MenuKeyword_FadeAmount::MenuKeyword_FadeAmount() : KeywordHash( "fadeAmount" )
{}

bool
MenuKeyword_FadeAmount::func( itemDef_t *item, int handle )
{
	menuDef_t *menu = reinterpret_cast<menuDef_t*>(item);

	if (!utils_->pc_Float_Parse(handle, &menu->fadeAmount)) 
		return false;
	
	return true;
}






















