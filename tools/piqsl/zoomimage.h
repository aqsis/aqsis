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
 * \brief A FLTK widget for viewing images
 */

#ifndef ZOOMIMAGE_H_INCLUDED
#define ZOOMIMAGE_H_INCLUDED

#include <aqsis/aqsis.h>

#include <boost/shared_ptr.hpp>
#include <FL/Fl_Widget.H>

#include <iostream>

namespace Aqsis {

class CqImage;

/** A FLTK widget for displaying a (possibly zoomed) image
 */
class CqZoomImage : public Fl_Widget
{
	public:
		/// Construct an image widget at the given location.
		CqZoomImage(int x, int y);

		/// Fltk required widget drawing function
		virtual void draw();
		/// Handle FLTK events.
		virtual int handle(int event);

		/// Set the scale factor for the image (>= 0)
		void setScale(int newScale);
		/// Increment or decrement the current scale factor
		void incScale(int scaleIncr);

		/// Set the image
		void setImage(const boost::shared_ptr<CqImage>& image);
		/// Return the current image
		boost::shared_ptr<CqImage> image() const;

		/// Redraw the entire image.
		void updateImage();
		/// Redraw a region of the currently displayed image
		void update(int X, int Y, int W, int H);

	private:
		boost::shared_ptr<CqImage> m_image;
		int m_scale;

		void fillScanline(int x, int y, int w, uchar* outBuf);
		static void draw_image_cb(void* self, int x, int y, int w, uchar* outBuf);
};

} // namespace Aqsis

#endif // ZOOMIMAGE_H_INCLUDED
