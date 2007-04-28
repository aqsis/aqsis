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
		\brief Implements the common book class functionality.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#include "book.h"


START_NAMESPACE( Aqsis )

TqUlong CqBook::m_nextID = 0;

TqUlong CqBook::addImage(boost::shared_ptr<CqImage>& image)
{
	m_images[m_nextID] = image;
	return(m_nextID++);
}

boost::shared_ptr<CqImage> CqBook::image(TqUlong id)
{
	if(m_images.find(id) != m_images.end())
		return(m_images[id]);
	else
	{
		boost::shared_ptr<CqImage> t;
		return(t);
	}
}

END_NAMESPACE( Aqsis )

