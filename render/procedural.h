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
		CqProcedural( CqBound& B);
                virtual TqInt   Split( std::vector<CqBasicSurface*>& );
		virtual ~CqProcedural();

		//---------------------------------------------- Inlined Public Methods
	public:
		CqBound	Bound() const
		{
			return m_Bound;
		};
		/**  We have no actual geometry to dice.
		 **/
		virtual TqBool Diceable()
		{
			return TqFalse;
		}
		virtual CqMicroPolyGridBase* Dice()
		{
			return NULL;
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
			/// \todo Must work out what this value should be.
			return ( 0 );
		}
		//------------------------------------------------------ Protexted
	protected:
		/* Contexy saved when the Procedural was declared */
		CqModeBlock *m_pconStored;
		// Implementation specific portion of Split()
		virtual void SplitProcedural(void) = 0;
};



/**
 * \class CqProcDelayedReadArchive
 *
 * Procedural primtive implementing DelayedReadArchive
 */
class CqProcDelayedReadArchive : public  CqProcedural
{
		//------------------------------------------------------ Public Methods
	public:
		CqProcDelayedReadArchive();
		CqProcDelayedReadArchive( CqBound& B, CqString& filename);
		virtual ~CqProcDelayedReadArchive();
		//---------------------------------------------- Inlined Public Methods
	public:
		/** Returns a string name of the class. */
		virtual CqString strName() const
		{
			return "CqProcDelayedReadArchive";
		}
		//------------------------------------------------------ Protexted
	protected:
		// Implementation specific portion of Split()
		CqString m_strFileName;
		virtual void SplitProcedural(void);

};



END_NAMESPACE( Aqsis )
#endif
