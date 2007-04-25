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
#include <sstream>
#include <iostream>


START_NAMESPACE( Aqsis )


void CqEqshibitBase::addNewBook(std::string name)
{
	m_books[name] = TqEqshibitBook();
	m_currentBookName = name;
}

void CqEqshibitBase::setCurrentBook(std::string name)
{
	if(m_books.find(name) != m_books.end())
		m_currentBookName = name;
}


TqEqshibitBook& CqEqshibitBase::currentBook()
{
	return(m_books.find(m_currentBookName)->second);
}


std::string CqEqshibitBase::currentBookName()
{
	return( m_currentBookName );
}

void CqEqshibitBase::addImageToCurrentBook(std::string name)
{
	if(m_currentBookName.empty())
	{
		std::cout << "Adding image" << std::endl;
		std::map<std::string, TqEqshibitBook>::size_type numBooks = m_books.size();
		std::stringstream strBkName;
		strBkName << "Book" << numBooks+1;
		addNewBook(strBkName.str());
	}
	currentBook().push_back(name);	
}

//---------------------------------------------------------------------

END_NAMESPACE( Aqsis )
