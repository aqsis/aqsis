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

#include "zoomimage.h"

#include <float.h>

#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Group.H>

#include "image.h"

namespace Aqsis {

CqZoomImage::CqZoomImage(int x, int y)
	: Fl_Widget(x,y,1,1,0),
	m_scale(1)
{
	setScale(1);
}

void CqZoomImage::draw()
{
	fl_color(FL_DARK1);
	fl_rect(x(), y(), w(), h());
	if(m_image && m_image->displayBuffer())
	{
		fl_draw_image(draw_image_cb, this,
					  x() + m_image->originX()*m_scale,
					  y() + m_image->originY()*m_scale,
					  m_image->displayBuffer()->width()*m_scale,
					  m_image->displayBuffer()->height()*m_scale, 3);
	}
	else
	{
		fl_color(FL_FOREGROUND_COLOR);
		fl_draw("No Image", parent()->x(), parent()->y(),
				parent()->w(), parent()->h(), FL_ALIGN_CENTER, 0, 0);
	}
}

void CqZoomImage::updateImage()
{
	if(m_image)
	{
		w(m_image->frameWidth()*m_scale);
		h(m_image->frameHeight()*m_scale);
		parent()->damage(FL_DAMAGE_ALL);
	}
}

void CqZoomImage::setScale(int newScale)
{
	if(newScale >= 1 && newScale < 100)
	{
		int scaleCenterX = Fl::event_x();
		int scaleCenterY = Fl::event_y();
		double ratio = double(newScale)/m_scale;
		x( static_cast<int>(scaleCenterX - ratio*(scaleCenterX - x())) );
		y( static_cast<int>(scaleCenterY - ratio*(scaleCenterY - y())) );
		m_scale = newScale;
		updateImage();
	}
}

void CqZoomImage::incScale(int scaleIncr)
{
	setScale(m_scale + scaleIncr);
}

int CqZoomImage::handle(int event)
{
	switch(event)
	{
		case FL_FOCUS:
			return 1;
		case FL_UNFOCUS:
			return 1;
		case FL_SHORTCUT:
		case FL_KEYDOWN:
			{
				int key = Fl::event_key();
				switch(key)
				{
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
					case '7':
					case '8':
					case '9':
						setScale(key - '1' + 1);
						return 1;
					case '-':
						incScale(-1);
						return 1;
					case '=':
						incScale(1);
						return 1;
					default:
						return 0;
				}
			}
			break;
		case FL_MOUSEWHEEL:
			{
				incScale(-Fl::event_dy());
				return 1;
			}
			break;
	}
	return Fl_Widget::handle(event);
}

void CqZoomImage::setImage(const boost::shared_ptr<CqImage>& image)
{
	m_image = image;
	updateImage();
}

boost::shared_ptr<CqImage> CqZoomImage::image() const
{
	return m_image;
}

void CqZoomImage::update(int X, int Y, int W, int H)
{
	if(W < 0 || H < 0 || X < 0 || Y < 0)
	{
		updateImage();
		damage(FL_DAMAGE_ALL);
	}
	else
	{
		damage(FL_DAMAGE_ALL, x() + (X*m_scale), y() + (Y*m_scale),
				W*m_scale, H*m_scale);
	}
}

namespace {
inline void fillPixel(const uchar* buf, uchar* outBuf, int depth)
{
	for(int i = 0; i < depth; ++i)
		outBuf[i] = buf[i];
}
}

/** \brief Fill an on-screen scanline with image data
 *
 * This function is called to fill a displayed scanline with image data.  It
 * copies data from the underlying image buffer, duplicating it where necessary
 * in order to perform the zooming operation.
 *
 * \param x - x-coordinate of the on-screen image
 * \param y - y-coordinate of the on-screen image
 * \param w - number of pixels to fill in the x-direction
 * \param outBuf - buffer for destination pixels.
 */
void CqZoomImage::fillScanline(int x, int y, int w, uchar* outBuf)
{
	if(m_image->isZBuffer())
	{
		const TqFloat clipNear = m_image->clippingNear();
		const TqFloat clipFar = m_image->clippingFar();
		const TqFloat invRange = 1/(clipFar - clipNear);
		boost::shared_ptr<const CqMixedImageBuffer> buf = m_image->imageBuffer();
		const TqFloat* bufData = reinterpret_cast<const TqFloat*>(buf->rawData());
		bufData += (buf->width()*(y/m_scale));
		for(int i = 0; i < w; ++i)
		{
			float z = bufData[(x+i)/m_scale];
			uchar d = static_cast<uchar>(
					clamp<TqFloat>(255*(1-invRange*(z - clipNear)), 0, 255) );
			outBuf[0] = d;
			outBuf[1] = d;
			outBuf[2] = d;
			outBuf += 3;
		}
	}
	else
	{
		boost::shared_ptr<const CqMixedImageBuffer> buf = m_image->displayBuffer();
		if(!buf)
			return;
		const uchar* bufData = buf->rawData();
		int depth = buf->channelList().numChannels();
		bufData += (buf->width()*(y/m_scale))*depth;
		for(int i = 0; i < w; ++i)
		{
			fillPixel(bufData + (x+i)/m_scale*depth, outBuf, depth);
			outBuf += depth;
		}
	}
}

void CqZoomImage::draw_image_cb(void* self, int x, int y, int w, uchar* outBuf)
{
	static_cast<CqZoomImage*>(self)->fillScanline(x, y, w, outBuf);
}

} // namespace Aqsis
