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
		\brief Implements functions for the shader virtual machine.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#include "shadervm.h"

#include <iostream>

#include "shadeopmacros.h"


namespace Aqsis {

DECLARE_SHADERSTACK_TEMPS

void CqShaderVM::SO_land()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		OpLAND_B( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
	RELEASE( B );
	RELEASE( A );
}

void CqShaderVM::SO_lor()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		OpLOR_B( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
	RELEASE( B );
	RELEASE( A );
}

void CqShaderVM::SO_exp()
{
	AUTOFUNC;
	FUNC1( type_float, m_pEnv->SO_exp );
}

void CqShaderVM::SO_sqrt()
{
	AUTOFUNC;
	FUNC1( type_float, m_pEnv->SO_sqrt );
}

void CqShaderVM::SO_log()
{
	AUTOFUNC;
	FUNC1( type_float, m_pEnv->SO_log );
}

void CqShaderVM::SO_log2()
{
	AUTOFUNC;
	FUNC2( type_float, m_pEnv->SO_log );
}

void CqShaderVM::SO_mod()
{
	AUTOFUNC;
	FUNC2( type_float, m_pEnv->SO_mod );
}

void CqShaderVM::SO_abs()
{
	AUTOFUNC;
	FUNC1( type_float, m_pEnv->SO_abs );
}

void CqShaderVM::SO_sign()
{
	AUTOFUNC;
	FUNC1( type_float, m_pEnv->SO_sign );
}

void CqShaderVM::SO_min()
{
	AUTOFUNC;
	FUNC2PLUS( type_float, m_pEnv->SO_min );
}

void CqShaderVM::SO_max()
{
	AUTOFUNC;
	FUNC2PLUS( type_float, m_pEnv->SO_max );
}

void CqShaderVM::SO_pmin()
{
	AUTOFUNC;
	FUNC2PLUS( type_point, m_pEnv->SO_pmin );
}

void CqShaderVM::SO_pmax()
{
	AUTOFUNC;
	FUNC2PLUS( type_point, m_pEnv->SO_pmax );
}

void CqShaderVM::SO_vmin()
{
	AUTOFUNC;
	FUNC2PLUS( type_point, m_pEnv->SO_pmin );
}

void CqShaderVM::SO_vmax()
{
	AUTOFUNC;
	FUNC2PLUS( type_point, m_pEnv->SO_pmax );
}

void CqShaderVM::SO_nmin()
{
	AUTOFUNC;
	FUNC2PLUS( type_point, m_pEnv->SO_pmin );
}

void CqShaderVM::SO_nmax()
{
	AUTOFUNC;
	FUNC2PLUS( type_point, m_pEnv->SO_pmax );
}

void CqShaderVM::SO_cmin()
{
	AUTOFUNC;
	FUNC2PLUS( type_color, m_pEnv->SO_cmin );
}

void CqShaderVM::SO_cmax()
{
	AUTOFUNC;
	FUNC2PLUS( type_color, m_pEnv->SO_cmax );
}

void CqShaderVM::SO_clamp()
{
	AUTOFUNC;
	FUNC3( type_float, m_pEnv->SO_clamp );
}

void CqShaderVM::SO_pclamp()
{
	AUTOFUNC;
	FUNC3( type_point, m_pEnv->SO_pclamp );
}

void CqShaderVM::SO_cclamp()
{
	AUTOFUNC;
	FUNC3( type_color, m_pEnv->SO_cclamp );
}

void CqShaderVM::SO_floor()
{
	AUTOFUNC;
	FUNC1( type_float, m_pEnv->SO_floor );
}

void CqShaderVM::SO_ceil()
{
	AUTOFUNC;
	FUNC1( type_float, m_pEnv->SO_ceil );
}

void CqShaderVM::SO_round()
{
	AUTOFUNC;
	FUNC1( type_float, m_pEnv->SO_round );
}

void CqShaderVM::SO_step()
{
	AUTOFUNC;
	FUNC2( type_float, m_pEnv->SO_step );
}

void CqShaderVM::SO_smoothstep()
{
	AUTOFUNC;
	FUNC3( type_float, m_pEnv->SO_smoothstep );
}


// Special macros for declaring the spline shadeops.
#define	SPLINE(t,func)	POPV(count);	\
						POPV(value); \
						POPV(vala);	\
						POPV(valb);	\
						POPV(valc);	\
						POPV(vald);	\
						TqFloat fc; \
						count->GetFloat( fc ); \
						TqInt cParams=static_cast<TqInt>( fc ) + 4; \
						IqShaderData** apSplinePoints=new IqShaderData*[cParams]; \
						SqStackEntry *stackitems = new SqStackEntry[cParams];\
						apSplinePoints[0]=vala; \
						apSplinePoints[1]=valb; \
						apSplinePoints[2]=valc; \
						apSplinePoints[3]=vald; \
						TqInt iSP; \
						for(iSP=4; iSP<cParams; iSP++) {\
							stackitems[iSP] = POP; \
							apSplinePoints[iSP]=stackitems[iSP].m_Data; \
						}\
						RESULT(t,__fVarying?class_varying:class_uniform); \
						if(m_pEnv->IsRunning()) \
							func(value,pResult,this,cParams,apSplinePoints); \
						delete[](apSplinePoints); \
                  				for(iSP=4; iSP<cParams; iSP++) {\
							Release( stackitems[iSP]);\
						}\
						delete[](stackitems); \
						Push(pResult); \
						RELEASE(count); \
						RELEASE(value); \
						RELEASE(vala); \
						RELEASE(valb); \
						RELEASE(valc); \
						RELEASE(vald);
#define	SSPLINE(t,func)	POPV(count); \
						POPV(basis); \
						POPV(value); \
						POPV(vala);	\
						POPV(valb);	\
						POPV(valc);	\
						POPV(vald);	\
						TqFloat fc; \
						count->GetFloat( fc ); \
						TqInt cParams=static_cast<TqInt>( fc ) + 4; \
						IqShaderData** apSplinePoints=new IqShaderData*[cParams]; \
						SqStackEntry *stackitems = new SqStackEntry[cParams];\
						apSplinePoints[0]=vala; \
						apSplinePoints[1]=valb; \
						apSplinePoints[2]=valc; \
						apSplinePoints[3]=vald; \
						TqInt iSP; \
						for(iSP=4; iSP<cParams; iSP++) {\
							stackitems[iSP] = POP; \
							apSplinePoints[iSP]=stackitems[iSP].m_Data; \
						}\
						RESULT(t,__fVarying?class_varying:class_uniform); \
						if(m_pEnv->IsRunning()) \
							func(basis,value,pResult,this,cParams,apSplinePoints); \
						delete[](apSplinePoints); \
						for(iSP=4; iSP<cParams; iSP++) {\
							Release( stackitems[iSP]);\
						}\
						delete[](stackitems); \
						Push(pResult); \
						RELEASE(count); \
						RELEASE(basis); \
						RELEASE(value); \
						RELEASE(vala); \
						RELEASE(valb); \
						RELEASE(valc); \
						RELEASE(vald);

void CqShaderVM::SO_fspline()
{
	AUTOFUNC;
	SPLINE( type_float, m_pEnv->SO_fspline );
}

void CqShaderVM::SO_cspline()
{
	AUTOFUNC;
	SPLINE( type_color, m_pEnv->SO_cspline );
}

void CqShaderVM::SO_pspline()
{
	AUTOFUNC;
	SPLINE( type_point, m_pEnv->SO_pspline );
}

void CqShaderVM::SO_sfspline()
{
	AUTOFUNC;
	SSPLINE( type_float, m_pEnv->SO_sfspline );
}

void CqShaderVM::SO_scspline()
{
	AUTOFUNC;
	SSPLINE( type_color, m_pEnv->SO_scspline );
}

void CqShaderVM::SO_spspline()
{
	AUTOFUNC;
	SSPLINE( type_point, m_pEnv->SO_spspline );
}

void CqShaderVM::SO_fDu()
{
	AUTOFUNC;
	FUNC1( type_float, m_pEnv->SO_fDu );
}

void CqShaderVM::SO_fDv()
{
	AUTOFUNC;
	FUNC1( type_float, m_pEnv->SO_fDv );
}

void CqShaderVM::SO_fDeriv()
{
	AUTOFUNC;
	FUNC2( type_float, m_pEnv->SO_fDeriv );
}

void CqShaderVM::SO_cDu()
{
	AUTOFUNC;
	FUNC1( type_color, m_pEnv->SO_cDu );
}

void CqShaderVM::SO_cDv()
{
	AUTOFUNC;
	FUNC1( type_color, m_pEnv->SO_cDv );
}

void CqShaderVM::SO_cDeriv()
{
	AUTOFUNC;
	FUNC2( type_color, m_pEnv->SO_cDeriv );
}

void CqShaderVM::SO_pDu()
{
	AUTOFUNC;
	FUNC1( type_point, m_pEnv->SO_pDu );
}

void CqShaderVM::SO_pDv()
{
	AUTOFUNC;
	FUNC1( type_point, m_pEnv->SO_pDv );
}

void CqShaderVM::SO_pDeriv()
{
	AUTOFUNC;
	FUNC2( type_point, m_pEnv->SO_pDeriv );
}

void CqShaderVM::SO_frandom()
{
	VARFUNC;
	FUNC( type_float, m_pEnv->SO_frandom );
}

void CqShaderVM::SO_crandom()
{
	VARFUNC;
	FUNC( type_color, m_pEnv->SO_crandom );
}

void CqShaderVM::SO_prandom()
{
	VARFUNC;
	FUNC( type_point, m_pEnv->SO_prandom );
}

void CqShaderVM::SO_noise1()
{
	AUTOFUNC;
	FUNC1( type_float, m_pEnv->SO_fnoise1 );
}

void CqShaderVM::SO_noise2()
{
	AUTOFUNC;
	FUNC2( type_float, m_pEnv->SO_fnoise2 );
}

void CqShaderVM::SO_noise3()
{
	AUTOFUNC;
	FUNC1( type_float, m_pEnv->SO_fnoise3 );
}

void CqShaderVM::SO_noise4()
{
	AUTOFUNC;
	FUNC2( type_float, m_pEnv->SO_fnoise4 );
}

void CqShaderVM::SO_cnoise1()
{
	AUTOFUNC;
	FUNC1( type_color, m_pEnv->SO_cnoise1 );
}

void CqShaderVM::SO_cnoise2()
{
	AUTOFUNC;
	FUNC2( type_color, m_pEnv->SO_cnoise2 );
}

void CqShaderVM::SO_cnoise3()
{
	AUTOFUNC;
	FUNC1( type_color, m_pEnv->SO_cnoise3 );
}

void CqShaderVM::SO_cnoise4()
{
	AUTOFUNC;
	FUNC2( type_color, m_pEnv->SO_cnoise4 );
}

void CqShaderVM::SO_pnoise1()
{
	AUTOFUNC;
	FUNC1( type_point, m_pEnv->SO_pnoise1 );
}

void CqShaderVM::SO_pnoise2()
{
	AUTOFUNC;
	FUNC2( type_point, m_pEnv->SO_pnoise2 );
}

void CqShaderVM::SO_pnoise3()
{
	AUTOFUNC;
	FUNC1( type_point, m_pEnv->SO_pnoise3 );
}

void CqShaderVM::SO_pnoise4()
{
	AUTOFUNC;
	FUNC2( type_point, m_pEnv->SO_pnoise4 );
}

void CqShaderVM::SO_xcomp()
{
	AUTOFUNC;
	POPV( A );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		OpCOMP_P( A, 0, pResult, m_pEnv->RunningState() );
	Push( pResult );
	RELEASE( A );
}

void CqShaderVM::SO_ycomp()
{
	AUTOFUNC;
	POPV( A );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		OpCOMP_P( A, 1, pResult, m_pEnv->RunningState() );
	Push( pResult );
	RELEASE( A );
}

void CqShaderVM::SO_zcomp()
{
	AUTOFUNC;
	POPV( A );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		OpCOMP_P( A, 2, pResult, m_pEnv->RunningState() );
	Push( pResult );
	RELEASE( A );
}

void CqShaderVM::SO_setxcomp()
{
	AUTOFUNC;
	VOIDFUNC2( m_pEnv->SO_setxcomp );
}

void CqShaderVM::SO_setycomp()
{
	AUTOFUNC;
	VOIDFUNC2( m_pEnv->SO_setycomp );
}

void CqShaderVM::SO_setzcomp()
{
	AUTOFUNC;
	VOIDFUNC2( m_pEnv->SO_setzcomp );
}

void CqShaderVM::SO_length()
{
	AUTOFUNC;
	FUNC1( type_float, m_pEnv->SO_length );
}

void CqShaderVM::SO_distance()
{
	AUTOFUNC;
	FUNC2( type_float, m_pEnv->SO_distance );
}

void CqShaderVM::SO_area()
{
	AUTOFUNC;
	FUNC1( type_float, m_pEnv->SO_area );
}

void CqShaderVM::SO_normalize()
{
	AUTOFUNC;
	FUNC1( type_vector, m_pEnv->SO_normalize );
}

void CqShaderVM::SO_faceforward()
{
	AUTOFUNC;
	FUNC2( type_vector, m_pEnv->SO_faceforward );
}

void CqShaderVM::SO_faceforward2()
{
	AUTOFUNC;
	FUNC3( type_vector, m_pEnv->SO_faceforward2 );
}

void CqShaderVM::SO_reflect()
{
	AUTOFUNC;
	FUNC2( type_vector, m_pEnv->SO_reflect );
}

void CqShaderVM::SO_refract()
{
	AUTOFUNC;
	FUNC3( type_vector, m_pEnv->SO_refract );
}

void CqShaderVM::SO_fresnel()
{
	AUTOFUNC;
	VOIDFUNC5( m_pEnv->SO_fresnel );
}

void CqShaderVM::SO_fresnel2()
{
	AUTOFUNC;
	VOIDFUNC7( m_pEnv->SO_fresnel );
}

void CqShaderVM::SO_transform2()
{
	AUTOFUNC;
	FUNC3( type_point, m_pEnv->SO_transform );
}

void CqShaderVM::SO_transform()
{
	AUTOFUNC;
	FUNC2( type_point, m_pEnv->SO_transform );
}

void CqShaderVM::SO_transformm()
{
	AUTOFUNC;
	FUNC2( type_point, m_pEnv->SO_transformm );
}

void CqShaderVM::SO_vtransform2()
{
	AUTOFUNC;
	FUNC3( type_vector, m_pEnv->SO_vtransform );
}

void CqShaderVM::SO_vtransform()
{
	AUTOFUNC;
	FUNC2( type_vector, m_pEnv->SO_vtransform );
}

void CqShaderVM::SO_vtransformm()
{
	AUTOFUNC;
	FUNC2( type_vector, m_pEnv->SO_vtransformm );
}

void CqShaderVM::SO_ntransform2()
{
	AUTOFUNC;
	FUNC3( type_normal, m_pEnv->SO_ntransform );
}

void CqShaderVM::SO_ntransform()
{
	AUTOFUNC;
	FUNC2( type_normal, m_pEnv->SO_ntransform );
}

void CqShaderVM::SO_ntransformm()
{
	AUTOFUNC;
	FUNC2( type_normal, m_pEnv->SO_ntransformm );
}

void CqShaderVM::SO_mtransform2()
{
	AUTOFUNC;
	FUNC3( type_matrix, m_pEnv->SO_mtransform );
}

void CqShaderVM::SO_mtransform()
{
	AUTOFUNC;
	FUNC2( type_matrix, m_pEnv->SO_mtransform );
}

void CqShaderVM::SO_depth()
{
	AUTOFUNC;
	FUNC1( type_float, m_pEnv->SO_depth );
}

void CqShaderVM::SO_calculatenormal()
{
	AUTOFUNC;
	FUNC1( type_normal, m_pEnv->SO_calculatenormal );
}

void CqShaderVM::SO_comp()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		OpCOMP_C( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
	RELEASE( B );
	RELEASE( A );
}

void CqShaderVM::SO_setcomp()
{
	AUTOFUNC;
	VOIDFUNC3( m_pEnv->SO_setcomp );
}

void CqShaderVM::SO_cmix()
{
	AUTOFUNC;
	FUNC3( type_color, m_pEnv->SO_cmix );
}

void CqShaderVM::SO_cmixc()
{
	AUTOFUNC;
	FUNC3( type_color, m_pEnv->SO_cmixc );
}

void CqShaderVM::SO_fmix()
{
	AUTOFUNC;
	FUNC3( type_float, m_pEnv->SO_fmix );
}

void CqShaderVM::SO_pmix()
{
	AUTOFUNC;
	FUNC3( type_point, m_pEnv->SO_pmix );
}

void CqShaderVM::SO_vmix()
{
	AUTOFUNC;
	FUNC3( type_vector, m_pEnv->SO_vmix );
}

void CqShaderVM::SO_nmix()
{
	AUTOFUNC;
	FUNC3( type_normal, m_pEnv->SO_nmix );
}

void CqShaderVM::SO_pmixc()
{
	AUTOFUNC;
	FUNC3( type_point, m_pEnv->SO_pmixc );
}

void CqShaderVM::SO_vmixc()
{
	AUTOFUNC;
	FUNC3( type_vector, m_pEnv->SO_vmixc );
}

void CqShaderVM::SO_nmixc()
{
	AUTOFUNC;
	FUNC3( type_normal, m_pEnv->SO_nmixc );
}

void CqShaderVM::SO_ambient()
{
	VARFUNC;
	FUNC( type_color, m_pEnv->SO_ambient );
}

void CqShaderVM::SO_diffuse()
{
	VARFUNC;
	FUNC1( type_color, m_pEnv->SO_diffuse );
}

void CqShaderVM::SO_specular()
{
	VARFUNC;
	FUNC3( type_color, m_pEnv->SO_specular );
}

void CqShaderVM::SO_phong()
{
	VARFUNC;
	FUNC3( type_color, m_pEnv->SO_phong );
}

void CqShaderVM::SO_trace()
{
	VARFUNC;
	FUNC2( type_color, m_pEnv->SO_trace );
}


// Macros for declaring the texture shadeops
#define	TEXTURE(t,func)	POPV(count); /* additional parameter count */\
						POPV(name); /* texture name */\
						POPV(channel); /* channel */\
						TqFloat fc; \
						count->GetFloat( fc ); \
						TqInt cParams=static_cast<TqInt>( fc ); \
						IqShaderData** aParams=new IqShaderData*[cParams]; \
						SqStackEntry *stackitems = new SqStackEntry[cParams];\
						TqInt iP=0; \
						while(iP!=cParams)	{\
							stackitems[iP]=POP; \
							aParams[iP]=stackitems[iP].m_Data;\
							iP++;\
						}\
						RESULT(t,__fVarying?class_varying:class_uniform); \
						if(m_pEnv->IsRunning()) \
							func(name,channel,pResult,this,cParams,aParams); \
						delete[](aParams); \
						iP=0; \
						while(iP!=cParams)	{\
							Release( stackitems[iP]);\
							iP++;\
						}\
						delete[](stackitems); \
						Push(pResult); \
						RELEASE(count); \
						RELEASE(name); \
						RELEASE(channel);
#define	TEXTURE1(t,func)	POPV(count); /* additional parameter count */\
						POPV(name); /* texture name */\
						POPV(channel); /* channel */\
						POPV(R); /* point */\
						TqFloat fc; \
						count->GetFloat( fc ); \
						TqInt cParams=static_cast<TqInt>( fc ); \
						IqShaderData** aParams=new IqShaderData*[cParams]; \
						SqStackEntry *stackitems = new SqStackEntry[cParams];\
						TqInt iP=0; \
						while(iP!=cParams)	{\
							stackitems[iP]=POP; \
							aParams[iP]=stackitems[iP].m_Data;\
							iP++;\
						}\
						RESULT(t,__fVarying?class_varying:class_uniform); \
						if(m_pEnv->IsRunning()) \
							func(name,channel,R,pResult,this, cParams, aParams); \
						delete[](aParams); \
						iP=0; \
						while(iP!=cParams)	{\
							Release( stackitems[iP]);\
							iP++;\
						}\
						delete[](stackitems); \
						Push(pResult); \
						RELEASE(count); \
						RELEASE(name); \
						RELEASE(channel); \
						RELEASE(R);
#define	TEXTURE3(t,func)	POPV(count); /* additional parameter count */\
						POPV(name); /* texture name */\
						POPV(channel); /* channel */\
						POPV(P); /* point */\
						POPV(N); /* normal */\
						POPV(samples); /* samples */\
						TqFloat fc; \
						count->GetFloat( fc ); \
						TqInt cParams=static_cast<TqInt>( fc ); \
						IqShaderData** aParams=new IqShaderData*[cParams]; \
						SqStackEntry *stackitems = new SqStackEntry[cParams];\
						TqInt iP=0; \
						while(iP!=cParams)	{\
							stackitems[iP]=POP; \
							aParams[iP]=stackitems[iP].m_Data;\
							iP++;\
						}\
						RESULT(t,__fVarying?class_varying:class_uniform); \
						if(m_pEnv->IsRunning()) \
							func(name,channel,P,N,samples,pResult,this, cParams, aParams); \
						delete[](aParams); \
						iP=0; \
						while(iP!=cParams)	{\
							Release( stackitems[iP]);\
							iP++;\
						}\
						delete[](stackitems); \
						Push(pResult); \
						RELEASE(count); \
						RELEASE(name); \
						RELEASE(channel); \
						RELEASE(P); \
						RELEASE(N); \
						RELEASE(samples);
#define	TEXTURE2(t,func)	POPV(count); /* additional parameter count */\
						POPV(name); /* texture name */\
						POPV(channel); /* channel */\
						POPV(s1); /* s */\
						POPV(t1); /* t */\
						TqFloat fc; \
						count->GetFloat( fc ); \
						TqInt cParams=static_cast<TqInt>( fc ); \
						IqShaderData** aParams=new IqShaderData*[cParams]; \
						SqStackEntry *stackitems = new SqStackEntry[cParams];\
						TqInt iP=0; \
						while(iP!=cParams)	{\
							stackitems[iP]=POP; \
							aParams[iP]=stackitems[iP].m_Data;\
							iP++;\
						}\
						RESULT(t,__fVarying?class_varying:class_uniform); \
						if(m_pEnv->IsRunning()) \
							func(name,channel,s1,t1,pResult,this, cParams, aParams); \
						delete[](aParams); \
						iP=0; \
						while(iP!=cParams)	{\
							Release( stackitems[iP]);\
							iP++;\
						}\
						delete[](stackitems); \
						Push(pResult); \
						RELEASE(count); \
						RELEASE(name); \
						RELEASE(channel); \
						RELEASE(s1); \
						RELEASE(t1);
#define	TEXTURE4(t,func)	POPV(count); /* additional parameter count */\
						POPV(name); /* texture name */\
						POPV(channel); /* channel */\
						POPV(R1); /* R1 */\
						POPV(R2); /* R2 */\
						POPV(R3); /* R3 */\
						POPV(R4); /* R4 */\
						TqFloat fc; \
						count->GetFloat( fc ); \
						TqInt cParams=static_cast<TqInt>( fc ); \
						IqShaderData** aParams=new IqShaderData*[cParams]; \
						SqStackEntry *stackitems = new SqStackEntry[cParams];\
						TqInt iP=0; \
						while(iP!=cParams)	{\
							stackitems[iP]=POP; \
							aParams[iP]=stackitems[iP].m_Data;\
							iP++;\
						}\
						RESULT(t,__fVarying?class_varying:class_uniform); \
						if(m_pEnv->IsRunning()) \
							func(name,channel,R1,R2,R3,R4,pResult,this,cParams,aParams); \
						delete[](aParams); \
						iP=0; \
						while(iP!=cParams)	{\
							Release( stackitems[iP]);\
							iP++;\
						}\
						delete[](stackitems); \
						Push(pResult); \
						RELEASE(count); \
						RELEASE(name); \
						RELEASE(channel); \
						RELEASE(R1); \
						RELEASE(R2); \
						RELEASE(R3); \
						RELEASE(R4);
#define	TEXTURE8(t,func)	POPV(count); /* additional parameter count */\
						POPV(name); /* texture name */\
						POPV(channel); /* channel */\
						POPV(s1); /* s1 */\
						POPV(t1); /* t1 */\
						POPV(s2); /* s2 */\
						POPV(t2); /* t2 */\
						POPV(s3); /* s3 */\
						POPV(t3); /* t3 */\
						POPV(s4); /* s4 */\
						POPV(t4); /* t4 */\
						TqFloat fc; \
						count->GetFloat( fc ); \
						TqInt cParams=static_cast<TqInt>( fc ); \
						IqShaderData** aParams=new IqShaderData*[cParams]; \
						SqStackEntry *stackitems = new SqStackEntry[cParams];\
						TqInt iP=0; \
						while(iP!=cParams)	{\
							stackitems[iP]=POP; \
							aParams[iP]=stackitems[iP].m_Data;\
							iP++;\
						}\
						RESULT(t,__fVarying?class_varying:class_uniform); \
						if(m_pEnv->IsRunning()) \
							func(name,channel,s1,t1,s2,t2,s3,t3,s4,t4,pResult,this,cParams,aParams); \
						delete[](aParams); \
						iP=0; \
						while(iP!=cParams)	{\
							Release( stackitems[iP]);\
							iP++;\
						}\
						delete[](stackitems); \
						Push(pResult); \
						RELEASE(count); \
						RELEASE(name); \
						RELEASE(channel); \
						RELEASE(s1); \
						RELEASE(t1); \
						RELEASE(s2); \
						RELEASE(t2); \
						RELEASE(s3); \
						RELEASE(t3); \
						RELEASE(s4); \
						RELEASE(t4);

void CqShaderVM::SO_shadow()
{
	VARFUNC;
	TEXTURE1( type_float, m_pEnv->SO_shadow );
}

void CqShaderVM::SO_shadow1()
{
	VARFUNC;
	TEXTURE4( type_float, m_pEnv->SO_shadow1 );
}

void CqShaderVM::SO_ftexture1()
{
	VARFUNC;
	TEXTURE( type_float, m_pEnv->SO_ftexture1 );
}

void CqShaderVM::SO_ftexture2()
{
	VARFUNC;
	TEXTURE2( type_float, m_pEnv->SO_ftexture2 );
}

void CqShaderVM::SO_ftexture3()
{
	VARFUNC;
	TEXTURE8( type_float, m_pEnv->SO_ftexture3 );
}

void CqShaderVM::SO_ctexture1()
{
	VARFUNC;
	TEXTURE( type_color, m_pEnv->SO_ctexture1 );
}

void CqShaderVM::SO_ctexture2()
{
	VARFUNC;
	TEXTURE2( type_color, m_pEnv->SO_ctexture2 );
}

void CqShaderVM::SO_ctexture3()
{
	VARFUNC;
	TEXTURE8( type_color, m_pEnv->SO_ctexture3 );
}

void CqShaderVM::SO_fenvironment2()
{
	VARFUNC;
	TEXTURE1( type_float, m_pEnv->SO_fenvironment2 );
}

void CqShaderVM::SO_fenvironment3()
{
	VARFUNC;
	TEXTURE4( type_float, m_pEnv->SO_fenvironment3 );
}

void CqShaderVM::SO_cenvironment2()
{
	VARFUNC;
	TEXTURE1( type_color, m_pEnv->SO_cenvironment2 );
}

void CqShaderVM::SO_cenvironment3()
{
	VARFUNC;
	TEXTURE4( type_color, m_pEnv->SO_cenvironment3 );
}

void CqShaderVM::SO_bump1()
{
	VARFUNC;
	TEXTURE( type_point, m_pEnv->SO_bump1 );
}

void CqShaderVM::SO_bump2()
{
	VARFUNC;
	TEXTURE2( type_point, m_pEnv->SO_bump2 );
}

void CqShaderVM::SO_bump3()
{
	VARFUNC;
	TEXTURE8( type_point, m_pEnv->SO_bump3 );
}

void CqShaderVM::SO_illuminate()
{
	VARFUNC;
	VOIDFUNC1( m_pEnv->SO_illuminate );
}

void CqShaderVM::SO_illuminate2()
{
	VARFUNC;
	VOIDFUNC3( m_pEnv->SO_illuminate );
}

void CqShaderVM::SO_init_illuminance()
{
	VARFUNC;
	POPV( A );
	RESULT(type_float, class_varying);
	if(m_pEnv->IsRunning())
	{
		m_pEnv->InvalidateIlluminanceCache();
		m_pEnv->ValidateIlluminanceCache( A, NULL, this );
		pResult->SetFloat( m_pEnv->SO_init_illuminance() );
	}
	Push( pResult );
	RELEASE( A );
}

void CqShaderVM::SO_init_illuminance2()
{
	VARFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_float, class_varying);
	if(m_pEnv->IsRunning())
	{
		m_pEnv->InvalidateIlluminanceCache();
		m_pEnv->ValidateIlluminanceCache( A, B, this );
		pResult->SetFloat( m_pEnv->SO_init_illuminance() );
	}
	Push( pResult );
	RELEASE( A );
}

void CqShaderVM::SO_advance_illuminance()
{
	RESULT(type_float, class_varying);
	if(m_pEnv->IsRunning())
		pResult->SetFloat( m_pEnv->SO_advance_illuminance() );
	Push( pResult );
}

void CqShaderVM::SO_illuminance()
{
	VARFUNC;
	VOIDFUNC2( m_pEnv->SO_illuminance );
}

void CqShaderVM::SO_illuminance2()
{
	VARFUNC;
	VOIDFUNC4( m_pEnv->SO_illuminance );
}

void CqShaderVM::SO_solar()
{
	CONSTFUNC;
	VOIDFUNC( m_pEnv->SO_solar );
}

void CqShaderVM::SO_solar2()
{
	VARFUNC;
	VOIDFUNC2( m_pEnv->SO_solar );
}

void CqShaderVM::SO_init_gather()
{
	VARFUNC;
	VOIDFUNC1( m_pEnv->SO_init_gather );
}

void CqShaderVM::SO_advance_gather()
{
	RESULT(type_float, class_varying);
	if(m_pEnv->IsRunning())
		pResult->SetFloat( m_pEnv->SO_advance_gather() );
	Push( pResult );
}

void CqShaderVM::SO_gather()
{
	VARFUNC;
	VOIDFUNC5PLUS( m_pEnv->SO_gather );
}

void CqShaderVM::SO_printf()
{
	AUTOFUNC;
	VOIDFUNC1PLUS( m_pEnv->SO_printf );
}

void CqShaderVM::SO_atmosphere()
{
	AUTOFUNC;
	IqShaderData* pV = GetVar( ReadNext().m_iVariable );
	POPV( Val );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		m_pEnv->SO_atmosphere( Val, pV, pResult );
	Push( pResult );
	RELEASE( Val );
}

void CqShaderVM::SO_displacement()
{
	AUTOFUNC;
	IqShaderData* pV = GetVar( ReadNext().m_iVariable );
	POPV( Val );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		m_pEnv->SO_displacement( Val, pV, pResult );
	Push( pResult );
	RELEASE( Val );
}

void CqShaderVM::SO_lightsource()
{
	AUTOFUNC;
	IqShaderData* pV = GetVar( ReadNext().m_iVariable );
	POPV( Val );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		m_pEnv->SO_lightsource( Val, pV, pResult );
	Push( pResult );
	RELEASE( Val );
}

void CqShaderVM::SO_surface()
{
	AUTOFUNC;
	IqShaderData* pV = GetVar( ReadNext().m_iVariable );
	POPV( Val );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		m_pEnv->SO_surface( Val, pV, pResult );
	Push( pResult );
	RELEASE( Val );
}

void CqShaderVM::SO_attribute()
{
	AUTOFUNC;
	IqShaderData* pV = GetVar( ReadNext().m_iVariable );
	POPV( Val );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		m_pEnv->SO_attribute( Val, pV, pResult );
	Push( pResult );
	RELEASE( Val );
}

void CqShaderVM::SO_option()
{
	AUTOFUNC;
	IqShaderData* pV = GetVar( ReadNext().m_iVariable );
	POPV( Val );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		m_pEnv->SO_option( Val, pV, pResult );
	Push( pResult );
	RELEASE( Val );
}

void CqShaderVM::SO_rendererinfo()
{
	AUTOFUNC;
	IqShaderData* pV = GetVar( ReadNext().m_iVariable );
	POPV( Val );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		m_pEnv->SO_rendererinfo( Val, pV, pResult );
	Push( pResult );
	RELEASE( Val );
}

void CqShaderVM::SO_textureinfo()
{

	AUTOFUNC;
	IqShaderData* pV = GetVar( ReadNext().m_iVariable );
	POPV( Val );
	POPV( DataInfo );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		m_pEnv->SO_textureinfo( Val, DataInfo, pV, pResult );
	Push( pResult );
	RELEASE( Val );
}

void CqShaderVM::SO_incident()
{
	AUTOFUNC;
	IqShaderData* pV = GetVar( ReadNext().m_iVariable );
	POPV( Val );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		m_pEnv->SO_incident( Val, pV, pResult );
	Push( pResult );
	RELEASE( Val );
}

void CqShaderVM::SO_opposite()
{
	AUTOFUNC;
	IqShaderData* pV = GetVar( ReadNext().m_iVariable );
	POPV( Val );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		m_pEnv->SO_opposite( Val, pV, pResult );
	Push( pResult );
	RELEASE( Val );
}

void CqShaderVM::SO_fcellnoise1()
{
	AUTOFUNC;
	FUNC1( type_float, m_pEnv->SO_fcellnoise1 );
}

void CqShaderVM::SO_fcellnoise2()
{
	AUTOFUNC;
	FUNC2( type_float, m_pEnv->SO_fcellnoise2 );
}

void CqShaderVM::SO_fcellnoise3()
{
	AUTOFUNC;
	FUNC1( type_float, m_pEnv->SO_fcellnoise3 );
}

void CqShaderVM::SO_fcellnoise4()
{
	AUTOFUNC;
	FUNC2( type_float, m_pEnv->SO_fcellnoise4 );
}

void CqShaderVM::SO_ccellnoise1()
{
	AUTOFUNC;
	FUNC1( type_color, m_pEnv->SO_ccellnoise1 );
}

void CqShaderVM::SO_ccellnoise2()
{
	AUTOFUNC;
	FUNC2( type_color, m_pEnv->SO_ccellnoise2 );
}

void CqShaderVM::SO_ccellnoise3()
{
	AUTOFUNC;
	FUNC1( type_color, m_pEnv->SO_ccellnoise3 );
}

void CqShaderVM::SO_ccellnoise4()
{
	AUTOFUNC;
	FUNC2( type_color, m_pEnv->SO_ccellnoise4 );
}

void CqShaderVM::SO_pcellnoise1()
{
	AUTOFUNC;
	FUNC1( type_point, m_pEnv->SO_pcellnoise1 );
}

void CqShaderVM::SO_pcellnoise2()
{
	AUTOFUNC;
	FUNC2( type_point, m_pEnv->SO_pcellnoise2 );
}

void CqShaderVM::SO_pcellnoise3()
{
	AUTOFUNC;
	FUNC1( type_point, m_pEnv->SO_pcellnoise3 );
}

void CqShaderVM::SO_pcellnoise4()
{
	AUTOFUNC;
	FUNC2( type_point, m_pEnv->SO_pcellnoise4 );
}

void CqShaderVM::SO_fpnoise1()
{
	AUTOFUNC;
	FUNC2( type_float, m_pEnv->SO_fpnoise1 );
}

void CqShaderVM::SO_fpnoise2()
{
	AUTOFUNC;
	FUNC4( type_float, m_pEnv->SO_fpnoise2 );
}

void CqShaderVM::SO_fpnoise3()
{
	AUTOFUNC;
	FUNC2( type_float, m_pEnv->SO_fpnoise3 );
}

void CqShaderVM::SO_fpnoise4()
{
	AUTOFUNC;
	FUNC4( type_float, m_pEnv->SO_fpnoise4 );
}

void CqShaderVM::SO_cpnoise1()
{
	AUTOFUNC;
	FUNC2( type_color, m_pEnv->SO_cpnoise1 );
}

void CqShaderVM::SO_cpnoise2()
{
	AUTOFUNC;
	FUNC4( type_color, m_pEnv->SO_cpnoise2 );
}

void CqShaderVM::SO_cpnoise3()
{
	AUTOFUNC;
	FUNC2( type_color, m_pEnv->SO_cpnoise3 );
}

void CqShaderVM::SO_cpnoise4()
{
	AUTOFUNC;
	FUNC4( type_color, m_pEnv->SO_cpnoise4 );
}

void CqShaderVM::SO_ppnoise1()
{
	AUTOFUNC;
	FUNC2( type_point, m_pEnv->SO_ppnoise1 );
}

void CqShaderVM::SO_ppnoise2()
{
	AUTOFUNC;
	FUNC4( type_point, m_pEnv->SO_ppnoise2 );
}

void CqShaderVM::SO_ppnoise3()
{
	AUTOFUNC;
	FUNC2( type_point, m_pEnv->SO_ppnoise3 );
}

void CqShaderVM::SO_ppnoise4()
{
	AUTOFUNC;
	FUNC4( type_point, m_pEnv->SO_ppnoise4 );
}

void CqShaderVM::SO_ctransform2()
{
	AUTOFUNC;
	FUNC3( type_color, m_pEnv->SO_ctransform );
}

void CqShaderVM::SO_ctransform()
{
	AUTOFUNC;
	FUNC2( type_color, m_pEnv->SO_ctransform );
}

void CqShaderVM::SO_ptlined()
{
	AUTOFUNC;
	FUNC3( type_float, m_pEnv->SO_ptlined );
}

void CqShaderVM::SO_inversesqrt()
{
	AUTOFUNC;
	FUNC1( type_float, m_pEnv->SO_inversesqrt );
}

void CqShaderVM::SO_concat()
{
	AUTOFUNC;
	FUNC2PLUS( type_string, m_pEnv->SO_concat );
}

void CqShaderVM::SO_format()
{
	AUTOFUNC;
	FUNC1PLUS( type_string, m_pEnv->SO_format );
}

void CqShaderVM::SO_match()
{
	AUTOFUNC;
	FUNC2( type_float, m_pEnv->SO_match );
}

void CqShaderVM::SO_rotate()
{
	AUTOFUNC;
	FUNC4( type_point, m_pEnv->SO_rotate );
}

void CqShaderVM::SO_filterstep()
{
	AUTOFUNC;
	;
	FUNC2PLUS( type_float, m_pEnv->SO_filterstep );
}

void CqShaderVM::SO_filterstep2()
{
	AUTOFUNC;
	FUNC3PLUS( type_float, m_pEnv->SO_filterstep2 );
}

void CqShaderVM::SO_specularbrdf()
{
	AUTOFUNC;
	FUNC4( type_color, m_pEnv->SO_specularbrdf );
}


void CqShaderVM::SO_mcomp()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	POPV( C );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		OpCOMPM( A, B, C, pResult, m_pEnv->RunningState() );
	Push( pResult );
	RELEASE( C );
	RELEASE( B );
	RELEASE( A );
}

void CqShaderVM::SO_setmcomp()
{
	AUTOFUNC;
	VOIDFUNC4( m_pEnv->SO_setmcomp );
}

void CqShaderVM::SO_determinant()
{
	AUTOFUNC;
	FUNC1( type_float, m_pEnv->SO_determinant );
}

void CqShaderVM::SO_mtranslate()
{
	AUTOFUNC;
	FUNC2( type_matrix, m_pEnv->SO_mtranslate );
}

void CqShaderVM::SO_mrotate()
{
	AUTOFUNC;
	FUNC3( type_matrix, m_pEnv->SO_mrotate );
}

void CqShaderVM::SO_mscale()
{
	AUTOFUNC;
	FUNC2( type_matrix, m_pEnv->SO_mscale );
}


void CqShaderVM::SO_fsplinea()
{
	AUTOFUNC;
	FUNC2( type_float, m_pEnv->SO_fsplinea );
}

void CqShaderVM::SO_csplinea()
{
	AUTOFUNC;
	FUNC2( type_color, m_pEnv->SO_csplinea );
}

void CqShaderVM::SO_psplinea()
{
	AUTOFUNC;
	FUNC2( type_point, m_pEnv->SO_psplinea );
}

void CqShaderVM::SO_sfsplinea()
{
	AUTOFUNC;
	FUNC3( type_float, m_pEnv->SO_sfsplinea );
}

void CqShaderVM::SO_scsplinea()
{
	AUTOFUNC;
	FUNC3( type_color, m_pEnv->SO_scsplinea );
}

void CqShaderVM::SO_spsplinea()
{
	AUTOFUNC;
	FUNC3( type_point, m_pEnv->SO_spsplinea );
}

void CqShaderVM::SO_shadername()
{
	AUTOFUNC;
	FUNC( type_string, m_pEnv->SO_shadername );
}

void CqShaderVM::SO_shadername2()
{
	AUTOFUNC;
	FUNC1( type_string, m_pEnv->SO_shadername2 );
}

// BAKING BASED ON APPLICATION FROM SIGGRAPH 2002 / Larry G.
void CqShaderVM::SO_bake_f()
{
	VARFUNC;
	VOIDFUNC4( m_pEnv->SO_bake_f );
}
void CqShaderVM::SO_bake_3c()
{
	VARFUNC;
	VOIDFUNC4( m_pEnv->SO_bake_3c );
}
void CqShaderVM::SO_bake_3p()
{
	VARFUNC;
	VOIDFUNC4( m_pEnv->SO_bake_3p );
}
void CqShaderVM::SO_bake_3v()
{
	VARFUNC;
	VOIDFUNC4( m_pEnv->SO_bake_3v );
}
void CqShaderVM::SO_bake_3n()
{
	VARFUNC;
	VOIDFUNC4( m_pEnv->SO_bake_3n );
}

void CqShaderVM::SO_external()
{
	AUTOFUNC;
	SqDSOExternalCall *pCall = ReadNext().m_pExtCall;
	// This is a little ugly, but it means we can still use RESULT with Voids.
	RESULT( (pCall->return_type != type_void ? pCall->return_type : type_float) , class_varying);

	SqStackEntry *stackitems = new SqStackEntry[pCall->arg_types.size()];
	IqShaderData **arg_data = new IqShaderData*[pCall->arg_types.size()];
	unsigned int x = 0;
	for ( x = 0 ; x < pCall->arg_types.size();x++)
	{
		stackitems[x] = POP;
		arg_data[x] = stackitems[x].m_Data;
	};

	if(m_pEnv->IsRunning())
		m_pEnv->SO_external(pCall->method, pCall->initData, pResult, this, pCall->arg_types.size(),arg_data);

	for ( x = 0 ; x < pCall->arg_types.size();x++)
	{
		Release( stackitems[x] );
	};

	delete[]( stackitems );
	delete[]( arg_data );

	Push( pResult );
}

/*
void CqShaderVM::SO_external_error()
{
	AUTOFUNC;
	SqDSOExternalCall *pCall = ReadNext().m_pExtCall;
	// This is a little ugly, but it means we can still use RESULT with Voids.
	RESULT( (pCall->return_type != type_void ? pCall->return_type : type_float) , class_varying);
 
	SqStackEntry *stackitems = new SqStackEntry[pCall->arg_types.size()];
	unsigned int x = 0;
	for ( x = 0 ; x < pCall->arg_types.size();x++){
	  	stackitems[x] = POP;
		Release( stackitems[x] );
	};
 
	delete( stackitems );
 
	if( pCall->return_type != type_void )
	{
		Push( pResult );
	} else {
	  	DeleteTemporaryStorage( pResult );
	};
}
*/

void CqShaderVM::SO_occlusion()
{
	VARFUNC;
	TEXTURE3( type_float, m_pEnv->SO_occlusion );
}

void CqShaderVM::SO_occlusion_rt()
{
	VARFUNC;
	FUNC3( type_float, m_pEnv->SO_occlusion_rt );
}

void CqShaderVM::SO_rayinfo()
{
	AUTOFUNC;
	IqShaderData* pV = GetVar( ReadNext().m_iVariable );
	POPV( DataInfo );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		m_pEnv->SO_rayinfo( DataInfo, pV, pResult );
	Push( pResult );
}

void CqShaderVM::SO_bake3d()
{
	VARFUNC;
	FUNC4PLUS( type_float, m_pEnv->SO_bake3d );
}

void CqShaderVM::SO_texture3d()
{
	VARFUNC;
	FUNC3PLUS( type_float, m_pEnv->SO_texture3d );
}

} // namespace Aqsis
//---------------------------------------------------------------------
