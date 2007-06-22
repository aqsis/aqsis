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
		\brief Implements the basic framebuffer functionality.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include "framebuffer.h"
#include "image.h"

#include "boost/bind.hpp"


void Fl_FrameBuffer_Widget::draw(void)
{
	Fl::lock();
	if(m_image)
		fl_draw_image(m_image,x(),y(),m_width,m_height,m_depth,m_width*m_depth); // draw image
	Fl::unlock();
}


START_NAMESPACE( Aqsis )

Fl_Menu_Item CqFramebuffer::m_popupMenuItems[] = {
  {"Red",       FL_ALT+'r'},
  {"Green",     FL_ALT+'g'},
  {"Blue",      FL_ALT+'b'},
  {"Strange",   FL_ALT+'s', 0, 0, FL_MENU_INACTIVE},
  {"&Charm",    FL_ALT+'c'},
  {"Truth",     FL_ALT+'t'},
  {"Beauty",    FL_ALT+'b'},
  {0}
};

CqFramebuffer::CqFramebuffer(TqUlong width, TqUlong height, TqInt depth)
{
	Fl::lock();
	m_theWindow = new Fl_Window(width, height);
	m_uiImageWidget = new Fl_FrameBuffer_Widget(0,0, width, height, depth, 0);
	m_theWindow->resizable(m_uiImageWidget);
//	m_theWindow->label(thisClient.m_image.m_filename.c_str());
	Fl::visual(FL_RGB);
	m_popupMenu = new Fl_Menu_Button(0,0,width, height, "");
	m_popupMenu->type(Fl_Menu_Button::POPUP3);
	m_popupMenu->box(FL_NO_BOX);
	m_popupMenu->menu(m_popupMenuItems); 
	m_theWindow->resizable(m_popupMenu);
	m_theWindow->end();
	m_theWindow->show();
	Fl::unlock();
}

CqFramebuffer::~CqFramebuffer()
{
	disconnect();
	m_theWindow->hide();
	delete m_theWindow;
}

void CqFramebuffer::show()
{
	m_theWindow->show();
}

void CqFramebuffer::connect(boost::shared_ptr<CqImage>& image)
{
	disconnect();
	m_associatedImage = image;	
	Fl::lock();
	m_uiImageWidget->setImageData(image->data());
	m_uiImageWidget->setImageProportions(image->frameWidth(), image->frameHeight(), image->numChannels());
	m_theWindow->size(image->frameWidth(), image->frameHeight());
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
	m_associatedImage.reset();
	m_uiImageWidget->setImageData(0);
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
	//Fl::check();
}

END_NAMESPACE( Aqsis )
