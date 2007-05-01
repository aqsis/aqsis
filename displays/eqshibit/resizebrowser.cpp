// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.com
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
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include "resizebrowser.h"
int Fl_Resize_Browser::handle(int e) 
{
	// Not showing column separators? Use default Fl_Browser::handle() logic
	if ( ! showcolsep() ) return(Fl_Browser::handle(e));
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
			{
				change_cursor(FL_CURSOR_WE);
			} 
			else 
			{
				change_cursor(FL_CURSOR_DEFAULT);
			}
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
				change_cursor(FL_CURSOR_DEFAULT);
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
	}
	if ( m_dragging ) return(1);			// dragging? don't pass event to Fl_Browser
		return(Fl_Browser::handle(e) ? 1 : ret);
}

void Fl_Resize_Browser::draw() 
{
	// DRAW BROWSER
	Fl_Browser::draw();
	if ( m_showcolsep ) 
	{
		// DRAW COLUMN SEPARATORS
		int colx = this->x() - hposition();
		int X,Y,W,H;
		Fl_Browser::bbox(X,Y,W,H);
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

