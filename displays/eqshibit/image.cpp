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
		\brief Implements the basic image functionality.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include "image.h"
#include "framebuffer.h"


START_NAMESPACE( Aqsis )
/*
void CqImage::connect(boost::shared_ptr<CqFramebuffer>& fb)
{
	// Disconnect any existing conduit
	//disconnect();
	// Create a new conduit, connecting the two objects.
	boost::shared_ptr<CqConduit<CqImage, CqFramebuffer> > conduit(new CqConduit<CqImage, CqFramebuffer>(this, fb.get()));
	m_associatedFB = conduit;
	// Connect the framebuffer to the conduit
	fb->connect(conduit);
}

void CqImage::connect(boost::shared_ptr<CqConduit<CqImage, CqFramebuffer> >& conduit)
{
	if(m_associatedFB)
		m_associatedFB.reset();
	m_associatedFB = conduit;
}

void CqImage::disconnect()
{
	// Disconnect the framebuffer from the conduit.
	if(m_associatedFB->b())
		m_associatedFB->b()->disconnect();
	m_associatedFB.reset();
}
*/
END_NAMESPACE( Aqsis )
