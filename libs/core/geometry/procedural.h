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

/**
        \file
        \brief Declares the classes and support structures for 
                handling RenderMan Procedurals primitives.
        \author Jonathan Merritt (j.merritt@pgrad.unimelb.edu.au)
*/

#ifndef PROCEDURAL_H_INCLUDED
#define PROCEDURAL_H_INCLUDED

#include <aqsis/aqsis.h>

#include <map>

#include <boost/shared_ptr.hpp>

#include <aqsis/math/matrix.h>
#include <aqsis/util/popen.h>
#include "surface.h"

namespace Aqsis {


/** \brief Class to store RiProcedural() arguments before the procedural is
 * called to generate geometry.
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
		void	Bound(CqBound* bound) const
		{
			bound->vecMin() = m_Bound.vecMin();
			bound->vecMax() = m_Bound.vecMax();

			AdjustBoundForTransformationMotion( bound );
		};
		virtual void    Transform( const CqMatrix& matTx, const CqMatrix& matITTx, const CqMatrix& matRTx, TqInt iTime = 0 );
		/*  We have no actual geometry to dice.
		 */
		virtual bool Diceable(const CqMatrix& /*matCtoR*/)
		{
			return false;
		}
		virtual CqMicroPolyGridBase* Dice()
		{
			return NULL;
		}

		/** Determine whether the passed surface is valid to be used as a
		 *  frame in motion blur for this surface.
		 */
		virtual bool	IsMotionBlurMatch( CqSurface* pSurf )
		{
			return( false );
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


//------------------------------------------------------------------------------
/** \brief Manager for child process streams created by RiProcRunProgram invocations.
 *
 * The repository keeps track of a seperate child process stream for each
 * distinct RunProgram command.  These can be retrieved using the find()
 * function in order to generate further RIB.
 */
class CqRunProgramRepository
{
	public:
		/** \brief Get an iostream pipe connected to the stdin and stdout of
		 * a child RunProgram process.
		 *
		 * If no child process corresponding to 'command' is currently running,
		 * a new process is started and the stdin and stdout connected to a
		 * pipe stream which is returned.
		 *
		 * If an error occurs during creation of the child process, an
		 * XqEnvironment exception will be throw.  Subsequent calls to find()
		 * will then result in a null pointer being returned.  In addition,
		 * streams which have their eof() or fail() bits set will be deleted
		 * and a null pointer returned.
		 *
		 * \param command - command line for the child process.  The command
		 * line will be split up into arguments delimited by whitespace, with
		 * the fist argument the name of the program to run.  No escaping
		 * mechanism for whitespace is currently supported.  The program is
		 * searched for in the procedural searchpath, with the system path as a
		 * fallback.
		 */
		std::iostream* find(const std::string& command);

	private:
		typedef boost::shared_ptr<TqPopenStream> TqPopenStreamPtr;
		typedef std::map<std::string, TqPopenStreamPtr> TqRunProgramMap;

		static void splitCommandLine(const std::string& command,
				std::vector<std::string>& argv);

		TqPopenStream* startNewRunProgram(const std::string& command);

		/// Set of active pipes for child processes.
		TqRunProgramMap m_activeRunPrograms;
};


} // namespace Aqsis

// The built in RiProcedurals
extern "C" RtVoid  RiProcFree( RtPointer data );
extern "C" RtVoid  RiProcDelayedReadArchive( RtPointer data, RtFloat detail );
extern "C" RtVoid  RiProcRunProgram( RtPointer data, RtFloat detail );
extern "C" RtVoid  RiProcDynamicLoad( RtPointer data, RtFloat detail );


#endif // PROCEDURAL_H_INCLUDED

