#include "ui_hashutils.h"
#include "ui_keywordhash.h"



UI_HashUtils::UI_HashUtils()
{
	addItemKeyword( new ItemKeyword_Name );
	addItemKeyword( new ItemKeyword_Text );
	addItemKeyword( new ItemKeyword_Group );
	addItemKeyword( new ItemKeyword_Asset_Model );
	addItemKeyword( new ItemKeyword_Asset_Shader );
	addItemKeyword( new ItemKeyword_Model_Origin );
	addItemKeyword( new ItemKeyword_Model_fovX );
	addItemKeyword( new ItemKeyword_Model_fovY );
	addItemKeyword( new ItemKeyword_Model_Rotation );
	addItemKeyword( new ItemKeyword_Model_Angle );
	addItemKeyword( new ItemKeyword_Rect );
	addItemKeyword( new ItemKeyword_Style );
	addItemKeyword( new ItemKeyword_Decoration );
	addItemKeyword( new ItemKeyword_NotSelectable );
	addItemKeyword( new ItemKeyword_Wrapped );
	addItemKeyword( new ItemKeyword_AutoWrapped );
	addItemKeyword( new ItemKeyword_HorizontalScroll );
	addItemKeyword( new ItemKeyword_Type );
	addItemKeyword( new ItemKeyword_ElementWidth );
	addItemKeyword( new ItemKeyword_ElementHeight );
	addItemKeyword( new ItemKeyword_Feeder );
	addItemKeyword( new ItemKeyword_ElementType );
	addItemKeyword( new ItemKeyword_Columns );
	addItemKeyword( new ItemKeyword_Border );
	addItemKeyword( new ItemKeyword_BorderSize );
	addItemKeyword( new ItemKeyword_Visible );
	addItemKeyword( new ItemKeyword_Ownerdraw );
	addItemKeyword( new ItemKeyword_Align );
	addItemKeyword( new ItemKeyword_TextAlign );
	addItemKeyword( new ItemKeyword_TextAlignX );
	addItemKeyword( new ItemKeyword_TextAlignY );
	addItemKeyword( new ItemKeyword_TextScale );
	addItemKeyword( new ItemKeyword_TextStyle );
	addItemKeyword( new ItemKeyword_BackColor );
	addItemKeyword( new ItemKeyword_ForeColor );
	addItemKeyword( new ItemKeyword_BorderColor );
	addItemKeyword( new ItemKeyword_OutlineColor );
	addItemKeyword( new ItemKeyword_Background );
	addItemKeyword( new ItemKeyword_OnFocus );
	addItemKeyword( new ItemKeyword_LeaveFocus );
	addItemKeyword( new ItemKeyword_MouseEnter );
	addItemKeyword( new ItemKeyword_MouseExit );
	addItemKeyword( new ItemKeyword_MouseEnterText );
	addItemKeyword( new ItemKeyword_MouseExitText );
	addItemKeyword( new ItemKeyword_Action );
	addItemKeyword( new ItemKeyword_Special );
	addItemKeyword( new ItemKeyword_Cvar );
	addItemKeyword( new ItemKeyword_MaxChars );
	addItemKeyword( new ItemKeyword_MaxPaintChars );
	addItemKeyword( new ItemKeyword_FocusSound );
	addItemKeyword( new ItemKeyword_CvarFloat );
	addItemKeyword( new ItemKeyword_CvarStrList );
	addItemKeyword( new ItemKeyword_CvarFloatList );
	addItemKeyword( new ItemKeyword_AddColorRange );
	addItemKeyword( new ItemKeyword_OwnerdrawFlag );
	addItemKeyword( new ItemKeyword_EnableCvar );
	addItemKeyword( new ItemKeyword_CvarTest );
	addItemKeyword( new ItemKeyword_DisableCvar );
	addItemKeyword( new ItemKeyword_ShowCvar );
	addItemKeyword( new ItemKeyword_HideCvar );
	addItemKeyword( new ItemKeyword_Cinematic );
	addItemKeyword( new ItemKeyword_DoubleClick );

	// -----------------

	addMenuKeyword( new MenuKeyword_Font );
	addMenuKeyword( new MenuKeyword_Name );
	addMenuKeyword( new MenuKeyword_Fullscreen );
	addMenuKeyword( new MenuKeyword_Rect );
	addMenuKeyword( new MenuKeyword_Style );
	addMenuKeyword( new MenuKeyword_Visible );
	addMenuKeyword( new MenuKeyword_OnOpen );
	addMenuKeyword( new MenuKeyword_OnClose );
	addMenuKeyword( new MenuKeyword_OnESC );
	addMenuKeyword( new MenuKeyword_Border );
	addMenuKeyword( new MenuKeyword_BorderSize );
	addMenuKeyword( new MenuKeyword_BackColor );
	addMenuKeyword( new MenuKeyword_ForeColor );
	addMenuKeyword( new MenuKeyword_BorderColor );
	addMenuKeyword( new MenuKeyword_FocusColor );
	addMenuKeyword( new MenuKeyword_DisableColor );
	addMenuKeyword( new MenuKeyword_OutlineColor );
	addMenuKeyword( new MenuKeyword_Background );
	addMenuKeyword( new MenuKeyword_Ownerdraw );
	addMenuKeyword( new MenuKeyword_OwnerdrawFlag );
	addMenuKeyword( new MenuKeyword_OutOfBoundsClick );
	addMenuKeyword( new MenuKeyword_SoundLoop );
	addMenuKeyword( new MenuKeyword_ItemDef );
	addMenuKeyword( new MenuKeyword_Cinematic );
	addMenuKeyword( new MenuKeyword_Popup );
	addMenuKeyword( new MenuKeyword_FadeClamp );
	addMenuKeyword( new MenuKeyword_FadeCycle );
	addMenuKeyword( new MenuKeyword_FadeAmount );
}

UI_HashUtils::~UI_HashUtils()
{
	for( KeywordMapIter it = itemKeywords_.begin(); it != itemKeywords_.end(); ++it )
	{
		delete ((*it).second);
		(*it).second = 0;
	}
	itemKeywords_.clear();

	for( KeywordMapIter it = menuKeywords_.begin(); it != menuKeywords_.end(); ++it )
	{
		delete ((*it).second);
		(*it).second = 0;
	}
	menuKeywords_.clear();
}

UI_Utils* UI_HashUtils::utils_ = 0;

void
UI_HashUtils::setUtils( UI_Utils* utils )
{
	utils_ = utils;
	KeywordHash::setUtils( utils );
}

KeywordHash*
UI_HashUtils::findKeyword( KeywordMap& map, const char* keyword )
{
	int key = getKey( keyword );
	KeywordMapIter it = map.find( key );
	if( it == map.end() )
		return 0;
	return it->second;
}

KeywordHash*
UI_HashUtils::findItemKeyword( const char* keyword )
{
	return findKeyword( itemKeywords_, keyword );
}

KeywordHash*
UI_HashUtils::findMenuKeyword( const char* keyword )
{
	return findKeyword( menuKeywords_, keyword );
}

int 
UI_HashUtils::getKey( const char* keyword ) 
{
	int register hash, i;

	hash = 0;
	for (i = 0; keyword[i] != '\0'; i++) 
	{
		if (keyword[i] >= 'A' && keyword[i] <= 'Z')
			hash += (keyword[i] + ('a' - 'A')) * (119 + i);
		else
			hash += keyword[i] * (119 + i);
	}
	hash = (hash ^ (hash >> 10) ^ (hash >> 20)) & (k_KEYWORDHASH_SIZE-1);
	return hash;
}

bool
UI_HashUtils::addItemKeyword( KeywordHash* kh )
{
	return addKeyword( itemKeywords_, kh );
}

bool
UI_HashUtils::addMenuKeyword( KeywordHash* kh )
{
	return addKeyword( menuKeywords_, kh );
}

bool
UI_HashUtils::addKeyword( KeywordMap& map, KeywordHash* kh )
{
	return map.insert( std::make_pair( getKey( kh->getKeyword().c_str() ), kh ) ).second;
}






