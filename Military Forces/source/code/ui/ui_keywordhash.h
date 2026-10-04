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
	bool	func( itemDef_t *item, int handle ) override;
};

struct ItemKeyword_Text : public KeywordHash
{
					ItemKeyword_Text();
	bool	func( itemDef_t *item, int handle ) override;
};

struct ItemKeyword_Group : public KeywordHash
{
					ItemKeyword_Group();
	bool	func( itemDef_t *item, int handle ) override;
};

struct ItemKeyword_Asset_Model : public KeywordHash
{
					ItemKeyword_Asset_Model();
	bool	func( itemDef_t *item, int handle ) override;
};

struct ItemKeyword_Asset_Shader : public KeywordHash
{
					ItemKeyword_Asset_Shader();
	bool	func( itemDef_t *item, int handle ) override;
};

struct ItemKeyword_Model_Origin : public KeywordHash
{
					ItemKeyword_Model_Origin();
	bool	func( itemDef_t *item, int handle ) override;
};

struct ItemKeyword_Model_fovX : public KeywordHash
{
					ItemKeyword_Model_fovX();
	bool	func( itemDef_t *item, int handle ) override;
};

struct ItemKeyword_Model_fovY : public KeywordHash
{
					ItemKeyword_Model_fovY();
	bool	func( itemDef_t *item, int handle ) override;
};

struct ItemKeyword_Model_Rotation : public KeywordHash
{
					ItemKeyword_Model_Rotation();
	bool	func( itemDef_t *item, int handle ) override;
};

struct ItemKeyword_Model_Angle : public KeywordHash
{
					ItemKeyword_Model_Angle();
	bool	func( itemDef_t *item, int handle ) override;
};

struct ItemKeyword_Rect : public KeywordHash
{
					ItemKeyword_Rect();
	bool	func( itemDef_t *item, int handle ) override;
};

struct ItemKeyword_Style : public KeywordHash
{
					ItemKeyword_Style();
	bool	func( itemDef_t *item, int handle ) override;
};

struct ItemKeyword_Decoration : public KeywordHash
{
					ItemKeyword_Decoration();
	bool	func( itemDef_t *item, int handle ) override;
};

struct ItemKeyword_NotSelectable : public KeywordHash
{
					ItemKeyword_NotSelectable();
	bool	func( itemDef_t *item, int handle ) override;
};

struct ItemKeyword_Wrapped : public KeywordHash
{
					ItemKeyword_Wrapped();
	bool	func( itemDef_t *item, int handle ) override;
};

struct ItemKeyword_AutoWrapped : public KeywordHash
{
					ItemKeyword_AutoWrapped();
	bool	func( itemDef_t *item, int handle ) override;
};

struct ItemKeyword_HorizontalScroll : public KeywordHash
{
					ItemKeyword_HorizontalScroll();
	bool	func( itemDef_t *item, int handle ) override;
};

struct ItemKeyword_Type : public KeywordHash
{
					ItemKeyword_Type();
	bool	func( itemDef_t *item, int handle ) override;
};

struct ItemKeyword_ElementWidth : public KeywordHash
{
					ItemKeyword_ElementWidth();
	bool	func( itemDef_t *item, int handle ) override;
};

struct ItemKeyword_ElementHeight : public KeywordHash
{
					ItemKeyword_ElementHeight();
	bool	func( itemDef_t *item, int handle ) override;
};

struct ItemKeyword_Feeder : public KeywordHash
{
					ItemKeyword_Feeder();
	bool	func( itemDef_t *item, int handle ) override;
};

struct ItemKeyword_ElementType : public KeywordHash
{
					ItemKeyword_ElementType();
	bool	func( itemDef_t *item, int handle ) override;
};

struct ItemKeyword_Columns : public KeywordHash
{
					ItemKeyword_Columns();
	bool	func( itemDef_t *item, int handle ) override;
};

struct ItemKeyword_Border : public KeywordHash
{
					ItemKeyword_Border();
	bool	func( itemDef_t *item, int handle ) override;
};

struct ItemKeyword_BorderSize : public KeywordHash
{
					ItemKeyword_BorderSize();
	bool	func( itemDef_t *item, int handle ) override;
};

struct ItemKeyword_Visible : public KeywordHash
{
					ItemKeyword_Visible();
	bool	func( itemDef_t *item, int handle ) override;
};

struct ItemKeyword_Ownerdraw : public KeywordHash
{
					ItemKeyword_Ownerdraw();
	bool	func( itemDef_t *item, int handle ) override;
};

struct ItemKeyword_Align : public KeywordHash
{
					ItemKeyword_Align();
	bool	func( itemDef_t *item, int handle ) override;
};

struct ItemKeyword_TextAlign : public KeywordHash
{
					ItemKeyword_TextAlign();
	bool	func( itemDef_t *item, int handle ) override;
};

struct ItemKeyword_TextAlignX : public KeywordHash
{
					ItemKeyword_TextAlignX();
	bool	func( itemDef_t *item, int handle ) override;
};

struct ItemKeyword_TextAlignY : public KeywordHash
{
					ItemKeyword_TextAlignY();
	bool	func( itemDef_t *item, int handle ) override;
};

struct ItemKeyword_TextScale : public KeywordHash
{
					ItemKeyword_TextScale();
	bool	func( itemDef_t *item, int handle ) override;
};

struct ItemKeyword_TextStyle : public KeywordHash
{
					ItemKeyword_TextStyle();
	bool	func( itemDef_t *item, int handle ) override;
};

struct ItemKeyword_BackColor : public KeywordHash
{
					ItemKeyword_BackColor();
	bool	func( itemDef_t *item, int handle ) override;
};

struct ItemKeyword_ForeColor : public KeywordHash
{
					ItemKeyword_ForeColor();
	bool	func( itemDef_t *item, int handle ) override;
};

struct ItemKeyword_BorderColor : public KeywordHash
{
					ItemKeyword_BorderColor();
	bool	func( itemDef_t *item, int handle ) override;
};

struct ItemKeyword_OutlineColor : public KeywordHash
{
					ItemKeyword_OutlineColor();
	bool	func( itemDef_t *item, int handle ) override;
};

struct ItemKeyword_Background : public KeywordHash
{
					ItemKeyword_Background();
	bool	func( itemDef_t *item, int handle ) override;
};

struct ItemKeyword_OnFocus : public KeywordHash
{
					ItemKeyword_OnFocus();
	bool	func( itemDef_t *item, int handle ) override;
};

struct ItemKeyword_LeaveFocus : public KeywordHash
{
					ItemKeyword_LeaveFocus();
	bool	func( itemDef_t *item, int handle ) override;
};

struct ItemKeyword_MouseEnter : public KeywordHash
{
					ItemKeyword_MouseEnter();
	bool	func( itemDef_t *item, int handle ) override;
};

struct ItemKeyword_MouseExit : public KeywordHash
{
					ItemKeyword_MouseExit();
	bool	func( itemDef_t *item, int handle ) override;
};

struct ItemKeyword_MouseEnterText : public KeywordHash
{
					ItemKeyword_MouseEnterText();
	bool	func( itemDef_t *item, int handle ) override;
};

struct ItemKeyword_MouseExitText : public KeywordHash
{
					ItemKeyword_MouseExitText();
	bool	func( itemDef_t *item, int handle ) override;
};

struct ItemKeyword_Action : public KeywordHash
{
					ItemKeyword_Action();
	bool	func( itemDef_t *item, int handle ) override;
};

struct ItemKeyword_Special : public KeywordHash
{
					ItemKeyword_Special();
	bool	func( itemDef_t *item, int handle ) override;
};

struct ItemKeyword_Cvar : public KeywordHash
{
					ItemKeyword_Cvar();
	bool	func( itemDef_t *item, int handle ) override;
};

struct ItemKeyword_MaxChars : public KeywordHash
{
					ItemKeyword_MaxChars();
	bool	func( itemDef_t *item, int handle ) override;
};

struct ItemKeyword_MaxPaintChars : public KeywordHash
{
					ItemKeyword_MaxPaintChars();
	bool	func( itemDef_t *item, int handle ) override;
};

struct ItemKeyword_FocusSound : public KeywordHash
{
					ItemKeyword_FocusSound();
	bool	func( itemDef_t *item, int handle ) override;
};

struct ItemKeyword_CvarFloat : public KeywordHash
{
					ItemKeyword_CvarFloat();
	bool	func( itemDef_t *item, int handle ) override;
};

struct ItemKeyword_CvarStrList : public KeywordHash
{
					ItemKeyword_CvarStrList();
	bool	func( itemDef_t *item, int handle ) override;
};

struct ItemKeyword_CvarFloatList : public KeywordHash
{
					ItemKeyword_CvarFloatList();
	bool	func( itemDef_t *item, int handle ) override;
};

struct ItemKeyword_AddColorRange : public KeywordHash
{
					ItemKeyword_AddColorRange();
	bool	func( itemDef_t *item, int handle ) override;
};

struct ItemKeyword_OwnerdrawFlag : public KeywordHash
{
					ItemKeyword_OwnerdrawFlag();
	bool	func( itemDef_t *item, int handle ) override;
};

struct ItemKeyword_EnableCvar : public KeywordHash
{
					ItemKeyword_EnableCvar();
	bool	func( itemDef_t *item, int handle ) override;
};

struct ItemKeyword_CvarTest : public KeywordHash
{
					ItemKeyword_CvarTest();
	bool	func( itemDef_t *item, int handle ) override;
};

struct ItemKeyword_DisableCvar : public KeywordHash
{
					ItemKeyword_DisableCvar();
	bool	func( itemDef_t *item, int handle ) override;
};

struct ItemKeyword_ShowCvar : public KeywordHash
{
					ItemKeyword_ShowCvar();
	bool	func( itemDef_t *item, int handle ) override;
};

struct ItemKeyword_HideCvar : public KeywordHash
{
					ItemKeyword_HideCvar();
	bool	func( itemDef_t *item, int handle ) override;
};

struct ItemKeyword_Cinematic : public KeywordHash
{
					ItemKeyword_Cinematic();
	bool	func( itemDef_t *item, int handle ) override;
};

struct ItemKeyword_DoubleClick : public KeywordHash
{
					ItemKeyword_DoubleClick();
	bool	func( itemDef_t *item, int handle ) override;
};

// ------------------

struct MenuKeyword_Font : public KeywordHash
{
					MenuKeyword_Font();
	bool	func( itemDef_t *item, int handle ) override;
};

struct MenuKeyword_Name : public KeywordHash
{
					MenuKeyword_Name();
	bool	func( itemDef_t *item, int handle ) override;
};

struct MenuKeyword_Fullscreen : public KeywordHash
{
					MenuKeyword_Fullscreen();
	bool	func( itemDef_t *item, int handle ) override;
};

struct MenuKeyword_Rect : public KeywordHash
{
					MenuKeyword_Rect();
	bool	func( itemDef_t *item, int handle ) override;
};

struct MenuKeyword_Style : public KeywordHash
{
					MenuKeyword_Style();
	bool	func( itemDef_t *item, int handle ) override;
};

struct MenuKeyword_Visible : public KeywordHash
{
					MenuKeyword_Visible();
	bool	func( itemDef_t *item, int handle ) override;
};

struct MenuKeyword_OnOpen : public KeywordHash
{
					MenuKeyword_OnOpen();
	bool	func( itemDef_t *item, int handle ) override;
};

struct MenuKeyword_OnClose : public KeywordHash
{
					MenuKeyword_OnClose();
	bool	func( itemDef_t *item, int handle ) override;
};

struct MenuKeyword_OnESC : public KeywordHash
{
					MenuKeyword_OnESC();
	bool	func( itemDef_t *item, int handle ) override;
};

struct MenuKeyword_Border : public KeywordHash
{
					MenuKeyword_Border();
	bool	func( itemDef_t *item, int handle ) override;
};

struct MenuKeyword_BorderSize : public KeywordHash
{
					MenuKeyword_BorderSize();
	bool	func( itemDef_t *item, int handle ) override;
};

struct MenuKeyword_BackColor : public KeywordHash
{
					MenuKeyword_BackColor();
	bool	func( itemDef_t *item, int handle ) override;
};

struct MenuKeyword_ForeColor : public KeywordHash
{
					MenuKeyword_ForeColor();
	bool	func( itemDef_t *item, int handle ) override;
};

struct MenuKeyword_BorderColor : public KeywordHash
{
					MenuKeyword_BorderColor();
	bool	func( itemDef_t *item, int handle ) override;
};

struct MenuKeyword_FocusColor : public KeywordHash
{
					MenuKeyword_FocusColor();
	bool	func( itemDef_t *item, int handle ) override;
};

struct MenuKeyword_DisableColor : public KeywordHash
{
					MenuKeyword_DisableColor();
	bool	func( itemDef_t *item, int handle ) override;
};

struct MenuKeyword_OutlineColor : public KeywordHash
{
					MenuKeyword_OutlineColor();
	bool	func( itemDef_t *item, int handle ) override;
};

struct MenuKeyword_Background : public KeywordHash
{
					MenuKeyword_Background();
	bool	func( itemDef_t *item, int handle ) override;
};

struct MenuKeyword_Ownerdraw : public KeywordHash
{
					MenuKeyword_Ownerdraw();
	bool	func( itemDef_t *item, int handle ) override;
};

struct MenuKeyword_OwnerdrawFlag : public KeywordHash
{
					MenuKeyword_OwnerdrawFlag();
	bool	func( itemDef_t *item, int handle ) override;
};

struct MenuKeyword_OutOfBoundsClick : public KeywordHash
{
					MenuKeyword_OutOfBoundsClick();
	bool	func( itemDef_t *item, int handle ) override;
};

struct MenuKeyword_SoundLoop : public KeywordHash
{
					MenuKeyword_SoundLoop();
	bool	func( itemDef_t *item, int handle ) override;
};

struct MenuKeyword_ItemDef : public KeywordHash
{
					MenuKeyword_ItemDef();
	bool	func( itemDef_t *item, int handle ) override;
};

struct MenuKeyword_Cinematic : public KeywordHash
{
					MenuKeyword_Cinematic();
	bool	func( itemDef_t *item, int handle ) override;
};

struct MenuKeyword_Popup : public KeywordHash
{
					MenuKeyword_Popup();
	bool	func( itemDef_t *item, int handle ) override;
};

struct MenuKeyword_FadeClamp : public KeywordHash
{
					MenuKeyword_FadeClamp();
	bool	func( itemDef_t *item, int handle ) override;
};

struct MenuKeyword_FadeCycle : public KeywordHash
{
					MenuKeyword_FadeCycle();
	bool	func( itemDef_t *item, int handle ) override;
};

struct MenuKeyword_FadeAmount : public KeywordHash
{
					MenuKeyword_FadeAmount();
	bool	func( itemDef_t *item, int handle ) override;
};




#endif // __UI_KEYWORDHASH_H__

