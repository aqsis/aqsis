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

	virtual boost::shared_ptr<CqBook>	addNewBook(std::string name);
	virtual void	setCurrentBook(boost::shared_ptr<CqBook>& book);
	boost::shared_ptr<CqBook>& currentBook();
	virtual TqUlong	addImageToCurrentBook(boost::shared_ptr<CqImage>& image);
	virtual void setCurrentImage(std::vector<boost::shared_ptr<CqImage> >::size_type index)
	{}
	virtual void updateImageList()
	{}
	virtual void saveConfigurationAs(const std::string& name);
	virtual void loadConfiguration(const std::string& name);
	virtual void loadImageToCurrentBook(const std::string& name, const std::string& filename);

	const std::string& currentConfigName() const
	{
		return(m_currentConfigName);
	}
	std::string& currentConfigName()
	{
		return(m_currentConfigName);
	}

	typedef std::vector<boost::shared_ptr<CqBook> >			TqBookList;
	typedef std::vector<boost::shared_ptr<CqBook> >::iterator	TqBookListIterator;
	TqBookListIterator booksBegin()
	{
		return(m_books.begin());
	}
	TqBookListIterator booksEnd()
	{
		return(m_books.end());
	}

private:
	std::vector<boost::shared_ptr<CqBook> >	m_books;
	boost::shared_ptr<CqBook>	m_currentBook;
	std::string m_currentConfigName;
};


//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

#endif	//	___display_Loaded___
