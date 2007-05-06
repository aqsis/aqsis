// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
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
		\brief Implements the common eqshibit functionality.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#include "eqshibitbase.h"
#include "book.h"


START_NAMESPACE( Aqsis )


void CqEqshibitBase::addNewBook(std::string name)
{
	boost::shared_ptr<CqBook> newBook(new CqBook(name));
	m_books[name] = newBook;
	m_currentBookName = name;
}

void CqEqshibitBase::setCurrentBook(std::string name)
{
	if(m_books.find(name) != m_books.end())
		m_currentBookName = name;
}


boost::shared_ptr<CqBook>& CqEqshibitBase::currentBook()
{
	return(m_books.find(m_currentBookName)->second);
}

boost::shared_ptr<CqBook>& CqEqshibitBase::book(const std::string& name)
{
	return(m_books.find(name)->second);
}

std::string CqEqshibitBase::currentBookName()
{
	return( m_currentBookName );
}

TqUlong CqEqshibitBase::addImageToCurrentBook(boost::shared_ptr<CqImage>& image)
{
	if(m_currentBookName.empty())
	{
		Aqsis::log() << Aqsis::debug << "Eqshibit adding image" << std::endl;
		std::map<std::string, boost::shared_ptr<CqBook> >::size_type numBooks = m_books.size();
		std::stringstream strBkName;
		strBkName << "Book" << numBooks+1;
		addNewBook(strBkName.str());
	}
	return( currentBook()->addImage(image));	
}

//---------------------------------------------------------------------

END_NAMESPACE( Aqsis )
