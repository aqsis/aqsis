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
#include	"renderer.h"
#include	"attributes.h"
#include	"stats.h"
#include	"messages.h"
#include	"imagebuffer.h"

START_NAMESPACE( Aqsis )


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
	m_State = State_Parsing;
	m_Complete = 0.0f;
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
	m_cMPGsAllocated = 0;
	m_cMPGsDeallocated = 0;
	m_cMPGsCurrent	= 0;
	m_cMPGsPeak = 0;
	m_cSamples = 0;
	m_cSampleBoundHits = 0;
	m_cSampleHits = 0;
	m_cVariablesAllocated = 0;
	m_cVariablesDeallocated = 0;
	m_cVariablesCurrent = 0;
	m_cVariablesPeak = 0;
	m_cParametersAllocated = 0;
	m_cParametersDeallocated = 0;
	m_cParametersCurrent = 0;
	m_cParametersPeak = 0;
	m_cGridsAllocated = 0;
	m_cGridsDeallocated = 0;
	m_cGridsCurrent = 0;
	m_cGridsPeak = 0;
	m_cGPrims = 0;
	m_cTotalGPrims = 0;
	m_cCulledGPrims = 0;
	m_cCulledGrids = 0;
	m_cMissedMPGs = 0;
	m_cCulledMPGs = 0;
	m_cTextureMemory = 0;
	memset( m_cTextureMisses, '\0', sizeof( m_cTextureMisses ) );
	memset( m_cTextureHits, '\0', sizeof( m_cTextureHits ) );
	m_timeTotalFrame = 0;
	m_frameTimerRunning = TqFalse;
	m_timeSurface.Reset();
	m_timeDisplacement.Reset();
	m_timeImager.Reset();
	m_timeAtmosphere.Reset();
	m_timeSplits.Reset();
	m_timeDicing.Reset();
	m_timeRenderMPGs.Reset();
	m_timeOcclusionCull.Reset();
	m_timeDiceable.Reset();
	m_timeTM.Reset();
	m_timeMakeTexture.Reset();
	m_timeMakeShadow.Reset();
	m_timeMakeEnv.Reset();
	m_timeFB.Reset();
	m_timeDB.Reset();
	m_timeParse.Reset();
	m_timeProject.Reset();
	m_timeCombine.Reset();
	m_timeOthers.Reset();

}

/** Start the frame timer.
 
    If the timer was already running, nothing happens (so it is safe
		to call it in RiFrameBegin() as well as in RiWorldBegin()).
 
   \see StopFrameTimer()
 */
void CqStats::StartFrameTimer()
{
	if ( !m_frameTimerRunning )
	{
		m_frameTimerRunning = TqTrue;
		m_timeTotalFrame = time( 0 );
	}
}

/** Stop the frame timer.
    The difference between the starting time and the current time is
		stored and also added to the total time.
 
    \see StartFrameTimer()
 */
void CqStats::StopFrameTimer()
{
	m_timeTotalFrame = time( 0 ) - m_timeTotalFrame;
	m_timeTotal += m_timeTotalFrame;
	m_frameTimerRunning = TqFalse;
}


//----------------------------------------------------------------------
/** Output rendering stats if required.
 
    \param level  Verbosity level as set by Options "statistics" "endofframe"
 */

void CqStats::PrintStats( TqInt level ) const
{
	/*! Levels
		Minimum := 0
		Normal  := 1
		Verbose := 2
		Max		:= 3
	*/

	std::strstream MSG;


	TqFloat timeSurface = static_cast<TqFloat>( m_timeSurface.TimeTotal() ) / CLOCKS_PER_SEC;
	TqFloat timeDisplacement = static_cast<TqFloat>( m_timeDisplacement.TimeTotal() ) / CLOCKS_PER_SEC;
	TqFloat timeImager = static_cast<TqFloat>( m_timeImager.TimeTotal() ) / CLOCKS_PER_SEC;
	TqFloat timeAtmosphere = static_cast<TqFloat>( m_timeAtmosphere.TimeTotal() ) / CLOCKS_PER_SEC;
	TqFloat timeSplits = static_cast<TqFloat>( m_timeSplits.TimeTotal() ) / CLOCKS_PER_SEC;
	TqFloat timeDicing = static_cast<TqFloat>( m_timeDicing.TimeTotal() ) / CLOCKS_PER_SEC;
	TqFloat timeRenderMPGs = static_cast<TqFloat>( m_timeRenderMPGs.TimeTotal() ) / CLOCKS_PER_SEC;
	TqFloat timeOcclusionCull = static_cast<TqFloat>( m_timeOcclusionCull.TimeTotal() ) / CLOCKS_PER_SEC;
	TqFloat timeDiceable = static_cast<TqFloat>( m_timeDiceable.TimeTotal() ) / CLOCKS_PER_SEC;
	TqFloat timeMakeTexture = static_cast<TqFloat>( m_timeMakeTexture.TimeTotal() ) / CLOCKS_PER_SEC;
	TqFloat timeMakeShadow = static_cast<TqFloat>( m_timeMakeShadow.TimeTotal() ) / CLOCKS_PER_SEC;
	TqFloat timeMakeEnv = static_cast<TqFloat>( m_timeMakeEnv.TimeTotal() ) / CLOCKS_PER_SEC;
	TqFloat timeTM = static_cast<TqFloat>( m_timeTM.TimeTotal() ) / CLOCKS_PER_SEC;
	TqFloat timeFB = static_cast<TqFloat>( m_timeFB.TimeTotal() - m_timeImager.TimeTotal() ) / CLOCKS_PER_SEC;
	TqFloat timeDB = static_cast<TqFloat>( m_timeDB.TimeTotal() ) / CLOCKS_PER_SEC;
	TqFloat timeParse = static_cast<TqFloat>( m_timeParse.TimeTotal() ) / CLOCKS_PER_SEC;
	TqFloat timeProject = static_cast<TqFloat>( m_timeProject.TimeTotal() ) / CLOCKS_PER_SEC;
	TqFloat timeCombine = static_cast<TqFloat>( m_timeCombine.TimeTotal() ) / CLOCKS_PER_SEC;
	TqFloat timeOthers = static_cast<TqFloat>( m_timeOthers.TimeTotal() ) / CLOCKS_PER_SEC;

	//! level >= 0
	MSG << "Total render time   : ";
	TimeToString( MSG, m_timeTotal ) << std::endl;
	MSG << "Last frame          : ";
	TimeToString( MSG, m_timeTotalFrame ) << std::endl;

	if ( level >= 1 )
	{
		MSG << "Imager shading      : ";
		TimeToString( MSG, timeImager ) << " (" << 100.0f * timeImager / m_timeTotalFrame << "%)" << std::endl;
		MSG << "Surface shading     : ";
		TimeToString( MSG, timeSurface ) << " (" << 100.0f * timeSurface / m_timeTotalFrame << "%)" << std::endl;
		MSG << "Displacement shading: ";
		TimeToString( MSG, timeDisplacement ) << " (" << 100.0f * timeDisplacement / m_timeTotalFrame << "%)" << std::endl;
		MSG << "Atmosphere shading  : ";
		TimeToString( MSG, timeAtmosphere ) << " (" << 100.0f * timeAtmosphere / m_timeTotalFrame << "%)" << std::endl;
		MSG << "Splits              : ";
		TimeToString( MSG, timeSplits ) << " (" << 100.0f * timeSplits / m_timeTotalFrame << "%)" << std::endl;
		MSG << "Dicing              : ";
		TimeToString( MSG, timeDicing ) << " (" << 100.0f * timeDicing / m_timeTotalFrame << "%)" << std::endl;
		MSG << "Render MPGs         : ";
		TimeToString( MSG, timeRenderMPGs ) << " (" << 100.0f * timeRenderMPGs / m_timeTotalFrame << "%)" << std::endl;
		MSG << "Occlusion Culling   : ";
		TimeToString( MSG, timeOcclusionCull ) << " (" << 100.0f * timeOcclusionCull / m_timeTotalFrame << "%)" << std::endl;
		MSG << "Diceable check      : ";
		TimeToString( MSG, timeDiceable ) << " (" << 100.0f * timeDiceable / m_timeTotalFrame << "%)" << std::endl;
		MSG << "Textures            : " << m_cTextureMemory << " bytes used." << std::endl;

		MSG << "Textures hits       : " << std::endl;
		for ( TqInt i = 0; i < 5; i++ )
		{
			/* Only if we missed something */
			if ( m_cTextureHits[ 0 ][ i ] )
			{
				switch ( i )
				{
						case 0: MSG << "\t\t\tMipMap   P(";
						break;
						case 1: MSG << "\t\t\tCube Env.P(";
						break;
						case 2: MSG << "\t\t\tLatLong  P(";
						break;
						case 3: MSG << "\t\t\tShadow   P(";
						break;
						case 4: MSG << "\t\t\tTiles    P(";
						break;
				}
				MSG << 100.0f * ( ( float ) m_cTextureHits[ 0 ][ i ] / ( float ) ( m_cTextureHits[ 0 ][ i ] + m_cTextureMisses[ i ] ) ) << "%)" << " of " << m_cTextureMisses[ i ] << " tries" << std::endl;
			}
			if ( m_cTextureHits[ 1 ][ i ] )
			{
				switch ( i )
				{
						case 0: MSG << "\t\t\tMipMap   S(";
						break;
						case 1: MSG << "\t\t\tCube Env.S(";
						break;
						case 2: MSG << "\t\t\tLatLong  S(";
						break;
						case 3: MSG << "\t\t\tShadow   S(";
						break;
						case 4: MSG << "\t\t\tTiles    S(";
						break;
				}
				MSG << 100.0f * ( ( float ) m_cTextureHits[ 1 ][ i ] / ( float ) ( m_cTextureHits[ 1 ][ i ] + m_cTextureMisses[ i ] ) ) << "%)" << std::endl;
			}

		}
		MSG << std::endl;

	}
	//! Most important informations
	if ( level == 2 )
	{
		MSG << "GPrims: \t" << m_cGPrims << std::endl;
		MSG << "Total GPrims:\t" << m_cTotalGPrims << " (" << m_cCulledGPrims << " culled)" << std::endl;

		MSG << "Grids:    \t" << m_cGridsAllocated << " created" << std::endl;

		MSG << "Micropolygons: \t" << m_cMPGsAllocated << " created" << " (" << m_cCulledMPGs << " culled)" << std::endl;

		MSG << "Sampling: \t" << m_cSamples << " samples" << std::endl;

		MSG << "Attributes: \t";
		MSG << ( TqInt ) Attribute_stack.size() << " created" << std::endl;

		MSG << "Transforms: \t";
		MSG << QGetRenderContext() ->TransformStack().size() << " created" << std::endl;

		MSG << "Variables: \t";
		MSG << m_cVariablesAllocated << " created" << std::endl;

		MSG << "Parameters: \t" << m_cParametersAllocated << " created" << std::endl;
		MSG << "MakeTexture check: \t"; TimeToString( MSG, timeMakeTexture ) << " (" << 100.0f * timeMakeTexture / m_timeTotalFrame << "%)" << std::endl;
		MSG << "MakeShadow  check: \t"; TimeToString( MSG, timeMakeShadow ) << " (" << 100.0f * timeMakeShadow / m_timeTotalFrame << "%)" << std::endl;
		MSG << "MakeCubeEnv check: \t"; TimeToString( MSG, timeMakeEnv ) << " (" << 100.0f * timeMakeEnv / m_timeTotalFrame << "%)" << std::endl;
		MSG << "SampleTexture check:\t"; TimeToString( MSG, timeTM ) << " (" << 100.0f * timeTM / m_timeTotalFrame << "%)" << std::endl;
		MSG << "FilterBucket check:\t"; TimeToString( MSG, timeFB ) << " (" << 100.0f * timeFB / m_timeTotalFrame << "%)" << std::endl;

	}
	if ( level == 3 )
	{
		MSG << "GPrims: \t" << m_cGPrims << std::endl;
		MSG << "Total GPrims:\t" << m_cTotalGPrims << " (" << m_cCulledGPrims << " culled)" << std::endl;

		MSG << "Grids:    \t" << m_cGridsAllocated << " created / ";
		MSG << m_cGridsAllocated - m_cGridsDeallocated << " remaining  / ";
		MSG << m_cGridsPeak << " peak  (";
		MSG << m_cCulledGrids << " culled)" << std::endl;

		MSG << "Micropolygons: \t" << m_cMPGsAllocated << " created / ";
		MSG << m_cMPGsAllocated - m_cMPGsDeallocated << " remaining / ";
		MSG << m_cMPGsPeak << " peak (+ ";
		MSG << m_cCulledMPGs << " culled)";
		if ( m_cMissedMPGs > 0 )
			MSG << " (** " << m_cMissedMPGs << " missed **)" << std::endl;
		else
			MSG << std::endl;

		MSG << "Sampling: \t" << m_cSamples << " samples" << std::endl;
		MSG << "          \t" << m_cSampleBoundHits << " bound hits (";
		MSG << ( 100.0f / m_cSamples ) * m_cSampleBoundHits << "% of samples)" << std::endl;
		MSG << "          \t" << m_cSampleHits << " hits (";
		MSG << ( 100.0f / m_cSamples ) * m_cSampleHits << "% of samples)" << std::endl;

		MSG << "Attributes: \t";
		MSG << ( TqInt ) Attribute_stack.size() << " created" << std::endl;

		MSG << "Transforms: \t";
		MSG << QGetRenderContext() ->TransformStack().size() << " created" << std::endl;

		MSG << "Variables: \t";
		MSG << m_cVariablesAllocated << " created / ";
		MSG << m_cVariablesAllocated - m_cVariablesDeallocated << " remaining / ";
		MSG << m_cVariablesPeak << " peak" << std::endl;

		MSG << "Parameters: \t" << m_cParametersAllocated << " created / ";
		MSG << m_cParametersAllocated - m_cParametersDeallocated << " remaining (5 expected) / ";
		MSG << m_cParametersPeak << " peak" << std::endl;
		MSG << "MakeTexture check: \t"; TimeToString( MSG, timeMakeTexture ) << " (" << 100.0f * timeMakeTexture / m_timeTotalFrame << "%)" << std::endl;
		MSG << "MakeShadow  check: \t"; TimeToString( MSG, timeMakeShadow ) << " (" << 100.0f * timeMakeShadow / m_timeTotalFrame << "%)" << std::endl;
		MSG << "MakeCubeEnv check: \t"; TimeToString( MSG, timeMakeEnv ) << " (" << 100.0f * timeMakeEnv / m_timeTotalFrame << "%)" << std::endl;
		MSG << "SampleTexture check:\t"; TimeToString( MSG, timeTM ) << " (" << 100.0f * timeTM / m_timeTotalFrame << "%)" << std::endl;
		MSG << "Combine check:\t"; TimeToString( MSG, timeCombine ) << " (" << 100.0f * timeCombine / m_timeTotalFrame << "%)" << std::endl;
		MSG << "DisplayBucket check:\t"; TimeToString( MSG, timeDB ) << " (" << 100.0f * timeDB / m_timeTotalFrame << "%)" << std::endl;
		MSG << "Parsing check:\t"; TimeToString( MSG, timeParse ) << " (" << 100.0f * timeParse / m_timeTotalFrame << "%)" << std::endl;
		MSG << "Project check:\t"; TimeToString( MSG, timeProject ) << " (" << 100.0f * timeProject / m_timeTotalFrame << "%)" << std::endl;
		MSG << "Init. Buckets: ";
		TimeToString( MSG, timeOthers ) << " (" << 100.0f * timeOthers / m_timeTotalFrame << "%)" << std::endl;
	}


	MSG << std::ends;

	CqString strMSG( MSG.str() );
	CqBasicError( 0, Severity_Normal, strMSG.c_str() );
}



/** Convert a time value into a string.
 
    \param os  Output stream
		\param t   Time value (in seconds).
		\return  os
 */
std::ostream& CqStats::TimeToString( std::ostream& os, TqFloat t ) const
{
	// Is the time negative? Then there's a bug somewhere.
	if ( t < 0.0 )
	{
		os << "<invalid>";
		return os;
	}

	// Round the time if it's more than 5sec
	if ( t > 5.0 ) t = fmod( t, 1 ) < 0.5 ? FLOOR( t ) : CEIL( t );

	TqInt h = static_cast<TqInt>( t / ( 60 * 60 ) );
	TqInt m = static_cast<TqInt>( ( t / 60 ) - ( h * 60 ) );
	TqFloat s = ( t ) - ( h * 60 * 60 ) - ( m * 60 );
	if ( h > 0 ) os << h << "hrs ";
	if ( m > 0 ) os << m << "mins ";
	os << s << "secs";
	return os;
}


//---------------------------------------------------------------------

END_NAMESPACE( Aqsis )

