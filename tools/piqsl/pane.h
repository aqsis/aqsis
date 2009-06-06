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


#ifndef PANE_H_INCLUDED
#define PANE_H_INCLUDED

#include <aqsis/aqsis.h>

#include <FL/Fl_Group.H>
#include <FL/Fl_Box.H>

#include <boost/function.hpp>

#include "bookbrowser.h"
#include "centerscroll.h"

namespace Aqsis {

//------------------------------------------------------------------------------
/// Divider box for the two regions of a CqPane
class CqPaneDividerBox : public Fl_Box
{
public:
	CqPaneDividerBox(int x, int y, int w, int h, const char* l = 0);

	/// Set the callback used when the box is dragged.
	void setDragCallback(const boost::function<void (int)>& fxn);

	/// FLTK-required widget drawing function.
	void draw();

	/// Handle a FLTK event
	int handle(int event);

	/// Resize the box
	void resize(int x, int y, int w, int h);

private:
	int m_prevDragX;
	boost::function<void (int)> m_dragCallback;
};


//------------------------------------------------------------------------------
/** \brief A fltk widget representing a resizeable pane with vertical seperator
 */
class CqPane : public Fl_Group
{
	public:
		/** \brief Construct a pane with the given position and size.
		 *
		 * \param x,y - position
		 * \param w,h - width & height
		 * \param l - label
		 */
		CqPane(int x, int y, int w, int h, const char* l = 0);
		/** \brief Construct a pane with the given position, size and divider position.
		 *
		 * \param x,y - position
		 * \param w,h - width & height
		 * \param divPos - position of the divider as a fraction of the width.
		 * \param l - label
		 */
		CqPane(int x, int y, int w, int h, double divPos, const char* l = 0);

		/// Collapse the left region down to zero width.
		void collapse1();
		/// Restore the previously collapsed region.
		void uncollapse();
		/// Add a widget to the left half of the pane
		void add1(CqBookBrowser* widget);
		/// Add a widget to the right half of the pane
		void add2(CqCenterScroll* widget);

		CqBookBrowser* browser();
		CqCenterScroll* centerScroll();

		/// Fltk widget resize method.
		virtual void resize(int x, int y, int w, int h);

	private:
		void moveDivider(int dx);
		void init();

		CqPaneDividerBox m_divider;
		int m_prevDividerPos;
		CqBookBrowser* m_browser;
		CqCenterScroll* m_centerScroll;
};


} // namespace Aqsis

#endif // PANE_H_INCLUDED
