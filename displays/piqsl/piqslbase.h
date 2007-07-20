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
		\brief Implements the default display devices for Aqsis.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#ifndef	___piqslbase_Loaded___
#define	___piqslbase_Loaded___

#include "aqsis.h"
#include "book.h"

#include <vector>
#include <string>
#include <list>

#include "boost/shared_ptr.hpp"
#include "image.h"
#include "framebuffer.h"
#include "logging.h"

START_NAMESPACE( Aqsis )

class CqPiqslBase
{
public:
	CqPiqslBase()		{}
	virtual ~CqPiqslBase()	{}

	/** Add a new book to the library.
	 * \param name		The name of the new book.
	 * \return		A shared pointer to the new book.
	 */
	virtual boost::shared_ptr<CqBook>	addNewBook(std::string name);
	/** Set the current book that all primary selection operations work on.
	 * \param book		A shared pointer to the book to set as current.
	 */
	virtual void	setCurrentBook(boost::shared_ptr<CqBook>& book);
	/** Get the current book.
	 * \return			A shared pointer to the current book.
	 */
	boost::shared_ptr<CqBook>& currentBook();
	/** Delete a given book, and all it's image references from the libarary.
	 * \param book		A shared pointer to the book to delete.
	 */
	virtual void deleteBook(boost::shared_ptr<CqBook>& book);
	/** Add a new image to the current book.
	 * \param image		A shared pointer to the new image to add.
	 * \return			The index of the image within the book.
	 */
	virtual TqUlong	addImageToCurrentBook(boost::shared_ptr<CqImage>& image);
	/** Set the current image index on the current book.
	 * \param index		The index of the current image in the current book.
	 */
	virtual void setCurrentImage(std::vector<boost::shared_ptr<CqImage> >::size_type index)
	{}
	/** Update the image list of the current book in the UI.
	 */
	virtual void updateImageList()
	{}
	/** Save the current library using the locally stored name.
	 */
	virtual void saveConfiguration();
	/** Save the current library, offering the user the chance to choose a name.
	 * This is overridden in the implementation class to use the UI tools to choose a name.
	 */
	virtual void saveConfigurationAs() = 0;
	/** Load a library from the given filename.
	 * \param name		The name of the XML file to load a library from.
	 */
	virtual void loadConfiguration(const std::string& name);
	/** Load an image from disk into the current book.
	 * \param name		The name to give to the image in the display.
	 * \param filename	The filename of the TIFF image to load.
	 */
	virtual void loadImageToCurrentBook(const std::string& name, const std::string& filename);
	/** Export a single book to a library file.
	 * \param book		A shared pointer to the book to save.
	 * \param name		The filename to save the book into.
	 */
	virtual void exportBook(boost::shared_ptr<CqBook>& book, const std::string& name) const;

	/** Get the current name that the library would be saved to.
	 * \return			The name of the library on disk.
	 */
	const std::string& currentConfigName() const
	{
		return(m_currentConfigName);
	}
	/** Get the current name that the library would be saved to.
	 * \return			The name of the library on disk.
	 */
	std::string& currentConfigName()
	{
		return(m_currentConfigName);
	}
	/** Set the current name that the library would be saved to.
	 * \param name		The name of the library on disk.
	 */
	void setCurrentConfigName(const std::string& name)
	{
		m_currentConfigName = name;
	}

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
	TqBookListIterator booksBegin()
	{
		return(m_books.begin());
	}
	/** Get an iterator to just past the last book in the library.
 	 * Follows the standard STL iterator conventions, so can be used in a != comparison
 	 * for iterating the books.
 	 */
	TqBookListIterator booksEnd()
	{
		return(m_books.end());
	}

private:
	std::vector<boost::shared_ptr<CqBook> >	m_books;	///< List of books in the library.
	boost::shared_ptr<CqBook>	m_currentBook;			///< Shared pointer to the current book.
	std::string m_currentConfigName;					///< Stored name of the library on disk.
};


//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

#endif	//	___display_Loaded___
