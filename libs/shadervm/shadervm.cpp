// aQSIS
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
		\brief Implements classes and support functionality for the shader virtual machine.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#include "shadervm.h"

#include <cstring>
#include <ctype.h>
#include <iostream>
#include <sstream>
#include <stddef.h>

#include <aqsis/core/isurface.h>
#include <aqsis/util/logging.h>
#include "shadervariable.h"
#include <aqsis/util/sstring.h>
#include <aqsis/version.h>


namespace Aqsis {

//------------------------------------------------------------------------------
// Implement external interface functions for CqShaderVM:

boost::shared_ptr<IqShader> createShaderVM(IqRenderer* renderContext)
{
	return boost::shared_ptr<IqShader>(new CqShaderVM(renderContext));
}

boost::shared_ptr<IqShader> createShaderVM(IqRenderer* renderContext,
                                           std::istream& programFile,
                                           const std::string& dsoPath)
{
	boost::shared_ptr<CqShaderVM> shader(new CqShaderVM(renderContext));
	if(!dsoPath.empty())
		shader->SetDSOPath(dsoPath.c_str());
	shader->LoadProgram(&programFile);
	return shader;
}

void shutdownShaderVM()
{
	CqShaderVM::ShutdownShaderEngine();
}

//------------------------------------------------------------------------------

/*
 * Type, name and private hash key for the name
 */
static struct shader_types
{
	const TqChar* name;
	EqShaderType type;
	TqUlong hash;
}
gShaderTypeNames[] =
    {
        {"surface", Type_Surface, 0},
        {"lightsource", Type_Lightsource, 0},
        {"volume", Type_Volume, 0},
        {"displacement", Type_Displacement, 0},
        {"transformation", Type_Transformation, 0},
        {"imager", Type_Imager, 0}
    };
TqInt gcShaderTypeNames = sizeof( gShaderTypeNames ) / sizeof( struct shader_types); //gShaderTypeNames[ 0 ] );


/*
 * Huge common shader routine definition translation table
 * Name, hash key for the opcode name,
 * Funtion pointer, N parameters, Parameters type, 
 */
SqOpCodeTrans CqShaderVM::m_TransTable[] =
    {
        {"RS_PUSH", 0, &CqShaderVM::SO_RS_PUSH, 0, {0}},
        {"RS_POP", 0, &CqShaderVM::SO_RS_POP, 0, {0}},
        {"RS_GET", 0, &CqShaderVM::SO_RS_GET, 0, {0}},
        {"RS_INVERSE", 0, &CqShaderVM::SO_RS_INVERSE, 0, {0}},
        {"RS_JZ", 0, &CqShaderVM::SO_RS_JZ, 1, {type_float}},
        {"RS_BREAK", 0, &CqShaderVM::SO_RS_BREAK, 1, {type_integer}},
        {"S_JZ", 0, &CqShaderVM::SO_S_JZ, 1, {type_float}},
        {"S_GET", 0, &CqShaderVM::SO_S_GET, 0, {0}},
        {"S_CLEAR", 0, &CqShaderVM::SO_S_CLEAR, 0, {0}},

        {"nop", 0, &CqShaderVM::SO_nop, 0, {0}},
        {"dup", 0, &CqShaderVM::SO_dup, 0, {0}},
        {"debug_break", 0, &CqShaderVM::SO_debug_break, 0, {0}},
        {"drop", 0, &CqShaderVM::SO_drop, 0, {0}},

        {"mergef", 0, &CqShaderVM::SO_mergef, 0, {0}},
        {"merges", 0, &CqShaderVM::SO_merges, 0, {0}},
        {"mergep", 0, &CqShaderVM::SO_mergep, 0, {0}},
        {"mergen", 0, &CqShaderVM::SO_mergep, 0, {0}},
        {"mergev", 0, &CqShaderVM::SO_mergep, 0, {0}},
        {"mergec", 0, &CqShaderVM::SO_mergec, 0, {0}},

        {"pushif", 0, &CqShaderVM::SO_pushif, 1, {type_float}},
        {"puship", 0, &CqShaderVM::SO_puship, 3, {type_float, type_float, type_float}},
        {"pushis", 0, &CqShaderVM::SO_pushis, 1, {type_string}},
        {"pushv", 0, &CqShaderVM::SO_pushv, 1, {type_invalid}},
        {"ipushv", 0, &CqShaderVM::SO_ipushv, 1, {type_invalid}},

        {"pop", 0, &CqShaderVM::SO_pop, 1, {type_invalid}},
        {"ipop", 0, &CqShaderVM::SO_ipop, 1, {type_invalid}},

        {"setfc", 0, &CqShaderVM::SO_setfc, 0, {0}},
        {"setfp", 0, &CqShaderVM::SO_setfp, 0, {0}},
        {"setfn", 0, &CqShaderVM::SO_setfp, 0, {0}},
        {"setfv", 0, &CqShaderVM::SO_setfp, 0, {0}},
        {"setfm", 0, &CqShaderVM::SO_setfm, 0, {0}},

        {"settc", 0, &CqShaderVM::SO_settc, 0, {0}},
        {"settp", 0, &CqShaderVM::SO_settp, 0, {0}},
        {"settn", 0, &CqShaderVM::SO_settp, 0, {0}},
        {"settv", 0, &CqShaderVM::SO_settp, 0, {0}},

        {"setpc", 0, &CqShaderVM::SO_setpc, 0, {0}},
        {"setvc", 0, &CqShaderVM::SO_setpc, 0, {0}},
        {"setnc", 0, &CqShaderVM::SO_setpc, 0, {0}},

        {"setcp", 0, &CqShaderVM::SO_setcp, 0, {0}},
        {"setcn", 0, &CqShaderVM::SO_setcp, 0, {0}},
        {"setcv", 0, &CqShaderVM::SO_setcp, 0, {0}},

        {"setwm", 0, &CqShaderVM::SO_setwm, 0, {0}},

        {"jnz", 0, &CqShaderVM::SO_jnz, 1, {type_float}},
        {"jz", 0, &CqShaderVM::SO_jz, 1, {type_float}},
        {"jmp", 0, &CqShaderVM::SO_jmp, 1, {type_float}},

        {"lsff", 0, &CqShaderVM::SO_lsff, 0, {0}},
        {"lspp", 0, &CqShaderVM::SO_lspp, 0, {0}},
        {"lshh", 0, &CqShaderVM::SO_lspp, 0, {0}},
        {"lscc", 0, &CqShaderVM::SO_lscc, 0, {0}},
        {"lsnn", 0, &CqShaderVM::SO_lspp, 0, {0}},
        {"lsvv", 0, &CqShaderVM::SO_lspp, 0, {0}},

        {"gtff", 0, &CqShaderVM::SO_gtff, 0, {0}},
        {"gtpp", 0, &CqShaderVM::SO_gtpp, 0, {0}},
        {"gthh", 0, &CqShaderVM::SO_gtpp, 0, {0}},
        {"gtcc", 0, &CqShaderVM::SO_gtcc, 0, {0}},
        {"gtnn", 0, &CqShaderVM::SO_gtpp, 0, {0}},
        {"gtvv", 0, &CqShaderVM::SO_gtpp, 0, {0}},

        {"geff", 0, &CqShaderVM::SO_geff, 0, {0}},
        {"gepp", 0, &CqShaderVM::SO_gepp, 0, {0}},
        {"gehh", 0, &CqShaderVM::SO_gepp, 0, {0}},
        {"gecc", 0, &CqShaderVM::SO_gecc, 0, {0}},
        {"genn", 0, &CqShaderVM::SO_gepp, 0, {0}},
        {"gevv", 0, &CqShaderVM::SO_gepp, 0, {0}},

        {"leff", 0, &CqShaderVM::SO_leff, 0, {0}},
        {"lepp", 0, &CqShaderVM::SO_lepp, 0, {0}},
        {"lehh", 0, &CqShaderVM::SO_lepp, 0, {0}},
        {"lecc", 0, &CqShaderVM::SO_lecc, 0, {0}},
        {"lenn", 0, &CqShaderVM::SO_lepp, 0, {0}},
        {"levv", 0, &CqShaderVM::SO_lepp, 0, {0}},

        {"eqff", 0, &CqShaderVM::SO_eqff, 0, {0}},
        {"eqpp", 0, &CqShaderVM::SO_eqpp, 0, {0}},
        {"eqhh", 0, &CqShaderVM::SO_eqpp, 0, {0}},
        {"eqcc", 0, &CqShaderVM::SO_eqcc, 0, {0}},
        {"eqss", 0, &CqShaderVM::SO_eqss, 0, {0}},
        {"eqnn", 0, &CqShaderVM::SO_eqpp, 0, {0}},
        {"eqvv", 0, &CqShaderVM::SO_eqpp, 0, {0}},

        {"neff", 0, &CqShaderVM::SO_neff, 0, {0}},
        {"nepp", 0, &CqShaderVM::SO_nepp, 0, {0}},
        {"nehh", 0, &CqShaderVM::SO_nepp, 0, {0}},
        {"necc", 0, &CqShaderVM::SO_necc, 0, {0}},
        {"ness", 0, &CqShaderVM::SO_ness, 0, {0}},
        {"nenn", 0, &CqShaderVM::SO_nepp, 0, {0}},
        {"nevv", 0, &CqShaderVM::SO_nepp, 0, {0}},

        {"mulff", 0, &CqShaderVM::SO_mulff, 0, {0}},
        {"divff", 0, &CqShaderVM::SO_divff, 0, {0}},
        {"addff", 0, &CqShaderVM::SO_addff, 0, {0}},
        {"subff", 0, &CqShaderVM::SO_subff, 0, {0}},
        {"negf", 0, &CqShaderVM::SO_negf, 0, {0}},

        {"mulpp", 0, &CqShaderVM::SO_mulpp, 0, {0}},
        {"divpp", 0, &CqShaderVM::SO_divpp, 0, {0}},
        {"addpp", 0, &CqShaderVM::SO_addpp, 0, {0}},
        {"subpp", 0, &CqShaderVM::SO_subpp, 0, {0}},
        {"crspp", 0, &CqShaderVM::SO_crspp, 0, {0}},
        {"dotpp", 0, &CqShaderVM::SO_dotpp, 0, {0}},
        {"negp", 0, &CqShaderVM::SO_negp, 0, {0}},

        {"mulcc", 0, &CqShaderVM::SO_mulcc, 0, {0}},
        {"divcc", 0, &CqShaderVM::SO_divcc, 0, {0}},
        {"addcc", 0, &CqShaderVM::SO_addcc, 0, {0}},
        {"subcc", 0, &CqShaderVM::SO_subcc, 0, {0}},
        {"dotcc", 0, &CqShaderVM::SO_dotcc, 0, {0}},
        {"negc", 0, &CqShaderVM::SO_negc, 0, {0}},

        {"mulfp", 0, &CqShaderVM::SO_mulfp, 0, {0}},
        {"divfp", 0, &CqShaderVM::SO_divfp, 0, {0}},
        {"addfp", 0, &CqShaderVM::SO_addfp, 0, {0}},
        {"subfp", 0, &CqShaderVM::SO_subfp, 0, {0}},

        {"mulfc", 0, &CqShaderVM::SO_mulfc, 0, {0}},
        {"divfc", 0, &CqShaderVM::SO_divfc, 0, {0}},
        {"addfc", 0, &CqShaderVM::SO_addfc, 0, {0}},
        {"subfc", 0, &CqShaderVM::SO_subfc, 0, {0}},

        {"mulmm", 0, &CqShaderVM::SO_mulmm, 0, {0}},
        {"divmm", 0, &CqShaderVM::SO_divmm, 0, {0}},

        {"land", 0, &CqShaderVM::SO_land, 0, {0}},
        {"lor", 0, &CqShaderVM::SO_lor, 0, {0}},

        {"radians", 0, &CqShaderVM::SO_radians, 0, {0}},
        {"degrees", 0, &CqShaderVM::SO_degrees, 0, {0}},
        {"sin", 0, &CqShaderVM::SO_sin, 0, {0}},
        {"asin", 0, &CqShaderVM::SO_asin, 0, {0}},
        {"cos", 0, &CqShaderVM::SO_cos, 0, {0}},
        {"acos", 0, &CqShaderVM::SO_acos, 0, {0}},
        {"tan", 0, &CqShaderVM::SO_tan, 0, {0}},
        {"atan", 0, &CqShaderVM::SO_atan, 0, {0}},
        {"atan2", 0, &CqShaderVM::SO_atan2, 0, {0}},
        {"pow", 0, &CqShaderVM::SO_pow, 0, {0}},
        {"exp", 0, &CqShaderVM::SO_exp, 0, {0}},
        {"sqrt", 0, &CqShaderVM::SO_sqrt, 0, {0}},
        {"log", 0, &CqShaderVM::SO_log, 0, {0}},
        {"log2", 0, &CqShaderVM::SO_log2, 0, {0}},
        {"mod", 0, &CqShaderVM::SO_mod, 0, {0}},
        {"abs", 0, &CqShaderVM::SO_abs, 0, {0}},
        {"sign", 0, &CqShaderVM::SO_sign, 0, {0}},
        {"min", 0, &CqShaderVM::SO_min, 0, {0}},
        {"max", 0, &CqShaderVM::SO_max, 0, {0}},
        {"pmin", 0, &CqShaderVM::SO_pmin, 0, {0}},
        {"pmax", 0, &CqShaderVM::SO_pmax, 0, {0}},
        {"vmin", 0, &CqShaderVM::SO_vmin, 0, {0}},
        {"vmax", 0, &CqShaderVM::SO_vmax, 0, {0}},
        {"nmin", 0, &CqShaderVM::SO_nmin, 0, {0}},
        {"nmax", 0, &CqShaderVM::SO_nmax, 0, {0}},
        {"cmin", 0, &CqShaderVM::SO_cmin, 0, {0}},
        {"cmax", 0, &CqShaderVM::SO_cmax, 0, {0}},
        {"clamp", 0, &CqShaderVM::SO_clamp, 0, {0}},
        {"pclamp", 0, &CqShaderVM::SO_pclamp, 0, {0}},
        {"vclamp", 0, &CqShaderVM::SO_pclamp, 0, {0}},
        {"nclamp", 0, &CqShaderVM::SO_pclamp, 0, {0}},
        {"cclamp", 0, &CqShaderVM::SO_cclamp, 0, {0}},
        {"floor", 0, &CqShaderVM::SO_floor, 0, {0}},
        {"ceil", 0, &CqShaderVM::SO_ceil, 0, {0}},
        {"round", 0, &CqShaderVM::SO_round, 0, {0}},
        {"step", 0, &CqShaderVM::SO_step, 0, {0}},
        {"smoothstep", 0, &CqShaderVM::SO_smoothstep, 0, {0}},
        {"fspline", 0, &CqShaderVM::SO_fspline, 0, {0}},
        {"cspline", 0, &CqShaderVM::SO_cspline, 0, {0}},
        {"pspline", 0, &CqShaderVM::SO_pspline, 0, {0}},
        {"vspline", 0, &CqShaderVM::SO_pspline, 0, {0}},
        {"sfspline", 0, &CqShaderVM::SO_sfspline, 0, {0}},
        {"scspline", 0, &CqShaderVM::SO_scspline, 0, {0}},
        {"spspline", 0, &CqShaderVM::SO_spspline, 0, {0}},
        {"svspline", 0, &CqShaderVM::SO_spspline, 0, {0}},
        {"fDu", 0, &CqShaderVM::SO_fDu, 0, {0}},
        {"fDv", 0, &CqShaderVM::SO_fDv, 0, {0}},
        {"fDeriv", 0, &CqShaderVM::SO_fDeriv, 0, {0}},
        {"cDu", 0, &CqShaderVM::SO_cDu, 0, {0}},
        {"cDv", 0, &CqShaderVM::SO_cDv, 0, {0}},
        {"cDeriv", 0, &CqShaderVM::SO_cDeriv, 0, {0}},
        {"pDu", 0, &CqShaderVM::SO_pDu, 0, {0}},
        {"pDv", 0, &CqShaderVM::SO_pDv, 0, {0}},
        {"pDeriv", 0, &CqShaderVM::SO_pDeriv, 0, {0}},
        {"hDu", 0, &CqShaderVM::SO_pDu, 0, {0}},
        {"hDv", 0, &CqShaderVM::SO_pDv, 0, {0}},
        {"hDeriv", 0, &CqShaderVM::SO_pDeriv, 0, {0}},
        {"nDu", 0, &CqShaderVM::SO_pDu, 0, {0}},
        {"nDv", 0, &CqShaderVM::SO_pDv, 0, {0}},
        {"nDeriv", 0, &CqShaderVM::SO_pDeriv, 0, {0}},
        {"vDu", 0, &CqShaderVM::SO_pDu, 0, {0}},
        {"vDv", 0, &CqShaderVM::SO_pDv, 0, {0}},
        {"vDeriv", 0, &CqShaderVM::SO_pDeriv, 0, {0}},
        {"frandom", 0, &CqShaderVM::SO_frandom, 0, {0}},
        {"crandom", 0, &CqShaderVM::SO_crandom, 0, {0}},
        {"prandom", 0, &CqShaderVM::SO_prandom, 0, {0}},
        {"noise1", 0, &CqShaderVM::SO_noise1, 0, {0}},
        {"noise2", 0, &CqShaderVM::SO_noise2, 0, {0}},
        {"noise3", 0, &CqShaderVM::SO_noise3, 0, {0}},
        {"noise4", 0, &CqShaderVM::SO_noise4, 0, {0}},
        {"cnoise1", 0, &CqShaderVM::SO_cnoise1, 0, {0}},
        {"cnoise2", 0, &CqShaderVM::SO_cnoise2, 0, {0}},
        {"cnoise3", 0, &CqShaderVM::SO_cnoise3, 0, {0}},
        {"cnoise4", 0, &CqShaderVM::SO_cnoise4, 0, {0}},
        {"pnoise1", 0, &CqShaderVM::SO_pnoise1, 0, {0}},
        {"pnoise2", 0, &CqShaderVM::SO_pnoise2, 0, {0}},
        {"pnoise3", 0, &CqShaderVM::SO_pnoise3, 0, {0}},
        {"pnoise4", 0, &CqShaderVM::SO_pnoise4, 0, {0}},
        {"xcomp", 0, &CqShaderVM::SO_xcomp, 0, {0}},
        {"ycomp", 0, &CqShaderVM::SO_ycomp, 0, {0}},
        {"zcomp", 0, &CqShaderVM::SO_zcomp, 0, {0}},
        {"setxcomp", 0, &CqShaderVM::SO_setxcomp, 0, {0}},
        {"setycomp", 0, &CqShaderVM::SO_setycomp, 0, {0}},
        {"setzcomp", 0, &CqShaderVM::SO_setzcomp, 0, {0}},
        {"length", 0, &CqShaderVM::SO_length, 0, {0}},
        {"distance", 0, &CqShaderVM::SO_distance, 0, {0}},
        {"area", 0, &CqShaderVM::SO_area, 0, {0}},
        {"normalize", 0, &CqShaderVM::SO_normalize, 0, {0}},
        {"faceforward", 0, &CqShaderVM::SO_faceforward, 0, {0}},
        {"faceforward2", 0, &CqShaderVM::SO_faceforward2, 0, {0}},
        {"reflect", 0, &CqShaderVM::SO_reflect, 0, {0}},
        {"refract", 0, &CqShaderVM::SO_refract, 0, {0}},
        {"fresnel", 0, &CqShaderVM::SO_fresnel, 0, {0}},
        {"fresnel2", 0, &CqShaderVM::SO_fresnel2, 0, {0}},
        {"transform2", 0, &CqShaderVM::SO_transform2, 0, {0}},
        {"transformm", 0, &CqShaderVM::SO_transformm, 0, {0}},
        {"transform", 0, &CqShaderVM::SO_transform, 0, {0}},
        {"vtransform2", 0, &CqShaderVM::SO_vtransform2, 0, {0}},
        {"vtransformm", 0, &CqShaderVM::SO_vtransformm, 0, {0}},
        {"vtransform", 0, &CqShaderVM::SO_vtransform, 0, {0}},
        {"ntransform2", 0, &CqShaderVM::SO_ntransform2, 0, {0}},
        {"ntransformm", 0, &CqShaderVM::SO_ntransformm, 0, {0}},
        {"ntransform", 0, &CqShaderVM::SO_ntransform, 0, {0}},
        {"depth", 0, &CqShaderVM::SO_depth, 0, {0}},
        {"calculatenormal", 0, &CqShaderVM::SO_calculatenormal, 0, {0}},
        {"cmix", 0, &CqShaderVM::SO_cmix, 0, {0}},
        {"fmix", 0, &CqShaderVM::SO_fmix, 0, {0}},
        {"pmix", 0, &CqShaderVM::SO_pmix, 0, {0}},
        {"vmix", 0, &CqShaderVM::SO_vmix, 0, {0}},
        {"nmix", 0, &CqShaderVM::SO_nmix, 0, {0}},
        {"comp", 0, &CqShaderVM::SO_comp, 0, {0}},
        {"cmixc", 0, &CqShaderVM::SO_cmixc, 0, {0}},
        {"pmixc", 0, &CqShaderVM::SO_pmixc, 0, {0}},
        {"vmixc", 0, &CqShaderVM::SO_vmixc, 0, {0}},
        {"nmixc", 0, &CqShaderVM::SO_nmixc, 0, {0}},
        {"setcomp", 0, &CqShaderVM::SO_setcomp, 0, {0}},
        {"ambient", 0, &CqShaderVM::SO_ambient, 0, {0}},
        {"diffuse", 0, &CqShaderVM::SO_diffuse, 0, {0}},
        {"specular", 0, &CqShaderVM::SO_specular, 0, {0}},
        {"phong", 0, &CqShaderVM::SO_phong, 0, {0}},
        {"trace", 0, &CqShaderVM::SO_trace, 0, {0}},
        {"ftexture1", 0, &CqShaderVM::SO_ftexture1, 0, {0}},
        {"ftexture2", 0, &CqShaderVM::SO_ftexture2, 0, {0}},
        {"ftexture3", 0, &CqShaderVM::SO_ftexture3, 0, {0}},
        {"ctexture1", 0, &CqShaderVM::SO_ctexture1, 0, {0}},
        {"ctexture2", 0, &CqShaderVM::SO_ctexture2, 0, {0}},
        {"ctexture3", 0, &CqShaderVM::SO_ctexture3, 0, {0}},
        {"textureinfo", 0, &CqShaderVM::SO_textureinfo, 1, {type_invalid}},
        {"fenvironment2", 0, &CqShaderVM::SO_fenvironment2, 0, {0}},
        {"fenvironment3", 0, &CqShaderVM::SO_fenvironment3, 0, {0}},
        {"cenvironment2", 0, &CqShaderVM::SO_cenvironment2, 0, {0}},
        {"cenvironment3", 0, &CqShaderVM::SO_cenvironment3, 0, {0}},
        {"bump1", 0, &CqShaderVM::SO_bump1, 0, {0}},
        {"bump2", 0, &CqShaderVM::SO_bump2, 0, {0}},
        {"bump3", 0, &CqShaderVM::SO_bump3, 0, {0}},
        {"shadow2", 0, &CqShaderVM::SO_shadow1, 0, {0}},
        {"shadow", 0, &CqShaderVM::SO_shadow, 0, {0}},
        {"illuminate2", 0, &CqShaderVM::SO_illuminate2, 0, {0}},
        {"illuminate", 0, &CqShaderVM::SO_illuminate, 0, {0}},
        {"illuminance2", 0, &CqShaderVM::SO_illuminance2, 0, {0}},
        {"illuminance", 0, &CqShaderVM::SO_illuminance, 0, {0}},
        {"init_illuminance2", 0, &CqShaderVM::SO_init_illuminance2, 0, {0}},
        {"init_illuminance", 0, &CqShaderVM::SO_init_illuminance, 0, {0}},
        {"advance_illuminance", 0, &CqShaderVM::SO_advance_illuminance, 0, {0}},
        {"solar2", 0, &CqShaderVM::SO_solar2, 0, {0}},
        {"solar", 0, &CqShaderVM::SO_solar, 0, {0}},
        {"init_gather", 0, &CqShaderVM::SO_init_gather, 0, {0}},
        {"advance_gather", 0, &CqShaderVM::SO_advance_gather, 0, {0}},
        {"gather", 0, &CqShaderVM::SO_gather, 0, {0}},
        {"printf", 0, &CqShaderVM::SO_printf, 0, {0}},

        {"fcellnoise1", 0, &CqShaderVM::SO_fcellnoise1, 0, {0}},
        {"fcellnoise2", 0, &CqShaderVM::SO_fcellnoise2, 0, {0}},
        {"fcellnoise3", 0, &CqShaderVM::SO_fcellnoise3, 0, {0}},
        {"fcellnoise4", 0, &CqShaderVM::SO_fcellnoise4, 0, {0}},
        {"ccellnoise1", 0, &CqShaderVM::SO_ccellnoise1, 0, {0}},
        {"ccellnoise2", 0, &CqShaderVM::SO_ccellnoise2, 0, {0}},
        {"ccellnoise3", 0, &CqShaderVM::SO_ccellnoise3, 0, {0}},
        {"ccellnoise4", 0, &CqShaderVM::SO_ccellnoise4, 0, {0}},
        {"pcellnoise1", 0, &CqShaderVM::SO_pcellnoise1, 0, {0}},
        {"pcellnoise2", 0, &CqShaderVM::SO_pcellnoise2, 0, {0}},
        {"pcellnoise3", 0, &CqShaderVM::SO_pcellnoise3, 0, {0}},
        {"pcellnoise4", 0, &CqShaderVM::SO_pcellnoise4, 0, {0}},

        {"fpnoise1", 0, &CqShaderVM::SO_fpnoise1, 0, {0}},
        {"fpnoise2", 0, &CqShaderVM::SO_fpnoise2, 0, {0}},
        {"fpnoise3", 0, &CqShaderVM::SO_fpnoise3, 0, {0}},
        {"fpnoise4", 0, &CqShaderVM::SO_fpnoise4, 0, {0}},
        {"cpnoise1", 0, &CqShaderVM::SO_cpnoise1, 0, {0}},
        {"cpnoise2", 0, &CqShaderVM::SO_cpnoise2, 0, {0}},
        {"cpnoise3", 0, &CqShaderVM::SO_cpnoise3, 0, {0}},
        {"cpnoise4", 0, &CqShaderVM::SO_cpnoise4, 0, {0}},
        {"ppnoise1", 0, &CqShaderVM::SO_ppnoise1, 0, {0}},
        {"ppnoise2", 0, &CqShaderVM::SO_ppnoise2, 0, {0}},
        {"ppnoise3", 0, &CqShaderVM::SO_ppnoise3, 0, {0}},
        {"ppnoise4", 0, &CqShaderVM::SO_ppnoise4, 0, {0}},

        {"atmosphere", 0, &CqShaderVM::SO_atmosphere, 1, {type_invalid}},
        {"displacement", 0, &CqShaderVM::SO_displacement, 1, {type_invalid}},
        {"lightsource", 0, &CqShaderVM::SO_lightsource, 1, {type_invalid}},
        {"surface", 0, &CqShaderVM::SO_surface, 1, {type_invalid}},

        {"attribute", 0, &CqShaderVM::SO_attribute, 1, {type_invalid}},
        {"option", 0, &CqShaderVM::SO_option, 1, {type_invalid}},
        {"rendererinfo", 0, &CqShaderVM::SO_rendererinfo, 1, {type_invalid}},
        {"incident", 0, &CqShaderVM::SO_incident, 1, {type_invalid}},
        {"opposite", 0, &CqShaderVM::SO_opposite, 1, {type_invalid}},

        {"ctransform", 0, &CqShaderVM::SO_ctransform, 0, {0}},
        {"ctransform2", 0, &CqShaderVM::SO_ctransform2, 0, {0}},

        {"ptlined", 0, &CqShaderVM::SO_ptlined, 0, {0}},
        {"inversesqrt", 0, &CqShaderVM::SO_inversesqrt, 0, {0}},
        {"concat", 0, &CqShaderVM::SO_concat, 0, {0}},
        {"format", 0, &CqShaderVM::SO_format, 0, {0}},
        {"match", 0, &CqShaderVM::SO_match, 0, {0}},
        {"rotate", 0, &CqShaderVM::SO_rotate, 0, {0}},
        {"filterstep", 0, &CqShaderVM::SO_filterstep, 0, {0}},
        {"filterstep2", 0, &CqShaderVM::SO_filterstep2, 0, {0}},
        {"specularbrdf", 0, &CqShaderVM::SO_specularbrdf, 0, {0}},

        {"mtransform2", 0, &CqShaderVM::SO_mtransform2, 0, {0}},
        {"mtransform", 0, &CqShaderVM::SO_mtransform, 0, {0}},
        {"mcomp", 0, &CqShaderVM::SO_mcomp, 0, {0}},
        {"setmcomp", 0, &CqShaderVM::SO_setmcomp, 0, {0}},
        {"determinant", 0, &CqShaderVM::SO_determinant, 0, {0}},
        {"mtranslate", 0, &CqShaderVM::SO_mtranslate, 0, {0}},
        {"mrotate", 0, &CqShaderVM::SO_mrotate, 0, {0}},
        {"mscale", 0, &CqShaderVM::SO_mscale, 0, {0}},

        {"fsplinea", 0, &CqShaderVM::SO_fsplinea, 0, {0}},
        {"csplinea", 0, &CqShaderVM::SO_csplinea, 0, {0}},
        {"psplinea", 0, &CqShaderVM::SO_psplinea, 0, {0}},
        {"vsplinea", 0, &CqShaderVM::SO_psplinea, 0, {0}},
        {"sfsplinea", 0, &CqShaderVM::SO_sfsplinea, 0, {0}},
        {"scsplinea", 0, &CqShaderVM::SO_scsplinea, 0, {0}},
        {"spsplinea", 0, &CqShaderVM::SO_spsplinea, 0, {0}},
        {"svsplinea", 0, &CqShaderVM::SO_spsplinea, 0, {0}},

        {"shadername2", 0, &CqShaderVM::SO_shadername2, 0, {0}},
        {"shadername", 0, &CqShaderVM::SO_shadername, 0, {0}},
        {"bake_f", 0, &CqShaderVM::SO_bake_f, 0, {0}},
        {"bake_3c", 0, &CqShaderVM::SO_bake_3c, 0, {0}},
        {"bake_3n", 0, &CqShaderVM::SO_bake_3n, 0, {0}},
        {"bake_3v", 0, &CqShaderVM::SO_bake_3v, 0, {0}},
        {"bake_3p", 0, &CqShaderVM::SO_bake_3p, 0, {0}},
      
        {"external", 0, &CqShaderVM::SO_external, 1, {type_invalid}},

        {"occlusion", 0, &CqShaderVM::SO_occlusion, 0, {0}},
        {"occlusion_rt", 0, &CqShaderVM::SO_occlusion_rt, 0, {0}},

        {"rayinfo", 0, &CqShaderVM::SO_rayinfo, 1, {type_invalid}},

        {"bake3d", 0, &CqShaderVM::SO_bake3d, 0, {0}},
        {"texture3d", 0, &CqShaderVM::SO_texture3d, 0, {0}},
    };

/*
 * Its size
 */
TqInt CqShaderVM::m_cTransSize = sizeof( m_TransTable ) / sizeof( m_TransTable[ 0 ] );

/*
 * Private hash keys for "Data", "Init", "Code", "segment", "param", 
 *          "varying", "uniform", "USES"
 */
static const TqUlong dhash = CqString::hash("Data");
;
static const TqUlong ihash = CqString::hash("Init");
static const TqUlong chash = CqString::hash("Code");
static const TqUlong shash = CqString::hash("segment");
static const TqUlong phash = CqString::hash("param");
static const TqUlong vhash = CqString::hash("varying");
static const TqUlong uhash = CqString::hash("uniform");
static const TqUlong ushash = CqString::hash("USES");
static const TqUlong ehash = CqString::hash("external");
static const TqUlong ohash = CqString::hash("output");


CqShaderVM::CqShaderVM(IqRenderer* pRenderContext)
	: CqShaderStack(),
	m_Uses(0xFFFFFFFF),
	m_strName(),
	m_Type(Type_Surface),
	m_LocalIndex(0),
	m_pEnv(0),
	m_pTransform(),
	m_LocalVars(),
	m_StoredArguments(),
	m_ProgramInit(),
	m_Program(),
	m_ProgramStrings(),
	m_uGridRes(0),
	m_vGridRes(0),
	m_shadingPointCount(0),
	m_PC(0),
	m_PO(0),
	m_PE(0),
	m_fAmbient(true),
	m_outsideWorld(false),
	m_pRenderContext(pRenderContext)
{
	// Find out if this shader is being declared outside the world construct. If so
	// if is effectively being defined in 'camera' space, which will affect the
	// transformation of parameters. Should only affect lightsource shaders as these
	// are the only ones valid outside the world.
	if (NULL != m_pRenderContext)
		m_outsideWorld = !m_pRenderContext->IsWorldBegin();
	else
		m_outsideWorld = false;
}

CqShaderVM::CqShaderVM(const CqShaderVM& From)
	: CqShaderStack(),
	m_Uses(0),
	m_strName(),
	m_Type(Type_Surface),
	m_LocalIndex(0),
	m_pEnv(0),
	m_pTransform(),
	m_LocalVars(),
	m_StoredArguments(),
	m_ProgramInit(),
	m_Program(),
	m_ProgramStrings(),
	m_uGridRes(0),
	m_vGridRes(0),
	m_shadingPointCount(0),
	m_PC(0),
	m_PO(0),
	m_PE(0),
	m_fAmbient(true),
	m_outsideWorld(false),
	m_pRenderContext(0)
{
	*this = From;
	// Find out if this shader is being declared outside the world construct. If so
	// if is effectively being defined in 'camera' space, which will affect the
	// transformation of parameters. Should only affect lightsource shaders as these
	// are the only ones valid outside the world.
	if (NULL != m_pRenderContext)
		m_outsideWorld = !m_pRenderContext->IsWorldBegin();
	else
		m_outsideWorld = false;
}

CqShaderVM::~CqShaderVM()
{
	// Delete the local variables.
	for ( std::vector<IqShaderData*>::iterator i = m_LocalVars.begin(); i != m_LocalVars.end(); i++ )
	{
		delete *i;
	}
	// Delete strings used by the program
	for ( std::list<CqString*>::iterator i = m_ProgramStrings.begin();
			i != m_ProgramStrings.end(); i++ )
	{
		delete *i;
	}
	// Delete stored shader arguments
	for(std::vector<SqArgumentRecord>::iterator i = m_StoredArguments.begin();
			i != m_StoredArguments.end(); ++i)
	{
		delete i->m_Value;
	}
}

//---------------------------------------------------------------------
/**
 *  Function to create a local variable for a specific shader
 */

IqShaderData* CqShaderVM::CreateVariable( EqVariableType Type, EqVariableClass Class, const CqString& name, bool fParameter, bool fOutput )
{
	// Create a VM specific shader variable, which implements the IqShaderData interface,
	// based on the type and class specified.
	switch ( Type )
	{
			case type_bool:    /* abviously they are missing here */
			case type_integer:
			case type_float:
			{
				switch ( Class )
				{
						case class_varying:
						return ( new CqShaderVariableVaryingFloat( name.c_str(), fParameter ) );
						case class_uniform:
						return ( new CqShaderVariableUniformFloat( name.c_str(), fParameter ) );
						default: // Clear up compiler warnings
						break;
				}
				assert( false );	// If we get here, something is wrong with the request.
				return ( NULL );
			}

			case type_point:
			{
				switch ( Class )
				{
						case class_varying:
						return ( new CqShaderVariableVaryingPoint( name.c_str(), fParameter ) );
						case class_uniform:
						return ( new CqShaderVariableUniformPoint( name.c_str(), fParameter ) );
						default: // Clear up compiler warnings
						break;
				}
				assert( false );	// If we get here, something is wrong with the request.
				return ( NULL );
			}

			case type_normal:
			{
				switch ( Class )
				{
						case class_varying:
						return ( new CqShaderVariableVaryingNormal( name.c_str(), fParameter ) );
						case class_uniform:
						return ( new CqShaderVariableUniformNormal( name.c_str(), fParameter ) );
						default: // Clear up compiler warnings
						break;
				}
				assert( false );	// If we get here, something is wrong with the request.
				return ( NULL );
			}

			case type_vector:
			{
				switch ( Class )
				{
						case class_varying:
						return ( new CqShaderVariableVaryingVector( name.c_str(), fParameter ) );
						case class_uniform:
						return ( new CqShaderVariableUniformVector( name.c_str(), fParameter ) );
						default: // Clear up compiler warnings
						break;
				}
				assert( false );	// If we get here, something is wrong with the request.
				return ( NULL );
			}

			case type_string:
			{
				switch ( Class )
				{
						case class_varying:
						return ( new CqShaderVariableVaryingString( name.c_str(), fParameter ) );
						case class_uniform:
						return ( new CqShaderVariableUniformString( name.c_str(), fParameter ) );
						default: // Clear up compiler warnings
						break;
				}
				assert( false );	// If we get here, something is wrong with the request.
				return ( NULL );
			}

			case type_color:
			{
				switch ( Class )
				{
						case class_varying:
						return ( new CqShaderVariableVaryingColor( name.c_str(), fParameter ) );
						case class_uniform:
						return ( new CqShaderVariableUniformColor( name.c_str(), fParameter ) );
						default: // Clear up compiler warnings
						break;
				}
				assert( false );	// If we get here, something is wrong with the request.
				return ( NULL );
			}

			case type_triple:
			case type_hpoint:
			case type_void:
			assert( false );	// We don't support triples in the engine as variables.
			return ( NULL );

			case type_matrix:
			{
				switch ( Class )
				{
						case class_varying:
						return ( new CqShaderVariableVaryingMatrix( name.c_str(), fParameter ) );
						case class_uniform:
						return ( new CqShaderVariableUniformMatrix( name.c_str(), fParameter ) );
						default: // Clear up compiler warnings
						break;
				}
				assert( false );	// If we get here, something is wrong with the request.
				return ( NULL );
			}
			default: // Clear up compiler warnings
			break;
	}
	assert( false );	// If we get here, something is wrong with the request.
	return ( NULL );
}

//---------------------------------------------------------------------
/**
 *  Function to create a local variable array for a specific shader
 */

IqShaderData* CqShaderVM::CreateVariableArray( EqVariableType VarType, EqVariableClass VarClass, const CqString& name, TqInt Count, bool fParameter, bool fOutput )
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
			default: // Clear up the warnings
			break;
	}
	CqShaderVariableArray* pArray = new CqShaderVariableArray( name.c_str(), Count, fParameter );
	pArray->aVariables() [ 0 ] = pVar;
	TqInt i;
	for ( i = 1; i < Count; i++ )
		pArray->aVariables() [ i ] = pVar->Clone();

	return ( pArray );
}

//---------------------------------------------------------------------
/**
 *  Function to create a very temporary variable array for a specific shader
 */

IqShaderData* CqShaderVM::CreateTemporaryStorage( EqVariableType type, EqVariableClass _class )
{
	CqString strName( "__temporary__" );
	return ( CreateVariable( type, _class, strName ) );
}


//---------------------------------------------------------------------
/**
 *  Function to Delete the very temporary variable array for a specific shader
 */

void CqShaderVM::DeleteTemporaryStorage( IqShaderData* pData )
{
	delete( pData );
}

//---------------------------------------------------------------------
/**
 *  Routine to emulate the default Surface shader. 
 *  TODO Still missing: Displacement, imager, volume, interior, exterior and 
 *          light default shaders
 */

void CqShaderVM::DefaultSurface()
{
	char	pDefSurfaceShader[] = " \
	                           surface \
	                           segment Data \
	                           USES 460803 \
	                           param uniform  float Kd \
	                           param uniform  float Ka \
	                           varying  float d \
	                           segment Init \
	                           pushif 0.8 \
	                           pop Kd \
	                           pushif 0.2 \
	                           pop Ka \
	                           segment Code \
	                           pushv N \
	                           normalize \
	                           pushv I \
	                           normalize \
	                           dotpp \
	                           pop d \
	                           pushv d \
	                           pushv d \
	                           pushv Kd \
	                           mulff \
	                           mulff \
	                           pushv Ka \
	                           addff \
	                           setfc \
	                           pushv Cs \
	                           mulcc \
	                           pop Ci \
	                           pushv Os \
	                           pop Oi \
	                           pushv Oi \
	                           pushv Ci \
	                           mulcc \
	                           pop Ci \
	                           ";

	std::stringstream defStream(pDefSurfaceShader);

	LoadProgram(&defStream);
}


//---------------------------------------------------------------------
/**
* Function to determine if not the character is ' '
*/

static bool notspace(char C)
{
	bool retval = true;

	if ((C == 0x20) ||
	        (( C >= 0x09 ) && (C <= 0x0D)))
		retval = false;

	return retval;
}

//---------------------------------------------------------------------
/** Set the shader type important for Imager' shader
*/
void CqShaderVM::SetType(EqShaderType type)
{
	m_Type = type;
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
	while ( notspace( c ) && i < l - 1 )
	{
		token[ i++ ] = c;
		token[ i ] = '\0';
		c = pFile->get();
	}
}


//---------------------------------------------------------------------
/** Load a program from a compiled slx file.
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
	boost::shared_ptr<CqShaderExecEnv> StdEnv(new CqShaderExecEnv(m_pRenderContext));
	TqInt	array_count = 0;
	TqUlong  htoken, i;

	bool fShaderSpec = false;
	while ( !pFile->eof() )
	{
		GetToken( token, 255, pFile );

		htoken = CqString::hash(token);

		// Check for type and version information.
		if ( !fShaderSpec )
		{
			TqInt i;
			static TqInt tmp = 0;
			i = tmp;
			for ( ; i < gcShaderTypeNames; i++ )
			{
				if (!gShaderTypeNames[i].hash)
				{
					gShaderTypeNames[i].hash = CqString::hash(gShaderTypeNames[i].name);
				}
				if ( gShaderTypeNames[i].hash == htoken )
				{
					m_Type = gShaderTypeNames[i].type;
					fShaderSpec = true;
					tmp = i;
					break;
				}
			}
			if (fShaderSpec == false)
			{
				for (i=0 ; i < tmp; i++ )
				{
					if (!gShaderTypeNames[i].hash)
					{
						gShaderTypeNames[i].hash = CqString::hash(gShaderTypeNames[i].name);
					}
					if ( gShaderTypeNames[i].hash == htoken )
					{
						m_Type = gShaderTypeNames[i].type;
						fShaderSpec = true;
						tmp = i;
						break;
					}
				}
			}
			if ( fShaderSpec ) continue;
		}

		if ( strcmp( token, "AQSIS_V" ) == 0 )
		{
			GetToken( token, 255, pFile );
			// Check that the version string matches the current one.  If not,
			// fail fatally.
			if(std::string(token) != AQSIS_VERSION_STR)
			{
				AQSIS_THROW_XQERROR(XqBadShader, EqE_NoShader,
					"Shader compiled with an old/different version ("
					<< token << ") of aqsis.  Please recompile.");
			}
			continue;
		}

		if ( ushash == htoken) // == "USES"
		{
			( *pFile ) >> m_Uses;
			continue;
		}

		if ( shash == htoken ) // == "segment"
		{
			GetToken( token, 255, pFile );
			htoken = CqString::hash(token);

			if ( dhash == htoken ) // == "Data"
				Segment = Seg_Data;
			else if ( ihash == htoken) // == "Init"
			{
				Segment = Seg_Init;
				pProgramArea = &m_ProgramInit;
				aLabels.clear();
			}
			else if (chash == htoken ) // == "Code"
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
			bool fVarArray = false;
			bool fParameter = false;
			bool fOutput = false;
			switch ( Segment )
			{
				case Seg_Data:
					VarType = type_invalid;
					VarClass = class_invalid;
					while ( VarType == type_invalid )
					{
						if ( ohash == htoken) // == "output"
							fOutput = true;
						else if ( phash == htoken) // == "param"
							fParameter = true;
						else if ( vhash == htoken) // == "varying"
							VarClass = class_varying;
						else if ( uhash == htoken) // == "uniform"
							VarClass = class_uniform;
						else
							VarType = enumCast<EqVariableType>(token);
						GetToken( token, 255, pFile );
						htoken = CqString::hash(token);
					}
					// Check for array type variable.
					if ( token[ strlen( token ) - 1 ] == ']' )
					{
						unsigned int i = 0;
						while ( i < strlen( token ) && token[ i ] != '[' ) i++;
						if ( i == strlen( token ) )
						{
							AQSIS_THROW_XQERROR(XqBadShader, EqE_NoShader,
								"Invalid variable specification in slx file");
						}
						token[ strlen( token ) - 1 ] = '\0';
						token[ i ] = '\0';
						i++;
						array_count = atoi( &token[ i ] );
						fVarArray = true;
					}
					// Check if there is a valid variable specifier
					if ( VarType == type_invalid ||
					        VarClass == class_invalid )
						continue;

					if ( fVarArray )
						AddLocalVariable( CreateVariableArray( VarType, VarClass, token, array_count, fParameter, fOutput ) );
					else
						AddLocalVariable( CreateVariable( VarType, VarClass, token, fParameter, fOutput ) );
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
						if ( !m_TransTable[ i ].m_hash )
						{
							m_TransTable[ i ].m_hash = CqString::hash(m_TransTable[ i ].m_strName);
						}

						if ( ehash == htoken )
						{
							CqString strFunc, strRetType, strArgTypes ;
							EqVariableType RetType;
							std::list<EqVariableType> ArgTypes;

							*pFile >> strFunc;
							strFunc = strFunc.substr(1,strFunc.length() - 2);
							std::list<SqDSOExternalCall*> *candidates = NULL;
							m_itActiveDSOMap = m_ActiveDSOMap.find( strFunc );
							if( m_itActiveDSOMap != m_ActiveDSOMap.end() )
							{
								candidates = ( *m_itActiveDSOMap ).second;
							}
							else
							{
								candidates = getShadeOpMethods(&strFunc);
								if( candidates == NULL )
								{
									AQSIS_THROW_XQERROR(XqBadShader, EqE_NoShader,
										"\"" << strName().c_str() << "\": No DSO found for "
										"external shadeop: \"" << strFunc.c_str() << "\"\n");
								}
								m_ActiveDSOMap[strFunc]=candidates;
							};

							// pick out the return type
							*pFile >> strRetType;
							m_itTypeIdMap = m_TypeIdMap.find( strRetType[1] );
							if (m_itTypeIdMap != m_TypeIdMap.end())
							{
								RetType = (*m_itTypeIdMap).second;
							}
							else
							{
								//error, we dont know this return type
								AQSIS_THROW_XQERROR(XqBadShader, EqE_NoShader, "\""
									<< strName() << "\": Invalid return type in call to external"
									" shadeop: \"" << strFunc << "\" : \"" << strRetType << "\"");
							}

							*pFile >> strArgTypes;
							for ( TqUint x=1; x < strArgTypes.length()-1; x++ )
							{
								m_itTypeIdMap = m_TypeIdMap.find( strArgTypes[x] )
								                ;
								if ( m_itTypeIdMap != m_TypeIdMap.end() )
								{
									ArgTypes.push_back( ( *m_itTypeIdMap ).second );
								}
								else
								{
									// Error, unknown arg type
									AQSIS_THROW_XQERROR(XqBadShader, EqE_NoShader,
										"\"" << strName() << "\": Invalid argument type in call "
										"to external shadeop: \"" << strFunc << "\" : \""
										<< strArgTypes[x] << "\"");
								}

							}

							//Now we need to find a good candidate.
							std::list<SqDSOExternalCall*>::iterator candidate;
							candidate = candidates->begin();
							while (candidate !=candidates->end())
							{
								// Do we have a match
								if ((*candidate)->return_type == RetType &&
								                        (*candidate)->arg_types == ArgTypes) break;
								candidate++;
							}

							// If we are looking for a void return type but have not
							// found an exact match, we will take the first match with
							// suitable arguments and force the return value to be
							// discarded.
							if(candidate == candidates->end() && RetType == type_void)
							{
								candidate = candidates->begin()
								            ;
								while (candidate !=candidates->end())
								{
									// Do we have a match
									if ( (*candidate)->arg_types == ArgTypes)
									{
										CqString strProto = strPrototype(&strFunc, (*candidate));
										Aqsis::log() << info << "\"" << strName().c_str() << "\": Using non-void DSO shadeop:  \"" << strProto.c_str() << "\"" <<
										"\"" << strName().c_str() << "\": In place of requested void shadeop: \"" << strFunc.c_str() << "\"" <<
										"\"" << strName().c_str() << "\": If this is not the operation you intended you should force the correct shadeop in your shader source." << std::endl;
										break;
									}
									candidate++;
								}
							}

							if(candidate == candidates->end())
							{
								Aqsis::log() << error << "\"" << strName()
									<< "\": No candidate found for call to external shadeop: \""
									<< strFunc << "\"" << strName() << "\": Perhaps you need some casts?"
									<< "\"" << strName() << "\": The following candidates are in you current DSO path:\n";
								candidate = candidates->begin();
								while (candidate !=candidates->end())
								{
									CqString strProto = strPrototype(&strFunc, (*candidate));
									Aqsis::log() << info << "\"" << strName().c_str() << "\": \t" << strProto.c_str() << std::endl;
									candidate++;
								}
								AQSIS_THROW_XQERROR(XqBadShader, EqE_NoShader,
									"External shadeop not found");
							}

							if(!(*candidate)->initialised )
							{
								// We have an initialiser we have not run yet
								if((*candidate)->init)
								{
									// WARNING: future bug on x86_64 if threading is implemented:
									//
									// The first (int) parameter to the initialiser should be a _unique_ thread identifier.
									// Casting to a smaller type (on x86_64, sizeof(int) < sizeof(void*) ) makes the result
									// possibly non-unique per thread.
									(*candidate)->initData =
									    ((*candidate)->init)(static_cast<int>(reinterpret_cast<ptrdiff_t>(this)),NULL);
								}
								(*candidate)->initialised = true;
							}

							AddCommand( &CqShaderVM::SO_external, pProgramArea );
							AddDSOExternalCall( (*candidate),pProgramArea );

							break;
						}

						if ( m_TransTable[ i ].m_hash == htoken)
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
								m_fAmbient = false;

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
									else if ( ( iVar = StdEnv->FindStandardVarIndex( token ) ) >= 0 )
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
											{
												( *pFile ) >> std::ws;
												TqFloat f;
												( *pFile ) >> f;
												AddFloat( f, pProgramArea );
											}
											break;
										case type_integer:
											{
												( *pFile ) >> std::ws;
												TqInt i;
												( *pFile ) >> i;
												AddInteger( i, pProgramArea );
											}
											break;
										case type_string:
											{
												CqString s = GetString(pFile);
												AddString( s.c_str(), pProgramArea );
											}
											break;
										default:
											AQSIS_THROW_XQERROR(XqBadShader, EqE_NoShader,
												"Unknown literal type");
									}
								}
							}
							break;
						}
					}
					if ( i == m_cTransSize )
					{
						// If we have not found the opcode, throw an error.
						AQSIS_THROW_XQERROR(XqBadShader, EqE_NoShader,
							"Invalid opcode found: " << token);
					}
					break;
			}
		}
		( *pFile ) >> std::ws;
	}
	// Now we need to complete any label jump statements.
	i = 0;
	while ( i < m_Program.size() )
	{
		UsProgramElement E = m_Program[ i++ ]
		                     ;
		if ( E.m_Command == &CqShaderVM::SO_jnz ||
		        E.m_Command == &CqShaderVM::SO_jmp ||
		        E.m_Command == &CqShaderVM::SO_jz ||
		        E.m_Command == &CqShaderVM::SO_RS_JZ ||
		        E.m_Command == &CqShaderVM::SO_S_JZ)
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

CqString CqShaderVM::GetString(std::istream* pFile)
{
	( *pFile ) >> std::ws;
	char c;
	CqString s( "" );
	bool escapeChar = false;

	pFile->get();
	while ( (( c = pFile->get() ) != '"') || escapeChar  )
	{
		if(escapeChar)
		{
			//Treatment for escape char
			switch(c)
			{
				case 'a'://Bell (alert)
					s += '\a';
					escapeChar = false;
					break;
				case 'v'://Vertical tab
					s += '\v';
					escapeChar = false;
					break;
				case '\''://Single quotation mark
					s += "'";
					escapeChar = false;
					break;
				case '?'://Literal question mark
					s += '\?';
					escapeChar = false;
					break;
				case 'n'://New line
					s += '\n';
					escapeChar = false;
					break;
				case 'r'://Carriage return
					s += '\r';
					escapeChar = false;
					break;
				case 't'://Horizontal tab
					s += '\t';
					escapeChar = false;
					break;
				case 'b'://Backspace
					s += '\b';
					escapeChar = false;
					break;
				case 'f'://Formfeed
					s += '\f';
					escapeChar = false;
					break;
				case '"'://Double quotation mark
					s += '"';
					escapeChar = false;
					break;
				case '\\'://Backslash
					s += '\\';
					escapeChar = false;
					break;
				case 'x'://Hexadecimal
				case '0'://octal
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					GetNumericEscapeChar(pFile, s,  c);
					escapeChar = false;
					break;
				default :
					escapeChar = false;
					break;
			}
		}
		else
			if (c ==  '\\')
				escapeChar = true;
			else
				s += c;
	}
	 return s;
}

void CqShaderVM::GetNumericEscapeChar(std::istream* pFile, CqString &s, char c)
{
	CqString a("");
	bool isHexadecimal = (c =='x');
	unsigned int maxLength;

	if (isHexadecimal)
	{
		maxLength = 2;
	}
	else
	{
		a += c;
		maxLength = 3;
	}
	c = pFile->get();

	while ((( c >= '0' && c <= '9') || ((( c >= 'a' && c <= 'f') || ( c >= 'A' && c <= 'F')) && isHexadecimal)) && (a.length() <maxLength))
	{
		a += c;
		c = pFile->get();
	}

	int numericalBase;

	if(isHexadecimal)
		numericalBase = 16;
	else
		numericalBase = 8;
	char result = strtoul(a.c_str(), NULL, numericalBase);

	if (result != 0) s += result;
	pFile->unget();
}

//---------------------------------------------------------------------
/**	Ready the shader for execution.
*/

void CqShaderVM::Initialise( const TqInt uGridRes, const TqInt vGridRes, TqInt shadingPointCount, IqShaderExecEnv* pEnv )
{
	m_pEnv = pEnv;
	// Initialise local variables.
	TqInt i;
	for ( i = m_LocalVars.size() - 1; i >= 0;
	        i-- )
		m_LocalVars[ i ] ->Initialise( shadingPointCount );

	m_uGridRes = uGridRes;
	m_vGridRes = vGridRes;
	m_shadingPointCount = shadingPointCount;

	// Reset the program counter.
	m_PC = 0;
}


//---------------------------------------------------------------------
/**	Assignment operator.
*/

CqShaderVM&	CqShaderVM::operator=( const CqShaderVM& From )
{
	m_Uses = From.m_Uses;
	m_pTransform = From.m_pTransform;
	m_strName = From.m_strName;
	m_fAmbient = From.m_fAmbient;
	m_outsideWorld = From.m_outsideWorld;
	m_pRenderContext = From.m_pRenderContext;

	// Copy the local variables...
	std::vector<IqShaderData*>::const_iterator i;
	for ( i = From.m_LocalVars.begin(); i != From.m_LocalVars.end(); i++ )
		m_LocalVars.push_back( ( *i ) ->Clone() );

	// Copy the intialisation program.
	m_ProgramInit.assign(From.m_ProgramInit.begin(), From.m_ProgramInit.end());

	// Copy the main program.
	m_Program.assign(From.m_Program.begin(), From.m_Program.end());

	return ( *this );
}


//---------------------------------------------------------------------
/**	Execute a series of shader language bytecodes.
*/

void CqShaderVM::Execute(IqShaderExecEnv* pEnv)
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

	CqShaderExecEnv Env(m_pRenderContext);
	Env.Initialise( 1, 1, 1, 1, false, IqAttributesPtr(), IqTransformPtr(), this, m_Uses );
	Initialise( 1, 1, 1, &Env );

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
		if( m_LocalVars[ i ] ->Type() == type )
		{
			// Create a storage for it.
			IqShaderData* pStoredArg = m_LocalVars[i]->Clone();
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

						default: // Clear up warnings
						break;
				}

				if ( pArray )
					pStoredArg->ArrayEntry( arrayindex++ ) ->SetValueFromVariable( pVMVal );
				else
					pStoredArg->SetValueFromVariable( pVMVal );

				DeleteTemporaryStorage(pVMVal);
			}
			SqArgumentRecord rec;
			rec.m_Value = pStoredArg;
			rec.m_strSpace = strSpace;
			rec.m_strName = strName;
			m_StoredArguments.push_back(rec);
			Aqsis::log() << debug << "Storing argument on shader @" << this << " : " << strName.c_str() << " : on : " << m_strName.c_str() << std::endl;
		}
		else
		{
			Aqsis::log() << warning << "Type mismatch in shader \"" << m_strName.c_str() << "\"" << std::endl;
		}
	}
	else
	{
		Aqsis::log() << warning << "Unknown parameter \"" << strName.c_str() << "\" in shader \"" << m_strName.c_str() << "\"" << std::endl;
	}
}


void CqShaderVM::PrepareShaderForUse( )
{
	// Only do the second stage setup of the shader parameters if the shader
	// was defined within the world. If defined outside the world, the shader state is constant
	// irrespective of any changes introduced during the render pass (i.e. autoshadows etc.).

	// However Imager shader are defined outside of the world... therefore it is required
	// to call InitialiseParameters().
	if(!m_outsideWorld || m_Type == Type_Imager)
		InitialiseParameters();

 	switch (m_Type)
	{
		case Type_Surface:
			Aqsis::log() << debug << "surface shader " << strName().c_str() << std::endl;
			break;
		case Type_Lightsource:
			Aqsis::log() << debug << "lightsource shader " << strName().c_str() << std::endl;
			break;
		case Type_Volume:
			Aqsis::log() << debug << "volume shader " << strName().c_str() << std::endl;
			break;
		case Type_Displacement:
			Aqsis::log() << debug << "displacement shader " << strName().c_str() << std::endl;
			break;
		case Type_Transformation:
			Aqsis::log() << debug << "transformation shader " << strName().c_str() << std::endl;
			break;
		case Type_Imager:
			Aqsis::log() << debug << "imager shader " << strName().c_str() << std::endl;
			break;
		default:
			Aqsis::log() << error << "unknown shader type " << strName().c_str() << std::endl;
			break;
	}	

}

void CqShaderVM::InitialiseParameters( )
{
	Aqsis::log() << debug << "Preparing shader @" << this << " : " << strName().c_str() << " [" << m_StoredArguments.size() << " args]"  << std::endl;

	// Reinitialise the local variables to their defaults.
	PrepareDefArgs();

	// Transfer the arguments from the store onto the shader proper, ready for use.
	TqUint i;
	for ( i = 0; i < m_StoredArguments.size(); i++ )
	{
		Aqsis::log() << debug << "Setting argument : " << m_StoredArguments[i].m_strName.c_str() << " : on shader : " << strName().c_str() << std::endl;
		TqInt varindex = FindLocalVarIndex( m_StoredArguments[i].m_strName.c_str() );

		int count = 1, arrayindex = 0;
		IqShaderData* pArray = NULL;

		if ( m_StoredArguments[ i ].m_Value->ArrayLength() > 0 )
		{
			pArray = m_LocalVars[ varindex ];
			count = pArray->ArrayLength();
		}

		CqString strSpace = m_StoredArguments[i].m_strSpace;
		CqString _strSpace( "shader" );
		if ( strSpace.compare( "" ) != 0 )
			_strSpace = strSpace;
		CqMatrix matTrans;

		if (getTransform())
			m_pRenderContext ->matSpaceToSpace( _strSpace.c_str(), "current", getTransform(), getTransform(), m_pRenderContext->Time(), matTrans );

		while ( count-- > 0 )
		{
			// Get the specified value out.
			IqShaderData* pVMVal = CreateTemporaryStorage(m_StoredArguments[i].m_Value->Type(), m_StoredArguments[i].m_Value->Class());

			// If it is a color or a point, ensure it is the correct 'space'
			if ( pVMVal->Type() == type_point || pVMVal->Type() == type_hpoint )
			{
				CqVector3D p;
				m_StoredArguments[i].m_Value->GetPoint( p, 0 );
				pVMVal->SetPoint( matTrans * p );
			}
			else if ( pVMVal->Type() == type_normal )
			{
				CqVector3D p;
				m_StoredArguments[i].m_Value->GetNormal( p, 0 );
				pVMVal->SetNormal( matTrans * p );
			}
			else if ( pVMVal->Type() == type_vector )
			{
				CqVector3D p;
				m_StoredArguments[i].m_Value->GetVector( p, 0 );
				pVMVal->SetVector( matTrans * p );
			}
			else if ( pVMVal->Type() == type_matrix )
			{
				CqMatrix m;
				m_StoredArguments[i].m_Value->GetMatrix( m, 0 );
				pVMVal->SetMatrix( matTrans * m );
			}
			else
			{
				if ( pArray )
					pVMVal->ArrayEntry(arrayindex)->SetValueFromVariable( m_StoredArguments[i].m_Value->ArrayEntry(arrayindex));
				else
					pVMVal->SetValueFromVariable(m_StoredArguments[i].m_Value);
			}

			if ( pArray )
			{
				pArray->ArrayEntry( arrayindex ) ->SetValueFromVariable( pVMVal->ArrayEntry( arrayindex ) );
				++arrayindex;
			}
			else
				m_LocalVars[ varindex ] ->SetValueFromVariable( pVMVal );
			DeleteTemporaryStorage(pVMVal);
		}
	}
}


//---------------------------------------------------------------------
/** Set the instance variables on this shader, used for varying variables which will be set from a parameter in the surface.
*/

void CqShaderVM::SetArgument( IqParameter* pParam, IqSurface* pSurface )
{
	// Find the relevant variable.
	TqInt i = FindLocalVarIndex( pParam->strName().c_str() );
	if ( i >= 0 )
	{
		IqShaderData* pVar = m_LocalVars[ i ];
		if(pVar->Type() == pParam->Type())
			pParam->Dice(m_uGridRes,m_vGridRes,pVar,pSurface);
	}
}


//---------------------------------------------------------------------
/** Set the instance variables on this shader, used for varying variables which will be set from a parameter in the surface.
*/

IqShaderData* CqShaderVM::FindArgument( const CqString& name )
{
	// Find the relevant variable.
	TqInt i = FindLocalVarIndex( name.c_str() );
	if ( i >= 0 )
		return( m_LocalVars[ i ] );
	else
		return( NULL );
}


//---------------------------------------------------------------------
/** Get a value from an instance variable on this shader, and fill in the passed variable reference.
*/

bool CqShaderVM::GetVariableValue( const char* name, IqShaderData* res ) const
{
	// Find the relevant variable.
	TqInt i = FindLocalVarIndex( name );
	if ( i >= 0 )
	{
		IqShaderData* src = m_LocalVars[i];
		// Check that the result and source variables are compatible: they
		// - have the same type
		// - array length, and
		// - we're not trying to assign a varying to a uniform.
		if(src->Type() != res->Type() || src->Size() > res->Size()
		   || src->ArrayLength() != res->ArrayLength())
			return false;
		// If everything is ok, set the value.
		res->SetValueFromVariable(src);
		return true;
	}
	return false;
}

//---------------------------------------------------------------------
const std::vector<IqShaderData*>& CqShaderVM::GetArguments() const
{
	return m_LocalVars;
}


//---------------------------------------------------------------------
/**
 *  Shutdown the engine, releasing any static data it may hold on to during it's lifetime..
 */

void CqShaderVM::ShutdownShaderEngine()
{
	// Free any temporary variables in the buckets.
	while( !m_UFPool.empty() )
	{
		delete(m_UFPool.front());
		m_UFPool.pop_front();
	}
	while( !m_VFPool.empty() )
	{
		delete(m_VFPool.front());
		m_VFPool.pop_front();
	}

	while( !m_UPPool.empty() )
	{
		delete(m_UPPool.front());
		m_UPPool.pop_front();
	}
	while( !m_VPPool.empty() )
	{
		delete(m_VPPool.front());
		m_VPPool.pop_front();
	}

	while( !m_USPool.empty() )
	{
		delete(m_USPool.front());
		m_USPool.pop_front();
	}
	while( !m_VSPool.empty() )
	{
		delete(m_VSPool.front());
		m_VSPool.pop_front();
	}

	while( !m_UCPool.empty() )
	{
		delete(m_UCPool.front());
		m_UCPool.pop_front();
	}
	while( !m_VCPool.empty() )
	{
		delete(m_VCPool.front());
		m_VCPool.pop_front();
	}

	while( !m_UNPool.empty() )
	{
		delete(m_UNPool.front());
		m_UNPool.pop_front();
	}
	while( !m_VNPool.empty() )
	{
		delete(m_VNPool.front());
		m_VNPool.pop_front();
	}

	while( !m_UVPool.empty() )
	{
		delete(m_UVPool.front());
		m_UVPool.pop_front();
	}
	while( !m_VVPool.empty() )
	{
		delete(m_VVPool.front());
		m_VVPool.pop_front();
	}

	while( !m_UMPool.empty() )
	{
		delete(m_UMPool.front());
		m_UMPool.pop_front();
	}
	while( !m_VMPool.empty() )
	{
		delete(m_VMPool.front());
		m_VMPool.pop_front();
	}
}


} // namespace Aqsis
//---------------------------------------------------------------------
