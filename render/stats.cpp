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

// Global accessor functions, defined like this so that other projects using libshadervm can
// simply provide empty implementations and not have to link to libaqsis.
void gStats_IncI( TqInt index )
{
    CqStats::IncI( index );
}

void gStats_DecI( TqInt index )
{
    CqStats::DecI( index );
}

TqInt gStats_getI( TqInt index )
{
    return( CqStats::getI( index ) );
}

void gStats_setI( TqInt index, TqInt value )
{
    CqStats::setI( index, value );
}

TqFloat gStats_getF( TqInt index )
{
    return( CqStats::getF( index ) );
}

void gStats_setF( TqInt index, TqFloat value )
{
    CqStats::setF( index, value );
}


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



    }
    //! Most important informations
    if ( level == 2 || level == 3 )
    {

        /*
        	-------------------------------------------------------------------
        	GPrim stats
        */

        TqFloat _gpr_c_q = 100.0f * STATS_INT_GETI( GPR_culled ) / STATS_INT_GETI( GPR_created_total );
        TqFloat _gpr_u_q = 100.0f * STATS_INT_GETI( GPR_created_total ) / STATS_INT_GETI( GPR_allocated );

        MSG << "Input geometry:\n\t" << STATS_INT_GETI( GPR_created ) << " primitives created\n\n"
        << "\t"	<<	STATS_INT_GETI( GPR_subdiv ) << " subdivision primitives\n\t"
        <<			STATS_INT_GETI( GPR_nurbs )	 << " NURBS primitives\n\t"
        <<			STATS_INT_GETI( GPR_poly )	 << " polygons\n\t"
        <<			STATS_INT_GETI( GPR_crv )	 << " curves\n\t"
        <<			STATS_INT_GETI( GPR_points ) << " points\n\t"
        <<			STATS_INT_GETI( GPR_patch )	 << " patches\n\t"
        <<			STATS_INT_GETI( GPR_quad )	 << " quadrics\n\t"
        << std::endl;

        MSG << "GPrims:\n\t"
        << STATS_INT_GETI( GPR_allocated ) <<  " allocated\n\t"
        << STATS_INT_GETI( GPR_created_total ) <<  " used (" << _gpr_u_q << "%), " << STATS_INT_GETI( GPR_peak ) << " peak,\n\t"
        << STATS_INT_GETI( GPR_culled ) << " culled (" << _gpr_c_q << "%)\n" << std::endl;

        /*
        	GPrim stats - End
        	-------------------------------------------------------------------
        */



        /*
        	-------------------------------------------------------------------
        	Geometry stats
        */

        // Curves
        TqFloat _geo_crv_s_q = 100.0f * STATS_INT_GETI( GEO_crv_splits ) / STATS_INT_GETI( GPR_crv );
        TqFloat _geo_crv_s_c_q = 100.0f * STATS_INT_GETI( GEO_crv_crv ) / STATS_INT_GETI( GEO_crv_splits );
        TqFloat _geo_crv_s_p_q = 100.0f * STATS_INT_GETI( GEO_crv_patch ) / STATS_INT_GETI( GEO_crv_splits );

        // Procedural
        TqFloat _geo_prc_s_q = 100.0f * STATS_INT_GETI( GEO_prc_split ) / STATS_INT_GETI( GEO_prc_created );


        MSG << "Geometry:\n\t"
        // Curves
        << "Curves:\n"
        <<					"\t\t" << STATS_INT_GETI( GPR_crv ) << " created\n\t"
        <<					"\t" << STATS_INT_GETI( GEO_crv_splits ) << " split (" << _geo_crv_s_q << "%)\n\t\t\t"
        <<							STATS_INT_GETI( GEO_crv_crv ) << " (" << _geo_crv_s_c_q << "%) into " << STATS_INT_GETI( GEO_crv_crv_created ) << " subcurves\n\t\t\t"
        <<							STATS_INT_GETI( GEO_crv_patch ) << " (" << _geo_crv_s_p_q << "%) into " << STATS_INT_GETI( GEO_crv_patch_created ) << " patches\n\t"
        << "Procedurals:\n"
        <<					"\t\t" << STATS_INT_GETI( GEO_prc_created ) << " created\n\t"
        <<					"\t" << STATS_INT_GETI( GEO_prc_split ) << " split (" << _geo_prc_s_q << "%)\n\t\t"
        <<							STATS_INT_GETI( GEO_prc_created_dl ) << " dynamic load,\n\t\t"
        <<							STATS_INT_GETI( GEO_prc_created_dra ) << " dynamic read archive,\n\t\t"
        <<							STATS_INT_GETI( GEO_prc_created_prp ) << " run program\n\t\t"
        << std::endl;

        /*
        	GPrim stats - End
        	-------------------------------------------------------------------
        */



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
        TqFloat	_grd_cull_quote =	100.0f *  STATS_INT_GETI( GRD_culled ) / STATS_INT_GETI( GRD_created );

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
        << STATS_INT_GETI( GRD_created ) << " created, " << STATS_INT_GETI( GRD_peak ) << " peak,\n\t"
        << _grd_init << " initialized (" << _grd_init_quote << "%),\n\t" << _grd_shade << " shaded (" << _grd_shade_quote << "%), " << STATS_INT_GETI( GRD_culled ) << " culled (" << _grd_cull_quote << "%)\n\n"
        << "\tGrid count/size (diced grids):\n"
        << "\t+------+------+------+------+------+------+------+------+\n"
        << "\t|<=  4 |<=  8 |<= 16 |<= 32 |<= 64 |<=128 |<=256 | >256 |\n"
        << "\t+------+------+------+------+------+------+------+------+\n\t|"
        << std::setw(6) << std::setprecision( 1 ) << std::setiosflags( std::ios::right ) << STATS_INT_GETI( GRD_size_4 ) << "|"
        << std::setw(6) << std::setprecision( 1 )<< std::setiosflags( std::ios::right ) << STATS_INT_GETI( GRD_size_8 ) << "|"
        << std::setw(6) << std::setprecision( 1 )<< std::setiosflags( std::ios::right ) << STATS_INT_GETI( GRD_size_16 ) << "|"
        << std::setw(6) << std::setprecision( 1 )<< std::setiosflags( std::ios::right ) << STATS_INT_GETI( GRD_size_32 ) << "|"
        << std::setw(6) << std::setprecision( 1 )<< std::setiosflags( std::ios::right ) << STATS_INT_GETI( GRD_size_64 ) << "|"
        << std::setw(6) << std::setprecision( 1 )<< std::setiosflags( std::ios::right ) << STATS_INT_GETI( GRD_size_128 ) << "|"
        << std::setw(6) << std::setprecision( 1 )<< std::setiosflags( std::ios::right ) << STATS_INT_GETI( GRD_size_256 ) << "|"
        << std::setw(6) << std::setprecision( 1 )<< std::setiosflags( std::ios::right ) << STATS_INT_GETI( GRD_size_g256 ) << "|\n"
        << "\t|" << std::setw(5) << std::setprecision( 1 )<< std::setiosflags( std::ios::right ) << _grd_4 << "%|"
        << std::setw(5) << std::setprecision( 1 )<< std::setiosflags( std::ios::right ) << _grd_8 << "%|"
        << std::setw(5) << std::setprecision( 1 )<< std::setiosflags( std::ios::right ) << _grd_16 << "%|"
        << std::setw(5) << std::setprecision( 1 )<< std::setiosflags( std::ios::right ) << _grd_32 << "%|"
        << std::setw(5) << std::setprecision( 1 )<< std::setiosflags( std::ios::right ) << _grd_64 << "%|"
        << std::setw(5) << std::setprecision( 1 )<< std::setiosflags( std::ios::right ) << _grd_128 << "%|"
        << std::setw(5) << std::setprecision( 1 )<< std::setiosflags( std::ios::right ) << _grd_256 << "%|"
        << std::setw(5) << std::setprecision( 1 )<< std::setiosflags( std::ios::right ) << _grd_g256 << "%|\n"
        << "\t+------+------+------+------+------+------+------+------+\n\n"
        << "\tGrid count/size (shaded grids):\n"
        << "\t+------+------+------+------+------+------+------+------+\n"
        << "\t|<=  4 |<=  8 |<= 16 |<= 32 |<= 64 |<=128 |<=256 | >256 |\n"
        << "\t+------+------+------+------+------+------+------+------+\n\t|"
        << std::setw(6) << std::setprecision( 1 )<< std::setiosflags( std::ios::right ) << STATS_INT_GETI( GRD_shd_size_4 ) << "|"
        << std::setw(6) << std::setprecision( 1 )<< std::setiosflags( std::ios::right ) << STATS_INT_GETI( GRD_shd_size_8 ) << "|"
        << std::setw(6) << std::setprecision( 1 )<< std::setiosflags( std::ios::right ) << STATS_INT_GETI( GRD_shd_size_16 ) << "|"
        << std::setw(6) << std::setprecision( 1 )<< std::setiosflags( std::ios::right ) << STATS_INT_GETI( GRD_shd_size_32 ) << "|"
        << std::setw(6) << std::setprecision( 1 )<< std::setiosflags( std::ios::right ) << STATS_INT_GETI( GRD_shd_size_64 ) << "|"
        << std::setw(6) << std::setprecision( 1 )<< std::setiosflags( std::ios::right ) << STATS_INT_GETI( GRD_shd_size_128 ) << "|"
        << std::setw(6) << std::setprecision( 1 )<< std::setiosflags( std::ios::right ) << STATS_INT_GETI( GRD_shd_size_256 ) << "|"
        << std::setw(6) << std::setprecision( 1 )<< std::setiosflags( std::ios::right ) << STATS_INT_GETI( GRD_shd_size_g256 ) << "|\n"
        << "\t|" << std::setw(5) << std::setprecision( 1 )<< std::setiosflags( std::ios::right ) << _grd_shd_4 << "%|"
        << std::setw(5) << std::setprecision( 1 )<< std::setiosflags( std::ios::right ) << _grd_shd_8 << "%|"
        << std::setw(5) << std::setprecision( 1 )<< std::setiosflags( std::ios::right ) << _grd_shd_16 << "%|"
        << std::setw(5) << std::setprecision( 1 )<< std::setiosflags( std::ios::right ) << _grd_shd_32 << "%|"
        << std::setw(5) << std::setprecision( 1 )<< std::setiosflags( std::ios::right ) << _grd_shd_64 << "%|"
        << std::setw(5) << std::setprecision( 1 )<< std::setiosflags( std::ios::right ) << _grd_shd_128 << "%|"
        << std::setw(5) << std::setprecision( 1 )<< std::setiosflags( std::ios::right ) << _grd_shd_256 << "%|"
        << std::setw(5) << std::setprecision( 1 )<< std::setiosflags( std::ios::right ) << _grd_shd_g256 << "%|\n"
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
        TqFloat _mpg_p_a = 100.0f * _mpg_pushes_all / STATS_INT_GETI( MPG_allocated );
        TqFloat _mpg_m_q = 100.0f * STATS_INT_GETI( MPG_missed ) /STATS_INT_GETI( MPG_allocated );

        // Sample hit quote
        TqInt _mpg_hits =	STATS_INT_GETI ( MPG_sample_coverage0_125 ) +
                          STATS_INT_GETI ( MPG_sample_coverage125_25 ) +
                          STATS_INT_GETI ( MPG_sample_coverage25_375 ) +
                          STATS_INT_GETI ( MPG_sample_coverage375_50 ) +
                          STATS_INT_GETI ( MPG_sample_coverage50_625 ) +
                          STATS_INT_GETI ( MPG_sample_coverage625_75 ) +
                          STATS_INT_GETI ( MPG_sample_coverage75_875 ) +
                          STATS_INT_GETI ( MPG_sample_coverage875_100 );

        TqFloat	_mpg_1		=	100.0f * STATS_INT_GETI( MPG_sample_coverage0_125 ) / _mpg_hits;
        TqFloat	_mpg_2		=	100.0f * STATS_INT_GETI( MPG_sample_coverage125_25 ) / _mpg_hits;
        TqFloat	_mpg_3		=	100.0f * STATS_INT_GETI( MPG_sample_coverage25_375 ) / _mpg_hits;
        TqFloat	_mpg_4		=	100.0f * STATS_INT_GETI( MPG_sample_coverage375_50 ) / _mpg_hits;
        TqFloat	_mpg_5		=	100.0f * STATS_INT_GETI( MPG_sample_coverage50_625 ) / _mpg_hits;
        TqFloat	_mpg_6		=	100.0f * STATS_INT_GETI( MPG_sample_coverage625_75 ) / _mpg_hits;
        TqFloat	_mpg_7		=	100.0f * STATS_INT_GETI( MPG_sample_coverage75_875 ) / _mpg_hits;
        TqFloat	_mpg_8		=	100.0f * STATS_INT_GETI( MPG_sample_coverage875_100 ) / _mpg_hits;

        MSG << "Micropolygons:\n\t"
        << STATS_INT_GETI( MPG_allocated ) << " created (" << STATS_INT_GETI( MPG_culled ) << " culled)\n"
        << "\t" <<STATS_INT_GETI( MPG_peak ) << " peak, " << STATS_INT_GETI( MPG_trimmed ) << " trimmed, " << STATS_INT_GETI( MPG_missed ) << " missed (" << _mpg_m_q << "%)\n\t"
        << "\n\tMPG Area:\t" <<STATS_INT_GETF( MPG_average_area ) / STATS_INT_GETI( MPG_allocated ) << " average \n\t\t\t"
        << STATS_INT_GETF( MPG_min_area ) << " min\n\t\t\t"
        << STATS_INT_GETF( MPG_max_area ) << " max\n\t"
        << "\n\t% of sample hits:\n"
        << "\t+------+------+------+------+------+------+------+------+\n"
        << "\t|<=12,5|<=  25|<=37,5|<=  50|<=62,5|<= 75 |<=87,5|<= 100|\n"
        << "\t+------+------+------+------+------+------+------+------+\n\t|"
        << std::setw(5) << std::setprecision( 1 )<< std::setiosflags( std::ios::right ) << _mpg_1 << "%|"
        << std::setw(5) << std::setprecision( 1 )<< std::setiosflags( std::ios::right ) << _mpg_2 << "%|"
        << std::setw(5) << std::setprecision( 1 )<< std::setiosflags( std::ios::right ) << _mpg_3 << "%|"
        << std::setw(5) << std::setprecision( 1 )<< std::setiosflags( std::ios::right ) << _mpg_4 << "%|"
        << std::setw(5) << std::setprecision( 1 )<< std::setiosflags( std::ios::right ) << _mpg_5 << "%|"
        << std::setw(5) << std::setprecision( 1 )<< std::setiosflags( std::ios::right ) << _mpg_6 << "%|"
        << std::setw(5) << std::setprecision( 1 )<< std::setiosflags( std::ios::right ) << _mpg_7 << "%|"
        << std::setw(5) << std::setprecision( 1 )<< std::setiosflags( std::ios::right ) << _mpg_8 << "%|\n"
        << "\t+------+------+------+------+------+------+------+------+\n\n"
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

        /*
        	Shading stats
        	-------------------------------------------------------------------
        */
        TqInt _shd_var_all = STATS_INT_GETI( SHD_var_array ) + STATS_INT_GETI( SHD_var_uniform ) + STATS_INT_GETI( SHD_var_varying );

        TqFloat _shd_var_a = 100.0f * STATS_INT_GETI( SHD_var_array ) / _shd_var_all;
        TqFloat _shd_var_u = 100.0f * STATS_INT_GETI( SHD_var_uniform ) / _shd_var_all;
        TqFloat _shd_var_v = 100.0f * STATS_INT_GETI( SHD_var_varying ) / _shd_var_all;

        TqFloat _shd_var_c_q = 100.0f * STATS_INT_GETI( SHD_var_created_total ) / _shd_var_all;


        MSG << 	"Shading:\n\t"
        <<					"Stack:\n\t\t"
        <<					STATS_INT_GETI( SHD_stk_peak ) << " max stack depth\n\t\t"
        <<					STATS_INT_GETI( SHD_stk_push ) << " push, " << STATS_INT_GETI( SHD_stk_pushv ) << " pushv\n\t\t"
        <<					STATS_INT_GETI( SHD_stk_pop ) << " pop, " << STATS_INT_GETI( SHD_stk_release ) << " release\n\t\t"
        <<					STATS_INT_GETI( SHD_stk_dup ) << " dup, " << STATS_INT_GETI( SHD_stk_drop ) << " drop\n\n\t"
        <<					"Variables:\n\t\t"
        <<							_shd_var_all << " allocated, " << STATS_INT_GETI( SHD_var_created_total )  << " created (" << _shd_var_c_q << "%), " << STATS_INT_GETI( SHD_var_peak ) << " peak\n\n\t\t"
        <<					"Arrays:\n\t\t"
        <<					"\t" << STATS_INT_GETI( SHD_var_array ) << " created (" << _shd_var_a << "%), " << STATS_INT_GETI( SHD_var_array_peak ) << " peak\n\t\t"
        <<					"\t" << STATS_INT_GETI( SHD_var_array_init ) << " initialized\n\n\t\t"
        <<					"Uniform:\n\t\t"
        <<					"\t" << STATS_INT_GETI( SHD_var_uniform ) << " created (" << _shd_var_u << "%), " << STATS_INT_GETI( SHD_var_uniform_peak ) << " peak\n\t\t"
        <<					"\t" << STATS_INT_GETI( SHD_var_uniform_init ) << " inits\n\n\t\t"
        <<					"\t\t" << STATS_INT_GETI( SHD_var_uniform_float ) << " float\n\t\t"
        <<					"\t\t" << STATS_INT_GETI( SHD_var_uniform_string ) << " string\n\t\t"
        <<					"\t\t" << STATS_INT_GETI( SHD_var_uniform_point ) << " point\n\t\t"
        <<					"\t\t" << STATS_INT_GETI( SHD_var_uniform_vector ) << " vector\n\t\t"
        <<					"\t\t" << STATS_INT_GETI( SHD_var_uniform_normal ) << " normal\n\t\t"
        <<					"\t\t" << STATS_INT_GETI( SHD_var_uniform_color ) << " color\n\t\t"
        <<					"\t\t" << STATS_INT_GETI( SHD_var_uniform_matrix ) << " matrix\n\n\t\t"
        <<					"Varying:\n\t\t"
        <<					"\t" << STATS_INT_GETI( SHD_var_varying ) << " created (" << _shd_var_v << "%), " << STATS_INT_GETI( SHD_var_varying_peak ) << " peak\n\t\t"
        <<					"\t" << STATS_INT_GETI( SHD_Var_varying_init ) << " inits\n\n\t\t"
        <<					"\t\t" << STATS_INT_GETI( SHD_var_varying_float ) << " float\n\t\t"
        <<					"\t\t" << STATS_INT_GETI( SHD_var_varying_string ) << " string\n\t\t"
        <<					"\t\t" << STATS_INT_GETI( SHD_var_varying_point ) << " point\n\t\t"
        <<					"\t\t" << STATS_INT_GETI( SHD_var_varying_vector ) << " vector\n\t\t"
        <<					"\t\t" << STATS_INT_GETI( SHD_var_varying_normal ) << " normal\n\t\t"
        <<					"\t\t" << STATS_INT_GETI( SHD_var_varying_color ) << " color\n\t\t"
        <<					"\t\t" << STATS_INT_GETI( SHD_var_varying_matrix ) << " matrix\n\n\t"


        <<	"Shadeop calls:\n\t\t";
        if( STATS_INT_GETI( SHD_so_abs ) ) MSG << STATS_INT_GETI( SHD_so_abs ) << "\tabs\n\t\t";
        if( STATS_INT_GETI( SHD_so_acos ) ) MSG << STATS_INT_GETI( SHD_so_acos ) << "\tacos\n\t\t";
        if( STATS_INT_GETI( SHD_so_ambient ) ) MSG << STATS_INT_GETI( SHD_so_ambient ) << "\tambient\n\t\t";
        if( STATS_INT_GETI( SHD_so_area ) ) MSG << STATS_INT_GETI( SHD_so_area ) << "\tarea\n\t\t";
        if( STATS_INT_GETI( SHD_so_asin ) ) MSG << STATS_INT_GETI( SHD_so_asin ) << "\tasin\n\t\t";
        if( STATS_INT_GETI( SHD_so_atan ) ) MSG << STATS_INT_GETI( SHD_so_atan ) << "\tatan\n\t\t";
        if( STATS_INT_GETI( SHD_so_atmosphere ) ) MSG << STATS_INT_GETI( SHD_so_atmosphere ) << "\tatmosphere\n\t\t";
        if( STATS_INT_GETI( SHD_so_attribute ) ) MSG << STATS_INT_GETI( SHD_so_attribute ) << "\tattribute\n\t\t";
        if( STATS_INT_GETI( SHD_so_bake ) ) MSG << STATS_INT_GETI( SHD_so_bake ) << "\tbake\n\t\t";
        if( STATS_INT_GETI( SHD_so_bump ) ) MSG << STATS_INT_GETI( SHD_so_bump ) << "\tbump\n\t\t";
        if( STATS_INT_GETI( SHD_so_cDeriv ) ) MSG << STATS_INT_GETI( SHD_so_cDeriv ) << "\tcDeriv\n\t\t";
        if( STATS_INT_GETI( SHD_so_cDu ) ) MSG << STATS_INT_GETI( SHD_so_cDu ) << "\tcDu\n\t\t";
        if( STATS_INT_GETI( SHD_so_cDv ) ) MSG << STATS_INT_GETI( SHD_so_cDv ) << "\tcDv\n\t\t";
        if( STATS_INT_GETI( SHD_so_calculatenormal ) ) MSG << STATS_INT_GETI( SHD_so_calculatenormal ) << "\tcalculatenormal\n\t\t";
        if( STATS_INT_GETI( SHD_so_ccellnoise1 ) ) MSG << STATS_INT_GETI( SHD_so_ccellnoise1 ) << "\tccellnoise1\n\t\t";
        if( STATS_INT_GETI( SHD_so_ccellnoise2 ) ) MSG << STATS_INT_GETI( SHD_so_ccellnoise2 ) << "\tccellnoise2\n\t\t";
        if( STATS_INT_GETI( SHD_so_ccellnoise3 ) ) MSG << STATS_INT_GETI( SHD_so_ccellnoise3 ) << "\tccellnoise3\n\t\t";
        if( STATS_INT_GETI( SHD_so_ccellnoise4 ) ) MSG << STATS_INT_GETI( SHD_so_ccellnoise4 ) << "\tccellnoise4\n\t\t";
        if( STATS_INT_GETI( SHD_so_ceil ) ) MSG << STATS_INT_GETI( SHD_so_ceil ) << "\tceil\n\t\t";
        if( STATS_INT_GETI( SHD_so_cenvironment2 ) ) MSG << STATS_INT_GETI( SHD_so_cenvironment2 ) << "\tcenvironment2\n\t\t";
        if( STATS_INT_GETI( SHD_so_cenvironment3 ) ) MSG << STATS_INT_GETI( SHD_so_cenvironment3 ) << "\tcenvironment3\n\t\t";
        if( STATS_INT_GETI( SHD_so_cclamp ) ) MSG << STATS_INT_GETI( SHD_so_cclamp ) << "\tcclamp\n\t\t";
        if( STATS_INT_GETI( SHD_so_clamp ) ) MSG << STATS_INT_GETI( SHD_so_clamp ) << "\tclamp\n\t\t";
        if( STATS_INT_GETI( SHD_so_cmax ) ) MSG << STATS_INT_GETI( SHD_so_cmax ) << "\tcmax\n\t\t";
        if( STATS_INT_GETI( SHD_so_cmin ) ) MSG << STATS_INT_GETI( SHD_so_cmin ) << "\tcmin\n\t\t";
        if( STATS_INT_GETI( SHD_so_cmix ) ) MSG << STATS_INT_GETI( SHD_so_cmix ) << "\tcmix\n\t\t";
        if( STATS_INT_GETI( SHD_so_cnoise1 ) ) MSG << STATS_INT_GETI( SHD_so_cnoise1 ) << "\tcnoise1\n\t\t";
        if( STATS_INT_GETI( SHD_so_cnoise2 ) ) MSG << STATS_INT_GETI( SHD_so_cnoise2 ) << "\tcnoise2\n\t\t";
        if( STATS_INT_GETI( SHD_so_cnoise3 ) ) MSG << STATS_INT_GETI( SHD_so_cnoise3 ) << "\tcnoise3\n\t\t";
        if( STATS_INT_GETI( SHD_so_cnoise4 ) ) MSG << STATS_INT_GETI( SHD_so_cnoise4 ) << "\tcnoise4\n\t\t";
        if( STATS_INT_GETI( SHD_so_concat ) ) MSG << STATS_INT_GETI( SHD_so_concat ) << "\tconcat\n\t\t";
        if( STATS_INT_GETI( SHD_so_cos ) ) MSG << STATS_INT_GETI( SHD_so_cos ) << "\tcos\n\t\t";
        if( STATS_INT_GETI( SHD_so_cpnoise1 ) ) MSG << STATS_INT_GETI( SHD_so_cpnoise1 ) << "\tcpnoise1\n\t\t";
        if( STATS_INT_GETI( SHD_so_cpnoise2 ) ) MSG << STATS_INT_GETI( SHD_so_cpnoise2 ) << "\tcpnoise2\n\t\t";
        if( STATS_INT_GETI( SHD_so_cpnoise3 ) ) MSG << STATS_INT_GETI( SHD_so_cpnoise3 ) << "\tcpnoise3\n\t\t";
        if( STATS_INT_GETI( SHD_so_cpnoise4 ) ) MSG << STATS_INT_GETI( SHD_so_cpnoise4 ) << "\tcpnoise4\n\t\t";
        if( STATS_INT_GETI( SHD_so_crandom ) ) MSG << STATS_INT_GETI( SHD_so_crandom ) << "\tcrandom\n\t\t";
        if( STATS_INT_GETI( SHD_so_cspline ) ) MSG << STATS_INT_GETI( SHD_so_cspline ) << "\tcspline\n\t\t";
        if( STATS_INT_GETI( SHD_so_csplinea ) ) MSG << STATS_INT_GETI( SHD_so_csplinea ) << "\tcsplinea\n\t\t";
        if( STATS_INT_GETI( SHD_so_ctexture1 ) ) MSG << STATS_INT_GETI( SHD_so_ctexture1 ) << "\tctexture1\n\t\t";
        if( STATS_INT_GETI( SHD_so_ctexture2 ) ) MSG << STATS_INT_GETI( SHD_so_ctexture2 ) << "\tctexture2\n\t\t";
        if( STATS_INT_GETI( SHD_so_ctexture3 ) ) MSG << STATS_INT_GETI( SHD_so_ctexture3 ) << "\tctexture3\n\t\t";
        if( STATS_INT_GETI( SHD_so_ctransform ) ) MSG << STATS_INT_GETI( SHD_so_ctransform ) << "\tctransform\n\t\t";
        if( STATS_INT_GETI( SHD_so_degrees ) ) MSG << STATS_INT_GETI( SHD_so_degrees ) << "\tdegrees\n\t\t";
        if( STATS_INT_GETI( SHD_so_depth ) ) MSG << STATS_INT_GETI( SHD_so_depth ) << "\tdepth\n\t\t";
        if( STATS_INT_GETI( SHD_so_determinant ) ) MSG << STATS_INT_GETI( SHD_so_determinant ) << "\tdeterminant\n\t\t";
        if( STATS_INT_GETI( SHD_so_diffuse ) ) MSG << STATS_INT_GETI( SHD_so_diffuse ) << "\tdiffuse\n\t\t";
        if( STATS_INT_GETI( SHD_so_displacement ) ) MSG << STATS_INT_GETI( SHD_so_displacement ) << "\tdisplacement\n\t\t";
        if( STATS_INT_GETI( SHD_so_distance ) ) MSG << STATS_INT_GETI( SHD_so_distance ) << "\tdistance\n\t\t";
        if( STATS_INT_GETI( SHD_so_exp ) ) MSG << STATS_INT_GETI( SHD_so_exp ) << "\texp\n\t\t";
        if( STATS_INT_GETI( SHD_so_external ) ) MSG << STATS_INT_GETI( SHD_so_external ) << "\texternal\n\t\t";
        if( STATS_INT_GETI( SHD_so_fDeriv ) ) MSG << STATS_INT_GETI( SHD_so_fDeriv ) << "\tfDeriv\n\t\t";
        if( STATS_INT_GETI( SHD_so_fDu ) ) MSG << STATS_INT_GETI( SHD_so_fDu ) << "\tfDu\n\t\t";
        if( STATS_INT_GETI( SHD_so_fDv ) ) MSG << STATS_INT_GETI( SHD_so_fDv ) << "\tfDv\n\t\t";
        if( STATS_INT_GETI( SHD_so_faceforward ) ) MSG << STATS_INT_GETI( SHD_so_faceforward ) << "\tfaceforward\n\t\t";
        if( STATS_INT_GETI( SHD_so_faceforward2 ) ) MSG << STATS_INT_GETI( SHD_so_faceforward2 ) << "\tfaceforward2\n\t\t";
        if( STATS_INT_GETI( SHD_so_fcellnoise1 ) ) MSG << STATS_INT_GETI( SHD_so_fcellnoise1 ) << "\tfcellnoise1\n\t\t";
        if( STATS_INT_GETI( SHD_so_fcellnoise2 ) ) MSG << STATS_INT_GETI( SHD_so_fcellnoise2 ) << "\tfcellnoise2\n\t\t";
        if( STATS_INT_GETI( SHD_so_fcellnoise3 ) ) MSG << STATS_INT_GETI( SHD_so_fcellnoise3 ) << "\tfcellnoise3\n\t\t";
        if( STATS_INT_GETI( SHD_so_fcellnoise4 ) ) MSG << STATS_INT_GETI( SHD_so_fcellnoise4 ) << "\tfcellnoise4\n\t\t";
        if( STATS_INT_GETI( SHD_so_fenvironment2 ) ) MSG << STATS_INT_GETI( SHD_so_fenvironment2 ) << "\tfenvironment2\n\t\t";
        if( STATS_INT_GETI( SHD_so_fenvironment3 ) ) MSG << STATS_INT_GETI( SHD_so_fenvironment3 ) << "\tfenvironment3\n\t\t";
        if( STATS_INT_GETI( SHD_so_filterstep ) ) MSG << STATS_INT_GETI( SHD_so_filterstep ) << "\tfilterstep\n\t\t";
        if( STATS_INT_GETI( SHD_so_filterstep2 ) ) MSG << STATS_INT_GETI( SHD_so_filterstep2 ) << "\tfilterstep2\n\t\t";
        if( STATS_INT_GETI( SHD_so_floor ) ) MSG << STATS_INT_GETI( SHD_so_floor ) << "\tfloor\n\t\t";
        if( STATS_INT_GETI( SHD_so_fmix ) ) MSG << STATS_INT_GETI( SHD_so_fmix ) << "\tfmix\n\t\t";
        if( STATS_INT_GETI( SHD_so_fnoise1 ) ) MSG << STATS_INT_GETI( SHD_so_fnoise1 ) << "\tfnoise1\n\t\t";
        if( STATS_INT_GETI( SHD_so_fnoise2 ) ) MSG << STATS_INT_GETI( SHD_so_fnoise2 ) << "\tfnoise2\n\t\t";
        if( STATS_INT_GETI( SHD_so_fnoise3 ) ) MSG << STATS_INT_GETI( SHD_so_fnoise3 ) << "\tfnoise3\n\t\t";
        if( STATS_INT_GETI( SHD_so_fnoise4 ) ) MSG << STATS_INT_GETI( SHD_so_fnoise4 ) << "\tfnoise4\n\t\t";
        if( STATS_INT_GETI( SHD_so_format ) ) MSG << STATS_INT_GETI( SHD_so_format ) << "\tformat\n\t\t";
        if( STATS_INT_GETI( SHD_so_fpnoise1 ) ) MSG << STATS_INT_GETI( SHD_so_fpnoise1 ) << "\tfpnoise1\n\t\t";
        if( STATS_INT_GETI( SHD_so_fpnoise2 ) ) MSG << STATS_INT_GETI( SHD_so_fpnoise2 ) << "\tfpnoise2\n\t\t";
        if( STATS_INT_GETI( SHD_so_fpnoise3 ) ) MSG << STATS_INT_GETI( SHD_so_fpnoise3 ) << "\tfpnoise3\n\t\t";
        if( STATS_INT_GETI( SHD_so_fpnoise4 ) ) MSG << STATS_INT_GETI( SHD_so_fpnoise4 ) << "\tfpnoise4\n\t\t";
        if( STATS_INT_GETI( SHD_so_frandom ) ) MSG << STATS_INT_GETI( SHD_so_frandom ) << "\tfrandom\n\t\t";
        if( STATS_INT_GETI( SHD_so_fresnel ) ) MSG << STATS_INT_GETI( SHD_so_fresnel ) << "\tfresnel\n\t\t";
        if( STATS_INT_GETI( SHD_so_fspline ) ) MSG << STATS_INT_GETI( SHD_so_fspline ) << "\tfspline\n\t\t";
        if( STATS_INT_GETI( SHD_so_fsplinea ) ) MSG << STATS_INT_GETI( SHD_so_fsplinea ) << "\tfsplinea\n\t\t";
        if( STATS_INT_GETI( SHD_so_ftexture1 ) ) MSG << STATS_INT_GETI( SHD_so_ftexture1 ) << "\tftexture1\n\t\t";
        if( STATS_INT_GETI( SHD_so_ftexture2 ) ) MSG << STATS_INT_GETI( SHD_so_ftexture2 ) << "\tftexture2\n\t\t";
        if( STATS_INT_GETI( SHD_so_ftexture3 ) ) MSG << STATS_INT_GETI( SHD_so_ftexture3 ) << "\tftexture3\n\t\t";
        if( STATS_INT_GETI( SHD_so_illuminance ) ) MSG << STATS_INT_GETI( SHD_so_illuminance ) << "\tilluminance\n\t\t";
        if( STATS_INT_GETI( SHD_so_illuminate ) ) MSG << STATS_INT_GETI( SHD_so_illuminate ) << "\tilluminate\n\t\t";
        if( STATS_INT_GETI( SHD_so_incident ) ) MSG << STATS_INT_GETI( SHD_so_incident ) << "\tincident\n\t\t";
        if( STATS_INT_GETI( SHD_so_inversesqrt ) ) MSG << STATS_INT_GETI( SHD_so_inversesqrt ) << "\tinversesqrt\n\t\t";
        if( STATS_INT_GETI( SHD_so_length ) ) MSG << STATS_INT_GETI( SHD_so_length ) << "\tlength\n\t\t";
        if( STATS_INT_GETI( SHD_so_lightsource ) ) MSG << STATS_INT_GETI( SHD_so_lightsource ) << "\tlightsource\n\t\t";
        if( STATS_INT_GETI( SHD_so_log ) ) MSG << STATS_INT_GETI( SHD_so_log ) << "\tlog\n\t\t";
        if( STATS_INT_GETI( SHD_so_match ) ) MSG << STATS_INT_GETI( SHD_so_match ) << "\tmatch\n\t\t";
        if( STATS_INT_GETI( SHD_so_max ) ) MSG << STATS_INT_GETI( SHD_so_max ) << "\tmax\n\t\t";
        if( STATS_INT_GETI( SHD_so_min ) ) MSG << STATS_INT_GETI( SHD_so_min ) << "\tmin\n\t\t";
        if( STATS_INT_GETI( SHD_so_mod ) ) MSG << STATS_INT_GETI( SHD_so_mod ) << "\tmod\n\t\t";
        if( STATS_INT_GETI( SHD_so_mrotate ) ) MSG << STATS_INT_GETI( SHD_so_mrotate ) << "\tmrotate\n\t\t";
        if( STATS_INT_GETI( SHD_so_mscale ) ) MSG << STATS_INT_GETI( SHD_so_mscale ) << "\tmscale\n\t\t";
        if( STATS_INT_GETI( SHD_so_mtranslate ) ) MSG << STATS_INT_GETI( SHD_so_mtranslate ) << "\tmtranslate\n\t\t";
        if( STATS_INT_GETI( SHD_so_normalize ) ) MSG << STATS_INT_GETI( SHD_so_normalize ) << "\tnormalize\n\t\t";
        if( STATS_INT_GETI( SHD_so_ntransform ) ) MSG << STATS_INT_GETI( SHD_so_ntransform ) << "\tntransform\n\t\t";
        if( STATS_INT_GETI( SHD_so_occlusion ) ) MSG << STATS_INT_GETI( SHD_so_occlusion ) << "\tocclusion\n\t\t";
        if( STATS_INT_GETI( SHD_so_opposite ) ) MSG << STATS_INT_GETI( SHD_so_opposite ) << "\topposite\n\t\t";
        if( STATS_INT_GETI( SHD_so_option ) ) MSG << STATS_INT_GETI( SHD_so_option ) << "\toption\n\t\t";
        if( STATS_INT_GETI( SHD_so_pDeriv ) ) MSG << STATS_INT_GETI( SHD_so_pDeriv ) << "\tpDeriv\n\t\t";
        if( STATS_INT_GETI( SHD_so_pDu ) ) MSG << STATS_INT_GETI( SHD_so_pDu ) << "\tpDu\n\t\t";
        if( STATS_INT_GETI( SHD_so_pDv ) ) MSG << STATS_INT_GETI( SHD_so_pDv ) << "\tpDv\n\t\t";
        if( STATS_INT_GETI( SHD_so_pcellnoise1 ) ) MSG << STATS_INT_GETI( SHD_so_pcellnoise1 ) << "\tpcellnoise1\n\t\t";
        if( STATS_INT_GETI( SHD_so_pcellnoise2 ) ) MSG << STATS_INT_GETI( SHD_so_pcellnoise2 ) << "\tpcellnoise2\n\t\t";
        if( STATS_INT_GETI( SHD_so_pcellnoise3 ) ) MSG << STATS_INT_GETI( SHD_so_pcellnoise3 ) << "\tpcellnoise3\n\t\t";
        if( STATS_INT_GETI( SHD_so_pcellnoise4 ) ) MSG << STATS_INT_GETI( SHD_so_pcellnoise4 ) << "\tpcellnoise4\n\t\t";
        if( STATS_INT_GETI( SHD_so_pclamp ) ) MSG << STATS_INT_GETI( SHD_so_pclamp ) << "\tpclamp\n\t\t";
        if( STATS_INT_GETI( SHD_so_phong ) ) MSG << STATS_INT_GETI( SHD_so_phong ) << "\tphong\n\t\t";
        if( STATS_INT_GETI( SHD_so_pmax ) ) MSG << STATS_INT_GETI( SHD_so_pmax ) << "\tpmax\n\t\t";
        if( STATS_INT_GETI( SHD_so_pmin ) ) MSG << STATS_INT_GETI( SHD_so_pmin ) << "\tpmin\n\t\t";
        if( STATS_INT_GETI( SHD_so_pmix ) ) MSG << STATS_INT_GETI( SHD_so_pmix ) << "\tpmix\n\t\t";
        if( STATS_INT_GETI( SHD_so_pnoise1 ) ) MSG << STATS_INT_GETI( SHD_so_pnoise1 ) << "\tpnoise1\n\t\t";
        if( STATS_INT_GETI( SHD_so_pnoise2 ) ) MSG << STATS_INT_GETI( SHD_so_pnoise2 ) << "\tpnoise2\n\t\t";
        if( STATS_INT_GETI( SHD_so_pnoise3 ) ) MSG << STATS_INT_GETI( SHD_so_pnoise3 ) << "\tpnoise3\n\t\t";
        if( STATS_INT_GETI( SHD_so_pnoise4 ) ) MSG << STATS_INT_GETI( SHD_so_pnoise4 ) << "\tpnoise4\n\t\t";
        if( STATS_INT_GETI( SHD_so_pow ) ) MSG << STATS_INT_GETI( SHD_so_pow ) << "\tpow\n\t\t";
        if( STATS_INT_GETI( SHD_so_ppnoise1 ) ) MSG << STATS_INT_GETI( SHD_so_ppnoise1 ) << "\tppnoise1\n\t\t";
        if( STATS_INT_GETI( SHD_so_ppnoise2 ) ) MSG << STATS_INT_GETI( SHD_so_ppnoise2 ) << "\tppnoise2\n\t\t";
        if( STATS_INT_GETI( SHD_so_ppnoise3 ) ) MSG << STATS_INT_GETI( SHD_so_ppnoise3 ) << "\tppnoise3\n\t\t";
        if( STATS_INT_GETI( SHD_so_ppnoise4 ) ) MSG << STATS_INT_GETI( SHD_so_ppnoise4 ) << "\tppnoise4\n\t\t";
        if( STATS_INT_GETI( SHD_so_prandom ) ) MSG << STATS_INT_GETI( SHD_so_prandom ) << "\tprandom\n\t\t";
        if( STATS_INT_GETI( SHD_so_printf ) ) MSG << STATS_INT_GETI( SHD_so_printf ) << "\tprintf\n\t\t";
        if( STATS_INT_GETI( SHD_so_pspline ) ) MSG << STATS_INT_GETI( SHD_so_pspline ) << "\tpspline\n\t\t";
        if( STATS_INT_GETI( SHD_so_psplinea ) ) MSG << STATS_INT_GETI( SHD_so_psplinea ) << "\tpsplinea\n\t\t";
        if( STATS_INT_GETI( SHD_so_ptlined ) ) MSG << STATS_INT_GETI( SHD_so_ptlined ) << "\tptlined\n\t\t";
        if( STATS_INT_GETI( SHD_so_radians ) ) MSG << STATS_INT_GETI( SHD_so_radians ) << "\tradians\n\t\t";
        if( STATS_INT_GETI( SHD_so_reflect ) ) MSG << STATS_INT_GETI( SHD_so_reflect ) << "\treflect\n\t\t";
        if( STATS_INT_GETI( SHD_so_refract ) ) MSG << STATS_INT_GETI( SHD_so_refract ) << "\trefract\n\t\t";
        if( STATS_INT_GETI( SHD_so_rendererinfo ) ) MSG << STATS_INT_GETI( SHD_so_rendererinfo ) << "\trendererinfo\n\t\t";
        if( STATS_INT_GETI( SHD_so_rotate ) ) MSG << STATS_INT_GETI( SHD_so_rotate ) << "\trotate\n\t\t";
        if( STATS_INT_GETI( SHD_so_round ) ) MSG << STATS_INT_GETI( SHD_so_round ) << "\tround\n\t\t";
        if( STATS_INT_GETI( SHD_so_scspline ) ) MSG << STATS_INT_GETI( SHD_so_scspline ) << "\tscspline\n\t\t";
        if( STATS_INT_GETI( SHD_so_scsplinea ) ) MSG << STATS_INT_GETI( SHD_so_scsplinea ) << "\tscsplinea\n\t\t";
        if( STATS_INT_GETI( SHD_so_setcomp ) ) MSG << STATS_INT_GETI( SHD_so_setcomp ) << "\tsetcomp\n\t\t";
        if( STATS_INT_GETI( SHD_so_setmcomp ) ) MSG << STATS_INT_GETI( SHD_so_setmcomp ) << "\tsetmcomp\n\t\t";
        if( STATS_INT_GETI( SHD_so_setxcomp ) ) MSG << STATS_INT_GETI( SHD_so_setxcomp ) << "\tsetxcomp\n\t\t";
        if( STATS_INT_GETI( SHD_so_setycomp ) ) MSG << STATS_INT_GETI( SHD_so_setycomp ) << "\tsetycomp\n\t\t";
        if( STATS_INT_GETI( SHD_so_setzcomp ) ) MSG << STATS_INT_GETI( SHD_so_setzcomp ) << "\tsetzcomp\n\t\t";
        if( STATS_INT_GETI( SHD_so_sfspline ) ) MSG << STATS_INT_GETI( SHD_so_sfspline ) << "\tsfspline\n\t\t";
        if( STATS_INT_GETI( SHD_so_sfsplinea ) ) MSG << STATS_INT_GETI( SHD_so_sfsplinea ) << "\tsfsplinea\n\t\t";
        if( STATS_INT_GETI( SHD_so_shadername ) ) MSG << STATS_INT_GETI( SHD_so_shadername ) << "\tshadername\n\t\t";
        if( STATS_INT_GETI( SHD_so_shadername2 ) ) MSG << STATS_INT_GETI( SHD_so_shadername2 ) << "\tshadername2\n\t\t";
        if( STATS_INT_GETI( SHD_so_shadow ) ) MSG << STATS_INT_GETI( SHD_so_shadow ) << "\tshadow\n\t\t";
        if( STATS_INT_GETI( SHD_so_shadow1 ) ) MSG << STATS_INT_GETI( SHD_so_shadow1 ) << "\tshadow1\n\t\t";
        if( STATS_INT_GETI( SHD_so_sign ) ) MSG << STATS_INT_GETI( SHD_so_sign ) << "\tsign\n\t\t";
        if( STATS_INT_GETI( SHD_so_sin ) ) MSG << STATS_INT_GETI( SHD_so_sin ) << "\tsin\n\t\t";
        if( STATS_INT_GETI( SHD_so_smoothstep ) ) MSG << STATS_INT_GETI( SHD_so_smoothstep ) << "\tsmoothstep\n\t\t";
        if( STATS_INT_GETI( SHD_so_solar ) ) MSG << STATS_INT_GETI( SHD_so_solar ) << "\tsolar\n\t\t";
        if( STATS_INT_GETI( SHD_so_specular ) ) MSG << STATS_INT_GETI( SHD_so_specular ) << "\tspecular\n\t\t";
        if( STATS_INT_GETI( SHD_so_specularbrdf ) ) MSG << STATS_INT_GETI( SHD_so_specularbrdf ) << "\tspecularbrdf\n\t\t";
        if( STATS_INT_GETI( SHD_so_spspline ) ) MSG << STATS_INT_GETI( SHD_so_spspline ) << "\tspspline\n\t\t";
        if( STATS_INT_GETI( SHD_so_spsplinea ) ) MSG << STATS_INT_GETI( SHD_so_spsplinea ) << "\tspsplinea\n\t\t";
        if( STATS_INT_GETI( SHD_so_sqrt ) ) MSG << STATS_INT_GETI( SHD_so_sqrt ) << "\tsqrt\n\t\t";
        if( STATS_INT_GETI( SHD_so_step ) ) MSG << STATS_INT_GETI( SHD_so_step ) << "\tstep\n\t\t";
        if( STATS_INT_GETI( SHD_so_surface ) ) MSG << STATS_INT_GETI( SHD_so_surface ) << "\tsurface\n\t\t";
        if( STATS_INT_GETI( SHD_so_tan ) ) MSG << STATS_INT_GETI( SHD_so_tan ) << "\ttan\n\t\t";
        if( STATS_INT_GETI( SHD_so_textureinfo ) ) MSG << STATS_INT_GETI( SHD_so_textureinfo ) << "\ttextureinfo\n\t\t";
        if( STATS_INT_GETI( SHD_so_trace ) ) MSG << STATS_INT_GETI( SHD_so_trace ) << "\ttrace\n\t\t";
        if( STATS_INT_GETI( SHD_so_transform ) ) MSG << STATS_INT_GETI( SHD_so_transform ) << "\ttransform\n\t\t";
        if( STATS_INT_GETI( SHD_so_vmix ) ) MSG << STATS_INT_GETI( SHD_so_vmix ) << "\tvmix\n\t\t";
        if( STATS_INT_GETI( SHD_so_vtransform ) ) MSG << STATS_INT_GETI( SHD_so_vtransform ) << "\tvtransform\n\t\t";
        MSG << std::endl;

        MSG << "Attributes:\n\t";
        MSG << ( TqInt ) Attribute_stack.size() << " created\n" << std::endl;

        MSG << "Transforms:\n\t";
        MSG << QGetRenderContext() ->TransformStack().size() << " created\n" << std::endl;

        MSG << "Parameters:\n\t" << STATS_INT_GETI( PRM_created ) << " created, " << STATS_INT_GETI( PRM_peak ) << " peak\n" << std::endl;

    }

    if ( level == 3 )
    {
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

    std::cerr << info << "Image settings:" << std::endl;
    std::cerr << info << "	Resolution: " << resX << " " << resY << std::endl;
    std::cerr << info << "	PixelAspectRatio: " << pratio << std::endl;
    std::cerr << info << "	Exposure:" << std::endl;
    std::cerr << info << "		Gain: " << gain << std::endl;
    std::cerr << info << "		Gamma: " << gamma << std::endl;
    std::cerr << info << "Shading:" << std::endl;
    std::cerr << info << "	Bucket size: [ " << bX << " " << bY << "]" << std::endl;
    std::cerr << info << "	Gridsize: " << gs << std::endl;
    std::cerr << info << "Anti-aliasing settings: " << std::endl;
    std::cerr << info << "	PixelSamples: " << psX << " " << psY << std::endl;
    std::cerr << info << "	FilterWidth: " << fX << " " << fY << std::endl;

}

//---------------------------------------------------------------------

END_NAMESPACE( Aqsis )

