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

#include	"aqsis.h"

#include  <strstream>
#include  <math.h>
#include  <iomanip>

#include	"renderer.h"
#include	"attributes.h"
#include	"stats.h"
//#include	"messages.h"
#include	"imagebuffer.h"
#include	<iostream>
#include	<iomanip>

START_NAMESPACE( Aqsis )

TqFloat	 CqStats::m_floatVars[ CqStats::_Last_float ];		///< Float variables
TqInt	 CqStats::m_intVars[ CqStats::_Last_int ];			///< Int variables

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
//	m_timeTotal = 0;
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
	m_timeTotalFrame.Reset();
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
	if( !m_timeTotalFrame.isStarted() )
		m_timeTotalFrame.Start();
}

/** Stop the frame timer.
    The difference between the starting time and the current time is
		stored and also added to the total time.
 
    \see StartFrameTimer()
 */
void CqStats::StopFrameTimer()
{
	m_timeTotalFrame.Stop();
	m_timeTotal+=m_timeTotalFrame;
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


	TqFloat timeTotal = m_timeSurface.TimeTotal() + 
						m_timeDisplacement.TimeTotal() + 
						m_timeImager.TimeTotal() + 
						m_timeAtmosphere.TimeTotal() + 
						m_timeSplits.TimeTotal() +
						m_timeDicing.TimeTotal() + 
						m_timeRenderMPGs.TimeTotal() + 
						m_timeOcclusionCull.TimeTotal() + 
						m_timeDiceable.TimeTotal() + 
						m_timeMakeTexture.TimeTotal() +
						m_timeMakeShadow.TimeTotal() + 
						m_timeMakeEnv.TimeTotal() + 
						m_timeTM.TimeTotal() + 
						m_timeFB.TimeTotal() + 
						m_timeDB.TimeTotal() + 
						m_timeParse.TimeTotal() + 
						m_timeProject.TimeTotal() +
						m_timeCombine.TimeTotal() + 
						m_timeOthers.TimeTotal();

	//! level >= 0
	MSG << "Total render time   : ";
	TimeToString( MSG, m_timeTotal.TimeTotal(), -1 ) << std::endl;
	MSG << "Last frame          : ";
	TimeToString( MSG, m_timeTotalFrame.TimeTotal(), -1 ) << std::endl;

	if ( level >= 1 )
	{
		MSG << "Parsing             : "; 
		TimeToString( MSG, m_timeParse.TimeTotal(), m_timeTotalFrame.TimeTotal() ) << std::endl;
		MSG << "Diceable check      : ";
		TimeToString( MSG, m_timeDiceable.TimeTotal(), m_timeTotalFrame.TimeTotal() ) << std::endl;
		MSG << "Splitting           : ";
		TimeToString( MSG, m_timeSplits.TimeTotal(), m_timeTotalFrame.TimeTotal() ) << std::endl;
		MSG << "Dicing              : ";
		TimeToString( MSG, m_timeDicing.TimeTotal(), m_timeTotalFrame.TimeTotal() ) << std::endl;
		MSG << "Render MPGs         : ";
		TimeToString( MSG, m_timeRenderMPGs.TimeTotal(), m_timeTotalFrame.TimeTotal() ) << std::endl;
		MSG << "Occlusion Culling   : ";
		TimeToString( MSG, m_timeOcclusionCull.TimeTotal(), m_timeTotalFrame.TimeTotal() ) << std::endl;
		MSG << "Imager shading      : ";
		TimeToString( MSG, m_timeImager.TimeTotal(), m_timeTotalFrame.TimeTotal() ) << std::endl;
		MSG << "Surface shading     : ";
		TimeToString( MSG, m_timeSurface.TimeTotal(), m_timeTotalFrame.TimeTotal() ) << std::endl;
		MSG << "Displacement shading: ";
		TimeToString( MSG, m_timeDisplacement.TimeTotal(), m_timeTotalFrame.TimeTotal() ) << std::endl;
		MSG << "Atmosphere shading  : ";
		TimeToString( MSG, m_timeAtmosphere.TimeTotal(), m_timeTotalFrame.TimeTotal() ) << std::endl;
		MSG << "SampleTexture       : "; 
		TimeToString( MSG, m_timeTM.TimeTotal(), m_timeTotalFrame.TimeTotal() ) << std::endl;
		MSG << "Combine             : "; 
		TimeToString( MSG, m_timeCombine.TimeTotal(), m_timeTotalFrame.TimeTotal() ) << std::endl;
		MSG << "Project             : "; 
		TimeToString( MSG, m_timeProject.TimeTotal(), m_timeTotalFrame.TimeTotal() ) << std::endl;
		MSG << "FilterBucket        : "; 
		TimeToString( MSG, m_timeFB.TimeTotal(), m_timeTotalFrame.TimeTotal() ) << std::endl;
		MSG << "DisplayBucket       : "; 
		TimeToString( MSG, m_timeDB.TimeTotal(), m_timeTotalFrame.TimeTotal() ) << std::endl;
		MSG << "MakeTexture         : "; 
		TimeToString( MSG, m_timeMakeTexture.TimeTotal(), m_timeTotalFrame.TimeTotal() ) << std::endl;
		MSG << "MakeShadow          : "; 
		TimeToString( MSG, m_timeMakeShadow.TimeTotal(), m_timeTotalFrame.TimeTotal() ) << std::endl;
		MSG << "MakeCubeEnv         : "; 
		TimeToString( MSG, m_timeMakeEnv.TimeTotal(), m_timeTotalFrame.TimeTotal() ) << std::endl;
		MSG << "Others              : ";
		TimeToString( MSG, m_timeOthers.TimeTotal(), m_timeTotalFrame.TimeTotal() ) << std::endl;

		MSG << "Total time measured : ";
		TimeToString( MSG, timeTotal, m_timeTotalFrame.TimeTotal() ) << std::endl;

		MSG << std::endl;

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
	if ( level == 2 || level == 3 )
	{
		MSG << "GPrims: \t" << m_cGPrims << std::endl;
		MSG << "Total GPrims:\t" << m_cTotalGPrims << " (" << m_cCulledGPrims << " culled)" << std::endl;

		/*
			-------------------------------------------------------------------
			Grid stats
		*/

		TqInt _grd_init =
					STATS_INT_GETI( GRD_size_4 ) +
					STATS_INT_GETI( GRD_size_8 ) +
					STATS_INT_GETI( GRD_size_16 ) +
					STATS_INT_GETI( GRD_size_32 ) +
					STATS_INT_GETI( GRD_size_64 ) +
					STATS_INT_GETI( GRD_size_128 ) +
					STATS_INT_GETI( GRD_size_256 ) +
					STATS_INT_GETI( GRD_size_g256 );

		TqInt _grd_shade =
					STATS_INT_GETI( GRD_shd_size_4 ) +
					STATS_INT_GETI( GRD_shd_size_8 ) +
					STATS_INT_GETI( GRD_shd_size_16 ) +
					STATS_INT_GETI( GRD_shd_size_32 ) +
					STATS_INT_GETI( GRD_shd_size_64 ) +
					STATS_INT_GETI( GRD_shd_size_128 ) +
					STATS_INT_GETI( GRD_shd_size_256 ) +
					STATS_INT_GETI( GRD_shd_size_g256 );
		
		TqFloat	_grd_init_quote	=	100.0f *  _grd_init / STATS_INT_GETI( GRD_created );
		TqFloat	_grd_shade_quote=	100.0f *  _grd_shade / STATS_INT_GETI( GRD_created );

		TqFloat	_grd_4		=	100.0f * STATS_INT_GETI( GRD_size_4 ) / _grd_init;
		TqFloat	_grd_8		=	100.0f * STATS_INT_GETI( GRD_size_8 ) / _grd_init;
		TqFloat	_grd_16		=	100.0f * STATS_INT_GETI( GRD_size_16 ) / _grd_init;
		TqFloat	_grd_32		=	100.0f * STATS_INT_GETI( GRD_size_32 ) / _grd_init;
		TqFloat	_grd_64		=	100.0f * STATS_INT_GETI( GRD_size_64 ) / _grd_init;
		TqFloat	_grd_128	=	100.0f * STATS_INT_GETI( GRD_size_128 ) / _grd_init;
		TqFloat	_grd_256	=	100.0f * STATS_INT_GETI( GRD_size_256 ) / _grd_init;
		TqFloat	_grd_g256	=	100.0f * STATS_INT_GETI( GRD_size_g256 ) / _grd_init;

		TqFloat	_grd_shd_4		=	100.0f * STATS_INT_GETI( GRD_shd_size_4 ) / _grd_shade;
		TqFloat	_grd_shd_8		=	100.0f * STATS_INT_GETI( GRD_shd_size_8 ) / _grd_shade;
		TqFloat	_grd_shd_16		=	100.0f * STATS_INT_GETI( GRD_shd_size_16 ) / _grd_shade;
		TqFloat	_grd_shd_32		=	100.0f * STATS_INT_GETI( GRD_shd_size_32 ) / _grd_shade;
		TqFloat	_grd_shd_64		=	100.0f * STATS_INT_GETI( GRD_shd_size_64 ) / _grd_shade;
		TqFloat	_grd_shd_128	=	100.0f * STATS_INT_GETI( GRD_shd_size_128 ) / _grd_shade;
		TqFloat	_grd_shd_256	=	100.0f * STATS_INT_GETI( GRD_shd_size_256 ) / _grd_shade;
		TqFloat	_grd_shd_g256	=	100.0f * STATS_INT_GETI( GRD_shd_size_g256 ) / _grd_shade;
		
		

		MSG << "Grids:\n\t"
			<< STATS_INT_GETI( GRD_created ) << " created, " << _grd_init << " initialized (" << _grd_init_quote << "%),\n\t" << _grd_shade << " shaded (" << _grd_shade_quote << "%), " << STATS_INT_GETI( GRD_culled ) << " culled\n\n"
			<< "\tGrid count/size (diced grids):\n"
			<< "\t+------+------+------+------+------+------+------+------+\n"
			<< "\t|<=  4 |<=  8 |<= 16 |<= 32 |<= 64 |<=128 |<=256 | >256 |\n"
			<< "\t+------+------+------+------+------+------+------+------+\n\t|"
			<< std::setw(6) << std::setiosflags( std::ios::right ) << STATS_INT_GETI( GRD_size_4 ) << "|" 
			<< std::setw(6) << std::setiosflags( std::ios::right ) << STATS_INT_GETI( GRD_size_8 ) << "|"
			<< std::setw(6) << std::setiosflags( std::ios::right ) << STATS_INT_GETI( GRD_size_16 ) << "|"
			<< std::setw(6) << std::setiosflags( std::ios::right ) << STATS_INT_GETI( GRD_size_32 ) << "|"
			<< std::setw(6) << std::setiosflags( std::ios::right ) << STATS_INT_GETI( GRD_size_64 ) << "|"
			<< std::setw(6) << std::setiosflags( std::ios::right ) << STATS_INT_GETI( GRD_size_128 ) << "|"
			<< std::setw(6) << std::setiosflags( std::ios::right ) << STATS_INT_GETI( GRD_size_256 ) << "|"
			<< std::setw(6) << std::setiosflags( std::ios::right ) << STATS_INT_GETI( GRD_size_g256 ) << "|\n"
			<< "\t|" << std::setw(5) << std::setiosflags( std::ios::right ) << _grd_4 << "%|" 
			<< std::setw(5) << std::setiosflags( std::ios::right ) << _grd_8 << "%|"
			<< std::setw(5) << std::setiosflags( std::ios::right ) << _grd_16 << "%|"
			<< std::setw(5) << std::setiosflags( std::ios::right ) << _grd_32 << "%|"
			<< std::setw(5) << std::setiosflags( std::ios::right ) << _grd_64 << "%|"
			<< std::setw(5) << std::setiosflags( std::ios::right ) << _grd_128 << "%|"
			<< std::setw(5) << std::setiosflags( std::ios::right ) << _grd_256 << "%|"
			<< std::setw(5) << std::setiosflags( std::ios::right ) << _grd_g256 << "%|\n"
			<< "\t+------+------+------+------+------+------+------+------+\n\n"
			<< "\tGrid count/size (shaded grids):\n"
			<< "\t+------+------+------+------+------+------+------+------+\n"
			<< "\t|<=  4 |<=  8 |<= 16 |<= 32 |<= 64 |<=128 |<=256 | >256 |\n"
			<< "\t+------+------+------+------+------+------+------+------+\n\t|"
			<< std::setw(6) << std::setiosflags( std::ios::right ) << STATS_INT_GETI( GRD_shd_size_4 ) << "|" 
			<< std::setw(6) << std::setiosflags( std::ios::right ) << STATS_INT_GETI( GRD_shd_size_8 ) << "|"
			<< std::setw(6) << std::setiosflags( std::ios::right ) << STATS_INT_GETI( GRD_shd_size_16 ) << "|"
			<< std::setw(6) << std::setiosflags( std::ios::right ) << STATS_INT_GETI( GRD_shd_size_32 ) << "|"
			<< std::setw(6) << std::setiosflags( std::ios::right ) << STATS_INT_GETI( GRD_shd_size_64 ) << "|"
			<< std::setw(6) << std::setiosflags( std::ios::right ) << STATS_INT_GETI( GRD_shd_size_128 ) << "|"
			<< std::setw(6) << std::setiosflags( std::ios::right ) << STATS_INT_GETI( GRD_shd_size_256 ) << "|"
			<< std::setw(6) << std::setiosflags( std::ios::right ) << STATS_INT_GETI( GRD_shd_size_g256 ) << "|\n"
			<< "\t|" << std::setw(5) << std::setiosflags( std::ios::right ) << _grd_shd_4 << "%|" 
			<< std::setw(5) << std::setiosflags( std::ios::right ) << _grd_shd_8 << "%|"
			<< std::setw(5) << std::setiosflags( std::ios::right ) << _grd_shd_16 << "%|"
			<< std::setw(5) << std::setiosflags( std::ios::right ) << _grd_shd_32 << "%|"
			<< std::setw(5) << std::setiosflags( std::ios::right ) << _grd_shd_64 << "%|"
			<< std::setw(5) << std::setiosflags( std::ios::right ) << _grd_shd_128 << "%|"
			<< std::setw(5) << std::setiosflags( std::ios::right ) << _grd_shd_256 << "%|"
			<< std::setw(5) << std::setiosflags( std::ios::right ) << _grd_shd_g256 << "%|\n"
			<< "\t+------+------+------+------+------+------+------+------+\n\n"
			<< std::endl;

		/*
			Grid stats - End
			-------------------------------------------------------------------
		*/

		/* MPGS */


		/*
			-------------------------------------------------------------------
			MPG stats
		*/

		TqInt _mpg_pushes_all = STATS_INT_GETI( MPG_pushed_forward ) + STATS_INT_GETI( MPG_pushed_down ) +
			STATS_INT_GETI( MPG_pushed_far_down );

		TqFloat _mpg_p_f =  100.0f * STATS_INT_GETI( MPG_pushed_forward ) / _mpg_pushes_all;
		TqFloat _mpg_p_d = 100.0f * STATS_INT_GETI( MPG_pushed_down ) / _mpg_pushes_all;
		TqFloat _mpg_p_fd = 100.0f * STATS_INT_GETI( MPG_pushed_far_down ) / _mpg_pushes_all;
		TqFloat _mpg_p_a = 100.0f * _mpg_pushes_all /STATS_INT_GETI( MPG_allocated );

		MSG << "Micropolygons:\n\t"
			<< STATS_INT_GETI( MPG_allocated ) << " created (" << STATS_INT_GETI( MPG_culled ) << " culled)\n"
			<< "\tPeak: " << STATS_INT_GETI( MPG_peak ) << "\n\t"
			<< std::endl;
		
		/*
			MPG Pushed
		*/
		MSG << "\tPushes:\t" << _mpg_pushes_all << " MPGs pushed ("		<< _mpg_p_a			<< "%)\n\t\t" 
							 << STATS_INT_GETI( MPG_pushed_forward )	<< " forward ("		<< _mpg_p_f	 << "%), "
							 << STATS_INT_GETI( MPG_pushed_down )		<< " down ("		<< _mpg_p_d	 << "%),\n\t\t"
							 << STATS_INT_GETI( MPG_pushed_far_down )	<< " far down ("	<< _mpg_p_fd << "%)\n"
							 << std::endl;

		/*
			MPG stats - End
			-------------------------------------------------------------------
		*/



		/*
			-------------------------------------------------------------------
			Sampling
		*/

		TqFloat _spl_b_h = 100.0f * STATS_INT_GETI( SPL_bound_hits ) / STATS_INT_GETI( SPL_count );
		TqFloat _spl_h = 100.0f * STATS_INT_GETI( SPL_hits ) / STATS_INT_GETI( SPL_count );
		TqFloat _spl_m = 100.0f - _spl_b_h - _spl_h;

		TqInt _spl_px = QGetRenderContext() ->optCurrent().GetIntegerOption( "System", "PixelSamples" ) [ 0 ];
		TqInt _spl_py = QGetRenderContext() ->optCurrent().GetIntegerOption( "System", "PixelSamples" ) [ 1 ];

		MSG << "Sampling:\n"
							<< "\tSamples per Pixel: " << _spl_px * _spl_py << " (" << _spl_px << " " << _spl_py << ")\n\t"
							<< STATS_INT_GETI( SPL_count ) << " samples" << std::endl;
		MSG					<< "\tHits: " << STATS_INT_GETI( SPL_hits ) << " (" << _spl_h << "%), "
							<< "bound hits: " << STATS_INT_GETI( SPL_bound_hits ) << " (" << _spl_b_h << "%),\n\tmisses: "
							<< STATS_INT_GETI( SPL_count ) - STATS_INT_GETI( SPL_hits ) - STATS_INT_GETI( SPL_bound_hits ) << " (" << _spl_m << "%)\n"
							<< std::endl;
		
		/*
			Sampling - End
			-------------------------------------------------------------------
		*/

		MSG << "Attributes: \t";
		MSG << ( TqInt ) Attribute_stack.size() << " created" << std::endl;

		MSG << "Transforms: \t";
		MSG << QGetRenderContext() ->TransformStack().size() << " created" << std::endl;

		MSG << "Variables: \t";
		MSG << m_cVariablesAllocated << " created" << std::endl;

		MSG << "Parameters: \t" << m_cParametersAllocated << " created" << std::endl;

	}/*
	if ( level == 4 )
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
		MSG << "               \t" << m_cMPGsPushedForward << " pushed forward, " << m_cMPGsPushedDown << " pushed down, " << m_cMPGsPushedFarDown << " pushed far down" << std::endl;

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
	}*/


	MSG << std::ends;

	CqString strMSG( MSG.str() );
	MSG.freeze(false);
	//CqBasicError( 0, Severity_Normal, strMSG.c_str() );
	std::cout << strMSG.c_str();
}



/** Convert a time value into a string.
 
    \param os  Output stream
		\param t   Time value (in seconds).
		\return  os
 */
std::ostream& CqStats::TimeToString( std::ostream& os, TqFloat ticks, TqFloat tot ) const
{
	TqFloat t = static_cast<TqFloat>(ticks) / CLOCKS_PER_SEC;

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
	if ( h > 0 ) 
		os << std::setiosflags(std::ios::fixed) << std::setprecision(1) << std::setw(6) << h << "hrs ";
	if ( m > 0 ) 
		os << std::setiosflags(std::ios::fixed) << std::setprecision(1) << std::setw(6) << m << "mins ";
	os << std::setiosflags(std::ios::fixed) << std::setprecision(1) << std::setw(6) << s << "secs";
	if( tot >= 0 )
	      os << " (" << std::setprecision(2) << std::setw(6) << 100.0f * ticks / tot << "%)";
	return os;
}

void CqStats::PrintInfo() const
{
	TqInt psX, psY; //< Pixel Samples
	TqInt resX, resY;	//< Image resolution
	TqInt fX, fY;	//< Filter width
	TqFloat gain, gamma; //< Exposure, gain
	TqFloat pratio; //< PixelAspectRatio
	TqInt bX = 16, bY = 16; //< Bucket Size
	TqInt gs; //< Grid Size

	psX = QGetRenderContext() ->optCurrent().GetIntegerOption( "System", "PixelSamples" ) [ 0 ];
	psY = QGetRenderContext() ->optCurrent().GetIntegerOption( "System", "PixelSamples" ) [ 1 ];

	resX = QGetRenderContext() ->optCurrent().GetIntegerOption( "System", "Resolution" ) [ 0 ];
	resY = QGetRenderContext() ->optCurrent().GetIntegerOption( "System", "Resolution" ) [ 1 ];
	
	fX = (TqInt) QGetRenderContext() ->optCurrent().GetFloatOption( "System", "FilterWidth" ) [ 0 ];
	fY = (TqInt) QGetRenderContext() ->optCurrent().GetFloatOption( "System", "FilterWidth" ) [ 1 ];

	gain = QGetRenderContext() ->optCurrent().GetFloatOption( "System", "Exposure" ) [ 0 ];
	gamma = QGetRenderContext() ->optCurrent().GetFloatOption( "System", "Exposure" ) [ 1 ];

	pratio = QGetRenderContext() ->optCurrent().GetFloatOption( "System", "PixelAspectRatio" ) [ 0 ];

	const TqInt* poptBucketSize = QGetRenderContext() ->optCurrent().GetIntegerOption( "limits", "bucketsize" );
	if ( poptBucketSize != 0 )
	{
		bX = poptBucketSize[ 0 ];
		bY = poptBucketSize[ 1 ];
	}

	const TqInt* poptGridSize = QGetRenderContext() ->optCurrent().GetIntegerOption( "limits", "gridsize" );
	
	if ( poptGridSize )
		gs = poptGridSize[ 0 ];
	else
		gs = 256;

	QGetRenderContext() ->Logger()->info( "Image settings:" );
	QGetRenderContext() ->Logger()->info( "	Resolution: %d %d", resX, resY );
	QGetRenderContext() ->Logger()->info( "	PixelAspectRatio: %f", pratio );
	QGetRenderContext() ->Logger()->info( "	Exposure:" );
	QGetRenderContext() ->Logger()->info( "		Gain: %f", gain );
	QGetRenderContext() ->Logger()->info( "		Gamma: %f", gamma );
	QGetRenderContext() ->Logger()->info( "Shading:" );
	QGetRenderContext() ->Logger()->info( "	Bucket size: [ %d %d ]", bX, bY );
	QGetRenderContext() ->Logger()->info( "	Gridsize: %d", gs );
	QGetRenderContext() ->Logger()->info( "Anti-aliasing settings: " );
	QGetRenderContext() ->Logger()->info( "	PixelSamples: %d %d", psX, psY );
	QGetRenderContext() ->Logger()->info( "	FilterWidth: %d %d", fX, fY );

}

//---------------------------------------------------------------------

END_NAMESPACE( Aqsis )

