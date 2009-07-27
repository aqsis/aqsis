#ifndef ZOOMIMAGE_H_INCLUDED
#define ZOOMIMAGE_H_INCLUDED

#include <FL/Fl.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/fl_draw.H>

#include <iostream>

class ZoomImage : public Fl_Widget
{
	private:
		Fl_PNG_Image* m_image;
		int m_scale;

		inline void fillPixel(const uchar* buf, uchar* outBuf, int depth)
		{
			for(int i = 0; i < depth; ++i)
				outBuf[i] = buf[i];
		}
		void fillScanline(int x, int y, int w, uchar* outBuf)
		{
			const uchar* buf = (const uchar*)(m_image->data()[0]);
			int depth = m_image->d();
			int inY = y/m_scale;
			buf += (m_image->w()*(y/m_scale))*depth;
			for(int i = 0; i < w; ++i)
			{
				fillPixel(buf + (x+i)/m_scale*depth, outBuf, depth);
				outBuf += depth;
			}
		}
		static void draw_image_cb(void* self, int x, int y, int w, uchar* outBuf)
		{
			static_cast<ZoomImage*>(self)->fillScanline(x, y, w, outBuf);
		}
	public:
		ZoomImage(int x, int y)
			: Fl_Widget(x,y,1,1,0),
			m_scale(1)
		{
			m_image = new Fl_PNG_Image("test.png");
			setScale(1);
		}
		void draw()
		{
			fl_draw_image(draw_image_cb, this, x(),y(), m_image->w()*m_scale,
					m_image->h()*m_scale, m_image->d());
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
				w(m_image->w()*m_scale);
				h(m_image->h()*m_scale);
				parent()->damage(FL_DAMAGE_ALL);
			}
		}
		void incScale(int scaleIncr)
		{
			setScale(m_scale + scaleIncr);
		}

		int handle(int event)
		{
			char* fltkEventnames[] =
			{
			  "FL_NO_EVENT",
			  "FL_PUSH",
			  "FL_RELEASE",
			  "FL_ENTER",
			  "FL_LEAVE",
			  "FL_DRAG",
			  "FL_FOCUS",
			  "FL_UNFOCUS",
			  "FL_KEYDOWN",
			  "FL_KEYUP",
			  "FL_CLOSE",
			  "FL_MOVE",
			  "FL_SHORTCUT",
			  "FL_DEACTIVATE",
			  "FL_ACTIVATE",
			  "FL_HIDE",
			  "FL_SHOW",
			  "FL_PASTE",
			  "FL_SELECTIONCLEAR",
			  "FL_MOUSEWHEEL",
			  "FL_DND_ENTER",
			  "FL_DND_DRAG",
			  "FL_DND_LEAVE",
			  "FL_DND_RELEASE",
			};
//			std::cout << fltkEventnames[event] << "\n";
			switch(event)
			{
				case FL_FOCUS:
					return 1;
				case FL_UNFOCUS:
					return 1;
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
};

#endif // ZOOMIMAGE_H_INCLUDED
