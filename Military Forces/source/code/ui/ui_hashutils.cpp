#include "ui_hashutils.h"
#include "ui_keywordhash.h"



UI_HashUtils::UI_HashUtils()
{
	addItemKeyword( std::make_unique<ItemKeyword_Name>() );
	addItemKeyword( std::make_unique<ItemKeyword_Text>() );
	addItemKeyword( std::make_unique<ItemKeyword_Group>() );
	addItemKeyword( std::make_unique<ItemKeyword_Asset_Model>() );
	addItemKeyword( std::make_unique<ItemKeyword_Asset_Shader>() );
	addItemKeyword( std::make_unique<ItemKeyword_Model_Origin>() );
	addItemKeyword( std::make_unique<ItemKeyword_Model_fovX>() );
	addItemKeyword( std::make_unique<ItemKeyword_Model_fovY>() );
	addItemKeyword( std::make_unique<ItemKeyword_Model_Rotation>() );
	addItemKeyword( std::make_unique<ItemKeyword_Model_Angle>() );
	addItemKeyword( std::make_unique<ItemKeyword_Rect>() );
	addItemKeyword( std::make_unique<ItemKeyword_Style>() );
	addItemKeyword( std::make_unique<ItemKeyword_Decoration>() );
	addItemKeyword( std::make_unique<ItemKeyword_NotSelectable>() );
	addItemKeyword( std::make_unique<ItemKeyword_Wrapped>() );
	addItemKeyword( std::make_unique<ItemKeyword_AutoWrapped>() );
	addItemKeyword( std::make_unique<ItemKeyword_HorizontalScroll>() );
	addItemKeyword( std::make_unique<ItemKeyword_Type>() );
	addItemKeyword( std::make_unique<ItemKeyword_ElementWidth>() );
	addItemKeyword( std::make_unique<ItemKeyword_ElementHeight>() );
	addItemKeyword( std::make_unique<ItemKeyword_Feeder>() );
	addItemKeyword( std::make_unique<ItemKeyword_ElementType>() );
	addItemKeyword( std::make_unique<ItemKeyword_Columns>() );
	addItemKeyword( std::make_unique<ItemKeyword_Border>() );
	addItemKeyword( std::make_unique<ItemKeyword_BorderSize>() );
	addItemKeyword( std::make_unique<ItemKeyword_Visible>() );
	addItemKeyword( std::make_unique<ItemKeyword_Ownerdraw>() );
	addItemKeyword( std::make_unique<ItemKeyword_Align>() );
	addItemKeyword( std::make_unique<ItemKeyword_TextAlign>() );
	addItemKeyword( std::make_unique<ItemKeyword_TextAlignX>() );
	addItemKeyword( std::make_unique<ItemKeyword_TextAlignY>() );
	addItemKeyword( std::make_unique<ItemKeyword_TextScale>() );
	addItemKeyword( std::make_unique<ItemKeyword_TextStyle>() );
	addItemKeyword( std::make_unique<ItemKeyword_BackColor>() );
	addItemKeyword( std::make_unique<ItemKeyword_ForeColor>() );
	addItemKeyword( std::make_unique<ItemKeyword_BorderColor>() );
	addItemKeyword( std::make_unique<ItemKeyword_OutlineColor>() );
	addItemKeyword( std::make_unique<ItemKeyword_Background>() );
	addItemKeyword( std::make_unique<ItemKeyword_OnFocus>() );
	addItemKeyword( std::make_unique<ItemKeyword_LeaveFocus>() );
	addItemKeyword( std::make_unique<ItemKeyword_MouseEnter>() );
	addItemKeyword( std::make_unique<ItemKeyword_MouseExit>() );
	addItemKeyword( std::make_unique<ItemKeyword_MouseEnterText>() );
	addItemKeyword( std::make_unique<ItemKeyword_MouseExitText>() );
	addItemKeyword( std::make_unique<ItemKeyword_Action>() );
	addItemKeyword( std::make_unique<ItemKeyword_Special>() );
	addItemKeyword( std::make_unique<ItemKeyword_Cvar>() );
	addItemKeyword( std::make_unique<ItemKeyword_MaxChars>() );
	addItemKeyword( std::make_unique<ItemKeyword_MaxPaintChars>() );
	addItemKeyword( std::make_unique<ItemKeyword_FocusSound>() );
	addItemKeyword( std::make_unique<ItemKeyword_CvarFloat>() );
	addItemKeyword( std::make_unique<ItemKeyword_CvarStrList>() );
	addItemKeyword( std::make_unique<ItemKeyword_CvarFloatList>() );
	addItemKeyword( std::make_unique<ItemKeyword_AddColorRange>() );
	addItemKeyword( std::make_unique<ItemKeyword_OwnerdrawFlag>() );
	addItemKeyword( std::make_unique<ItemKeyword_EnableCvar>() );
	addItemKeyword( std::make_unique<ItemKeyword_CvarTest>() );
	addItemKeyword( std::make_unique<ItemKeyword_DisableCvar>() );
	addItemKeyword( std::make_unique<ItemKeyword_ShowCvar>() );
	addItemKeyword( std::make_unique<ItemKeyword_HideCvar>() );
	addItemKeyword( std::make_unique<ItemKeyword_Cinematic>() );
	addItemKeyword( std::make_unique<ItemKeyword_DoubleClick>() );

	// -----------------

	addMenuKeyword( std::make_unique<MenuKeyword_Font>() );
	addMenuKeyword( std::make_unique<MenuKeyword_Name>() );
	addMenuKeyword( std::make_unique<MenuKeyword_Fullscreen>() );
	addMenuKeyword( std::make_unique<MenuKeyword_Rect>() );
	addMenuKeyword( std::make_unique<MenuKeyword_Style>() );
	addMenuKeyword( std::make_unique<MenuKeyword_Visible>() );
	addMenuKeyword( std::make_unique<MenuKeyword_OnOpen>() );
	addMenuKeyword( std::make_unique<MenuKeyword_OnClose>() );
	addMenuKeyword( std::make_unique<MenuKeyword_OnESC>() );
	addMenuKeyword( std::make_unique<MenuKeyword_Border>() );
	addMenuKeyword( std::make_unique<MenuKeyword_BorderSize>() );
	addMenuKeyword( std::make_unique<MenuKeyword_BackColor>() );
	addMenuKeyword( std::make_unique<MenuKeyword_ForeColor>() );
	addMenuKeyword( std::make_unique<MenuKeyword_BorderColor>() );
	addMenuKeyword( std::make_unique<MenuKeyword_FocusColor>() );
	addMenuKeyword( std::make_unique<MenuKeyword_DisableColor>() );
	addMenuKeyword( std::make_unique<MenuKeyword_OutlineColor>() );
	addMenuKeyword( std::make_unique<MenuKeyword_Background>() );
	addMenuKeyword( std::make_unique<MenuKeyword_Ownerdraw>() );
	addMenuKeyword( std::make_unique<MenuKeyword_OwnerdrawFlag>() );
	addMenuKeyword( std::make_unique<MenuKeyword_OutOfBoundsClick>() );
	addMenuKeyword( std::make_unique<MenuKeyword_SoundLoop>() );
	addMenuKeyword( std::make_unique<MenuKeyword_ItemDef>() );
	addMenuKeyword( std::make_unique<MenuKeyword_Cinematic>() );
	addMenuKeyword( std::make_unique<MenuKeyword_Popup>() );
	addMenuKeyword( std::make_unique<MenuKeyword_FadeClamp>() );
	addMenuKeyword( std::make_unique<MenuKeyword_FadeCycle>() );
	addMenuKeyword( std::make_unique<MenuKeyword_FadeAmount>() );
}

UI_HashUtils::~UI_HashUtils()
{
}

UI_Utils* UI_HashUtils::utils_ = nullptr;

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
		return nullptr;
	return it->second.get();
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
	int hash, i;

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
UI_HashUtils::addItemKeyword( std::unique_ptr<KeywordHash> kh )
{
	return addKeyword( itemKeywords_, std::move( kh ) );
}

bool
UI_HashUtils::addMenuKeyword( std::unique_ptr<KeywordHash> kh )
{
	return addKeyword( menuKeywords_, std::move( kh ) );
}

bool
UI_HashUtils::addKeyword( KeywordMap& map, std::unique_ptr<KeywordHash> kh )
{
	// a keyword that collides with an earlier key is dropped
	const int key = getKey( kh->getKeyword().c_str() );
	return map.emplace( key, std::move( kh ) ).second;
}






