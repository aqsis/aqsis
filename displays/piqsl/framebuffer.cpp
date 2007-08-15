// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
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
		\brief Implements the basic framebuffer functionality.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#include	<boost/bind.hpp>

#include	"framebuffer.h"
#include	"image.h"
#include	"piqsl_ui.h"

void Fl_FrameBuffer_Widget::draw()
{
	Fl::lock();
	if(m_image)
		fl_draw_image(m_image->displayData().get(),x(),y(),
			m_image->imageWidth(),m_image->imageHeight(),
			m_image->numChannels(),
			m_image->imageWidth()*m_image->numChannels()); // draw image
	else
	{
		fl_draw_box(FL_FLAT_BOX, x(), y(), w(), h(), FL_BACKGROUND_COLOR);
		fl_color(FL_FOREGROUND_COLOR);
		fl_draw("No Image", x(), y(), w(), h(), FL_ALIGN_CENTER, 0, 0);
	}
	Fl::unlock();
}

extern CqPiqslMainWindow* window;

START_NAMESPACE( Aqsis )

void piqsl_cb(Fl_Widget* w, void* v);

Fl_Menu_Item CqFramebuffer::m_popupMenuItems[] = {
  {"Open &Piqsl",       FL_ALT+'p', (Fl_Callback*)piqsl_cb },
  {0}
};


void piqsl_cb(Fl_Widget* w, void* v)
{
	Fl::lock();
	if(window)
		window->show();
	Fl::unlock();
}

CqFramebuffer::CqFramebuffer(TqUlong width, TqUlong height, TqInt depth, const std::string& bookName) : m_doResize(false), m_bookName(bookName)
{
	Fl::lock();
	m_theWindow = new Fl_Window(width, height);
	boost::shared_ptr<CqImage> t;
	m_uiImageWidget = new Fl_FrameBuffer_Widget(0,0, width, height, t);
	m_theWindow->resizable(m_uiImageWidget);
	Fl::visual(FL_RGB);
	m_popupMenu = new Fl_Menu_Button(0,0,width, height, "");
	m_popupMenu->type(Fl_Menu_Button::POPUP3);
	m_popupMenu->box(FL_NO_BOX);
	m_popupMenu->menu(m_popupMenuItems); 
	m_theWindow->resizable(m_popupMenu);
	m_title = bookName;
	m_theWindow->label(m_title.c_str());
	m_theWindow->end();
	m_theWindow->show();
	Fl::unlock();
}

CqFramebuffer::~CqFramebuffer()
{
	Fl::lock();
	disconnect();
	m_theWindow->hide();
	delete m_theWindow;
	Fl::unlock();
}

void CqFramebuffer::show()
{
	Fl::lock();
	m_theWindow->show();
	Fl::unlock();
}

void CqFramebuffer::connect(boost::shared_ptr<CqImage>& image)
{
	disconnect();
	m_associatedImage = image;	
	Fl::lock();
	m_uiImageWidget->setImage(image);
	queueResize();
	boost::function<void(int,int,int,int)> f;
	f = boost::bind(&CqFramebuffer::update, this, _1, _2, _3, _4);
	image->setUpdateCallback(f);
	Fl::unlock();
}

void CqFramebuffer::disconnect()
{
	Fl::lock();
	if(m_associatedImage)
	{
		boost::function<void(int,int,int,int)> f;
		m_associatedImage->setUpdateCallback(f);
	}
	m_theWindow->label("");
	boost::shared_ptr<CqImage> t;
	m_associatedImage = t;
	m_uiImageWidget->setImage(t);
	queueResize();
	Fl::unlock();
}

void CqFramebuffer::queueResize()
{
	Fl::lock();
	m_doResize = true;
	Fl::unlock();
}

void CqFramebuffer::resize()
{
	Fl::lock();
	std::stringstream title;
	title << m_bookName;
	int fw = 100;
	int fh = 100;
	if(m_associatedImage)
	{
		if(m_associatedImage->frameWidth() > 0 && m_associatedImage->frameHeight() > 0)
		{
			fw = m_associatedImage->frameWidth();
			fh = m_associatedImage->frameHeight();
		}
		title << ":" << m_associatedImage->name();
	}
	m_theWindow->size(fw, fh);

	m_title = title.str();
	m_theWindow->label(m_title.c_str());
	m_doResize = false;
	Fl::unlock();
}

void CqFramebuffer::update(int X, int Y, int W, int H)
{
	Fl::lock();
	if(W < 0 || H < 0 || X < 0 || Y < 0)
		m_uiImageWidget->damage(1);
	else
		m_uiImageWidget->damage(1, X, Y, W, H);
	Fl::awake();
	Fl::unlock();
}

/// \note: This should only ever be called from the main thread.
void CqFramebuffer::checkResize()
{
	if(m_doResize)
		resize();
}

END_NAMESPACE( Aqsis )
