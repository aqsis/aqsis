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
	fl_color(FL_BACKGROUND_COLOR);
	fl_rectf(x(), y(), w(), h());
	if(m_image)
	{
		boost::shared_ptr<const Aqsis::CqMixedImageBuffer> buf = m_image->displayBuffer();
		if(buf)
		{
			fl_draw_image(buf->rawData(),
				x()+m_image->originX(), y()+m_image->originY(),
				buf->width(), buf->height(),
				buf->channelList().numChannels(),
				0); // draw image
		}
	}
	else
	{
		fl_color(FL_FOREGROUND_COLOR);
		fl_draw("No Image", x(), y(), w(), h(), FL_ALIGN_CENTER, 0, 0);
	}
	Fl::unlock();
}

extern CqPiqslMainWindow* window;

namespace Aqsis {

void piqsl_cb(Fl_Widget* w, void* v);

Fl_Menu_Item CqFramebuffer::m_popupMenuItems[] = {
  {"Open &Piqsl",       FL_ALT+'p', (Fl_Callback*)piqsl_cb },
  {0}
};


void piqsl_cb(Fl_Widget* /*w*/, void* /*v*/)
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
	m_doResize(false),
	m_title(bookName),
	m_bookName(bookName),
	m_keyHeld(false)
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
	m_scroll->color(FL_LIGHT1);
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
			{
				boost::shared_ptr<Aqsis::CqImage>& image
					= m_uiImageWidget->image();
				int key = Fl::event_key();
				if(Fl::event_ctrl())
				{
					switch(key)
					{
						case FL_KP + '+':
							incSubImage(true);
							return 1;
						case FL_KP + '-':
							incSubImage(false);
							return 1;
					}
				}
				else if(Fl::event_shift())
				{
					switch(key)
					{
						case 'r':
							if(image)
								image->reloadFromFile();
							return 1;
					}
				}
				else
				{
					switch(key)
					{
						case 'h':
							// 'Home' widget back to center
							centerImageWidget();
							m_scroll->redraw();
							return 1;
						case FL_KP + '+':
							incZoom(1);
							return 1;
						case FL_KP + '-':
							incZoom(-1);
							return 1;
					}
				}
			}
			break;
		case FL_KEYUP:
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
					fl_cursor(FL_CURSOR_DEFAULT, FL_FOREGROUND_COLOR,
							FL_BACKGROUND_COLOR);
					return 1;
			}
			break;
		case FL_DRAG:
			switch (Fl::event_button())
			{
				case FL_MIDDLE_MOUSE:
					fl_cursor(FL_CURSOR_MOVE, FL_FOREGROUND_COLOR,
							FL_BACKGROUND_COLOR);
					int dx = Fl::event_x() - m_lastPos[0];
					int dy = Fl::event_y() - m_lastPos[1];
					m_uiImageWidget->position(m_uiImageWidget->x() + dx,
							m_uiImageWidget->y()+dy);
					m_scroll->redraw();
					m_lastPos[0] = Fl::event_x();
					m_lastPos[1] = Fl::event_y();
					return 1;
			}
			break;
		case FL_MOUSEWHEEL:
			{
				if(Fl::event_ctrl())
					incSubImage(Fl::event_dy() < 0);
				else 
					incZoom(-Fl::event_dy());
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

void CqFramebuffer::resize(int X, int Y, int W, int H)
{
	// is the window actually getting resized? (might just be moved)
	bool isResizing = W != w() || H != h();
	// call parent's resize()
	Fl_Double_Window::resize(X, Y, W, H);
	if(isResizing)
		centerImageWidget();
}

void CqFramebuffer::centerImageWidget()
{
	m_uiImageWidget->position(
			(m_scroll->w() - m_uiImageWidget->w())/2,
			(m_scroll->h() - m_uiImageWidget->h())/2);
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
	// Resize the window if it's too small, leave it if it's big enough or bigger.
	if( w() < fw || h() < fh )
		resize(x(), y(), min(fw, static_cast<int>(Fl::w()*0.9)), min(fh, static_cast<int>(Fl::h()*0.9)));
	centerImageWidget();
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
		m_uiImageWidget->damage(FL_DAMAGE_SCROLL);
	else
		m_uiImageWidget->damage(FL_DAMAGE_SCROLL, m_uiImageWidget->x() + X,
				m_uiImageWidget->y() + Y, W, H);
	Fl::awake();
	Fl::unlock();
}

/// \note: This should only ever be called from the main thread.
void CqFramebuffer::checkResize()
{
	if(m_doResize)
		resize();
}

void CqFramebuffer::incZoom(TqInt increment)
{
	boost::shared_ptr<Aqsis::CqImage>& image = m_uiImageWidget->image();
	if(!image)
		return;

	TqInt newZoom = image->zoom() + increment;
	// Only zoom if the new image will be "small enough".
	/// \todo This restriction should be lifted once we fix the zoom support with a proper image zooming widget.
	if( newZoom > 0 && (increment < 0
		|| newZoom * image->imageWidth() * image->imageHeight() < 8000*8000) )
	{
		image->setZoom(newZoom);
		m_scroll->redraw();
		queueResize();
	}
}

void CqFramebuffer::incSubImage(bool increase)
{
	boost::shared_ptr<Aqsis::CqImage>& image = m_uiImageWidget->image();
	if(!image)
		return;

	// Reset zoom level for safety.
	/// \todo Fix this when we get a proper image zooming widget!
	image->setZoom(1);

	if(increase)
		image->loadNextSubImage();
	else
		image->loadPrevSubImage();
	m_scroll->redraw();
	queueResize();
}


} // namespace Aqsis
