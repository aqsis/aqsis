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
		\brief Declare a FLTK browser for displaying book contents.
		Based on original code from Greg Ercolano (http://seriss.com/people/erco/) to 
		provide sizeable columns. The class extends Fl_Browser_ and implements the item_*
		functions to read list data directly from the CqBook class that
		is associated with it.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

//? Is bookbrowser.h included already?
#ifndef BOOK_BROWSER_H_INCLUDED
#define BOOK_BROWSER_H_INCLUDED 1

#include "book.h"

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Browser_.H>
#include <FL/fl_draw.H>

#include <boost/shared_ptr.hpp>

namespace Aqsis {


class CqBookBrowser : public Fl_Browser_
{
public:
    CqBookBrowser(int X,int Y,int W,int H,const char*L=0);

    Fl_Color colsepcolor() const;

    void colsepcolor(const Fl_Color val);
    // GET/SET DISPLAY OF COLUMN SEPARATOR LINES
    //     1: show lines, 0: don't show lines
    //
    int showcolsep() const;
    void showcolsep(int val);
    // GET/SET COLUMN WIDTHS ARRAY
    //    Just like fltk method, but array is non-const.
    //
    int *column_widths() const;
    void column_widths(int *val);

	boost::shared_ptr<CqBook>& book();
	void setBook(boost::shared_ptr<CqBook>& book);

	CqBook::TqImageList::size_type currentSelected() const;
	void setCurrentSelected(CqBook::TqImageList::size_type index);

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
	CqBook::TqImageList::size_type m_currentSelected;
    // CHANGE CURSOR
    //     Does nothing if cursor already set to value specified.
    //
    void change_cursor(Fl_Cursor newcursor);
    // RETURN THE COLUMN MOUSE IS 'NEAR'
    //     Returns -1 if none.
    //
    int which_col_near_mouse();
};


// Implementation of inline functions.
inline CqBookBrowser::CqBookBrowser(int X,int Y,int W,int H,const char*L) : 
	Fl_Browser_(X,Y,W,H,L), 
	m_colsepcolor(Fl_Color(FL_GRAY)),
	m_showcolsep(0),
	m_last_cursor(FL_CURSOR_DEFAULT),
	m_dragging(0)
{
	m_nowidths[0] = 0;
	m_widths      = m_nowidths;
	// this is the text color of a deselected tab
	labelcolor(FL_FOREGROUND_COLOR);
	// this is the background color of a deselected tab
	// and deselected list entry
	selection_color(FL_BACKGROUND_COLOR);
	
}


inline Fl_Color CqBookBrowser::colsepcolor() const 
{
	return(m_colsepcolor);
}

inline void CqBookBrowser::colsepcolor(const Fl_Color val) 
{
	m_colsepcolor = val;
}

inline int CqBookBrowser::showcolsep() const 
{
	return(m_showcolsep);
}

inline void CqBookBrowser::showcolsep(int val) 
{
	m_showcolsep = val;
}

inline int *CqBookBrowser::column_widths() const 
{
	return(m_widths);
}

inline void CqBookBrowser::column_widths(int *val) 
{
	m_widths = val;
}


inline boost::shared_ptr<CqBook>& CqBookBrowser::book()
{
	return(m_theBook);
}

inline void CqBookBrowser::setBook(boost::shared_ptr<CqBook>& book)
{
	m_theBook = book;
}

inline CqBook::TqImageList::size_type CqBookBrowser::currentSelected() const
{
	return(m_currentSelected);
}

inline void CqBookBrowser::setCurrentSelected(CqBook::TqImageList::size_type index)
{
	m_currentSelected = index;
	select((void*)index, 1, 1);
}

} // namespace Aqsis

#endif //BOOK_BROWSER_H_INCLUDED 1
