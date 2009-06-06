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
 * \brief Declare the class for a basic framebuffer window.
 */

//? Is framebuffer.h included already?
#ifndef FRAMEBUFFER_H_INCLUDED
#define FRAMEBUFFER_H_INCLUDED 1

#include	<aqsis/aqsis.h>

#include	<FL/Fl.H>
#include	<FL/Fl_Double_Window.H>
#include	<FL/Fl_Box.H>
#include	<FL/Fl_Image.H>
#include	<FL/Fl_Group.H>
#include	<FL/Fl_PNG_Image.H>
#include	<FL/fl_draw.H>
#include	<FL/Fl_Menu_Button.H>
#include	<FL/Fl_Pack.H>
#include	<FL/Fl_Scroll.H>
#include 	<boost/shared_ptr.hpp>
#include	<boost/thread/mutex.hpp>

#include	"image.h"

/** FLTK Widget used to show a constantly updating image.
 *
 */
class Fl_FrameBuffer_Widget : public Fl_Widget
{
	public:
		Fl_FrameBuffer_Widget(int x, int y, int imageW, int imageH, boost::shared_ptr<Aqsis::CqImage>& image);
		virtual ~Fl_FrameBuffer_Widget() {}

		inline void setImage(boost::shared_ptr<Aqsis::CqImage>& image);
		inline boost::shared_ptr<Aqsis::CqImage>& image();

		virtual void draw();

	private:
		boost::shared_ptr<Aqsis::CqImage> m_image;
};


namespace Aqsis {

//---------------------------------------------------------------------
/** \class CqFramebuffer
 * Class encapsulating the framebuffer window.
 */

class CqFramebuffer : public Fl_Double_Window
{
public:
    CqFramebuffer( TqUlong width, TqUlong height, TqInt depth, const std::string& bookName );
    virtual ~CqFramebuffer();

	/** Ensure that the FLTK window for this framebuffer is displayed.
 	 * Brings the window to the front if already displayed.
 	 */
	virtual void show();

	/** Connect a CqImage to this framebuffer for display.
	 * \param image		Shared pointer to the image to display in the framebuffer. A shared pointer will be kept.
	 */
	void connect(boost::shared_ptr<CqImage>& image);
	/** Disconnect any image associated with this framebuffer.
	 */
	void disconnect();

	/** Get a shared pointer to the image that is being displayed in this framebuffer.
	 * \return			A shared pointer to the associated image.
	 */
	boost::shared_ptr<CqImage>& image();
	/** Get a const shared pointer to the image that is being displayed in this framebuffer.
	 * \return			A shared pointer to the associated image.
	 */
	const boost::shared_ptr<CqImage>& image() const;
	/** Get the name of the book that this framebuffer is associated with, if any.
	 * \return			A reference to the book name.
	 */
	const std::string& bookName() const;
	/** Set the name of the book that this framebuffer is linked to.
	 * This is used to set the title of the window, for information purposes only.
	 * \param name		The name of the book that the framebuffer is linked to.
	 */
	void setBookName(const std::string& name);

	/** Queue a request to recalculate the size of this window.
	 * The resize operation can only be performed on the main thread, this allows other threads to force a recalc.
	 */
	void queueResize();
	/** Perform a recalculation of the size of this framebuffer.
	 * Actually recalculate the size of the framebuffer window according to the image being displayed. Can
	 * only be performed in the main thread.
	 */
	void resize();
	/** Overriden Fl_Widget::resize()
	 */
	virtual void resize(int X, int Y, int W, int H);
	/** Perform an update on a region of the framebuffer.
	 * Pass -1 for all values to update the whole framebuffer.
	 * \param X			The X origin of the region (bucket) to update.
	 * \param Y			The Y origin of the region (bucket) to update.
	 * \param W			The width of the region (bucket) to update.
	 * \param H			The height of the region (bucket) to update.
	 */
	void update(int X = -1, int Y = -1, int W = -1, int H = -1);
	/** Check if a resize has been queued and perform it.
	 * \note Must only be called from the main thread.
	 */
	void checkResize();

	/** Get a reference to the unique mutex for this framebuffer.
	 * Used when locking the framebuffer during multithreaded operation.
	 * \return			A reference to the unique mutex for this framebuffer.
	 */
	boost::mutex& mutex();

	/** Event handler for the framebuffer
	 */
	int handle(int event);

	/** Centers the image widget in the scrolled area
	 */
	void centerImageWidget();

	static const int defaultWidth; ///< Default framebuffer width.
	static const int defaultHeight; ///< Default framebuffer height.

private:
	/** \brief Increment the current image zoom level
	 *
	 * \param increment - amount to increase or decrease the current zoom by.
	 */
	void incZoom(TqInt increment);
	/** \brief Modify the current sub image level
	 *
	 * \param increase - If true, increase the sub image index, else decrease it.
	 */
	void incSubImage(bool increase);


	Fl_FrameBuffer_Widget* m_uiImageWidget;			///< The custom image widget.
	Fl_Menu_Button* m_popupMenu;					///< The right click menu widget.
	Fl_Scroll *m_scroll;
	static Fl_Menu_Item m_popupMenuItems[];			///< Static list of menuitems for the popup menu.
	bool	m_doResize;								///< Flag indicating a resize has been requested.

	boost::shared_ptr<CqImage>	m_associatedImage;	///< Shared pointer to the image to display.
	boost::mutex	m_mutex;						///< Unique mutex for this image.
	std::string		m_title;						///< Local string that contains the title for the window.
	std::string		m_bookName;						///< The name of the book, if any, this framebuffer is linked to.
	bool m_keyHeld;
	int m_lastPos[2];
};


// Implementation of inline functions.
inline boost::shared_ptr<CqImage>& CqFramebuffer::image()
{
	return( m_associatedImage );
}

inline const boost::shared_ptr<CqImage>& CqFramebuffer::image() const
{
	return( m_associatedImage );
}

inline const std::string& CqFramebuffer::bookName() const
{
	return(m_bookName);
}


inline void CqFramebuffer::setBookName(const std::string& name)
{
	m_bookName = name;
}


inline boost::mutex& CqFramebuffer::mutex()
{
	return(m_mutex);
}

} // namespace Aqsis

inline Fl_FrameBuffer_Widget::Fl_FrameBuffer_Widget(int x, int y, int imageW,
		int imageH, boost::shared_ptr<Aqsis::CqImage>& image) : 
	Fl_Widget(x,y,imageW,imageH)
{
	m_image = image;
}

inline void Fl_FrameBuffer_Widget::setImage(boost::shared_ptr<Aqsis::CqImage>& image)
{
	m_image = image;
}

inline boost::shared_ptr<Aqsis::CqImage>& Fl_FrameBuffer_Widget::image()
{
	return m_image;
}

#endif	// FRAMEBUFFER_H_INCLUDED
