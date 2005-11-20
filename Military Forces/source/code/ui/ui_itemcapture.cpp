#include "ui_itemcapture.h"
#include "ui_scrollinfo.h"
#include "ui_utils.h"



UI_Utils* UI_CaptureBase::utils_ = 0;

UI_CaptureBase::UI_CaptureBase( UI_ScrollInfo* si ) :
	scrollInfo_(si)
{

}

void
UI_CaptureBase::setUtils( UI_Utils* utils )
{
	utils_ = utils;
}

UI_Capture_ListBoxAuto::UI_Capture_ListBoxAuto( UI_ScrollInfo* si ) :
	UI_CaptureBase( si )
{
}

void
UI_Capture_ListBoxAuto::capture()
{
	if (utils_->dc_->realTime_ > scrollInfo_->nextScrollTime_) 
	{ 
		// need to scroll which is done by simulating a click to the item
		// this is done a bit sideways as the autoscroll "knows" that the item is a listbox
		// so it calls it directly
		utils_->item_ListBox_HandleKey(scrollInfo_->item_, scrollInfo_->scrollKey_, 1, false);
		scrollInfo_->nextScrollTime_ = utils_->dc_->realTime_ + scrollInfo_->adjustValue_; 
	}

	if (utils_->dc_->realTime_ > scrollInfo_->nextAdjustTime_) 
	{
		scrollInfo_->nextAdjustTime_ = utils_->dc_->realTime_ + UI_Utils::SCROLL_TIME_ADJUST;
		if (scrollInfo_->adjustValue_ > UI_Utils::SCROLL_TIME_FLOOR) 
			scrollInfo_->adjustValue_ -= UI_Utils::SCROLL_TIME_ADJUSTOFFSET;
	}
}

UI_Capture_ListBoxThumb::UI_Capture_ListBoxThumb( UI_ScrollInfo* si ) :
	UI_CaptureBase( si )
{
}

void 
UI_Capture_ListBoxThumb::capture()
{
	rectDef_t r;
	int pos, max;

	listBoxDef_t *listPtr = (listBoxDef_t*)scrollInfo_->item_->typeData;
	if (scrollInfo_->item_->window.flags & WINDOW_HORIZONTAL) 
	{
		if (utils_->dc_->cursorX_ == scrollInfo_->xStart_) 
		{
			return;
		}
		r.x = scrollInfo_->item_->window.rect.x + SCROLLBAR_SIZE + 1;
		r.y = scrollInfo_->item_->window.rect.y + scrollInfo_->item_->window.rect.h - SCROLLBAR_SIZE - 1;
		r.h = SCROLLBAR_SIZE;
		r.w = scrollInfo_->item_->window.rect.w - (SCROLLBAR_SIZE*2) - 2;
		max = utils_->item_ListBox_MaxScroll(scrollInfo_->item_);
		//
		pos = (utils_->dc_->cursorX_ - r.x - SCROLLBAR_SIZE/2) * max / (r.w - SCROLLBAR_SIZE);
		if (pos < 0) 
			pos = 0;
		else if (pos > max) 
			pos = max;

		listPtr->startPos = pos;
		scrollInfo_->xStart_ = utils_->dc_->cursorX_;
	}
	else if (utils_->dc_->cursorY_ != scrollInfo_->yStart_) 
	{

		r.x = scrollInfo_->item_->window.rect.x + scrollInfo_->item_->window.rect.w - SCROLLBAR_SIZE - 1;
		r.y = scrollInfo_->item_->window.rect.y + SCROLLBAR_SIZE + 1;
		r.h = scrollInfo_->item_->window.rect.h - (SCROLLBAR_SIZE*2) - 2;
		r.w = SCROLLBAR_SIZE;
		max = utils_->item_ListBox_MaxScroll(scrollInfo_->item_);
		//
		pos = (utils_->dc_->cursorY_ - r.y - SCROLLBAR_SIZE/2) * max / (r.h - SCROLLBAR_SIZE);
		if (pos < 0) 
			pos = 0;
		else if (pos > max) 
			pos = max;
		
		listPtr->startPos = pos;
		scrollInfo_->yStart_ = utils_->dc_->cursorY_;
	}

	if (utils_->dc_->realTime_ > scrollInfo_->nextScrollTime_) 
	{ 
		// need to scroll which is done by simulating a click to the item
		// this is done a bit sideways as the autoscroll "knows" that the item is a listbox
		// so it calls it directly
		utils_->item_ListBox_HandleKey(scrollInfo_->item_, scrollInfo_->scrollKey_, 1, false);
		scrollInfo_->nextScrollTime_ = utils_->dc_->realTime_ + scrollInfo_->adjustValue_; 
	}

	if (utils_->dc_->realTime_ > scrollInfo_->nextAdjustTime_) {
		scrollInfo_->nextAdjustTime_ = utils_->dc_->realTime_ + UI_Utils::SCROLL_TIME_ADJUST;
		if (scrollInfo_->adjustValue_ > UI_Utils::SCROLL_TIME_FLOOR) {
			scrollInfo_->adjustValue_ -= UI_Utils::SCROLL_TIME_ADJUSTOFFSET;
		}
	}
}

UI_Capture_SliderThumb::UI_Capture_SliderThumb( UI_ScrollInfo* si ) :
	UI_CaptureBase( si )
{
}

void
UI_Capture_SliderThumb::capture()
{
	float x, value, cursorx;
	editFieldDef_t *editDef = reinterpret_cast<editFieldDef_t*>(scrollInfo_->item_->typeData);

	if (scrollInfo_->item_->text) 
		x = scrollInfo_->item_->textRect.x + scrollInfo_->item_->textRect.w + 8;
	else 
		x = scrollInfo_->item_->window.rect.x;

	cursorx = utils_->dc_->cursorX_;

	if (cursorx < x) 
		cursorx = x;
	else if (cursorx > x + SLIDER_WIDTH) 
		cursorx = x + SLIDER_WIDTH;

	value = cursorx - x;
	value /= SLIDER_WIDTH;
	value *= (editDef->maxVal - editDef->minVal);
	value += editDef->minVal;
	utils_->dc_->setCVar(scrollInfo_->item_->cvar, va("%f", value));
}





