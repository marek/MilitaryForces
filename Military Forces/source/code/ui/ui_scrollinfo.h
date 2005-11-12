#ifndef __UI_SCROLLINFO_H__
#define __UI_SCROLLINFO_H__




struct itemDef_t;

struct UI_ScrollInfo 
{
	int			nextScrollTime_;
	int			nextAdjustTime_;
	int			adjustValue_;
	int			scrollKey_;
	float		xStart_;
	float		yStart_;
	itemDef_t*	item_;
	bool		scrollDir_;
};




#endif // __UI_SCROLLINFO_H__
