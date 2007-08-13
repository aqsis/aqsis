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
#include	"fluid_piqsl_ui.h"

void Fl_FrameBuffer_Widget::draw()
{
	Fl::lock();
	boost::shared_ptr<const Aqsis::CqImageBuffer> buf;
	if(m_image && (buf = m_image->displayBuffer()))
	{
		fl_draw_image(buf->rawData().get(), x(), y(),
			buf->width(), buf->height(),
			buf->numChannels(),
			buf->width()*buf->numChannels()); // draw image
	}
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

const int CqFramebuffer::defaultWidth = 400;
const int CqFramebuffer::defaultHeight = 300;

CqFramebuffer::CqFramebuffer(TqUlong width, TqUlong height, TqInt depth,
		const std::string& bookName) : 
	Fl_Double_Window(width+20, height, bookName.c_str()),
	m_doResize(false), m_bookName(bookName), m_keyHeld(false),
	m_title(bookName)
{
	Fl::lock();
	size_range(400, 300); // restrict min size
	// I was going to make a toolbar, but I decided to disable it for now
//	m_theWindow = new Fl_Window(width, height+16);
	
//	Fl_Pack *vpack = new Fl_Pack(0, 0, width, height);
//	vpack->type(Fl_Pack::VERTICAL);

//	Fl_Pack *toolbar_pck = new Fl_Pack(0,0, width, 30);
//	toolbar_pck->type(Fl_Pack::HORIZONTAL);
//	Fl_Button *but1 = new Fl_Button(0,0, 100,30, "Red");
//	but1->image(new Fl_PNG_Image("red_chan.png"));
//	Fl_Button *but2 = new Fl_Button(0,0, 100,30, "Green");
//	but2->image(new Fl_PNG_Image("green_chan.png"));
//	Fl_Button *but3 = new Fl_Button(0,0, 100,30, "Blue");
//	but3->image(new Fl_PNG_Image("blue_chan.png"));
//	toolbar_pck->end();
//	toolbar_pck->resizable(toolbar_pck);
	boost::shared_ptr<CqImage> t;
	m_scroll = new Fl_Scroll(0, 0, width+20, height);
	m_uiImageWidget = new Fl_FrameBuffer_Widget(0,0, width, height, t);
	m_scroll->end();

	Fl::visual(FL_RGB);
	m_popupMenu = new Fl_Menu_Button(0,0,width, height, "");
	m_popupMenu->type(Fl_Menu_Button::POPUP3);
	m_popupMenu->box(FL_NO_BOX);
	m_popupMenu->menu(m_popupMenuItems); 

	resizable(m_scroll);

	end();
	Fl_Window::show();
	Fl::unlock();
}

CqFramebuffer::~CqFramebuffer()
{
	Fl::lock();
	disconnect();
	//hide();
	//delete m_theWindow;
	Fl::unlock();
}

// Event handler for the framebuffer
int CqFramebuffer::handle(int event)
{
	switch(event)
	{
		case FL_FOCUS:
			return 1;
		case FL_UNFOCUS:
			return 1;
		case FL_KEYDOWN:
			if (!m_keyHeld)
			{
				m_keyHeld = true;
				//std::cout << "Key: " << Fl::event_key() << " down" << std::endl;
				switch (Fl::event_key())
				{
					case 'r':
						//std::cout << "Red channel toggle" << std::endl;
						return 1;
				}
			}
			break;

		case FL_KEYUP:
			m_keyHeld = false;
			switch (Fl::event_key())
			{
				case 'r':
					//std::cout << "R up" << std::endl;
					return 1;
			}
			break;

		case FL_PUSH:
			switch (Fl::event_button())
			{
				case FL_MIDDLE_MOUSE:
					// pan
					m_lastPos[0] = Fl::event_x();
					m_lastPos[1] = Fl::event_y();
					return 1;

			}
			break;
		case FL_RELEASE:
			switch (Fl::event_button())
			{
				case FL_MIDDLE_MOUSE:
					return 1;
			}
			break;
		case FL_DRAG:
			switch (Fl::event_button())
			{
				case FL_MIDDLE_MOUSE:
					int dx = Fl::event_x() - m_lastPos[0];
					int dy = Fl::event_y() - m_lastPos[1];
					//m_scroll->position(m_scroll->xposition() + dx,
					//		m_scroll->yposition() + dy);
					m_uiImageWidget->position(m_uiImageWidget->x() + dx,
							m_uiImageWidget->y()+dy);
					m_scroll->redraw();
					m_lastPos[0] = Fl::event_x();
					m_lastPos[1] = Fl::event_y();
					return 1;
			}
			break;
	}
	return Fl_Window::handle(event);
}

void CqFramebuffer::show()
{
	Fl::lock();
	Fl_Window::show();
	Fl::unlock();
}

void CqFramebuffer::connect(boost::shared_ptr<CqImage>& image)
{
	disconnect();
	m_associatedImage = image;	
	Fl::lock();
	m_uiImageWidget->setImage(image);
	queueResize();

	// update window title

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
	//label("");
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
	int fw = defaultWidth;
	int fh = defaultHeight;
	if(m_associatedImage)
	{
		if(m_associatedImage->frameWidth() > 0 && m_associatedImage->frameHeight() > 0)
		{
			fw = m_associatedImage->frameWidth();
			fh = m_associatedImage->frameHeight();
		}
		title << ": " << m_associatedImage->name();
	}
	m_uiImageWidget->size(fw, fh);
	redraw();

	m_title = title.str();
	label(m_title.c_str());
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
