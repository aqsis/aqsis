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
		\brief Implements the common piqsl functionality.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#include <aqsis/aqsis.h>

#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Menu_Button.H>
#include <FL/fl_ask.H>

#include <aqsis/util/logging.h>
#include <aqsis/version.h>

#include "piqslmainwindow.h"
#include "image.h"

extern Aqsis::CqPiqslMainWindow* window;

namespace Aqsis {

static void quit_cb(Fl_Widget* w, void*);
static void about_cb(Fl_Widget* w, void*);

static Fl_Menu_Item mainMenu[] = {
	{"&File", FL_ALT+'f', 0, 0, FL_SUBMENU},
		{"&Open Library", FL_COMMAND+'o', (Fl_Callback*)CqPiqslMainWindow::loadLibrary_cb},
		{"&Save Library", FL_COMMAND+'s', (Fl_Callback*)CqPiqslMainWindow::saveLibrary_cb},
		{"Save Library &As", FL_SHIFT+FL_COMMAND+'S', (Fl_Callback*)CqPiqslMainWindow::saveLibraryAs_cb, 0, FL_MENU_DIVIDER},
		{"&Quit", FL_COMMAND+'q', quit_cb},
		{0},
	// TODO: Re-introduce 'Book' functionality for Aqsis 1.7 (and later).
/*	{"&Book", FL_ALT+'b', 0, 0, FL_SUBMENU},
		{"&New"},
		{"&Export"},
		{"&Rename"},
		{"Re&move"},
		{0},
*/
	{"&Image", FL_ALT+'i', 0, 0, FL_SUBMENU},
		{"&Add", FL_SHIFT+FL_COMMAND+'O', (Fl_Callback*)CqPiqslMainWindow::addImage_cb},
		{"&Remove", FL_SHIFT+FL_COMMAND+'X', (Fl_Callback*)CqPiqslMainWindow::removeImage_cb},
		{0},
	{"&Help", FL_ALT+'h', 0, 0, FL_SUBMENU},
		{"&About", 0, (Fl_Callback*)about_cb},
		{0},
	{0}
};

CqPiqslMainWindow::CqPiqslMainWindow(int w, int h, const char* title)
	: Fl_Double_Window(w, h, title),
	m_menuBar(0, 0, w, 25),
	m_pane(0),
	m_scroll(0),
	m_fullScreenImage(false),
	m_doResize(false)
{
	// Set user_data to point to this, so we can
	// get back to the main window from anywhere.
	user_data((void*)(this));

	mainMenu[1].user_data_ = this;
	mainMenu[2].user_data_ = this;
	mainMenu[3].user_data_ = this;
	m_menuBar.menu(mainMenu);
	// Setup some callback user data.
	m_menuBar.box(FL_THIN_UP_BOX);

	m_pane = new CqPane(0, m_menuBar.h(), w, h - m_menuBar.h(), 0.2);

	CqBookBrowser* browser = new CqBookBrowser(0, 0, 1, 100);
	browser->box(FL_THIN_DOWN_BOX);
	m_pane->add1(browser);

	m_scroll = new CqCenterScroll(0, m_menuBar.h(), w, h - m_menuBar.h());
	m_pane->add2(m_scroll);

	browser->take_focus();

	resizable(m_pane);
	end();
}

int CqPiqslMainWindow::handle(int event)
{
	switch(event)
	{
		case FL_SHORTCUT:
			switch(Fl::event_key())
			{
				case 'f':
					if(!Fl::event_alt())
					{
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
				// Swallow the unmodified menu shortcuts, so the menus only activate with alt
				case 'b':
				case 'i':
				case 'h':
					if(!Fl::event_alt())
						return 1;
				case FL_BackSpace:
				case FL_Delete:
					if(Fl::event_state(FL_CTRL))
					{
						// Remove the current image from the current book.
						//
						// It's pretty ugly to do the deltion from here;
						// Piqsl needs to use a proper signals/slots
						// mechanism or something...
						removeImage();
						return 1;
					}
					else
						return 0;
			}
			break;
	}
	return Fl_Group::handle(event);
}

void CqPiqslMainWindow::saveConfigurationAs() 
{
    char* filename = fl_file_chooser("Save Books As", "*.bks", currentConfigName().c_str());
	if(filename != NULL)
	{
		setCurrentConfigName(filename);
		saveConfiguration();
	}
}

static void quit_cb(Fl_Widget* w, void*)
{
	window->hide();
}


void CqPiqslMainWindow::addImage_cb(Fl_Widget* w, void*)
{
	((CqPiqslMainWindow*)(w->parent()->user_data()))->addImage();
}

void CqPiqslMainWindow::addImage()
{
	Fl::lock();
	char* filename = fl_file_chooser("Load Image", "All Supported Files (*.{tif,tiff,exr,env,tx,tex,shad,zfile,sm})\tTIFF Files (*.{tif,tiff})\tOpenEXR Files (*.exr)\tTeqser Files (*.{env,tx,tex})\tShadow Files (*.{shad,zfile,sm})", "");
	if(filename)
	{
		std::string name = boost::filesystem::path(filename).leaf();
		loadImageToCurrentBook(name, filename);
		updateImageList();
	}
	Fl::unlock();
}

void CqPiqslMainWindow::removeImage_cb(Fl_Widget* w, void*)
{
	((CqPiqslMainWindow*)(w->parent()->user_data()))->removeImage();
}

void CqPiqslMainWindow::removeImage()
{
	Fl::lock();
	Aqsis::CqBook::TqImageList::size_type index = m_pane->browser()->currentSelected();
	if (index > 0) // something must be selected
	{
		Aqsis::CqBook::TqImageListIterator item = m_pane->browser()->book()->imagesBegin();
		item += (index-1);
		m_pane->browser()->book()->removeImage(item);
		// If possible, select the image below the one deleted, of not
		// select the one above, otherwise, select none.
		if(m_pane->browser()->book()->numImages() < index)
		{
			if(m_pane->browser()->book()->numImages() > 0)
				index -= 1;
			else
				index = 0;
		}
		m_pane->browser()->setCurrentSelected(index);
		select();
		damage(FL_DAMAGE_ALL);
		//m_menuImagesRemove->deactivate();
		updateImageList();
		Fl::awake();
	}
	Fl::unlock();
}
void CqPiqslMainWindow::setImage(const boost::shared_ptr<CqImage>& image)
{
	m_scroll->setImage(image);
}

void CqPiqslMainWindow::update(int X, int Y, int W, int H)
{
	Fl::lock();
	m_scroll->update(X, Y, W, H);
	Fl::awake();
	Fl::unlock();
}

void CqPiqslMainWindow::setCurrentImage(CqBook::TqImageList::size_type index)
{
	Fl::lock();
	// Note: in the FLTK gui, the image indices are 1 based, but inside the CqBook
	// they are zero based, this is because the (void*) stuff passed around in 
	// FLTK for the Fl_Browser_ class uses 0 to indicate false.
	setImage(currentBook()->image(index));
	boost::function<void(int,int,int,int)> f;
	f = boost::bind(&CqPiqslMainWindow::update, this, _1, _2, _3, _4);
	currentBook()->image(index)->setUpdateCallback(f);
	m_pane->browser()->setCurrentSelected(index+1);
	m_pane->redraw();
	Fl::awake();
	Fl::unlock();
}

void CqPiqslMainWindow::updateImageList()
{
    Fl::lock();
	m_pane->redraw();
	Fl::awake();
	Fl::unlock();
}

boost::shared_ptr<CqBook>	CqPiqslMainWindow::addNewBook(std::string name)
{
    Fl::lock();

	boost::shared_ptr<CqBook> newBook(new CqBook(name));
	m_books.push_back(newBook);
	m_currentBook = newBook;

	Aqsis::CqBookBrowser *o = m_pane->browser();
	o->setBook(newBook);
	m_columnWidths[0] = 200;
	m_columnWidths[1] = 0;
	o->column_widths(m_columnWidths);
	o->type(FL_MULTI_BROWSER);
	o->callback(&CqPiqslMainWindow::select_cb, this);
	o->showcolsep(1);
	Fl::awake();
	Fl::unlock();
	return(newBook);
}

void CqPiqslMainWindow::select_cb(Fl_Widget* w, void* d)
{
	((CqPiqslMainWindow*)(d))->select();
}

void CqPiqslMainWindow::select()
{
    Fl::lock();
	Aqsis::CqBookBrowser* browser = m_pane->browser();
	int theEvent = Fl::event();
	if(theEvent == FL_RELEASE || theEvent == FL_KEYDOWN || theEvent == FL_SHORTCUT)
	{
		if(currentBook())
		{
			std::vector<boost::shared_ptr<Aqsis::CqImage> >::size_type selected = 
				browser->currentSelected();
			boost::shared_ptr<Aqsis::CqImage> image = currentBook()->image(selected-1);
			if(image)
			{
				setImage(image);
				//m_menuImagesRemove->activate();
			}
			else
			{
				//m_menuImagesRemove->deactivate();
			}
		}
	}
	Fl::awake();
	Fl::unlock();
}

void CqPiqslMainWindow::setCurrentBook(boost::shared_ptr<CqBook>& book)
{
	if(std::find(m_books.begin(), m_books.end(), book) != m_books.end())
		m_currentBook = book;
}


void CqPiqslMainWindow::deleteBook(boost::shared_ptr<CqBook>& book)
{
	std::vector<boost::shared_ptr<CqBook> >::iterator entry;
	if((entry = std::find(m_books.begin(), m_books.end(), book)) != m_books.end())
	{
		m_books.erase(entry);
		if(m_currentBook == book)
		{
			boost::shared_ptr<CqBook> t;
			m_currentBook = t;
		}
	}
}

boost::shared_ptr<CqBook>& CqPiqslMainWindow::currentBook()
{
	return(m_currentBook);
}

TqUlong CqPiqslMainWindow::addImageToCurrentBook(boost::shared_ptr<CqImage>& image)
{
	Aqsis::log() << Aqsis::debug << "Piqsl adding image" << std::endl;
	if(!m_currentBook)
	{
		std::map<std::string, boost::shared_ptr<CqBook> >::size_type numBooks = m_books.size();
		std::stringstream strBkName;
		strBkName << "Book" << numBooks+1;
		addNewBook(strBkName.str());
	}
	TqUlong id = currentBook()->addImage(image);
	setCurrentImage(id);
	return( id );	
}

void CqPiqslMainWindow::saveConfiguration()
{
	Fl::lock();
	if(!m_currentConfigName.empty())
	{
		boost::filesystem::path saveDir =
			boost::filesystem::path(m_currentConfigName).branch_path();
		TiXmlDocument doc(m_currentConfigName);
		TiXmlDeclaration* decl = new TiXmlDeclaration("1.0","","yes");
		TiXmlElement* booksXML = new TiXmlElement("Books");

		std::vector<boost::shared_ptr<CqBook> >::iterator book;
		for(book = m_books.begin(); book != m_books.end(); ++book) 
		{
			TiXmlElement* bookXML = new TiXmlElement("Book");
			bookXML->SetAttribute("name", (*book)->name());
			booksXML->LinkEndChild(bookXML);
			TiXmlElement* imagesXML = new TiXmlElement("Images");

			CqBook::TqImageListIterator image;
			for(image = (*book)->imagesBegin(); image != (*book)->imagesEnd(); ++image)
			{
				// Serialise the image first.
				(*image)->serialise(saveDir);
				TiXmlElement* imageXML = (*image)->serialiseToXML();
				imagesXML->LinkEndChild(imageXML);
			}
			bookXML->LinkEndChild(imagesXML);
		}
		doc.LinkEndChild(decl);
		doc.LinkEndChild(booksXML);
		doc.SaveFile(m_currentConfigName);
	}
	else
		saveConfigurationAs();
	Fl::unlock();
}


void CqPiqslMainWindow::exportBook(boost::shared_ptr<CqBook>& book, const std::string& name) const
{
	Fl::lock();
	if(!name.empty() && book)
	{
		boost::filesystem::path saveDir = boost::filesystem::path(name).branch_path();
		TiXmlDocument doc(name);
		TiXmlDeclaration* decl = new TiXmlDeclaration("1.0","","yes");
		TiXmlElement* booksXML = new TiXmlElement("Books");

		TiXmlElement* bookXML = new TiXmlElement("Book");
		bookXML->SetAttribute("name", book->name());
		booksXML->LinkEndChild(bookXML);
		TiXmlElement* imagesXML = new TiXmlElement("Images");

		CqBook::TqImageListIterator image;
		for(image = book->imagesBegin(); image != book->imagesEnd(); ++image)
		{
			// Serialise the image first.
			(*image)->serialise(saveDir);
			TiXmlElement* imageXML = (*image)->serialiseToXML();
			imagesXML->LinkEndChild(imageXML);
		}
		bookXML->LinkEndChild(imagesXML);
		
		doc.LinkEndChild(decl);
		doc.LinkEndChild(booksXML);
		doc.SaveFile(name);
	}
	Fl::unlock();
}


void CqPiqslMainWindow::loadConfiguration(const std::string& name)
{
	Fl::lock();

	if(!name.empty())
	{
		m_currentConfigName = name;
		TiXmlDocument doc(name);
		bool loadOkay = doc.LoadFile();
		if(loadOkay)
		{
			TiXmlElement* booksXML = doc.RootElement();
			if(booksXML)
			{
				TiXmlElement* bookXML = booksXML->FirstChildElement("Book");
				while(bookXML)
				{
					std::string bookName = bookXML->Attribute("name");
					addNewBook(bookName);

					TiXmlElement* imagesXML = bookXML->FirstChildElement("Images");
					if(imagesXML)
					{
						TiXmlElement* imageXML = imagesXML->FirstChildElement("Image");
						while(imageXML)
						{
							std::string imageName("");
							std::string imageFilename("");
							TiXmlElement* nameXML = imageXML->FirstChildElement("Name");
							if(nameXML && nameXML->GetText())
								imageName = nameXML->GetText();
							TiXmlElement* fileNameXML = imageXML->FirstChildElement("Filename");
							if(fileNameXML && fileNameXML->GetText())
								imageFilename = fileNameXML->GetText();
							loadImageToCurrentBook(imageName, imageFilename);
							imageXML = imageXML->NextSiblingElement("Image");
						}
					}
					bookXML = bookXML->NextSiblingElement("Book");
				}
			}
		}
		else
		{
			Aqsis::log() << Aqsis::error << "Failed to load configuration file" << std::endl;
		}
	}
	Fl::unlock();
}

void CqPiqslMainWindow::loadImageToCurrentBook(const std::string& name, const std::string& filename)
{
	boost::shared_ptr<CqImage> newImage(new CqImage(name));
	newImage->loadFromFile(filename);
	TqUlong id = addImageToCurrentBook(newImage);

	setCurrentImage(id);
}



void CqPiqslMainWindow::loadLibrary_cb(Fl_Widget* w, void* d)
{
	((CqPiqslMainWindow*)(d))->loadLibrary();
}

void CqPiqslMainWindow::loadLibrary()
{
	char* name = fl_file_chooser("Load Books", "*.bks", currentConfigName().c_str());
	if(name)
		loadConfiguration(name);
}

void CqPiqslMainWindow::saveLibrary_cb(Fl_Widget* w, void* d)
{
	((CqPiqslMainWindow*)(d))->saveConfiguration();
}

void CqPiqslMainWindow::saveLibraryAs_cb(Fl_Widget* w, void* d)
{
	((CqPiqslMainWindow*)(d))->saveConfigurationAs();
}


void CqPiqslMainWindow::queueResize()
{
	Fl::lock();
	m_doResize = true;
	Fl::unlock();
}

void CqPiqslMainWindow::resize(int X, int Y, int W, int H)
{
	// is the window actually getting resized? (might just be moved)
	//bool isResizing = W != w() || H != h();
	// call parent's resize()
	Fl_Double_Window::resize(X, Y, W, H);
//	if(isResizing)
//		centerImageWidget();
}

void CqPiqslMainWindow::resize()
{
	Fl::lock();
	int fw = 640;
	int fh = 480;
	boost::shared_ptr<CqImage> image = m_pane->centerScroll()->image();
	if(image)
	{
		if(image->frameWidth() > 0 && image->frameHeight() > 0)
		{
			fw = image->frameWidth();
			fh = image->frameHeight();
		}
	}
	m_pane->centerScroll()->resizeImageWidget(fw, fh);
	// Resize the window if it's too small, leave it if it's big enough or bigger.
	/*
	int thisW = m_pane->centerScroll()->x() + fw;
	int thisH = m_pane->centerScroll()->y() + fh;
	if( w() < thisW || h() < thisH )
		resize(x(), y(), min(thisW, static_cast<int>(Fl::w()*0.9)), min(thisH, static_cast<int>(Fl::h()*0.9)));
	*/
	m_pane->centerScroll()->centerImageWidget();
	redraw();

	m_doResize = false;
	Fl::unlock();
}


/// \note: This should only ever be called from the main thread.
void CqPiqslMainWindow::checkResize()
{
	if(m_doResize)
		resize();
}

static void about_cb(Fl_Widget* w, void*)
{
	fl_message("piqsl version %s\ncompiled %s %s",
			   AQSIS_VERSION_STR_FULL, __DATE__, __TIME__);
}

} // namespace Aqsis
