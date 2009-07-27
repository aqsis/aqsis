#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Browser.H>
#include <FL/Fl_Select_Browser.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Menu_Button.H>
#include <FL/Fl_Pack.H>

#include "ZoomImage.h"
#include "CenterScroll.h"
#include "Pane.h"

void quit_cb(Fl_Widget* w, void*);

Fl_Menu_Item mainMenu[] = {
	{"&File", 0, 0, 0, FL_SUBMENU},
		{"New Book"},
		{"Open Book"},
		{"Save Book"},
		{"Save Book As", 0, 0, 0, FL_MENU_DIVIDER},
		{"Quit", 0, quit_cb},
		{0},
	{"&View", 0, 0, 0, FL_SUBMENU},
		{"Library dialog"},
		{"..."},
		{0},
	{"&Book", 0, 0, 0, FL_SUBMENU},
		{"Rename"},
		{"Add Image"},
		{"..."},
		{0},
	{"&Window", 0, 0, 0, FL_SUBMENU},
		{"Book1"},
		{"Book2"},
		{"Book3"},
		{"..."},
		{0},
	{"&Help", 0, 0, 0, FL_SUBMENU},
		{"About"},
		{0},
	{0}
};


class TestWin : public Fl_Double_Window
{
	private:
		Fl_Menu_Bar m_menuBar;
		Pane* m_pane;
		CenterScroll* m_scroll;
		bool m_fullScreenImage;
	public:
		TestWin(int w, int h, const char* title)
			: Fl_Double_Window(w, h, title),
			m_menuBar(0, 0, w, 25),
			m_pane(0),
			m_scroll(0),
			m_fullScreenImage(false)
		{
			m_menuBar.menu(mainMenu);
			m_menuBar.box(FL_THIN_UP_BOX);
			m_pane = new Pane(0, m_menuBar.h(), w, h - m_menuBar.h(), 0.2);

//			Fl_Pack* pack = new Fl_Pack(0, 0, 1, 1);
//			pack->type(FL_VERTICAL);
			Fl_Browser* browser = new Fl_Select_Browser(0, 0, 1, 100);
			browser->add("blah.tif");
			browser->add("blah1.tif");
			browser->add("blah2.tif");
			browser->add("blah3.tif");
			browser->box(FL_THIN_DOWN_BOX);
			m_pane->add1(browser);
//				pack->resizable(browser);
//			pack->end();
//			m_pane->add1(pack);

			m_scroll = new CenterScroll(0, m_menuBar.h(), w, h - m_menuBar.h());
			m_pane->add2(m_scroll);

			resizable(m_pane);
			end();
		}
		virtual int handle(int event)
		{
			switch(event)
			{
				case FL_SHORTCUT:
					switch(Fl::event_key())
					{
						case 'f':
							if(m_fullScreenImage)
							{
								m_menuBar.show();
								m_pane->resize(0, m_menuBar.h(), w(), h() - m_menuBar.h());
								m_pane->uncollapse();
								m_fullScreenImage = false;
								//init_sizes();
							}
							else
							{
								m_menuBar.hide();
								m_pane->resize(0, 0, w(), h());
								m_pane->collapse1();
								m_fullScreenImage = true;
								//init_sizes();
							}
							return 1;
					}
					break;
			}
			return Fl_Group::handle(event);
		}
};

TestWin* mainWin;

void quit_cb(Fl_Widget* w, void*)
{
	mainWin->hide();
}

int main()
{
	TestWin win(640, 480, "TestWin");
	mainWin = &win;
	win.show();
	return Fl::run();
}
