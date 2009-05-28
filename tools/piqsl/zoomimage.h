#ifndef ZOOMIMAGE_H_INCLUDED
#define ZOOMIMAGE_H_INCLUDED

#include <FL/Fl.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/fl_draw.H>

#include <iostream>

#include "image.h"

namespace Aqsis {

class CqZoomImage : public Fl_Widget
{
public:
	CqZoomImage(int x, int y)
		: Fl_Widget(x,y,1,1,0),
		m_scale(1)
	{
		setScale(1);
	}
	void draw()
	{
		if(m_image && m_image->displayBuffer())
		{
			fl_draw_image(draw_image_cb, this, x(),y(), m_image->displayBuffer()->width()*m_scale,
					m_image->displayBuffer()->height()*m_scale, m_image->displayBuffer()->channelList().numChannels());
		}
		else
		{
			fl_color(FL_FOREGROUND_COLOR);
			fl_draw("No Image", x(), y(), parent()->w(), parent()->h(), FL_ALIGN_CENTER, 0, 0);
		}
	}

	void updateImage()
	{
		if(m_image)
		{
			w(m_image->imageWidth()*m_scale);
			h(m_image->imageHeight()*m_scale);
			parent()->damage(FL_DAMAGE_ALL);
		}
	}

	void setScale(int newScale)
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
	void incScale(int scaleIncr)
	{
		setScale(m_scale + scaleIncr);
	}

	int handle(int event)
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

	void setImage(const boost::shared_ptr<CqImage>& image)
	{
		m_image = image;
		updateImage();
	}

	boost::shared_ptr<CqImage> image() const
	{
		return m_image;
	}

	void update(int X, int Y, int W, int H)
	{
		if(W < 0 || H < 0 || X < 0 || Y < 0)
		{
			updateImage();
			damage(FL_DAMAGE_ALL);
		}
		else
			damage(FL_DAMAGE_ALL, x() + (X*m_scale), y() + (Y*m_scale), W*m_scale, H*m_scale);
	}
private:
	boost::shared_ptr<CqImage> m_image;
	int m_scale;

	inline void fillPixel(const uchar* buf, uchar* outBuf, int depth)
	{
		for(int i = 0; i < depth; ++i)
			outBuf[i] = buf[i];
	}
	void fillScanline(int x, int y, int w, uchar* outBuf)
	{
		boost::shared_ptr<const Aqsis::CqMixedImageBuffer> buf = m_image->displayBuffer();
		if(buf)
		{
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
	static void draw_image_cb(void* self, int x, int y, int w, uchar* outBuf)
	{
		static_cast<CqZoomImage*>(self)->fillScanline(x, y, w, outBuf);
	}
};

//-----------------------------------------------------------------------

} // namespace Aqsis

#endif // ZOOMIMAGE_H_INCLUDED
