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

#include  <strstream>
#include  <math.h>

#include	"aqsis.h"
#include  "renderer.h"
#include  "attributes.h"
#include	"stats.h"
#include  "messages.h"

START_NAMESPACE(Aqsis)


/**
   Initialise every variable.

   This method has to be called whenever a new render session starts
	 (e.g. when an entirely new RIB file is read).
	 It is also called from inside the constructor and it calls
	 InitialiseFrame() as well.

	 \see InitialiseFrame()
 */
void CqStats::Initialise()
{
  m_State     = State_Parsing; 
  m_Complete  = 0.0f;
	m_timeTotal = 0;
	InitialiseFrame();
}


/**
   Initialise all variables before processing the next frame.

   This method resets all variables that contain information
	 specific to one individual frame. It has to be called every
	 time a new frame is rendered (in RiFrameBegin()).

	 \see Initialise()
 */
void CqStats::InitialiseFrame()
{
	m_cMPGsAllocated         = 0;
	m_cMPGsDeallocated       = 0;
	m_cSamples               = 0;
	m_cSampleBoundHits       = 0;
	m_cSampleHits            = 0;
	m_cVariablesAllocated    = 0;
	m_cVariablesDeallocated  = 0;
	m_cParametersAllocated   = 0;
	m_cParametersDeallocated = 0;
	m_cGridsAllocated        = 0;
	m_cGridsDeallocated      = 0;
	m_cGPrims                = 0;

  m_timeTotalFrame         = 0;
	m_frameTimerRunning      = TqFalse;
	m_timeSurface            = 0;
	m_timeDisplacement       = 0;
	m_timeAtmosphere         = 0;
	m_dummytime              = 0;
}

/** Start the frame timer.

    If the timer was already running, nothing happens (so it is safe
		to call it in RiFrameBegin() as well as in RiWorldBegin()).

   \see StopFrameTimer()
 */
void CqStats::StartFrameTimer() 
{ 
  if (!m_frameTimerRunning)
	{
		m_frameTimerRunning = TqTrue;
		m_timeTotalFrame    = time(0);
	}
}

/** Stop the frame timer.
    The difference between the starting time and the current time is
		stored and also added to the total time.

    \see StartFrameTimer()
 */
void CqStats::StopFrameTimer()
{ 
  m_timeTotalFrame     = time(0)-m_timeTotalFrame; 
	m_timeTotal         += m_timeTotalFrame;
  m_frameTimerRunning  = TqFalse;
}


//----------------------------------------------------------------------
/** Output rendering stats if required.

    \param level  Verbosity level as set by Options "statistics" "endofframe"
 */

void CqStats::PrintStats(TqInt level) const
{
	if(level>0)
	{
		std::strstream MSG;

    TqFloat timeSurface      = static_cast<TqFloat>(m_timeSurface)/CLOCKS_PER_SEC;
    TqFloat timeDisplacement = static_cast<TqFloat>(m_timeDisplacement)/CLOCKS_PER_SEC;
    TqFloat timeAtmosphere   = static_cast<TqFloat>(m_timeAtmosphere)/CLOCKS_PER_SEC;

    MSG << "Total render time   : ";
		TimeToString(MSG,m_timeTotal) << std::endl;
    MSG << "Last frame          : ";
    TimeToString(MSG,m_timeTotalFrame) << std::endl;

    MSG << "Surface shading     : ";
		TimeToString(MSG,timeSurface) << " (" << 100.0f*timeSurface/m_timeTotalFrame << "%)" << std::endl;
    MSG << "Displacement shading: ";
		TimeToString(MSG,timeDisplacement) << " (" << 100.0f*timeDisplacement/m_timeTotalFrame << "%)" << std::endl;
    MSG << "Atmosphere shading  : ";
		TimeToString(MSG,timeAtmosphere) << " (" << 100.0f*timeAtmosphere/m_timeTotalFrame << "%)" << std::endl;
		MSG << std::endl;

		MSG << "Grids:    \t" << m_cGridsAllocated << " created / ";
		MSG << m_cGridsAllocated-m_cGridsDeallocated << " remaining" << std::endl;

		MSG << "Micropolygons: \t" << m_cMPGsAllocated << " created / ";
		MSG << m_cMPGsAllocated-m_cMPGsDeallocated << " remaining" << std::endl;

		MSG << "Sampling: \t" << m_cSamples << " samples" << std::endl;
		MSG << "          \t" << m_cSampleBoundHits << " bound hits (";
		MSG << (100.0f/m_cSamples)*m_cSampleBoundHits << "% of samples)" << std::endl;
		MSG << "          \t" << m_cSampleHits << " hits (";
		MSG << (100.0f/m_cSamples)*m_cSampleHits << "% of samples)" << std::endl;

		MSG << "GPrims: \t" << m_cGPrims << std::endl;

		MSG << "Attributes: \t";
		MSG << (TqInt)Attribute_stack.size() << " created" << std::endl;

		MSG << "Transforms: \t";
		MSG << QGetRenderContext()->TransformStack().size() << " created" << std::endl;

		MSG << "Variables: \t";
		MSG << m_cVariablesAllocated << " created / ";
		MSG << m_cVariablesAllocated-m_cVariablesDeallocated << " remaining" << std::endl;

		MSG << "Parameters: \t" << m_cParametersAllocated << " created / ";
		MSG << m_cParametersAllocated-m_cParametersDeallocated << " remaining" << std::endl;
		MSG << std::ends;

		CqString strMSG(MSG.str());
		CqBasicError(0,Severity_Normal,strMSG.c_str());
	}
}


/** Convert a time value into a string.

    \param os  Output stream
		\param t   Time value (in seconds).
		\return  os
 */
std::ostream& CqStats::TimeToString(std::ostream& os, TqFloat t) const
{
    // Is the time negative? Then there's a bug somewhere.
    if (t<0.0)
		{
		  os << "<invalid>";
			return os;
		}

    // Round the time if it's more than 5sec
    if (t>5.0)  t = fmod(t,1)<0.5? floor(t) : ceil(t);

		TqInt   h = t/(60*60);
		TqInt   m = (t/60)-(h*60);
		TqFloat s = (t)-(h*60*60)-(m*60);
		if (h>0) os << h << "hrs ";
		if (m>0) os << m << "mins ";
		os << s << "secs";
		return os;
}


//---------------------------------------------------------------------
 
END_NAMESPACE(Aqsis)

