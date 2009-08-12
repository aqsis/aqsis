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
/** \file
		\brief Declares CqStats class for holding global renderer statistics information.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#include "stats.h"

#include <iomanip>
#include <iostream>
#include <cstring>
#include <string>

#include "attributes.h"
#include "imagebuffer.h"
#include "renderer.h"
#include "transform.h"
#include <aqsis/math/math.h>

namespace Aqsis {

#ifdef USE_TIMERS

/// Global collection of timer objects.
CqTimerSet<EqTimerStats> g_timerSet;

#endif // USE_TIMERS

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
	TqInt i;
	m_Complete = 0.0f;
	for (i = _First_int; i < _Last_int; i++)
		m_intVars[i] = 0;
	for (i = _First_float; i < _Last_float; i++)
		m_floatVars[i] = 0.0f;
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
}
//----------------------------------------------------------------------
/** Output rendering stats if required.
 
    \param level  Verbosity level as set by Options "statistics" "endofframe"
 */
void CqStats::PrintStats( TqInt level ) const
{
#	define STATS_INT_GETI( index )	getI( index )
#	define STATS_INT_GETF( index )	getF( index )

	std::ostream& MSG = std::cout;
	/*! Levels
		Minimum := 0
		Normal  := 1
		Verbose := 2
		Max		:= 3
	*/
#	ifdef USE_TIMERS
	if( level > 0 )
		g_timerSet.printTimes(MSG);
#	endif // USE_TIMERS

	MSG << std::setiosflags(std::ios_base::fixed)
		<< std::setfill(' ') << std::setprecision(6);

	//! Most important informations
	if ( level == 2 || level == 3 )
	{
		/*
			-------------------------------------------------------------------
			GPrim stats
		*/
		TqFloat _gpr_c_q = 100.0f * STATS_INT_GETI( GPR_culled ) / STATS_INT_GETI( GPR_created_total );
		TqFloat _gpr_oc_q = 100.0f * STATS_INT_GETI( GPR_occlusion_culled ) / STATS_INT_GETI( GPR_created_total );
		TqFloat _gpr_u_q = 100.0f * STATS_INT_GETI( GPR_created_total ) / STATS_INT_GETI( GPR_allocated );
		MSG << "Input geometry:\n\t" << STATS_INT_GETI( GPR_created ) << " primitives created\n\n"
		<< "\t"	<<	STATS_INT_GETI( GPR_subdiv ) << " subdivision primitives\n\t"
		<<			STATS_INT_GETI( GPR_blobbies )   << " blobbies\n\t"
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
		<< STATS_INT_GETI( GPR_culled ) << " culled (" << _gpr_c_q << "%)\n\t"
		<< STATS_INT_GETI( GPR_occlusion_culled ) << " occlusion culled (" << _gpr_oc_q << "%)\n" << std::endl;
		/*
			GPrim stats - End
			-------------------------------------------------------------------
		*/
		/*
			-------------------------------------------------------------------
			Geometry stats
		*/
		// Curves
		TqFloat _geo_crv_s_q = 0.0f;
		if (STATS_INT_GETI(GPR_crv))
			_geo_crv_s_q = 100.0f * STATS_INT_GETI( GEO_crv_splits ) / STATS_INT_GETI( GPR_crv );
		TqFloat _geo_crv_s_c_q  = 0.0f;
		if (STATS_INT_GETI(GEO_crv_splits))
			_geo_crv_s_c_q = 100.0f * STATS_INT_GETI( GEO_crv_crv ) / STATS_INT_GETI( GEO_crv_splits );
		TqFloat _geo_crv_s_p_q = 0.0f;
		if (STATS_INT_GETI(GEO_crv_splits))
			_geo_crv_s_p_q = 100.0f * STATS_INT_GETI( GEO_crv_patch ) / STATS_INT_GETI( GEO_crv_splits );
		// Procedural
		TqFloat _geo_prc_s_q = 0.0f;
		if (STATS_INT_GETI( GEO_prc_created ))
			_geo_prc_s_q = 100.0f * STATS_INT_GETI( GEO_prc_split ) / STATS_INT_GETI( GEO_prc_created );
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
		TqFloat	_grd_init_quote	= 0.0f;
		TqFloat	_grd_shade_quote= 0.0f;
		TqFloat	_grd_cull_quote = 0.0f;
		if (STATS_INT_GETI(GRD_created))
		{
			_grd_init_quote = 100.0f *  _grd_init / STATS_INT_GETI( GRD_created );
			_grd_shade_quote = 100.0f *  _grd_shade / STATS_INT_GETI( GRD_created );
			_grd_cull_quote = 100.0f *  STATS_INT_GETI( GRD_culled ) / STATS_INT_GETI( GRD_created );
		}
		if (_grd_init == 0)
			_grd_init = 1;
		TqFloat	_grd_4		=	100.0f * STATS_INT_GETI( GRD_size_4 ) / _grd_init;
		TqFloat	_grd_8		=	100.0f * STATS_INT_GETI( GRD_size_8 ) / _grd_init;
		TqFloat	_grd_16		=	100.0f * STATS_INT_GETI( GRD_size_16 ) / _grd_init;
		TqFloat	_grd_32		=	100.0f * STATS_INT_GETI( GRD_size_32 ) / _grd_init;
		TqFloat	_grd_64		=	100.0f * STATS_INT_GETI( GRD_size_64 ) / _grd_init;
		TqFloat	_grd_128	=	100.0f * STATS_INT_GETI( GRD_size_128 ) / _grd_init;
		TqFloat	_grd_256	=	100.0f * STATS_INT_GETI( GRD_size_256 ) / _grd_init;
		TqFloat	_grd_g256	=	100.0f * STATS_INT_GETI( GRD_size_g256 ) / _grd_init;
		if (_grd_shade == 0)
			_grd_shade = 1;
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
		if (_mpg_pushes_all == 0)
			_mpg_pushes_all = 1;
		TqFloat _mpg_p_f =  100.0f * STATS_INT_GETI( MPG_pushed_forward ) / _mpg_pushes_all;
		TqFloat _mpg_p_d = 100.0f * STATS_INT_GETI( MPG_pushed_down ) / _mpg_pushes_all;
		TqFloat _mpg_p_fd = 100.0f * STATS_INT_GETI( MPG_pushed_far_down ) / _mpg_pushes_all;
		TqFloat _mpg_p_a = 0.0f;
		TqFloat _mpg_m_q = 0.0f;
		TqFloat _mpg_average_ratio = 0.0f;
		if (STATS_INT_GETI(MPG_allocated))
		{
			_mpg_p_a = 100.0f * _mpg_pushes_all / STATS_INT_GETI( MPG_allocated );
			_mpg_m_q =  100.0f * STATS_INT_GETI( MPG_missed ) /STATS_INT_GETI( MPG_allocated );
			_mpg_average_ratio = STATS_INT_GETF( MPG_average_area ) / (TqFloat) STATS_INT_GETI( MPG_allocated );
		}
		// Sample hit quote
		TqInt _mpg_hits =	STATS_INT_GETI ( MPG_sample_coverage0_125 ) +
		                  STATS_INT_GETI ( MPG_sample_coverage125_25 ) +
		                  STATS_INT_GETI ( MPG_sample_coverage25_375 ) +
		                  STATS_INT_GETI ( MPG_sample_coverage375_50 ) +
		                  STATS_INT_GETI ( MPG_sample_coverage50_625 ) +
		                  STATS_INT_GETI ( MPG_sample_coverage625_75 ) +
		                  STATS_INT_GETI ( MPG_sample_coverage75_875 ) +
		                  STATS_INT_GETI ( MPG_sample_coverage875_100 );
		if (_mpg_hits == 0)
			_mpg_hits = 1;
		TqFloat	_mpg_1		=	100.0f * STATS_INT_GETI( MPG_sample_coverage0_125 ) / _mpg_hits;
		TqFloat	_mpg_2		=	100.0f * STATS_INT_GETI( MPG_sample_coverage125_25 ) / _mpg_hits;
		TqFloat	_mpg_3		=	100.0f * STATS_INT_GETI( MPG_sample_coverage25_375 ) / _mpg_hits;
		TqFloat	_mpg_4		=	100.0f * STATS_INT_GETI( MPG_sample_coverage375_50 ) / _mpg_hits;
		TqFloat	_mpg_5		=	100.0f * STATS_INT_GETI( MPG_sample_coverage50_625 ) / _mpg_hits;
		TqFloat	_mpg_6		=	100.0f * STATS_INT_GETI( MPG_sample_coverage625_75 ) / _mpg_hits;
		TqFloat	_mpg_7		=	100.0f * STATS_INT_GETI( MPG_sample_coverage75_875 ) / _mpg_hits;
		TqFloat	_mpg_8		=	100.0f * STATS_INT_GETI( MPG_sample_coverage875_100 ) / _mpg_hits;
		TqFloat _mpg_min = 0.0f;
		if (STATS_INT_GETF( MPG_min_area ) != FLT_MAX)
			_mpg_min = STATS_INT_GETF( MPG_min_area );
		TqFloat _mpg_max = 0.0f;
		if (STATS_INT_GETF( MPG_max_area ) != FLT_MIN)
			_mpg_max = STATS_INT_GETF( MPG_max_area );
		MSG << "Micropolygons:\n\t"
		<< STATS_INT_GETI( MPG_allocated ) << " created (" << STATS_INT_GETI( MPG_culled ) << " culled)\n"
		<< "\t" <<STATS_INT_GETI( MPG_peak ) << " peak, " << STATS_INT_GETI( MPG_trimmed ) << " trimmed, ( " << STATS_INT_GETI( MPG_trimmedout ) << " completely ) " << STATS_INT_GETI( MPG_missed ) << " missed (" << _mpg_m_q << "%)\n\t"
		<< "\n\tMPG Area:\t" << _mpg_average_ratio << " average \n\t\t\t"
		<<  _mpg_min << " min\n\t\t\t"
		<<  _mpg_max << " max\n\t"
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
		TqFloat _spl_b_h = 0.0f;
		TqFloat _spl_h = 0.0f;
		TqFloat _spl_m = 100.0f;
		if (STATS_INT_GETI( SPL_count ))
		{
			_spl_b_h = 100.0f * STATS_INT_GETI( SPL_bound_hits ) / STATS_INT_GETI( SPL_count );
			_spl_h = 100.0f * STATS_INT_GETI( SPL_hits ) / STATS_INT_GETI( SPL_count );
			_spl_m = 100.0f - _spl_b_h - _spl_h;
		}
		TqInt _spl_px = QGetRenderContext() ->poptCurrent()->GetIntegerOption( "System", "PixelSamples" ) [ 0 ];
		TqInt _spl_py = QGetRenderContext() ->poptCurrent()->GetIntegerOption( "System", "PixelSamples" ) [ 1 ];
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
		MSG << "Attributes:\n\t";
		MSG << ( TqInt ) Attribute_stack.size() << " created\n" << std::endl;
		// MSG << "Transforms:\n\t";
		// MSG << ( TqInt ) Transform_stack.size() << " created\n" << std::endl;
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
						case 0:
						MSG << "\t\t\tMipMap   P(";
						break;
						case 1:
						MSG << "\t\t\tCube Env.P(";
						break;
						case 2:
						MSG << "\t\t\tLatLong  P(";
						break;
						case 3:
						MSG << "\t\t\tShadow   P(";
						break;
						case 4:
						MSG << "\t\t\tTiles    P(";
						break;
				}
				MSG << 100.0f * ( ( float ) m_cTextureHits[ 0 ][ i ] / ( float ) ( m_cTextureHits[ 0 ][ i ] + m_cTextureMisses[ i ] ) ) << "%)" << " of " << m_cTextureMisses[ i ] << " tries" << std::endl;
			}
			if ( m_cTextureHits[ 1 ][ i ] )
			{
				switch ( i )
				{
						case 0:
						MSG << "\t\t\tMipMap   S(";
						break;
						case 1:
						MSG << "\t\t\tCube Env.S(";
						break;
						case 2:
						MSG << "\t\t\tLatLong  S(";
						break;
						case 3:
						MSG << "\t\t\tShadow   S(";
						break;
						case 4:
						MSG << "\t\t\tTiles    S(";
						break;
				}
				MSG << 100.0f * ( ( float ) m_cTextureHits[ 1 ][ i ] / ( float ) ( m_cTextureHits[ 1 ][ i ] + m_cTextureMisses[ i ] ) ) << "%)" << std::endl;
			}
		}
		MSG << std::endl;
	}
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
	if ( t > 5.0 )
		t = round(t);
	TqInt h = static_cast<TqInt>( t / ( 60 * 60 ) );
	TqInt m = static_cast<TqInt>( ( t / 60 ) - ( h * 60 ) );
	TqFloat s = ( t ) - ( h * 60 * 60 ) - ( m * 60 );
	if ( h > 0 )
		os << std::setiosflags(std::ios::fixed)
			<< std::setprecision(1) << std::setw(6) << h << "hrs ";
	if ( m > 0 )
		os << std::setiosflags(std::ios::fixed)
			<< std::setprecision(1) << std::setw(6) << m << "mins ";
	os << std::setiosflags(std::ios::fixed)
		<< std::setprecision(1) << std::setw(6) << s << "secs";
	if( tot >= 0 )
		os << " (" << std::setprecision(2) << std::setw(6) << 100.0f * ticks / tot << "%)";
	return os;
}

const char* filterFunctionName(RtFilterFunc func)
{
	if(func == RiGaussianFilter)
		return "gaussian";
	else if(func == RiMitchellFilter)
		return "mitchell";
	else if(func == RiBoxFilter)
		return "box";
	else if(func == RiTriangleFilter)
		return "triangle";
	else if(func == RiCatmullRomFilter)
		return "catmull-rom";
	else if(func == RiSincFilter)
		return "sinc";
	else if(func == RiDiskFilter)
		return "disk";
	else if(func == RiBesselFilter)
		return "bessel";
	else
		return "user-defined";
}

void CqStats::PrintInfo() const
{
	TqInt psX, psY; //< Pixel Samples
	TqInt resX, resY;	//< Image resolution
	TqInt fX, fY;	//< Filter width
	std::string fName; //< Filter name
	TqFloat gain, gamma; //< Exposure, gain
	TqFloat pratio; //< PixelAspectRatio
	TqInt bX = 16, bY = 16; //< Bucket Size
	TqInt gs; //< Grid Size
	psX = QGetRenderContext() ->poptCurrent()->GetIntegerOption( "System", "PixelSamples" ) [ 0 ];
	psY = QGetRenderContext() ->poptCurrent()->GetIntegerOption( "System", "PixelSamples" ) [ 1 ];
	resX = QGetRenderContext() ->poptCurrent()->GetIntegerOption( "System", "Resolution" ) [ 0 ];
	resY = QGetRenderContext() ->poptCurrent()->GetIntegerOption( "System", "Resolution" ) [ 1 ];
	fX = (TqInt) QGetRenderContext() ->poptCurrent()->GetFloatOption( "System", "FilterWidth" ) [ 0 ];
	fY = (TqInt) QGetRenderContext() ->poptCurrent()->GetFloatOption( "System", "FilterWidth" ) [ 1 ];

	fName = filterFunctionName(QGetRenderContext()->poptCurrent()->funcFilter());

	gain = QGetRenderContext() ->poptCurrent()->GetFloatOption( "System", "Exposure" ) [ 0 ];
	gamma = QGetRenderContext() ->poptCurrent()->GetFloatOption( "System", "Exposure" ) [ 1 ];
	pratio = QGetRenderContext() ->poptCurrent()->GetFloatOption( "System", "PixelAspectRatio" ) [ 0 ];
	const TqInt* poptBucketSize = QGetRenderContext() ->poptCurrent()->GetIntegerOption( "limits", "bucketsize" );
	if ( poptBucketSize != 0 )
	{
		bX = poptBucketSize[ 0 ];
		bY = poptBucketSize[ 1 ];
	}
	const TqInt* poptGridSize = QGetRenderContext() ->poptCurrent()->GetIntegerOption( "limits", "gridsize" );
	if ( poptGridSize )
		gs = poptGridSize[ 0 ];
	else
		gs = 256;
	Aqsis::log() << info << "Image settings:" << std::endl;
	Aqsis::log() << info << "	Resolution: " << resX << " " << resY << std::endl;
	Aqsis::log() << info << "	PixelAspectRatio: " << pratio << std::endl;
	Aqsis::log() << info << "	Exposure:" << std::endl;
	Aqsis::log() << info << "		Gain: " << gain << std::endl;
	Aqsis::log() << info << "		Gamma: " << gamma << std::endl;
	Aqsis::log() << info << "Shading:" << std::endl;
	Aqsis::log() << info << "	Bucket size: " << bX << " " << bY << std::endl;
	Aqsis::log() << info << "	Gridsize: " << gs << std::endl;
	Aqsis::log() << info << "Anti-aliasing settings: " << std::endl;
	Aqsis::log() << info << "	PixelSamples: " << psX << " " << psY << std::endl;
	Aqsis::log() << info << "	PixelFilter: \"" << fName << "\" " << fX << " " << fY << std::endl;
}
//---------------------------------------------------------------------
} // namespace Aqsis
