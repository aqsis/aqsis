#ifndef CENTERSCROLL_H_INCLUDED
#define CENTERSCROLL_H_INCLUDED

#include <FL/Fl_Scroll.H>

#include "zoomimage.h"

namespace Aqsis {

class CqCenterScroll : public Fl_Scroll
{
public:
	CqCenterScroll(int x, int y, int w, int h, const char* l = 0)
		: Fl_Scroll(x,y,w,h,l),
		m_imageWidget(x,y),
		m_prevDragX(0),
		m_prevDragY(0)
	{ }
	int handle(int event)
	{
		switch(event)
		{
			case FL_PUSH:
				if(Fl::event_button() == FL_MIDDLE_MOUSE)
				{
					m_prevDragX = Fl::event_x();
					m_prevDragY = Fl::event_y();
					return 1;
				}
				break;
			case FL_RELEASE:
				if(Fl::event_button() == FL_MIDDLE_MOUSE)
					return 1;
				break;
			case FL_DRAG:
				if(Fl::event_button() == FL_MIDDLE_MOUSE)
				{
					position(xposition() - (Fl::event_x() - m_prevDragX),
							yposition() - (Fl::event_y() - m_prevDragY));
					m_prevDragX = Fl::event_x();
					m_prevDragY = Fl::event_y();
					return 1;
				}
				break;
		}
		if(m_imageWidget.handle(event))
			return(1);
		else
			return Fl_Scroll::handle(event);
	}
	void setImage(const boost::shared_ptr<CqImage>& image)
	{
		m_imageWidget.position(x() + (w()/2)-(image->imageWidth()/2), y() + (h()/2)-(image->imageHeight()/2));
		m_imageWidget.setImage(image);
	}

	void update(int X, int Y, int W, int H)
	{
		m_imageWidget.update(X, Y, W, H);
	}
private:
	CqZoomImage m_imageWidget;
	int m_prevDragX;
	int m_prevDragY;
};

//-----------------------------------------------------------------------

} // namespace Aqsis

#endif // CENTERSCROLL_H_INCLUDED
