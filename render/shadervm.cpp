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
		\brief Implements classes and support functionality for the shader virtual machine.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include	<iostream>

#include	<ctype.h>

#include	"aqsis.h"
#include	"shadervm.h"
#include	"symbols.h"
#include	"irenderer.h"
#include	"version.h"
#include	"sstring.h"

START_NAMESPACE(Aqsis)

char* gShaderTypeNames[]=
{
	"surface",
	"lightsource",
	"volume",
	"displacement",
	"transformation",
	"imager",
};
TqInt gcShaderTypeNames=sizeof(gShaderTypeNames)/sizeof(gShaderTypeNames[0]);


CqParameter* (*gVariableCreateFuncsUniform[])(const char* strName, TqInt Count)=
{
	0,
	CqParameterTypedUniform<TqFloat,Type_UniformFloat>::Create,
	CqParameterTypedUniform<TqInt,Type_UniformInteger>::Create,
	CqParameterTypedUniform<CqVector3D,Type_UniformPoint>::Create,
	CqParameterTypedUniform<CqString,Type_UniformString>::Create,
	CqParameterTypedUniform<CqColor,Type_UniformColor>::Create,
	0,
	CqParameterTypedUniform<CqVector4D,Type_UniformhPoint>::Create,
	CqParameterTypedUniform<CqVector3D,Type_UniformNormal>::Create,
	CqParameterTypedUniform<CqVector3D,Type_UniformVector>::Create,
	CqParameterTypedUniform<CqMatrix,Type_UniformMatrix>::Create,
};

CqParameter* (*gVariableCreateFuncsVarying[])(const char* strName, TqInt Count)=
{
	0,
	CqParameterTypedVarying<TqFloat,Type_VaryingFloat>::Create,
	CqParameterTypedVarying<TqInt,Type_VaryingInteger>::Create,
	CqParameterTypedVarying<CqVector3D,Type_VaryingPoint>::Create,
	CqParameterTypedVarying<CqString,Type_VaryingString>::Create,
	CqParameterTypedVarying<CqColor,Type_VaryingColor>::Create,
	0,
	CqParameterTypedVarying<CqVector4D,Type_VaryinghPoint>::Create,
	CqParameterTypedVarying<CqVector3D,Type_VaryingNormal>::Create,
	CqParameterTypedVarying<CqVector3D,Type_VaryingVector>::Create,
	CqParameterTypedVarying<CqMatrix,Type_VaryingMatrix>::Create,
};

CqParameter* (*gVariableCreateFuncsVertex[])(const char* strName, TqInt Count)=
{
	0,
	CqParameterTypedVertex<TqFloat,Type_VertexFloat>::Create,
	CqParameterTypedVertex<TqInt,Type_VertexInteger>::Create,
	CqParameterTypedVertex<CqVector3D,Type_VertexPoint>::Create,
	CqParameterTypedVertex<CqString,Type_VertexString>::Create,
	CqParameterTypedVertex<CqColor,Type_VertexColor>::Create,
	0,
	CqParameterTypedVertex<CqVector4D,Type_VertexhPoint>::Create,
	CqParameterTypedVertex<CqVector3D,Type_VertexNormal>::Create,
	CqParameterTypedVertex<CqVector3D,Type_VertexVector>::Create,
	CqParameterTypedVertex<CqMatrix,Type_VertexMatrix>::Create,
};


/** Default constructor
 * \param strName Character pointer to parameter name.
 * \param Count Integer value count, for arrays.
 */
CqParameter::CqParameter(const char* strName, TqInt Count) : 
					m_strName(strName),
					m_Count(Count)
{
	pCurrentRenderer()->Stats().cParametersAllocated()++;
}

/** Copy constructor
 */
CqParameter::CqParameter(const CqParameter& From) :
					m_strName(From.m_strName),
					m_Count(From.m_Count)
{
	pCurrentRenderer()->Stats().cParametersAllocated()++;
}

CqParameter::~CqParameter()
{
	pCurrentRenderer()->Stats().cParametersDeallocated()++;
}



SqOpCodeTrans CqShaderVM::m_TransTable[]=
{
	{"RS_PUSH",		&CqShaderVM::SO_RS_PUSH,		0, },
	{"RS_POP",		&CqShaderVM::SO_RS_POP,		0, },
	{"RS_GET",		&CqShaderVM::SO_RS_GET,		0, },
	{"RS_INVERSE",	&CqShaderVM::SO_RS_INVERSE,	0, },
	{"RS_JZ",		&CqShaderVM::SO_RS_JZ,		1, {Type_Float}},
	{"RS_JNZ",		&CqShaderVM::SO_RS_JNZ,		1, {Type_Float}},
	{"S_JZ",		&CqShaderVM::SO_S_JZ,		1, {Type_Float}},
	{"S_JNZ",		&CqShaderVM::SO_S_JNZ,		1, {Type_Float}},
	{"S_GET",		&CqShaderVM::SO_S_GET,		0, },
	{"S_CLEAR",		&CqShaderVM::SO_S_CLEAR,		0, },

	{"nop",			&CqShaderVM::SO_nop,			0, },
	{"dup",			&CqShaderVM::SO_dup,			0, },
	{"debug_break",	&CqShaderVM::SO_debug_break,	0, },
	{"drop",		&CqShaderVM::SO_drop,		0, },

	{"mergef",		&CqShaderVM::SO_mergef,		0, },
	{"merges",		&CqShaderVM::SO_merges,		0, },
	{"mergep",		&CqShaderVM::SO_mergep,		0, },
	{"mergen",		&CqShaderVM::SO_mergep,		0, },
	{"mergev",		&CqShaderVM::SO_mergep,		0, },
	{"mergec",		&CqShaderVM::SO_mergec,		0, },

	{"pushif",		&CqShaderVM::SO_pushif,		1, {Type_Float}},
	{"puship",		&CqShaderVM::SO_puship,		3, {Type_Float,Type_Float,Type_Float}},
	{"pushis",		&CqShaderVM::SO_pushis,		1, {Type_String}},
	{"pushv",		&CqShaderVM::SO_pushv,		1, {Type_Variable}},
	{"ipushv",		&CqShaderVM::SO_ipushv,		1, {Type_Variable}},

	{"pop",			&CqShaderVM::SO_pop,			1, {Type_Variable}},
	{"ipop",		&CqShaderVM::SO_ipop,		1, {Type_Variable}},

	{"setfc",		&CqShaderVM::SO_setfc,		0, },
	{"setfp",		&CqShaderVM::SO_setfp,		0, },
	{"setfn",		&CqShaderVM::SO_setfp,		0, },
	{"setfv",		&CqShaderVM::SO_setfp,		0, },
	{"setfm",		&CqShaderVM::SO_setfm,		0, },

	{"settc",		&CqShaderVM::SO_settc,		0, },
	{"settp",		&CqShaderVM::SO_settp,		0, },
	{"settn",		&CqShaderVM::SO_settp,		0, },
	{"settv",		&CqShaderVM::SO_settp,		0, },

	{"setpc",		&CqShaderVM::SO_setpc,		0, },
	{"setvc",		&CqShaderVM::SO_setpc,		0, },
	{"setnc",		&CqShaderVM::SO_setpc,		0, },

	{"setcp",		&CqShaderVM::SO_setcp,		0, },
	{"setcn",		&CqShaderVM::SO_setcp,		0, },
	{"setcv",		&CqShaderVM::SO_setcp,		0, },

	{"setwm",		&CqShaderVM::SO_setwm,		0, },

	{"jnz",			&CqShaderVM::SO_jnz,			1, {Type_Float}},
	{"jz",			&CqShaderVM::SO_jz,			1, {Type_Float}},
	{"jmp",			&CqShaderVM::SO_jmp,			1, {Type_Float}},

	{"lsff",		&CqShaderVM::SO_lsff,		0, },
	{"lspp",		&CqShaderVM::SO_lspp,		0, },
	{"lshh",		&CqShaderVM::SO_lspp,		0, },
	{"lscc",		&CqShaderVM::SO_lscc,		0, },
	{"lsnn",		&CqShaderVM::SO_lspp,		0, },
	{"lsvv",		&CqShaderVM::SO_lspp,		0, },

	{"gtff",		&CqShaderVM::SO_gtff,		0, },
	{"gtpp",		&CqShaderVM::SO_gtpp,		0, },
	{"gthh",		&CqShaderVM::SO_gtpp,		0, },
	{"gtcc",		&CqShaderVM::SO_gtcc,		0, },
	{"gtnn",		&CqShaderVM::SO_gtpp,		0, },
	{"gtvv",		&CqShaderVM::SO_gtpp,		0, },

	{"geff",		&CqShaderVM::SO_geff,		0, },
	{"gepp",		&CqShaderVM::SO_gepp,		0, },
	{"gehh",		&CqShaderVM::SO_gepp,		0, },
	{"gecc",		&CqShaderVM::SO_gecc,		0, },
	{"genn",		&CqShaderVM::SO_gepp,		0, },
	{"gevv",		&CqShaderVM::SO_gepp,		0, },

	{"leff",		&CqShaderVM::SO_leff,		0, },
	{"lepp",		&CqShaderVM::SO_lepp,		0, },
	{"lehh",		&CqShaderVM::SO_lepp,		0, },
	{"lecc",		&CqShaderVM::SO_lecc,		0, },
	{"lenn",		&CqShaderVM::SO_lepp,		0, },
	{"levv",		&CqShaderVM::SO_lepp,		0, },

	{"eqff",		&CqShaderVM::SO_eqff,		0, },
	{"eqpp",		&CqShaderVM::SO_eqpp,		0, },
	{"eqhh",		&CqShaderVM::SO_eqpp,		0, },
	{"eqcc",		&CqShaderVM::SO_eqcc,		0, },
	{"eqss",		&CqShaderVM::SO_eqss,		0, },
	{"eqnn",		&CqShaderVM::SO_eqpp,		0, },
	{"eqvv",		&CqShaderVM::SO_eqpp,		0, },

	{"neff",		&CqShaderVM::SO_neff,		0, },
	{"nepp",		&CqShaderVM::SO_nepp,		0, },
	{"nehh",		&CqShaderVM::SO_nepp,		0, },
	{"necc",		&CqShaderVM::SO_necc,		0, },
	{"ness",		&CqShaderVM::SO_ness,		0, },
	{"nenn",		&CqShaderVM::SO_nepp,		0, },
	{"nevv",		&CqShaderVM::SO_nepp,		0, },

	{"mulff",		&CqShaderVM::SO_mulff,		0, },
	{"divff",		&CqShaderVM::SO_divff,		0, },
	{"addff",		&CqShaderVM::SO_addff,		0, },
	{"subff",		&CqShaderVM::SO_subff,		0, },
	{"negf",		&CqShaderVM::SO_negf,		0, },
	
	{"mulpp",		&CqShaderVM::SO_mulpp,		0, },
	{"divpp",		&CqShaderVM::SO_divpp,		0, },
	{"addpp",		&CqShaderVM::SO_addpp,		0, },
	{"subpp",		&CqShaderVM::SO_subpp,		0, },
	{"crspp",		&CqShaderVM::SO_crspp,		0, },
	{"dotpp",		&CqShaderVM::SO_dotpp,		0, },
	{"negp",		&CqShaderVM::SO_negp,		0, },

	{"mulcc",		&CqShaderVM::SO_mulcc,		0, },
	{"divcc",		&CqShaderVM::SO_divcc,		0, },
	{"addcc",		&CqShaderVM::SO_addcc,		0, },
	{"subcc",		&CqShaderVM::SO_subcc,		0, },
	{"crscc",		&CqShaderVM::SO_crscc,		0, },
	{"dotcc",		&CqShaderVM::SO_dotcc,		0, },
	{"negc",		&CqShaderVM::SO_negc,		0, },

	{"mulfp",		&CqShaderVM::SO_mulfp,		0, },
	{"divfp",		&CqShaderVM::SO_divfp,		0, },
	{"addfp",		&CqShaderVM::SO_addfp,		0, },
	{"subfp",		&CqShaderVM::SO_subfp,		0, },

	{"mulfc",		&CqShaderVM::SO_mulfc,		0, },
	{"divfc",		&CqShaderVM::SO_divfc,		0, },
	{"addfc",		&CqShaderVM::SO_addfc,		0, },
	{"subfc",		&CqShaderVM::SO_subfc,		0, },

	{"mulmm",		&CqShaderVM::SO_mulmm,		0, },
	{"divmm",		&CqShaderVM::SO_divmm,		0, },

	{"land",		&CqShaderVM::SO_land,		0, },
	{"lor",			&CqShaderVM::SO_lor,			0, },

	{"radians",		&CqShaderVM::SO_radians,		0, },
	{"degrees",		&CqShaderVM::SO_degrees,		0, },
	{"sin",			&CqShaderVM::SO_sin,			0, },
	{"asin",		&CqShaderVM::SO_asin,		0, },
	{"cos",			&CqShaderVM::SO_cos,			0, },
	{"acos",		&CqShaderVM::SO_acos,		0, },
	{"tan",			&CqShaderVM::SO_tan,			0, },
	{"atan",		&CqShaderVM::SO_atan,		0, },
	{"atan2",		&CqShaderVM::SO_atan2,		0, },
	{"pow",			&CqShaderVM::SO_pow,			0, },
	{"exp",			&CqShaderVM::SO_exp,			0, },
	{"sqrt",		&CqShaderVM::SO_sqrt,		0, },
	{"log",			&CqShaderVM::SO_log,			0, },
	{"log2",		&CqShaderVM::SO_log2,		0, },
	{"mod",			&CqShaderVM::SO_mod,			0, },
	{"abs",			&CqShaderVM::SO_abs,			0, },
	{"sign",		&CqShaderVM::SO_sign,		0, },
	{"min",			&CqShaderVM::SO_min,			0, },
	{"max",			&CqShaderVM::SO_max,			0, },
	{"pmin",		&CqShaderVM::SO_pmin,		0, },
	{"pmax",		&CqShaderVM::SO_pmax,		0, },
	{"vmin",		&CqShaderVM::SO_vmin,		0, },
	{"vmax",		&CqShaderVM::SO_vmax,		0, },
	{"nmin",		&CqShaderVM::SO_nmin,		0, },
	{"nmax",		&CqShaderVM::SO_nmax,		0, },
	{"cmin",		&CqShaderVM::SO_cmin,		0, },
	{"cmax",		&CqShaderVM::SO_cmax,		0, },
	{"clamp",		&CqShaderVM::SO_clamp,		0, },
	{"pclamp",		&CqShaderVM::SO_pclamp,		0, },
	{"vclamp",		&CqShaderVM::SO_pclamp,		0, },
	{"nclamp",		&CqShaderVM::SO_pclamp,		0, },
	{"cclamp",		&CqShaderVM::SO_cclamp,		0, },
	{"floor",		&CqShaderVM::SO_floor,		0, },
	{"ceil",		&CqShaderVM::SO_ceil,		0, },
	{"round",		&CqShaderVM::SO_round,		0, },
	{"step",		&CqShaderVM::SO_step,		0, },
	{"smoothstep",	&CqShaderVM::SO_smoothstep,	0, },
	{"fspline",		&CqShaderVM::SO_fspline,		0, },
	{"cspline",		&CqShaderVM::SO_cspline,		0, },
	{"pspline",		&CqShaderVM::SO_pspline,		0, },
	{"vspline",		&CqShaderVM::SO_pspline,		0, },
	{"sfspline",	&CqShaderVM::SO_sfspline,	0, },
	{"scspline",	&CqShaderVM::SO_scspline,	0, },
	{"spspline",	&CqShaderVM::SO_spspline,	0, },
	{"svspline",	&CqShaderVM::SO_spspline,	0, },
	{"fDu",			&CqShaderVM::SO_fDu,			0, },
	{"fDv",			&CqShaderVM::SO_fDv,			0, },
	{"fDeriv",		&CqShaderVM::SO_fDeriv,		0, },
	{"cDu",			&CqShaderVM::SO_cDu,			0, },
	{"cDv",			&CqShaderVM::SO_cDv,			0, },
	{"cDeriv",		&CqShaderVM::SO_cDeriv,		0, },
	{"pDu",			&CqShaderVM::SO_pDu,			0, },
	{"pDv",			&CqShaderVM::SO_pDv,			0, },
	{"pDeriv",		&CqShaderVM::SO_pDeriv,		0, },
	{"hDu",			&CqShaderVM::SO_pDu,			0, },
	{"hDv",			&CqShaderVM::SO_pDv,			0, },
	{"hDeriv",		&CqShaderVM::SO_pDeriv,		0, },
	{"nDu",			&CqShaderVM::SO_pDu,			0, },
	{"nDv",			&CqShaderVM::SO_pDv,			0, },
	{"nDeriv",		&CqShaderVM::SO_pDeriv,		0, },
	{"vDu",			&CqShaderVM::SO_pDu,			0, },
	{"vDv",			&CqShaderVM::SO_pDv,			0, },
	{"vDeriv",		&CqShaderVM::SO_pDeriv,		0, },
	{"frandom",		&CqShaderVM::SO_frandom,		0, },
	{"crandom",		&CqShaderVM::SO_crandom,		0, },
	{"prandom",		&CqShaderVM::SO_prandom,		0, },
	{"noise1",		&CqShaderVM::SO_noise1,		0, },
	{"noise2",		&CqShaderVM::SO_noise2,		0, },
	{"noise3",		&CqShaderVM::SO_noise3,		0, },
	{"cnoise1",		&CqShaderVM::SO_cnoise1,		0, },
	{"cnoise2",		&CqShaderVM::SO_cnoise2,		0, },
	{"cnoise3",		&CqShaderVM::SO_cnoise3,		0, },
	{"pnoise1",		&CqShaderVM::SO_pnoise1,		0, },
	{"pnoise2",		&CqShaderVM::SO_pnoise2,		0, },
	{"pnoise3",		&CqShaderVM::SO_pnoise3,		0, },
	{"xcomp",		&CqShaderVM::SO_xcomp,		0, },
	{"ycomp",		&CqShaderVM::SO_ycomp,		0, },
	{"zcomp",		&CqShaderVM::SO_zcomp,		0, },
	{"setxcomp",	&CqShaderVM::SO_setxcomp,	0, },
	{"setycomp",	&CqShaderVM::SO_setycomp,	0, },
	{"setzcomp",	&CqShaderVM::SO_setzcomp,	0, },
	{"length",		&CqShaderVM::SO_length,		0, },
	{"distance",	&CqShaderVM::SO_distance,	0, },
	{"area",		&CqShaderVM::SO_area,		0, },
	{"normalize",	&CqShaderVM::SO_normalize,	0, },
	{"faceforward",	&CqShaderVM::SO_faceforward,	0, },
	{"reflect",		&CqShaderVM::SO_reflect,		0, },
	{"refract",		&CqShaderVM::SO_refract,		0, },
	{"fresnel",		&CqShaderVM::SO_fresnel,		0, },
	{"fresnel2",	&CqShaderVM::SO_fresnel2,	0, },
	{"transform2",	&CqShaderVM::SO_transform2,	0, },
	{"transform",	&CqShaderVM::SO_transform,	0, },
	{"transformm",	&CqShaderVM::SO_transformm,	0, },
	{"vtransform2",	&CqShaderVM::SO_vtransform2,	0, },
	{"vtransform",	&CqShaderVM::SO_vtransform,	0, },
	{"vtransformm",	&CqShaderVM::SO_vtransformm,	0, },
	{"ntransform2",	&CqShaderVM::SO_ntransform2,	0, },
	{"ntransform",	&CqShaderVM::SO_ntransform,	0, },
	{"ntransformm",	&CqShaderVM::SO_ntransformm,	0, },
	{"depth",		&CqShaderVM::SO_depth,		0, },
	{"calculatenormal", &CqShaderVM::SO_calculatenormal, 0, },
	{"cmix",		&CqShaderVM::SO_cmix,		0, },
	{"fmix",		&CqShaderVM::SO_fmix,		0, },
	{"pmix",		&CqShaderVM::SO_pmix,		0, },
	{"vmix",		&CqShaderVM::SO_vmix,		0, },
	{"nmix",		&CqShaderVM::SO_nmix,		0, },
	{"comp",		&CqShaderVM::SO_comp,		0, },
	{"setcomp",		&CqShaderVM::SO_setcomp,		0, },
	{"ambient",		&CqShaderVM::SO_ambient,		0, },
	{"diffuse",		&CqShaderVM::SO_diffuse,		0, },
	{"specular",	&CqShaderVM::SO_specular,	0, },
	{"phong",		&CqShaderVM::SO_phong,		0, },
	{"trace",		&CqShaderVM::SO_trace,		0, },
	{"ftexture1",	&CqShaderVM::SO_ftexture1,	0, },
	{"ftexture2",	&CqShaderVM::SO_ftexture2,	0, },
	{"ftexture3",	&CqShaderVM::SO_ftexture3,	0, },
	{"ctexture1",	&CqShaderVM::SO_ctexture1,	0, },
	{"ctexture2",	&CqShaderVM::SO_ctexture2,	0, },
	{"ctexture3",	&CqShaderVM::SO_ctexture3,	0, },
	{"fenvironment2",&CqShaderVM::SO_fenvironment2,0, },
	{"fenvironment3",&CqShaderVM::SO_fenvironment3,0, },
	{"cenvironment2",&CqShaderVM::SO_cenvironment2,0, },
	{"cenvironment3",&CqShaderVM::SO_cenvironment3,0, },
	{"bump1",		&CqShaderVM::SO_bump1,		0, },
	{"bump2",		&CqShaderVM::SO_bump2,		0, },
	{"bump3",		&CqShaderVM::SO_bump3,		0, },
	{"shadow",		&CqShaderVM::SO_shadow,		0, },
	{"shadow2",		&CqShaderVM::SO_shadow1,		0, },
	{"illuminate",	&CqShaderVM::SO_illuminate,	0, },
	{"illuminate2",	&CqShaderVM::SO_illuminate2,	0, },
	{"illuminance",	&CqShaderVM::SO_illuminance,	0, },
	{"illuminance2",&CqShaderVM::SO_illuminance2,0, },
	{"init_illuminance", &CqShaderVM::SO_init_illuminance, 0, },
	{"advance_illuminance", &CqShaderVM::SO_advance_illuminance, 0, },
	{"solar",		&CqShaderVM::SO_solar,		0, },
	{"solar2",		&CqShaderVM::SO_solar2,		0, },
	{"printf",		&CqShaderVM::SO_printf,		0, },

	{"fcellnoise1",	&CqShaderVM::SO_fcellnoise1,	0, },
	{"fcellnoise2",	&CqShaderVM::SO_fcellnoise2,	0, },
	{"fcellnoise3",	&CqShaderVM::SO_fcellnoise3,	0, },
	{"fcellnoise4",	&CqShaderVM::SO_fcellnoise4,	0, },
	{"ccellnoise1",	&CqShaderVM::SO_ccellnoise1,	0, },
	{"ccellnoise2",	&CqShaderVM::SO_ccellnoise2,	0, },
	{"ccellnoise3",	&CqShaderVM::SO_ccellnoise3,	0, },
	{"ccellnoise4",	&CqShaderVM::SO_ccellnoise4,	0, },
	{"pcellnoise1",	&CqShaderVM::SO_pcellnoise1,	0, },
	{"pcellnoise2",	&CqShaderVM::SO_pcellnoise2,	0, },
	{"pcellnoise3",	&CqShaderVM::SO_pcellnoise3,	0, },
	{"pcellnoise4",	&CqShaderVM::SO_pcellnoise4,	0, },

	{"fpnoise1",	&CqShaderVM::SO_fpnoise1,	0, },
	{"fpnoise2",	&CqShaderVM::SO_fpnoise2,	0, },
	{"fpnoise3",	&CqShaderVM::SO_fpnoise3,	0, },
	{"cpnoise1",	&CqShaderVM::SO_cpnoise1,	0, },
	{"cpnoise2",	&CqShaderVM::SO_cpnoise2,	0, },
	{"cpnoise3",	&CqShaderVM::SO_cpnoise3,	0, },
	{"ppnoise1",	&CqShaderVM::SO_ppnoise1,	0, },
	{"pnoise2",		&CqShaderVM::SO_ppnoise2,	0, },
	{"ppnoise3",	&CqShaderVM::SO_ppnoise3,	0, },

	{"atmosphere",	&CqShaderVM::SO_atmosphere,	1, {Type_Variable}},
	{"displacement",&CqShaderVM::SO_displacement,1, {Type_Variable}},
	{"lightsource",	&CqShaderVM::SO_lightsource,	1, {Type_Variable}},
	{"surface",		&CqShaderVM::SO_surface,		1, {Type_Variable}},

	{"attribute",	&CqShaderVM::SO_attribute,	1, {Type_Variable}},
	{"option",		&CqShaderVM::SO_option,		1, {Type_Variable}},
	{"rendererinfo",&CqShaderVM::SO_rendererinfo,1, {Type_Variable}},
	{"incident",	&CqShaderVM::SO_incident,	1, {Type_Variable}},
	{"opposite",	&CqShaderVM::SO_opposite,	1, {Type_Variable}},

	{"ctransform",	&CqShaderVM::SO_ctransform,	0, },
	{"ctransform2",	&CqShaderVM::SO_ctransform2,	0, },

	{"ptlined",		&CqShaderVM::SO_ptlined,		0, },
	{"inversesqrt",	&CqShaderVM::SO_inversesqrt,	0, },
	{"concat",		&CqShaderVM::SO_concat,		0, },
	{"format",		&CqShaderVM::SO_format,		0, },
	{"match",		&CqShaderVM::SO_match,		0, },
	{"rotate",		&CqShaderVM::SO_rotate,		0, },
	{"filterstep",	&CqShaderVM::SO_filterstep,	0, },
	{"filterstep2",	&CqShaderVM::SO_filterstep2,	0, },
	{"specularbrdf",&CqShaderVM::SO_specularbrdf,0, },

	{"mcomp",		&CqShaderVM::SO_mcomp,		0, },
	{"setmcomp",	&CqShaderVM::SO_setmcomp,	0, },
	{"determinant", &CqShaderVM::SO_determinant, 0, },
	{"mtranslate",	&CqShaderVM::SO_mtranslate,	0, },
	{"mrotate",		&CqShaderVM::SO_mrotate,		0, },
	{"mscale",		&CqShaderVM::SO_mscale,		0, },

	{"fsplinea",	&CqShaderVM::SO_fsplinea,	0, },
	{"csplinea",	&CqShaderVM::SO_csplinea,	0, },
	{"psplinea",	&CqShaderVM::SO_psplinea,	0, },
	{"vsplinea",	&CqShaderVM::SO_psplinea,	0, },
	{"sfsplinea",	&CqShaderVM::SO_sfsplinea,	0, },
	{"scsplinea",	&CqShaderVM::SO_scsplinea,	0, },
	{"spsplinea",	&CqShaderVM::SO_spsplinea,	0, },
	{"svsplinea",	&CqShaderVM::SO_spsplinea,	0, },

	{"shadername",	&CqShaderVM::SO_shadername,	0, },
	{"shadername2",	&CqShaderVM::SO_shadername2,	0, },
};
TqInt CqShaderVM::m_cTransSize=sizeof(m_TransTable)/sizeof(m_TransTable[0]);


CqVMStackEntry		gUniformResult(1);
CqVMStackEntry		gVaryingResult(2);


CqShaderVariableArray* CreateNewArray(EqVariableType VarType, char* name, TqInt Count)
{
	CqShaderVariable* pVar=0;
	switch(VarType&Type_Mask)
	{
		case Type_Float:
			if(VarType&Type_Varying)
				pVar=new CqShaderVariableVarying<Type_Float,TqFloat>(name);
			else
				pVar=new CqShaderVariableUniform<Type_Float,TqFloat>(name);
			break;

		case Type_Point:
			if(VarType&Type_Varying)
				pVar=new CqShaderVariableVarying<Type_Point,CqVector3D>(name);
			else
				pVar=new CqShaderVariableUniform<Type_Point,CqVector3D>(name);
			break;

		case Type_Normal:
			if(VarType&Type_Varying)
				pVar=new CqShaderVariableVarying<Type_Normal,CqVector3D>(name);
			else
				pVar=new CqShaderVariableUniform<Type_Normal,CqVector3D>(name);
			break;

		case Type_Vector:
			if(VarType&Type_Varying)
				pVar=new CqShaderVariableVarying<Type_Vector,CqVector3D>(name);
			else
				pVar=new CqShaderVariableUniform<Type_Vector,CqVector3D>(name);
			break;

		case Type_Color:
			if(VarType&Type_Varying)
				pVar=new CqShaderVariableVarying<Type_Color,CqColor>(name);
			else
				pVar=new CqShaderVariableUniform<Type_Color,CqColor>(name);
			break;

		case Type_String:
			if(VarType&Type_Varying)
				pVar=new CqShaderVariableVarying<Type_String,CqString>(name);
			else
				pVar=new CqShaderVariableUniform<Type_String,CqString>(name);
			break;
	}
	CqShaderVariableArray* pArray=new CqShaderVariableArray(name, Count);
	pArray->aVariables()[0]=pVar;
	TqInt i;
	for(i=1; i<Count; i++)
		pArray->aVariables()[i]=pVar->Clone();

	return(pArray);
}

//---------------------------------------------------------------------
/** Load a token from a compiled slx file.
 */

void CqShaderVM::GetToken(char* token, TqInt l, std::istream* pFile)
{
	char c;
	TqInt i=0;
	(*pFile) >> std::ws;
	c=pFile->get();
	if(c==':' && i==0)
	{
		token[0]=c;
		token[1]='\0';
		return;	// Special case for labels.
	}
	while(!isspace(c) && i<l-1)
	{
		token[i++]=c;
		token[i]='\0';
		c=pFile->get();
	}
}


//---------------------------------------------------------------------
/** Load a program from a compiled slc file.
 */

void CqShaderVM::LoadProgram(std::istream* pFile)
{
	enum EqSegment
	{
		Seg_Data=0,
		Seg_Init,
		Seg_Code,
	};
	char token[255];
	EqSegment	Segment;
	std::vector<UsProgramElement>*	pProgramArea;
	std::vector<TqInt>	aLabels;
	CqShaderExecEnv	StdEnv;
	TqInt	array_count=0;
	
	TqBool fShaderSpec=TqFalse;
	while(!pFile->eof())
	{
		GetToken(token,255,pFile);

		// Check for type and version information.
		if(!fShaderSpec)
		{
			TqInt i;
			for(i=0; i<gcShaderTypeNames; i++)
			{
				if(strcmp(token,gShaderTypeNames[i])==0)
				{
					// TODO: Should store the type so that it can be checked.
					fShaderSpec=TqTrue;
					break;
				}
			}
			if(fShaderSpec)	continue;
		}
		
		if(strcmp(token, "AQSIS")==0)
		{
			GetToken(token,255,pFile);
			
			// Get the version information.
//			CqString strVersion(token);
//			TqInt vMaj, vMin, build;
//			GET_VERSION_FROM_STRING(vMaj,vMin,build);
//			if(CHECK_NEWER_VERSION(vMaj,vMin,build))
//			{
//				CqBasicError(0,Severity_Fatal,"SLX built by more recent version of Aqsis");
//				return;
//			}			  
		}

		if(strcmp(token, "USES")==0)
		{
			(*pFile) >> m_Uses;
			continue;
		}

		if(strcmp(token,"segment")==0)
		{
			GetToken(token,255,pFile);
			if(strcmp(token,"Data")==0)
				Segment=Seg_Data;
			else if(strcmp(token,"Init")==0)
			{
				Segment=Seg_Init;
				pProgramArea=&m_ProgramInit;
				aLabels.clear();
			}
			else if(strcmp(token,"Code")==0)
			{
				Segment=Seg_Code;
				pProgramArea=&m_Program;
				aLabels.clear();
			}	
		}
		else
		{
			EqVariableType VarType=Type_Varying;
			switch(Segment)
			{
				case Seg_Data:
					VarType=Type_Nil;
					while((VarType&Type_Mask)==Type_Nil)
					{
						if(strcmp(token,"param")==0)
							VarType=VarType;	// Do nothing.
						else if(strcmp(token,"varying")==0)
							VarType=Type_Varying;
						else if(strcmp(token,"uniform")==0)
							VarType=Type_Uniform;
						else if(strcmp(token,gVariableTypeNames[Type_Float])==0)
							VarType=(EqVariableType)(VarType|Type_Float);
						else if(strcmp(token,gVariableTypeNames[Type_Point])==0)
							VarType=(EqVariableType)(VarType|Type_Point);
						else if(strcmp(token,gVariableTypeNames[Type_Color])==0)
							VarType=(EqVariableType)(VarType|Type_Color);
						else if(strcmp(token,gVariableTypeNames[Type_String])==0)
							VarType=(EqVariableType)(VarType|Type_String);
						else if(strcmp(token,gVariableTypeNames[Type_Normal])==0)
							VarType=(EqVariableType)(VarType|Type_Normal);
						else if(strcmp(token,gVariableTypeNames[Type_Vector])==0)
							VarType=(EqVariableType)(VarType|Type_Vector);
						else if(strcmp(token,gVariableTypeNames[Type_Matrix])==0)
							VarType=(EqVariableType)(VarType|Type_Matrix);
						else break;

						GetToken(token,255,pFile);
					}
					// Check for array type variable.
					if(token[strlen(token)-1]==']')
					{
						unsigned int i=0;
						while(i<strlen(token) && token[i]!='[')	i++;
						if(i==strlen(token))
						{
							CqBasicError(0,Severity_Fatal,"Invalid variable specification in slx file");
							return;
						}
						token[strlen(token)-1]='\0';
						token[i]='\0';
						i++;
						array_count=atoi(&token[i]);
						VarType=(EqVariableType)(VarType|Type_Array);
					}
					// Check if there is a valid variable specifier
					if((VarType&Storage_Mask)==0 ||
					   (VarType&Type_Mask)==Type_Nil)
						continue;

					switch(VarType&Type_Mask)
					{
						case Type_Float:
							if(VarType&Type_Array)
								AddLocalVariable(CreateNewArray(VarType,token, array_count));
							else if(VarType&Type_Varying)
								AddLocalVariable(new CqShaderVariableVarying<Type_Float,TqFloat>(token));
							else
								AddLocalVariable(new CqShaderVariableUniform<Type_Float,TqFloat>(token));
							break;

						case Type_Point:
							if(VarType&Type_Array)
								AddLocalVariable(CreateNewArray(VarType,token,array_count));
							else if(VarType&Type_Varying)
								AddLocalVariable(new CqShaderVariableVarying<Type_Point,CqVector3D>(token));
							else
								AddLocalVariable(new CqShaderVariableUniform<Type_Point,CqVector3D>(token));
							break;

						case Type_Normal:
							if(VarType&Type_Array)
								AddLocalVariable(CreateNewArray(VarType,token,array_count));
							else if(VarType&Type_Varying)
								AddLocalVariable(new CqShaderVariableVarying<Type_Normal,CqVector3D>(token));
							else
								AddLocalVariable(new CqShaderVariableUniform<Type_Normal,CqVector3D>(token));
							break;

						case Type_Vector:
							if(VarType&Type_Array)
								AddLocalVariable(CreateNewArray(VarType,token,array_count));
							else if(VarType&Type_Varying)
								AddLocalVariable(new CqShaderVariableVarying<Type_Vector,CqVector3D>(token));
							else
								AddLocalVariable(new CqShaderVariableUniform<Type_Vector,CqVector3D>(token));
							break;

						case Type_Color:
							if(VarType&Type_Array)
								AddLocalVariable(CreateNewArray(VarType,token,array_count));
							else if(VarType&Type_Varying)
								AddLocalVariable(new CqShaderVariableVarying<Type_Color,CqColor>(token));
							else
								AddLocalVariable(new CqShaderVariableUniform<Type_Color,CqColor>(token));
							break;

						case Type_String:
							if(VarType&Type_Array)
								AddLocalVariable(CreateNewArray(VarType,token,array_count));
							else if(VarType&Type_Varying)
								AddLocalVariable(new CqShaderVariableVarying<Type_String,CqString>(token));
							else
								AddLocalVariable(new CqShaderVariableUniform<Type_String,CqString>(token));
							break;

						case Type_Matrix:
							if(VarType&Type_Array)
								AddLocalVariable(CreateNewArray(VarType,token,array_count));
							else if(VarType&Type_Varying)
								AddLocalVariable(new CqShaderVariableVarying<Type_Matrix,CqMatrix>(token));
							else
								AddLocalVariable(new CqShaderVariableUniform<Type_Matrix,CqMatrix>(token));
							break;
					}
					break;

				case Seg_Init:
				case Seg_Code:
					// Check if it is a label
					if(strcmp(token,":")==0)
					{
						(*pFile) >> std::ws;
						TqFloat f;
						(*pFile) >> f;
						if(aLabels.size()<(f+1))
							aLabels.resize(f+1);
						aLabels[f]=pProgramArea->size();
						AddCommand(&CqShaderVM::SO_nop,pProgramArea);
						break;
					}
					// Find the opcode in the translation table.
					TqInt i;
					for(i=0; i<m_cTransSize; i++)
					{
						if(strcmp(m_TransTable[i].m_strName,token)==0)
						{
							// If the opcodes command pointer is 0, just ignore this opcode.
							if(m_TransTable[i].m_pCommand==0)
								break;

							// Add this opcode to the program segment.
							AddCommand(m_TransTable[i].m_pCommand,pProgramArea);
							// Process this opcodes parameters.
							TqInt p;
							for(p=0; p<m_TransTable[i].m_cParams; p++)
							{
								if(m_TransTable[i].m_aParamTypes[p]&Type_Variable)
								{
									GetToken(token,255,pFile);
									TqInt iVar;
									if((iVar=FindLocalVarIndex(token))>=0)
										AddVariable(iVar,pProgramArea);
									else if((iVar=StdEnv.FindStandardVarIndex(token))>=0)
										AddVariable(iVar|0x8000,pProgramArea);
									else
										// TODO: Report error.
										AddVariable(0,pProgramArea);
								}
								else
								{
									switch(m_TransTable[i].m_aParamTypes[p]&Type_Mask)
									{
										case Type_Float:
											(*pFile) >> std::ws;
											TqFloat f;
											(*pFile) >> f;
											AddFloat(f,pProgramArea);
											break;

										case Type_String:
											(*pFile) >> std::ws;
											char c;
											CqString s("");
											pFile->get();
											while((c=pFile->get())!='"')
												s+=c;
											AddString(s.c_str(),pProgramArea);
											break;
									}
								}
							}
							break;
						}
					}
					// If we have not found the opcode, throw an error.
					if(i==m_cTransSize)
					{
						CqString strErr("Invlaid opcode found : ");
						strErr+=token;
						CqBasicError(0,Severity_Fatal,strErr.c_str());
						return;
					}
					break;
			}
		}
		(*pFile) >> std::ws;
	}
	// Now we need to complete any label jump statements.
	TqInt i=0;
	while(i<m_Program.size())
	{
		UsProgramElement E=m_Program[i++];
		if(E.m_Command==&CqShaderVM::SO_jnz ||
		   E.m_Command==&CqShaderVM::SO_jmp ||
		   E.m_Command==&CqShaderVM::SO_jz ||
		   E.m_Command==&CqShaderVM::SO_RS_JZ ||
		   E.m_Command==&CqShaderVM::SO_RS_JNZ ||
		   E.m_Command==&CqShaderVM::SO_S_JZ ||
		   E.m_Command==&CqShaderVM::SO_S_JNZ)
		{
			SqLabel lab;
			lab.m_Offset=aLabels[m_Program[i].m_FloatVal];
			lab.m_pAddress=&m_Program[lab.m_Offset];
			m_Program[i].m_Label=lab;
			i++;
		}
		else
		{
			// Find the command so that we can skip the parameters
			TqInt j;
			for(j=0; j<m_cTransSize; j++)
			{
				if(m_TransTable[j].m_pCommand==E.m_Command)
				{
					i+=m_TransTable[j].m_cParams;
					break;
				}
			}
		}
	}
}


//---------------------------------------------------------------------
/**	Ready the shader for execution.
 */

void CqShaderVM::Initialise(const TqInt uGridRes, const TqInt vGridRes, CqShaderExecEnv& Env)			
{
	CqShader::Initialise(uGridRes,vGridRes,Env);
	m_pEnv=&Env;
	// Initialise local variables.
	TqInt i;
	for(i=m_LocalVars.size()-1; i>=0; i--)
		m_LocalVars[i]->Initialise(uGridRes, vGridRes, Env.GridI());

	gVaryingResult.SetSize((uGridRes+1)*(vGridRes+1));
	
	// Reset the program counter.
	m_PC=0;
}


//---------------------------------------------------------------------
/**	Assignment operator.
 */

CqShaderVM&	CqShaderVM::operator=(const CqShaderVM& From)
{
	// Copy the local variables...
	TqInt i;
	for(i=0; i<From.m_LocalVars.size(); i++)
		m_LocalVars.push_back(From.m_LocalVars[i]->Clone());

	// Copy the intialisation program.
	for(i=0; i<From.m_ProgramInit.size(); i++)
		m_ProgramInit.push_back(From.m_ProgramInit[i]);
	
	// Copy the main program.
	for(i=0; i<From.m_Program.size(); i++)
		m_Program.push_back(From.m_Program[i]);
	
	return(*this);
}


//---------------------------------------------------------------------
/**	Execute a series of shader language bytecodes.
 */

void CqShaderVM::Execute(CqShaderExecEnv& Env)
{
	// Check if there is anything to execute.
	if(m_Program.size()<=0)
		return;

	m_pEnv=&Env;

	Env.InvalidateIlluminanceCache();

	// Execute the main program.
	m_PC=&m_Program[0];
	m_PO=0;
	m_PE=m_Program.size();
	UsProgramElement* pE;

	while(!fDone())
	{
		pE=&ReadNext();
		(this->*pE->m_Command)();
	}
	// Check that the stack is empty.
	assert(m_iTop==0);
	m_Stack.clear();
}


//---------------------------------------------------------------------
/**	Execute the program segment which initialises the default values of instance variables.
 */

void CqShaderVM::ExecuteInit()
{
	// Check if there is anything to execute.
	if(m_ProgramInit.size()<=0)
		return;
	
	// Fake an environment
	CqShaderExecEnv	Env;
	Env.Initialise(1,1,0,m_Uses);
	Initialise(1,1,Env);
	
	// Execute the init program.
	m_PC=&m_ProgramInit[0];
	m_PO=0;
	m_PE=m_ProgramInit.size();
	UsProgramElement* pE;

	while(!fDone())
	{
		pE=&ReadNext();
		(this->*pE->m_Command)();
	}
	// Check that the stack is empty.
	assert(m_iTop==0);
	m_Stack.clear();
}


//---------------------------------------------------------------------
/** Set the instance variables on this shader.
 */

void CqShaderVM::SetValue(const char* name, TqPchar val)
{
	// Find the relevant variable.
	SqParameterDeclaration Decl=pCurrentRenderer()->FindParameterDecl(name);
	TqInt i=FindLocalVarIndex(Decl.m_strName.c_str());
	if(i>=0)
	{
		int index=0,count=1,arrayindex=0;
		CqShaderVariableArray* pArray=0;
	
		if(m_LocalVars[i]->Type()&Type_Array)
		{
			pArray=static_cast<CqShaderVariableArray*>(m_LocalVars[i]);
			count=pArray->ArrayLength();
		}

		while(count-->0)
		{
			CqVMStackEntry VMVal;
			switch(m_LocalVars[i]->Type()&Type_Mask)
			{
				case	Type_Float:
					VMVal=reinterpret_cast<TqFloat*>(val)[index++];
					break;

				case	Type_Point:
				case	Type_Normal:
				case	Type_Vector:
					VMVal=CqVector3D(reinterpret_cast<TqFloat*>(val)[index+0],reinterpret_cast<TqFloat*>(val)[index+1],reinterpret_cast<TqFloat*>(val)[index+2]);
					index+=3;
					break;

				case	Type_hPoint:
					VMVal=CqVector4D(reinterpret_cast<TqFloat*>(val)[index+0],reinterpret_cast<TqFloat*>(val)[index+1],reinterpret_cast<TqFloat*>(val)[index+2],reinterpret_cast<TqFloat*>(val)[index+3]);
					index+=4;
					break;

				case	Type_Color:
					VMVal=CqColor(reinterpret_cast<TqFloat*>(val)[index+0],reinterpret_cast<TqFloat*>(val)[index+1],reinterpret_cast<TqFloat*>(val)[index+2]);
					index+=3;
					break;

				case	Type_Matrix:
					VMVal=CqMatrix(reinterpret_cast<TqFloat*>(val)[index+ 0],reinterpret_cast<TqFloat*>(val)[index+ 1],reinterpret_cast<TqFloat*>(val)[index+ 2],reinterpret_cast<TqFloat*>(val)[index+ 3],
								   reinterpret_cast<TqFloat*>(val)[index+ 4],reinterpret_cast<TqFloat*>(val)[index+ 5],reinterpret_cast<TqFloat*>(val)[index+ 6],reinterpret_cast<TqFloat*>(val)[index+ 7],
								   reinterpret_cast<TqFloat*>(val)[index+ 8],reinterpret_cast<TqFloat*>(val)[index+ 9],reinterpret_cast<TqFloat*>(val)[index+10],reinterpret_cast<TqFloat*>(val)[index+11],
								   reinterpret_cast<TqFloat*>(val)[index+12],reinterpret_cast<TqFloat*>(val)[index+13],reinterpret_cast<TqFloat*>(val)[index+14],reinterpret_cast<TqFloat*>(val)[index+15]);
					index+=16;
					break;

				case	Type_String:
					VMVal=reinterpret_cast<char**>(val)[index++];
					break;
			}
			
			// If it is a color or a point, ensure it is the correct 'space'
			if((m_LocalVars[i]->Type()&Type_Mask)==Type_Point || (m_LocalVars[i]->Type()&Type_Mask)==Type_hPoint)
			{
				CqString strSpace("shader");
				CqVector3D p;
				if(Decl.m_strName!="" && Decl.m_strSpace!="")
					strSpace=Decl.m_strSpace;
				VMVal=pCurrentRenderer()->matSpaceToSpace(strSpace.c_str(), "camera", matCurrent(), pCurrentRenderer()->matCurrent())*VMVal.Value(p,0);
			}
			else if((m_LocalVars[i]->Type()&Type_Mask)==Type_Normal)
			{
				CqString strSpace("shader");
				CqVector3D p;
				if(Decl.m_strName!="" && Decl.m_strSpace!="")
					strSpace=Decl.m_strSpace;
				VMVal=pCurrentRenderer()->matNSpaceToSpace(strSpace.c_str(), "camera", matCurrent(), pCurrentRenderer()->matCurrent())*VMVal.Value(p,0);
			}
			else if((m_LocalVars[i]->Type()&Type_Mask)==Type_Vector)
			{
				CqString strSpace("shader");
				CqVector3D p;
				if(Decl.m_strName!="" && Decl.m_strSpace!="")
					strSpace=Decl.m_strSpace;
				VMVal=pCurrentRenderer()->matVSpaceToSpace(strSpace.c_str(), "camera", matCurrent(), pCurrentRenderer()->matCurrent())*VMVal.Value(p,0);
			}
			else if((m_LocalVars[i]->Type()&Type_Mask)==Type_Matrix)
			{
				CqString strSpace("shader");
				CqMatrix m;
				if(Decl.m_strName!="" && Decl.m_strSpace!="")
					strSpace=Decl.m_strSpace;
				VMVal=pCurrentRenderer()->matVSpaceToSpace(strSpace.c_str(), "camera", matCurrent(), pCurrentRenderer()->matCurrent())*VMVal.Value(m,0);
			}

			if(pArray)	(*pArray)[arrayindex++]->SetValue(VMVal);
			else		m_LocalVars[i]->SetValue(VMVal);
		}
	}
}


//---------------------------------------------------------------------
/** Get a value from an instance variable on this shader, and fill in the passed variable reference.
 */

TqBool CqShaderVM::GetValue(const char* name, CqShaderVariable* res)
{
	// Find the relevant variable.
	TqInt i=FindLocalVarIndex(name);
	if(i>=0)
	{
		CqVMStackEntry VMVal;
		// TODO: Should check for varying here, and avoid overhead of repeated GetValue calls.
		TqInt j;
		for(j=0; j<m_pEnv->GridSize(); j++)
			m_LocalVars[i]->GetValue(j,VMVal);
		res->SetValue(VMVal);
		return(TqTrue);
	}
	return(TqFalse);
}



void CqShaderVM::SO_nop()					
{
}

void CqShaderVM::SO_dup()					
{
	Dup();
}

void CqShaderVM::SO_drop()					
{
	Drop();
}

void CqShaderVM::SO_debug_break()			
{
}

void CqShaderVM::SO_pushif()					
{
	Push(ReadNext().m_FloatVal);
}

void CqShaderVM::SO_puship()					
{
	TqFloat f=ReadNext().m_FloatVal; 
	TqFloat f2=ReadNext().m_FloatVal; 
	TqFloat f3=ReadNext().m_FloatVal; 
	Push(f,f2,f3);
}

void CqShaderVM::SO_pushis()					
{
	CqString* ps=ReadNext().m_pString; 
	Push(*ps);
}

void CqShaderVM::SO_pushv()					
{
	Push(GetVar(ReadNext().m_iVariable));
}

void CqShaderVM::SO_ipushv()					
{
	AUTOFUNC;
	POPV(A);	// Index
	RESULT;
	CqShaderVariable* pVar=GetVar(ReadNext().m_iVariable);
	if(pVar->Type()&Type_Array==0)
	{
		// Report error.
		CqBasicError(0,Severity_Fatal,"Attempt to index a non array variable");
		return;
	}
	CqShaderVariableArray* pVarArray=static_cast<CqShaderVariableArray*>(pVar);
	//TqInt ext=__fVarying?m_pEnv->GridSize():1;
	TqInt ext=m_pEnv->GridSize();
	TqInt i;
	for(i=0; i<ext; i++)
		(*pVarArray)[FLOAT(A)]->GetValue(i,Result);
	Push(Result);
}

void CqShaderVM::SO_pop()					
{
	AUTOFUNC;
	CqShaderVariable* pV=GetVar(ReadNext().m_iVariable); 
	POPV(Val);
	pV->SetValue(Val,m_pEnv->RunningState());
}

void CqShaderVM::SO_ipop()					
{
	AUTOFUNC;
	UsProgramElement& el=ReadNext();
	CqShaderVariable* pV=GetVar(el.m_iVariable);
	CqShaderVariableArray* pVA=static_cast<CqShaderVariableArray*>(pV);
	if(pV->Type()&Type_Array==0)
	{
		// Report error.
		CqBasicError(0,Severity_Fatal,"Attempt to index a non array variable");
		return;
	}
	POPV(A);
	POPV(Val);
	//TqInt ext=__fVarying?m_pEnv->GridSize():1;
	TqInt ext=m_pEnv->GridSize();
	TqInt i;
	for(i=0; i<ext; i++)
	{
		if(m_pEnv->RunningState().Value(i))
			(*pVA)[FLOAT(A)]->SetValue(i,Val);
	}
}

void CqShaderVM::SO_mergef()					
{
	// Get the current state from the current stack entry
	AUTOFUNC;
	POPV(A);	// Relational result
	POPV(F);	// False statement
	POPV(T);	// True statement
	RESULT;
	TqInt i;
	for(i=0; i<m_pEnv->GridSize(); i++)
	{
		if(BOOLEAN(A))	Result.SetValue(i,FLOAT(T));
		else			Result.SetValue(i,FLOAT(F));
	}
	Push(Result);
}

void CqShaderVM::SO_merges()					
{
	// Get the current state from the current stack entry
	AUTOFUNC;
	POPV(A);	// Relational result
	POPV(F);	// False statement
	POPV(T);	// True statement
	RESULT;
	TqInt i;
	for(i=0; i<m_pEnv->GridSize(); i++)
	{
		if(BOOLEAN(A))	Result.SetValue(i,STRING(T));
		else			Result.SetValue(i,STRING(F));
	}
	Push(Result);
}

void CqShaderVM::SO_mergep()					
{
	// Get the current state from the current stack entry
	AUTOFUNC;
	POPV(A);	// Relational result
	POPV(F);	// False statement
	POPV(T);	// True statement
	RESULT;
	TqInt i;
	for(i=0; i<m_pEnv->GridSize(); i++)
	{
		if(BOOLEAN(A))	Result.SetValue(i,POINT(T));
		else			Result.SetValue(i,POINT(F));
	}
	Push(Result);
}

void CqShaderVM::SO_mergec()					
{
	// Get the current state from the current stack entry
	AUTOFUNC;
	POPV(A);	// Relational result
	POPV(F);	// False statement
	POPV(T);	// True statement
	RESULT;
	TqInt i;
	for(i=m_pEnv->GridSize()-1; i>=0; i--)
	{
		if(BOOLEAN(A))	Result.SetValue(i,COLOR(T));
		else			Result.SetValue(i,COLOR(F));
	}
	Push(Result);
}

void CqShaderVM::SO_setfc()					
{
	AUTOFUNC; 
	POPV(A); 
	RESULT; 
	A.OpCAST_FC(Result,m_pEnv->RunningState()); 
	Push(Result);
}

void CqShaderVM::SO_setfp()					
{
	AUTOFUNC; 
	POPV(A); 
	RESULT; 
	A.OpCAST_FP(Result,m_pEnv->RunningState()); 
	Push(Result);
}


void CqShaderVM::SO_setfm()					
{
	AUTOFUNC; 
	POPV(A); 
	RESULT; 
	A.OpCAST_FM(Result,m_pEnv->RunningState()); 
	Push(Result);
}

void CqShaderVM::SO_settc()					
{
	AUTOFUNC; 
	POPV(A); 
	POPV(B); 
	POPV(C); 
	RESULT; 
	Result.OpTRIPLE_C(A,B,C,m_pEnv->RunningState()); 
	Push(Result);
}

void CqShaderVM::SO_settp()					
{
	AUTOFUNC; 
	POPV(A); 
	POPV(B); 
	POPV(C); 
	RESULT; 
	Result.OpTRIPLE_P(A,B,C,m_pEnv->RunningState()); 
	Push(Result);
}

void CqShaderVM::SO_setpc()					
{
	AUTOFUNC; 
	POPV(A); 
	RESULT; 
	A.OpCAST_PC(Result,m_pEnv->RunningState()); 
	Push(Result);
}

void CqShaderVM::SO_setcp()					
{
	AUTOFUNC; 
	POPV(A); 
	RESULT; 
	A.OpCAST_CP(Result,m_pEnv->RunningState()); 
	Push(Result);
}

void CqShaderVM::SO_setwm()					
{
	AUTOFUNC; 
	POPV(A); 
	POPV(B); 
	POPV(C); 
	POPV(D); 
	POPV(E); 
	POPV(F); 
	POPV(G); 
	POPV(H); 
	POPV(I); 
	POPV(J); 
	POPV(K); 
	POPV(L); 
	POPV(M); 
	POPV(N); 
	POPV(O); 
	POPV(P); 
	RESULT; 
	Result.OpHEXTUPLE_M(P,O,N,M,L,K,J,I,H,G,F,E,D,C,B,A,m_pEnv->RunningState()); 
	Push(Result);
}

void CqShaderVM::SO_RS_PUSH()				
{
	m_pEnv->PushState();
}

void CqShaderVM::SO_RS_POP()					
{
	m_pEnv->PopState();
}

void CqShaderVM::SO_RS_GET()					
{
	m_pEnv->GetCurrentState();
}

void CqShaderVM::SO_RS_INVERSE()				
{
	m_pEnv->InvertRunningState();
}

void CqShaderVM::SO_S_CLEAR()				
{
	m_pEnv->ClearCurrentState();
}

void CqShaderVM::SO_S_GET()					
{
	// Get the current state from the current stack entry
	AUTOFUNC;
	POPV(ValA);
	TqInt i;
	for(i=m_pEnv->GridSize()-1; i>=0; i--)
	{
		if(m_pEnv->RunningState().Value(i))
			m_pEnv->CurrentState().SetValue(i,BOOLEAN(ValA));
	}
}

void CqShaderVM::SO_RS_JZ()					
{
	SqLabel lab=ReadNext().m_Label;
	if(m_pEnv->RunningState().Count()==0)
	{
		m_PO=lab.m_Offset;
		m_PC=lab.m_pAddress;
	}
}

void CqShaderVM::SO_RS_JNZ()					
{
	SqLabel lab=ReadNext().m_Label;
	if(m_pEnv->RunningState().Count()==m_pEnv->RunningState().Size())
	{
		m_PO=lab.m_Offset;
		m_PC=lab.m_pAddress;
	}
}

void CqShaderVM::SO_S_JZ()					
{
	SqLabel lab=ReadNext().m_Label;
	if(m_pEnv->CurrentState().Count()==0)
	{
		m_PO=lab.m_Offset;
		m_PC=lab.m_pAddress;
	}
}

void CqShaderVM::SO_S_JNZ()					
{
	SqLabel lab=ReadNext().m_Label;
	if(m_pEnv->CurrentState().Count()==m_pEnv->RunningState().Size())
	{
		m_PO=lab.m_Offset;
		m_PC=lab.m_pAddress;
	}
}

void CqShaderVM::SO_jnz()					
{
	SqLabel lab=ReadNext().m_Label;
	AUTOFUNC;
	CqVMStackEntry f=POP;
	m_pEnv->Reset();
	do
	{
		TqInt i=m_pEnv->GridI();
		if(!__fVarying || m_pEnv->RunningState().Value(i)) \
			if(!BOOLEAN(f))	return;
	}while(m_pEnv->Advance());
	m_PO=lab.m_Offset;
	m_PC=lab.m_pAddress;
}

void CqShaderVM::SO_jz()						
{
	SqLabel lab=ReadNext().m_Label;
	AUTOFUNC;
	CqVMStackEntry f=POP;
	m_pEnv->Reset();
	do
	{
		TqInt i=m_pEnv->GridI();
		if(!__fVarying || m_pEnv->RunningState().Value(i)) \
			if(BOOLEAN(f))	return;
	}while(m_pEnv->Advance());
	m_PO=lab.m_Offset;
	m_PC=lab.m_pAddress;
}

void CqShaderVM::SO_jmp()					
{
	SqLabel lab=ReadNext().m_Label;
	m_PO=lab.m_Offset;
	m_PC=lab.m_pAddress;
}
									
void CqShaderVM::SO_lsff()					
{
	AUTOFUNC; 
	POPV(A); 
	POPV(B); 
	RESULT; 
	A.OpLSS_FF(B,Result,m_pEnv->RunningState()); 
	Push(Result);
}

void CqShaderVM::SO_lspp()					
{
	AUTOFUNC; 
	POPV(A); 
	POPV(B); 
	RESULT; 
	A.OpLSS_PP(B,Result,m_pEnv->RunningState()); 
	Push(Result);
}

void CqShaderVM::SO_lscc()					
{
	AUTOFUNC; 
	POPV(A); 
	POPV(B); 
	RESULT; 
	A.OpLSS_CC(B,Result,m_pEnv->RunningState()); 
	Push(Result);
}
									
void CqShaderVM::SO_gtff()					
{
	AUTOFUNC; 
	POPV(A); 
	POPV(B); 
	RESULT; 
	A.OpGRT_FF(B,Result,m_pEnv->RunningState()); 
	Push(Result);
}

void CqShaderVM::SO_gtpp()					
{
	AUTOFUNC; 
	POPV(A); 
	POPV(B); 
	RESULT; 
	A.OpGRT_PP(B,Result,m_pEnv->RunningState()); 
	Push(Result);
}

void CqShaderVM::SO_gtcc()					
{
	AUTOFUNC; 
	POPV(A); 
	POPV(B); 
	RESULT; 
	A.OpGRT_CC(B,Result,m_pEnv->RunningState()); 
	Push(Result);
}
									
void CqShaderVM::SO_geff()					
{
	AUTOFUNC; 
	POPV(A); 
	POPV(B); 
	RESULT; 
	A.OpGE_FF(B,Result,m_pEnv->RunningState()); 
	Push(Result);
}

void CqShaderVM::SO_gepp()					
{
	AUTOFUNC; 
	POPV(A); 
	POPV(B); 
	RESULT; 
	A.OpGE_PP(B,Result,m_pEnv->RunningState()); 
	Push(Result);
}

void CqShaderVM::SO_gecc()					
{
	AUTOFUNC; 
	POPV(A); 
	POPV(B); 
	RESULT; 
	A.OpGE_CC(B,Result,m_pEnv->RunningState()); 
	Push(Result);
}
									
void CqShaderVM::SO_leff()					
{
	AUTOFUNC; 
	POPV(A); 
	POPV(B); 
	RESULT; 
	A.OpLE_FF(B,Result,m_pEnv->RunningState()); 
	Push(Result);
}

void CqShaderVM::SO_lepp()					
{
	AUTOFUNC; 
	POPV(A); 
	POPV(B); 
	RESULT; 
	A.OpLE_PP(B,Result,m_pEnv->RunningState()); 
	Push(Result);
}

void CqShaderVM::SO_lecc()					
{
	AUTOFUNC; 
	POPV(A); 
	POPV(B); 
	RESULT; 
	A.OpLE_CC(B,Result,m_pEnv->RunningState()); 
	Push(Result);
}

void CqShaderVM::SO_eqff()					
{
	AUTOFUNC; 
	POPV(A); 
	POPV(B); 
	RESULT; 
	A.OpEQ_FF(B,Result,m_pEnv->RunningState()); 
	Push(Result);
}

void CqShaderVM::SO_eqpp()					
{
	AUTOFUNC; 
	POPV(A); 
	POPV(B); 
	RESULT; 
	A.OpEQ_PP(B,Result,m_pEnv->RunningState()); 
	Push(Result);
}

void CqShaderVM::SO_eqcc()					
{
	AUTOFUNC; 
	POPV(A); 
	POPV(B); 
	RESULT; 
	A.OpEQ_CC(B,Result,m_pEnv->RunningState()); 
	Push(Result);
}

void CqShaderVM::SO_eqss()					
{
	AUTOFUNC; 
	POPV(A); 
	POPV(B); 
	RESULT; 
	A.OpEQ_SS(B,Result,m_pEnv->RunningState()); 
	Push(Result);
}
									
void CqShaderVM::SO_neff()					
{
	AUTOFUNC; 
	POPV(A); 
	POPV(B); 
	RESULT; 
	A.OpNE_FF(B,Result,m_pEnv->RunningState()); 
	Push(Result);
}

void CqShaderVM::SO_nepp()					
{
	AUTOFUNC; 
	POPV(A); 
	POPV(B); 
	RESULT; 
	A.OpNE_PP(B,Result,m_pEnv->RunningState()); 
	Push(Result);
}

void CqShaderVM::SO_necc()					
{
	AUTOFUNC; 
	POPV(A); 
	POPV(B); 
	RESULT; 
	A.OpNE_CC(B,Result,m_pEnv->RunningState()); 
	Push(Result);
}

void CqShaderVM::SO_ness()					
{
	AUTOFUNC; 
	POPV(A); 
	POPV(B); 
	RESULT; 
	A.OpNE_SS(B,Result,m_pEnv->RunningState()); 
	Push(Result);
}
									
void CqShaderVM::SO_mulff()					
{
	AUTOFUNC; 
	POPV(A); 
	POPV(B); 
	RESULT; 
	A.OpMUL_FF(B,Result,m_pEnv->RunningState()); 
	Push(Result);
}

void CqShaderVM::SO_divff()					
{
	AUTOFUNC; 
	POPV(A); 
	POPV(B); 
	RESULT; 
	A.OpDIV_FF(B,Result,m_pEnv->RunningState()); 
	Push(Result);
}

void CqShaderVM::SO_addff()					
{
	AUTOFUNC; 
	POPV(A); 
	POPV(B); 
	RESULT; 
	A.OpADD_FF(B,Result,m_pEnv->RunningState()); 
	Push(Result);
}

void CqShaderVM::SO_subff()					
{
	AUTOFUNC; 
	POPV(A); 
	POPV(B); 
	RESULT; 
	A.OpSUB_FF(B,Result,m_pEnv->RunningState()); 
	Push(Result);
}

void CqShaderVM::SO_negf()					
{
	AUTOFUNC; 
	POPV(A); 
	RESULT; 
	A.OpNEG_F(Result,m_pEnv->RunningState()); 
	Push(Result);
}
									
void CqShaderVM::SO_mulpp()					
{
	AUTOFUNC; 
	POPV(A); 
	POPV(B); 
	RESULT; 
	A.OpMULV(B,Result,m_pEnv->RunningState()); 
	Push(Result);
}

void CqShaderVM::SO_divpp()					
{
	AUTOFUNC; 
	POPV(A); 
	POPV(B); 
	RESULT; 
	A.OpDIV_PP(B,Result,m_pEnv->RunningState()); 
	Push(Result);
}

void CqShaderVM::SO_addpp()					
{
	AUTOFUNC; 
	POPV(A); 
	POPV(B); 
	RESULT; 
	A.OpADD_PP(B,Result,m_pEnv->RunningState()); 
	Push(Result);
}

void CqShaderVM::SO_subpp()					
{
	AUTOFUNC; POPV(A); 
	POPV(B); 
	RESULT; 
	A.OpSUB_PP(B,Result,m_pEnv->RunningState()); 
	Push(Result);
}

void CqShaderVM::SO_crspp()					
{
	AUTOFUNC; 
	POPV(A); 
	POPV(B); 
	RESULT; 
	A.OpCRS_PP(B,Result,m_pEnv->RunningState()); 
	Push(Result);
}

void CqShaderVM::SO_dotpp()					
{
	AUTOFUNC; 
	POPV(A); 
	POPV(B); 
	RESULT; 
	A.OpDOT_PP(B,Result,m_pEnv->RunningState()); 
	Push(Result);
}

void CqShaderVM::SO_negp()					
{
	AUTOFUNC; 
	POPV(A); 
	RESULT; 
	A.OpNEG_P(Result,m_pEnv->RunningState()); 
	Push(Result);
}
									
void CqShaderVM::SO_mulcc()					
{
	AUTOFUNC; 
	POPV(A); 
	POPV(B); 
	RESULT; 
	A.OpMUL_CC(B,Result,m_pEnv->RunningState()); 
	Push(Result);
}

void CqShaderVM::SO_divcc()					
{
	AUTOFUNC; 
	POPV(A); 
	POPV(B); 
	RESULT; 
	A.OpDIV_CC(B,Result,m_pEnv->RunningState()); 
	Push(Result);
}

void CqShaderVM::SO_addcc()					
{
	AUTOFUNC; 
	POPV(A); 
	POPV(B); 
	RESULT; 
	A.OpADD_CC(B,Result,m_pEnv->RunningState()); 
	Push(Result);
}

void CqShaderVM::SO_subcc()					
{
	AUTOFUNC; 
	POPV(A); 
	POPV(B); 
	RESULT; 
	A.OpSUB_CC(B,Result,m_pEnv->RunningState()); 
	Push(Result);
}

void CqShaderVM::SO_crscc()					
{
	AUTOFUNC; 
	POPV(A); 
	POPV(B); 
	RESULT; 
	A.OpCRS_CC(B,Result,m_pEnv->RunningState()); 
	Push(Result);
}

void CqShaderVM::SO_dotcc()					
{
	AUTOFUNC; POPV(A); 
	POPV(B); 
	RESULT; 
	A.OpDOT_CC(B,Result,m_pEnv->RunningState()); 
	Push(Result);
}

void CqShaderVM::SO_negc()					
{
	AUTOFUNC; 
	POPV(A); 
	RESULT; 
	A.OpNEG_C(Result,m_pEnv->RunningState()); 
	Push(Result);
}
									
void CqShaderVM::SO_mulfp()					
{
	AUTOFUNC; 
	POPV(A); 
	POPV(B); 
	RESULT; 
	A.OpMUL_FP(B,Result,m_pEnv->RunningState()); 
	Push(Result);
}

void CqShaderVM::SO_divfp()					
{
	AUTOFUNC; 
	POPV(A); 
	POPV(B); 
	RESULT; 
	A.OpDIV_FP(B,Result,m_pEnv->RunningState()); 
	Push(Result);
}

void CqShaderVM::SO_addfp()					
{
	AUTOFUNC; 
	POPV(A); 
	POPV(B); 
	RESULT; 
	A.OpADD_FP(B,Result,m_pEnv->RunningState()); 
	Push(Result);
}

void CqShaderVM::SO_subfp()					
{
	AUTOFUNC; 
	POPV(A); 
	POPV(B); 
	RESULT; 
	A.OpSUB_FP(B,Result,m_pEnv->RunningState()); 
	Push(Result);
}
									
void CqShaderVM::SO_mulfc()					
{
	AUTOFUNC; 
	POPV(A); 
	POPV(B); 
	RESULT; 
	A.OpMUL_FC(B,Result,m_pEnv->RunningState()); 
	Push(Result);
}

void CqShaderVM::SO_divfc()					
{
	AUTOFUNC; 
	POPV(A); 
	POPV(B); 
	RESULT; 
	A.OpDIV_FC(B,Result,m_pEnv->RunningState()); 
	Push(Result);
}

void CqShaderVM::SO_addfc()					
{
	AUTOFUNC; 
	POPV(A); 
	POPV(B); 
	RESULT; 
	A.OpADD_FC(B,Result,m_pEnv->RunningState()); 
	Push(Result);
}

void CqShaderVM::SO_subfc()					
{
	AUTOFUNC; 
	POPV(A); 
	POPV(B); 
	RESULT; 
	A.OpSUB_FC(B,Result,m_pEnv->RunningState()); 
	Push(Result);
}
									
void CqShaderVM::SO_mulmm()					
{
	Push((TqFloat)0.0);	/* TODO: Implement matrices in the VM*/
}

void CqShaderVM::SO_divmm()					
{
	Push((TqFloat)0.0); /* TODO: Implement matrices in the VM*/
}
									
void CqShaderVM::SO_land()					
{
	AUTOFUNC; 
	POPV(A); 
	POPV(B); 
	RESULT; 
	A.OpLAND_B(B,Result,m_pEnv->RunningState()); 
	Push(Result);
}

void CqShaderVM::SO_lor()					
{
	AUTOFUNC; 
	POPV(A); 
	POPV(B); 
	RESULT; 
	A.OpLOR_B(B,Result,m_pEnv->RunningState()); 
	Push(Result);
}

void CqShaderVM::SO_radians()				
{
	AUTOFUNC; 
	FUNC1(m_pEnv->SO_radians);
}

void CqShaderVM::SO_degrees()				
{
	AUTOFUNC; 
	FUNC1(m_pEnv->SO_degrees);
}

void CqShaderVM::SO_sin()					
{
	AUTOFUNC; 
	FUNC1(m_pEnv->SO_sin);
}

void CqShaderVM::SO_asin()					
{
	AUTOFUNC; 
	FUNC1(m_pEnv->SO_asin);
}

void CqShaderVM::SO_cos()					
{
	AUTOFUNC; 
	FUNC1(m_pEnv->SO_cos);
}

void CqShaderVM::SO_acos()					
{
	AUTOFUNC; 
	FUNC1(m_pEnv->SO_acos);
}

void CqShaderVM::SO_tan()					
{
	AUTOFUNC; 
	FUNC1(m_pEnv->SO_tan);
}

void CqShaderVM::SO_atan()					
{
	AUTOFUNC; 
	FUNC1(m_pEnv->SO_atan);
}

void CqShaderVM::SO_atan2()					
{
	AUTOFUNC; 
	FUNC2(m_pEnv->SO_atan);
}

void CqShaderVM::SO_pow()					
{
	AUTOFUNC; 
	FUNC2(m_pEnv->SO_pow);
}

void CqShaderVM::SO_exp()					
{
	AUTOFUNC; 
	FUNC1(m_pEnv->SO_exp);
}

void CqShaderVM::SO_sqrt()					
{
	AUTOFUNC; 
	FUNC1(m_pEnv->SO_sqrt);
}

void CqShaderVM::SO_log()					
{
	AUTOFUNC; 
	FUNC1(m_pEnv->SO_log);
}

void CqShaderVM::SO_log2()					
{
	AUTOFUNC; 
	FUNC2(m_pEnv->SO_log);
}

void CqShaderVM::SO_mod()					
{
	AUTOFUNC; 
	FUNC2(m_pEnv->SO_mod);
}

void CqShaderVM::SO_abs()					
{
	AUTOFUNC; 
	FUNC1(m_pEnv->SO_abs);
}

void CqShaderVM::SO_sign()					
{
	AUTOFUNC; 
	FUNC1(m_pEnv->SO_sign);
}

void CqShaderVM::SO_min()					
{
	AUTOFUNC; 
	FUNC2PLUS(m_pEnv->SO_min);
}

void CqShaderVM::SO_max()					
{
	AUTOFUNC; 
	FUNC2PLUS(m_pEnv->SO_max);
}

void CqShaderVM::SO_pmin()					
{
	AUTOFUNC; 
	FUNC2PLUS(m_pEnv->SO_pmin);
}

void CqShaderVM::SO_pmax()					
{
	AUTOFUNC; 
	FUNC2PLUS(m_pEnv->SO_pmax);
}

void CqShaderVM::SO_vmin()					
{
	AUTOFUNC; 
	FUNC2PLUS(m_pEnv->SO_pmin);
}

void CqShaderVM::SO_vmax()					
{
	AUTOFUNC; 
	FUNC2PLUS(m_pEnv->SO_pmax);
}

void CqShaderVM::SO_nmin()					
{
	AUTOFUNC; 
	FUNC2PLUS(m_pEnv->SO_pmin);
}

void CqShaderVM::SO_nmax()					
{
	AUTOFUNC; 
	FUNC2PLUS(m_pEnv->SO_pmax);
}

void CqShaderVM::SO_cmin()					
{
	AUTOFUNC; 
	FUNC2PLUS(m_pEnv->SO_cmin);
}

void CqShaderVM::SO_cmax()					
{
	AUTOFUNC; 
	FUNC2PLUS(m_pEnv->SO_cmax);
}

void CqShaderVM::SO_clamp()					
{
	AUTOFUNC; 
	FUNC3(m_pEnv->SO_clamp);
}

void CqShaderVM::SO_pclamp()					
{
	AUTOFUNC; 
	FUNC3(m_pEnv->SO_pclamp);
}

void CqShaderVM::SO_cclamp()					
{
	AUTOFUNC; 
	FUNC3(m_pEnv->SO_cclamp);
}

void CqShaderVM::SO_floor()					
{
	AUTOFUNC; 
	FUNC1(m_pEnv->SO_floor);
}

void CqShaderVM::SO_ceil()					
{
	AUTOFUNC; 
	FUNC1(m_pEnv->SO_ceil);
}

void CqShaderVM::SO_round()					
{
	AUTOFUNC; 
	FUNC1(m_pEnv->SO_round);
}

void CqShaderVM::SO_step()					
{
	AUTOFUNC; 
	FUNC2(m_pEnv->SO_step);
}

void CqShaderVM::SO_smoothstep()				
{
	AUTOFUNC; 
	FUNC3(m_pEnv->SO_smoothstep);
}

void CqShaderVM::SO_fspline()				
{
	AUTOFUNC; 
	SPLINE(m_pEnv->SO_fspline);
}

void CqShaderVM::SO_cspline()				
{
	AUTOFUNC; 
	SPLINE(m_pEnv->SO_cspline);
}

void CqShaderVM::SO_pspline()				
{
	AUTOFUNC; 
	SPLINE(m_pEnv->SO_pspline);
}

void CqShaderVM::SO_sfspline()				
{
	AUTOFUNC; 
	SSPLINE(m_pEnv->SO_sfspline);
}

void CqShaderVM::SO_scspline()				
{
	AUTOFUNC; 
	SSPLINE(m_pEnv->SO_scspline);
}

void CqShaderVM::SO_spspline()				
{
	AUTOFUNC; 
	SSPLINE(m_pEnv->SO_spspline);
}

void CqShaderVM::SO_fDu()					
{
	AUTOFUNC; 
	FUNC1(m_pEnv->SO_fDu);
}

void CqShaderVM::SO_fDv()					
{
	AUTOFUNC; 
	FUNC1(m_pEnv->SO_fDv);
}

void CqShaderVM::SO_fDeriv()					
{
	AUTOFUNC; 
	FUNC2(m_pEnv->SO_fDeriv);
}

void CqShaderVM::SO_cDu()					
{
	AUTOFUNC; 
	FUNC1(m_pEnv->SO_cDu);
}

void CqShaderVM::SO_cDv()					
{
	AUTOFUNC; 
	FUNC1(m_pEnv->SO_cDv);
}

void CqShaderVM::SO_cDeriv()					
{
	AUTOFUNC; 
	FUNC2(m_pEnv->SO_cDeriv);
}

void CqShaderVM::SO_pDu()					
{
	AUTOFUNC; 
	FUNC1(m_pEnv->SO_pDu);
}

void CqShaderVM::SO_pDv()					
{
	AUTOFUNC; 
	FUNC1(m_pEnv->SO_pDv);
}

void CqShaderVM::SO_pDeriv()					
{
	AUTOFUNC; 
	FUNC2(m_pEnv->SO_pDeriv);
}

void CqShaderVM::SO_frandom()				
{
	AUTOFUNC; 
	FUNC(m_pEnv->SO_frandom);
}

void CqShaderVM::SO_crandom()				
{
	AUTOFUNC; 
	FUNC(m_pEnv->SO_crandom);
}

void CqShaderVM::SO_prandom()				
{
	AUTOFUNC; 
	FUNC(m_pEnv->SO_prandom);
}

void CqShaderVM::SO_noise1()					
{
	AUTOFUNC; 
	FUNC1(m_pEnv->SO_fnoise1);
}

void CqShaderVM::SO_noise2()					
{
	AUTOFUNC; 
	FUNC2(m_pEnv->SO_fnoise2);
}

void CqShaderVM::SO_noise3()					
{
	AUTOFUNC; 
	FUNC1(m_pEnv->SO_fnoise3);
}

void CqShaderVM::SO_noise4()					
{
	AUTOFUNC; 
	FUNC2(m_pEnv->SO_fnoise4);
}

void CqShaderVM::SO_cnoise1()				
{
	AUTOFUNC; 
	FUNC1(m_pEnv->SO_cnoise1);
}

void CqShaderVM::SO_cnoise2()				
{
	AUTOFUNC; 
	FUNC2(m_pEnv->SO_cnoise2);
}

void CqShaderVM::SO_cnoise3()				
{
	AUTOFUNC; 
	FUNC1(m_pEnv->SO_cnoise3);
}

void CqShaderVM::SO_cnoise4()				
{
	AUTOFUNC; 
	FUNC2(m_pEnv->SO_cnoise4);
}

void CqShaderVM::SO_pnoise1()				
{
	AUTOFUNC; 
	FUNC1(m_pEnv->SO_pnoise1);
}

void CqShaderVM::SO_pnoise2()				
{	
	AUTOFUNC; 
	FUNC2(m_pEnv->SO_pnoise2);
}

void CqShaderVM::SO_pnoise3()				
{
	AUTOFUNC; 
	FUNC1(m_pEnv->SO_pnoise3);
}

void CqShaderVM::SO_pnoise4()				
{
	AUTOFUNC; 
	FUNC2(m_pEnv->SO_pnoise4);
}

void CqShaderVM::SO_xcomp()					
{
	AUTOFUNC; 
	POPV(A); 
	RESULT; 
	A.OpCOMP_P(0,Result,m_pEnv->RunningState()); 
	Push(Result);
}

void CqShaderVM::SO_ycomp()					
{
	AUTOFUNC; 
	POPV(A); 
	RESULT; 
	A.OpCOMP_P(1,Result,m_pEnv->RunningState()); 
	Push(Result);
}

void CqShaderVM::SO_zcomp()					
{
	AUTOFUNC; 
	POPV(A); 
	RESULT; 
	A.OpCOMP_P(2,Result,m_pEnv->RunningState()); 
	Push(Result);
}

void CqShaderVM::SO_setxcomp()				
{
	AUTOFUNC; 
	VOIDFUNC2(m_pEnv->SO_setxcomp); 
}

void CqShaderVM::SO_setycomp()				
{
	AUTOFUNC; 
	VOIDFUNC2(m_pEnv->SO_setycomp); 
}

void CqShaderVM::SO_setzcomp()				
{
	AUTOFUNC; 
	VOIDFUNC2(m_pEnv->SO_setzcomp); 
}

void CqShaderVM::SO_length()					
{
	AUTOFUNC; 
	FUNC1(m_pEnv->SO_length);
}

void CqShaderVM::SO_distance()				
{
	AUTOFUNC; 
	FUNC2(m_pEnv->SO_distance);
}

void CqShaderVM::SO_area()					
{
	AUTOFUNC; 
	FUNC1(m_pEnv->SO_area);
}

void CqShaderVM::SO_normalize()				
{
	AUTOFUNC; 
	FUNC1(m_pEnv->SO_normalize);
}

void CqShaderVM::SO_faceforward()			
{
	AUTOFUNC; 
	FUNC2(m_pEnv->SO_faceforward);
}

void CqShaderVM::SO_reflect()				
{
	AUTOFUNC; 
	FUNC2(m_pEnv->SO_reflect);
}

void CqShaderVM::SO_refract()				
{
	AUTOFUNC; 
	FUNC3(m_pEnv->SO_refract);
}

void CqShaderVM::SO_fresnel()				
{
	AUTOFUNC; 
	VOIDFUNC5(m_pEnv->SO_fresnel);
}

void CqShaderVM::SO_fresnel2()				
{
	AUTOFUNC; 
	VOIDFUNC7(m_pEnv->SO_fresnel);
}

void CqShaderVM::SO_transform2()				
{
	AUTOFUNC; 
	FUNC3(m_pEnv->SO_transform);
}

void CqShaderVM::SO_transform()				
{
	AUTOFUNC; 
	FUNC2(m_pEnv->SO_transform);
}

void CqShaderVM::SO_transformm()				
{
	AUTOFUNC; 
	FUNC2(m_pEnv->SO_transformm);
}

void CqShaderVM::SO_vtransform2()			
{
	AUTOFUNC; 
	FUNC3(m_pEnv->SO_vtransform);
}

void CqShaderVM::SO_vtransform()				
{
	AUTOFUNC; 
	FUNC2(m_pEnv->SO_vtransform);
}

void CqShaderVM::SO_vtransformm()				
{
	AUTOFUNC; 
	FUNC2(m_pEnv->SO_vtransformm);
}

void CqShaderVM::SO_ntransform2()			
{
	AUTOFUNC; 
	FUNC3(m_pEnv->SO_ntransform);
}

void CqShaderVM::SO_ntransform()				
{
	AUTOFUNC; 
	FUNC2(m_pEnv->SO_ntransform);
}

void CqShaderVM::SO_ntransformm()				
{
	AUTOFUNC; 
	FUNC2(m_pEnv->SO_ntransformm);
}

void CqShaderVM::SO_depth()					
{
	AUTOFUNC; 
	FUNC1(m_pEnv->SO_depth);
}

void CqShaderVM::SO_calculatenormal()		
{
	AUTOFUNC; 
	FUNC1(m_pEnv->SO_calculatenormal);
}

void CqShaderVM::SO_comp()					
{
	AUTOFUNC; 
	POPV(A); 
	POPV(B); 
	RESULT; A.OpCOMP_C(B,Result,m_pEnv->RunningState()); 
	Push(Result);
}

void CqShaderVM::SO_setcomp()				
{
	AUTOFUNC; 
	VOIDFUNC3(m_pEnv->SO_setcomp); 
}

void CqShaderVM::SO_cmix()					
{
	AUTOFUNC; 
	FUNC3(m_pEnv->SO_cmix);
}

void CqShaderVM::SO_fmix()					
{
	AUTOFUNC; 
	FUNC3(m_pEnv->SO_fmix);
}

void CqShaderVM::SO_pmix()					
{
	AUTOFUNC; 
	FUNC3(m_pEnv->SO_pmix);
}

void CqShaderVM::SO_vmix()					
{
	AUTOFUNC; 
	FUNC3(m_pEnv->SO_vmix);
}

void CqShaderVM::SO_nmix()					
{
	AUTOFUNC; 
	FUNC3(m_pEnv->SO_nmix);
}

void CqShaderVM::SO_ambient()				
{
	VARFUNC; 
	FUNC(m_pEnv->SO_ambient);
}

void CqShaderVM::SO_diffuse()				
{
	VARFUNC; 
	FUNC1(m_pEnv->SO_diffuse);
}

void CqShaderVM::SO_specular()				
{
	VARFUNC; 
	FUNC3(m_pEnv->SO_specular);
}

void CqShaderVM::SO_phong()					
{
	VARFUNC; 
	FUNC3(m_pEnv->SO_phong);
}

void CqShaderVM::SO_trace()					
{
	VARFUNC; 
	FUNC2(m_pEnv->SO_trace);
}

void CqShaderVM::SO_shadow()					
{
	VARFUNC; 
	TEXTURE1(m_pEnv->SO_shadow);
}

void CqShaderVM::SO_shadow1()				
{
	VARFUNC; 
	TEXTURE4(m_pEnv->SO_shadow1);
}

void CqShaderVM::SO_ftexture1()				
{
	VARFUNC; 
	TEXTURE(m_pEnv->SO_ftexture1);
}

void CqShaderVM::SO_ftexture2()				
{
	VARFUNC; 
	TEXTURE2(m_pEnv->SO_ftexture2);
}

void CqShaderVM::SO_ftexture3()				
{
	VARFUNC; 
	TEXTURE8(m_pEnv->SO_ftexture3);
}

void CqShaderVM::SO_ctexture1()				
{
	VARFUNC; 
	TEXTURE(m_pEnv->SO_ctexture1);
}

void CqShaderVM::SO_ctexture2()				
{
	VARFUNC; 
	TEXTURE2(m_pEnv->SO_ctexture2);
}

void CqShaderVM::SO_ctexture3()				
{
	VARFUNC; 
	TEXTURE8(m_pEnv->SO_ctexture3);
}

void CqShaderVM::SO_fenvironment2()			
{
	VARFUNC; 
	TEXTURE1(m_pEnv->SO_fenvironment2);
}

void CqShaderVM::SO_fenvironment3()			
{
	VARFUNC; 
	TEXTURE4(m_pEnv->SO_fenvironment3);
}

void CqShaderVM::SO_cenvironment2()			
{
	VARFUNC; 
	TEXTURE1(m_pEnv->SO_cenvironment2);
}

void CqShaderVM::SO_cenvironment3()			
{
	VARFUNC; 
	TEXTURE4(m_pEnv->SO_cenvironment3);
}

void CqShaderVM::SO_bump1()					
{
	VARFUNC; 
	TEXTURE(m_pEnv->SO_bump1);
}

void CqShaderVM::SO_bump2()					
{
	VARFUNC; 
	TEXTURE2(m_pEnv->SO_bump2);
}

void CqShaderVM::SO_bump3()					
{
	VARFUNC; 
	TEXTURE8(m_pEnv->SO_bump3);
}

void CqShaderVM::SO_illuminate()				
{
	VARFUNC; 
	VOIDFUNC1(m_pEnv->SO_illuminate);
}

void CqShaderVM::SO_illuminate2()			
{
	VARFUNC; 
	VOIDFUNC3(m_pEnv->SO_illuminate);
}

void CqShaderVM::SO_init_illuminance()
{
	VARFUNC;
	POPV(A);
	m_pEnv->InvalidateIlluminanceCache();
	m_pEnv->ValidateIlluminanceCache(A,this);
	Push(m_pEnv->SO_init_illuminance());
}

void CqShaderVM::SO_advance_illuminance()	
{
	Push(m_pEnv->SO_advance_illuminance());
}

void CqShaderVM::SO_illuminance()			
{
	VARFUNC; 
	VOIDFUNC2(m_pEnv->SO_illuminance);
}

void CqShaderVM::SO_illuminance2()			
{
	VARFUNC; 
	VOIDFUNC4(m_pEnv->SO_illuminance);
}

void CqShaderVM::SO_solar()					
{
	VARFUNC; 
	VOIDFUNC(m_pEnv->SO_solar);
}

void CqShaderVM::SO_solar2()					
{
	VARFUNC; 
	VOIDFUNC2(m_pEnv->SO_solar);
}

void CqShaderVM::SO_printf()					
{
	AUTOFUNC;
	VOIDFUNC1PLUS(m_pEnv->SO_printf);
}

void CqShaderVM::SO_atmosphere()				
{
	AUTOFUNC;
	CqShaderVariable* pV=GetVar(ReadNext().m_iVariable);
	POPV(Val);
	RESULT;
	m_pEnv->SO_atmosphere(Val,pV,Result);
	Push(Result);
}

void CqShaderVM::SO_displacement()			
{
	AUTOFUNC;
	CqShaderVariable* pV=GetVar(ReadNext().m_iVariable); 
	POPV(Val);
	RESULT;
	m_pEnv->SO_displacement(Val,pV,Result);
	Push(Result);
}

void CqShaderVM::SO_lightsource()			
{
	AUTOFUNC;
	CqShaderVariable* pV=GetVar(ReadNext().m_iVariable); 
	POPV(Val);
	RESULT;
	m_pEnv->SO_lightsource(Val,pV,Result);
	Push(Result);
}

void CqShaderVM::SO_surface()				
{
	AUTOFUNC;
	CqShaderVariable* pV=GetVar(ReadNext().m_iVariable); 
	POPV(Val);
	RESULT;
	m_pEnv->SO_surface(Val,pV,Result);
	Push(Result);
}

void CqShaderVM::SO_attribute()				
{
	AUTOFUNC;
	CqShaderVariable* pV=GetVar(ReadNext().m_iVariable); 
	POPV(Val);
	RESULT;
	m_pEnv->SO_attribute(Val,pV,Result);
	Push(Result);
}

void CqShaderVM::SO_option()				
{
	AUTOFUNC;
	CqShaderVariable* pV=GetVar(ReadNext().m_iVariable); 
	POPV(Val);
	RESULT;
	m_pEnv->SO_option(Val,pV,Result);
	Push(Result);
}

void CqShaderVM::SO_rendererinfo()				
{
	AUTOFUNC;
	CqShaderVariable* pV=GetVar(ReadNext().m_iVariable); 
	POPV(Val);
	RESULT;
	m_pEnv->SO_rendererinfo(Val,pV,Result);
	Push(Result);
}

void CqShaderVM::SO_incident()				
{
	AUTOFUNC;
	CqShaderVariable* pV=GetVar(ReadNext().m_iVariable); 
	POPV(Val);
	RESULT;
	m_pEnv->SO_incident(Val,pV,Result);
	Push(Result);
}

void CqShaderVM::SO_opposite()				
{
	AUTOFUNC;
	CqShaderVariable* pV=GetVar(ReadNext().m_iVariable); 
	POPV(Val);
	RESULT;
	m_pEnv->SO_opposite(Val,pV,Result);
	Push(Result);
}

void CqShaderVM::SO_fcellnoise1()			
{
	AUTOFUNC; 
	FUNC1(m_pEnv->SO_fcellnoise1);
}

void CqShaderVM::SO_fcellnoise2()			
{
	AUTOFUNC; 
	FUNC2(m_pEnv->SO_fcellnoise2);
}

void CqShaderVM::SO_fcellnoise3()			
{
	AUTOFUNC; 
	FUNC1(m_pEnv->SO_fcellnoise3);
}

void CqShaderVM::SO_fcellnoise4()			
{
	AUTOFUNC; 
	FUNC2(m_pEnv->SO_fcellnoise4);
}

void CqShaderVM::SO_ccellnoise1()			
{
	AUTOFUNC; 
	FUNC1(m_pEnv->SO_ccellnoise1);
}

void CqShaderVM::SO_ccellnoise2()			
{
	AUTOFUNC; 
	FUNC2(m_pEnv->SO_ccellnoise2);
}

void CqShaderVM::SO_ccellnoise3()			
{
	AUTOFUNC; 
	FUNC1(m_pEnv->SO_ccellnoise3);
}

void CqShaderVM::SO_ccellnoise4()			
{
	AUTOFUNC; 
	FUNC2(m_pEnv->SO_ccellnoise4);
}

void CqShaderVM::SO_pcellnoise1()			
{
	AUTOFUNC; 
	FUNC1(m_pEnv->SO_pcellnoise1);
}

void CqShaderVM::SO_pcellnoise2()			
{
	AUTOFUNC; 
	FUNC2(m_pEnv->SO_pcellnoise2);
}

void CqShaderVM::SO_pcellnoise3()			
{
	AUTOFUNC; 
	FUNC1(m_pEnv->SO_pcellnoise3);
}

void CqShaderVM::SO_pcellnoise4()			
{
	AUTOFUNC; 
	FUNC2(m_pEnv->SO_pcellnoise4);
}

void CqShaderVM::SO_fpnoise1()				
{
	AUTOFUNC; 
	FUNC2(m_pEnv->SO_fpnoise1);
}

void CqShaderVM::SO_fpnoise2()				
{
	AUTOFUNC; 
	FUNC4(m_pEnv->SO_fpnoise2);
}

void CqShaderVM::SO_fpnoise3()				
{
	AUTOFUNC; 
	FUNC2(m_pEnv->SO_fpnoise3);
}

void CqShaderVM::SO_fpnoise4()				
{
	AUTOFUNC; 
	FUNC4(m_pEnv->SO_fpnoise4);
}

void CqShaderVM::SO_cpnoise1()				
{
	AUTOFUNC; 
	FUNC2(m_pEnv->SO_cpnoise1);
}

void CqShaderVM::SO_cpnoise2()				
{
	AUTOFUNC; 
	FUNC4(m_pEnv->SO_cpnoise2);
}

void CqShaderVM::SO_cpnoise3()				
{
	AUTOFUNC; 
	FUNC2(m_pEnv->SO_cpnoise3);
}

void CqShaderVM::SO_cpnoise4()				
{
	AUTOFUNC; 
	FUNC4(m_pEnv->SO_cpnoise4);
}

void CqShaderVM::SO_ppnoise1()				
{
	AUTOFUNC; 
	FUNC2(m_pEnv->SO_ppnoise1);
}

void CqShaderVM::SO_ppnoise2()				
{
	AUTOFUNC; 
	FUNC4(m_pEnv->SO_ppnoise2);
}

void CqShaderVM::SO_ppnoise3()				
{
	AUTOFUNC; 
	FUNC2(m_pEnv->SO_ppnoise3);
}

void CqShaderVM::SO_ppnoise4()				
{
	AUTOFUNC; 
	FUNC4(m_pEnv->SO_ppnoise4);
}
									
void CqShaderVM::SO_ctransform2()			
{
	AUTOFUNC; 
	FUNC3(m_pEnv->SO_ctransform);
}

void CqShaderVM::SO_ctransform()				
{
	AUTOFUNC; 
	FUNC2(m_pEnv->SO_ctransform);
}
									
void CqShaderVM::SO_ptlined()				
{
	AUTOFUNC; 
	FUNC3(m_pEnv->SO_ptlined);
}

void CqShaderVM::SO_inversesqrt()			
{
	AUTOFUNC; 
	FUNC1(m_pEnv->SO_inversesqrt);
}

void CqShaderVM::SO_concat()					
{
	AUTOFUNC;
	FUNC2PLUS(m_pEnv->SO_concat);
}

void CqShaderVM::SO_format()					
{
	AUTOFUNC;
	FUNC1PLUS(m_pEnv->SO_format);
}

void CqShaderVM::SO_match()					
{
	AUTOFUNC; 
	FUNC2(m_pEnv->SO_match);
}

void CqShaderVM::SO_rotate()					
{
	AUTOFUNC; 
	FUNC4(m_pEnv->SO_rotate);
}

void CqShaderVM::SO_filterstep()				
{
	AUTOFUNC;;
	FUNC2PLUS(m_pEnv->SO_filterstep);
}

void CqShaderVM::SO_filterstep2()			
{
	AUTOFUNC;
	FUNC3PLUS(m_pEnv->SO_filterstep2);
}

void CqShaderVM::SO_specularbrdf()			
{
	AUTOFUNC; 
	FUNC4(m_pEnv->SO_specularbrdf);
}


void CqShaderVM::SO_mcomp()			
{
	AUTOFUNC; 
	POPV(A); 
	POPV(B); 
	POPV(C); 
	RESULT; 
	A.OpCOMPM(B,C,Result,m_pEnv->RunningState()); 
	Push(Result);
}

void CqShaderVM::SO_setmcomp()			
{
	AUTOFUNC; 
	VOIDFUNC4(m_pEnv->SO_setmcomp); 
}

void CqShaderVM::SO_determinant()			
{
	AUTOFUNC; 
	FUNC1(m_pEnv->SO_determinant);
}

void CqShaderVM::SO_mtranslate()			
{
	AUTOFUNC; 
	FUNC2(m_pEnv->SO_mtranslate);
}

void CqShaderVM::SO_mrotate()			
{
	AUTOFUNC; 
	FUNC3(m_pEnv->SO_mrotate);
}

void CqShaderVM::SO_mscale()			
{
	AUTOFUNC; 
	FUNC2(m_pEnv->SO_mscale);
}


void CqShaderVM::SO_fsplinea()				
{
	AUTOFUNC; 
	FUNC2(m_pEnv->SO_fsplinea);
}

void CqShaderVM::SO_csplinea()				
{
	AUTOFUNC; 
	FUNC2(m_pEnv->SO_csplinea);
}

void CqShaderVM::SO_psplinea()				
{
	AUTOFUNC; 
	FUNC2(m_pEnv->SO_psplinea);
}

void CqShaderVM::SO_sfsplinea()				
{
	AUTOFUNC; 
	FUNC3(m_pEnv->SO_sfsplinea);
}

void CqShaderVM::SO_scsplinea()				
{
	AUTOFUNC; 
	FUNC3(m_pEnv->SO_scsplinea);
}

void CqShaderVM::SO_spsplinea()				
{
	AUTOFUNC; 
	FUNC3(m_pEnv->SO_spsplinea);
}

void CqShaderVM::SO_shadername()				
{
	AUTOFUNC; 
	FUNC(m_pEnv->SO_shadername);
}

void CqShaderVM::SO_shadername2()				
{
	AUTOFUNC; 
	FUNC1(m_pEnv->SO_shadername2);
}


END_NAMESPACE(Aqsis)
//---------------------------------------------------------------------
