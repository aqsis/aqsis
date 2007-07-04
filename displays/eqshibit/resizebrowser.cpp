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
#include <string.h>

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

struct FL_BLINE {	// data is in a linked list of these
  FL_BLINE* prev;
  FL_BLINE* next;
  void* data;
  short length;		// sizeof(txt)-1, may be longer than string
  char flags;		// selected, displayed
  char txt[1];		// start of allocated array
};

void* Fl_Resize_Browser::item_first() const
{
	if(m_theBook && m_theBook->numImages() > 0)
		return((void*)1);
	else
		return((void*)0);
}

void* Fl_Resize_Browser::item_next(void* p) const
{
	long index = (long)p;
	if(index <= 0)
		return(p);
	else
	{
		if(m_theBook && index >= m_theBook->numImages())
			return((void*)0);
		else
			return((void*)(index + 1));
	}
}

void* Fl_Resize_Browser::item_prev(void* p) const
{
	long index = (long)p;
	if(index <= 0)
		return(p);
	else
	{
		if(index <= 1)
			return((void*)0);
		else
			return((void*)(index - 1));
	}
}

int Fl_Resize_Browser::item_selected(void* l) const 
{
	return 0;
}

void Fl_Resize_Browser::item_select(void* l, int v) 
{
}

int Fl_Resize_Browser::item_height(void* lv) const {
  int hmax = 2; // use 2 to insure we don't return a zero!
	char* testString = "Something\there";

//  if (!l->txt[0]) {
    // For blank lines set the height to exactly 1 line!
//    fl_font(textfont(), textsize());
//    int hh = fl_height();
//    if (hh > hmax) hmax = hh;
//  }
//  else 
	{
    const int* i = column_widths();
    // do each column separately as they may all set different fonts:
    for (char* str = testString; str && *str; str++) {
      Fl_Font font = textfont(); // default font
      int tsize = textsize(); // default size
      while (*str==format_char()) {
	str++;
	switch (*str++) {
	case 'l': case 'L': tsize = 24; break;
	case 'm': case 'M': tsize = 18; break;
	case 's': tsize = 11; break;
	case 'b': font = (Fl_Font)(font|FL_BOLD); break;
	case 'i': font = (Fl_Font)(font|FL_ITALIC); break;
	case 'f': case 't': font = FL_COURIER; break;
	case 'B':
	case 'C': strtol(str, &str, 10); break;// skip a color number
	case 'F': font = (Fl_Font)strtol(str,&str,10); break;
	case 'S': tsize = strtol(str,&str,10); break;
	case 0: case '@': str--;
	case '.': goto END_FORMAT;
	}
      }
      END_FORMAT:
      char* ptr = str;
      if (ptr && *i++) str = strchr(str, column_char());
      else str = NULL;
      if((!str && *ptr) || (str && ptr < str)) {
	fl_font(font, tsize); int hh = fl_height();
	if (hh > hmax) hmax = hh;
      }
      if (!str || !*str) break;
    }
  }

  return hmax; // previous version returned hmax+2!
}


int Fl_Resize_Browser::item_width(void* v) const {
  char* str = "Something\there";
  const int* i = column_widths();
  int ww = 0;

  while (*i) { // add up all tab-seperated fields
    char* e;
    e = strchr(str, column_char());
    if (!e) break; // last one occupied by text
    str = e+1;
    ww += *i++;
  }

  // OK, we gotta parse the string and find the string width...
  int tsize = textsize();
  Fl_Font font = textfont();
  int done = 0;

  while (*str == '@' && str[1] && str[1] != '@') {
    str ++;
    switch (*str++) {
    case 'l': case 'L': tsize = 24; break;
    case 'm': case 'M': tsize = 18; break;
    case 's': tsize = 11; break;
    case 'b': font = (Fl_Font)(font|FL_BOLD); break;
    case 'i': font = (Fl_Font)(font|FL_ITALIC); break;
    case 'f': case 't': font = FL_COURIER; break;
    case 'B':
    case 'C': strtol(str, &str, 10); break;// skip a color number
    case 'F': font = (Fl_Font)strtol(str, &str, 10); break;
    case 'S': tsize = strtol(str, &str, 10); break;
    case '.':
      done = 1;
      break;
    case '@':
      str--;
      done = 1;
    }

    if (done)
      break;
  }

  if (*str == '@' && str[1])
    str ++;

  fl_font(font, tsize);
  return ww + int(fl_width(str)) + 6;
}

void Fl_Resize_Browser::item_draw(void* v, int X, int Y, int W, int H) const 
{
	char* testStr = "Something\tHere";
	char* str = new char[strlen(testStr)+1];
	strcpy(str, testStr);

  const int* i = column_widths();

  while (W > 6) {	// do each tab-seperated field
    int w1 = W;	// width for this field
    char* e = 0; // pointer to end of field or null if none
    if (*i) { // find end of field and temporarily replace with 0
      e = strchr(str, column_char());
      if (e) {*e = 0; w1 = *i++;}
    }
    int tsize = textsize();
    Fl_Font font = textfont();
    Fl_Color lcol = textcolor();
    Fl_Align talign = FL_ALIGN_LEFT;
    // check for all the @-lines recognized by XForms:
    while (*str == format_char() && *++str && *str != format_char()) {
      switch (*str++) {
      case 'l': case 'L': tsize = 24; break;
      case 'm': case 'M': tsize = 18; break;
      case 's': tsize = 11; break;
      case 'b': font = (Fl_Font)(font|FL_BOLD); break;
      case 'i': font = (Fl_Font)(font|FL_ITALIC); break;
      case 'f': case 't': font = FL_COURIER; break;
      case 'c': talign = FL_ALIGN_CENTER; break;
      case 'r': talign = FL_ALIGN_RIGHT; break;
      case 'B': 
//	if (!(((FL_BLINE*)v)->flags & SELECTED)) {
//	  fl_color((Fl_Color)strtol(str, &str, 10));
//	  fl_rectf(X, Y, w1, H);
//	} 
//	else 
		strtol(str, &str, 10);
        break;
      case 'C':
	lcol = (Fl_Color)strtol(str, &str, 10);
	break;
      case 'F':
	font = (Fl_Font)strtol(str, &str, 10);
	break;
      case 'N':
	lcol = FL_INACTIVE_COLOR;
	break;
      case 'S':
	tsize = strtol(str, &str, 10);
	break;
      case '-':
	fl_color(FL_DARK3);
	fl_line(X+3, Y+H/2, X+w1-3, Y+H/2);
	fl_color(FL_LIGHT3);
	fl_line(X+3, Y+H/2+1, X+w1-3, Y+H/2+1);
	break;
      case 'u':
      case '_':
	fl_color(lcol);
	fl_line(X+3, Y+H-1, X+w1-3, Y+H-1);
	break;
      case '.':
	goto BREAK;
      case '@':
	str--; goto BREAK;
      }
    }
  BREAK:
    fl_font(font, tsize);
//    if (((FL_BLINE*)v)->flags & SELECTED)
//      lcol = fl_contrast(lcol, selection_color());
    if (!active_r()) lcol = fl_inactive(lcol);
    fl_color(lcol);
    fl_draw(str, X+3, Y, w1-6, H, e ? Fl_Align(talign|FL_ALIGN_CLIP) : talign, 0, 0);
    if (!e) break; // no more fields...
    *e = column_char(); // put the seperator back
    X += w1;
    W -= w1;
    str = e+1;
  }
	//delete[](str);
}

