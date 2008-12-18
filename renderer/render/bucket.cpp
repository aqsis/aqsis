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
		\brief Implements the CqBucket class responsible for bookkeeping the primitives and storing the results.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/


#include	"aqsis.h"

#ifdef WIN32
#include    <windows.h>
#endif
#include	<cstring>
#include	<algorithm>
#include	<valarray>

#include	"aqsismath.h"
#include	"surface.h"
#include	"imagepixel.h"
#include	"occlusion.h"
#include	"renderer.h"
#include	"micropolygon.h"
#include	"imagebuffer.h"

#include	"bucket.h"
#include	"stats.h"

#include	<algorithm>
#include	<valarray>
#include	<cmath>


namespace Aqsis {


//----------------------------------------------------------------------
CqBucket::CqBucket() : m_bProcessed(false)
{
}

//----------------------------------------------------------------------
TqInt CqBucket::getCol() const
{
	return m_col;
}

//----------------------------------------------------------------------
void CqBucket::setCol(TqInt value)
{
	m_col = value;
}

//----------------------------------------------------------------------
TqInt CqBucket::getRow() const
{
	return m_row;
}

//----------------------------------------------------------------------
void CqBucket::setRow(TqInt value)
{
	m_row = value;
}

//----------------------------------------------------------------------
/** Mark this bucket as processed
 */
void CqBucket::SetProcessed( bool bProc )
{
	assert( !bProc || (bProc && !hasPendingSurfaces()) );
	m_bProcessed = bProc;
}


//----------------------------------------------------------------------
/** Initialise the static image storage area.
 */

void CqBucket::PrepareBucket( const CqVector2D& bucketPos, const CqVector2D& bucketSize,
			      TqInt pixelXSamples, TqInt pixelYSamples, TqFloat filterXWidth, TqFloat filterYWidth,
			      bool fJitter)
{
	m_PixelXSamples = pixelXSamples;
	m_PixelYSamples = pixelYSamples;
	m_FilterXWidth = filterXWidth;
	m_FilterYWidth = filterYWidth;

	m_DRegion = CqRegion( bucketPos, bucketPos + bucketSize );

	m_DiscreteShiftX = lfloor(m_FilterXWidth/2.0f);
	m_DiscreteShiftY = lfloor(m_FilterYWidth/2.0f);

	TqFloat sminx = lfloor(bucketPos.x()) - m_DiscreteShiftX;
	TqFloat sminy = lfloor(bucketPos.y()) - m_DiscreteShiftY;
	TqFloat smaxx = sminx + bucketSize.x() + (m_DiscreteShiftX*2);
	TqFloat smaxy = sminy + bucketSize.y() + (m_DiscreteShiftY*2);
	m_SRegion = CqRegion( sminx, sminy, smaxx, smaxy );
}


//----------------------------------------------------------------------
/** Check if there are any surfaces in this bucket to be processed.
 */
bool CqBucket::hasPendingSurfaces() const
{
	return ! m_gPrims.empty();
}


//----------------------------------------------------------------------
/** Add an MP to the list of deferred MPs.
 */
void CqBucket::AddMP( boost::shared_ptr<CqMicroPolygon>& pMP )
{
	m_micropolygons.push_back( pMP );
}


//---------------------------------------------------------------------
/* Pure virtual destructor for CqBucket
 */
CqBucket::~CqBucket()
{
}

} // namespace Aqsis


