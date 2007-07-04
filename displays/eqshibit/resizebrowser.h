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
		\brief Declare a class extending Fl_Browser with resizable columns.
		Based on original code from Greg Ercolano (http://seriss.com/people/erco/)
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is ddclient.h included already?
#ifndef RESIZE_BROWSER_H_INCLUDED
#define RESIZE_BROWSER_H_INCLUDED 1

#include "book.h"

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Browser.H>
#include <FL/fl_draw.H>

#include <boost/shared_ptr.hpp>
//
// Demonstrate how to derive a class extending Fl_Browser with interactively resizable columns
// erco 1.10 12/09/2005
//
class Fl_Resize_Browser : public Fl_Browser 
{
public:
    Fl_Resize_Browser(int X,int Y,int W,int H,const char*L=0) : Fl_Browser(X,Y,W,H,L) {
        m_colsepcolor = Fl_Color(FL_GRAY);
        m_last_cursor = FL_CURSOR_DEFAULT;
        m_showcolsep  = 0;
        m_dragging    = 0;
        m_nowidths[0] = 0;
        m_widths      = m_nowidths;
    }

    Fl_Color colsepcolor() const 
	{
        return(m_colsepcolor);
    }

    void colsepcolor(Fl_Color val) 
	{
        m_colsepcolor = val;
    }
    // GET/SET DISPLAY OF COLUMN SEPARATOR LINES
    //     1: show lines, 0: don't show lines
    //
    int showcolsep() const 
	{
        return(m_showcolsep);
    }
    void showcolsep(int val) 
	{
        m_showcolsep = val;
    }
    // GET/SET COLUMN WIDTHS ARRAY
    //    Just like fltk method, but array is non-const.
    //
    int *column_widths() const 
	{
        return(m_widths);
    }
    void column_widths(int *val) 
	{
        m_widths = val;
        Fl_Browser::column_widths(val);
    }

	// Locally overridden Fl_Browser_ functions to manage the 
	// item list.
	void* item_first() const;
	void* item_next(void*) const;
	void* item_prev(void*) const;
	void item_draw(void* v, int X, int Y, int W, int H) const;
  int item_selected(void*) const ;
  void item_select(void*, int);
  int item_height(void*) const ;
  int item_width(void*) const ;

protected:
    // MANAGE EVENTS TO HANDLE COLUMN RESIZING
    int handle(int e);
    void draw();

private:
    Fl_Color  m_colsepcolor;	// color of column separator lines 
    int       m_showcolsep;	// flag to enable drawing column separators
    Fl_Cursor m_last_cursor;	// saved cursor state info
    int       m_dragging;	// 1=user dragging a column
    int       m_dragcol;	// col# user is currently dragging
    int      *m_widths;		// pointer to user's width[] array
    int       m_nowidths[1];	// default width array (non-const)
	boost::shared_ptr<Aqsis::CqBook>	m_theBook;
    // CHANGE CURSOR
    //     Does nothing if cursor already set to value specified.
    //
    void change_cursor(Fl_Cursor newcursor) 
	{
        if ( newcursor != m_last_cursor ) 
		{
            fl_cursor(newcursor, FL_BLACK, FL_WHITE);
            m_last_cursor = newcursor;
        }
    }
    // RETURN THE COLUMN MOUSE IS 'NEAR'
    //     Returns -1 if none.
    //
    int which_col_near_mouse() 
	{
        int X,Y,W,H;
        Fl_Browser::bbox(X,Y,W,H);		// area inside browser's box()
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


};

#endif //RESIZE_BROWSER_H_INCLUDED 1
