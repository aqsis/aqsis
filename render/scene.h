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
		\brief Declares the scene storage class for holding Grims.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is .h included already?
#ifndef SCENE_H_INCLUDED
#define SCENE_H_INCLUDED 1

#include	"specific.h"	// Needed for namespace macros.

#include	"surface.h"

#define		_qShareName	CORE
#include	"share.h"

START_NAMESPACE(Aqsis)

class CqImageBuffer;

//----------------------------------------------------------------------
/** \class CqScene
 * Storage class for the main scene.
 */

class _qShareC	CqScene
{
	public:
	_qShareM			CqScene()	{}
						/** Destructor.
						 * Deletes any remaining primitives.
						 */
	_qShareM			~CqScene()	{
										CqBasicSurface* pSurface=m_lSurfaces.pFirst();
										while(pSurface!=0)
										{
											delete(pSurface);
											pSurface=m_lSurfaces.pFirst();
										}
									}

	_qShareM			void		AddSurface(CqBasicSurface* pSurface);
						/** Get a references to the list of surfaces.
						 */
	_qShareM			CqList<CqBasicSurface>&	lSurfaces()	{return(m_lSurfaces);}
	_qShareM			TqInt		cGPrims();
	_qShareM			void		ClearScene();

	private:
			CqList<CqBasicSurface>	m_lSurfaces;	///< The list of surfaces in this scene.
};


//-----------------------------------------------------------------------

END_NAMESPACE(Aqsis)

#endif	// !SCENE_H_INCLUDED
