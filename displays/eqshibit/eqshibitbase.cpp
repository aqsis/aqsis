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
#include "tinyxml.h"
#include "file.h"

#include <FL/Fl_File_Chooser.H>

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


void CqEqshibitBase::saveConfigurationAs()
{
	Fl::lock();
	char* name = fl_file_chooser("Save Configuration As", "*.xml", m_currentConfigName.c_str());
#if	1
	if(name != NULL)
	{
		std::string _base = CqFile::basePath(name);
		m_currentConfigName = name;
		TiXmlDocument doc(name);
		TiXmlDeclaration* decl = new TiXmlDeclaration("1.0","","yes");
		TiXmlElement* booksXML = new TiXmlElement("Books");

		std::map<std::string, boost::shared_ptr<CqBook> >::iterator book;
		for(book = m_books.begin(); book != m_books.end(); ++book) 
		{
			TiXmlElement* bookXML = new TiXmlElement("Book");
			bookXML->SetAttribute("name", book->first);
			booksXML->LinkEndChild(bookXML);
			TiXmlElement* imagesXML = new TiXmlElement("Images");

			std::map<TqUlong, boost::shared_ptr<CqImage> >::iterator image;
			for(image = book->second->images().begin(); image != book->second->images().end(); ++image)
			{
				// Serialise the image first.
				image->second->serialise(_base);
				TiXmlElement* imageXML = image->second->serialiseToXML();
				imagesXML->LinkEndChild(imageXML);
			}
			bookXML->LinkEndChild(imagesXML);
		}
		doc.LinkEndChild(decl);
		doc.LinkEndChild(booksXML);
		doc.SaveFile(name);
	}
#else
	if(name != NULL)
	{
		m_currentConfigName = name;
		TiXmlDocument doc(name);
		TiXmlDeclaration* decl = new TiXmlDeclaration("1.0","","yes");
		TiXmlElement* booksXML = new TiXmlElement("Books");

		std::map<std::string, boost::shared_ptr<CqBook> >::iterator book;
		for(book = m_books.begin(); book != m_books.end(); ++book) 
		{
			TiXmlElement* bookXML = new TiXmlElement("Book");
			bookXML->SetAttribute("name", book->first);
			booksXML->LinkEndChild(bookXML);
			TiXmlElement* imagesXML = new TiXmlElement("Images");

			std::map<TqUlong, boost::shared_ptr<CqImage> >::iterator image;
			for(image = book->second->images().begin(); image != book->second->images().end(); ++image)
			{
				// Serialise the image first.
				TiXmlElement* imageXML = image->second->serialiseToXML();
				imagesXML->LinkEndChild(imageXML);
			}
			bookXML->LinkEndChild(imagesXML);
		}
		doc.LinkEndChild(decl);
		doc.LinkEndChild(booksXML);
		doc.SaveFile(name);
	}
#endif
	Fl::unlock();
}

//---------------------------------------------------------------------

END_NAMESPACE( Aqsis )
