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
#include "logging.h"


START_NAMESPACE( Aqsis )

void CqImage::PrepareImageBuffer()
{
	//boost::mutex::scoped_lock lock(mutex());
	m_data = reinterpret_cast<unsigned char*>(malloc( m_imageWidth * m_imageHeight * m_channels * sizeof(TqUchar)));
	// Initialise the display to a checkerboard to show alpha
	for (TqUlong i = 0; i < imageHeight(); i ++)
	{
		for (TqUlong j = 0; j < imageWidth(); j++)
		{
			int     t       = 0;
			TqUchar d = 255;

			if ( ( (imageHeight() - 1 - i) & 31 ) < 16 )
				t ^= 1;
			if ( ( j & 31 ) < 16 )
				t ^= 1;
			if ( t )
				d      = 128;
			for(TqUint chan = 0; chan < channels(); chan++)
				data()[channels() * (i*imageWidth() + j) + chan ] = d;
		}
	}
}

void CqImage::setUpdateCallback(boost::function<void(int,int,int,int)> f)
{
	m_updateCallback = f;
}

END_NAMESPACE( Aqsis )
