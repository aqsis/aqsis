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
#include	"version.h"
#include	"sstring.h"

#include	"irenderer.h"
#include	"shadervariable.h"

START_NAMESPACE( Aqsis )

char* gShaderTypeNames[] =
    {
        "surface",
        "lightsource",
        "volume",
        "displacement",
        "transformation",
        "imager",
    };
TqInt gcShaderTypeNames = sizeof( gShaderTypeNames ) / sizeof( gShaderTypeNames[ 0 ] );


SqOpCodeTrans CqShaderVM::m_TransTable[] =
    {
        {"RS_PUSH", &CqShaderVM::SO_RS_PUSH, 0, },
        {"RS_POP", &CqShaderVM::SO_RS_POP, 0, },
        {"RS_GET", &CqShaderVM::SO_RS_GET, 0, },
        {"RS_INVERSE", &CqShaderVM::SO_RS_INVERSE, 0, },
        {"RS_JZ", &CqShaderVM::SO_RS_JZ, 1, {type_float}},
        {"RS_JNZ", &CqShaderVM::SO_RS_JNZ, 1, {type_float}},
        {"S_JZ", &CqShaderVM::SO_S_JZ, 1, {type_float}},
        {"S_JNZ", &CqShaderVM::SO_S_JNZ, 1, {type_float}},
        {"S_GET", &CqShaderVM::SO_S_GET, 0, },
        {"S_CLEAR", &CqShaderVM::SO_S_CLEAR, 0, },

        {"nop", &CqShaderVM::SO_nop, 0, },
        {"dup", &CqShaderVM::SO_dup, 0, },
        {"debug_break", &CqShaderVM::SO_debug_break, 0, },
        {"drop", &CqShaderVM::SO_drop, 0, },

        {"mergef", &CqShaderVM::SO_mergef, 0, },
        {"merges", &CqShaderVM::SO_merges, 0, },
        {"mergep", &CqShaderVM::SO_mergep, 0, },
        {"mergen", &CqShaderVM::SO_mergep, 0, },
        {"mergev", &CqShaderVM::SO_mergep, 0, },
        {"mergec", &CqShaderVM::SO_mergec, 0, },

        {"pushif", &CqShaderVM::SO_pushif, 1, {type_float}},
        {"puship", &CqShaderVM::SO_puship, 3, {type_float, type_float, type_float}},
        {"pushis", &CqShaderVM::SO_pushis, 1, {type_string}},
        {"pushv", &CqShaderVM::SO_pushv, 1, {type_invalid}},
        {"ipushv", &CqShaderVM::SO_ipushv, 1, {type_invalid}},

        {"pop", &CqShaderVM::SO_pop, 1, {type_invalid}},
        {"ipop", &CqShaderVM::SO_ipop, 1, {type_invalid}},

        {"setfc", &CqShaderVM::SO_setfc, 0, },
        {"setfp", &CqShaderVM::SO_setfp, 0, },
        {"setfn", &CqShaderVM::SO_setfp, 0, },
        {"setfv", &CqShaderVM::SO_setfp, 0, },
        {"setfm", &CqShaderVM::SO_setfm, 0, },

        {"settc", &CqShaderVM::SO_settc, 0, },
        {"settp", &CqShaderVM::SO_settp, 0, },
        {"settn", &CqShaderVM::SO_settp, 0, },
        {"settv", &CqShaderVM::SO_settp, 0, },

        {"setpc", &CqShaderVM::SO_setpc, 0, },
        {"setvc", &CqShaderVM::SO_setpc, 0, },
        {"setnc", &CqShaderVM::SO_setpc, 0, },

        {"setcp", &CqShaderVM::SO_setcp, 0, },
        {"setcn", &CqShaderVM::SO_setcp, 0, },
        {"setcv", &CqShaderVM::SO_setcp, 0, },

        {"setwm", &CqShaderVM::SO_setwm, 0, },

        {"jnz", &CqShaderVM::SO_jnz, 1, {type_float}},
        {"jz", &CqShaderVM::SO_jz, 1, {type_float}},
        {"jmp", &CqShaderVM::SO_jmp, 1, {type_float}},

        {"lsff", &CqShaderVM::SO_lsff, 0, },
        {"lspp", &CqShaderVM::SO_lspp, 0, },
        {"lshh", &CqShaderVM::SO_lspp, 0, },
        {"lscc", &CqShaderVM::SO_lscc, 0, },
        {"lsnn", &CqShaderVM::SO_lspp, 0, },
        {"lsvv", &CqShaderVM::SO_lspp, 0, },

        {"gtff", &CqShaderVM::SO_gtff, 0, },
        {"gtpp", &CqShaderVM::SO_gtpp, 0, },
        {"gthh", &CqShaderVM::SO_gtpp, 0, },
        {"gtcc", &CqShaderVM::SO_gtcc, 0, },
        {"gtnn", &CqShaderVM::SO_gtpp, 0, },
        {"gtvv", &CqShaderVM::SO_gtpp, 0, },

        {"geff", &CqShaderVM::SO_geff, 0, },
        {"gepp", &CqShaderVM::SO_gepp, 0, },
        {"gehh", &CqShaderVM::SO_gepp, 0, },
        {"gecc", &CqShaderVM::SO_gecc, 0, },
        {"genn", &CqShaderVM::SO_gepp, 0, },
        {"gevv", &CqShaderVM::SO_gepp, 0, },

        {"leff", &CqShaderVM::SO_leff, 0, },
        {"lepp", &CqShaderVM::SO_lepp, 0, },
        {"lehh", &CqShaderVM::SO_lepp, 0, },
        {"lecc", &CqShaderVM::SO_lecc, 0, },
        {"lenn", &CqShaderVM::SO_lepp, 0, },
        {"levv", &CqShaderVM::SO_lepp, 0, },

        {"eqff", &CqShaderVM::SO_eqff, 0, },
        {"eqpp", &CqShaderVM::SO_eqpp, 0, },
        {"eqhh", &CqShaderVM::SO_eqpp, 0, },
        {"eqcc", &CqShaderVM::SO_eqcc, 0, },
        {"eqss", &CqShaderVM::SO_eqss, 0, },
        {"eqnn", &CqShaderVM::SO_eqpp, 0, },
        {"eqvv", &CqShaderVM::SO_eqpp, 0, },

        {"neff", &CqShaderVM::SO_neff, 0, },
        {"nepp", &CqShaderVM::SO_nepp, 0, },
        {"nehh", &CqShaderVM::SO_nepp, 0, },
        {"necc", &CqShaderVM::SO_necc, 0, },
        {"ness", &CqShaderVM::SO_ness, 0, },
        {"nenn", &CqShaderVM::SO_nepp, 0, },
        {"nevv", &CqShaderVM::SO_nepp, 0, },

        {"mulff", &CqShaderVM::SO_mulff, 0, },
        {"divff", &CqShaderVM::SO_divff, 0, },
        {"addff", &CqShaderVM::SO_addff, 0, },
        {"subff", &CqShaderVM::SO_subff, 0, },
        {"negf", &CqShaderVM::SO_negf, 0, },

        {"mulpp", &CqShaderVM::SO_mulpp, 0, },
        {"divpp", &CqShaderVM::SO_divpp, 0, },
        {"addpp", &CqShaderVM::SO_addpp, 0, },
        {"subpp", &CqShaderVM::SO_subpp, 0, },
        {"crspp", &CqShaderVM::SO_crspp, 0, },
        {"dotpp", &CqShaderVM::SO_dotpp, 0, },
        {"negp", &CqShaderVM::SO_negp, 0, },

        {"mulcc", &CqShaderVM::SO_mulcc, 0, },
        {"divcc", &CqShaderVM::SO_divcc, 0, },
        {"addcc", &CqShaderVM::SO_addcc, 0, },
        {"subcc", &CqShaderVM::SO_subcc, 0, },
        {"crscc", &CqShaderVM::SO_crscc, 0, },
        {"dotcc", &CqShaderVM::SO_dotcc, 0, },
        {"negc", &CqShaderVM::SO_negc, 0, },

        {"mulfp", &CqShaderVM::SO_mulfp, 0, },
        {"divfp", &CqShaderVM::SO_divfp, 0, },
        {"addfp", &CqShaderVM::SO_addfp, 0, },
        {"subfp", &CqShaderVM::SO_subfp, 0, },

        {"mulfc", &CqShaderVM::SO_mulfc, 0, },
        {"divfc", &CqShaderVM::SO_divfc, 0, },
        {"addfc", &CqShaderVM::SO_addfc, 0, },
        {"subfc", &CqShaderVM::SO_subfc, 0, },

        {"mulmm", &CqShaderVM::SO_mulmm, 0, },
        {"divmm", &CqShaderVM::SO_divmm, 0, },

        {"land", &CqShaderVM::SO_land, 0, },
        {"lor", &CqShaderVM::SO_lor, 0, },

        {"radians", &CqShaderVM::SO_radians, 0, },
        {"degrees", &CqShaderVM::SO_degrees, 0, },
        {"sin", &CqShaderVM::SO_sin, 0, },
        {"asin", &CqShaderVM::SO_asin, 0, },
        {"cos", &CqShaderVM::SO_cos, 0, },
        {"acos", &CqShaderVM::SO_acos, 0, },
        {"tan", &CqShaderVM::SO_tan, 0, },
        {"atan", &CqShaderVM::SO_atan, 0, },
        {"atan2", &CqShaderVM::SO_atan2, 0, },
        {"pow", &CqShaderVM::SO_pow, 0, },
        {"exp", &CqShaderVM::SO_exp, 0, },
        {"sqrt", &CqShaderVM::SO_sqrt, 0, },
        {"log", &CqShaderVM::SO_log, 0, },
        {"log2", &CqShaderVM::SO_log2, 0, },
        {"mod", &CqShaderVM::SO_mod, 0, },
        {"abs", &CqShaderVM::SO_abs, 0, },
        {"sign", &CqShaderVM::SO_sign, 0, },
        {"min", &CqShaderVM::SO_min, 0, },
        {"max", &CqShaderVM::SO_max, 0, },
        {"pmin", &CqShaderVM::SO_pmin, 0, },
        {"pmax", &CqShaderVM::SO_pmax, 0, },
        {"vmin", &CqShaderVM::SO_vmin, 0, },
        {"vmax", &CqShaderVM::SO_vmax, 0, },
        {"nmin", &CqShaderVM::SO_nmin, 0, },
        {"nmax", &CqShaderVM::SO_nmax, 0, },
        {"cmin", &CqShaderVM::SO_cmin, 0, },
        {"cmax", &CqShaderVM::SO_cmax, 0, },
        {"clamp", &CqShaderVM::SO_clamp, 0, },
        {"pclamp", &CqShaderVM::SO_pclamp, 0, },
        {"vclamp", &CqShaderVM::SO_pclamp, 0, },
        {"nclamp", &CqShaderVM::SO_pclamp, 0, },
        {"cclamp", &CqShaderVM::SO_cclamp, 0, },
        {"floor", &CqShaderVM::SO_floor, 0, },
        {"ceil", &CqShaderVM::SO_ceil, 0, },
        {"round", &CqShaderVM::SO_round, 0, },
        {"step", &CqShaderVM::SO_step, 0, },
        {"smoothstep", &CqShaderVM::SO_smoothstep, 0, },
        {"fspline", &CqShaderVM::SO_fspline, 0, },
        {"cspline", &CqShaderVM::SO_cspline, 0, },
        {"pspline", &CqShaderVM::SO_pspline, 0, },
        {"vspline", &CqShaderVM::SO_pspline, 0, },
        {"sfspline", &CqShaderVM::SO_sfspline, 0, },
        {"scspline", &CqShaderVM::SO_scspline, 0, },
        {"spspline", &CqShaderVM::SO_spspline, 0, },
        {"svspline", &CqShaderVM::SO_spspline, 0, },
        {"fDu", &CqShaderVM::SO_fDu, 0, },
        {"fDv", &CqShaderVM::SO_fDv, 0, },
        {"fDeriv", &CqShaderVM::SO_fDeriv, 0, },
        {"cDu", &CqShaderVM::SO_cDu, 0, },
        {"cDv", &CqShaderVM::SO_cDv, 0, },
        {"cDeriv", &CqShaderVM::SO_cDeriv, 0, },
        {"pDu", &CqShaderVM::SO_pDu, 0, },
        {"pDv", &CqShaderVM::SO_pDv, 0, },
        {"pDeriv", &CqShaderVM::SO_pDeriv, 0, },
        {"hDu", &CqShaderVM::SO_pDu, 0, },
        {"hDv", &CqShaderVM::SO_pDv, 0, },
        {"hDeriv", &CqShaderVM::SO_pDeriv, 0, },
        {"nDu", &CqShaderVM::SO_pDu, 0, },
        {"nDv", &CqShaderVM::SO_pDv, 0, },
        {"nDeriv", &CqShaderVM::SO_pDeriv, 0, },
        {"vDu", &CqShaderVM::SO_pDu, 0, },
        {"vDv", &CqShaderVM::SO_pDv, 0, },
        {"vDeriv", &CqShaderVM::SO_pDeriv, 0, },
        {"frandom", &CqShaderVM::SO_frandom, 0, },
        {"crandom", &CqShaderVM::SO_crandom, 0, },
        {"prandom", &CqShaderVM::SO_prandom, 0, },
        {"noise1", &CqShaderVM::SO_noise1, 0, },
        {"noise2", &CqShaderVM::SO_noise2, 0, },
        {"noise3", &CqShaderVM::SO_noise3, 0, },
        {"cnoise1", &CqShaderVM::SO_cnoise1, 0, },
        {"cnoise2", &CqShaderVM::SO_cnoise2, 0, },
        {"cnoise3", &CqShaderVM::SO_cnoise3, 0, },
        {"pnoise1", &CqShaderVM::SO_pnoise1, 0, },
        {"pnoise2", &CqShaderVM::SO_pnoise2, 0, },
        {"pnoise3", &CqShaderVM::SO_pnoise3, 0, },
        {"xcomp", &CqShaderVM::SO_xcomp, 0, },
        {"ycomp", &CqShaderVM::SO_ycomp, 0, },
        {"zcomp", &CqShaderVM::SO_zcomp, 0, },
        {"setxcomp", &CqShaderVM::SO_setxcomp, 0, },
        {"setycomp", &CqShaderVM::SO_setycomp, 0, },
        {"setzcomp", &CqShaderVM::SO_setzcomp, 0, },
        {"length", &CqShaderVM::SO_length, 0, },
        {"distance", &CqShaderVM::SO_distance, 0, },
        {"area", &CqShaderVM::SO_area, 0, },
        {"normalize", &CqShaderVM::SO_normalize, 0, },
        {"faceforward", &CqShaderVM::SO_faceforward, 0, },
        {"reflect", &CqShaderVM::SO_reflect, 0, },
        {"refract", &CqShaderVM::SO_refract, 0, },
        {"fresnel", &CqShaderVM::SO_fresnel, 0, },
        {"fresnel2", &CqShaderVM::SO_fresnel2, 0, },
        {"transform2", &CqShaderVM::SO_transform2, 0, },
        {"transform", &CqShaderVM::SO_transform, 0, },
        {"transformm", &CqShaderVM::SO_transformm, 0, },
        {"vtransform2", &CqShaderVM::SO_vtransform2, 0, },
        {"vtransform", &CqShaderVM::SO_vtransform, 0, },
        {"vtransformm", &CqShaderVM::SO_vtransformm, 0, },
        {"ntransform2", &CqShaderVM::SO_ntransform2, 0, },
        {"ntransform", &CqShaderVM::SO_ntransform, 0, },
        {"ntransformm", &CqShaderVM::SO_ntransformm, 0, },
        {"depth", &CqShaderVM::SO_depth, 0, },
        {"calculatenormal", &CqShaderVM::SO_calculatenormal, 0, },
        {"cmix", &CqShaderVM::SO_cmix, 0, },
        {"fmix", &CqShaderVM::SO_fmix, 0, },
        {"pmix", &CqShaderVM::SO_pmix, 0, },
        {"vmix", &CqShaderVM::SO_vmix, 0, },
        {"nmix", &CqShaderVM::SO_nmix, 0, },
        {"comp", &CqShaderVM::SO_comp, 0, },
        {"setcomp", &CqShaderVM::SO_setcomp, 0, },
        {"ambient", &CqShaderVM::SO_ambient, 0, },
        {"diffuse", &CqShaderVM::SO_diffuse, 0, },
        {"specular", &CqShaderVM::SO_specular, 0, },
        {"phong", &CqShaderVM::SO_phong, 0, },
        {"trace", &CqShaderVM::SO_trace, 0, },
        {"ftexture1", &CqShaderVM::SO_ftexture1, 0, },
        {"ftexture2", &CqShaderVM::SO_ftexture2, 0, },
        {"ftexture3", &CqShaderVM::SO_ftexture3, 0, },
        {"ctexture1", &CqShaderVM::SO_ctexture1, 0, },
        {"ctexture2", &CqShaderVM::SO_ctexture2, 0, },
        {"ctexture3", &CqShaderVM::SO_ctexture3, 0, },
        {"textureinfo", &CqShaderVM::SO_textureinfo, 1, {type_invalid}},
        {"fenvironment2", &CqShaderVM::SO_fenvironment2, 0, },
        {"fenvironment3", &CqShaderVM::SO_fenvironment3, 0, },
        {"cenvironment2", &CqShaderVM::SO_cenvironment2, 0, },
        {"cenvironment3", &CqShaderVM::SO_cenvironment3, 0, },
        {"bump1", &CqShaderVM::SO_bump1, 0, },
        {"bump2", &CqShaderVM::SO_bump2, 0, },
        {"bump3", &CqShaderVM::SO_bump3, 0, },
        {"shadow", &CqShaderVM::SO_shadow, 0, },
        {"shadow2", &CqShaderVM::SO_shadow1, 0, },
        {"illuminate", &CqShaderVM::SO_illuminate, 0, },
        {"illuminate2", &CqShaderVM::SO_illuminate2, 0, },
        {"illuminance", &CqShaderVM::SO_illuminance, 0, },
        {"illuminance2", &CqShaderVM::SO_illuminance2, 0, },
        {"init_illuminance", &CqShaderVM::SO_init_illuminance, 0, },
        {"advance_illuminance", &CqShaderVM::SO_advance_illuminance, 0, },
        {"solar", &CqShaderVM::SO_solar, 0, },
        {"solar2", &CqShaderVM::SO_solar2, 0, },
        {"printf", &CqShaderVM::SO_printf, 0, },

        {"fcellnoise1", &CqShaderVM::SO_fcellnoise1, 0, },
        {"fcellnoise2", &CqShaderVM::SO_fcellnoise2, 0, },
        {"fcellnoise3", &CqShaderVM::SO_fcellnoise3, 0, },
        {"fcellnoise4", &CqShaderVM::SO_fcellnoise4, 0, },
        {"ccellnoise1", &CqShaderVM::SO_ccellnoise1, 0, },
        {"ccellnoise2", &CqShaderVM::SO_ccellnoise2, 0, },
        {"ccellnoise3", &CqShaderVM::SO_ccellnoise3, 0, },
        {"ccellnoise4", &CqShaderVM::SO_ccellnoise4, 0, },
        {"pcellnoise1", &CqShaderVM::SO_pcellnoise1, 0, },
        {"pcellnoise2", &CqShaderVM::SO_pcellnoise2, 0, },
        {"pcellnoise3", &CqShaderVM::SO_pcellnoise3, 0, },
        {"pcellnoise4", &CqShaderVM::SO_pcellnoise4, 0, },

        {"fpnoise1", &CqShaderVM::SO_fpnoise1, 0, },
        {"fpnoise2", &CqShaderVM::SO_fpnoise2, 0, },
        {"fpnoise3", &CqShaderVM::SO_fpnoise3, 0, },
        {"cpnoise1", &CqShaderVM::SO_cpnoise1, 0, },
        {"cpnoise2", &CqShaderVM::SO_cpnoise2, 0, },
        {"cpnoise3", &CqShaderVM::SO_cpnoise3, 0, },
        {"ppnoise1", &CqShaderVM::SO_ppnoise1, 0, },
        {"pnoise2", &CqShaderVM::SO_ppnoise2, 0, },
        {"ppnoise3", &CqShaderVM::SO_ppnoise3, 0, },

        {"atmosphere", &CqShaderVM::SO_atmosphere, 1, {type_invalid}},
        {"displacement", &CqShaderVM::SO_displacement, 1, {type_invalid}},
        {"lightsource", &CqShaderVM::SO_lightsource, 1, {type_invalid}},
        {"surface", &CqShaderVM::SO_surface, 1, {type_invalid}},

        {"attribute", &CqShaderVM::SO_attribute, 1, {type_invalid}},
        {"option", &CqShaderVM::SO_option, 1, {type_invalid}},
        {"rendererinfo", &CqShaderVM::SO_rendererinfo, 1, {type_invalid}},
        {"incident", &CqShaderVM::SO_incident, 1, {type_invalid}},
        {"opposite", &CqShaderVM::SO_opposite, 1, {type_invalid}},

        {"ctransform", &CqShaderVM::SO_ctransform, 0, },
        {"ctransform2", &CqShaderVM::SO_ctransform2, 0, },

        {"ptlined", &CqShaderVM::SO_ptlined, 0, },
        {"inversesqrt", &CqShaderVM::SO_inversesqrt, 0, },
        {"concat", &CqShaderVM::SO_concat, 0, },
        {"format", &CqShaderVM::SO_format, 0, },
        {"match", &CqShaderVM::SO_match, 0, },
        {"rotate", &CqShaderVM::SO_rotate, 0, },
        {"filterstep", &CqShaderVM::SO_filterstep, 0, },
        {"filterstep2", &CqShaderVM::SO_filterstep2, 0, },
        {"specularbrdf", &CqShaderVM::SO_specularbrdf, 0, },

        {"mcomp", &CqShaderVM::SO_mcomp, 0, },
        {"setmcomp", &CqShaderVM::SO_setmcomp, 0, },
        {"determinant", &CqShaderVM::SO_determinant, 0, },
        {"mtranslate", &CqShaderVM::SO_mtranslate, 0, },
        {"mrotate", &CqShaderVM::SO_mrotate, 0, },
        {"mscale", &CqShaderVM::SO_mscale, 0, },

        {"fsplinea", &CqShaderVM::SO_fsplinea, 0, },
        {"csplinea", &CqShaderVM::SO_csplinea, 0, },
        {"psplinea", &CqShaderVM::SO_psplinea, 0, },
        {"vsplinea", &CqShaderVM::SO_psplinea, 0, },
        {"sfsplinea", &CqShaderVM::SO_sfsplinea, 0, },
        {"scsplinea", &CqShaderVM::SO_scsplinea, 0, },
        {"spsplinea", &CqShaderVM::SO_spsplinea, 0, },
        {"svsplinea", &CqShaderVM::SO_spsplinea, 0, },

        {"shadername", &CqShaderVM::SO_shadername, 0, },
        {"shadername2", &CqShaderVM::SO_shadername2, 0, },

    };
TqInt CqShaderVM::m_cTransSize = sizeof( m_TransTable ) / sizeof( m_TransTable[ 0 ] );


IqShaderData* CqShaderVM::CreateVariable(EqVariableType Type, EqVariableClass Class, const CqString& name, TqBool fParameter )
{
	// Create a VM specific shader variable, which implements the IqShaderData interface,
	// based on the type and class specified.
	switch(Type)
	{
	    case type_bool: /* abviously they are missing here */
		case type_integer:
		case type_float:
		{
			switch(Class)
			{
				case class_varying:
					return(new CqShaderVariableVaryingFloat( name.c_str(), fParameter ) );
				case class_uniform:
					return(new CqShaderVariableUniformFloat( name.c_str(), fParameter ) );
			}
			assert(TqFalse);	// If we get here, something is wrong with the request.
			return(NULL);
		}

		case type_point:
		{
			switch(Class)
			{
				case class_varying:
					return(new CqShaderVariableVaryingPoint( name.c_str(), fParameter ) );
				case class_uniform:
					return(new CqShaderVariableUniformPoint( name.c_str(), fParameter ) );
			}
			assert(TqFalse);	// If we get here, something is wrong with the request.
			return(NULL);
		}

		case type_normal:
		{
			switch(Class)
			{
				case class_varying:
					return(new CqShaderVariableVaryingNormal( name.c_str(), fParameter ) );
				case class_uniform:
					return(new CqShaderVariableUniformNormal( name.c_str(), fParameter ) );
			}
			assert(TqFalse);	// If we get here, something is wrong with the request.
			return(NULL);
		}

		case type_vector:
		{
			switch(Class)
			{
				case class_varying:
					return(new CqShaderVariableVaryingVector( name.c_str(), fParameter ) );
				case class_uniform:
					return(new CqShaderVariableUniformVector( name.c_str(), fParameter ) );
			}
			assert(TqFalse);	// If we get here, something is wrong with the request.
			return(NULL);
		}
		
		case type_string:
		{
			switch(Class)
			{
				case class_varying:
					return(new CqShaderVariableVaryingString( name.c_str(), fParameter ) );
				case class_uniform:
					return(new CqShaderVariableUniformString( name.c_str(), fParameter ) );
			}
			assert(TqFalse);	// If we get here, something is wrong with the request.
			return(NULL);
		}

		case type_color:
		{
			switch(Class)
			{
				case class_varying:
					return(new CqShaderVariableVaryingColor( name.c_str(), fParameter ) );
				case class_uniform:
					return(new CqShaderVariableUniformColor( name.c_str(), fParameter ) );
			}
			assert(TqFalse);	// If we get here, something is wrong with the request.
			return(NULL);
		}

		case type_triple:
		case type_hpoint:
		case type_void:
			assert(TqFalse);	// We don't support triples in the engine as variables.
			return(NULL);

		case type_matrix:
		{
			switch(Class)
			{
				case class_varying:
					return(new CqShaderVariableVaryingMatrix( name.c_str(), fParameter ) );
				case class_uniform:
					return(new CqShaderVariableUniformMatrix( name.c_str(), fParameter ) );
			}
			assert(TqFalse);	// If we get here, something is wrong with the request.
			return(NULL);
		}
	}
	assert(TqFalse);	// If we get here, something is wrong with the request.
	return(NULL);
}



IqShaderData* CqShaderVM::CreateVariableArray( EqVariableType VarType, EqVariableClass VarClass, const CqString& name, TqInt Count, TqBool fParameter)
{
	IqShaderData * pVar = 0;
	switch ( VarType )
	{
			case type_float:
			if ( VarClass == class_varying )
				pVar = new CqShaderVariableVaryingFloat( name.c_str(), fParameter );
			else
				pVar = new CqShaderVariableUniformFloat( name.c_str(), fParameter );
			break;

			case type_point:
			if ( VarClass == class_varying )
				pVar = new CqShaderVariableVaryingPoint( name.c_str(), fParameter );
			else
				pVar = new CqShaderVariableUniformPoint( name.c_str(), fParameter );
			break;

			case type_normal:
			if ( VarClass == class_varying )
				pVar = new CqShaderVariableVaryingNormal( name.c_str(), fParameter );
			else
				pVar = new CqShaderVariableUniformNormal( name.c_str(), fParameter );
			break;

			case type_vector:
			if ( VarClass == class_varying )
				pVar = new CqShaderVariableVaryingVector( name.c_str(), fParameter );
			else
				pVar = new CqShaderVariableUniformVector( name.c_str(), fParameter );
			break;

			case type_color:
			if ( VarClass == class_varying )
				pVar = new CqShaderVariableVaryingColor( name.c_str(), fParameter );
			else
				pVar = new CqShaderVariableUniformColor( name.c_str(), fParameter );
			break;

			case type_string:
			if ( VarClass == class_varying )
				pVar = new CqShaderVariableVaryingString( name.c_str(), fParameter );
			else
				pVar = new CqShaderVariableUniformString( name.c_str(), fParameter );
			break;

			case type_matrix:
			if ( VarClass == class_varying )
				pVar = new CqShaderVariableVaryingMatrix( name.c_str(), fParameter );
			else
				pVar = new CqShaderVariableUniformMatrix( name.c_str(), fParameter );
			break;
	}
	CqShaderVariableArray* pArray = new CqShaderVariableArray( name.c_str(), Count, fParameter );
	pArray->aVariables() [ 0 ] = pVar;
	TqInt i;
	for ( i = 1; i < Count; i++ )
		pArray->aVariables() [ i ] = pVar->Clone();

	return ( pArray );
}


IqShaderData* CqShaderVM::CreateTemporaryStorage(EqVariableType type, EqVariableClass _class)
{
	static CqString strName("__temporary__");
	return( CreateVariable(type, _class, strName) );
}


void CqShaderVM::DeleteTemporaryStorage( IqShaderData* pData )
{
	delete( pData );
}

//---------------------------------------------------------------------
/** Load a token from a compiled slx file.
 */

void CqShaderVM::GetToken( char* token, TqInt l, std::istream* pFile )
{
	char c;
	TqInt i = 0;
	( *pFile ) >> std::ws;
	c = pFile->get();
	if ( c == ':' && i == 0 )
	{
		token[ 0 ] = c;
		token[ 1 ] = '\0';
		return ;	// Special case for labels.
	}
	while ( !isspace( c ) && i < l - 1 )
	{
		token[ i++ ] = c;
		token[ i ] = '\0';
		c = pFile->get();
	}
}


//---------------------------------------------------------------------
/** Load a program from a compiled slc file.
 */

void CqShaderVM::LoadProgram( std::istream* pFile )
{
	enum EqSegment
	{
	    Seg_Data = 0,
	    Seg_Init,
	    Seg_Code,
	};
	char token[ 255 ];
	EqSegment	Segment = Seg_Data;
	std::vector<UsProgramElement>*	pProgramArea = NULL;
	std::vector<TqInt>	aLabels;
	CqShaderExecEnv	StdEnv;
	TqInt	array_count = 0;

	TqBool fShaderSpec = TqFalse;
	while ( !pFile->eof() )
	{
		GetToken( token, 255, pFile );

		// Check for type and version information.
		if ( !fShaderSpec )
		{
			TqInt i;
			for ( i = 0; i < gcShaderTypeNames; i++ )
			{
				if ( strcmp( token, gShaderTypeNames[ i ] ) == 0 )
				{
					// TODO: Should store the type so that it can be checked.
                    
                    /* Begin changes to store shader type for libslxargs */
                    // Symbolic references for the tokens would be better than literal strings,
                    // but we will use literals for now.
                    // m_Type = Type_Unknown;
                    if(strcmp(token,"surface")==0) 
                            m_Type = Type_Surface;
                    else if(strcmp(token,"lightsource")==0)
                            m_Type = Type_Lightsource;
                    else if(strcmp(token,"volume")==0)
                            m_Type = Type_Volume;
                    else if(strcmp(token,"displacement")==0)
                            m_Type = Type_Displacement;
                    else if(strcmp(token,"transformation")==0)
                            m_Type = Type_Transformation;
                    else if(strcmp(token,"imager")==0)
                            m_Type = Type_Imager;
                    /* End changes to store shader type for libslxargs */
                    
					fShaderSpec = TqTrue;
					break;
				}
			}
			if ( fShaderSpec ) continue;
		}

		if ( strcmp( token, "AQSIS_V" ) == 0 )
		{
			GetToken( token, 255, pFile );

			continue;

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

		if ( strcmp( token, "USES" ) == 0 )
		{
			( *pFile ) >> m_Uses;
			continue;
		}

		if ( strcmp( token, "segment" ) == 0 )
		{
			GetToken( token, 255, pFile );
			if ( strcmp( token, "Data" ) == 0 )
				Segment = Seg_Data;
			else if ( strcmp( token, "Init" ) == 0 )
			{
				Segment = Seg_Init;
				pProgramArea = &m_ProgramInit;
				aLabels.clear();
			}
			else if ( strcmp( token, "Code" ) == 0 )
			{
				Segment = Seg_Code;
				pProgramArea = &m_Program;
				aLabels.clear();
			}
		}
		else
		{
			EqVariableType VarType = type_invalid;
			EqVariableClass VarClass = class_varying;
			TqBool			fVarArray = TqFalse;
			TqBool			fParameter = TqFalse;
			switch ( Segment )
			{
					case Seg_Data:
					VarType = type_invalid;
					VarClass = class_invalid;
					while ( VarType == type_invalid )
					{
						if ( strcmp( token, "param" ) == 0 )
							fParameter = TqTrue;
						else if ( strcmp( token, "varying" ) == 0 )
							VarClass = class_varying;
						else if ( strcmp( token, "uniform" ) == 0 )
							VarClass = class_uniform;
						else 
						{
							TqInt itype = 0;
							for(itype = 0; itype<type_last; itype++)
								if ( strcmp( token, gVariableTypeNames[ itype ] ) == 0 )
								{
									VarType = static_cast<EqVariableType>(itype);
									break;
								}
						}
						GetToken( token, 255, pFile );
					}
					// Check for array type variable.
					if ( token[ strlen( token ) - 1 ] == ']' )
					{
						unsigned int i = 0;
						while ( i < strlen( token ) && token[ i ] != '[' ) i++;
						if ( i == strlen( token ) )
						{
							//CqBasicError( 0, Severity_Fatal, "Invalid variable specification in slx file" );
							return ;
						}
						token[ strlen( token ) - 1 ] = '\0';
						token[ i ] = '\0';
						i++;
						array_count = atoi( &token[ i ] );
						fVarArray = TqTrue;
					}
					// Check if there is a valid variable specifier
					if ( VarType == type_invalid ||
					     VarClass == class_invalid )
						continue;

					if ( fVarArray )
						AddLocalVariable( CreateVariableArray( VarType, VarClass, token, array_count, fParameter ) );
					else
						AddLocalVariable( CreateVariable( VarType, VarClass, token, fParameter ) );
					break;

					case Seg_Init:
					case Seg_Code:
					// Check if it is a label
					if ( strcmp( token, ":" ) == 0 )
					{
						( *pFile ) >> std::ws;
						TqFloat f;
						( *pFile ) >> f;
						if ( aLabels.size() < ( f + 1 ) )
							aLabels.resize( static_cast<TqInt>( f ) + 1 );
						aLabels[ static_cast<TqInt>( f ) ] = pProgramArea->size();
						AddCommand( &CqShaderVM::SO_nop, pProgramArea );
						break;
					}
					// Find the opcode in the translation table.
					TqInt i;
					for ( i = 0; i < m_cTransSize; i++ )
					{
						if ( strcmp( m_TransTable[ i ].m_strName, token ) == 0 )
						{
							// If the opcodes command pointer is 0, just ignore this opcode.
							if ( m_TransTable[ i ].m_pCommand == 0 )
								break;

							// If this is an 'illuminate' or 'solar' statement, then we can safely say this 
							// is not an ambient light.
							if( &CqShaderVM::SO_illuminate == m_TransTable[ i ].m_pCommand ||
							    &CqShaderVM::SO_illuminate2 == m_TransTable[ i ].m_pCommand ||
								&CqShaderVM::SO_solar == m_TransTable[ i ].m_pCommand ||
								&CqShaderVM::SO_solar2 == m_TransTable[ i ].m_pCommand )
								m_fAmbient = TqFalse;
							
							// Add this opcode to the program segment.
							AddCommand( m_TransTable[ i ].m_pCommand, pProgramArea );
							// Process this opcodes parameters.
							TqInt p;
							for ( p = 0; p < m_TransTable[ i ].m_cParams; p++ )
							{
								if ( m_TransTable[ i ].m_aParamTypes[ p ] == type_invalid )
								{
									GetToken( token, 255, pFile );
									TqInt iVar;
									if ( ( iVar = FindLocalVarIndex( token ) ) >= 0 )
										AddVariable( iVar, pProgramArea );
									else if ( ( iVar = StdEnv.FindStandardVarIndex( token ) ) >= 0 )
										AddVariable( iVar | 0x8000, pProgramArea );
									else
										// TODO: Report error.
										AddVariable( 0, pProgramArea );
								}
								else
								{
									switch ( m_TransTable[ i ].m_aParamTypes[ p ] )
									{
											case type_float:
											( *pFile ) >> std::ws;
											TqFloat f;
											( *pFile ) >> f;
											AddFloat( f, pProgramArea );
											break;

											case type_string:
											( *pFile ) >> std::ws;
											char c;
											CqString s( "" );
											pFile->get();
											while ( ( c = pFile->get() ) != '"' )
												s += c;
											AddString( s.c_str(), pProgramArea );
											break;
									}
								}
							}
							break;
						}
					}
					// If we have not found the opcode, throw an error.
					if ( i == m_cTransSize )
					{
						CqString strErr( "Invalid opcode found : " );
						strErr += token;
						//CqBasicError( 0, Severity_Fatal, strErr.c_str() );
						return ;
					}
					break;
			}
		}
		( *pFile ) >> std::ws;
	}
	// Now we need to complete any label jump statements.
	TqUint i = 0;
	while ( i < m_Program.size() )
	{
		UsProgramElement E = m_Program[ i++ ];
		if ( E.m_Command == &CqShaderVM::SO_jnz ||
		        E.m_Command == &CqShaderVM::SO_jmp ||
		        E.m_Command == &CqShaderVM::SO_jz ||
		        E.m_Command == &CqShaderVM::SO_RS_JZ ||
		        E.m_Command == &CqShaderVM::SO_RS_JNZ ||
		        E.m_Command == &CqShaderVM::SO_S_JZ ||
		        E.m_Command == &CqShaderVM::SO_S_JNZ )
		{
			SqLabel lab;
			lab.m_Offset = aLabels[ static_cast<unsigned int>( m_Program[ i ].m_FloatVal ) ];
			lab.m_pAddress = &m_Program[ lab.m_Offset ];
			m_Program[ i ].m_Label = lab;
			i++;
		}
		else
		{
			// Find the command so that we can skip the parameters
			TqInt j;
			for ( j = 0; j < m_cTransSize; j++ )
			{
				if ( m_TransTable[ j ].m_pCommand == E.m_Command )
				{
					i += m_TransTable[ j ].m_cParams;
					break;
				}
			}
		}
	}
}


//---------------------------------------------------------------------
/**	Ready the shader for execution.
 */

void CqShaderVM::Initialise( const TqInt uGridRes, const TqInt vGridRes, IqShaderExecEnv* pEnv )
{
	m_pEnv = pEnv;
	// Initialise local variables.
	TqInt i;
	for ( i = m_LocalVars.size() - 1; i >= 0; i-- )
		m_LocalVars[ i ] ->Initialise( uGridRes, vGridRes );

	m_uGridRes = uGridRes;
	m_vGridRes = vGridRes;

	// Reset the program counter.
	m_PC = 0;
}


//---------------------------------------------------------------------
/**	Assignment operator.
 */

CqShaderVM&	CqShaderVM::operator=( const CqShaderVM& From )
{
	m_Uses = From.m_Uses;
	m_matCurrent = From.m_matCurrent;
	m_strName = From.m_strName;
	m_fAmbient = From.m_fAmbient;

	// Copy the local variables...
	std::vector<IqShaderData*>::const_iterator i;
	for ( i = From.m_LocalVars.begin(); i != From.m_LocalVars.end(); i++ )
		m_LocalVars.push_back( ( *i ) ->Clone() );

	// Copy the intialisation program.
	std::vector<UsProgramElement>::const_iterator p;
	for ( p = From.m_ProgramInit.begin(); p != From.m_ProgramInit.end(); p++ )
		m_ProgramInit.push_back( *p );

	// Copy the main program.
	for ( p = From.m_Program.begin(); p != From.m_Program.end(); p++ )
		m_Program.push_back( *p );

	return ( *this );
}


//---------------------------------------------------------------------
/**	Execute a series of shader language bytecodes.
 */

void CqShaderVM::Execute( IqShaderExecEnv* pEnv )
{
	// Check if there is anything to execute.
	if ( m_Program.size() <= 0 )
		return ;

	m_pEnv = pEnv;

	pEnv->InvalidateIlluminanceCache();

	// Execute the main program.
	m_PC = &m_Program[ 0 ];
	m_PO = 0;
	m_PE = m_Program.size();
	UsProgramElement* pE;

	while ( !fDone() )
	{
		pE = &ReadNext();
		( this->*pE->m_Command ) ();
	}
	// Check that the stack is empty.
	assert( m_iTop == 0 );
	m_Stack.clear();
}


//---------------------------------------------------------------------
/**	Execute the program segment which initialises the default values of instance variables.
 */

void CqShaderVM::ExecuteInit()
{
	// Check if there is anything to execute.
	if ( m_ProgramInit.size() <= 0 )
		return ;

	// Fake an environment
	IqShaderExecEnv* pOldEnv = m_pEnv;
	
	CqShaderExecEnv	Env;
	Env.Initialise( 1, 1, 0, this, m_Uses );
	Initialise( 1, 1, &Env );

	// Execute the init program.
	m_PC = &m_ProgramInit[ 0 ];
	m_PO = 0;
	m_PE = m_ProgramInit.size();
	UsProgramElement* pE;

	while ( !fDone() )
	{
		pE = &ReadNext();
		( this->*pE->m_Command ) ();
	}
	// Check that the stack is empty.
	assert( m_iTop == 0 );
	m_Stack.clear();

	m_pEnv = pOldEnv;
}


//---------------------------------------------------------------------
/** Set the instance variables on this shader.
 */

void CqShaderVM::SetArgument( const CqString& strName, EqVariableType type, const CqString& strSpace, void* pval )
{
	// Find the relevant variable.
	TqInt i = FindLocalVarIndex( strName.c_str() );
	if ( i >= 0 )
	{
		int index = 0, count = 1, arrayindex = 0;
		IqShaderData* pArray = 0;

		if ( m_LocalVars[ i ] ->ArrayLength() > 0 )
		{
			pArray = m_LocalVars[ i ];
			count = pArray->ArrayLength();
		}

		// Ensure that the type passed matches what the variable expects.
		//assert( m_LocalVars[ i ] ->Type() == type );
		while ( count-- > 0 )
		{
			IqShaderData* pVMVal = CreateTemporaryStorage(type, class_uniform);
			switch ( m_LocalVars[ i ] ->Type() )
			{
				case	type_float:
				{
					pVMVal->SetFloat(reinterpret_cast<TqFloat*>( pval ) [ index++ ] );
				}
				break;

				case	type_point:
				{
					TqFloat* pvecval = reinterpret_cast<TqFloat*>( pval );
					pVMVal->SetPoint( CqVector3D( pvecval[ index + 0 ], pvecval[ index + 1 ], pvecval[ index + 2 ] ) );
					index += 3;
				}
				break;

				case	type_normal:
				{
					TqFloat* pvecval = reinterpret_cast<TqFloat*>( pval );
					pVMVal->SetNormal( CqVector3D( pvecval[ index + 0 ], pvecval[ index + 1 ], pvecval[ index + 2 ] ) );
					index += 3;
				}
				break;

				case	type_vector:
				{
					TqFloat* pvecval = reinterpret_cast<TqFloat*>( pval );
					pVMVal->SetVector( CqVector3D( pvecval[ index + 0 ], pvecval[ index + 1 ], pvecval[ index + 2 ] ) );
					index += 3;
				}
				break;

				case	type_color:
				{
					TqFloat* pvecval = reinterpret_cast<TqFloat*>( pval );
					pVMVal->SetColor( CqColor( pvecval[ index + 0 ], pvecval[ index + 1 ], pvecval[ index + 2 ] ) );
					index += 3;
				}
				break;

				case	type_matrix:
				{
					TqFloat* pvecval = reinterpret_cast<TqFloat*>( pval );
					pVMVal->SetMatrix( CqMatrix( pvecval[ index + 0 ], pvecval[ index + 1 ], pvecval[ index + 2 ], pvecval[ index + 3 ],
												 pvecval[ index + 4 ], pvecval[ index + 5 ], pvecval[ index + 6 ], pvecval[ index + 7 ],
												 pvecval[ index + 8 ], pvecval[ index + 9 ], pvecval[ index + 10 ], pvecval[ index + 11 ],
												 pvecval[ index + 12 ], pvecval[ index + 13 ], pvecval[ index + 14 ], pvecval[ index + 15 ] ) );
					index += 16;
				}
				break;

				case	type_string:
				{
					pVMVal->SetString( reinterpret_cast<char**>( pval ) [ index++ ] );
				}
				break;
			}

			CqMatrix matObjectToWorld = matCurrent();
			if( NULL != m_pEnv )
				matObjectToWorld = m_pEnv->pTransform()->matObjectToWorld();
			
			// If it is a color or a point, ensure it is the correct 'space'
			if ( m_LocalVars[ i ] ->Type() == type_point || m_LocalVars[ i ] ->Type() == type_hpoint )
			{
				CqString _strSpace( "shader" );
				if ( strSpace.compare( "" ) != 0 )
					_strSpace = strSpace;
				CqVector3D p;
				pVMVal->GetPoint( p, 0 );
				pVMVal->SetPoint( QGetRenderContextI() ->matSpaceToSpace( _strSpace.c_str(), "camera", matCurrent(), matObjectToWorld ) * p );
			}
			else if ( m_LocalVars[ i ] ->Type() == type_normal )
			{
				CqString _strSpace( "shader" );
				if ( strSpace.compare( "" ) != 0 )
					_strSpace = strSpace;
				CqVector3D p;
				pVMVal->GetNormal( p, 0 );
				pVMVal->SetNormal( QGetRenderContextI() ->matNSpaceToSpace( _strSpace.c_str(), "camera", matCurrent(), matObjectToWorld ) * p );
			}
			else if ( m_LocalVars[ i ] ->Type() == type_vector )
			{
				CqString _strSpace( "shader" );
				if ( strSpace.compare( "" ) != 0 )
					_strSpace = strSpace;
				CqVector3D p;
				pVMVal->GetVector( p, 0 );
				pVMVal->SetVector( QGetRenderContextI() ->matVSpaceToSpace( _strSpace.c_str(), "camera", matCurrent(), matObjectToWorld ) * p );
			}
			else if ( m_LocalVars[ i ] ->Type() == type_matrix )
			{
				CqString _strSpace( "shader" );
				if ( strSpace.compare( "" ) != 0 )
					_strSpace = strSpace;
				CqMatrix m;
				pVMVal->GetMatrix( m, 0 );
				pVMVal->SetMatrix( QGetRenderContextI() ->matVSpaceToSpace( _strSpace.c_str(), "camera", matCurrent(), matObjectToWorld ) * m );
			}

			if ( pArray ) 
				pArray->ArrayEntry( arrayindex++ ) ->SetValueFromVariable( pVMVal );
			else
				m_LocalVars[ i ] ->SetValueFromVariable( pVMVal );

			DeleteTemporaryStorage(pVMVal);
		}
	}
}


//---------------------------------------------------------------------
/** Get a value from an instance variable on this shader, and fill in the passed variable reference.
 */

TqBool CqShaderVM::GetValue( const char* name, IqShaderData* res )
{
	// Find the relevant variable.
	TqInt i = FindLocalVarIndex( name );
	if ( i >= 0 )
	{
		res->SetValueFromVariable( m_LocalVars[ i ]);
		return ( TqTrue );
	}
	return ( TqFalse );
}


/* Begin changes to add accessors for libslxargs */
int CqShaderVM::GetShaderVarCount()
{
    return m_LocalVars.size();
}

IqShaderData * CqShaderVM::GetShaderVarAt(int varIndex)
{
    IqShaderData * result;
    result = NULL;
    if (varIndex >= 0)
    {
        if (varIndex < m_LocalVars.size())
        {
            result = m_LocalVars[varIndex];
        }
    }
    return result;
}
/* End changes to add accessors for libslxargs */



void CqShaderVM::SO_nop()
{}

void CqShaderVM::SO_dup()
{
	Dup();
}

void CqShaderVM::SO_drop()
{
	Drop();
}

void CqShaderVM::SO_debug_break()
{}

void CqShaderVM::SO_pushif()
{
	AUTOFUNC;
	RESULT(type_float, class_uniform);
	pResult->SetFloat( ReadNext().m_FloatVal );
	Push( pResult );
}

void CqShaderVM::SO_puship()
{
	AUTOFUNC;
	TqFloat f = ReadNext().m_FloatVal;
	TqFloat f2 = ReadNext().m_FloatVal;
	TqFloat f3 = ReadNext().m_FloatVal;
	RESULT(type_point, class_uniform);
	CqVector3D v(f,f2,f3);
	pResult->SetValue(v);
	Push( pResult );
}

void CqShaderVM::SO_pushis()
{
	AUTOFUNC;
	RESULT(type_string, class_uniform);
	CqString * ps = ReadNext().m_pString;
	pResult->SetValue( *ps );
	Push( pResult );
}

void CqShaderVM::SO_pushv()
{
	Push( GetVar( ReadNext().m_iVariable ) );
}

void CqShaderVM::SO_ipushv()
{
	AUTOFUNC;
	POPV( A );	// Index
	IqShaderData* pVar = GetVar( ReadNext().m_iVariable );
	if ( pVar->ArrayLength() == 0 )
	{
		// Report error.
		//CqBasicError( 0, Severity_Fatal, "Attempt to index a non array variable" );
		return ;
	}
	RESULT(pVar->Type(), pVar->Class());
	TqInt ext = m_pEnv->GridSize();
	TqInt i;
	for ( i = 0; i < ext; i++ )
	{
		TqFloat _aq_A;
		A->GetFloat( _aq_A, i );
		pResult->SetValueFromVariable( pVar->ArrayEntry( static_cast<unsigned int>( _aq_A ) ), i );
	}
	Push( pResult );
}

void CqShaderVM::SO_pop()
{
	AUTOFUNC;
	IqShaderData* pV = GetVar( ReadNext().m_iVariable );
	POPV( Val );
	TqInt ext = MAX( m_pEnv->GridSize(), pV->Size() );
	TqBool fVarying = ext > 1;
	TqInt i;
	CqBitVector& RS = m_pEnv->RunningState();
	for ( i = 0; i < ext; i++ )
	{
		if(!fVarying || RS.Value( i ))
			pV->SetValueFromVariable( Val, i );
	}
}

void CqShaderVM::SO_ipop()
{
	AUTOFUNC;
	UsProgramElement& el = ReadNext();
	IqShaderData* pV = GetVar( el.m_iVariable );
	CqShaderVariableArray* pVA = static_cast<CqShaderVariableArray*>( pV );
	if ( pV->ArrayLength() == 0 )
	{
		// Report error.
		//CqBasicError( 0, Severity_Fatal, "Attempt to index a non array variable" );
		return ;
	}
	POPV( A );
	POPV( Val );
	//TqInt ext=__fVarying?m_pEnv->GridSize():1;
	TqInt ext = MAX( m_pEnv->GridSize(), pV->Size() );
	TqBool fVarying = ext > 1;
	TqInt i;
	CqBitVector& RS = m_pEnv->RunningState();
	for ( i = 0; i < ext; i++ )
	{
		if ( !fVarying || RS.Value( i ) )
		{
			TqFloat fIndex;
			A->GetFloat( fIndex, i );
			TqInt index = static_cast<unsigned int>( fIndex );
			( *pVA ) [ index ] ->SetValueFromVariable( Val, i );
		}
	}
}

void CqShaderVM::SO_mergef()
{
	// Get the current state from the current stack entry
	AUTOFUNC;
	POPV( A );	// Relational result
	POPV( F );	// False statement
	POPV( T );	// True statement
	RESULT(type_float, class_varying);
	TqInt i;
	for ( i = 0; i < m_pEnv->GridSize(); i++ )
	{
		TqBool _aq_A;
		TqFloat _aq_T, _aq_F;
		A->GetBool( _aq_A, i );
		T->GetFloat( _aq_T, i );
		F->GetFloat( _aq_F, i );
		if ( _aq_A ) pResult->SetValue( _aq_T, i );
		else	pResult->SetValue( _aq_F, i );
	}
	Push( pResult );
}

void CqShaderVM::SO_merges()
{
	// Get the current state from the current stack entry
	AUTOFUNC;
	POPV( A );	// Relational result
	POPV( F );	// False statement
	POPV( T );	// True statement
	RESULT(type_string, class_varying);
	TqInt i;
	for ( i = 0; i < m_pEnv->GridSize(); i++ )
	{
		TqBool _aq_A;
		CqString _aq_T, _aq_F;
		A->GetBool( _aq_A, i );
		T->GetString( _aq_T, i );
		F->GetString( _aq_F, i );
		if ( _aq_A ) pResult->SetValue( _aq_T, i );
		else	pResult->SetValue( _aq_F, i );
	}
	Push( pResult );
}

void CqShaderVM::SO_mergep()
{
	// Get the current state from the current stack entry
	AUTOFUNC;
	POPV( A );	// Relational result
	POPV( F );	// False statement
	POPV( T );	// True statement
	RESULT(type_point, class_varying);
	TqInt i;
	for ( i = 0; i < m_pEnv->GridSize(); i++ )
	{
		TqBool _aq_A;
		CqVector3D _aq_T, _aq_F;
		A->GetBool( _aq_A, i );
		T->GetPoint( _aq_T, i );
		F->GetPoint( _aq_F, i );
		if ( _aq_A ) pResult->SetValue( _aq_T, i );
		else	pResult->SetValue( _aq_F, i );
	}
	Push( pResult );
}

void CqShaderVM::SO_mergec()
{
	// Get the current state from the current stack entry
	AUTOFUNC;
	POPV( A );	// Relational result
	POPV( F );	// False statement
	POPV( T );	// True statement
	RESULT(type_color, class_varying);
	TqInt i;
	for ( i = m_pEnv->GridSize() - 1; i >= 0; i-- )
	{
		TqBool _aq_A;
		CqColor _aq_T, _aq_F;
		A->GetBool( _aq_A, i );
		T->GetColor( _aq_T, i );
		F->GetColor( _aq_F, i );
		if ( _aq_A ) pResult->SetValue( _aq_T, i );
		else	pResult->SetValue( _aq_F, i );
	}
	Push( pResult );
}

void CqShaderVM::SO_setfc()
{
	AUTOFUNC;
	POPV( A );
	RESULT(type_color, __fVarying?class_varying:class_uniform);
	OpCAST_FC( A, pResult, m_pEnv->RunningState() );
	Push( pResult );
}

void CqShaderVM::SO_setfp()
{
	AUTOFUNC;
	POPV( A );
	RESULT(type_point, __fVarying?class_varying:class_uniform);
	OpCAST_FP( A, pResult, m_pEnv->RunningState() );
	Push( pResult );
}


void CqShaderVM::SO_setfm()
{
	AUTOFUNC;
	POPV( A );
	RESULT(type_matrix, __fVarying?class_varying:class_uniform);
	OpCAST_FM( A, pResult, m_pEnv->RunningState() );
	Push( pResult );
}

void CqShaderVM::SO_settc()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	POPV( C );
	RESULT(type_color, __fVarying?class_varying:class_uniform);
	OpTRIPLE_C( pResult, A, B, C, m_pEnv->RunningState() );
	Push( pResult );
}

void CqShaderVM::SO_settp()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	POPV( C );
	RESULT(type_point, __fVarying?class_varying:class_uniform);
	OpTRIPLE_P( pResult, A, B, C, m_pEnv->RunningState() );
	Push( pResult );
}

void CqShaderVM::SO_setpc()
{
	AUTOFUNC;
	POPV( A );
	RESULT(type_color, __fVarying?class_varying:class_uniform);
	OpCAST_PC( A, pResult, m_pEnv->RunningState() );
	Push( pResult );
}

void CqShaderVM::SO_setcp()
{
	AUTOFUNC;
	POPV( A );
	RESULT(type_point, __fVarying?class_varying:class_uniform);
	OpCAST_CP( A, pResult, m_pEnv->RunningState() );
	Push( pResult );
}

void CqShaderVM::SO_setwm()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	POPV( C );
	POPV( D );
	POPV( E );
	POPV( F );
	POPV( G );
	POPV( H );
	POPV( I );
	POPV( J );
	POPV( K );
	POPV( L );
	POPV( M );
	POPV( N );
	POPV( O );
	POPV( P );
	RESULT(type_matrix, __fVarying?class_varying:class_uniform);
	OpHEXTUPLE_M( pResult, P, O, N, M, L, K, J, I, H, G, F, E, D, C, B, A, m_pEnv->RunningState() );
	Push( pResult );
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
	POPV( A );
	TqInt i;
	CqBitVector& RS = m_pEnv->RunningState();
	for ( i = m_pEnv->GridSize() - 1; i >= 0; i-- )
	{
		if ( RS.Value( i ) )
		{
			TqBool _aq_A;
			A->GetBool( _aq_A, i );
			m_pEnv->CurrentState().SetValue( i, _aq_A );
		}
	}
}

void CqShaderVM::SO_RS_JZ()
{
	SqLabel lab = ReadNext().m_Label;
	if ( m_pEnv->RunningState().Count() == 0 )
	{
		m_PO = lab.m_Offset;
		m_PC = lab.m_pAddress;
	}
}

void CqShaderVM::SO_RS_JNZ()
{
	SqLabel lab = ReadNext().m_Label;
	if ( m_pEnv->RunningState().Count() == m_pEnv->RunningState().Size() )
	{
		m_PO = lab.m_Offset;
		m_PC = lab.m_pAddress;
	}
}

void CqShaderVM::SO_S_JZ()
{
	SqLabel lab = ReadNext().m_Label;
	if ( m_pEnv->CurrentState().Count() == 0 )
	{
		m_PO = lab.m_Offset;
		m_PC = lab.m_pAddress;
	}
}

void CqShaderVM::SO_S_JNZ()
{
	SqLabel lab = ReadNext().m_Label;
	if ( m_pEnv->CurrentState().Count() == m_pEnv->RunningState().Size() )
	{
		m_PO = lab.m_Offset;
		m_PC = lab.m_pAddress;
	}
}

void CqShaderVM::SO_jnz()
{
	SqLabel lab = ReadNext().m_Label;
	AUTOFUNC;
	IqShaderData* f = POP;
	TqInt __iGrid = 0;
	do
	{
		if ( !__fVarying || m_pEnv->RunningState().Value( __iGrid ) )
		{
			TqBool _f;
			f->GetBool( _f, __iGrid );
			if ( !_f ) return ;
		}
	}
	while ( ++__iGrid < m_pEnv->GridSize() );
	m_PO = lab.m_Offset;
	m_PC = lab.m_pAddress;
}

void CqShaderVM::SO_jz()
{
	SqLabel lab = ReadNext().m_Label;
	AUTOFUNC;
	IqShaderData* f = POP;
	TqInt __iGrid = 0;
	do
	{
		if ( !__fVarying || m_pEnv->RunningState().Value( __iGrid ) )
		{
			TqBool _f;
			f->GetBool( _f, __iGrid );
			if ( _f ) return ;
		}
	}
	while ( ++__iGrid < m_pEnv->GridSize() );
	m_PO = lab.m_Offset;
	m_PC = lab.m_pAddress;
}

void CqShaderVM::SO_jmp()
{
	SqLabel lab = ReadNext().m_Label;
	m_PO = lab.m_Offset;
	m_PC = lab.m_pAddress;
}

void CqShaderVM::SO_lsff()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	OpLSS_FF( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
}

void CqShaderVM::SO_lspp()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	OpLSS_PP( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
}

void CqShaderVM::SO_lscc()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	OpLSS_CC( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
}

void CqShaderVM::SO_gtff()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	OpGRT_FF( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
}

void CqShaderVM::SO_gtpp()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	OpGRT_PP( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
}

void CqShaderVM::SO_gtcc()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	OpGRT_CC( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
}

void CqShaderVM::SO_geff()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	OpGE_FF( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
}

void CqShaderVM::SO_gepp()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	OpGE_PP( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
}

void CqShaderVM::SO_gecc()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	OpGE_CC( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
}

void CqShaderVM::SO_leff()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	OpLE_FF( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
}

void CqShaderVM::SO_lepp()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	OpLE_PP( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
}

void CqShaderVM::SO_lecc()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	OpLE_CC( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
}

void CqShaderVM::SO_eqff()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	OpEQ_FF( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
}

void CqShaderVM::SO_eqpp()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	OpEQ_PP( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
}

void CqShaderVM::SO_eqcc()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	OpEQ_CC( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
}

void CqShaderVM::SO_eqss()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	OpEQ_SS( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
}

void CqShaderVM::SO_neff()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	OpNE_FF( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
}

void CqShaderVM::SO_nepp()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	OpNE_PP( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
}

void CqShaderVM::SO_necc()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	OpNE_CC( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
}

void CqShaderVM::SO_ness()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	OpNE_SS( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
}

void CqShaderVM::SO_mulff()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	OpMUL_FF( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
}

void CqShaderVM::SO_divff()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	OpDIV_FF( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
}

void CqShaderVM::SO_addff()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	OpADD_FF( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
}

void CqShaderVM::SO_subff()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	OpSUB_FF( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
}

void CqShaderVM::SO_negf()
{
	AUTOFUNC;
	POPV( A );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	OpNEG_F( A, pResult, m_pEnv->RunningState() );
	Push( pResult );
}

void CqShaderVM::SO_mulpp()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_point, __fVarying?class_varying:class_uniform);
	OpMULV( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
}

void CqShaderVM::SO_divpp()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_point, __fVarying?class_varying:class_uniform);
	OpDIV_PP( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
}

void CqShaderVM::SO_addpp()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_point, __fVarying?class_varying:class_uniform);
	OpADD_PP( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
}

void CqShaderVM::SO_subpp()
{
	AUTOFUNC; 
	POPV( A );
	POPV( B );
	RESULT(type_point, __fVarying?class_varying:class_uniform);
	OpSUB_PP( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
}

void CqShaderVM::SO_crspp()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_point, __fVarying?class_varying:class_uniform);
	OpCRS_PP( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
}

void CqShaderVM::SO_dotpp()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	OpDOT_PP( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
}

void CqShaderVM::SO_negp()
{
	AUTOFUNC;
	POPV( A );
	RESULT(type_point, __fVarying?class_varying:class_uniform);
	OpNEG_P( A, pResult, m_pEnv->RunningState() );
	Push( pResult );
}

void CqShaderVM::SO_mulcc()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_color, __fVarying?class_varying:class_uniform);
	OpMUL_CC( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
}

void CqShaderVM::SO_divcc()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_color, __fVarying?class_varying:class_uniform);
	OpDIV_CC( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
}

void CqShaderVM::SO_addcc()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_color, __fVarying?class_varying:class_uniform);
	OpADD_CC( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
}

void CqShaderVM::SO_subcc()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_color, __fVarying?class_varying:class_uniform);
	OpSUB_CC( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
}

void CqShaderVM::SO_crscc()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_color, __fVarying?class_varying:class_uniform);
	OpCRS_CC( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
}

void CqShaderVM::SO_dotcc()
{
	AUTOFUNC; 
	POPV( A );
	POPV( B );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	OpDOT_CC( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
}

void CqShaderVM::SO_negc()
{
	AUTOFUNC;
	POPV( A );
	RESULT(type_color, __fVarying?class_varying:class_uniform);
	OpNEG_C( A, pResult, m_pEnv->RunningState() );
	Push( pResult );
}

void CqShaderVM::SO_mulfp()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_point, __fVarying?class_varying:class_uniform);
	OpMUL_FP( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
}

void CqShaderVM::SO_divfp()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_point, __fVarying?class_varying:class_uniform);
	OpDIV_FP( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
}

void CqShaderVM::SO_addfp()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_point, __fVarying?class_varying:class_uniform);
	OpADD_FP( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
}

void CqShaderVM::SO_subfp()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_point, __fVarying?class_varying:class_uniform);
	OpSUB_FP( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
}

void CqShaderVM::SO_mulfc()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_color, __fVarying?class_varying:class_uniform);
	OpMUL_FC( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
}

void CqShaderVM::SO_divfc()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_color, __fVarying?class_varying:class_uniform);
	OpDIV_FC( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
}

void CqShaderVM::SO_addfc()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_color, __fVarying?class_varying:class_uniform);
	OpADD_FC( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
}

void CqShaderVM::SO_subfc()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_color, __fVarying?class_varying:class_uniform);
	OpSUB_FC( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
}

void CqShaderVM::SO_mulmm()
{
	AUTOFUNC;
	RESULT(type_float, class_uniform);
	pResult->SetFloat( 0.0f );
	Push( pResult );	/* TODO: Implement matrices in the VM*/
}

void CqShaderVM::SO_divmm()
{
	AUTOFUNC;
	RESULT(type_float, class_uniform);
	pResult->SetFloat( 0.0f );
	Push( pResult );	/* TODO: Implement matrices in the VM*/
}

void CqShaderVM::SO_land()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	OpLAND_B( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
}

void CqShaderVM::SO_lor()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	OpLOR_B( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
}

void CqShaderVM::SO_radians()
{
	AUTOFUNC;
	FUNC1( type_float, m_pEnv->SO_radians );
}

void CqShaderVM::SO_degrees()
{
	AUTOFUNC;
	FUNC1( type_float, m_pEnv->SO_degrees );
}

void CqShaderVM::SO_sin()
{
	AUTOFUNC;
	FUNC1( type_float, m_pEnv->SO_sin );
}

void CqShaderVM::SO_asin()
{
	AUTOFUNC;
	FUNC1( type_float, m_pEnv->SO_asin );
}

void CqShaderVM::SO_cos()
{
	AUTOFUNC;
	FUNC1( type_float, m_pEnv->SO_cos );
}

void CqShaderVM::SO_acos()
{
	AUTOFUNC;
	FUNC1( type_float, m_pEnv->SO_acos );
}

void CqShaderVM::SO_tan()
{
	AUTOFUNC;
	FUNC1( type_float, m_pEnv->SO_tan );
}

void CqShaderVM::SO_atan()
{
	AUTOFUNC;
	FUNC1( type_float, m_pEnv->SO_atan );
}

void CqShaderVM::SO_atan2()
{
	AUTOFUNC;
	FUNC2( type_float, m_pEnv->SO_atan );
}

void CqShaderVM::SO_pow()
{
	AUTOFUNC;
	FUNC2( type_float, m_pEnv->SO_pow );
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
	AUTOFUNC;
	FUNC( type_float, m_pEnv->SO_frandom );
}

void CqShaderVM::SO_crandom()
{
	AUTOFUNC;
	FUNC( type_color, m_pEnv->SO_crandom );
}

void CqShaderVM::SO_prandom()
{
	AUTOFUNC;
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
	OpCOMP_P( A, 0, pResult, m_pEnv->RunningState() );
	Push( pResult );
}

void CqShaderVM::SO_ycomp()
{
	AUTOFUNC;
	POPV( A );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	OpCOMP_P( A, 1, pResult, m_pEnv->RunningState() );
	Push( pResult );
}

void CqShaderVM::SO_zcomp()
{
	AUTOFUNC;
	POPV( A );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	OpCOMP_P( A, 2, pResult, m_pEnv->RunningState() );
	Push( pResult );
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
	OpCOMP_C( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
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
	m_pEnv->InvalidateIlluminanceCache();
	m_pEnv->ValidateIlluminanceCache( A, this );
	RESULT(type_float, class_varying);
	pResult->SetFloat( m_pEnv->SO_init_illuminance() );
	Push( pResult );
}

void CqShaderVM::SO_advance_illuminance()
{
	RESULT(type_float, class_varying);
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
	VARFUNC;
	VOIDFUNC( m_pEnv->SO_solar );
}

void CqShaderVM::SO_solar2()
{
	VARFUNC;
	VOIDFUNC2( m_pEnv->SO_solar );
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
	m_pEnv->SO_atmosphere( Val, pV, pResult );
	Push( pResult );
}

void CqShaderVM::SO_displacement()
{
	AUTOFUNC;
	IqShaderData* pV = GetVar( ReadNext().m_iVariable );
	POPV( Val );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	m_pEnv->SO_displacement( Val, pV, pResult );
	Push( pResult );
}

void CqShaderVM::SO_lightsource()
{
	AUTOFUNC;
	IqShaderData* pV = GetVar( ReadNext().m_iVariable );
	POPV( Val );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	m_pEnv->SO_lightsource( Val, pV, pResult );
	Push( pResult );
}

void CqShaderVM::SO_surface()
{
	AUTOFUNC;
	IqShaderData* pV = GetVar( ReadNext().m_iVariable );
	POPV( Val );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	m_pEnv->SO_surface( Val, pV, pResult );
	Push( pResult );
}

void CqShaderVM::SO_attribute()
{
	AUTOFUNC;
	IqShaderData* pV = GetVar( ReadNext().m_iVariable );
	POPV( Val );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	m_pEnv->SO_attribute( Val, pV, pResult );
	Push( pResult );
}

void CqShaderVM::SO_option()
{
	AUTOFUNC;
	IqShaderData* pV = GetVar( ReadNext().m_iVariable );
	POPV( Val );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	m_pEnv->SO_option( Val, pV, pResult );
	Push( pResult );
}

void CqShaderVM::SO_rendererinfo()
{
	AUTOFUNC;
	IqShaderData* pV = GetVar( ReadNext().m_iVariable );
	POPV( Val );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	m_pEnv->SO_rendererinfo( Val, pV, pResult );
	Push( pResult );
}

void CqShaderVM::SO_textureinfo()
{

	AUTOFUNC;
	IqShaderData* pV = GetVar( ReadNext().m_iVariable );
	POPV( Val );
	POPV( DataInfo );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	m_pEnv->SO_textureinfo( Val, DataInfo, pV, pResult );
	Push( pResult );
}

void CqShaderVM::SO_incident()
{
	AUTOFUNC;
	IqShaderData* pV = GetVar( ReadNext().m_iVariable );
	POPV( Val );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	m_pEnv->SO_incident( Val, pV, pResult );
	Push( pResult );
}

void CqShaderVM::SO_opposite()
{
	AUTOFUNC;
	IqShaderData* pV = GetVar( ReadNext().m_iVariable );
	POPV( Val );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	m_pEnv->SO_opposite( Val, pV, pResult );
	Push( pResult );
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
	AUTOFUNC;;
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
	OpCOMPM( A, B, C, pResult, m_pEnv->RunningState() );
	Push( pResult );
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


END_NAMESPACE( Aqsis )
//---------------------------------------------------------------------
