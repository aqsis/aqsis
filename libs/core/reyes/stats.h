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

//? Is .h included already?
#ifndef STATS_H_INCLUDED
#define STATS_H_INCLUDED 1

#include <aqsis/aqsis.h>

#include <time.h>
#include <iostream>

#include <aqsis/util/timer.h>
#include <aqsis/ri/ri.h>
#include <aqsis/util/enum.h>

namespace Aqsis {

extern void gStats_IncI( TqInt index );
extern void gStats_DecI( TqInt index );
extern TqInt gStats_getI( TqInt index );
extern void gStats_setI( TqInt index, TqInt value );
extern TqFloat gStats_getF( TqInt index );
extern void gStats_setF( TqInt index, TqFloat value );

#define STATS_INC( index )				gStats_IncI( CqStats::index )
#define STATS_DEC( index )				gStats_DecI( CqStats::index )
#define	STATS_GETI( index )				gStats_getI( CqStats::index )
#define	STATS_SETI( index , value )		gStats_setI( CqStats::index , value )
#define	STATS_GETF( index )				gStats_getF( CqStats::index )
#define	STATS_SETF( index , value )		gStats_setF( CqStats::index , value )


//----------------------------------------------------------------------
// Timer stuff.

#ifdef USE_TIMERS

/// Append time taken to the end of the current scope to the named timer.
#define AQSIS_TIME_SCOPE(id) CqScopeTimer aq_scope_timer__(\
		g_timerSet.getTimer(EqTimerStats::id))
/// Start the named timer.
#define AQSIS_TIMER_START(id) g_timerSet.getTimer(EqTimerStats::id).start()
/// Stop the named timer and append the time since the corresponding TIMER_START
#define AQSIS_TIMER_STOP(id) g_timerSet.getTimer(EqTimerStats::id).stop()

/// A class enum containing constants for each operation to be timed.
struct EqTimerStats
{
	enum Enum
	{
		// surface handling
		Post_surface,
		Dicable_check,
		Dicing,
		Splitting,
		// buckets
		Display_bucket,
		Prepare_bucket,
		// culling
		Backface_culling,
		Occlusion_culling_initialisation,
		Occlusion_culling_surfaces,
		Occlusion_culling,
		Transparency_culling_micropolygons,
		// grids
		Project_points,
		Bust_grids,
		// shading
		Atmosphere_shading,
		Displacement_shading,
		Imager_shading,
		Surface_shading,
		// texturing
		Make_texture,
		// sampling
		Combine_samples,
		Filter_samples,
		Render_MPGs,
		// high level
		Frame,
		Parse,

		// invalid
		LAST,
	};
	static const TqInt size = LAST;
};

AQSIS_ENUM_INFO_BEGIN(EqTimerStats::Enum, EqTimerStats::LAST)
	// surface handling
	"Post surface",
	"Dicable check",
	"Dicing",
	"Splitting",
	//buckets
	"Display bucket",
	"Prepare bucket",
	// culling
	"Backface culling",
	"Occlusion culling initialisation",
	"Occlusion culling surfaces",
	"Occlusion culling",
	"Transparency culling micropolygons",
	// grids
	"Project points",
	"Bust grids",
	// shading
	"Atmosphere shading",
	"Displacement shading",
	"Imager shading",
	"Surface shading",
	// texturing
	"Make texture",
	// sampling
	"Combine samples",
	"Filter samples",
	"Render MPGs",
	// high level
	"Frame",
	"Parse",
	// invalid
	"LAST"
AQSIS_ENUM_INFO_END

extern CqTimerSet<EqTimerStats> g_timerSet;

#else // USE_TIMERS

// dummy declarations if compiled without timers.
#define AQSIS_TIME_SCOPE(name)
#define AQSIS_TIMER_START(identifier)
#define AQSIS_TIMER_STOP(identifier)

#endif // USE_TIMERS

//----------------------------------------------------------------------
/** \class CqStats
   \brief Class containing statistics information.
 
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
		CqStats()
		{
			Initialise();
		}

		~CqStats()
		{ }

		void Initialise();
		void InitialiseFrame();

		/** Get the percentage complete.
		 */
		TqFloat	Complete() const
		{
			return ( m_Complete );
		}
		/** Set the percentage complete.
		 */
		void	SetComplete( TqFloat complete )
		{
			m_Complete = complete;
		}

		//! Increase an integer specified by an EqIntIndex value by one
		static void IncI( const TqInt index )
		{
			m_intVars[ index ]++;
		}

		//! Decrease an integer specified by an EqIntIndex value by one
		static void DecI( const TqInt index )
		{
			m_intVars[ index ]--;
		}

		//! Set an integer specified by an EqIntIndex value to value
		static void setI( const TqInt index, const TqInt value )
		{
			m_intVars[ index ] = value;
		}

		//! Get an integer specified by an EqIntIndex value
		static TqInt getI( const TqInt index )
		{
			return m_intVars[ index ];
		}

		//! Set a float specified by an EqfloatIndex value to value
		static void setF( const TqInt index, const TqFloat value )
		{
			m_floatVars[ index ] = value;
		}

		//! Get a float specified by an EqfloatIndex value
		static TqFloat getF( const TqInt index )
		{
			return m_floatVars[ index ];
		}

		/**
			\param	value	This has to be a 32-bit integer!
		 */
		/* Must be 32-bit integer!! */
		static unsigned int stats_log2( int value )
		{
			// As described in http://graphics.stanford.edu/~seander/bithacks.html#IntegerLog
			// This method should take roughly O(lg(N)) operations for a N-bit integer

			const unsigned int b[] =
			    {
			        0x2, 0xC, 0xF0, 0xFF00, 0xFFFF0000
			    };
			const unsigned int S[] =
			    {
			        1, 2, 4, 8, 16
			    };
			int i;

			register unsigned int c = 0; // result of log2(v) will go here
			for (i = 4; i >= 0; i--) // unroll for speed...
			{
				if (value & b[i])
				{
					value >>= S[i];
					c |= S[i];
				}
			}

			return c;
		}

		//! Enum to index the float array
		enum {	_First_float,

		       // MPG stats
		       // Size
		       MPG_average_area,
		       MPG_min_area,
		       MPG_max_area,

		       _Last_float } EqFloatIndex;

		//! Enum to index the integer array
		enum {	_First_int,

		       // GPrim stats

		       GPR_allocated,
		       GPR_created,
		       GPR_created_total,
		       GPR_current,
		       GPR_peak,
		       GPR_culled,
		       GPR_occlusion_culled,

		       // GPrim types

		       GPR_nurbs,
                       GPR_blobbies,
		       GPR_poly,
		       GPR_subdiv,
		       GPR_crv,
		       GPR_points,
		       GPR_quad,
		       GPR_patch,

		       // Geometry stats

		       // Curve stats

		       GEO_crv_splits,
		       GEO_crv_crv,
		       GEO_crv_patch,
		       GEO_crv_crv_created,
		       GEO_crv_patch_created,

		       // Procedural

		       GEO_prc_created,
		       GEO_prc_split,
		       GEO_prc_created_dl, // Dynamic load
		       GEO_prc_created_dra,
		       GEO_prc_created_prp,

		       // Grid stats

		       GRD_created,
		       GRD_culled,
		       GRD_current,
		       GRD_peak,
		       GRD_allocated,
		       GRD_deallocated,

		       //Unshaded grids
		       GRD_size_4,
		       GRD_size_8,
		       GRD_size_16,
		       GRD_size_32,
		       GRD_size_64,
		       GRD_size_128,
		       GRD_size_256,
		       GRD_size_g256,

		       //Shaded grids
		       GRD_shd_size_4,
		       GRD_shd_size_8,
		       GRD_shd_size_16,
		       GRD_shd_size_32,
		       GRD_shd_size_64,
		       GRD_shd_size_128,
		       GRD_shd_size_256,
		       GRD_shd_size_g256,

		       // MPG stats

		       //(De)Allocs
		       MPG_allocated,
		       MPG_deallocated,
		       MPG_current,
		       MPG_peak,
		       MPG_culled,
		       MPG_missed,
		       MPG_trimmed,
		       MPG_trimmedout,

		       // Sample hit quote
		       MPG_sample_coverage0_125,
		       MPG_sample_coverage125_25,
		       MPG_sample_coverage25_375,
		       MPG_sample_coverage375_50,
		       MPG_sample_coverage50_625,
		       MPG_sample_coverage625_75,
		       MPG_sample_coverage75_875,
		       MPG_sample_coverage875_100,

		       //Pushes
		       MPG_pushed_forward,
		       MPG_pushed_down,
		       MPG_pushed_far_down,

		       // Shading stats

		       // Sampling stats

		       SPL_count,
		       SPL_bound_hits,
		       SPL_hits,

		       // Parameters
		       PRM_created,
		       PRM_current,
		       PRM_peak,

		       _Last_int } EqIntIndex;


		/// \name Increasing counters
		//@{

		/** Increase the texture memory used.
		 */
		void	IncTextureMemory( TqInt n = 0 )
		{
			m_cTextureMemory += n;
			if (m_cTextureMemory < 0)
				m_cTextureMemory = 0;
		}
		void IncTextureHits( TqInt primary, TqInt which )
		{
			m_cTextureHits[ primary ][ which ] ++;
		}
		void IncTextureMisses( TqInt which )
		{
			m_cTextureMisses[ which ] ++;
		}

		/** Get the texture memory used.
		 */
		TqInt GetTextureMemory()
		{
			return m_cTextureMemory;
		}

		//@}

		void PrintStats( TqInt level ) const;
		void PrintInfo() const;

	private:
		std::ostream& TimeToString( std::ostream& os, TqFloat t, TqFloat tot ) const;

		TqFloat	m_Complete;						///< Current percentage complete.

		static TqFloat	 m_floatVars[ _Last_float ];		///< Float variables
		static TqInt		m_intVars[ _Last_int ];			///< Int variables

		TqInt m_cTextureMemory;     ///< Count of the memory used by texturemap.cpp
		TqInt m_cTextureHits[ 2 ][ 5 ];     ///< Count of the hits encountered used by texturemap.cpp
		TqInt m_cTextureMisses[ 5 ];     ///< Count of the hits encountered used by texturemap.cpp
};


//-----------------------------------------------------------------------

} // namespace Aqsis

#endif	// !STATS_H_INCLUDED
