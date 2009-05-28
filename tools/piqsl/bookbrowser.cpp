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
		\brief Implement a class extending Fl_Browser with resizable columns.
		Based on original code from Greg Ercolano (http://seriss.com/people/erco/)
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#include	<aqsis/aqsis.h>

#include	<string>
#include	<boost/format.hpp>

#include	"bookbrowser.h"
#include	"piqslmainwindow.h"
#include	"image.h"

extern Aqsis::CqPiqslMainWindow* window;

namespace Aqsis {


int CqBookBrowser::handle(int e) 
{
	// Not showing column separators? Use default Fl_Browser::handle() logic
	if ( ! showcolsep() ) return(Fl_Browser_::handle(e));
	// Handle column resizing
	int ret = 0;
	switch ( e ) 
	{
		case FL_ENTER: 
		{
			ret = 1;
			break;
		}
		case FL_MOVE: 
		{
			if ( which_col_near_mouse() >= 0 ) 
				change_cursor(FL_CURSOR_WE);
			else 
				change_cursor(FL_CURSOR_DEFAULT);
			ret = 1;
			break;
		}
		case FL_PUSH: 
		{
			int whichcol = which_col_near_mouse();
			if ( whichcol >= 0 ) 
			{
				// CLICKED ON RESIZER? START DRAGGING
				ret = 1;
				m_dragging = 1;
				m_dragcol = whichcol;
				//change_cursor(FL_CURSOR_DEFAULT);
			}
			break;
		}
		case FL_DRAG: 
		{
			if ( m_dragging ) 
			{
				ret = 1;
		// Sum up column widths to determine position
				int mousex = Fl::event_x() + hposition();
				int newwidth = mousex - x();
				for ( int t=0; m_widths[t] && t<m_dragcol; t++ ) 
				{
					newwidth -= m_widths[t];
				}
				if ( newwidth > 0 ) 
				{
		// Apply new width, redraw interface
					m_widths[m_dragcol] = newwidth;
					if ( m_widths[m_dragcol] < 2 ) 
					{
						m_widths[m_dragcol] = 2;
					}
					redraw();
				}
			}
			break;
		}
		case FL_LEAVE:
		case FL_RELEASE: 
		{
			m_dragging = 0;				// disable drag mode
			change_cursor(FL_CURSOR_DEFAULT);	// ensure normal cursor
			ret = 1;
			break;
		}
		case FL_KEYDOWN:
		case FL_SHORTCUT:
		{
			int key = Fl::event_key();
			switch(key)
			{
				case FL_Down:
					if(selection() != NULL && item_next(selection()) != NULL)
					{
						select(item_next(selection()),1, 1);
						::window->select();
					}
					return 1;
				case FL_Up:
					if(selection() != NULL && item_prev(selection()) != NULL)
					{
						select(item_prev(selection()),1, 1);
						::window->select();
					}
					return 1;
			}
			break;
		}
	}
	if(m_dragging) 
		return(1);			// dragging? don't pass event to Fl_Browser
	int hret = Fl_Browser_::handle(e);
	return(hret ? 1 : ret);
}

void CqBookBrowser::draw() 
{
	// DRAW BROWSER
	Fl_Browser_::draw();
	if ( m_showcolsep ) 
	{
		// DRAW COLUMN SEPARATORS
		int colx = this->x() - hposition();
		int X,Y,W,H;
		Fl_Browser_::bbox(X,Y,W,H);
		fl_color(m_colsepcolor);
		for ( int t=0; m_widths[t]; t++ ) 
		{
			colx += m_widths[t];
			if ( colx > X && colx < (X+W) ) 
			{
				fl_line(colx, Y, colx, Y+H-1);
			}
		}
	}
}

void* CqBookBrowser::item_first() const
{
	if(m_theBook && m_theBook->numImages() > 0)
		return((void*)1);
	else
		return((void*)0);
}

void* CqBookBrowser::item_next(void* p) const
{
	if(!p)
		return(p);
	else
	{
		CqBook::TqImageList::size_type index = 
			reinterpret_cast<CqBook::TqImageList::size_type>(p);
		if(m_theBook && index < m_theBook->numImages())
			return((void*)(index + 1));
		else
			return((void*)0);
	}
}

void* CqBookBrowser::item_prev(void* p) const
{
	if(!p)
		return(p);
	else
	{
		CqBook::TqImageList::size_type index = 
			reinterpret_cast<CqBook::TqImageList::size_type>(p);
		if(m_theBook && index > 1)
			return((void*)(index - 1));
		else
			return((void*)0);
	}
}

int CqBookBrowser::item_selected(void* l) const 
{
	CqBook::TqImageList::size_type index = 
		reinterpret_cast<CqBook::TqImageList::size_type>(l);
	
	return(index == currentSelected());
}

void CqBookBrowser::item_select(void* l, int v) 
{
	CqBook::TqImageList::size_type index = 
		reinterpret_cast<CqBook::TqImageList::size_type>(l);
	if(v)
		setCurrentSelected(index);
	else
		setCurrentSelected(0);
}

int CqBookBrowser::item_height(void* lv) const 
{
	int hmax = 2; // use 2 to insure we don't return a zero!

	fl_font(textfont(), textsize());
	int hh = fl_height();
	if (hh > hmax) 
		hmax = hh;

	return hmax; 
}


int CqBookBrowser::item_width(void* v) const 
{
	const int* i = column_widths();
	int ww = i[0] + i[1];

	int tsize = textsize();
	Fl_Font font = textfont();

	fl_font(font, tsize);

	return ww + 6;
}

void CqBookBrowser::item_draw(void* v, int X, int Y, int W, int H) const 
{
	CqBook::TqImageList::size_type index = 
		reinterpret_cast<CqBook::TqImageList::size_type>(v);
	index--;
	if(!m_theBook || index < 0 || index > m_theBook->numImages())
		return;

	const int* i = column_widths();

	int tsize = textsize();
	Fl_Font font = textfont();
	Fl_Color lcol = textcolor();
	Fl_Align talign = FL_ALIGN_LEFT;

	fl_font(font, tsize);
//	if (((FL_BLINE*)v)->flags & SELECTED)
//		lcol = fl_contrast(lcol, selection_color());
	if (!active_r()) 
	{
		lcol = fl_inactive(lcol);
	}
	fl_color(lcol);
	fl_draw(m_theBook->image(index)->name().c_str(), X+3, Y, i[0]-6, H,
			Fl_Align(talign|FL_ALIGN_CLIP), 0, 0);
	X += i[0];
	W -= i[0];
	boost::format size("%1%x%2%x%3% ");
	size % m_theBook->image(index)->imageWidth();
	size % m_theBook->image(index)->imageHeight();
	size % m_theBook->image(index)->numChannels();
	std::string _result = size.str();
	if ( !m_theBook->image(index)->description().empty())
	{
		_result.append(m_theBook->image(index)->description());
	}
	fl_draw(_result.c_str(), X+3, Y, i[1]-6, H, talign, 0, 0);
}

void CqBookBrowser::change_cursor(Fl_Cursor newcursor) 
{
	if ( newcursor != m_last_cursor ) 
	{
		fl_cursor(newcursor, FL_BLACK, FL_WHITE);
		m_last_cursor = newcursor;
	}
}

int CqBookBrowser::which_col_near_mouse() 
{
	int X,Y,W,H;
	Fl_Browser_::bbox(X,Y,W,H);		// area inside browser's box()
	// EVENT NOT INSIDE BROWSER AREA? (eg. on a scrollbar)
	if ( ! Fl::event_inside(X,Y,W,H) ) 
	{
		return(-1);
	}
	int mousex = Fl::event_x() + hposition();
	int colx = this->x();
	for ( int t=0; m_widths[t]; t++ ) 
	{
		colx += m_widths[t];
		int diff = mousex - colx;
		// MOUSE 'NEAR' A COLUMN?
		//     Return column #
		//
		if ( diff >= -4 && diff <= 4 ) {
			return(t);
		}
	}
	return(-1);
}

} // namespace Aqsis
