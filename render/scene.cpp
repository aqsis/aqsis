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
		\brief Implements the scene storage class for holding Grims.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include	"aqsis.h"
#include	"scene.h"
#include	"shaderstack.h"

START_NAMESPACE(Aqsis)

//----------------------------------------------------------------------
/** Add a new surface to the scene.
 */

void CqScene::AddSurface(CqBasicSurface* pSurface)
{
	m_lSurfaces.LinkLast(pSurface);
}


//----------------------------------------------------------------------
/** Clear all surfaces from the scene.
 */

void CqScene::ClearScene()
{
	CqBasicSurface* pSurface=m_lSurfaces.pFirst();
	while(pSurface)
	{
		delete(pSurface);
		pSurface=m_lSurfaces.pFirst();
	}
}


//----------------------------------------------------------------------
/** Clear all surfaces from the scene.
 */

TqInt CqScene::cGPrims()
{
	TqInt cGPrims=0;
	CqBasicSurface* pSurface=m_lSurfaces.pFirst();
	while(pSurface)
	{
		cGPrims++;
		pSurface=pSurface->pNext();
	}
	return(cGPrims);
}


END_NAMESPACE(Aqsis)
//---------------------------------------------------------------------
