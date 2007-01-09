// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
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
		\brief Declares classes and support structures for the shader execution environment.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

//? Is .h included already?
#ifndef SHADEREXECENV_H_INCLUDED
#define SHADEREXECENV_H_INCLUDED 1

#include	"aqsis.h"

#include	<vector>
#include	<stack>
#include	<map>

#include	"bitvector.h"
#include	"color.h"
#include	"noise.h"
#include	"random.h"
#include	"cellnoise.h"
#include	"vector3d.h"
#include	"vector4d.h"
#include	"ishaderdata.h"
#include	"ishader.h"
#include	"ishaderexecenv.h"
#include	"irenderer.h"
#include	"matrix.h"

#include	"iattributes.h"
#include	"transform.h"

START_NAMESPACE( Aqsis )


extern char*	gVariableClassNames[];
extern TqInt	gcVariableClassNames;
extern char*	gVariableTypeNames[];
extern TqInt	gcVariableTypeNames;
extern char*	gVariableNames[];	///< Vector of variable names.
extern TqUlong	gVariableTokens[];	///< Vector of hash key from above names.


extern TqInt gDefUses;
extern TqInt gDefLightUses;

#define	INIT_SO			TqBool __fVarying=TqFalse; /* A flag which will be set to indicate if the operation has any varying components. */ \
						TqInt __iGrid; /* Integer index used to track progress through the varying data */
#define	CHECKVARY(A)	__fVarying=(A)->Class()==class_varying||__fVarying;
#define	FOR_EACH		__iGrid = 0; \
						CqBitVector& RS = RunningState(); \
						do \
						{ \
							if(!__fVarying || RS.Value( __iGrid ) ) \
							{
#define	END_FOR				} \
						}while( ( ++__iGrid < GridSize() ) && __fVarying);

#define	BEGIN_UNIFORM_SECTION	__iGrid = 0;
#define	END_UNIFORM_SECTION

#define	BEGIN_VARYING_SECTION	FOR_EACH
#define	END_VARYING_SECTION		END_FOR

#define	GETFLOAT(Val)		TqFloat _aq_##Val; (Val)->GetFloat(_aq_##Val,__iGrid)
#define	GETPOINT(Val)		CqVector3D _aq_##Val; (Val)->GetPoint(_aq_##Val,__iGrid)
#define	GETVECTOR(Val)		CqVector3D _aq_##Val; (Val)->GetVector(_aq_##Val,__iGrid)
#define	GETNORMAL(Val)		CqVector3D _aq_##Val; (Val)->GetNormal(_aq_##Val,__iGrid)
#define	GETCOLOR(Val)		CqColor _aq_##Val; (Val)->GetColor(_aq_##Val,__iGrid)
#define	GETSTRING(Val)		CqString _aq_##Val; (Val)->GetString(_aq_##Val,__iGrid)
#define	GETBOOLEAN(Val)		TqBool _aq_##Val; (Val)->GetBool(_aq_##Val,__iGrid)
#define	GETMATRIX(Val)		CqMatrix _aq_##Val; (Val)->GetMatrix(_aq_##Val,__iGrid)

#define	SETFLOAT(Val, v)	(Val)->SetFloat(v,__iGrid)
#define	SETPOINT(Val, v)	(Val)->SetPoint(v,__iGrid)
#define	SETVECTOR(Val, v)	(Val)->SetVector(v,__iGrid)
#define	SETNORMAL(Val, v)	(Val)->SetNormal(v,__iGrid)
#define	SETCOLOR(Val, v)	(Val)->SetColor(v,__iGrid)
#define	SETSTRING(Val, v)	(Val)->SetString(v,__iGrid)
#define	SETBOOLEAN(Val, v)	(Val)->SetBool(v,__iGrid)
#define	SETMATRIX(Val, v)	(Val)->SetMatrix(v,__iGrid)

#define	FLOAT(Val)			_aq_##Val
#define	POINT(Val)			_aq_##Val
#define	VECTOR(Val)			_aq_##Val
#define	NORMAL(Val)			_aq_##Val
#define	COLOR(Val)			_aq_##Val
#define	STRING(Val)			_aq_##Val
#define	BOOLEAN(Val)		_aq_##Val
#define	MATRIX(Val)			_aq_##Val


#define	DEFPARAMIMPL		IqShaderData* Result, IqShader* pShader
#define	DEFVOIDPARAMIMPL	IqShader* pShader
#define DEFPARAMVARIMPL		DEFPARAMIMPL, int cParams, IqShaderData** apParams
#define	DEFVOIDPARAMVARIMPL	DEFVOIDPARAMIMPL, int cParams, IqShaderData** apParams

#define	GET_FILTER_PARAMS	float _pswidth=1.0f,_ptwidth=1.0f; \
							GetFilterParams(cParams, apParams, _pswidth,_ptwidth);
#define	GET_TEXTURE_PARAMS	std::map<std::string, IqShaderData*> paramMap; \
							GetTexParams(cParams, apParams, paramMap);


//----------------------------------------------------------------------
/** \class CqShaderExecEnv
 * Standard shader execution environment. Contains standard variables, and provides SIMD functionality.
 */

class CqShaderExecEnv : public IqShaderExecEnv
{
	public:
		CqShaderExecEnv(boost::shared_ptr<IqRenderer> pRenderContext);
		virtual	~CqShaderExecEnv();

#ifdef _DEBUG

		CqString className() const
		{
			return CqString("CqShaderExecEnv");
		}
#endif

		// Overidden from IqShaderExecEnv, see ishaderexecenv.h for descriptions.
		virtual	void	Initialise( const TqInt uGridRes, const TqInt vGridRes, TqInt microPolygonCount, TqInt shadingPointCount, IqAttributes* pAttr, const boost::shared_ptr<IqTransform>& pTrans, IqShader* pShader, TqInt Uses );
		virtual	TqInt	uGridRes() const
		{
			return ( m_uGridRes );
		}
		virtual	TqInt	vGridRes() const
		{
			return ( m_vGridRes );
		}
		virtual	TqUint	microPolygonCount() const
		{
			return ( m_microPolygonCount );
		}
		virtual	TqUint	shadingPointCount() const
		{
			return ( m_shadingPointCount );
		}
		virtual	const CqMatrix&	matObjectToWorld() const;
		const IqAttributes*	pAttributes() const
		{
			return ( m_pAttributes );
		}
		boost::shared_ptr<const IqTransform>	pTransform() const
		{
			return ( boost::static_pointer_cast<const IqTransform>(m_pTransform) );
		}
		virtual void SetCurrentSurface(IqSurface* pEnv)
		{
			m_pCurrentSurface = pEnv;
		}
		virtual const IqSurface* GetCurrentSurface() const
		{
			return(m_pCurrentSurface);
		}
		virtual	void	ValidateIlluminanceCache( IqShaderData* pP, IqShaderData* pN, IqShader* pShader );
		virtual	void	InvalidateIlluminanceCache()
		{
			m_IlluminanceCacheValid = TqFalse;
		}
		virtual	CqBitVector& CurrentState()
		{
			return ( m_CurrentState );
		}
		virtual	CqBitVector& RunningState()
		{
			return ( m_RunningState );
		}
		virtual	void	GetCurrentState()
		{
			m_RunningState = m_CurrentState;
		}
		virtual	void	ClearCurrentState()
		{
			m_CurrentState.SetAll( TqFalse );
		}
		virtual	void	PushState()
		{
			m_stkState.push( m_RunningState );
		}
		virtual	void	PopState()
		{
			m_RunningState = m_stkState.top();
			m_stkState.pop();
		}
		virtual	void	InvertRunningState()
		{
			m_RunningState.Complement();
			if ( !m_stkState.empty() )
				m_RunningState.Intersect( m_stkState.top() );
		}
		virtual IqShaderData* FindStandardVar( const char* pname );

		virtual	TqInt	FindStandardVarIndex( const char* pname );

		virtual IqShaderData*	pVar( TqInt Index )
		{
			return ( m_apVariables[ Index ] );
		}
		virtual	void	DeleteVariable( TqInt Index )
		{
			delete( m_apVariables[ Index ] );
			m_apVariables[ Index ] = 0;
		}
		virtual IqShaderData* Cs()
		{
			return ( m_apVariables[ EnvVars_Cs ] );
		}
		virtual IqShaderData* Os()
		{
			return ( m_apVariables[ EnvVars_Os ] );
		}
		virtual IqShaderData* Ng()
		{
			return ( m_apVariables[ EnvVars_Ng ] );
		}
		virtual IqShaderData* du()
		{
			return ( m_apVariables[ EnvVars_du ] );
		}
		virtual IqShaderData* dv()
		{
			return ( m_apVariables[ EnvVars_dv ] );
		}
		virtual IqShaderData* L()
		{
			return ( m_apVariables[ EnvVars_L ] );
		}
		virtual IqShaderData* Cl()
		{
			return ( m_apVariables[ EnvVars_Cl ] );
		}
		virtual IqShaderData* Ol()
		{
			return ( m_apVariables[ EnvVars_Ol ] );
		}
		virtual IqShaderData* P()
		{
			return ( m_apVariables[ EnvVars_P ] );
		}
		virtual IqShaderData* dPdu()
		{
			return ( m_apVariables[ EnvVars_dPdu ] );
		}
		virtual IqShaderData* dPdv()
		{
			return ( m_apVariables[ EnvVars_dPdv ] );
		}
		virtual IqShaderData* N()
		{
			return ( m_apVariables[ EnvVars_N ] );
		}
		virtual IqShaderData* u()
		{
			return ( m_apVariables[ EnvVars_u ] );
		}
		virtual IqShaderData* v()
		{
			return ( m_apVariables[ EnvVars_v ] );
		}
		virtual IqShaderData* s()
		{
			return ( m_apVariables[ EnvVars_s ] );
		}
		virtual IqShaderData* t()
		{
			return ( m_apVariables[ EnvVars_t ] );
		}
		virtual IqShaderData* I()
		{
			return ( m_apVariables[ EnvVars_I ] );
		}
		virtual IqShaderData* Ci()
		{
			return ( m_apVariables[ EnvVars_Ci ] );
		}
		virtual IqShaderData* Oi()
		{
			return ( m_apVariables[ EnvVars_Oi ] );
		}
		virtual IqShaderData* Ps()
		{
			return ( m_apVariables[ EnvVars_Ps ] );
		}
		virtual IqShaderData* E()
		{
			return ( m_apVariables[ EnvVars_E ] );
		}
		virtual IqShaderData* ncomps()
		{
			return ( m_apVariables[ EnvVars_ncomps ] );
		}
		virtual IqShaderData* time()
		{
			return ( m_apVariables[ EnvVars_time ] );
		}
		virtual IqShaderData* alpha()
		{
			return ( m_apVariables[ EnvVars_alpha ] );
		}
		virtual IqShaderData* Ns()
		{
			return ( m_apVariables[ EnvVars_Ns ] );
		}
		virtual boost::shared_ptr<IqRenderer> getRenderContext() const
		{
			return ( m_pRenderContext );
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

				if ( strParam.compare( "width" ) == 0 )
					_pswidth = _ptwidth = f;
				else if ( strParam.compare( "swidth" ) == 0 )
					_pswidth = f;
				else if ( strParam.compare( "twidth" ) == 0 )
					_ptwidth = f;
				i += 2;
				cParams -= 2;
			}
		}
		/** Internal function to extract additional named texture control parameters from an array of stack entries.
		 */
		void	GetTexParams( int cParams, IqShaderData** apParams, std::map<std::string, IqShaderData*>& map )
		{
			CqString strParam;
			TqInt i = 0;
			while ( cParams > 0 )
			{
				apParams[ i ] ->GetString( strParam, 0 );
				map[ strParam ] = apParams[ i + 1 ];
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

		TqInt	m_uGridRes;				///< The resolution of the grid in u.
		TqInt	m_vGridRes;				///< The resolution of the grid in u.
		TqInt	m_microPolygonCount;			///< The resolution of the grid.
		TqInt	m_shadingPointCount;			///< The resolution of the grid.
		TqInt	m_GridI;				///< The current SIMD index.
		TqUint	m_li;					///< Light index, used during illuminance loop.
		TqInt	m_Illuminate;
		TqBool	m_IlluminanceCacheValid;	///< Flag indicating whether the illuminance cache is valid.
		IqAttributes* m_pAttributes;	///< Pointer to the associated attributes.
		CqTransformPtr m_pTransform;		///< Pointer to the associated transform.
		CqBitVector	m_CurrentState;			///< SIMD execution state bit vector accumulator.
		CqBitVector	m_RunningState;			///< SIMD running execution state bit vector.
		std::stack<CqBitVector>	m_stkState;				///< Stack of execution state bit vectors.
		boost::shared_ptr<IqRenderer>	m_pRenderContext;
		TqInt	m_LocalIndex;			///< Local cached variable index to speed repeated access to the same local variable.
		IqSurface*	m_pCurrentSurface;	///< Pointer to the surface being shaded.

	public:

		virtual	TqBool	SO_init_illuminance();
		virtual	TqBool	SO_advance_illuminance();

		// ShadeOps
		virtual STD_SO	SO_radians( FLOATVAL degrees, DEFPARAM );
		virtual STD_SO	SO_degrees( FLOATVAL radians, DEFPARAM );
		virtual STD_SO	SO_sin( FLOATVAL a, DEFPARAM );
		virtual STD_SO	SO_asin( FLOATVAL a, DEFPARAM );
		virtual STD_SO	SO_cos( FLOATVAL a, DEFPARAM );
		virtual STD_SO	SO_acos( FLOATVAL a, DEFPARAM );
		virtual STD_SO	SO_tan( FLOATVAL a, DEFPARAM );
		virtual STD_SO	SO_atan( FLOATVAL yoverx, DEFPARAM );
		virtual STD_SO	SO_atan( FLOATVAL y, FLOATVAL x, DEFPARAM );
		virtual STD_SO	SO_pow( FLOATVAL x, FLOATVAL y, DEFPARAM );
		virtual STD_SO	SO_exp( FLOATVAL x, DEFPARAM );
		virtual STD_SO	SO_sqrt( FLOATVAL x, DEFPARAM );
		virtual STD_SO	SO_log( FLOATVAL x, DEFPARAM );
		virtual STD_SO	SO_log( FLOATVAL x, FLOATVAL base, DEFPARAM );
		virtual STD_SO	SO_mod( FLOATVAL a, FLOATVAL b, DEFPARAM );
		virtual STD_SO	SO_abs( FLOATVAL x, DEFPARAM );
		virtual STD_SO	SO_sign( FLOATVAL x, DEFPARAM );
		virtual STD_SO	SO_min( FLOATVAL a, FLOATVAL b, DEFPARAMVAR );
		virtual STD_SO	SO_max( FLOATVAL a, FLOATVAL b, DEFPARAMVAR );
		virtual STD_SO	SO_pmin( POINTVAL a, POINTVAL b, DEFPARAMVAR );
		virtual STD_SO	SO_pmax( POINTVAL a, POINTVAL b, DEFPARAMVAR );
		virtual STD_SO	SO_cmin( COLORVAL a, COLORVAL b, DEFPARAMVAR );
		virtual STD_SO	SO_cmax( COLORVAL a, COLORVAL b, DEFPARAMVAR );
		virtual STD_SO	SO_clamp( FLOATVAL a, FLOATVAL _min, FLOATVAL _max, DEFPARAM );
		virtual STD_SO	SO_pclamp( POINTVAL a, POINTVAL _min, POINTVAL _max, DEFPARAM );
		virtual STD_SO	SO_cclamp( COLORVAL a, COLORVAL _min, COLORVAL _max, DEFPARAM );
		virtual STD_SO	SO_floor( FLOATVAL x, DEFPARAM );
		virtual STD_SO	SO_ceil( FLOATVAL x, DEFPARAM );
		virtual STD_SO	SO_round( FLOATVAL x, DEFPARAM );
		virtual STD_SO	SO_step( FLOATVAL _min, FLOATVAL value, DEFPARAM );
		virtual STD_SO	SO_smoothstep( FLOATVAL _min, FLOATVAL _max, FLOATVAL value, DEFPARAM );
		virtual STD_SO	SO_fspline( FLOATVAL value, DEFPARAMVAR );
		virtual STD_SO	SO_cspline( FLOATVAL value, DEFPARAMVAR );
		virtual STD_SO	SO_pspline( FLOATVAL value, DEFPARAMVAR );
		virtual STD_SO	SO_sfspline( STRINGVAL basis, FLOATVAL value, DEFPARAMVAR );
		virtual STD_SO	SO_scspline( STRINGVAL basis, FLOATVAL value, DEFPARAMVAR );
		virtual STD_SO	SO_spspline( STRINGVAL basis, FLOATVAL value, DEFPARAMVAR );
		virtual STD_SO	SO_fDu( FLOATVAL p, DEFPARAM );
		virtual STD_SO	SO_fDv( FLOATVAL p, DEFPARAM );
		virtual STD_SO	SO_fDeriv( FLOATVAL p, FLOATVAL den, DEFPARAM );
		virtual STD_SO	SO_cDu( COLORVAL p, DEFPARAM );
		virtual STD_SO	SO_cDv( COLORVAL p, DEFPARAM );
		virtual STD_SO	SO_cDeriv( COLORVAL p, FLOATVAL den, DEFPARAM );
		virtual STD_SO	SO_pDu( POINTVAL p, DEFPARAM );
		virtual STD_SO	SO_pDv( POINTVAL p, DEFPARAM );
		virtual STD_SO	SO_pDeriv( POINTVAL p, FLOATVAL den, DEFPARAM );
		virtual STD_SO	SO_frandom( DEFPARAM );
		virtual STD_SO	SO_crandom( DEFPARAM );
		virtual STD_SO	SO_prandom( DEFPARAM );
		virtual STD_SO	SO_fnoise1( FLOATVAL v, DEFPARAM );
		virtual STD_SO	SO_fnoise2( FLOATVAL u, FLOATVAL v, DEFPARAM );
		virtual STD_SO	SO_fnoise3( POINTVAL p, DEFPARAM );
		virtual STD_SO	SO_fnoise4( POINTVAL p, FLOATVAL t, DEFPARAM );
		virtual STD_SO	SO_cnoise1( FLOATVAL v, DEFPARAM );
		virtual STD_SO	SO_cnoise2( FLOATVAL u, FLOATVAL v, DEFPARAM );
		virtual STD_SO	SO_cnoise3( POINTVAL p, DEFPARAM );
		virtual STD_SO	SO_cnoise4( POINTVAL p, FLOATVAL t, DEFPARAM );
		virtual STD_SO	SO_pnoise1( FLOATVAL v, DEFPARAM );
		virtual STD_SO	SO_pnoise2( FLOATVAL u, FLOATVAL v, DEFPARAM );
		virtual STD_SO	SO_pnoise3( POINTVAL p, DEFPARAM );
		virtual STD_SO	SO_pnoise4( POINTVAL p, FLOATVAL t, DEFPARAM );
		virtual STD_SO	SO_setcomp( COLORVAL p, FLOATVAL i, FLOATVAL v, DEFVOIDPARAM );
		virtual STD_SO	SO_setxcomp( POINTVAL p, FLOATVAL v, DEFVOIDPARAM );
		virtual STD_SO	SO_setycomp( POINTVAL p, FLOATVAL v, DEFVOIDPARAM );
		virtual STD_SO	SO_setzcomp( POINTVAL p, FLOATVAL v, DEFVOIDPARAM );
		virtual STD_SO	SO_length( VECTORVAL V, DEFPARAM );
		virtual STD_SO	SO_distance( POINTVAL P1, POINTVAL P2, DEFPARAM );
		virtual STD_SO	SO_area( POINTVAL p, DEFPARAM );
		virtual STD_SO	SO_normalize( VECTORVAL V, DEFPARAM );
		virtual STD_SO	SO_faceforward( NORMALVAL N, VECTORVAL I, DEFPARAM );
		virtual STD_SO	SO_faceforward2( NORMALVAL N, VECTORVAL I, NORMALVAL Nref, DEFPARAM );
		virtual STD_SO	SO_reflect( VECTORVAL I, NORMALVAL N, DEFPARAM );
		virtual STD_SO	SO_refract( VECTORVAL I, NORMALVAL N, FLOATVAL eta, DEFPARAM );
		virtual STD_SO	SO_fresnel( VECTORVAL I, NORMALVAL N, FLOATVAL eta, FLOATVAL Kr, FLOATVAL Kt, DEFVOIDPARAM );
		virtual STD_SO	SO_fresnel( VECTORVAL I, NORMALVAL N, FLOATVAL eta, FLOATVAL Kr, FLOATVAL Kt, VECTORVAL R, VECTORVAL T, DEFVOIDPARAM );
		virtual STD_SO	SO_transform( STRINGVAL fromspace, STRINGVAL tospace, POINTVAL p, DEFPARAM );
		virtual STD_SO	SO_transform( STRINGVAL tospace, POINTVAL p, DEFPARAM );
		virtual STD_SO	SO_transformm( MATRIXVAL tospace, POINTVAL p, DEFPARAM );
		virtual STD_SO	SO_vtransform( STRINGVAL fromspace, STRINGVAL tospace, VECTORVAL p, DEFPARAM );
		virtual STD_SO	SO_vtransform( STRINGVAL tospace, VECTORVAL p, DEFPARAM );
		virtual STD_SO	SO_vtransformm( MATRIXVAL tospace, VECTORVAL p, DEFPARAM );
		virtual STD_SO	SO_ntransform( STRINGVAL fromspace, STRINGVAL tospace, NORMALVAL p, DEFPARAM );
		virtual STD_SO	SO_ntransform( STRINGVAL tospace, NORMALVAL p, DEFPARAM );
		virtual STD_SO	SO_ntransformm( MATRIXVAL tospace, NORMALVAL p, DEFPARAM );
		virtual STD_SO	SO_depth( POINTVAL p, DEFPARAM );
		virtual STD_SO	SO_calculatenormal( POINTVAL p, DEFPARAM );
		virtual STD_SO	SO_cmix( COLORVAL color0, COLORVAL color1, FLOATVAL value, DEFPARAM );
		virtual STD_SO	SO_fmix( FLOATVAL f0, FLOATVAL f1, FLOATVAL value, DEFPARAM );
		virtual STD_SO	SO_pmix( POINTVAL p0, POINTVAL p1, FLOATVAL value, DEFPARAM );
		virtual STD_SO	SO_vmix( VECTORVAL v0, VECTORVAL v1, FLOATVAL value, DEFPARAM );
		virtual STD_SO	SO_nmix( NORMALVAL n0, NORMALVAL n1, FLOATVAL value, DEFPARAM );
		virtual STD_SO	SO_ambient( DEFPARAM );
		virtual STD_SO	SO_diffuse( NORMALVAL N, DEFPARAM );
		virtual STD_SO	SO_specular( NORMALVAL N, VECTORVAL V, FLOATVAL roughness, DEFPARAM );
		virtual STD_SO	SO_phong( NORMALVAL N, VECTORVAL V, FLOATVAL size, DEFPARAM );
		virtual STD_SO	SO_trace( POINTVAL P, VECTORVAL R, DEFPARAM );
		virtual STD_SO	SO_ftexture1( STRINGVAL name, FLOATVAL channel, DEFPARAMVAR );
		virtual STD_SO	SO_ftexture2( STRINGVAL name, FLOATVAL channel, FLOATVAL s, FLOATVAL t, DEFPARAMVAR );
		virtual STD_SO	SO_ftexture3( STRINGVAL name, FLOATVAL channel, FLOATVAL s1, FLOATVAL t1, FLOATVAL s2, FLOATVAL t2, FLOATVAL s3, FLOATVAL t3, FLOATVAL s4, FLOATVAL t4, DEFPARAMVAR );
		virtual STD_SO	SO_ctexture1( STRINGVAL name, FLOATVAL channel, DEFPARAMVAR );
		virtual STD_SO	SO_ctexture2( STRINGVAL name, FLOATVAL channel, FLOATVAL s, FLOATVAL t, DEFPARAMVAR );
		virtual STD_SO	SO_ctexture3( STRINGVAL name, FLOATVAL channel, FLOATVAL s1, FLOATVAL t1, FLOATVAL s2, FLOATVAL t2, FLOATVAL s3, FLOATVAL t3, FLOATVAL s4, FLOATVAL t4, DEFPARAMVAR );
		virtual STD_SO	SO_fenvironment2( STRINGVAL name, FLOATVAL channel, VECTORVAL R, DEFPARAMVAR );
		virtual STD_SO	SO_fenvironment3( STRINGVAL name, FLOATVAL channel, VECTORVAL R1, VECTORVAL R2, VECTORVAL R3, VECTORVAL R4, DEFPARAMVAR );
		virtual STD_SO	SO_cenvironment2( STRINGVAL name, FLOATVAL channel, VECTORVAL R, DEFPARAMVAR );
		virtual STD_SO	SO_cenvironment3( STRINGVAL name, FLOATVAL channel, VECTORVAL R1, VECTORVAL R2, VECTORVAL R3, VECTORVAL R4, DEFPARAMVAR );
		virtual STD_SO	SO_bump1( STRINGVAL name, FLOATVAL channel, DEFPARAMVAR );
		virtual STD_SO	SO_bump2( STRINGVAL name, FLOATVAL channel, FLOATVAL s, FLOATVAL t, DEFPARAMVAR );
		virtual STD_SO	SO_bump3( STRINGVAL name, FLOATVAL channel, FLOATVAL s1, FLOATVAL t1, FLOATVAL s2, FLOATVAL t2, FLOATVAL s3, FLOATVAL t3, FLOATVAL s4, FLOATVAL t4, DEFPARAMVAR );
		virtual STD_SO	SO_shadow( STRINGVAL name, FLOATVAL channel, POINTVAL P, DEFPARAMVAR );
		virtual STD_SO	SO_shadow1( STRINGVAL name, FLOATVAL channel, POINTVAL P1, POINTVAL P2, POINTVAL P3, POINTVAL P4, DEFPARAMVAR );
		virtual STD_SO	SO_illuminance( STRINGVAL Category, POINTVAL P, DEFVOIDPARAM );
		virtual STD_SO	SO_illuminance( STRINGVAL Category, POINTVAL P, VECTORVAL Axis, FLOATVAL Angle, DEFVOIDPARAM );
		virtual STD_SO	SO_illuminate( POINTVAL P, VECTORVAL Axis, FLOATVAL Angle, DEFVOIDPARAM );
		virtual STD_SO	SO_illuminate( POINTVAL P, DEFVOIDPARAM );
		virtual STD_SO	SO_solar( VECTORVAL Axis, FLOATVAL Angle, DEFVOIDPARAM );
		virtual STD_SO	SO_solar( DEFVOIDPARAM );
		virtual STD_SO	SO_printf( STRINGVAL str, DEFVOIDPARAMVAR );
		virtual STD_SO	SO_format( STRINGVAL str, DEFPARAMVAR );
		virtual STD_SO	SO_concat( STRINGVAL stra, STRINGVAL strb, DEFPARAMVAR );
		virtual STD_SO	SO_atmosphere( STRINGVAL name, IqShaderData* pV, DEFPARAM );
		virtual STD_SO	SO_displacement( STRINGVAL name, IqShaderData* pV, DEFPARAM );
		virtual STD_SO	SO_lightsource( STRINGVAL name, IqShaderData* pV, DEFPARAM );
		virtual STD_SO	SO_surface( STRINGVAL name, IqShaderData* pV, DEFPARAM );
		virtual STD_SO	SO_attribute( STRINGVAL name, IqShaderData* pV, DEFPARAM );
		virtual STD_SO	SO_option( STRINGVAL name, IqShaderData* pV, DEFPARAM );
		virtual STD_SO	SO_rendererinfo( STRINGVAL name, IqShaderData* pV, DEFPARAM );
		virtual STD_SO	SO_incident( STRINGVAL name, IqShaderData* pV, DEFPARAM );
		virtual STD_SO	SO_opposite( STRINGVAL name, IqShaderData* pV, DEFPARAM );
		virtual STD_SO	SO_fcellnoise1( FLOATVAL v, DEFPARAM );
		virtual STD_SO	SO_fcellnoise2( FLOATVAL u, FLOATVAL v, DEFPARAM );
		virtual STD_SO	SO_fcellnoise3( POINTVAL p, DEFPARAM );
		virtual STD_SO	SO_fcellnoise4( POINTVAL p, FLOATVAL v, DEFPARAM );
		virtual STD_SO	SO_ccellnoise1( FLOATVAL v, DEFPARAM );
		virtual STD_SO	SO_ccellnoise2( FLOATVAL u, FLOATVAL v, DEFPARAM );
		virtual STD_SO	SO_ccellnoise3( POINTVAL p, DEFPARAM );
		virtual STD_SO	SO_ccellnoise4( POINTVAL p, FLOATVAL v, DEFPARAM );
		virtual STD_SO	SO_pcellnoise1( FLOATVAL v, DEFPARAM );
		virtual STD_SO	SO_pcellnoise2( FLOATVAL u, FLOATVAL v, DEFPARAM );
		virtual STD_SO	SO_pcellnoise3( POINTVAL p, DEFPARAM );
		virtual STD_SO	SO_pcellnoise4( POINTVAL p, FLOATVAL v, DEFPARAM );
		virtual STD_SO	SO_fpnoise1( FLOATVAL v, FLOATVAL period, DEFPARAM );
		virtual STD_SO	SO_fpnoise2( FLOATVAL u, FLOATVAL v, FLOATVAL uperiod, FLOATVAL vperiod, DEFPARAM );
		virtual STD_SO	SO_fpnoise3( POINTVAL p, POINTVAL pperiod, DEFPARAM );
		virtual STD_SO	SO_fpnoise4( POINTVAL p, FLOATVAL t, POINTVAL pperiod, FLOATVAL tperiod, DEFPARAM );
		virtual STD_SO	SO_cpnoise1( FLOATVAL v, FLOATVAL period, DEFPARAM );
		virtual STD_SO	SO_cpnoise2( FLOATVAL u, FLOATVAL v, FLOATVAL uperiod, FLOATVAL vperiod, DEFPARAM );
		virtual STD_SO	SO_cpnoise3( POINTVAL p, POINTVAL pperiod, DEFPARAM );
		virtual STD_SO	SO_cpnoise4( POINTVAL p, FLOATVAL t, POINTVAL pperiod, FLOATVAL tperiod, DEFPARAM );
		virtual STD_SO	SO_ppnoise1( FLOATVAL v, FLOATVAL period, DEFPARAM );
		virtual STD_SO	SO_ppnoise2( FLOATVAL u, FLOATVAL v, FLOATVAL uperiod, FLOATVAL vperiod, DEFPARAM );
		virtual STD_SO	SO_ppnoise3( POINTVAL p, POINTVAL pperiod, DEFPARAM );
		virtual STD_SO	SO_ppnoise4( POINTVAL p, FLOATVAL t, POINTVAL pperiod, FLOATVAL tperiod, DEFPARAM );
		virtual STD_SO	SO_ctransform( STRINGVAL fromspace, STRINGVAL tospace, COLORVAL c, DEFPARAM );
		virtual STD_SO	SO_ctransform( STRINGVAL tospace, COLORVAL c, DEFPARAM );
		virtual STD_SO	SO_ptlined( POINTVAL P0, POINTVAL P1, POINTVAL Q, DEFPARAM );
		virtual STD_SO	SO_inversesqrt( FLOATVAL x, DEFPARAM );
		virtual STD_SO	SO_match( STRINGVAL a, STRINGVAL b, DEFPARAM );
		virtual STD_SO	SO_rotate( VECTORVAL Q, FLOATVAL angle, POINTVAL P0, POINTVAL P1, DEFPARAM );
		virtual STD_SO	SO_filterstep( FLOATVAL edge, FLOATVAL s1, DEFPARAMVAR );
		virtual STD_SO	SO_filterstep2( FLOATVAL edge, FLOATVAL s1, FLOATVAL s2, DEFPARAMVAR );
		virtual STD_SO	SO_specularbrdf( VECTORVAL L, NORMALVAL N, VECTORVAL V, FLOATVAL rough, DEFPARAM );
		virtual STD_SO	SO_mtransform( STRINGVAL fromspace, STRINGVAL tospace, MATRIXVAL m, DEFPARAM );
		virtual STD_SO	SO_mtransform( STRINGVAL tospace, MATRIXVAL m, DEFPARAM );
		virtual STD_SO	SO_setmcomp( MATRIXVAL M, FLOATVAL row, FLOATVAL column, FLOATVAL val, DEFVOIDPARAM );
		virtual STD_SO	SO_determinant( MATRIXVAL M, DEFPARAM );
		virtual STD_SO	SO_mtranslate( MATRIXVAL M, VECTORVAL V, DEFPARAM );
		virtual STD_SO	SO_mrotate( MATRIXVAL M, FLOATVAL angle, VECTORVAL axis, DEFPARAM );
		virtual STD_SO	SO_mscale( MATRIXVAL M, POINTVAL s, DEFPARAM );
		virtual STD_SO	SO_fsplinea( FLOATVAL value, FLOATARRAYVAL a, DEFPARAM );
		virtual STD_SO	SO_csplinea( FLOATVAL value, COLORARRAYVAL a, DEFPARAM );
		virtual STD_SO	SO_psplinea( FLOATVAL value, POINTARRAYVAL a, DEFPARAM );
		virtual STD_SO	SO_sfsplinea( STRINGVAL basis, FLOATVAL value, FLOATARRAYVAL a, DEFPARAM );
		virtual STD_SO	SO_scsplinea( STRINGVAL basis, FLOATVAL value, COLORARRAYVAL a, DEFPARAM );
		virtual STD_SO	SO_spsplinea( STRINGVAL basis, FLOATVAL value, POINTARRAYVAL a, DEFPARAM );
		virtual STD_SO	SO_shadername( DEFPARAM );
		virtual STD_SO	SO_shadername2( STRINGVAL shader, DEFPARAM );
		virtual STD_SO	SO_textureinfo( STRINGVAL shader, STRINGVAL dataname, IqShaderData* pV, DEFPARAM );
		virtual STD_SO	SO_bake_f( STRINGVAL name, FLOATVAL s, FLOATVAL t, FLOATVAL f, DEFVOIDPARAMVAR );
		virtual STD_SO	SO_bake_3c( STRINGVAL name, FLOATVAL s, FLOATVAL t, COLORVAL f, DEFVOIDPARAMVAR );
		virtual STD_SO	SO_bake_3p( STRINGVAL name, FLOATVAL s, FLOATVAL t, POINTVAL f, DEFVOIDPARAMVAR );
		virtual STD_SO	SO_bake_3v( STRINGVAL name, FLOATVAL s, FLOATVAL t, VECTORVAL f, DEFVOIDPARAMVAR );
		virtual STD_SO	SO_bake_3n( STRINGVAL name, FLOATVAL s, FLOATVAL t, NORMALVAL f, DEFVOIDPARAMVAR );
		virtual STD_SO	SO_external( DSOMethod method, void* initData, DEFPARAMVAR );
		virtual	STD_SO	SO_occlusion( STRINGVAL occlmap, FLOATVAL channel, POINTVAL P, NORMALVAL N, FLOATVAL samples, DEFPARAMVAR );
};




//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

#endif	// !SHADEREXECENV_H_INCLUDED
