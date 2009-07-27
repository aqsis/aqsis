#ifndef PANE_H_INCLUDED
#define PANE_H_INCLUDED

#include <iostream>

#include <FL/Fl_Group.H>
#include <FL/Fl_Box.H>

#include <boost/function.hpp>
#include <boost/bind.hpp>

int clamp(int x, int min, int max)
{
	if(x < min)
		return min;
	if(x > max)
		return max;
	return x;
}

class PaneDividerBox : public Fl_Box
{
	private:
		int m_prevDragX;
		boost::function<void (int)> m_dragCallback;
	public:
		PaneDividerBox(int x, int y, int w, int h, const char* l = 0)
			: Fl_Box(FL_FLAT_BOX, x, y, w, h, l),
			m_prevDragX(0)
		{ }

		void setDragCallback(const boost::function<void (int)>& fxn)
		{
			m_dragCallback = fxn;
		}

		void draw()
		{
			Fl_Box::draw();
			for(int i = 0; i < 5; ++i)
			{
				fl_color(FL_DARK2);
				fl_rectf(x() + 2, y() + h()/2 + (i-2)*5, 2, 2);
				fl_color(FL_LIGHT2);
				fl_point(x() + 2, y() + h()/2 + (i-2)*5);
			}
		}
		int handle(int event)
		{
			switch(event)
			{
				case FL_ENTER:
					color(FL_LIGHT1);
					fl_cursor(FL_CURSOR_WE, FL_FOREGROUND_COLOR, FL_BACKGROUND_COLOR);
					redraw();
					return 1;
				case FL_LEAVE:
					color(FL_BACKGROUND_COLOR);
					fl_cursor(FL_CURSOR_DEFAULT, FL_FOREGROUND_COLOR, FL_BACKGROUND_COLOR);
					redraw();
					return 1;
				case FL_PUSH:
					switch(Fl::event_button())
					{
						case FL_LEFT_MOUSE:
							m_prevDragX = Fl::event_x();
							return 1;
					}
					break;
				case FL_RELEASE:
					switch(Fl::event_button())
					{
						case FL_LEFT_MOUSE:
							return 1;
					}
					break;
				case FL_DRAG:
					switch(Fl::event_button())
					{
						case FL_LEFT_MOUSE:
							if(m_dragCallback)
								m_dragCallback(Fl::event_x() - m_prevDragX);
							m_prevDragX = Fl::event_x();
							return 1;
					}
					break;
			}
			return Fl_Box::handle(event);
		}
		void resize(int x, int y, int w, int h)
		{
			Fl_Box::resize(x, y, 6, h);
		}
};

class Pane : public Fl_Group
{
	private:
		PaneDividerBox m_divider;
		int m_prevDividerPos;
		Fl_Widget* m_widget1;
		Fl_Widget* m_widget2;
		void moveDivider(int dx)
		{
			m_prevDividerPos = m_divider.x();
			int dividerX = clamp(m_divider.x() + dx, 0, w() - m_divider.w());
			m_divider.resize(dividerX, y(), m_divider.w(), h());
			if(m_widget1)
			{
				m_widget1->resize(x(), y(), m_divider.x()-x(), h());
			}
			if(m_widget2)
			{
				int w2X = m_divider.x() + m_divider.w();
				m_widget2->resize(w2X, y(), w() - w2X, h());
				init_sizes();
			}
			damage(FL_DAMAGE_ALL);
		}
		void init()
		{
			box(FL_FLAT_BOX);
			m_divider.setDragCallback(boost::bind(&Pane::moveDivider, this, _1));
			end();
//			add1(new Fl_Box(FL_FLAT_BOX, 0, 0, 1, 1, 0));
//			add2(new Fl_Box(FL_FLAT_BOX, 0, 0, 1, 1, 0));
		}
	public:
		Pane(int x, int y, int w, int h, const char* l = 0)
			: Fl_Group(x,y,w,h,l),
			m_divider(x+w/2, y, 0, h),
			m_prevDividerPos(0),
			m_widget1(0),
			m_widget2(0)
		{
			init();
		}
		Pane(int x, int y, int w, int h, double divPos, const char* l = 0)
			: Fl_Group(x,y,w,h,l),
			m_divider(x+static_cast<int>(divPos*w), y, 0, h),
			m_prevDividerPos(0),
			m_widget1(0),
			m_widget2(0)
		{
			init();
		}

		void uncollapse()
		{
			moveDivider(m_prevDividerPos - m_divider.x());
		}
		void collapse1()
		{
			if(m_divider.x() != x())
				moveDivider(x() - m_divider.x());
		}
		void add1(Fl_Widget* widget)
		{
			remove(m_widget1);
			m_widget1 = widget;
			moveDivider(0);
			Fl_Group::add(widget);
		}
		void add2(Fl_Widget* widget)
		{
			remove(m_widget2);
			m_widget2 = widget;
			resizable(widget);
			moveDivider(0);
			Fl_Group::add(widget);
		}

		virtual void draw()
		{
			// Why didn't this work??
			//fl_rectf(x(), y(), w(), y());
			Fl_Group::draw();
		}
		virtual void resize(int x, int y, int w, int h)
		{
			Fl_Group::resize(x, y, w, h);
			if(m_divider.x() + m_divider.w() > x + w)
				moveDivider(0);
		}
};

#endif // PANE_H_INCLUDED
