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
 * \brief Declare a filtered texture mapping class
 *
 * \author Chris Foster [ chris42f (at) gmail (dot) com ]
 */

#include "texturemap2.h"

#include "vector3d.h"
#include "matrix.h"

namespace Aqsis
{

//------------------------------------------------------------------------------
// Implementation of CqTextureMap2

CqTextureMap2::CqTextureMap2(const std::string& fileName)
	: m_fileName(fileName)
{
}

void CqTextureMap2::Open()
{
	// \todo implementation
}

void CqTextureMap2::Close()
{
	// \todo implementation
}

void CqTextureMap2::PrepareSampleOptions(std::map<std::string, IqShaderData*>& paramMap )
{
	// \todo implementation
}

void CqTextureMap2::SampleMap(TqFloat s1, TqFloat t1, TqFloat swidth, TqFloat twidth,
		std::valarray<TqFloat>& val)
{
	// \todo implementation
}

void CqTextureMap2::SampleMap(TqFloat s1, TqFloat t1, TqFloat s2, TqFloat t2,
		TqFloat s3, TqFloat t3, TqFloat s4, TqFloat t4,
		std::valarray<TqFloat>& val )
{
	// \todo implementation
}

inline CqMatrix& CqTextureMap2::GetMatrix(TqInt which, TqInt index)
{
	// This matrix isn't incredibly meaningful for normal texture maps.
	// However some of the "stupid RAT tricks" have used it IIRC.
	static CqMatrix unused;
	return unused;
}

} // namespace Aqsis
