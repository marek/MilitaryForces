#ifndef __UI_ITEMCAPTURE_H__
#define __UI_ITEMCAPTURE_H__



struct UI_ScrollInfo;
struct UI_Utils;

struct UI_CaptureBase
{
	explicit			UI_CaptureBase( UI_ScrollInfo* si );
	virtual				~UI_CaptureBase() {}

	virtual void		capture() = 0;

	static void			setUtils( UI_Utils* utils );

protected:
	// data
	static UI_Utils*	utils_;

	UI_ScrollInfo*		scrollInfo_;
};

struct UI_Capture_ListBoxAuto : public UI_CaptureBase
{
	explicit			UI_Capture_ListBoxAuto( UI_ScrollInfo* si );

	virtual void		capture();
};

struct UI_Capture_ListBoxThumb : public UI_CaptureBase
{
	explicit			UI_Capture_ListBoxThumb( UI_ScrollInfo* si );

	virtual void		capture();
};

struct UI_Capture_SliderThumb : public UI_CaptureBase
{
	explicit			UI_Capture_SliderThumb( UI_ScrollInfo* si );

	virtual void		capture();
};




#endif // __UI_ITEMCAPTURE_H__


