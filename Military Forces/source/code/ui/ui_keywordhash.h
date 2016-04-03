#ifndef __UI_KEYWORDHASH_H__
#define __UI_KEYWORDHASH_H__


struct UI_Utils;
struct itemDef_t;

#include <string>

struct KeywordHash
{
	virtual					~KeywordHash() {}

	virtual bool			func( itemDef_t *item, int handle ) = 0;

	static void				setUtils( UI_Utils* utils );

	std::string&			getKeyword() { return keyword_; }

protected:
							KeywordHash( std::string const& keyword );

	//data
	static UI_Utils*		utils_;

	std::string				keyword_;
};

struct ItemKeyword_Name : public KeywordHash
{
					ItemKeyword_Name();
	virtual bool	func( itemDef_t *item, int handle );
};

struct ItemKeyword_Text : public KeywordHash
{
					ItemKeyword_Text();
	virtual bool	func( itemDef_t *item, int handle );
};

struct ItemKeyword_Group : public KeywordHash
{
					ItemKeyword_Group();
	virtual bool	func( itemDef_t *item, int handle );
};

struct ItemKeyword_Asset_Model : public KeywordHash
{
					ItemKeyword_Asset_Model();
	virtual bool	func( itemDef_t *item, int handle );
};

struct ItemKeyword_Asset_Shader : public KeywordHash
{
					ItemKeyword_Asset_Shader();
	virtual bool	func( itemDef_t *item, int handle );
};

struct ItemKeyword_Model_Origin : public KeywordHash
{
					ItemKeyword_Model_Origin();
	virtual bool	func( itemDef_t *item, int handle );
};

struct ItemKeyword_Model_fovX : public KeywordHash
{
					ItemKeyword_Model_fovX();
	virtual bool	func( itemDef_t *item, int handle );
};

struct ItemKeyword_Model_fovY : public KeywordHash
{
					ItemKeyword_Model_fovY();
	virtual bool	func( itemDef_t *item, int handle );
};

struct ItemKeyword_Model_Rotation : public KeywordHash
{
					ItemKeyword_Model_Rotation();
	virtual bool	func( itemDef_t *item, int handle );
};

struct ItemKeyword_Model_Angle : public KeywordHash
{
					ItemKeyword_Model_Angle();
	virtual bool	func( itemDef_t *item, int handle );
};

struct ItemKeyword_Rect : public KeywordHash
{
					ItemKeyword_Rect();
	virtual bool	func( itemDef_t *item, int handle );
};

struct ItemKeyword_Style : public KeywordHash
{
					ItemKeyword_Style();
	virtual bool	func( itemDef_t *item, int handle );
};

struct ItemKeyword_Decoration : public KeywordHash
{
					ItemKeyword_Decoration();
	virtual bool	func( itemDef_t *item, int handle );
};

struct ItemKeyword_NotSelectable : public KeywordHash
{
					ItemKeyword_NotSelectable();
	virtual bool	func( itemDef_t *item, int handle );
};

struct ItemKeyword_Wrapped : public KeywordHash
{
					ItemKeyword_Wrapped();
	virtual bool	func( itemDef_t *item, int handle );
};

struct ItemKeyword_AutoWrapped : public KeywordHash
{
					ItemKeyword_AutoWrapped();
	virtual bool	func( itemDef_t *item, int handle );
};

struct ItemKeyword_HorizontalScroll : public KeywordHash
{
					ItemKeyword_HorizontalScroll();
	virtual bool	func( itemDef_t *item, int handle );
};

struct ItemKeyword_Type : public KeywordHash
{
					ItemKeyword_Type();
	virtual bool	func( itemDef_t *item, int handle );
};

struct ItemKeyword_ElementWidth : public KeywordHash
{
					ItemKeyword_ElementWidth();
	virtual bool	func( itemDef_t *item, int handle );
};

struct ItemKeyword_ElementHeight : public KeywordHash
{
					ItemKeyword_ElementHeight();
	virtual bool	func( itemDef_t *item, int handle );
};

struct ItemKeyword_Feeder : public KeywordHash
{
					ItemKeyword_Feeder();
	virtual bool	func( itemDef_t *item, int handle );
};

struct ItemKeyword_ElementType : public KeywordHash
{
					ItemKeyword_ElementType();
	virtual bool	func( itemDef_t *item, int handle );
};

struct ItemKeyword_Columns : public KeywordHash
{
					ItemKeyword_Columns();
	virtual bool	func( itemDef_t *item, int handle );
};

struct ItemKeyword_Border : public KeywordHash
{
					ItemKeyword_Border();
	virtual bool	func( itemDef_t *item, int handle );
};

struct ItemKeyword_BorderSize : public KeywordHash
{
					ItemKeyword_BorderSize();
	virtual bool	func( itemDef_t *item, int handle );
};

struct ItemKeyword_Visible : public KeywordHash
{
					ItemKeyword_Visible();
	virtual bool	func( itemDef_t *item, int handle );
};

struct ItemKeyword_Ownerdraw : public KeywordHash
{
					ItemKeyword_Ownerdraw();
	virtual bool	func( itemDef_t *item, int handle );
};

struct ItemKeyword_Align : public KeywordHash
{
					ItemKeyword_Align();
	virtual bool	func( itemDef_t *item, int handle );
};

struct ItemKeyword_TextAlign : public KeywordHash
{
					ItemKeyword_TextAlign();
	virtual bool	func( itemDef_t *item, int handle );
};

struct ItemKeyword_TextAlignX : public KeywordHash
{
					ItemKeyword_TextAlignX();
	virtual bool	func( itemDef_t *item, int handle );
};

struct ItemKeyword_TextAlignY : public KeywordHash
{
					ItemKeyword_TextAlignY();
	virtual bool	func( itemDef_t *item, int handle );
};

struct ItemKeyword_TextScale : public KeywordHash
{
					ItemKeyword_TextScale();
	virtual bool	func( itemDef_t *item, int handle );
};

struct ItemKeyword_TextStyle : public KeywordHash
{
					ItemKeyword_TextStyle();
	virtual bool	func( itemDef_t *item, int handle );
};

struct ItemKeyword_BackColor : public KeywordHash
{
					ItemKeyword_BackColor();
	virtual bool	func( itemDef_t *item, int handle );
};

struct ItemKeyword_ForeColor : public KeywordHash
{
					ItemKeyword_ForeColor();
	virtual bool	func( itemDef_t *item, int handle );
};

struct ItemKeyword_BorderColor : public KeywordHash
{
					ItemKeyword_BorderColor();
	virtual bool	func( itemDef_t *item, int handle );
};

struct ItemKeyword_OutlineColor : public KeywordHash
{
					ItemKeyword_OutlineColor();
	virtual bool	func( itemDef_t *item, int handle );
};

struct ItemKeyword_Background : public KeywordHash
{
					ItemKeyword_Background();
	virtual bool	func( itemDef_t *item, int handle );
};

struct ItemKeyword_OnFocus : public KeywordHash
{
					ItemKeyword_OnFocus();
	virtual bool	func( itemDef_t *item, int handle );
};

struct ItemKeyword_LeaveFocus : public KeywordHash
{
					ItemKeyword_LeaveFocus();
	virtual bool	func( itemDef_t *item, int handle );
};

struct ItemKeyword_MouseEnter : public KeywordHash
{
					ItemKeyword_MouseEnter();
	virtual bool	func( itemDef_t *item, int handle );
};

struct ItemKeyword_MouseExit : public KeywordHash
{
					ItemKeyword_MouseExit();
	virtual bool	func( itemDef_t *item, int handle );
};

struct ItemKeyword_MouseEnterText : public KeywordHash
{
					ItemKeyword_MouseEnterText();
	virtual bool	func( itemDef_t *item, int handle );
};

struct ItemKeyword_MouseExitText : public KeywordHash
{
					ItemKeyword_MouseExitText();
	virtual bool	func( itemDef_t *item, int handle );
};

struct ItemKeyword_Action : public KeywordHash
{
					ItemKeyword_Action();
	virtual bool	func( itemDef_t *item, int handle );
};

struct ItemKeyword_Special : public KeywordHash
{
					ItemKeyword_Special();
	virtual bool	func( itemDef_t *item, int handle );
};

struct ItemKeyword_Cvar : public KeywordHash
{
					ItemKeyword_Cvar();
	virtual bool	func( itemDef_t *item, int handle );
};

struct ItemKeyword_MaxChars : public KeywordHash
{
					ItemKeyword_MaxChars();
	virtual bool	func( itemDef_t *item, int handle );
};

struct ItemKeyword_MaxPaintChars : public KeywordHash
{
					ItemKeyword_MaxPaintChars();
	virtual bool	func( itemDef_t *item, int handle );
};

struct ItemKeyword_FocusSound : public KeywordHash
{
					ItemKeyword_FocusSound();
	virtual bool	func( itemDef_t *item, int handle );
};

struct ItemKeyword_CvarFloat : public KeywordHash
{
					ItemKeyword_CvarFloat();
	virtual bool	func( itemDef_t *item, int handle );
};

struct ItemKeyword_CvarStrList : public KeywordHash
{
					ItemKeyword_CvarStrList();
	virtual bool	func( itemDef_t *item, int handle );
};

struct ItemKeyword_CvarFloatList : public KeywordHash
{
					ItemKeyword_CvarFloatList();
	virtual bool	func( itemDef_t *item, int handle );
};

struct ItemKeyword_AddColorRange : public KeywordHash
{
					ItemKeyword_AddColorRange();
	virtual bool	func( itemDef_t *item, int handle );
};

struct ItemKeyword_OwnerdrawFlag : public KeywordHash
{
					ItemKeyword_OwnerdrawFlag();
	virtual bool	func( itemDef_t *item, int handle );
};

struct ItemKeyword_EnableCvar : public KeywordHash
{
					ItemKeyword_EnableCvar();
	virtual bool	func( itemDef_t *item, int handle );
};

struct ItemKeyword_CvarTest : public KeywordHash
{
					ItemKeyword_CvarTest();
	virtual bool	func( itemDef_t *item, int handle );
};

struct ItemKeyword_DisableCvar : public KeywordHash
{
					ItemKeyword_DisableCvar();
	virtual bool	func( itemDef_t *item, int handle );
};

struct ItemKeyword_ShowCvar : public KeywordHash
{
					ItemKeyword_ShowCvar();
	virtual bool	func( itemDef_t *item, int handle );
};

struct ItemKeyword_HideCvar : public KeywordHash
{
					ItemKeyword_HideCvar();
	virtual bool	func( itemDef_t *item, int handle );
};

struct ItemKeyword_Cinematic : public KeywordHash
{
					ItemKeyword_Cinematic();
	virtual bool	func( itemDef_t *item, int handle );
};

struct ItemKeyword_DoubleClick : public KeywordHash
{
					ItemKeyword_DoubleClick();
	virtual bool	func( itemDef_t *item, int handle );
};

// ------------------

struct MenuKeyword_Font : public KeywordHash
{
					MenuKeyword_Font();
	virtual bool	func( itemDef_t *item, int handle );
};

struct MenuKeyword_Name : public KeywordHash
{
					MenuKeyword_Name();
	virtual bool	func( itemDef_t *item, int handle );
};

struct MenuKeyword_Fullscreen : public KeywordHash
{
					MenuKeyword_Fullscreen();
	virtual bool	func( itemDef_t *item, int handle );
};

struct MenuKeyword_Rect : public KeywordHash
{
					MenuKeyword_Rect();
	virtual bool	func( itemDef_t *item, int handle );
};

struct MenuKeyword_Style : public KeywordHash
{
					MenuKeyword_Style();
	virtual bool	func( itemDef_t *item, int handle );
};

struct MenuKeyword_Visible : public KeywordHash
{
					MenuKeyword_Visible();
	virtual bool	func( itemDef_t *item, int handle );
};

struct MenuKeyword_OnOpen : public KeywordHash
{
					MenuKeyword_OnOpen();
	virtual bool	func( itemDef_t *item, int handle );
};

struct MenuKeyword_OnClose : public KeywordHash
{
					MenuKeyword_OnClose();
	virtual bool	func( itemDef_t *item, int handle );
};

struct MenuKeyword_OnESC : public KeywordHash
{
					MenuKeyword_OnESC();
	virtual bool	func( itemDef_t *item, int handle );
};

struct MenuKeyword_Border : public KeywordHash
{
					MenuKeyword_Border();
	virtual bool	func( itemDef_t *item, int handle );
};

struct MenuKeyword_BorderSize : public KeywordHash
{
					MenuKeyword_BorderSize();
	virtual bool	func( itemDef_t *item, int handle );
};

struct MenuKeyword_BackColor : public KeywordHash
{
					MenuKeyword_BackColor();
	virtual bool	func( itemDef_t *item, int handle );
};

struct MenuKeyword_ForeColor : public KeywordHash
{
					MenuKeyword_ForeColor();
	virtual bool	func( itemDef_t *item, int handle );
};

struct MenuKeyword_BorderColor : public KeywordHash
{
					MenuKeyword_BorderColor();
	virtual bool	func( itemDef_t *item, int handle );
};

struct MenuKeyword_FocusColor : public KeywordHash
{
					MenuKeyword_FocusColor();
	virtual bool	func( itemDef_t *item, int handle );
};

struct MenuKeyword_DisableColor : public KeywordHash
{
					MenuKeyword_DisableColor();
	virtual bool	func( itemDef_t *item, int handle );
};

struct MenuKeyword_OutlineColor : public KeywordHash
{
					MenuKeyword_OutlineColor();
	virtual bool	func( itemDef_t *item, int handle );
};

struct MenuKeyword_Background : public KeywordHash
{
					MenuKeyword_Background();
	virtual bool	func( itemDef_t *item, int handle );
};

struct MenuKeyword_Ownerdraw : public KeywordHash
{
					MenuKeyword_Ownerdraw();
	virtual bool	func( itemDef_t *item, int handle );
};

struct MenuKeyword_OwnerdrawFlag : public KeywordHash
{
					MenuKeyword_OwnerdrawFlag();
	virtual bool	func( itemDef_t *item, int handle );
};

struct MenuKeyword_OutOfBoundsClick : public KeywordHash
{
					MenuKeyword_OutOfBoundsClick();
	virtual bool	func( itemDef_t *item, int handle );
};

struct MenuKeyword_SoundLoop : public KeywordHash
{
					MenuKeyword_SoundLoop();
	virtual bool	func( itemDef_t *item, int handle );
};

struct MenuKeyword_ItemDef : public KeywordHash
{
					MenuKeyword_ItemDef();
	virtual bool	func( itemDef_t *item, int handle );
};

struct MenuKeyword_Cinematic : public KeywordHash
{
					MenuKeyword_Cinematic();
	virtual bool	func( itemDef_t *item, int handle );
};

struct MenuKeyword_Popup : public KeywordHash
{
					MenuKeyword_Popup();
	virtual bool	func( itemDef_t *item, int handle );
};

struct MenuKeyword_FadeClamp : public KeywordHash
{
					MenuKeyword_FadeClamp();
	virtual bool	func( itemDef_t *item, int handle );
};

struct MenuKeyword_FadeCycle : public KeywordHash
{
					MenuKeyword_FadeCycle();
	virtual bool	func( itemDef_t *item, int handle );
};

struct MenuKeyword_FadeAmount : public KeywordHash
{
					MenuKeyword_FadeAmount();
	virtual bool	func( itemDef_t *item, int handle );
};




#endif // __UI_KEYWORDHASH_H__

