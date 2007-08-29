// Aqsis
// Copyright (C) 1997 - 2007, Paul C. Gregory
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
 *
 * \brief Implementation of classes and functions for creating mipmaps
 *
 * \author Chris Foster
 */

#include "mipmap.h"
#include "aqsismath.h"

namespace Aqsis
{
//------------------------------------------------------------------------------

void mipmapLevelSizes(TqUint width, TqUint height,
		std::vector<TqUint>& levelWidths, std::vector<TqUint>& levelHeights)
{
	levelWidths.clear();
	levelHeights.clear();
	levelWidths.push_back(width);
	levelHeights.push_back(height);
	if(width == 0 || height == 0)
		return;
	while(width > 1 || height > 1)
	{
		width = (width+1)/2;
		height = (height+1)/2;
		levelWidths.push_back(width);
		levelHeights.push_back(height);
	}
}

//------------------------------------------------------------------------------
} // namespace Aqsis
