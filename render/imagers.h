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
		\brief Declares the classes for handling RenderMan imagersources, plus any built in sources.
		\author Michel Joron (joron@sympatico.ca)
*/

//? Is .h included already?
#ifndef IMAGERS_H_INCLUDED 
//{
#define IMAGERS_H_INCLUDED 1

#include	<vector>

#include	"aqsis.h"

#include "ri.h"
#include "color.h"
#include "list.h"
#include "matrix.h"
#include "shadervm.h"
#include "shaderexecenv.h"
#include "vector4d.h"
#include "imagebuffer.h"
#include "version.h"

#define		_qShareName	CORE
#include	"share.h"

START_NAMESPACE( Aqsis )

//----------------------------------------------------------------------
/** \class CqImagersource
 * Abstract base class from which all Imagersources are derived.
 */

class CqImagersource : public CqListEntry<CqImagersource>, public CqShaderExecEnv
{
	public:
		CqImagersource( IqShader* pShader, TqBool fActive = TqTrue );
		virtual	~CqImagersource();

		/** Get a pointer to the associated Imagersource shader.
		 * \return a pointer to a CqShader derived class.
		 */
		IqShader*	pShader()
		{
			return ( m_pShader );
		}

		/** Get a pointer to the associated Imagersource shader.
		 * \return a pointer to a IqShader derived class.
		 */
		const IqShader*	pShader() const
		{
			return ( m_pShader );
		}

		/** Initialise the shader execution environment & execute the
		 *  shader code.
		 * \param uGridRes,vGridRes Integer grid size.
		 * \param x,y Floating value about the Origin of the bucket.
		 */
		void	Initialise( TqInt uGridRes, TqInt vGridRes,
		                 TqFloat x, TqFloat y,
		                 CqColor *color, CqColor *opacity,
		                 TqFloat *depth, TqFloat *coverage );



	public:
		CqColor Color( TqFloat x, TqFloat y );
		CqColor Opacity( TqFloat x, TqFloat y );
		TqFloat Alpha( TqFloat x, TqFloat y );

	private:
		IqShader*	m_pShader;				///< Pointer to the associated shader.
		CqAttributes*	m_pAttributes;			///< Pointer to the associated attributes.

		TqInt m_vGridRes;						///< Size of the bucket X
		TqInt m_uGridRes;						///<                    Y in pixels
		TqInt m_uXOrigin;						///< its origin 0,0 top left corner
		TqInt m_uYOrigin;						///<                      in pixels

}
;


//--------------------
// background is hard-coded within in sync with options.cpp,options.h
//--------------------

//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

//}  // End of #ifdef IMAGERS_H_INCLUDED
#endif
