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
#include <FL/Fl_Menu_Button.H>

#include	"aqsis.h"
#include	"image.h"
#include 	<boost/shared_ptr.hpp>
#include	<boost/thread/mutex.hpp>

/** FLTK Widget used to show a constantly updating image.
 *
 */
class Fl_FrameBuffer_Widget : public Fl_Widget
{
	public:
		Fl_FrameBuffer_Widget(int x, int y, int imageW, int imageH, boost::shared_ptr<Aqsis::CqImage>& image) : Fl_Widget(x,y,imageW,imageH)
		{
			m_image = image;
		}

		void setImage(boost::shared_ptr<Aqsis::CqImage>& image)
		{
			m_image = image;
		}

		void draw(void);

	private:
		boost::shared_ptr<Aqsis::CqImage> m_image;
};

START_NAMESPACE( Aqsis )

//---------------------------------------------------------------------
/** \class CqFramebuffer
 * Class encapsulating the framebuffer window.
 */

class CqFramebuffer
{
public:
    CqFramebuffer( TqUlong width, TqUlong height, TqInt depth, const std::string& bookName );
    ~CqFramebuffer();

	void show();

	void connect(boost::shared_ptr<CqImage>& image);
	void disconnect();

	boost::shared_ptr<CqImage>& image()
	{
		return( m_associatedImage );
	}
	const boost::shared_ptr<CqImage>& image() const
	{
		return( m_associatedImage );
	}

	void queueResize();
	void resize();
	void update(int X = -1, int Y = -1, int W = -1, int H = -1);
	void checkResize();

	boost::mutex& mutex()
	{
		return(m_mutex);
	}

private:
	Fl_Window*	m_theWindow;
	Fl_FrameBuffer_Widget* m_uiImageWidget;
	Fl_RGB_Image*	m_uiImage;
	Fl_Menu_Button* m_popupMenu;
	static Fl_Menu_Item m_popupMenuItems[];
	bool	m_doResize;

	boost::shared_ptr<CqImage>	m_associatedImage;
	boost::mutex	m_mutex;
	std::string		m_title;
	std::string		m_bookName;
};

END_NAMESPACE( Aqsis )

#endif	// FRAMEBUFFER_H_INCLUDED
