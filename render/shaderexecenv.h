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
		\brief Declares classes and support structures for the shader execution environment.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is .h included already?
#ifndef SHADEREXECENV_H_INCLUDED
#define SHADEREXECENV_H_INCLUDED 1

#include	<vector>
#include	<stack>

#include	"aqsis.h"

#include	"bitvector.h"
#include	"color.h"
#include	"noise.h"
#include	"cellnoise.h"
#include	"vector3d.h"
#include	"vector4d.h"
#include	"ishaderdata.h"
#include	"matrix.h"

START_NAMESPACE( Aqsis )

/** \enum EqEnvVars
 * Identifiers for the standard environment variables.
 */

enum EqEnvVars
{
    EnvVars_Cs,  		///< Surface color.
    EnvVars_Os,  		///< Surface opacity.
    EnvVars_Ng,  		///< Geometric normal.
    EnvVars_du,  		///< First derivative in u.
    EnvVars_dv,  		///< First derivative in v.
    EnvVars_L,  		///< Incoming light direction.
    EnvVars_Cl,  		///< Light color.
    EnvVars_Ol,  		///< Light opacity.
    EnvVars_P,  		///< Point being shaded.
    EnvVars_dPdu,  	///< Change in P with respect to change in u.
    EnvVars_dPdv,  	///< Change in P with respect to change in v.
    EnvVars_N,  		///< Surface normal.
    EnvVars_u,  		///< Surface u coordinate.
    EnvVars_v,  		///< Surface v coordinate.
    EnvVars_s,  		///< Texture s coordinate.
    EnvVars_t,  		///< Texture t coordinate.
    EnvVars_I,  		///< Incident ray direction.
    EnvVars_Ci,  		///< Incident color.
    EnvVars_Oi,  		///< Incident opacity.
    EnvVars_Ps,  		///< Point being lit.
    EnvVars_E,  		///< Viewpoint position.
    EnvVars_ncomps,  	///< Number of color components.
    EnvVars_time,  	///< Frame time.
    EnvVars_alpha,  	///< Fractional pixel coverage.

    EnvVars_Last
};

extern TqInt gDefUses;
extern TqInt gDefLightUses;

#define	INIT_SO			TqBool __fVarying=TqFalse; /* A flag which will be set to indicate if the operation has any varying components. */ \
						TqInt __iGrid; /* Integer index used to track progress through the varying data */
#define	CHECKVARY(A)	__fVarying=(A)->Class()==class_varying||__fVarying;
#define	FOR_EACH		__iGrid = 0; \
						do \
						{ \
							if(!__fVarying || RunningState().Value( __iGrid ) ) \
							{
#define	END_FOR				} \
						}while( ( ++__iGrid < GridSize() ) && __fVarying);

#define	BEGIN_UNIFORM_SECTION	__iGrid = 0;
#define	END_UNIFORM_SECTION

#define	BEGIN_VARYING_SECTION	FOR_EACH
#define	END_VARYING_SECTION		END_FOR

#define	GETFLOAT(Val)		TqFloat _##Val; (Val)->GetFloat(_##Val,__iGrid)
#define	GETPOINT(Val)		CqVector3D _##Val; (Val)->GetPoint(_##Val,__iGrid)
#define	GETVECTOR(Val)		CqVector3D _##Val; (Val)->GetVector(_##Val,__iGrid)
#define	GETNORMAL(Val)		CqVector3D _##Val; (Val)->GetNormal(_##Val,__iGrid)
#define	GETCOLOR(Val)		CqColor _##Val; (Val)->GetColor(_##Val,__iGrid)
#define	GETSTRING(Val)		CqString _##Val; (Val)->GetString(_##Val,__iGrid)
#define	GETBOOLEAN(Val)		TqBool _##Val; (Val)->GetBool(_##Val,__iGrid)
#define	GETMATRIX(Val)		CqMatrix _##Val; (Val)->GetMatrix(_##Val,__iGrid)

#define	SETFLOAT(Val, v)	(Val)->SetFloat(v,__iGrid)
#define	SETPOINT(Val, v)	(Val)->SetPoint(v,__iGrid)
#define	SETVECTOR(Val, v)	(Val)->SetVector(v,__iGrid)
#define	SETNORMAL(Val, v)	(Val)->SetNormal(v,__iGrid)
#define	SETCOLOR(Val, v)	(Val)->SetColor(v,__iGrid)
#define	SETSTRING(Val, v)	(Val)->SetString(v,__iGrid)
#define	SETBOOLEAN(Val, v)	(Val)->SetBool(v,__iGrid)
#define	SETMATRIX(Val, v)	(Val)->SetMatrix(v,__iGrid)

#define	FLOAT(Val)			_##Val
#define	POINT(Val)			_##Val
#define	VECTOR(Val)			_##Val
#define	NORMAL(Val)			_##Val
#define	COLOR(Val)			_##Val
#define	STRING(Val)			_##Val
#define	BOOLEAN(Val)		_##Val
#define	MATRIX(Val)			_##Val


#define	DEFPARAM		IqShaderData* Result, CqShader* pShader=0
#define	DEFVOIDPARAM	CqShader* pShader=0
#define	DEFPARAMVAR		DEFPARAM, int cParams=0, IqShaderData** apParams=0
#define	DEFVOIDPARAMVAR	DEFVOIDPARAM, int cParams=0, IqShaderData** apParams=0

#define	DEFPARAMIMPL		IqShaderData* Result, CqShader* pShader
#define	DEFVOIDPARAMIMPL	CqShader* pShader
#define DEFPARAMVARIMPL		DEFPARAMIMPL, int cParams, IqShaderData** apParams
#define	DEFVOIDPARAMVARIMPL	DEFVOIDPARAMIMPL, int cParams, IqShaderData** apParams

#define	STD_SO		void
#define	STD_SOIMPL	void

#define	GET_FILTER_PARAMS	float _pswidth=1.0f,_ptwidth=1.0f; \
							GetFilterParams(cParams, apParams, _pswidth,_ptwidth);
#define	GET_TEXTURE_PARAMS	float _pswidth=1.0f,_ptwidth=1.0f,_psblur=0.0f,_ptblur=0.0f, _pfill=0.0f; \
							GetTexParams(cParams, apParams, _pswidth,_ptwidth,_psblur,_ptblur,_pfill);

#define	FLOATVAL	IqShaderData*
#define	POINTVAL	IqShaderData*
#define	VECTORVAL	IqShaderData*
#define	NORMALVAL	IqShaderData*
#define	COLORVAL	IqShaderData*
#define	STRINGVAL	IqShaderData*
#define	MATRIXVAL	IqShaderData*

#define	FLOATPTR	IqShaderData**
#define	POINTPTR	IqShaderData**
#define	VECTORPTR	IqShaderData**
#define	NORMALPTR	IqShaderData**
#define	COLORPTR	IqShaderData**
#define	STRINGPTR	IqShaderData**
#define	MATRIXPTR	IqShaderData**

#define	FLOATARRAYVAL	IqShaderData*
#define	POINTARRAYVAL	IqShaderData*
#define	VECTORARRAYVAL	IqShaderData*
#define	NORMALARRAYVAL	IqShaderData*
#define	COLORARRAYVAL	IqShaderData*
#define	STRINGARRAYVAL	IqShaderData*
#define	MATRIXARRAYVAL	IqShaderData*

class CqLightsource;
class CqShader;
class CqSurface;

//----------------------------------------------------------------------
/** \class CqShaderExecEnv
 * Standard shader execution environment. Contains standard variables, and provides SIMD functionality.
 */

class CqShaderExecEnv
{
	public:
		CqShaderExecEnv();
		virtual	~CqShaderExecEnv();

		/** Reset the internal SIMD counter.
		 */
		void	Reset()
		{
			m_GridI = 0;
		}
		/** Advance the internal SIMD counter.
		 * \return Boolean indicating SIMD completion.
		 */
		TqBool	Advance()
		{
			m_GridI++; return ( m_GridI < m_GridSize );
		}
		void	Initialise( const TqInt uGridRes, const TqInt vGridRes, CqSurface* pSurface, TqInt Uses );
		/** Get internal SIMD index.
		 */
		TqInt&	GridI()
		{
			return ( m_GridI );
		}
		/** Get grid size in u
		 */
		TqInt	uGridRes()
		{
			return ( m_uGridRes );
		}
		/** Get grid size in v
		 */
		TqInt	vGridRes()
		{
			return ( m_vGridRes );
		}
		/** Get total grid size.
		 */
		TqInt	GridSize()
		{
			return ( m_GridSize );
		}
		/** Get a pointer to the associated surface.
		 */
		CqSurface*	pSurface() const
		{
			return ( m_pSurface );
		}
		const CqMatrix&	matObjectToWorld() const;

		void	ValidateIlluminanceCache( IqShaderData* pP, CqShader* pShader );
		/** Reset the illuminance cache.
		 */
		void	InvalidateIlluminanceCache()
		{
			m_IlluminanceCacheValid = TqFalse;
		}
		/** Get the current execution state. Bits in the vector indicate which SIMD indexes have passed the current condition.
		 */
		CqBitVector& CurrentState()
		{
			return ( m_CurrentState );
		}
		/** Get the running execution state. Bits in the vector indicate which SIMD indexes are valid.
		 */
		CqBitVector& RunningState()
		{
			return ( m_RunningState );
		}
		/** Transfer the current state into the running state.
		 */
		void	GetCurrentState()
		{
			m_RunningState = m_CurrentState;
		}
		/** Clear the current state ready for a new condition.
		 */
		void	ClearCurrentState()
		{
			m_CurrentState.SetAll( TqFalse );
		}
		/** Push the running state onto the stack.
		 */
		void	PushState()
		{
			m_stkState.push( m_RunningState );
		}
		/** Pop the running state from the stack.
		 */
		void	PopState()
		{
			m_RunningState = m_stkState.top(); m_stkState.pop();
		}
		/** Invert the bits in the running state, to perform the opposite to the condition, i.e. else.
		 */
		void	InvertRunningState()
		{
			m_RunningState.Complement();
			if ( !m_stkState.empty() )
				m_RunningState.Intersect( m_stkState.top() );
		}
		/** Find a named standard variable in the list.
		 * \param pname Character pointer to the name.
		 * \return IqShaderData pointer or 0.
		 */
		IqShaderData* FindStandardVar( char* pname )
		{
			TqInt i;
			for ( i = 0; i < EnvVars_Last; i++ )
			{
				if ( strcmp( m_apVariableNames[ i ], pname ) == 0 )
					return ( m_apVariables[ i ] );
			}
			return ( 0 );
		}
		/** Find a named standard variable in the list.
		 * \param pname Character pointer to the name.
		 * \return Integer index in the list or -1.
		 */
		TqInt	FindStandardVarIndex( char* pname )
		{
			TqInt i;
			for ( i = 0; i < EnvVars_Last; i++ )
			{
				if ( strcmp( m_apVariableNames[ i ], pname ) == 0 )
					return ( i );
			}
			return ( -1 );
		}

		/** Get a standard variable pointer given an index.
		 * \param Index The integer index returned from FindStandardVarIndex.
		 * \return IqShaderData pointer.
		 */
		IqShaderData*	pVar( TqInt Index )
		{
			return ( m_apVariables[ Index ] );
		}
		/** Delete an indexed variable from the list.
		 * \param Index The integer index returned from FindStandardVarIndex.
		 */
		void	DeleteVariable( TqInt Index )
		{
			delete( m_apVariables[ Index ] );
			m_apVariables[ Index ] = 0;
		}

		/** Get a reference to the Cq standard variable.
		 */
		IqShaderData* Cs()
		{
			return ( m_apVariables[ EnvVars_Cs ] );
		}
		/** Get a reference to the Os standard variable.
		 */
		IqShaderData* 	Os()
		{
			return ( m_apVariables[ EnvVars_Os ] );
		}
		/** Get a reference to the Ng standard variable.
		 */
		IqShaderData* Ng()
		{
			return ( m_apVariables[ EnvVars_Ng ] );
		}
		/** Get a reference to the du standard variable.
		 */
		IqShaderData* du()
		{
			return ( m_apVariables[ EnvVars_du ] );
		}
		/** Get a reference to the dv standard variable.
		 */
		IqShaderData* dv()
		{
			return ( m_apVariables[ EnvVars_dv ] );
		}
		/** Get a reference to the L standard variable.
		 */
		IqShaderData* L()
		{
			return ( m_apVariables[ EnvVars_L ] );
		}
		/** Get a reference to the Cl standard variable.
		 */
		IqShaderData* Cl()
		{
			return ( m_apVariables[ EnvVars_Cl ] );
		}
		/** Get a reference to the Ol standard variable.
		 */
		IqShaderData* Ol()
		{
			return ( m_apVariables[ EnvVars_Ol ] );
		}
		/** Get a reference to the P standard variable.
		 */
		IqShaderData* P()
		{
			return ( m_apVariables[ EnvVars_P ] );
		}
		/** Get a reference to the dPdu standard variable.
		 */
		IqShaderData* dPdu()
		{
			return ( m_apVariables[ EnvVars_dPdu ] );
		}
		/** Get a reference to the dPdv standard variable.
		 */
		IqShaderData* dPdv()
		{
			return ( m_apVariables[ EnvVars_dPdv ] );
		}
		/** Get a reference to the N standard variable.
		 */
		IqShaderData* N()
		{
			return ( m_apVariables[ EnvVars_N ] );
		}
		/** Get a reference to the u standard variable.
		 */
		IqShaderData* u()
		{
			return ( m_apVariables[ EnvVars_u ] );
		}
		/** Get a reference to the v standard variable.
		 */
		IqShaderData* v()
		{
			return ( m_apVariables[ EnvVars_v ] );
		}
		/** Get a reference to the s standard variable.
		 */
		IqShaderData* s()
		{
			return ( m_apVariables[ EnvVars_s ] );
		}
		/** Get a reference to the t standard variable.
		 */
		IqShaderData* t()
		{
			return ( m_apVariables[ EnvVars_t ] );
		}
		/** Get a reference to the I standard variable.
		 */
		IqShaderData* I()
		{
			return ( m_apVariables[ EnvVars_I ] );
		}
		/** Get a reference to the Ci standard variable.
		 */
		IqShaderData* Ci()
		{
			return ( m_apVariables[ EnvVars_Ci ] );
		}
		/** Get a reference to the Oi standard variable.
		 */
		IqShaderData* Oi()
		{
			return ( m_apVariables[ EnvVars_Oi ] );
		}
		/** Get a reference to the Ps standard variable.
		 */
		IqShaderData* Ps()
		{
			return ( m_apVariables[ EnvVars_Ps ] );
		}
		/** Get a reference to the E standard variable.
		 */
		IqShaderData* E()
		{
			return ( m_apVariables[ EnvVars_E ] );
		}
		/** Get a reference to the ncomps standard variable.
		 */
		IqShaderData* ncomps()
		{
			return ( m_apVariables[ EnvVars_ncomps ] );
		}
		/** Get a reference to the time standard variable.
		 */
		IqShaderData* time()
		{
			return ( m_apVariables[ EnvVars_time ] );
		}
		/** Get a reference to the alpha standard variable.
		 */
		IqShaderData* alpha()
		{
			return ( m_apVariables[ EnvVars_alpha ] );
		}

	private:
		/** Internal function to extract additional named filter parameters from an array of stack entries.
		 */
		void	GetFilterParams( int cParams, IqShaderData** apParams, float& _pswidth, float& _ptwidth )
		{
			CqString strParam;
			TqFloat f;

			int i = 0;
			while ( cParams > 0 )
			{
				apParams[ i ] ->GetString( strParam, 0 );
				apParams[ i + 1 ] ->GetFloat( f, 0 );

				if ( strParam.compare( "width" ) == 0 ) _pswidth = _ptwidth = f;
				else if ( strParam.compare( "swidth" ) == 0 ) _pswidth = f;
				else if ( strParam.compare( "twidth" ) == 0 ) _ptwidth = f;
				i += 2;
				cParams -= 2;
			}
		}
		/** Internal function to extract additional named texture control parameters from an array of stack entries.
		 */
		void	GetTexParams( int cParams, IqShaderData** apParams, float& _pswidth, float& _ptwidth, float& _psblur, float& _ptblur, float& _pfill )
		{
			CqString strParam;
			TqFloat f;

			int i = 0;
			while ( cParams > 0 )
			{
				apParams[ i ] ->GetString( strParam, 0 );
				apParams[ i + 1 ] ->GetFloat( f, 0 );
				if ( strParam.compare( "width" ) == 0 ) _pswidth = _ptwidth = f;
				else if ( strParam.compare( "swidth" ) == 0 ) _pswidth = f;
				else if ( strParam.compare( "twidth" ) == 0 ) _ptwidth = f;
				else if ( strParam.compare( "blur" ) == 0 ) _psblur = _ptblur = f;
				else if ( strParam.compare( "sblur" ) == 0 ) _psblur = f;
				else if ( strParam.compare( "tblur" ) == 0 ) _ptblur = f;
				else if ( strParam.compare( "fill" ) == 0 ) _pfill = f;
				i += 2;
				cParams -= 2;
			}
		}

		std::vector<IqShaderData*>	m_apVariables;	///< Vector of pointers to shader variables.
		struct SqVarName
		{
			char*	m_strName;
			EqEnvVars	m_Index;
		};
		static	CqNoise	m_noise;		///< One off noise generator, used by all envs.
		static	CqCellNoise	m_cellnoise;	///< One off cell noise generator, used by all envs.
		static	CqRandom	m_random;		///< One off random number generator used by all envs.
		static	CqMatrix	m_matIdentity;
		static	char*	m_apVariableNames[];	///< Vector of variable names.

		TqInt	m_uGridRes;				///< The resolution of the grid in u.
		TqInt	m_vGridRes;				///< The resolution of the grid in u.
		TqInt	m_GridSize;				///< The resolution of the grid.
		TqInt	m_GridI;				///< The current SIMD index.
		TqInt	m_li;					///< Light index, used during illuminance loop.
		TqInt	m_Illuminate;
		TqBool	m_IlluminanceCacheValid;	///< Flag indicating whether the illuminance cache is valid.
		CqSurface*	m_pSurface;				///< Pointer to the associated surface.
		CqBitVector	m_CurrentState;			///< SIMD execution state bit vector accumulator.
		CqBitVector	m_RunningState;			///< SIMD running execution state bit vector.
		std::stack<CqBitVector>	m_stkState;				///< Stack of execution state bit vectors.
		TqInt	m_LocalIndex;			///< Local cached variable index to speed repeated access to the same local variable.

	public:
		TqInt	m_vfCulled;	///< Shader variable indicating whether the individual micropolys are culled.

		TqBool	SO_init_illuminance();
		TqBool	SO_advance_illuminance();

		// ShadeOps
		STD_SO	SO_radians( FLOATVAL degrees, DEFPARAM );
		STD_SO	SO_degrees( FLOATVAL radians, DEFPARAM );
		STD_SO	SO_sin( FLOATVAL a, DEFPARAM );
		STD_SO	SO_asin( FLOATVAL a, DEFPARAM );
		STD_SO	SO_cos( FLOATVAL a, DEFPARAM );
		STD_SO	SO_acos( FLOATVAL a, DEFPARAM );
		STD_SO	SO_tan( FLOATVAL a, DEFPARAM );
		STD_SO	SO_atan( FLOATVAL yoverx, DEFPARAM );
		STD_SO	SO_atan( FLOATVAL y, FLOATVAL x, DEFPARAM );
		STD_SO	SO_pow( FLOATVAL x, FLOATVAL y, DEFPARAM );
		STD_SO	SO_exp( FLOATVAL x, DEFPARAM );
		STD_SO	SO_sqrt( FLOATVAL x, DEFPARAM );
		STD_SO	SO_log( FLOATVAL x, DEFPARAM );
		STD_SO	SO_log( FLOATVAL x, FLOATVAL base, DEFPARAM );
		STD_SO	SO_mod( FLOATVAL a, FLOATVAL b, DEFPARAM );
		STD_SO	SO_abs( FLOATVAL x, DEFPARAM );
		STD_SO	SO_sign( FLOATVAL x, DEFPARAM );
		STD_SO	SO_min( FLOATVAL a, FLOATVAL b, DEFPARAMVAR );
		STD_SO	SO_max( FLOATVAL a, FLOATVAL b, DEFPARAMVAR );
		STD_SO	SO_pmin( POINTVAL a, POINTVAL b, DEFPARAMVAR );
		STD_SO	SO_pmax( POINTVAL a, POINTVAL b, DEFPARAMVAR );
		STD_SO	SO_cmin( COLORVAL a, COLORVAL b, DEFPARAMVAR );
		STD_SO	SO_cmax( COLORVAL a, COLORVAL b, DEFPARAMVAR );
		STD_SO	SO_clamp( FLOATVAL a, FLOATVAL min, FLOATVAL max, DEFPARAM );
		STD_SO	SO_pclamp( POINTVAL a, POINTVAL min, POINTVAL max, DEFPARAM );
		STD_SO	SO_cclamp( COLORVAL a, COLORVAL min, COLORVAL max, DEFPARAM );
		STD_SO	SO_floor( FLOATVAL x, DEFPARAM );
		STD_SO	SO_ceil( FLOATVAL x, DEFPARAM );
		STD_SO	SO_round( FLOATVAL x, DEFPARAM );
		STD_SO	SO_step( FLOATVAL min, FLOATVAL value, DEFPARAM );
		STD_SO	SO_smoothstep( FLOATVAL min, FLOATVAL max, FLOATVAL value, DEFPARAM );
		STD_SO	SO_fspline( FLOATVAL value, DEFPARAMVAR );
		STD_SO	SO_cspline( FLOATVAL value, DEFPARAMVAR );
		STD_SO	SO_pspline( FLOATVAL value, DEFPARAMVAR );
		STD_SO	SO_sfspline( STRINGVAL basis, FLOATVAL value, DEFPARAMVAR );
		STD_SO	SO_scspline( STRINGVAL basis, FLOATVAL value, DEFPARAMVAR );
		STD_SO	SO_spspline( STRINGVAL basis, FLOATVAL value, DEFPARAMVAR );
		STD_SO	SO_fDu( FLOATVAL p, DEFPARAM );
		STD_SO	SO_fDv( FLOATVAL p, DEFPARAM );
		STD_SO	SO_fDeriv( FLOATVAL p, FLOATVAL den, DEFPARAM );
		STD_SO	SO_cDu( COLORVAL p, DEFPARAM );
		STD_SO	SO_cDv( COLORVAL p, DEFPARAM );
		STD_SO	SO_cDeriv( COLORVAL p, FLOATVAL den, DEFPARAM );
		STD_SO	SO_pDu( POINTVAL p, DEFPARAM );
		STD_SO	SO_pDv( POINTVAL p, DEFPARAM );
		STD_SO	SO_pDeriv( POINTVAL p, FLOATVAL den, DEFPARAM );
		STD_SO	SO_frandom( DEFPARAM );
		STD_SO	SO_crandom( DEFPARAM );
		STD_SO	SO_prandom( DEFPARAM );
		STD_SO	SO_fnoise1( FLOATVAL v, DEFPARAM );
		STD_SO	SO_fnoise2( FLOATVAL u, FLOATVAL v, DEFPARAM );
		STD_SO	SO_fnoise3( POINTVAL p, DEFPARAM );
		STD_SO	SO_fnoise4( POINTVAL p, FLOATVAL t, DEFPARAM );
		STD_SO	SO_cnoise1( FLOATVAL v, DEFPARAM );
		STD_SO	SO_cnoise2( FLOATVAL u, FLOATVAL v, DEFPARAM );
		STD_SO	SO_cnoise3( POINTVAL p, DEFPARAM );
		STD_SO	SO_cnoise4( POINTVAL p, FLOATVAL t, DEFPARAM );
		STD_SO	SO_pnoise1( FLOATVAL v, DEFPARAM );
		STD_SO	SO_pnoise2( FLOATVAL u, FLOATVAL v, DEFPARAM );
		STD_SO	SO_pnoise3( POINTVAL p, DEFPARAM );
		STD_SO	SO_pnoise4( POINTVAL p, FLOATVAL t, DEFPARAM );
		STD_SO	SO_setcomp( COLORVAL p, FLOATVAL i, FLOATVAL v, DEFVOIDPARAM );
		STD_SO	SO_setxcomp( POINTVAL p, FLOATVAL v, DEFVOIDPARAM );
		STD_SO	SO_setycomp( POINTVAL p, FLOATVAL v, DEFVOIDPARAM );
		STD_SO	SO_setzcomp( POINTVAL p, FLOATVAL v, DEFVOIDPARAM );
		STD_SO	SO_length( VECTORVAL V, DEFPARAM );
		STD_SO	SO_distance( POINTVAL P1, POINTVAL P2, DEFPARAM );
		STD_SO	SO_area( POINTVAL p, DEFPARAM );
		STD_SO	SO_normalize( VECTORVAL V, DEFPARAM );
		STD_SO	SO_faceforward( NORMALVAL N, VECTORVAL I /* [Nref] */, DEFPARAM );
		STD_SO	SO_reflect( VECTORVAL I, NORMALVAL N, DEFPARAM );
		STD_SO	SO_refract( VECTORVAL I, NORMALVAL N, FLOATVAL eta, DEFPARAM );
		STD_SO	SO_fresnel( VECTORVAL I, NORMALVAL N, FLOATVAL eta, FLOATVAL Kr, FLOATVAL Kt, DEFVOIDPARAM );
		STD_SO	SO_fresnel( VECTORVAL I, NORMALVAL N, FLOATVAL eta, FLOATVAL Kr, FLOATVAL Kt, VECTORVAL R, VECTORVAL T, DEFVOIDPARAM );
		STD_SO	SO_transform( STRINGVAL fromspace, STRINGVAL tospace, POINTVAL p, DEFPARAM );
		STD_SO	SO_transform( STRINGVAL tospace, POINTVAL p, DEFPARAM );
		STD_SO	SO_transformm( MATRIXVAL tospace, POINTVAL p, DEFPARAM );
		STD_SO	SO_vtransform( STRINGVAL fromspace, STRINGVAL tospace, VECTORVAL p, DEFPARAM );
		STD_SO	SO_vtransform( STRINGVAL tospace, VECTORVAL p, DEFPARAM );
		STD_SO	SO_vtransformm( MATRIXVAL tospace, VECTORVAL p, DEFPARAM );
		STD_SO	SO_ntransform( STRINGVAL fromspace, STRINGVAL tospace, NORMALVAL p, DEFPARAM );
		STD_SO	SO_ntransform( STRINGVAL tospace, NORMALVAL p, DEFPARAM );
		STD_SO	SO_ntransformm( MATRIXVAL tospace, NORMALVAL p, DEFPARAM );
		STD_SO	SO_depth( POINTVAL p, DEFPARAM );
		STD_SO	SO_calculatenormal( POINTVAL p, DEFPARAM );
		STD_SO	SO_cmix( COLORVAL color0, COLORVAL color1, FLOATVAL value, DEFPARAM );
		STD_SO	SO_fmix( FLOATVAL f0, FLOATVAL f1, FLOATVAL value, DEFPARAM );
		STD_SO	SO_pmix( POINTVAL p0, POINTVAL p1, FLOATVAL value, DEFPARAM );
		STD_SO	SO_vmix( VECTORVAL v0, VECTORVAL v1, FLOATVAL value, DEFPARAM );
		STD_SO	SO_nmix( NORMALVAL n0, NORMALVAL n1, FLOATVAL value, DEFPARAM );
		STD_SO	SO_ambient( DEFPARAM );
		STD_SO	SO_diffuse( NORMALVAL N, DEFPARAM );
		STD_SO	SO_specular( NORMALVAL N, VECTORVAL V, FLOATVAL roughness, DEFPARAM );
		STD_SO	SO_phong( NORMALVAL N, VECTORVAL V, FLOATVAL size, DEFPARAM );
		STD_SO	SO_trace( POINTVAL P, VECTORVAL R, DEFPARAM );
		STD_SO	SO_ftexture1( STRINGVAL name, FLOATVAL channel, DEFPARAMVAR );
		STD_SO	SO_ftexture2( STRINGVAL name, FLOATVAL channel, FLOATVAL s, FLOATVAL t, DEFPARAMVAR );
		STD_SO	SO_ftexture3( STRINGVAL name, FLOATVAL channel, FLOATVAL s1, FLOATVAL t1, FLOATVAL s2, FLOATVAL t2, FLOATVAL s3, FLOATVAL t3, FLOATVAL s4, FLOATVAL t4, DEFPARAMVAR );
		STD_SO	SO_ctexture1( STRINGVAL name, FLOATVAL channel, DEFPARAMVAR );
		STD_SO	SO_ctexture2( STRINGVAL name, FLOATVAL channel, FLOATVAL s, FLOATVAL t, DEFPARAMVAR );
		STD_SO	SO_ctexture3( STRINGVAL name, FLOATVAL channel, FLOATVAL s1, FLOATVAL t1, FLOATVAL s2, FLOATVAL t2, FLOATVAL s3, FLOATVAL t3, FLOATVAL s4, FLOATVAL t4, DEFPARAMVAR );
		STD_SO	SO_fenvironment2( STRINGVAL name, FLOATVAL channel, VECTORVAL R, DEFPARAMVAR );
		STD_SO	SO_fenvironment3( STRINGVAL name, FLOATVAL channel, VECTORVAL R1, VECTORVAL R2, VECTORVAL R3, VECTORVAL R4, DEFPARAMVAR );
		STD_SO	SO_cenvironment2( STRINGVAL name, FLOATVAL channel, VECTORVAL R, DEFPARAMVAR );
		STD_SO	SO_cenvironment3( STRINGVAL name, FLOATVAL channel, VECTORVAL R1, VECTORVAL R2, VECTORVAL R3, VECTORVAL R4, DEFPARAMVAR );
		STD_SO	SO_bump1( STRINGVAL name, FLOATVAL channel, DEFPARAMVAR );
		STD_SO	SO_bump2( STRINGVAL name, FLOATVAL channel, FLOATVAL s, FLOATVAL t, DEFPARAMVAR );
		STD_SO	SO_bump3( STRINGVAL name, FLOATVAL channel, FLOATVAL s1, FLOATVAL t1, FLOATVAL s2, FLOATVAL t2, FLOATVAL s3, FLOATVAL t3, FLOATVAL s4, FLOATVAL t4, DEFPARAMVAR );
		STD_SO	SO_shadow( STRINGVAL name, FLOATVAL channel, POINTVAL P, DEFPARAMVAR );
		STD_SO	SO_shadow1( STRINGVAL name, FLOATVAL channel, POINTVAL P1, POINTVAL P2, POINTVAL P3, POINTVAL P4, DEFPARAMVAR );
		STD_SO	SO_illuminance( POINTVAL P, FLOATVAL nsamples, DEFVOIDPARAM );
		STD_SO	SO_illuminance( POINTVAL P, VECTORVAL Axis, FLOATVAL Angle, FLOATVAL nsamples, DEFVOIDPARAM );
		STD_SO	SO_illuminate( POINTVAL P, VECTORVAL Axis, FLOATVAL Angle, DEFVOIDPARAM );
		STD_SO	SO_illuminate( POINTVAL P, DEFVOIDPARAM );
		STD_SO	SO_solar( VECTORVAL Axis, FLOATVAL Angle, DEFVOIDPARAM );
		STD_SO	SO_solar( DEFVOIDPARAM );
		STD_SO	SO_printf( STRINGVAL str, DEFVOIDPARAMVAR );
		STD_SO	SO_format( STRINGVAL str, DEFPARAMVAR );
		STD_SO	SO_concat( STRINGVAL stra, STRINGVAL strb, DEFPARAMVAR );
		STD_SO	SO_atmosphere( STRINGVAL name, IqShaderData* pV, DEFPARAM );
		STD_SO	SO_displacement( STRINGVAL name, IqShaderData* pV, DEFPARAM );
		STD_SO	SO_lightsource( STRINGVAL name, IqShaderData* pV, DEFPARAM );
		STD_SO	SO_surface( STRINGVAL name, IqShaderData* pV, DEFPARAM );
		STD_SO	SO_attribute( STRINGVAL name, IqShaderData* pV, DEFPARAM );
		STD_SO	SO_option( STRINGVAL name, IqShaderData* pV, DEFPARAM );
		STD_SO	SO_rendererinfo( STRINGVAL name, IqShaderData* pV, DEFPARAM );
		STD_SO	SO_incident( STRINGVAL name, IqShaderData* pV, DEFPARAM );
		STD_SO	SO_opposite( STRINGVAL name, IqShaderData* pV, DEFPARAM );
		STD_SO	SO_fcellnoise1( FLOATVAL v, DEFPARAM );
		STD_SO	SO_fcellnoise2( FLOATVAL u, FLOATVAL v, DEFPARAM );
		STD_SO	SO_fcellnoise3( POINTVAL p, DEFPARAM );
		STD_SO	SO_fcellnoise4( POINTVAL p, FLOATVAL v, DEFPARAM );
		STD_SO	SO_ccellnoise1( FLOATVAL v, DEFPARAM );
		STD_SO	SO_ccellnoise2( FLOATVAL u, FLOATVAL v, DEFPARAM );
		STD_SO	SO_ccellnoise3( POINTVAL p, DEFPARAM );
		STD_SO	SO_ccellnoise4( POINTVAL p, FLOATVAL v, DEFPARAM );
		STD_SO	SO_pcellnoise1( FLOATVAL v, DEFPARAM );
		STD_SO	SO_pcellnoise2( FLOATVAL u, FLOATVAL v, DEFPARAM );
		STD_SO	SO_pcellnoise3( POINTVAL p, DEFPARAM );
		STD_SO	SO_pcellnoise4( POINTVAL p, FLOATVAL v, DEFPARAM );
		STD_SO	SO_fpnoise1( FLOATVAL v, FLOATVAL period, DEFPARAM );
		STD_SO	SO_fpnoise2( FLOATVAL u, FLOATVAL v, FLOATVAL uperiod, FLOATVAL vperiod, DEFPARAM );
		STD_SO	SO_fpnoise3( POINTVAL p, POINTVAL pperiod, DEFPARAM );
		STD_SO	SO_fpnoise4( POINTVAL p, FLOATVAL t, POINTVAL pperiod, FLOATVAL tperiod, DEFPARAM );
		STD_SO	SO_cpnoise1( FLOATVAL v, FLOATVAL period, DEFPARAM );
		STD_SO	SO_cpnoise2( FLOATVAL u, FLOATVAL v, FLOATVAL uperiod, FLOATVAL vperiod, DEFPARAM );
		STD_SO	SO_cpnoise3( POINTVAL p, POINTVAL pperiod, DEFPARAM );
		STD_SO	SO_cpnoise4( POINTVAL p, FLOATVAL t, POINTVAL pperiod, FLOATVAL tperiod, DEFPARAM );
		STD_SO	SO_ppnoise1( FLOATVAL v, FLOATVAL period, DEFPARAM );
		STD_SO	SO_ppnoise2( FLOATVAL u, FLOATVAL v, FLOATVAL uperiod, FLOATVAL vperiod, DEFPARAM );
		STD_SO	SO_ppnoise3( POINTVAL p, POINTVAL pperiod, DEFPARAM );
		STD_SO	SO_ppnoise4( POINTVAL p, FLOATVAL t, POINTVAL pperiod, FLOATVAL tperiod, DEFPARAM );
		STD_SO	SO_ctransform( STRINGVAL fromspace, STRINGVAL tospace, COLORVAL c, DEFPARAM );
		STD_SO	SO_ctransform( STRINGVAL tospace, COLORVAL c, DEFPARAM );
		STD_SO	SO_ptlined( POINTVAL P0, POINTVAL P1, POINTVAL Q, DEFPARAM );
		STD_SO	SO_inversesqrt( FLOATVAL x, DEFPARAM );
		STD_SO	SO_match( STRINGVAL a, STRINGVAL b, DEFPARAM );
		STD_SO	SO_rotate( VECTORVAL Q, FLOATVAL angle, POINTVAL P0, POINTVAL P1, DEFPARAM );
		STD_SO	SO_filterstep( FLOATVAL edge, FLOATVAL s1, DEFPARAMVAR );
		STD_SO	SO_filterstep2( FLOATVAL edge, FLOATVAL s1, FLOATVAL s2, DEFPARAMVAR );
		STD_SO	SO_specularbrdf( VECTORVAL L, NORMALVAL N, VECTORVAL V, FLOATVAL rough, DEFPARAM );
		STD_SO	SO_setmcomp( MATRIXVAL M, FLOATVAL row, FLOATVAL column, FLOATVAL val, DEFVOIDPARAM );
		STD_SO	SO_determinant( MATRIXVAL M, DEFPARAM );
		STD_SO	SO_mtranslate( MATRIXVAL M, VECTORVAL V, DEFPARAM );
		STD_SO	SO_mrotate( MATRIXVAL M, FLOATVAL angle, VECTORVAL axis, DEFPARAM );
		STD_SO	SO_mscale( MATRIXVAL M, POINTVAL s, DEFPARAM );
		STD_SO	SO_fsplinea( FLOATVAL value, FLOATARRAYVAL a, DEFPARAM );
		STD_SO	SO_csplinea( FLOATVAL value, COLORARRAYVAL a, DEFPARAM );
		STD_SO	SO_psplinea( FLOATVAL value, POINTARRAYVAL a, DEFPARAM );
		STD_SO	SO_sfsplinea( STRINGVAL basis, FLOATVAL value, FLOATARRAYVAL a, DEFPARAM );
		STD_SO	SO_scsplinea( STRINGVAL basis, FLOATVAL value, COLORARRAYVAL a, DEFPARAM );
		STD_SO	SO_spsplinea( STRINGVAL basis, FLOATVAL value, POINTARRAYVAL a, DEFPARAM );
		STD_SO	SO_shadername( DEFPARAM );
		STD_SO	SO_shadername2( STRINGVAL shader, DEFPARAM );
		STD_SO	SO_textureinfo( STRINGVAL shader, STRINGVAL dataname, IqShaderData* pV, DEFPARAM );
};


/** Templatised derivative function. Calculates the derivative of the provided stack entry with respect to u.
 */
template <class R>
R SO_DuType( IqShaderData* Var, TqInt i, CqShaderExecEnv& s )
{
	R Ret;
	TqInt uRes = s.uGridRes();
	TqInt GridX = i % ( uRes + 1 );
	
	TqFloat fdu;
	s.du()->GetFloat( fdu );
	
	R v1,v2;
	if ( GridX < uRes )
	{
		Var->GetValue( v1, i + 1 );
		Var->GetValue( v2, i );
		Ret = ( v1 - v2 ) / fdu;
	}
	else
	{
		Var->GetValue( v1, i );
		Var->GetValue( v2, i - 1 );
		Ret = ( v1 - v2 ) / fdu;
	}
	return ( Ret );
}


/** Templatised derivative function. Calculates the derivative of the provided stack entry with respect to v.
 */
template <class R>
R SO_DvType( IqShaderData* Var, TqInt i, CqShaderExecEnv& s )
{
	R Ret;
	TqInt uRes = s.uGridRes();
	TqInt vRes = s.vGridRes();
	TqInt GridY = ( i / ( uRes + 1 ) );

	TqFloat fdv;
	s.dv()->GetFloat( fdv );

	R v1,v2;
	if ( GridY < vRes )
	{
		Var->GetValue( v1, i + uRes + 1 );
		Var->GetValue( v2, i );
		Ret = ( v1 - v2 ) / fdv;
	}
	else
	{
		Var->GetValue( v1, i );
		Var->GetValue( v2, i - ( uRes + 1 ) );
		Ret = ( v1 - v2 ) / fdv;
	}
	return ( Ret );
}


/** Templatised derivative function. Calculates the derivative of the provided stack entry with respect to a second stack entry.
 */
template <class R>
R SO_DerivType( IqShaderData* Var, IqShaderData* den, TqInt i, CqShaderExecEnv& s )
{
	assert( NULL != Var );
	
	R Retu, Retv;
	TqInt uRes = s.uGridRes();
	TqInt vRes = s.vGridRes();
	TqInt GridX = i % ( uRes + 1 );
	TqInt GridY = ( i / ( uRes + 1 ) );

	R v1, v2;
	TqFloat u = 1.0f, v = 1.0f;

	// Calculate deriviative in u
	if ( GridX < uRes )
	{
		Var->GetValue( v1, i + 1);
		Var->GetValue( v2, i );
		if( NULL != den )	
			den->GetValue( u, i );
		Retu = ( v1 - v2 ) / u;
	}
	else
	{
		Var->GetValue( v1, i );
		Var->GetValue( v2, i - 1 );
		if( NULL != den )	
			den->GetValue( u, i );
		Retu = ( v1 - v2 ) / u;
	}

	// Calculate deriviative in v
	if ( GridY < vRes )
	{
		Var->GetValue( v1, i + uRes + 1 );
		Var->GetValue( v2, i );
		if( NULL != den )	
			den->GetValue( v, i );
		Retv = ( v1 - v2 ) / v;
	}
	else
	{
		Var->GetValue( v1, i );
		Var->GetValue( v2, i - ( uRes - 1 ) );
		if( NULL != den )	
			den->GetValue( v, i );
		Retv = ( v1 - v2 ) / v;
	}

	return ( Retu + Retv );
}


//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

#endif	// !SHADEREXECENV_H_INCLUDED
