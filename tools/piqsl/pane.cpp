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

/** \file
 * \brief A pane widget with movable divider separating left and right columns.
 */

#include "pane.h"

#include <boost/bind.hpp>

#include <aqsis/math/math.h>

namespace Aqsis {

//------------------------------------------------------------------------------
// CqPaneDividerBox implementation
CqPaneDividerBox::CqPaneDividerBox(int x, int y, int w, int h, const char* l)
	: Fl_Box(FL_FLAT_BOX, x, y, w, h, l),
	m_prevDragX(0)
{ }

void CqPaneDividerBox::setDragCallback(const boost::function<void (int)>& fxn)
{
	m_dragCallback = fxn;
}

void CqPaneDividerBox::draw()
{
	Fl_Box::draw();
	for(int i = 0; i < 5; ++i)
	{
		fl_color(FL_DARK2);
		fl_rectf(x() + 2, y() + h()/2 + (i-2)*5, 2, 2);
		fl_color(FL_LIGHT2);
		fl_point(x() + 2, y() + h()/2 + (i-2)*5);
	}
}

int CqPaneDividerBox::handle(int event)
{
	switch(event)
	{
		case FL_ENTER:
			color(FL_LIGHT1);
			fl_cursor(FL_CURSOR_WE, FL_FOREGROUND_COLOR, FL_BACKGROUND_COLOR);
			redraw();
			return 1;
		case FL_LEAVE:
			color(FL_BACKGROUND_COLOR);
			fl_cursor(FL_CURSOR_DEFAULT, FL_FOREGROUND_COLOR, FL_BACKGROUND_COLOR);
			redraw();
			return 1;
		case FL_PUSH:
			switch(Fl::event_button())
			{
				case FL_LEFT_MOUSE:
					m_prevDragX = Fl::event_x();
					return 1;
			}
			break;
		case FL_RELEASE:
			switch(Fl::event_button())
			{
				case FL_LEFT_MOUSE:
					return 1;
			}
			break;
		case FL_DRAG:
			switch(Fl::event_button())
			{
				case FL_LEFT_MOUSE:
					if(m_dragCallback)
						m_dragCallback(Fl::event_x() - m_prevDragX);
					m_prevDragX = Fl::event_x();
					return 1;
			}
			break;
	}
	return Fl_Box::handle(event);
}

void CqPaneDividerBox::resize(int x, int y, int w, int h)
{
	Fl_Box::resize(x, y, 6, h);
}


//------------------------------------------------------------------------------
// CqPane implementation
CqPane::CqPane(int x, int y, int w, int h, const char* l)
	: Fl_Group(x,y,w,h,l),
	m_divider(x+w/2, y, 0, h),
	m_prevDividerPos(0),
	m_browser(0),
	m_centerScroll(0)
{
	init();
}

CqPane::CqPane(int x, int y, int w, int h, double divPos, const char* l)
	: Fl_Group(x,y,w,h,l),
	m_divider(x+static_cast<int>(divPos*w), y, 0, h),
	m_prevDividerPos(0),
	m_browser(0),
	m_centerScroll(0)
{
	init();
}

void CqPane::uncollapse()
{
	moveDivider(m_prevDividerPos - m_divider.x());
}

void CqPane::collapse1()
{
	if(m_divider.x() != x())
		moveDivider(x() - m_divider.x());
}

void CqPane::add1(CqBookBrowser* widget)
{
	remove(m_browser);
	m_browser = widget;
	moveDivider(0);
	Fl_Group::add(widget);
}

void CqPane::add2(CqCenterScroll* widget)
{
	remove(m_centerScroll);
	m_centerScroll = widget;
	resizable(widget);
	moveDivider(0);
	Fl_Group::add(widget);
}

CqBookBrowser* CqPane::browser()
{
	return m_browser;
}

CqCenterScroll* CqPane::centerScroll()
{
	return m_centerScroll;
}

void CqPane::resize(int x, int y, int w, int h)
{
	Fl_Group::resize(x, y, w, h);
	if(m_divider.x() + m_divider.w() > x + w)
		moveDivider(0);
}

/// Move the position of the divider by an offset.
void CqPane::moveDivider(int dx)
{
	m_prevDividerPos = m_divider.x();
	int dividerX = clamp(m_divider.x() + dx, 0, w() - m_divider.w());
	m_divider.resize(dividerX, y(), m_divider.w(), h());
	if(m_browser)
	{
		m_browser->resize(x(), y(), m_divider.x()-x(), h());
	}
	if(m_centerScroll)
	{
		int w2X = m_divider.x() + m_divider.w();
		m_centerScroll->resize(w2X, y(), w() - w2X, h());
		init_sizes();
	}
	damage(FL_DAMAGE_ALL);
}

void CqPane::init()
{
	box(FL_FLAT_BOX);
	m_divider.setDragCallback(boost::bind(&CqPane::moveDivider, this, _1));
	end();
}

} // namespace Aqsis
