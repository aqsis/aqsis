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
		\brief Implements the common book class functionality.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#include "book.h"
#include "image.h"


namespace Aqsis {

CqBook::CqBook( const std::string& name ) : m_name(name)
{
//	m_framebuffer = boost::shared_ptr<CqFramebuffer>(
//			new CqFramebuffer(CqFramebuffer::defaultWidth,
//				CqFramebuffer::defaultHeight, 3, name));
//	m_framebuffer->show();
}


std::vector<boost::shared_ptr<CqImage> >::size_type CqBook::addImage(boost::shared_ptr<CqImage>& image)
{
	m_images.push_back(image);
	return(m_images.size()-1);
}

boost::shared_ptr<CqImage> CqBook::image(CqBook::TqImageList::size_type index)
{
	if(m_images.size() > index)
		return(m_images[index]);
	else
	{
		boost::shared_ptr<CqImage> t;
		return(t);
	}
}

void CqBook::setName( const std::string& name )
{
	m_name = name;
//	if(m_framebuffer)
//		m_framebuffer->setBookName(name);
}

void CqBook::removeImage(TqImageListIterator item)
{
	if(item != m_images.end())
	{
//		if(framebuffer()->image() == (*item))
//		{
//			framebuffer()->disconnect();
//		}
		m_images.erase(item);
	}
}

} // namespace Aqsis

