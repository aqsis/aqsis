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

#include  <time.h>
#include  <iostream>
#include	"ri.h"

#include	"aqsis.h"

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
   Class containing statistics information.
 
   Before a rendering session the method Initialise() has to be called
	 (it is also called by the constructor). Before each individual frame 
	 the variables have to be reset by calling InitialiseFrame().
	 After that the counters can be increased by calling the appropriate
	 IncXyz()-Method. To measure various times there are several pairs
	 of StartXyzTimer() and StopXyZTimer() methods.
	 The statistics for each frame can be printed with PrintStats().
 */

class CqStats
{
	public:
					CqStats()	{ Initialise(); }

					~CqStats()	{ }

			void Initialise();
			void InitialiseFrame();

					/** Get the process identifier.
					 */
			EqState	State() const	{return(m_State);}
					/** Set the current process.
					 */
			void	SetState(const EqState State)
									{m_State=State;}

					/** Get the percentage complete.
					 */
			TqFloat	Complete() const	{return(m_Complete);}
					/** Set the percentage complete.
					 */
			void	SetComplete(TqFloat complete)
										{m_Complete=complete;}

      /// \name Increasing counters
			//@{

					/** Increase the micropolygons allocated count by 1.
					 */
			void	IncMPGsAllocated()	{
											m_cMPGsAllocated++;
											m_cMPGsCurrent++;
											m_cMPGsPeak=(m_cMPGsCurrent>m_cMPGsPeak)?m_cMPGsCurrent:m_cMPGsPeak;
										}
					/** Increase the micropolygons deallocated count by 1.
					 */
			void	IncMPGsDeallocated()	{ 
												m_cMPGsDeallocated++; 
												m_cMPGsCurrent--;
											}
					/** Increase the sample count by 1.
					 */
			void	IncSamples()			{ m_cSamples++; }
					/** Increase the sample bound hit count by 1.
					 */
			void	IncSampleBoundHits()	{ m_cSampleBoundHits++; }
					/** Increase the sample hit count by 1.
					 */
			void	IncSampleHits()			{ m_cSampleHits++; }
					/** Increase the shader variables allocated count by 1.
					 */
			void	IncVariablesAllocated()	{ 
												m_cVariablesAllocated++; 
												m_cVariablesCurrent++;
												m_cVariablesPeak=(m_cVariablesCurrent>m_cVariablesPeak)?m_cVariablesCurrent:m_cVariablesPeak;
											}
					/** Increase the shader variables deallocated count by 1.
					 */
			void	IncVariablesDeallocated(){
												m_cVariablesDeallocated++; 
												m_cVariablesCurrent--;
											}
					/** Increase the surface parameters allocated count by 1.
					 */
			void	IncParametersAllocated() { 
												m_cParametersAllocated++;
												m_cParametersCurrent++;
												m_cParametersPeak=(m_cParametersCurrent>m_cParametersPeak)?m_cParametersCurrent:m_cParametersPeak;
											}
					/** Increase the surface parameters deallocated count by 1.
					 */
			void	IncParametersDeallocated(){ 
												m_cParametersDeallocated++; 
												m_cParametersCurrent--;
											}
					/** Increase the micropolygrids allocated count by 1.
					 */
			void	IncGridsAllocated()		{
												m_cGridsAllocated++; 
												m_cGridsCurrent++;
												m_cGridsPeak=(m_cGridsCurrent>m_cGridsPeak)?m_cGridsCurrent:m_cGridsPeak;
											}
					/** Increase the micropolygrids deallocated count by 1.
					 */
			void	IncGridsDeallocated()	{
												m_cGridsDeallocated++; 
												m_cGridsCurrent--;
											}
					/** Increase the GPrim count by 1.
					    These counter should only be increased when adding a toplevel gprim
							(e.g. in the Ri...() calls).
					 */
			void	IncGPrims()			{ m_cGPrims++; }
					/** Increase the total GPrim count by 1.
					    Here \em every gprim is counted (including those resulting from a split).
					 */
			void	IncTotalGPrims()	{ m_cTotalGPrims++; }
					/** Decrease the total GPrim count by 1.
					    This method should be called when a gprim is split since the
							gprim will be replaced by a number of smaller gprims and therefore 
							shouldn't appear in the statistics.
					 */
			void	DecTotalGPrims()	{ m_cTotalGPrims--; }
					/** Increase the culled GPrim count by 1.
					 */
			void	IncCulledGPrims()	{ m_cCulledGPrims++; }
					/** Increase the culled micropoly grid count by 1.
					 */
			void	IncCulledGrids()	{ m_cCulledGrids++; }
					/** Increase the culled micropoly count by n.
					    This counter should only be increased if the according grid
							was \em not culled.
					 */
			void	IncCulledMPGs(TqInt n=1)	{ m_cCulledMPGs+=n; }

			//@}


      /// \name Timer methods
      //@{
      void    StartFrameTimer();
			void    StopFrameTimer();

          /** Start the surface timer.
					    \see StopSurfaceTimer()
		    	 */
      void    StartSurfaceTimer() { m_dummytime = clock(); };

          /** Stop the surface timer.
					    The difference between the starting time and the current time is
							added to the total surface shading time.
					    \see StartSurfaceTimer()
		    	 */
			void    StopSurfaceTimer()  { m_timeSurface += clock()-m_dummytime; }

          /** Start the displacement timer.
					    \see StopDisplacementTimer()
		    	 */
      void    StartDisplacementTimer() { m_dummytime = clock(); };

          /** Stop the displacement timer.
					    The difference between the starting time and the current time is
							added to the total displacement shading time.
					    \see StartDisplacementTimer()
		    	 */
			void    StopDisplacementTimer()  { m_timeDisplacement += clock()-m_dummytime; }

          /** Start the atmosphere timer.
					    \see StopAtmosphereTimer()
		    	 */
      void    StartAtmosphereTimer() { m_dummytime = clock(); };

          /** Stop the atmosphere timer.
					    The difference between the starting time and the current time is
							added to the total atmosphere shading time.
					    \see StartAtmosphereTimer()
		    	 */
			void    StopAtmosphereTimer()  { m_timeAtmosphere += clock()-m_dummytime; }
			//@}


			void    PrintStats(TqInt level) const;

  private:
	    std::ostream& TimeToString(std::ostream& os, TqFloat t) const;

	private:
			EqState	m_State;						///< Current process identifier.
			TqFloat	m_Complete;						///< Current percentage comlpete.
			
			TqInt	m_cMPGsAllocated;				///< Count of micropolygons allocated.
			TqInt	m_cMPGsDeallocated;				///< Count of microplygons dallocated.
			TqInt	m_cMPGsCurrent;					///< Current count of allocated MPGs.
			TqInt	m_cMPGsPeak;					///< Peak count of allocated MPGs.
			TqInt	m_cSamples;						///< Count of samples tested.
			TqInt	m_cSampleBoundHits;				///< Count of sample boundary hits.
			TqInt	m_cSampleHits;					///< Count of sample micropolygon hits.
			TqInt	m_cVariablesAllocated;			///< Count of shader variables allocated.
			TqInt	m_cVariablesDeallocated;		///< Count of shader variables deallocated.
			TqInt	m_cVariablesCurrent;			///< Current count of variables allocated.
			TqInt	m_cVariablesPeak;				///< Peak count of variables allocated.
			TqInt	m_cParametersAllocated;			///< Count of surface parameters allocated.
			TqInt	m_cParametersDeallocated;		///< Count of surface parameters deallocated.
			TqInt	m_cParametersCurrent;			///< Current count of parameters allocated.
			TqInt	m_cParametersPeak;				///< Peak count of parameters allocated.
			TqInt	m_cGridsAllocated;				///< Count of micropolygrids allocated.
			TqInt	m_cGridsDeallocated;			///< Count of micropolygrids deallocated.
			TqInt	m_cGridsCurrent;				///< Current count of grids allocated.
			TqInt	m_cGridsPeak;					///< Peak count of grids allocated.
			TqInt	m_cGPrims;						///< Count of GPrims.
			TqInt	m_cTotalGPrims;					///< Count of total GPrims (including gprims resulting from splits).
			TqInt	m_cCulledGPrims;				///< Count of culled GPrims.
			TqInt	m_cCulledGrids;					///< Count of culled micro poly grids.
			TqInt	m_cCulledMPGs;					///< Count of culled micro polys.

			time_t	m_timeTotal;				 ///< Total time spent on the entire animation.
			time_t  m_timeTotalFrame;    ///< Time spent on processing one individual frame.
			TqBool  m_frameTimerRunning; ///< True, if the frame timer was started and not yet stopped.
			clock_t m_timeSurface;       ///< Time spent on surface shading.
			clock_t m_timeDisplacement;  ///< Time spent on displacement shading.
			clock_t m_timeAtmosphere;    ///< Time spent on volume shading (atmosphere).
      clock_t m_dummytime;         ///< Variable that keeps starting times for surface, displacement and atmosphere timers.
};


//-----------------------------------------------------------------------

END_NAMESPACE(Aqsis)

#endif	// !STATS_H_INCLUDED
