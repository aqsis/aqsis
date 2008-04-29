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
 * \brief Implementation of operations on sampling quadrilaterals.
 *
 * \author Chris Foster [ chris42f (at) gmail (dot) com ]
 */

#include "samplequad.h"

#include "matrix.h"

namespace Aqsis {

//------------------------------------------------------------------------------
// SqSampleQuad implementation
void SqSampleQuad::scaleWidth(TqFloat xWidth, TqFloat yWidth)
{
	if(xWidth != 1 || yWidth != 1)
	{
		CqVector2D c = center();
		TqFloat cxWeighted = (1 - xWidth)*c.x();
		TqFloat cyWeighted = (1 - yWidth)*c.y();
		// Expand v1...v4 away from the quad center by multiplying the x
		// and y components of the vectors which point from the quad center to
		// the vertices by the x and y widths respectively.
		v1.x(xWidth*v1.x() + cxWeighted);
		v1.y(yWidth*v1.y() + cyWeighted);
		v2.x(xWidth*v2.x() + cxWeighted);
		v2.y(yWidth*v2.y() + cyWeighted);
		v3.x(xWidth*v3.x() + cxWeighted);
		v3.y(yWidth*v3.y() + cyWeighted);
		v4.x(xWidth*v4.x() + cxWeighted);
		v4.y(yWidth*v4.y() + cyWeighted);
	}
}


//------------------------------------------------------------------------------
// Sq3DSampleQuad implementation
void Sq3DSampleQuad::transform(const CqMatrix& mat)
{
	v1 = mat*v1;
	v2 = mat*v2;
	v3 = mat*v3;
	v4 = mat*v4;
}


} // namespace Aqsis
