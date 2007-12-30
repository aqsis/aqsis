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
 * \brief Implementation of functions for dealing with texture sampling
 * options.
 *
 * \author Chris Foster [chris42f (at) gmail (d0t) com]
 */

#include "texturesampleoptions.h"
#include <string.h>

#include "logging.h"

namespace Aqsis
{

EqTextureFilter texFilterTypeFromString(const char* filterName)
{
	if(strcmp(filterName, "box") == 0)
		return TextureFilter_Box;
	else if(strcmp(filterName, "gaussian") == 0)
		return TextureFilter_Gaussian;
	else if(strcmp(filterName, "none") == 0)
		return TextureFilter_None;
	else
		return TextureFilter_Unknown;
}


//------------------------------------------------------------------------------
// CqTextureSampleOptions implementation

void CqTextureSampleOptions::adjustSampleQuad(SqSampleQuad& quad) const
{
	if(m_sWidth != 1 || m_tWidth != 1)
	{
		CqVector2D center = quad.center();
		TqFloat csWeighted = (1-m_sWidth)*center.x();
		TqFloat ctWeighted = (1-m_tWidth)*center.y();
		// Expand v1...v4 away from the quad center by multiplying the x
		// and y components of the vectors which point from the quad center to
		// the vertices by the s and t widths respectively.
		quad.v1.x(m_sWidth*quad.v1.x() + csWeighted);
		quad.v1.y(m_tWidth*quad.v1.y() + ctWeighted);
		quad.v2.x(m_sWidth*quad.v2.x() + csWeighted);
		quad.v2.y(m_tWidth*quad.v2.y() + ctWeighted);
		quad.v3.x(m_sWidth*quad.v3.x() + csWeighted);
		quad.v3.y(m_tWidth*quad.v3.y() + ctWeighted);
		quad.v4.x(m_sWidth*quad.v4.x() + csWeighted);
		quad.v4.y(m_tWidth*quad.v4.y() + ctWeighted);
	}
}

void CqTextureSampleOptions::checkBlurAndFilter()
{
	if( m_filterType != TextureFilter_Gaussian && (m_tBlur != 0 || m_sBlur != 0) )
	{
		Aqsis::log() << warning
			<< "texture blur not supported with non-gaussian filters\n";
	}
}

} // namespace Aqsis
