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
		\brief Declare the class controlling book's of images.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is book.h included already?
#ifndef BOOK_H_INCLUDED
#define BOOK_H_INCLUDED 1

#include 	<boost/shared_ptr.hpp>
#include	<vector>

#include	"aqsis.h"
#include	"sstring.h"
#include	"ndspy.h"

START_NAMESPACE( Aqsis )

class CqFramebuffer;
class CqImage;

//---------------------------------------------------------------------
/** \class CqBook
 * Class encapsulating the image book functionality.
 */

class CqBook
{
public:
    CqBook( const CqString& name ) : m_name(name)
	{}
    ~CqBook()
	{}

    CqString&	name()
    {
        return ( m_name );
    }
    void	setName( const CqString& name )
    {
        m_name = name;
    }

	boost::shared_ptr<CqFramebuffer> framebuffer()
	{
		return(m_framebuffer);
	}
	void setFramebuffer(boost::shared_ptr<CqFramebuffer>& fb)
	{
		m_framebuffer = fb;
	}

	typedef std::vector<boost::shared_ptr<CqImage> >			TqImageList;
	typedef std::vector<boost::shared_ptr<CqImage> >::iterator	TqImageListIterator;
	TqImageListIterator imagesBegin()
	{
		return(m_images.begin());
	}
	TqImageListIterator imagesEnd()
	{
		return(m_images.end());
	}

	TqImageList::size_type addImage(boost::shared_ptr<CqImage>& image);
	boost::shared_ptr<CqImage> image(TqImageList::size_type index);
	TqUlong numImages() const
	{
		return(m_images.size());
	}

private:
    CqString	m_name;			///< Book name.
	TqImageList m_images;
	boost::shared_ptr<CqFramebuffer> m_framebuffer;
	static TqUlong	m_nextID;
};

END_NAMESPACE( Aqsis )

#endif	// BOOK_H_INCLUDED
