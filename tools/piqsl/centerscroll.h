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


#ifndef CENTERSCROLL_H_INCLUDED
#define CENTERSCROLL_H_INCLUDED

#include <aqsis/aqsis.h>

#include <FL/Fl_Scroll.H>

#include "zoomimage.h"

namespace Aqsis {

/** \brief A scrollable pane which responds to mouse drags and clicks.
 */
class CqCenterScroll : public Fl_Scroll
{
public:
	CqCenterScroll(int x, int y, int w, int h, const char* l = 0);
	/// FLTK event handling
	virtual int handle(int event);
	/// FLTK widget resize
	virtual void resize(int x, int y, int w, int h);

	/// Center the image.
	void centerImageWidget();

	/// Set the image of the underlying zoom widget.
	void setImage(const boost::shared_ptr<CqImage>& image);
	/// Get the underlying image
	boost::shared_ptr<CqImage> image();
	
	/// Resize the underlying zoomable image widget
	void resizeImageWidget(int w, int h);

	/// Inform the zoomable image that a section has been changed and needs to
	/// be redrawn.
	void update(int X, int Y, int W, int H);

private:
	CqZoomImage m_imageWidget;
	int m_prevDragX;
	int m_prevDragY;
};

//-----------------------------------------------------------------------

} // namespace Aqsis

#endif // CENTERSCROLL_H_INCLUDED
