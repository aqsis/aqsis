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

START_NAMESPACE( Aqsis )

#define STATS_INC( index )	CqStats::IncI( CqStats::##index )
#define STATS_DEC( index )	CqStats::DecI( CqStats::##index )
#define	STATS_GETI( index )	CqStats::getI( CqStats::##index )
#define	STATS_SETI( index , value )	CqStats::setI( CqStats::##index , value )
#define	STATS_GETF( index )	CqStats::getF( CqStats::##index )
#define	STATS_SETF( index , value )	CqStats::setF( CqStats::##index , value )
#define STATS_INT_GETI( index )	getI( index )
#define STATS_INT_GETF( index )	getF( index )

//----------------------------------------------------------------------
/** \enum EqState
 * Current process identifiers.
 */

enum EqState
{
    State_Parsing,    		///< Parsing a RIB file.
    State_Shadows,    		///< Processing shadows.
    State_Rendering,    	///< Rendering image.
    State_Complete,    		///< Rendering complete.
};


//----------------------------------------------------------------------
/** \class CqStatTimer
 * Class to handle timing of rendering processes as intervals.
 */
class CqStatTimer
{
	public:
		CqStatTimer() : m_timeStart( 0 ), m_timeTotal( 0 ), m_cStarted( 0 )
		{}
		~CqStatTimer()
		{}


		/** Get the total time that ths timer has recorded.
		 */
		clock_t	TimeTotal() const
		{
			return ( m_timeTotal );
		}

		/** Start the timer, asserts that the timer has not already been started, nesting is not allowed.
		 */
		void	Start()
		{
			if ( m_cStarted == 0 )
				m_timeStart = clock();
			m_cStarted++;
		}

		/** Stop the timer and update the total time for this timer. Asserts that the timer was actually running.
		 */
		void	Stop()
		{
			assert( m_cStarted > 0 );
			m_cStarted--;
			if ( m_cStarted == 0 )
				m_timeTotal += clock() - m_timeStart;
		}

		/** Reset the total time for this timer, asserts that the timer is not running.
		 */
		void	Reset()
		{
			m_timeTotal = 0;
			m_cStarted = 0;
		}

		/** Check if this timer is started.
		 */
		TqBool	isStarted()
		{
			return( m_cStarted > 0 );
		}

		/** Add two timers together
		 */
		CqStatTimer& operator+=( const CqStatTimer& from )
		{
			m_timeTotal += from.m_timeTotal;
			return(*this);
		}

	private:
		clock_t	m_timeStart;		///< Time at which the current interval started.
		clock_t	m_timeTotal;		///< Total time recorded by this timer.
		TqInt	m_cStarted;		///< Count of how many times the timer has been started.
}
;


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
		CqStats()
		{
			Initialise();
		}

		~CqStats()
		{ }

		void Initialise();
		void InitialiseFrame();

		/** Get the process identifier.
		 */
		EqState	State() const
		{
			return ( m_State );
		}
		/** Set the current process.
		 */
		void	SetState( const EqState State )
		{
			m_State = State;
		}

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

		static void IncI( const TqInt index )
		{
			m_intVars[ index ]++;
		}

		static void DecI( const TqInt index )
		{
			m_intVars[ index ]--;
		}

		static void setI( const TqInt index, const TqInt value )
		{
			m_intVars[ index ] = value;
		}

		static TqInt getI( const TqInt index )
		{
			return m_intVars[ index ];
		}

		static void setF( const TqInt index, const TqFloat value )
		{
			m_intVars[ index ] = value;
		}

		static TqFloat getF( const TqInt index )
		{
			return m_floatVars[ index ];
		}

		enum {	_First_float,
				_Last_float } EqFloatIndex;
		enum {	_First_int,

				// GPrim stats

					GPR_allocated,
					GPR_created,
					GPR_created_total,
					GPR_current,
					GPR_peak,
					GPR_culled,

					// GPrim types

					GPR_nurbs,
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

					//Pushes
					MPG_pushed_forward,
					MPG_pushed_down,
					MPG_pushed_far_down,

				// Shading stats

					// Per-shadeop stats
					
					SHD_so_abs,
					SHD_so_acos,
					SHD_so_ambient,
					SHD_so_area,
					SHD_so_asin,
					SHD_so_atan,
					SHD_so_atmosphere,
					SHD_so_attribute,
					SHD_so_bake,
					SHD_so_bump,
					SHD_so_cDeriv,
					SHD_so_cDu,
					SHD_so_cDv,
					SHD_so_calculatenormal,
					SHD_so_ccellnoise1,
					SHD_so_ccellnoise2,
					SHD_so_ccellnoise3,
					SHD_so_ccellnoise4,
					SHD_so_ceil,
					SHD_so_cenvironment2,
					SHD_so_cenvironment3,
					SHD_so_cclamp,
					SHD_so_clamp,
					SHD_so_cmax,
					SHD_so_cmin,
					SHD_so_cmix,
					SHD_so_cnoise1,
					SHD_so_cnoise2,
					SHD_so_cnoise3,
					SHD_so_cnoise4,
					SHD_so_concat,
					SHD_so_cos,
					SHD_so_cpnoise1,
					SHD_so_cpnoise2,
					SHD_so_cpnoise3,
					SHD_so_cpnoise4,
					SHD_so_crandom,
					SHD_so_cspline,
					SHD_so_csplinea,
					SHD_so_ctexture1,
					SHD_so_ctexture2,
					SHD_so_ctexture3,
					SHD_so_ctransform,
					SHD_so_degrees,
					SHD_so_depth,
					SHD_so_determinant,
					SHD_so_diffuse,
					SHD_so_displacement,
					SHD_so_distance,
					SHD_so_exp,
					SHD_so_external,
					SHD_so_fDeriv,
					SHD_so_fDu,
					SHD_so_fDv,
					SHD_so_faceforward,
					SHD_so_faceforward2,
					SHD_so_fcellnoise1,
					SHD_so_fcellnoise2,
					SHD_so_fcellnoise3,
					SHD_so_fcellnoise4,
					SHD_so_fenvironment2,
					SHD_so_fenvironment3,
					SHD_so_filterstep,
					SHD_so_filterstep2,
					SHD_so_floor,
					SHD_so_fmix,
					SHD_so_fnoise1,
					SHD_so_fnoise2,
					SHD_so_fnoise3,
					SHD_so_fnoise4,
					SHD_so_format,
					SHD_so_fpnoise1,
					SHD_so_fpnoise2,
					SHD_so_fpnoise3,
					SHD_so_fpnoise4,
					SHD_so_frandom,
					SHD_so_fresnel,
					SHD_so_fspline,
					SHD_so_fsplinea,
					SHD_so_ftexture1,
					SHD_so_ftexture2,
					SHD_so_ftexture3,
					SHD_so_illuminance,
					SHD_so_illuminate,
					SHD_so_incident,
					SHD_so_inversesqrt,
					SHD_so_length,
					SHD_so_lightsource,
					SHD_so_log,
					SHD_so_match,
					SHD_so_max,
					SHD_so_min,
					SHD_so_mod,
					SHD_so_mrotate,
					SHD_so_mscale,
					SHD_so_mtranslate,
					SHD_so_normalize,
					SHD_so_ntransform,
					SHD_so_occlusion,
					SHD_so_opposite,
					SHD_so_option,
					SHD_so_pDeriv,
					SHD_so_pDu,
					SHD_so_pDv,
					SHD_so_pcellnoise1,
					SHD_so_pcellnoise2,
					SHD_so_pcellnoise3,
					SHD_so_pcellnoise4,
					SHD_so_pclamp,
					SHD_so_phong,
					SHD_so_pmax,
					SHD_so_pmin,
					SHD_so_pmix,
					SHD_so_pnoise1,
					SHD_so_pnoise2,
					SHD_so_pnoise3,
					SHD_so_pnoise4,
					SHD_so_pow,
					SHD_so_ppnoise1,
					SHD_so_ppnoise2,
					SHD_so_ppnoise3,
					SHD_so_ppnoise4,
					SHD_so_prandom,
					SHD_so_printf,
					SHD_so_pspline,
					SHD_so_psplinea,
					SHD_so_ptlined,
					SHD_so_radians,
					SHD_so_reflect,
					SHD_so_refract,
					SHD_so_rendererinfo,
					SHD_so_rotate,
					SHD_so_round,
					SHD_so_scspline,
					SHD_so_scsplinea,
					SHD_so_setcomp,
					SHD_so_setmcomp,
					SHD_so_setxcomp,
					SHD_so_setycomp,
					SHD_so_setzcomp,
					SHD_so_sfspline,
					SHD_so_sfsplinea,
					SHD_so_shadername,
					SHD_so_shadername2,
					SHD_so_shadow,
					SHD_so_shadow1,
					SHD_so_sign,
					SHD_so_sin,
					SHD_so_smoothstep,
					SHD_so_solar,
					SHD_so_specular,
					SHD_so_specularbrdf,
					SHD_so_spspline,
					SHD_so_spsplinea,
					SHD_so_sqrt,
					SHD_so_step,
					SHD_so_surface,
					SHD_so_tan,
					SHD_so_textureinfo,
					SHD_so_trace,
					SHD_so_transform,
					SHD_so_vmix,
					SHD_so_vtransform,

				// Sampling stats

					SPL_count,
					SPL_bound_hits,
					SPL_hits,

				_Last_int } EqIntIndex;


		/// \name Increasing counters
		//@{


		/** Increase the shader variables allocated count by 1.
		 */
		void	IncVariablesAllocated()
		{
			m_cVariablesAllocated++;
			m_cVariablesCurrent++;
			m_cVariablesPeak = ( m_cVariablesCurrent > m_cVariablesPeak ) ? m_cVariablesCurrent : m_cVariablesPeak;
		}
		/** Increase the shader variables deallocated count by 1.
		 */
		void	IncVariablesDeallocated()
		{
			m_cVariablesDeallocated++;
			m_cVariablesCurrent--;
		}
		/** Increase the surface parameters allocated count by 1.
		 */
		void	IncParametersAllocated()
		{
			m_cParametersAllocated++;
			m_cParametersCurrent++;
			m_cParametersPeak = ( m_cParametersCurrent > m_cParametersPeak ) ? m_cParametersCurrent : m_cParametersPeak;
		}
		/** Increase the surface parameters deallocated count by 1.
		 */
		void	IncParametersDeallocated()
		{
			m_cParametersDeallocated++;
			m_cParametersCurrent--;
		}

		/** Increase the texture memory used.
		 */
		void	IncTextureMemory( TqInt n = 0 )
		{
			m_cTextureMemory += n;
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


		/** Start the current frame timer.
		 */
		void StartFrameTimer();

		/** Start the current frame timer.
		 */
		void StopFrameTimer();

		/** Get the surface timer.
		*/
		CqStatTimer& SurfaceTimer()
		{
			return ( m_timeSurface );
		};

		/** Get the displacement timer.
		*/
		CqStatTimer& DisplacementTimer()
		{
			return ( m_timeDisplacement );
		};
		/** Get the Imager timer.
		   	 */
		CqStatTimer& ImagerTimer()
		{
			return ( m_timeImager );
		};

		/** Get the atmosphere timer.
		*/
		CqStatTimer& AtmosphereTimer()
		{
			return ( m_timeAtmosphere );
		};

		/** Get the splits timer.
		*/
		CqStatTimer& SplitsTimer()
		{
			return ( m_timeSplits );
		};

		/** Get the dicing timer.
		*/
		CqStatTimer& DicingTimer()
		{
			return ( m_timeDicing );
		};
		/** Get the texturesampling timer.
		*/
		CqStatTimer& TextureMapTimer()
		{
			return ( m_timeTM );
		};
		/** Get the render MPGs timer.
		*/
		CqStatTimer& RenderMPGsTimer()
		{
			return ( m_timeRenderMPGs );
		};
		/** Get the occlusion culling timer.
		*/
		CqStatTimer& OcclusionCullTimer()
		{
			return ( m_timeOcclusionCull );
		};
		/** Get the diceable timer.
		*/
		CqStatTimer& DiceableTimer()
		{
			return ( m_timeDiceable );
		};
		/** Get the MakeTextureV, MakeShadowV, ... timers
		*/
		CqStatTimer& MakeTextureTimer()
		{
			return ( m_timeMakeTexture );
		};
		CqStatTimer& MakeShadowTimer()
		{
			return ( m_timeMakeShadow );
		};
		CqStatTimer& MakeEnvTimer()
		{
			return ( m_timeMakeEnv );
		};
		CqStatTimer& MakeFilterBucket()
		{
			return ( m_timeFB );
		};
		CqStatTimer& MakeDisplayBucket()
		{
			return ( m_timeDB );
		};
		CqStatTimer& MakeCombine()
		{
			return ( m_timeCombine );
		};
		CqStatTimer& MakeParse()
		{
			return ( m_timeParse );
		};
		CqStatTimer& MakeProject()
		{
			return ( m_timeProject );
		};
		CqStatTimer& Others()
		{
			return ( m_timeOthers );
		};

		TqInt GetCurrentParametersAllocated() const
		{
			return ( m_cParametersCurrent );
		}
		TqInt GetParametersAllocated() const
		{
			return ( m_cParametersAllocated );
		}
		TqInt GetParametersDeallocated() const
		{
			return ( m_cParametersDeallocated );
		}

		//@}

		void PrintStats( TqInt level ) const;
		void PrintInfo() const;

	private:
		std::ostream& TimeToString( std::ostream& os, TqFloat t, TqFloat tot ) const;

	private:
		EqState	m_State;						///< Current process identifier.
		TqFloat	m_Complete;						///< Current percentage complete.

		static TqFloat	 m_floatVars[ _Last_float ];		///< Float variables
		static TqInt		m_intVars[ _Last_int ];			///< Int variables

			TqInt	m_cVariablesAllocated;			///< Count of shader variables allocated.
		TqInt	m_cVariablesDeallocated;		///< Count of shader variables deallocated.
		TqInt	m_cVariablesCurrent;			///< Current count of variables allocated.
		TqInt	m_cVariablesPeak;				///< Peak count of variables allocated.
		TqInt	m_cParametersAllocated;			///< Count of surface parameters allocated.
		TqInt	m_cParametersDeallocated;		///< Count of surface parameters deallocated.
		TqInt	m_cParametersCurrent;			///< Current count of parameters allocated.
		TqInt	m_cParametersPeak;				///< Peak count of parameters allocated.


		TqInt m_cTextureMemory;     ///< Count of the memory used by texturemap.cpp
		TqInt m_cTextureHits[ 2 ][ 5 ];     ///< Count of the hits encountered used by texturemap.cpp
		TqInt m_cTextureMisses[ 5 ];     ///< Count of the hits encountered used by texturemap.cpp

		CqStatTimer	m_timeTotal;				///< Total time spent on the entire animation.
		CqStatTimer m_timeTotalFrame;			///< Time spent on processing one individual frame.

		CqStatTimer m_timeSurface;				///< Time spent on surface shading.
		CqStatTimer m_timeImager;			///< Time spent on imager shading.
		CqStatTimer m_timeDisplacement;			///< Time spent on displacement shading.
		CqStatTimer m_timeAtmosphere;			///< Time spent on volume shading (atmosphere).
		CqStatTimer m_timeSplits;				///< Time spent on surface splitting.
		CqStatTimer m_timeDicing;				///< Time spent on surface dicing.
		CqStatTimer m_timeRenderMPGs;			///< Time spent on rendering MPGs.
		CqStatTimer m_timeOcclusionCull;		///< Time spent on occlusion culling.
		CqStatTimer m_timeDiceable;				///< Time spent on diceable checking.
		CqStatTimer m_timeTM;				///< Time spent on SampleMipMap checking.
		CqStatTimer m_timeMakeTexture;			///< Time spent on MakeTextureV call.
		CqStatTimer m_timeMakeShadow;			///< Time spent on MakeShadowV call.
		CqStatTimer m_timeMakeEnv;		    	///< Time spent on MakeCubeEnvironmenV call.
		CqStatTimer m_timeFB;		    	    ///< Time spent on Filter the Bucket call.
		CqStatTimer m_timeDB;		    	    ///< Time spent on sending the Bucket information to the display.
		CqStatTimer m_timeCombine;		    ///< Time spent on combining the bucket subpixels
		CqStatTimer m_timeParse;		    ///< Time spent on sending the parsing RIB file or Ri Calls
		CqStatTimer m_timeProject;		    ///< Time spent on sending the Project grids to raster space
		CqStatTimer m_timeOthers;		    ///< Time spent on init. the buckets

}
;


//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

#endif	// !STATS_H_INCLUDED
