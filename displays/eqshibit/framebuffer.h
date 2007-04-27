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
		\brief Declare the class for a basic framebuffer window.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is framebuffer.h included already?
#ifndef FRAMEBUFFER_H_INCLUDED
#define FRAMEBUFFER_H_INCLUDED 1

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Image.H>
#include <FL/fl_draw.H>

#include	"aqsis.h"
#include 	<boost/shared_ptr.hpp>

/** FLTK Widget used to show a constantly updating image.
 *
 */
class Fl_FrameBuffer_Widget : public Fl_Widget
{
	public:
		Fl_FrameBuffer_Widget(int x, int y, int imageW, int imageH, int depth, unsigned char* imageD) : Fl_Widget(x,y,imageW,imageH)
		{
			w = imageW;
			h = imageH;
			d = depth;
			image = imageD;
		}

		void setImageData(unsigned char* data)
		{
			image = data;
		}

		void draw(void);

	private:
		int w,h,d;
		unsigned char* image;
};

START_NAMESPACE( Aqsis )

class CqImage;

//---------------------------------------------------------------------
/** \class CqFramebuffer
 * Class encapsulating the framebuffer window.
 */

class CqFramebuffer
{
public:
    CqFramebuffer( TqUlong width, TqUlong height, TqInt depth );
    ~CqFramebuffer();

	void show();

	void connect(boost::shared_ptr<CqImage>& image);
	//void connect(boost::shared_ptr<CqConduit<CqImage, CqFramebuffer> >& conduit);
	void disconnect();

	boost::shared_ptr<CqImage>& image()
	{
		return( m_associatedImage );
	}
	const boost::shared_ptr<CqImage>& image() const
	{
		return( m_associatedImage );
	}

	void update(int X, int Y, int W, int H);

private:
	Fl_Window*	m_theWindow;
	Fl_FrameBuffer_Widget* m_uiImageWidget;
	Fl_RGB_Image*	m_uiImage;

	boost::shared_ptr<CqImage>	m_associatedImage;
};

END_NAMESPACE( Aqsis )

#endif	// FRAMEBUFFER_H_INCLUDED
