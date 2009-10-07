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
		\brief Implements the default display devices for Aqsis.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#ifndef	PIQSLMAINWINDOW_H_INCLUDED
#define	PIQSLMAINWINDOW_H_INCLUDED

#include <aqsis/aqsis.h>

#include "pane.h"
#include "centerscroll.h"
#include "bookbrowser.h"

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Sys_Menu_Bar.H>
#include <FL/Fl_Pack.H>

namespace Aqsis {

class CqPiqslMainWindow : public Fl_Double_Window
{
private:
	Fl_Menu_Bar m_menuBar;
	CqPane* m_pane;
	CqCenterScroll* m_scroll;
	bool m_fullScreenImage;
	int m_columnWidths[2];
	std::vector<boost::shared_ptr<CqBook> >	m_books;	///< List of books in the library.
	boost::shared_ptr<CqBook>	m_currentBook;			///< Shared pointer to the current book.
	std::string m_currentConfigName;					///< Stored name of the library on disk.
	bool	m_doResize;								///< Flag indicating a resize has been requested.
public:
	CqPiqslMainWindow(int w, int h, const char* title);
	virtual int handle(int event);
	void update(int X, int Y, int W, int H);

	/** Add a new book to the library.
	 * \param name		The name of the new book.
	 * \return		A shared pointer to the new book.
	 */
	boost::shared_ptr<CqBook>	addNewBook(std::string name);
	/** Set the current book that all primary selection operations work on.
	 * \param book		A shared pointer to the book to set as current.
	 */
	void	setCurrentBook(boost::shared_ptr<CqBook>& book);
	/** Get the current book.
	 * \return			A shared pointer to the current book.
	 */
	boost::shared_ptr<CqBook>& currentBook();
	/** Delete a given book, and all it's image references from the libarary.
	 * \param book		A shared pointer to the book to delete.
	 */
	void deleteBook(boost::shared_ptr<CqBook>& book);
	/** Add a new image to the current book.
	 * \param image		A shared pointer to the new image to add.
	 * \return			The index of the image within the book.
	 */
	TqUlong	addImageToCurrentBook(boost::shared_ptr<CqImage>& image);
	/** Set the current image index on the current book.
	 * \param index		The index of the current image in the current book.
	 */
	void setCurrentImage(CqBook::TqImageList::size_type index);
	/** Update the image list of the current book in the UI.
	 */
	void updateImageList();
	/** Save the current library using the locally stored name.
	 */
	void saveConfiguration();
	/** Save the current library, offering the user the chance to choose a name.
	 * This is overridden in the implementation class to use the UI tools to choose a name.
	 */
	void saveConfigurationAs();
	/** Load a library from the given filename.
	 * \param name		The name of the XML file to load a library from.
	 */
	void loadConfiguration(const std::string& name);
	/** Load an image from disk into the current book.
	 * \param name		The name to give to the image in the display.
	 * \param filename	The filename of the TIFF image to load.
	 */
	void loadImageToCurrentBook(const std::string& name, const std::string& filename);
	/** Export a single book to a library file.
	 * \param book		A shared pointer to the book to save.
	 * \param name		The filename to save the book into.
	 */
	void exportBook(boost::shared_ptr<CqBook>& book, const std::string& name) const;

	/** Get the current name that the library would be saved to.
	 * \return			The name of the library on disk.
	 */
	const std::string& currentConfigName() const;
	/** Get the current name that the library would be saved to.
	 * \return			The name of the library on disk.
	 */
	std::string& currentConfigName();
	/** Set the current name that the library would be saved to.
	 * \param name		The name of the library on disk.
	 */
	void setCurrentConfigName(const std::string& name);

	/** \typedef TqBookList
	 * Typedef for the list of books in the libary.
	 */
	typedef std::vector<boost::shared_ptr<CqBook> >			TqBookList;
	/** \typedef TqBookListIterator
	 * Typedef for an iterator over the list of books in the libary.
	 */
	typedef std::vector<boost::shared_ptr<CqBook> >::iterator	TqBookListIterator;
	/** Get an iterator to the start of the books in the library.
	 */
	TqBookListIterator booksBegin();
	/** Get an iterator to just past the last book in the library.
 	 * Follows the standard STL iterator conventions, so can be used in a != comparison
 	 * for iterating the books.
 	 */
	TqBookListIterator booksEnd();

	/** Queue a request to recalculate the size of this window.
	 * The resize operation can only be performed on the main thread, this allows other threads to force a recalc.
	 */
	void queueResize();
	/** Perform a recalculation of the size of this framebuffer.
	 * Actually recalculate the size of the framebuffer window according to the image being displayed. Can
	 * only be performed in the main thread.
	 */
	void resize();
	/** Overriden Fl_Widget::resize()
	 */
	virtual void resize(int X, int Y, int W, int H);
	/** Check if a resize has been queued and perform it.
	 * \note Must only be called from the main thread.
	 */
	void checkResize();

	void setImage(const boost::shared_ptr<CqImage>& image);

	void addImage();
	static void addImage_cb(Fl_Widget* w, void*);
	void select();
	static void select_cb(Fl_Widget* w, void*);
	void loadLibrary();
	static void loadLibrary_cb(Fl_Widget* w, void*);
	static void saveLibrary_cb(Fl_Widget* w, void*);
	static void saveLibraryAs_cb(Fl_Widget* w, void*);
	void removeImage();
	static void removeImage_cb(Fl_Widget* w, void*);
};

inline const std::string& CqPiqslMainWindow::currentConfigName() const
{
	return(m_currentConfigName);
}

inline std::string& CqPiqslMainWindow::currentConfigName()
{
	return(m_currentConfigName);
}

inline void CqPiqslMainWindow::setCurrentConfigName(const std::string& name)
{
	m_currentConfigName = name;
}

inline CqPiqslMainWindow::TqBookListIterator CqPiqslMainWindow::booksBegin()
{
	return(m_books.begin());
}

inline CqPiqslMainWindow::TqBookListIterator CqPiqslMainWindow::booksEnd()
{
	return(m_books.end());
}

//-----------------------------------------------------------------------

} // namespace Aqsis

#endif	
