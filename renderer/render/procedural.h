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

/**
        \file
        \brief Declares the classes and support structures for 
                handling RenderMan Procedurals primitives.
        \author Jonathan Merritt (j.merritt@pgrad.unimelb.edu.au)
*/

#ifndef PROCEDURAL_H_INCLUDED
#define PROCEDURAL_H_INCLUDED

#include        "aqsis.h"
#include        "matrix.h"
#include        "surface.h"

START_NAMESPACE( Aqsis )


/**
 * \class CqProcedural
 * 
 * Abstract base class for all Procedural objects, providing the basic state
 * management.
 */
class CqProcedural : public CqSurface
{
		//------------------------------------------------------ Public Methods
	public:
		CqProcedural();
		CqProcedural(RtPointer, CqBound&, RtProcSubdivFunc, RtProcFreeFunc);
		/** Split this GPrim into a number of other GPrims.
		 * \param aSplits A reference to a CqSurface array to fill in with the new GPrim pointers.
		 * \return Integer count of new GPrims created.
		 */
		virtual	TqInt	Split( std::vector<boost::shared_ptr<CqSurface> >& aSplits );
		virtual ~CqProcedural();

		//---------------------------------------------- Inlined Public Methods
	public:
		CqBound	Bound() const
		{
			return m_Bound;
		};
		virtual void    Transform( const CqMatrix& matTx, const CqMatrix& matITTx, const CqMatrix& matRTx, TqInt iTime = 0 );
		/*  We have no actual geometry to dice.
		 */
		virtual TqBool Diceable()
		{
			return TqFalse;
		}
		virtual CqMicroPolyGridBase* Dice()
		{
			return NULL;
		}

		/** Determine whether the passed surface is valid to be used as a
		 *  frame in motion blur for this surface.
		 */
		virtual TqBool	IsMotionBlurMatch( CqSurface* pSurf )
		{
			return( TqFalse );
		}

		/** Returns a string name of the class. */
		virtual CqString strName() const
		{
			return "CqProcedural";
		}
		virtual TqUint  cUniform() const
		{
			return ( 0 );
		}
		virtual TqUint  cVarying() const
		{
			return ( 0 );
		}
		virtual TqUint  cVertex() const
		{
			return ( 0 );
		}
		virtual TqUint  cFaceVarying() const
		{
			return ( 0 );
		}
		virtual CqSurface* Clone() const
		{
			//return(new CqProcedural(*this));
			return(NULL);
		}
		//------------------------------------------------------ Protexted
	protected:
		/* Contexy saved when the Procedural was declared */
		boost::shared_ptr<CqModeBlock> m_pconStored;

		/* The RIB request data */
		RtPointer m_pData;
		RtProcSubdivFunc m_pSubdivFunc;
		RtProcFreeFunc m_pFreeFunc;

};

// The built in RiProcedurals
extern "C" RtVoid  RiProcFree( RtPointer data );
extern "C" RtVoid  RiProcDelayedReadArchive( RtPointer data, RtFloat detail );
extern "C" RtVoid  RiProcRunProgram( RtPointer data, RtFloat detail );
extern "C" RtVoid  RiProcDynamicLoad( RtPointer data, RtFloat detail );

END_NAMESPACE( Aqsis )

#endif // PROCEDURAL_H_INCLUDED

