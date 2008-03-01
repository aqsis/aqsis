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
#include "texfileheader.h"

namespace Aqsis
{

//------------------------------------------------------------------------------
// free function implementations
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

void CqTextureSampleOptions::fillFromFileHeader(const CqTexFileHeader& header)
{
	/// \todo Find a way to store & retrieve the downsampling filter?
	const SqWrapModes* wrapModes = header.findPtr<Attr::WrapModes>();
	if(wrapModes)
	{
		m_sWrapMode = wrapModes->sWrap;
		m_tWrapMode = wrapModes->tWrap;
	}
}

} // namespace Aqsis
