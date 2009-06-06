// Aqsis
// Copyright (C) 1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.org
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#include "centerscroll.h"

#include <FL/Fl.H>

#include "image.h"

namespace Aqsis {

CqCenterScroll::CqCenterScroll(int x, int y, int w, int h, const char* l)
	: Fl_Scroll(x,y,w,h,l),
	m_imageWidget(x,y),
	m_prevDragX(0),
	m_prevDragY(0)
{ }

int CqCenterScroll::handle(int event)
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
		case FL_KEYDOWN:
		case FL_SHORTCUT:
			int key = Fl::event_key();
			switch(key)
			{
				case 'h':
					if(Fl::event_alt())
						return 0;
						// passthrough
				case FL_Home:
					centerImageWidget();
					return 1;
					break;

				// Ignore cursors, so that the browser gets them.
				case FL_Up:
				case FL_Down:
				case FL_Left:
				case FL_Right:
					return 0;
			}
			break;
	}
	if(m_imageWidget.handle(event))
		return(1);
	else
		return Fl_Scroll::handle(event);
}

void CqCenterScroll::setImage(const boost::shared_ptr<CqImage>& image)
{
	//m_imageWidget.position(x() + (w()/2)-(image->imageWidth()/2), y() + (h()/2)-(image->imageHeight()/2));
	m_imageWidget.setImage(image);
}

void CqCenterScroll::centerImageWidget()
{
	if(m_imageWidget.image())
	{
		m_imageWidget.position(x() + (w()/2)-(m_imageWidget.image()->imageWidth()/2),
								y() + (h()/2)-(m_imageWidget.image()->imageHeight()/2));
		damage(FL_DAMAGE_ALL);
	}
}

boost::shared_ptr<CqImage> CqCenterScroll::image()
{
	return m_imageWidget.image();
}

void CqCenterScroll::resizeImageWidget(int w, int h)
{
	m_imageWidget.size(w, h);
}

void CqCenterScroll::update(int X, int Y, int W, int H)
{
	m_imageWidget.update(X, Y, W, H);
}

} // namespace Aqsis
