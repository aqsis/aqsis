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
		\brief Declares CqStats class for holding global renderer statistics information.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is .h included already?
#ifndef STATS_H_INCLUDED
#define STATS_H_INCLUDED 1

#include	"ri.h"

#include	"specific.h"	// Needed for namespace macros.

START_NAMESPACE(Aqsis)

//----------------------------------------------------------------------
/** \enum EqState
 * Current process identifiers.
 */

enum EqState
{
	State_Parsing,		///< Parsing a RIB file.
	State_Shadows,		///< Processing shadows.
	State_Rendering,	///< Rendering image.
	State_Complete,		///< Rendering complete.
};


//----------------------------------------------------------------------
/** \class CqStats
 * Class containing statistics information.
 */

class CqStats
{
	public:
					CqStats()	: m_State(State_Parsing),
								  m_Complete(0.0f),
								  m_cMPGsAllocated(0),
								  m_cMPGsDeallocated(0),
								  m_cSamples(0),
								  m_cSampleHits(0),
								  m_cVariablesAllocated(0),
								  m_cVariablesDeallocated(0),
								  m_cParametersAllocated(0),
								  m_cParametersDeallocated(0),
								  m_cGridsAllocated(0),
								  m_cGridsDeallocated(0)
								{}
					~CqStats()	{}

					/** Get the process identifier.
					 */
			EqState	State() const	{return(m_State);}
					/** Set the current process.
					 */
			void	SetState(const EqState State)
									{m_State=State;}

					/** Get the precentage complete.
					 */
			TqFloat	Complete() const	{return(m_Complete);}
					/** Set the percentage complete.
					 */
			void	SetComplete(TqFloat complete)
										{m_Complete=complete;}

					/** Get a reference to the micropolygons allocated count.
					 */
			TqInt&	cMPGsAllocated()	{return(m_cMPGsAllocated);}
					/** Get a reference to the micropolygons deallocated count.
					 */
			TqInt&	cMPGsDeallocated()	{return(m_cMPGsDeallocated);}
					/** Get a reference to the samples count.
					 */
			TqInt&	cSamples()			{return(m_cSamples);}
					/** Get a reference to the sample bound hit count.
					 */
			TqInt&	cSampleBoundHits()	{return(m_cSampleBoundHits);}
					/** Get a reference to the sample hit count.
					 */
			TqInt&	cSampleHits()		{return(m_cSampleHits);}
					/** Get a reference to the shader variables allocated count.
					 */
			TqInt&	cVariablesAllocated()	{return(m_cVariablesAllocated);}
					/** Get a reference to the shader variables deallocated count.
					 */
			TqInt&	cVariablesDeallocated()	{return(m_cVariablesDeallocated);}
					/** Get a reference to the surface parameters allocated count.
					 */
			TqInt&	cParametersAllocated()	{return(m_cParametersAllocated);}
					/** Get a reference to the surface parameters deallocated count.
					 */
			TqInt&	cParametersDeallocated(){return(m_cParametersDeallocated);}
					/** Get a reference to the micropolygrids allocated count.
					 */
			TqInt&	cGridsAllocated()	{return(m_cGridsAllocated);}
					/** Get a reference to the micropolygrids deallocated count.
					 */
			TqInt&	cGridsDeallocated()	{return(m_cGridsDeallocated);}

	private:
			EqState	m_State;						///< Current process identifier.
			TqFloat	m_Complete;						///< Current percentage comlpete.
			
			TqInt	m_cMPGsAllocated;				///< Count of microplygons allocated.
			TqInt	m_cMPGsDeallocated;				///< Count of microplygons dallocated.
			TqInt	m_cSamples;						///< Count of samples tested.
			TqInt	m_cSampleBoundHits;				///< Count of sample boundary hits.
			TqInt	m_cSampleHits;					///< Count of sample micropolygon hits.
			TqInt	m_cVariablesAllocated;			///< Count of shader variables allocated.
			TqInt	m_cVariablesDeallocated;		///< Count of shader variables deallocated.
			TqInt	m_cParametersAllocated;			///< Count of surface parameters allocated.
			TqInt	m_cParametersDeallocated;		///< Count of surface parameters deallocated.
			TqInt	m_cGridsAllocated;				///< Count of micropolygrids allocated.
			TqInt	m_cGridsDeallocated;			///< Count of micropolygrids allocated.
};


//-----------------------------------------------------------------------

END_NAMESPACE(Aqsis)

#endif	// !STATS_H_INCLUDED
