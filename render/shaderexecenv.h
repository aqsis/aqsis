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

#include	"bitvector.h"
#include	"color.h"
#include	"noise.h"
#include	"cellnoise.h"
#include	"vector3d.h"
#include	"vector4d.h"
#include	"shadervariable.h"
#include	"matrix.h"
#include	"shaderstack.h"

#include	"specific.h"	// Needed for namespace macros.

START_NAMESPACE(Aqsis)

/** \enum EqEnvVars
 * Identifiers for the standard environment variables.
 */

enum EqEnvVars
{
	EnvVars_Cs,		///< Surface color.
	EnvVars_Os,		///< Surface opacity.
	EnvVars_Ng,		///< Geometric normal.
	EnvVars_du,		///< First derivative in u.
	EnvVars_dv,		///< First derivative in v.
	EnvVars_L,		///< Incoming light direction.
	EnvVars_Cl,		///< Light color.
	EnvVars_Ol,		///< Light opacity.
 	EnvVars_P,		///< Point being shaded.
	EnvVars_dPdu,	///< Change in P with respect to change in u.
	EnvVars_dPdv,	///< Change in P with respect to change in v.
	EnvVars_N,		///< Surface normal.
	EnvVars_u,		///< Surface u coordinate.
	EnvVars_v,		///< Surface v coordinate.
	EnvVars_s,		///< Texture s coordinate.
	EnvVars_t,		///< Texture t coordinate.
	EnvVars_I,		///< Incident ray direction.
	EnvVars_Ci,		///< Incident color.
	EnvVars_Oi,		///< Incident opacity.
	EnvVars_Ps,		///< Point being lit.
 	EnvVars_E,		///< Viewpoint position.
	EnvVars_ncomps,	///< Number of color components.
	EnvVars_time,	///< Frame time.
	EnvVars_alpha,	///< Fractional pixel coverage.

	EnvVars_Last
};

extern TqInt gDefUses;
extern TqInt gDefLightUses;

#define	INIT_SOR		TqBool __fVarying=TqFalse;
#define	CHECKVARY(A)	__fVarying=(A).Size()>1||__fVarying;
#define	FOR_EACHR		/*TqBool __f=__fVarying?RunningState().Count()<GridSize():TqFalse;*/ \
						Reset(); \
						do \
						{ \
							TqInt i=GridI(); \
							if(!__fVarying || RunningState().Value(i)) \
							{
#define	END_FORR			} \
						}while(Advance() && __fVarying);

#define	FLOAT(Val)		(Val).Value(temp_float,i)
#define	POINT(Val)		(Val).Value(temp_point,i)
#define	VECTOR(Val)		(Val).Value(temp_point,i)
#define	NORMAL(Val)		(Val).Value(temp_point,i)
#define	COLOR(Val)		(Val).Value(temp_color,i)
#define	STRING(Val)		(Val).Value(temp_string,i)
#define	BOOLEAN(Val)	(Val).Value(temp_bool,i)
#define	MATRIX(Val)		(Val).Value(temp_matrix,i)

#define	DEFPARAM		CqVMStackEntry& Result, CqShader* pShader=0
#define	DEFVOIDPARAM	CqShader* pShader=0
#define	DEFPARAMVAR		DEFPARAM, int cParams=0, CqVMStackEntry** apParams=0
#define	DEFVOIDPARAMVAR	DEFVOIDPARAM, int cParams=0, CqVMStackEntry** apParams=0

#define	DEFPARAMIMPL		CqVMStackEntry& Result, CqShader* pShader
#define	DEFVOIDPARAMIMPL	CqShader* pShader
#define DEFPARAMVARIMPL		DEFPARAMIMPL, int cParams, CqVMStackEntry** apParams
#define	DEFVOIDPARAMVARIMPL	DEFVOIDPARAMIMPL, int cParams, CqVMStackEntry** apParams

#define	STD_SO		void
#define	STD_SOIMPL	void

#define	GET_FILTER_PARAMS	float _pswidth=1.0f,_ptwidth=1.0f; \
							GetFilterParams(cParams, apParams, _pswidth,_ptwidth);
#define	GET_TEXTURE_PARAMS	float _pswidth=1.0f,_ptwidth=1.0f,_psblur=0.0f,_ptblur=0.0f, _pfill=0.0f; \
							GetTexParams(cParams, apParams, _pswidth,_ptwidth,_psblur,_ptblur,_pfill);

#define	FLOATVAL	CqVMStackEntry&
#define	POINTVAL	CqVMStackEntry&
#define	VECTORVAL	CqVMStackEntry&
#define	NORMALVAL	CqVMStackEntry&
#define	COLORVAL	CqVMStackEntry&
#define	STRINGVAL	CqVMStackEntry&
#define	MATRIXVAL	CqVMStackEntry&

#define	FLOATPTR	CqVMStackEntry**
#define	POINTPTR	CqVMStackEntry**
#define	VECTORPTR	CqVMStackEntry**
#define	NORMALPTR	CqVMStackEntry**
#define	COLORPTR	CqVMStackEntry**
#define	STRINGPTR	CqVMStackEntry**
#define	MATRIXPTR	CqVMStackEntry**

#define	FLOATARRAYVAL	CqVMStackEntry&
#define	POINTARRAYVAL	CqVMStackEntry&
#define	VECTORARRAYVAL	CqVMStackEntry&
#define	NORMALARRAYVAL	CqVMStackEntry&
#define	COLORARRAYVAL	CqVMStackEntry&
#define	STRINGARRAYVAL	CqVMStackEntry&
#define	MATRIXARRAYVAL	CqVMStackEntry&

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
			void		Reset()		{m_GridI=0;}
						/** Advance the internal SIMD counter.
						 * \return Boolean indicating SIMD completion.
						 */
			TqBool		Advance()	{m_GridI++; return(m_GridI<m_GridSize);}
			void		Initialise(const TqInt uGridRes, const TqInt vGridRes, CqSurface* pSurface, TqInt Uses);
						/** Get internal SIMD index.
						 */
			TqInt&		GridI()		{return(m_GridI);}
						/** Get grid size in u
						 */
			TqInt		uGridRes()	{return(m_uGridRes);}
						/** Get grid size in v
						 */
			TqInt		vGridRes()	{return(m_vGridRes);}
						/** Get total grid size.
						 */
			TqInt		GridSize()	{return(m_GridSize);}
						/** Get a pointer to the associated surface.
						 */
			CqSurface*	pSurface() const	{return(m_pSurface);}
			const CqMatrix&	matObjectToWorld() const;
			
			void		ValidateIlluminanceCache(CqVMStackEntry& P, CqShader* pShader);
						/** Reset the illuminance cache.
						 */
			void		InvalidateIlluminanceCache()	{m_IlluminanceCacheValid=TqFalse;}
						/** Get the current execution state. Bits in the vector indicate which SIMD indexes have passed the current condition.
						 */
			CqBitVector& CurrentState()	{return(m_CurrentState);}
						/** Get the running execution state. Bits in the vector indicate which SIMD indexes are valid.
						 */
			CqBitVector& RunningState()	{return(m_RunningState);}
						/** Transfer the current state into the running state.
						 */
			void		GetCurrentState()	{m_RunningState=m_CurrentState;}
						/** Clear the current state ready for a new condition.
						 */
			void		ClearCurrentState()	{m_CurrentState.SetAll(TqFalse);}
						/** Push the running state onto the stack.
						 */
			void		PushState()	{m_stkState.push(m_RunningState);}
						/** Pop the running state from the stack.
						 */
			void		PopState()	{m_RunningState=m_stkState.top(); m_stkState.pop();}
						/** Invert the bits in the running state, to perform the opposite to the condition, i.e. else.
						 */
			void		InvertRunningState()
										{
											m_RunningState.Complement();
											if(!m_stkState.empty())
												m_RunningState.Intersect(m_stkState.top());
										}	
						/** Find a named standard variable in the list.
						 * \param pname Character pointer to the name.
						 * \return CqShaderVariable pointer or 0.
						 */
			CqShaderVariable* FindStandardVar(char* pname)
										{
											TqInt i;
											for(i=0; i<EnvVars_Last; i++)
											{
												if(strcmp(m_apVariableNames[i],pname)==0)
													return(m_apVariables[i]);
											}
											return(0);
										}
						/** Find a named standard variable in the list.
						 * \param pname Character pointer to the name.
						 * \return Integer index in the list or -1.
						 */
			TqInt		FindStandardVarIndex(char* pname)
										{
											TqInt i;
											for(i=0; i<EnvVars_Last; i++)
											{
												if(strcmp(m_apVariableNames[i],pname)==0)
													return(i);
											}
											return(-1);
										}

						/** Get a standard variable pointer given an index.
						 * \param Index The integer index returned from FindStandardVarIndex.
						 * \return CqShaderVariable pointer.
						 */
			CqShaderVariable*			pVar(TqInt Index)		{return(m_apVariables[Index]);}
						/** Delete an indexed variable from the list.
						 * \param Index The integer index returned from FindStandardVarIndex.
						 */
			void						DeleteVariable(TqInt Index)
																{
																	delete(m_apVariables[Index]);
																	m_apVariables[Index]=0;
																}

						/** Get a reference to the Cq standard variable.
						 */
			CqShaderVariableVarying<Type_Color,CqColor>&		Cs()		{return(*static_cast<CqShaderVariableVarying<Type_Color,CqColor>*>(m_apVariables[EnvVars_Cs    ]));}
						/** Get a reference to the Os standard variable.
						 */
			CqShaderVariableVarying<Type_Color,CqColor>&		Os()		{return(*static_cast<CqShaderVariableVarying<Type_Color,CqColor>*>(m_apVariables[EnvVars_Os    ]));}
						/** Get a reference to the Ng standard variable.
						 */
			CqShaderVariableVarying<Type_Normal,CqVector3D>&	Ng()		{return(*static_cast<CqShaderVariableVarying<Type_Normal,CqVector3D>*>(m_apVariables[EnvVars_Ng  ]));}
						/** Get a reference to the du standard variable.
						 */
			CqShaderVariableUniform<Type_Float,TqFloat>&			du()		{return(*static_cast<CqShaderVariableUniform<Type_Float,TqFloat>*>(m_apVariables[EnvVars_du     ]));}
						/** Get a reference to the dv standard variable.
						 */
			CqShaderVariableUniform<Type_Float,TqFloat>&			dv()		{return(*static_cast<CqShaderVariableUniform<Type_Float,TqFloat>*>(m_apVariables[EnvVars_dv     ]));}
						/** Get a reference to the L standard variable.
						 */
			CqShaderVariableVarying<Type_Vector,CqVector3D>&	L()			{return(*static_cast<CqShaderVariableVarying<Type_Vector,CqVector3D>*>(m_apVariables[EnvVars_L   ]));}
						/** Get a reference to the Cl standard variable.
						 */
			CqShaderVariableVarying<Type_Color,CqColor>&		Cl()		{return(*static_cast<CqShaderVariableVarying<Type_Color,CqColor>*>(m_apVariables[EnvVars_Cl    ]));}
						/** Get a reference to the Ol standard variable.
						 */
			CqShaderVariableVarying<Type_Color,CqColor>&		Ol()		{return(*static_cast<CqShaderVariableVarying<Type_Color,CqColor>*>(m_apVariables[EnvVars_Ol    ]));}
						/** Get a reference to the P standard variable.
						 */
			CqShaderVariableVarying<Type_Point,CqVector3D>&		P()			{return(*static_cast<CqShaderVariableVarying<Type_Point,CqVector3D>*>(m_apVariables[EnvVars_P  ]));}
						/** Get a reference to the dPdu standard variable.
						 */
			CqShaderVariableVarying<Type_Point,CqVector3D>&		dPdu()		{return(*static_cast<CqShaderVariableVarying<Type_Point,CqVector3D>*>(m_apVariables[EnvVars_dPdu]));}
						/** Get a reference to the dPdv standard variable.
						 */
			CqShaderVariableVarying<Type_Point,CqVector3D>&		dPdv()		{return(*static_cast<CqShaderVariableVarying<Type_Point,CqVector3D>*>(m_apVariables[EnvVars_dPdv]));}
						/** Get a reference to the N standard variable.
						 */
			CqShaderVariableVarying<Type_Normal,CqVector3D>&	N()			{return(*static_cast<CqShaderVariableVarying<Type_Normal,CqVector3D>*>(m_apVariables[EnvVars_N   ]));}
						/** Get a reference to the u standard variable.
						 */
			CqShaderVariableVarying<Type_Float,TqFloat>&			u()			{return(*static_cast<CqShaderVariableVarying<Type_Float,TqFloat>*>(m_apVariables[EnvVars_u      ]));}
						/** Get a reference to the v standard variable.
						 */
			CqShaderVariableVarying<Type_Float,TqFloat>&			v()			{return(*static_cast<CqShaderVariableVarying<Type_Float,TqFloat>*>(m_apVariables[EnvVars_v      ]));}
						/** Get a reference to the s standard variable.
						 */
			CqShaderVariableVarying<Type_Float,TqFloat>&			s()			{return(*static_cast<CqShaderVariableVarying<Type_Float,TqFloat>*>(m_apVariables[EnvVars_s      ]));}
						/** Get a reference to the t standard variable.
						 */
			CqShaderVariableVarying<Type_Float,TqFloat>&			t()			{return(*static_cast<CqShaderVariableVarying<Type_Float,TqFloat>*>(m_apVariables[EnvVars_t      ]));}
						/** Get a reference to the I standard variable.
						 */
			CqShaderVariableVarying<Type_Vector,CqVector3D>&	I()			{return(*static_cast<CqShaderVariableVarying<Type_Vector,CqVector3D>*>(m_apVariables[EnvVars_I   ]));}
						/** Get a reference to the Ci standard variable.
						 */
			CqShaderVariableVarying<Type_Color,CqColor>&		Ci()		{return(*static_cast<CqShaderVariableVarying<Type_Color,CqColor>*>(m_apVariables[EnvVars_Ci    ]));}
						/** Get a reference to the Oi standard variable.
						 */
			CqShaderVariableVarying<Type_Color,CqColor>&		Oi()		{return(*static_cast<CqShaderVariableVarying<Type_Color,CqColor>*>(m_apVariables[EnvVars_Oi    ]));}
						/** Get a reference to the Ps standard variable.
						 */
			CqShaderVariableVarying<Type_Point,CqVector3D>&		Ps()		{return(*static_cast<CqShaderVariableVarying<Type_Point,CqVector3D>*>(m_apVariables[EnvVars_Ps  ]));}
						/** Get a reference to the E standard variable.
						 */
			CqShaderVariableUniform<Type_Point,CqVector3D>&		E()			{return(*static_cast<CqShaderVariableUniform<Type_Point,CqVector3D>*>(m_apVariables[EnvVars_E   ]));}
						/** Get a reference to the ncomps standard variable.
						 */
			CqShaderVariableUniform<Type_Float,TqFloat>&			ncomps()	{return(*static_cast<CqShaderVariableUniform<Type_Float,TqFloat>*>(m_apVariables[EnvVars_ncomps ]));}
						/** Get a reference to the time standard variable.
						 */
			CqShaderVariableUniform<Type_Float,TqFloat>&			time()		{return(*static_cast<CqShaderVariableUniform<Type_Float,TqFloat>*>(m_apVariables[EnvVars_time   ]));}
						/** Get a reference to the alpha standard variable.
						 */
			CqShaderVariableUniform<Type_Float,TqFloat>&			alpha()		{return(*static_cast<CqShaderVariableUniform<Type_Float,TqFloat>*>(m_apVariables[EnvVars_alpha  ]));}

	private:
						/** Internal function to extract additional named filter parameters from an array of stack entries.
						 */
			void	GetFilterParams(int cParams, CqVMStackEntry** apParams, float& _pswidth, float& _ptwidth)
					{
						static CqString temp_string;
						static float temp_float;

						int i=0;
						while(cParams>0)
						{
							CqString& strParam=apParams[i]->Value(temp_string,0);
							if(strParam.compare("width")==0)		_pswidth=_ptwidth=apParams[i+1]->Value(temp_float,0);
							else if(strParam.compare("swidth")==0)	_pswidth=apParams[i+1]->Value(temp_float,0);
							else if(strParam.compare("twidth")==0)	_ptwidth=apParams[i+1]->Value(temp_float,0);
							i+=2;
							cParams-=2;
						}
					}
						/** Internal function to extract additional named texture control parameters from an array of stack entries.
						 */
			void	GetTexParams(int cParams, CqVMStackEntry** apParams, float& _pswidth, float& _ptwidth, float& _psblur, float& _ptblur, float& _pfill)
					{
						static CqString temp_string;
						static float temp_float;

						int i=0;
						while(cParams>0)
						{
							CqString& strParam=apParams[i]->Value(temp_string,0);
							if(strParam.compare("width")==0)		_pswidth=_ptwidth=apParams[i+1]->Value(temp_float,0);
							else if(strParam.compare("swidth")==0)	_pswidth=apParams[i+1]->Value(temp_float,0);
							else if(strParam.compare("twidth")==0)	_ptwidth=apParams[i+1]->Value(temp_float,0);
							else if(strParam.compare("blur")==0)	_psblur=_ptblur=apParams[i+1]->Value(temp_float,0);
							else if(strParam.compare("sblur")==0)	_psblur=apParams[i+1]->Value(temp_float,0);
							else if(strParam.compare("tblur")==0)	_ptblur=apParams[i+1]->Value(temp_float,0);
							else if(strParam.compare("fill")==0)	_pfill=apParams[i+1]->Value(temp_float,0);
							i+=2;
							cParams-=2;
						}
					}

			std::vector<CqShaderVariable*>	m_apVariables;	///< Vector of pointers to shader variables.
			struct SqVarName
			{
				char*		m_strName;
				EqEnvVars	m_Index;
			};
	static	CqNoise			m_noise;		///< One off noise generator, used by all envs.
	static	CqCellNoise		m_cellnoise;	///< One off cell noise generator, used by all envs.
	static	CqRandom		m_random;		///< One off random number generator used by all envs.
	static	CqMatrix		m_matIdentity;
	static	char*			m_apVariableNames[];	///< Vector of variable names.

			TqInt					m_uGridRes;				///< The resolution of the grid in u.
			TqInt					m_vGridRes;				///< The resolution of the grid in u.
			TqInt					m_GridSize;				///< The resolution of the grid.
			TqInt					m_GridI;				///< The current SIMD index.
			TqInt					m_li;					///< Light index, used during illuminance loop.
			TqInt					m_Illuminate;
			TqBool					m_IlluminanceCacheValid;	///< Flag indicating whether the illuminance cache is valid.
			CqSurface*				m_pSurface;				///< Pointer to the associated surface.
			CqBitVector				m_CurrentState;			///< SIMD execution state bit vector accumulator.
			CqBitVector				m_RunningState;			///< SIMD running execution state bit vector.
			std::stack<CqBitVector>	m_stkState;				///< Stack of execution state bit vectors.
	
	protected:
			CqShaderVariableVarying<Type_Float,TqInt>	m_vfCulled;	///< Shader variable indicating whether the individual micropolys are culled.

	public:
			TqBool		SO_init_illuminance();
			TqBool		SO_advance_illuminance();

			// ShadeOps			
			STD_SO	SO_radians(FLOATVAL degrees, DEFPARAM);
			STD_SO	SO_degrees(FLOATVAL radians, DEFPARAM);
			STD_SO	SO_sin(FLOATVAL a, DEFPARAM);
			STD_SO	SO_asin(FLOATVAL a, DEFPARAM);
			STD_SO	SO_cos(FLOATVAL a, DEFPARAM);
			STD_SO	SO_acos(FLOATVAL a, DEFPARAM);
			STD_SO	SO_tan(FLOATVAL a, DEFPARAM);
			STD_SO	SO_atan(FLOATVAL yoverx, DEFPARAM);
			STD_SO	SO_atan(FLOATVAL y, FLOATVAL x, DEFPARAM);
			STD_SO	SO_pow(FLOATVAL x, FLOATVAL y, DEFPARAM);
			STD_SO	SO_exp(FLOATVAL x, DEFPARAM);
			STD_SO	SO_sqrt(FLOATVAL x, DEFPARAM);
			STD_SO	SO_log(FLOATVAL x, DEFPARAM);
			STD_SO	SO_log(FLOATVAL x, FLOATVAL base, DEFPARAM);
			STD_SO	SO_mod(FLOATVAL a, FLOATVAL b, DEFPARAM);
			STD_SO	SO_abs(FLOATVAL x, DEFPARAM);
			STD_SO	SO_sign(FLOATVAL x, DEFPARAM);
			STD_SO	SO_min(FLOATVAL a, FLOATVAL b, DEFPARAMVAR);
			STD_SO	SO_max(FLOATVAL a, FLOATVAL b, DEFPARAMVAR);
			STD_SO	SO_pmin(POINTVAL a, POINTVAL b, DEFPARAMVAR);
			STD_SO	SO_pmax(POINTVAL a, POINTVAL b, DEFPARAMVAR);
			STD_SO	SO_cmin(COLORVAL a, COLORVAL b, DEFPARAMVAR);
			STD_SO	SO_cmax(COLORVAL a, COLORVAL b, DEFPARAMVAR);
			STD_SO	SO_clamp(FLOATVAL a, FLOATVAL min, FLOATVAL max, DEFPARAM);
			STD_SO	SO_pclamp(POINTVAL a, POINTVAL min, POINTVAL max, DEFPARAM);
			STD_SO	SO_cclamp(COLORVAL a, COLORVAL min, COLORVAL max, DEFPARAM);
			STD_SO	SO_floor(FLOATVAL x, DEFPARAM);
			STD_SO	SO_ceil(FLOATVAL x, DEFPARAM);
			STD_SO	SO_round(FLOATVAL x, DEFPARAM);
			STD_SO	SO_step(FLOATVAL min, FLOATVAL value, DEFPARAM);
			STD_SO	SO_smoothstep(FLOATVAL min, FLOATVAL max, FLOATVAL value, DEFPARAM);
			STD_SO	SO_fspline(FLOATVAL value, DEFPARAMVAR);
			STD_SO	SO_cspline(FLOATVAL value, DEFPARAMVAR);
			STD_SO	SO_pspline(FLOATVAL value, DEFPARAMVAR);
			STD_SO	SO_sfspline(STRINGVAL basis, FLOATVAL value, DEFPARAMVAR);
			STD_SO	SO_scspline(STRINGVAL basis, FLOATVAL value, DEFPARAMVAR);
			STD_SO	SO_spspline(STRINGVAL basis, FLOATVAL value, DEFPARAMVAR);
			STD_SO	SO_fDu(FLOATVAL p, DEFPARAM);
			STD_SO	SO_fDv(FLOATVAL p, DEFPARAM);
			STD_SO	SO_fDeriv(FLOATVAL p, FLOATVAL den, DEFPARAM);
			STD_SO	SO_cDu(COLORVAL p, DEFPARAM);
			STD_SO	SO_cDv(COLORVAL p, DEFPARAM);
			STD_SO	SO_cDeriv(COLORVAL p, FLOATVAL den, DEFPARAM);
			STD_SO	SO_pDu(POINTVAL p, DEFPARAM);
			STD_SO	SO_pDv(POINTVAL p, DEFPARAM);
			STD_SO	SO_pDeriv(POINTVAL p, FLOATVAL den, DEFPARAM);
			STD_SO	SO_frandom(DEFPARAM);
			STD_SO	SO_crandom(DEFPARAM);
			STD_SO	SO_prandom(DEFPARAM);
			STD_SO	SO_fnoise1(FLOATVAL v, DEFPARAM);
			STD_SO	SO_fnoise2(FLOATVAL u, FLOATVAL v, DEFPARAM);
			STD_SO	SO_fnoise3(POINTVAL p, DEFPARAM);
			STD_SO	SO_fnoise4(POINTVAL p, FLOATVAL t, DEFPARAM);
			STD_SO	SO_cnoise1(FLOATVAL v, DEFPARAM);
			STD_SO	SO_cnoise2(FLOATVAL u, FLOATVAL v, DEFPARAM);
			STD_SO	SO_cnoise3(POINTVAL p, DEFPARAM);
			STD_SO	SO_cnoise4(POINTVAL p, FLOATVAL t, DEFPARAM);
			STD_SO	SO_pnoise1(FLOATVAL v, DEFPARAM);
			STD_SO	SO_pnoise2(FLOATVAL u, FLOATVAL v, DEFPARAM);
			STD_SO	SO_pnoise3(POINTVAL p, DEFPARAM);
			STD_SO	SO_pnoise4(POINTVAL p, FLOATVAL t, DEFPARAM);
			STD_SO	SO_setcomp(COLORVAL p, FLOATVAL i, FLOATVAL v, DEFVOIDPARAM);
			STD_SO	SO_setxcomp(POINTVAL p, FLOATVAL v, DEFVOIDPARAM);
			STD_SO	SO_setycomp(POINTVAL p, FLOATVAL v, DEFVOIDPARAM);
			STD_SO	SO_setzcomp(POINTVAL p, FLOATVAL v, DEFVOIDPARAM);
			STD_SO	SO_length(VECTORVAL V, DEFPARAM);
			STD_SO	SO_distance(POINTVAL P1, POINTVAL P2, DEFPARAM);
			STD_SO	SO_area(POINTVAL p, DEFPARAM);
			STD_SO	SO_normalize(VECTORVAL V, DEFPARAM);
			STD_SO	SO_faceforward(NORMALVAL N, VECTORVAL I /* [Nref] */, DEFPARAM);
			STD_SO	SO_reflect(VECTORVAL I, NORMALVAL N, DEFPARAM);
			STD_SO	SO_refract(VECTORVAL I, NORMALVAL N, FLOATVAL eta, DEFPARAM);
			STD_SO	SO_fresnel(VECTORVAL I, NORMALVAL N, FLOATVAL eta, FLOATVAL Kr, FLOATVAL Kt, DEFVOIDPARAM);
			STD_SO	SO_fresnel(VECTORVAL I, NORMALVAL N, FLOATVAL eta, FLOATVAL Kr, FLOATVAL Kt, VECTORVAL R, VECTORVAL T, DEFVOIDPARAM);
			STD_SO	SO_transform(STRINGVAL fromspace, STRINGVAL tospace, POINTVAL p, DEFPARAM);
			STD_SO	SO_transform(STRINGVAL tospace, POINTVAL p, DEFPARAM);
			STD_SO	SO_transformm(MATRIXVAL tospace, POINTVAL p, DEFPARAM);
			STD_SO	SO_vtransform(STRINGVAL fromspace, STRINGVAL tospace, VECTORVAL p, DEFPARAM);
			STD_SO	SO_vtransform(STRINGVAL tospace, VECTORVAL p, DEFPARAM);
			STD_SO	SO_vtransformm(MATRIXVAL tospace, VECTORVAL p, DEFPARAM);
			STD_SO	SO_ntransform(STRINGVAL fromspace, STRINGVAL tospace, NORMALVAL p, DEFPARAM);
			STD_SO	SO_ntransform(STRINGVAL tospace, NORMALVAL p, DEFPARAM);
			STD_SO	SO_ntransformm(MATRIXVAL tospace, NORMALVAL p, DEFPARAM);
			STD_SO	SO_depth(POINTVAL p, DEFPARAM);
			STD_SO	SO_calculatenormal(POINTVAL p, DEFPARAM);
			STD_SO	SO_cmix(COLORVAL color0, COLORVAL color1, FLOATVAL value, DEFPARAM);
			STD_SO	SO_fmix(FLOATVAL f0, FLOATVAL f1, FLOATVAL value, DEFPARAM);
			STD_SO	SO_pmix(POINTVAL p0, POINTVAL p1, FLOATVAL value, DEFPARAM);
			STD_SO	SO_vmix(VECTORVAL v0, VECTORVAL v1, FLOATVAL value, DEFPARAM);
			STD_SO	SO_nmix(NORMALVAL n0, NORMALVAL n1, FLOATVAL value, DEFPARAM);
			STD_SO	SO_ambient(DEFPARAM);
			STD_SO	SO_diffuse(NORMALVAL N, DEFPARAM);
			STD_SO	SO_specular(NORMALVAL N, VECTORVAL V, FLOATVAL roughness, DEFPARAM);
			STD_SO	SO_phong(NORMALVAL N, VECTORVAL V, FLOATVAL size, DEFPARAM);
			STD_SO	SO_trace(POINTVAL P, VECTORVAL R, DEFPARAM);
			STD_SO	SO_ftexture1(STRINGVAL name, FLOATVAL channel, DEFPARAMVAR);
			STD_SO	SO_ftexture2(STRINGVAL name, FLOATVAL channel, FLOATVAL s, FLOATVAL t, DEFPARAMVAR);
			STD_SO	SO_ftexture3(STRINGVAL name, FLOATVAL channel, FLOATVAL s1, FLOATVAL t1, FLOATVAL s2, FLOATVAL t2, FLOATVAL s3, FLOATVAL t3, FLOATVAL s4, FLOATVAL t4, DEFPARAMVAR);
			STD_SO	SO_ctexture1(STRINGVAL name, FLOATVAL channel, DEFPARAMVAR);
			STD_SO	SO_ctexture2(STRINGVAL name, FLOATVAL channel, FLOATVAL s, FLOATVAL t, DEFPARAMVAR);
			STD_SO	SO_ctexture3(STRINGVAL name, FLOATVAL channel, FLOATVAL s1, FLOATVAL t1, FLOATVAL s2, FLOATVAL t2, FLOATVAL s3, FLOATVAL t3, FLOATVAL s4, FLOATVAL t4, DEFPARAMVAR);
			STD_SO	SO_fenvironment2(STRINGVAL name, FLOATVAL channel, VECTORVAL R, DEFPARAMVAR);
			STD_SO	SO_fenvironment3(STRINGVAL name, FLOATVAL channel, VECTORVAL R1, VECTORVAL R2, VECTORVAL R3, VECTORVAL R4, DEFPARAMVAR);
			STD_SO	SO_cenvironment2(STRINGVAL name, FLOATVAL channel, VECTORVAL R, DEFPARAMVAR);
			STD_SO	SO_cenvironment3(STRINGVAL name, FLOATVAL channel, VECTORVAL R1, VECTORVAL R2, VECTORVAL R3, VECTORVAL R4, DEFPARAMVAR);
			STD_SO	SO_bump1(STRINGVAL name, FLOATVAL channel, DEFPARAMVAR);
			STD_SO	SO_bump2(STRINGVAL name, FLOATVAL channel, FLOATVAL s, FLOATVAL t, DEFPARAMVAR);
			STD_SO	SO_bump3(STRINGVAL name, FLOATVAL channel, FLOATVAL s1, FLOATVAL t1, FLOATVAL s2, FLOATVAL t2, FLOATVAL s3, FLOATVAL t3, FLOATVAL s4, FLOATVAL t4, DEFPARAMVAR);
			STD_SO	SO_shadow(STRINGVAL name, FLOATVAL channel, POINTVAL P, DEFPARAMVAR);
			STD_SO	SO_shadow1(STRINGVAL name, FLOATVAL channel, POINTVAL P1, POINTVAL P2, POINTVAL P3, POINTVAL P4, DEFPARAMVAR);
			STD_SO	SO_illuminance(POINTVAL P, FLOATVAL nsamples, DEFVOIDPARAM);
			STD_SO	SO_illuminance(POINTVAL P, VECTORVAL Axis, FLOATVAL Angle, FLOATVAL nsamples, DEFVOIDPARAM);
			STD_SO	SO_illuminate(POINTVAL P, VECTORVAL Axis, FLOATVAL Angle, DEFVOIDPARAM);
			STD_SO	SO_illuminate(POINTVAL P, DEFVOIDPARAM);
			STD_SO	SO_solar(VECTORVAL Axis, FLOATVAL Angle, DEFVOIDPARAM);
			STD_SO	SO_solar(DEFVOIDPARAM);
			STD_SO	SO_printf(STRINGVAL str, DEFVOIDPARAMVAR);
			STD_SO	SO_format(STRINGVAL str, DEFPARAMVAR);
			STD_SO	SO_concat(STRINGVAL stra, STRINGVAL strb, DEFPARAMVAR);
			STD_SO	SO_atmosphere(STRINGVAL name, CqShaderVariable* pV, DEFPARAM);
			STD_SO	SO_displacement(STRINGVAL name, CqShaderVariable* pV, DEFPARAM);
			STD_SO	SO_lightsource(STRINGVAL name, CqShaderVariable* pV, DEFPARAM);
			STD_SO	SO_surface(STRINGVAL name, CqShaderVariable* pV, DEFPARAM);
			STD_SO	SO_attribute(STRINGVAL name, CqShaderVariable* pV, DEFPARAM);
			STD_SO	SO_option(STRINGVAL name, CqShaderVariable* pV, DEFPARAM);
			STD_SO	SO_rendererinfo(STRINGVAL name, CqShaderVariable* pV, DEFPARAM);
			STD_SO	SO_incident(STRINGVAL name, CqShaderVariable* pV, DEFPARAM);
			STD_SO	SO_opposite(STRINGVAL name, CqShaderVariable* pV, DEFPARAM);
			STD_SO	SO_fcellnoise1(FLOATVAL v, DEFPARAM);
			STD_SO	SO_fcellnoise2(FLOATVAL u, FLOATVAL v, DEFPARAM);
			STD_SO	SO_fcellnoise3(POINTVAL p, DEFPARAM);
			STD_SO	SO_fcellnoise4(POINTVAL p, FLOATVAL v, DEFPARAM);
			STD_SO	SO_ccellnoise1(FLOATVAL v, DEFPARAM);
			STD_SO	SO_ccellnoise2(FLOATVAL u, FLOATVAL v, DEFPARAM);
			STD_SO	SO_ccellnoise3(POINTVAL p, DEFPARAM);
			STD_SO	SO_ccellnoise4(POINTVAL p, FLOATVAL v, DEFPARAM);
			STD_SO	SO_pcellnoise1(FLOATVAL v, DEFPARAM);
			STD_SO	SO_pcellnoise2(FLOATVAL u, FLOATVAL v, DEFPARAM);
			STD_SO	SO_pcellnoise3(POINTVAL p, DEFPARAM);
			STD_SO	SO_pcellnoise4(POINTVAL p, FLOATVAL v, DEFPARAM);
			STD_SO	SO_fpnoise1(FLOATVAL v, FLOATVAL period, DEFPARAM);
			STD_SO	SO_fpnoise2(FLOATVAL u, FLOATVAL v, FLOATVAL uperiod, FLOATVAL vperiod, DEFPARAM);
			STD_SO	SO_fpnoise3(POINTVAL p, POINTVAL pperiod, DEFPARAM);
			STD_SO	SO_fpnoise4(POINTVAL p, FLOATVAL t, POINTVAL pperiod, FLOATVAL tperiod, DEFPARAM);
			STD_SO	SO_cpnoise1(FLOATVAL v, FLOATVAL period, DEFPARAM);
			STD_SO	SO_cpnoise2(FLOATVAL u, FLOATVAL v, FLOATVAL uperiod, FLOATVAL vperiod, DEFPARAM);
			STD_SO	SO_cpnoise3(POINTVAL p, POINTVAL pperiod, DEFPARAM);
			STD_SO	SO_cpnoise4(POINTVAL p, FLOATVAL t, POINTVAL pperiod, FLOATVAL tperiod, DEFPARAM);
			STD_SO	SO_ppnoise1(FLOATVAL v, FLOATVAL period, DEFPARAM);
			STD_SO	SO_ppnoise2(FLOATVAL u, FLOATVAL v, FLOATVAL uperiod, FLOATVAL vperiod, DEFPARAM);
			STD_SO	SO_ppnoise3(POINTVAL p, POINTVAL pperiod, DEFPARAM);
			STD_SO	SO_ppnoise4(POINTVAL p, FLOATVAL t, POINTVAL pperiod, FLOATVAL tperiod, DEFPARAM);
			STD_SO	SO_ctransform(STRINGVAL fromspace, STRINGVAL tospace, COLORVAL c, DEFPARAM);
			STD_SO	SO_ctransform(STRINGVAL tospace, COLORVAL c, DEFPARAM);
			STD_SO	SO_ptlined(POINTVAL P0, POINTVAL P1, POINTVAL Q, DEFPARAM);
			STD_SO	SO_inversesqrt(FLOATVAL x, DEFPARAM);
			STD_SO	SO_match(STRINGVAL a, STRINGVAL b, DEFPARAM);
			STD_SO	SO_rotate(VECTORVAL Q, FLOATVAL angle, POINTVAL P0, POINTVAL P1, DEFPARAM);
			STD_SO	SO_filterstep(FLOATVAL edge, FLOATVAL s1, DEFPARAMVAR);
			STD_SO	SO_filterstep2(FLOATVAL edge, FLOATVAL s1, FLOATVAL s2, DEFPARAMVAR);
			STD_SO	SO_specularbrdf(VECTORVAL L, NORMALVAL N, VECTORVAL V, FLOATVAL rough, DEFPARAM);
			STD_SO	SO_setmcomp(MATRIXVAL M, FLOATVAL row, FLOATVAL column, FLOATVAL val, DEFVOIDPARAM);
			STD_SO	SO_determinant(MATRIXVAL M, DEFPARAM);
			STD_SO	SO_mtranslate(MATRIXVAL M, VECTORVAL V, DEFPARAM);
			STD_SO	SO_mrotate(MATRIXVAL M, FLOATVAL angle, VECTORVAL axis, DEFPARAM);
			STD_SO	SO_mscale(MATRIXVAL M, POINTVAL s, DEFPARAM);
			STD_SO	SO_fsplinea(FLOATVAL value, FLOATARRAYVAL a, DEFPARAM);
			STD_SO	SO_csplinea(FLOATVAL value, COLORARRAYVAL a, DEFPARAM);
			STD_SO	SO_psplinea(FLOATVAL value, POINTARRAYVAL a, DEFPARAM);
			STD_SO	SO_sfsplinea(STRINGVAL basis, FLOATVAL value, FLOATARRAYVAL a, DEFPARAM);
			STD_SO	SO_scsplinea(STRINGVAL basis, FLOATVAL value, COLORARRAYVAL a, DEFPARAM);
			STD_SO	SO_spsplinea(STRINGVAL basis, FLOATVAL value, POINTARRAYVAL a, DEFPARAM);
			STD_SO	SO_shadername(DEFPARAM);
			STD_SO	SO_shadername2(STRINGVAL shader, DEFPARAM);
};

/** Templatised derivative function. Calculates the derivative of the provided shader variable with respect to u.
 */
template<class R>
R SO_DuType(CqShaderVariableTyped<R>& Var, TqInt i, CqShaderExecEnv& s)
{
	R Ret;
	TqInt uRes=s.uGridRes();
	TqInt GridX=i%(uRes+1);
	if(GridX<=(uRes-1))
		Ret=(static_cast<R>(Var[i+1])-static_cast<R>(Var[i]))/s.du()[i];
	else
	{
		if(GridX>1)
		{
			R Tm1=static_cast<R>(Var[i-1]);
			R Tm2=static_cast<R>(Var[i-2]);
			R dTm1=(static_cast<R>(Var[i])-static_cast<R>(Tm1))/s.du()[i];
			R dTm2=(Tm1-Tm2)/s.du()[i];
			Ret=2*dTm1-dTm2;
		}
		else
		{
			R Tm1=static_cast<R>(Var[i-1]);
			Ret=(static_cast<R>(Var[i])-Tm1)/s.du()[i];
		}
	}
	return(Ret);
}


/** Templatised derivative function. Calculates the derivative of the provided shader variable with respect to v.
 */
template<class R>
R SO_DvType(CqShaderVariableTyped<R>& Var, TqInt i, CqShaderExecEnv& s)
{
	R Ret;
	TqInt uRes=s.uGridRes();
	TqInt vRes=s.vGridRes();
	TqInt GridY=(i/(uRes+1));
	if(GridY<=(vRes-1))
		Ret=(static_cast<R>(Var[i+uRes+1])-static_cast<R>(Var[i]))/s.dv()[i];
	else
	{
		if(GridY>1)
		{
			R Tm1=static_cast<R>(Var[i-(uRes+1)]);
			R Tm2=static_cast<R>(Var[i-(2*(uRes+1))]);
			R dTm1=(static_cast<R>(Var[i])-Tm1)/s.dv()[i];
			R dTm2=(Tm1-Tm2)/s.dv()[i];
			Ret=2*dTm1-dTm2;
		}
		else
		{
			R Tm1=static_cast<R>(Var[i-(uRes+1)]);
			Ret=(static_cast<R>(Var[i])-Tm1)/s.dv()[i];
		}
	}
	return(Ret);
}


/** Templatised derivative function. Calculates the derivative of the provided shader variable with respect to a second variable.
 */
template<class R>
R SO_DerivType(CqShaderVariableTyped<R>& Var, CqShaderVariableTyped<TqFloat>& den, TqInt i, CqShaderExecEnv& s)
{
	R Retu,Retv;
	TqInt uRes=s.uGridRes();
	TqInt vRes=s.vGridRes();
	TqInt GridX=i%(uRes+1);
	TqInt GridY=(i/(uRes+1));

	// Calculate deriviative in u
	if(GridX<=uRes)
		Retu=(static_cast<R>(Var[i+1])-static_cast<R>(Var[i]))/static_cast<TqFloat>(den[i]);
	else
	{
		if(GridX>1)
		{
			R Tm1=static_cast<R>(Var[i-1]);
			R Tm2=static_cast<R>(Var[i-2]);
			R dTm1=(static_cast<R>(Var[i])-static_cast<R>(Tm1))/static_cast<TqFloat>(den[i]);
			R dTm2=(Tm1-Tm2)/static_cast<TqFloat>(den[i]);
			Retu=2*dTm1-dTm2;
		}
		else
		{
			R Tm1=static_cast<R>(Var[i-1]);
			Retu=(static_cast<R>(Var[i])-Tm1)/static_cast<TqFloat>(den[i]);
		}
	}

	// Calculate deriviative in v
	if(GridY<=(vRes-1))
		Retv=(static_cast<R>(Var[i+uRes+1])-static_cast<R>(Var[i]))/static_cast<TqFloat>(den[i]);
	else
	{
		if(GridY>1)
		{
			R Tm1=static_cast<R>(Var[i-(uRes+1)]);
			R Tm2=static_cast<R>(Var[i-(2*(uRes+1))]);
			R dTm1=(static_cast<R>(Var[i])-Tm1)/static_cast<TqFloat>(den[i]);
			R dTm2=(Tm1-Tm2)/static_cast<TqFloat>(den[i]);
			Retv=2*dTm1-dTm2;
		}
		else
		{
			R Tm1=static_cast<R>(Var[i-(uRes+1)]);
			Retv=(static_cast<R>(Var[i])-Tm1)/static_cast<TqFloat>(den[i]);
		}
	}

	return(Retu+Retv);
}


/** Templatised derivative function. Calculates the derivative of the provided stack entry with respect to u.
 */
template<class R>
R SO_DuType(CqVMStackEntry& Var, TqInt i, CqShaderExecEnv& s)
{
	R Ret;
	TqInt uRes=s.uGridRes();
	TqInt GridX=i%(uRes+1);
	if(GridX<uRes)
		Ret=(Var.Value(Ret,i+1)-Var.Value(Ret,i))/s.du()[i];
	else
	{
		if(GridX>1)
		{
			R Tm1=Var.Value(Ret,i-1);
			R Tm2=Var.Value(Ret,i-2);
			R dTm1=(Var.Value(Ret,i)-Tm1)/s.du()[i];
			R dTm2=(Tm1-Tm2)/s.du()[i];
			Ret=2*dTm1-dTm2;
		}
		else
		{
			R Tm1=Var.Value(Ret,i-1);
			Ret=(Var.Value(Ret,i)-Tm1)/s.du()[i];
		}
	}
	return(Ret);
}


/** Templatised derivative function. Calculates the derivative of the provided stack entry with respect to v.
 */
template<class R>
R SO_DvType(CqVMStackEntry& Var, TqInt i, CqShaderExecEnv& s)
{
	R Ret;
	TqInt uRes=s.uGridRes();
	TqInt vRes=s.vGridRes();
	TqInt GridY=(i/(uRes+1));
	if(GridY<(vRes-1))
		Ret=(Var.Value(Ret,i+uRes+1)-Var.Value(Ret,i))/s.dv()[i];
	else
	{
		if(GridY>1)
		{
			R Tm1=Var.Value(Ret,i-(uRes+1));
			R Tm2=Var.Value(Ret,i-(2*(uRes+1)));
			R dTm1=(Var.Value(Ret,i)-Tm1)/s.dv()[i];
			R dTm2=(Tm1-Tm2)/s.dv()[i];
			Ret=2*dTm1-dTm2;
		}
		else
		{
			R Tm1=Var.Value(Ret,i-(uRes+1));
			Ret=(Var.Value(Ret,i)-Tm1)/s.dv()[i];
		}
	}
	return(Ret);
}


/** Templatised derivative function. Calculates the derivative of the provided stack entry with respect to a second stack entry.
 */
template<class R>
R SO_DerivType(CqVMStackEntry& Var, CqVMStackEntry& den, TqInt i, CqShaderExecEnv& s)
{
	TqFloat f;
	R Retu,Retv;
	TqInt uRes=s.uGridRes();
	TqInt vRes=s.vGridRes();
	TqInt GridX=i%(uRes+1);
	TqInt GridY=(i/(uRes+1));

	// Calculate deriviative in u
	if(GridX<uRes)
		Retu=(Var.Value(Retu,i+1)-Var.Value(Retu,i))/den.Value(f,i);
	else
	{
		if(GridX>1)
		{
			R Tm1=Var.Value(Retu,i-1);
			R Tm2=Var.Value(Retu,i-2);
			R dTm1=(Var.Value(Retu,i)-Tm1)/den.Value(f,i);
			R dTm2=(Tm1-Tm2)/den.Value(f,i);
			Retu=2*dTm1-dTm2;
		}
		else
		{
			R Tm1=Var.Value(Retu,i-1);
			Retu=(Var.Value(Retu,i)-Tm1)/den.Value(f,i);
		}
	}

	// Calculate deriviative in v
	if(GridY<(vRes-1))
		Retv=(Var.Value(Retu,i+uRes+1)-Var.Value(Retu,i))/den.Value(f,i);
	else
	{
		if(GridY>1)
		{
			R Tm1=Var.Value(Retu,i-(uRes+1));
			R Tm2=Var.Value(Retu,i-(2*(uRes+1)));
			R dTm1=(Var.Value(Retu,i)-Tm1)/den.Value(f,i);
			R dTm2=(Tm1-Tm2)/den.Value(f,i);
			Retv=2*dTm1-dTm2;
		}
		else
		{
			R Tm1=Var.Value(Retu,i-(uRes+1));
			Retv=(Var.Value(Retu,i)-Tm1)/den.Value(f,i);
		}
	}

	return(Retu+Retv);
}


//-----------------------------------------------------------------------

END_NAMESPACE(Aqsis)

#endif	// !SHADEREXECENV_H_INCLUDED
