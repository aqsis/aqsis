////---------------------------------------------------------------------
////    Class definition file:  FUNCDEF.CPP
////    Associated header file: FUNCDEF.H
////
////    Author:					Paul C. Gregory
////    Creation date:			22/07/99
////---------------------------------------------------------------------

#include	<aqsis/aqsis.h>
#include	"parsenode.h"
#include	"funcdef.h"

namespace Aqsis {

///---------------------------------------------------------------------
/// Global array of standard function definitions

const char* gVariableTypeIdentifiers[] =
    {
        "@",
        "f",
        "i",
        "p",
        "s",
        "c",
        "t",
        "h",
        "n",
        "v",
        "x",
        "m",
        "w",
    };
TqInt gcVariableTypeIdentifiers = sizeof( gVariableTypeIdentifiers ) / sizeof( gVariableTypeIdentifiers[ 0 ] );


CqFuncDef	gStandardFuncs[] = {
                                 CqFuncDef( Type_Float, "operator*", "mulff", "ff" ),
                                 CqFuncDef( Type_Float, "operator/", "divff", "ff" ),
                                 CqFuncDef( Type_Float, "operator+", "addff", "ff" ),
                                 CqFuncDef( Type_Float, "operator-", "subff", "ff" ),
                                 CqFuncDef( Type_Float, "operatorneg", "negf", "f" ),

                                 CqFuncDef( Type_Point, "operator*", "mulpp", "pp" ),
                                 CqFuncDef( Type_Point, "operator/", "divpp", "pp" ),
                                 CqFuncDef( Type_Point, "operator+", "addpp", "pp" ),
                                 CqFuncDef( Type_Point, "operator-", "subpp", "pp" ),
                                 CqFuncDef( Type_Point, "operator^", "crspp", "pp" ),
                                 CqFuncDef( Type_Float, "operator.", "dotpp", "pp" ),
                                 CqFuncDef( Type_Point, "operatorneg", "negp", "p" ),

                                 CqFuncDef( Type_Color, "operator*", "mulcc", "cc" ),
                                 CqFuncDef( Type_Color, "operator/", "divcc", "cc" ),
                                 CqFuncDef( Type_Color, "operator+", "addcc", "cc" ),
                                 CqFuncDef( Type_Color, "operator-", "subcc", "cc" ),
                                 CqFuncDef( Type_Color, "operatorneg", "negc", "c" ),

                                 CqFuncDef( Type_Vector, "operator*", "mulpp", "vv" ),
                                 CqFuncDef( Type_Vector, "operator/", "divpp", "vv" ),
                                 CqFuncDef( Type_Vector, "operator+", "addpp", "vv" ),
                                 CqFuncDef( Type_Vector, "operator-", "subpp", "vv" ),
                                 CqFuncDef( Type_Vector, "operator^", "crspp", "vv" ),
                                 CqFuncDef( Type_Float, "operator.", "dotpp", "vv" ),
                                 CqFuncDef( Type_Vector, "operatorneg", "negp", "v" ),

                                 CqFuncDef( Type_Normal, "operator*", "mulpp", "nn" ),
                                 CqFuncDef( Type_Normal, "operator/", "divpp", "nn" ),
                                 CqFuncDef( Type_Normal, "operator+", "addpp", "nn" ),
                                 CqFuncDef( Type_Normal, "operator-", "subpp", "nn" ),
                                 CqFuncDef( Type_Normal, "operator^", "crspp", "nn" ),
                                 CqFuncDef( Type_Float, "operator.", "dotpp", "nn" ),
                                 CqFuncDef( Type_Normal, "operatorneg", "negp", "n" ),

                                 CqFuncDef( Type_Point, "operator*", "mulfp", "fp" ),
                                 CqFuncDef( Type_Point, "operator/", "divfp", "fp" ),
                                 CqFuncDef( Type_Point, "operator+", "addfp", "fp" ),
                                 CqFuncDef( Type_Point, "operator-", "subfp", "fp" ),

                                 CqFuncDef( Type_Color, "operator*", "mulfc", "fc" ),
                                 CqFuncDef( Type_Color, "operator/", "divfc", "fc" ),
                                 CqFuncDef( Type_Color, "operator+", "addfc", "fc" ),
                                 CqFuncDef( Type_Color, "operator-", "subfc", "fc" ),

                                 CqFuncDef( Type_Vector, "operator*", "mulfp", "fv" ),
                                 CqFuncDef( Type_Vector, "operator/", "divfp", "fv" ),
                                 CqFuncDef( Type_Vector, "operator+", "addfp", "fv" ),
                                 CqFuncDef( Type_Vector, "operator-", "subfp", "fv" ),

                                 CqFuncDef( Type_Normal, "operator*", "mulfp", "fn" ),
                                 CqFuncDef( Type_Normal, "operator/", "divfp", "fn" ),
                                 CqFuncDef( Type_Normal, "operator+", "addfp", "fn" ),
                                 CqFuncDef( Type_Normal, "operator-", "subfp", "fn" ),

                                 CqFuncDef( Type_Matrix, "operator*", "mulmm", "mm" ),
                                 CqFuncDef( Type_Matrix, "operator/", "divmm", "mm" ),

                                 CqFuncDef( Type_Float, "radians", "radians", "f" ),
                                 CqFuncDef( Type_Float, "degrees", "degrees", "f" ),
                                 CqFuncDef( Type_Float, "sin", "sin", "f" ),
                                 CqFuncDef( Type_Float, "asin", "asin", "f" ),
                                 CqFuncDef( Type_Float, "cos", "cos", "f" ),
                                 CqFuncDef( Type_Float, "acos", "acos", "f" ),
                                 CqFuncDef( Type_Float, "tan", "tan", "f" ),
                                 CqFuncDef( Type_Float, "atan", "atan", "f" ),
                                 CqFuncDef( Type_Float, "atan", "atan2", "ff" ),
                                 CqFuncDef( Type_Float, "pow", "pow", "ff" ),
                                 CqFuncDef( Type_Float, "exp", "exp", "f" ),
                                 CqFuncDef( Type_Float, "sqrt", "sqrt", "f" ),
                                 CqFuncDef( Type_Float, "log", "log", "f" ),
                                 CqFuncDef( Type_Float, "log", "log2", "ff" ),
                                 CqFuncDef( Type_Float, "mod", "mod", "ff" ),
                                 CqFuncDef( Type_Float, "abs", "abs", "f" ),
                                 CqFuncDef( Type_Float, "sign", "sign", "f" ),
                                 CqFuncDef( Type_Float, "min", "min", "ff*" ),
                                 CqFuncDef( Type_Float, "max", "max", "ff*" ),
                                 CqFuncDef( Type_Point, "min", "pmin", "pp*" ),
                                 CqFuncDef( Type_Point, "max", "pmax", "pp*" ),
                                 CqFuncDef( Type_Vector, "min", "vmin", "vv*" ),
                                 CqFuncDef( Type_Vector, "max", "vmax", "vv*" ),
                                 CqFuncDef( Type_Normal, "min", "nmin", "nn*" ),
                                 CqFuncDef( Type_Normal, "max", "nmax", "nn*" ),
                                 CqFuncDef( Type_Color, "min", "cmin", "cc*" ),
                                 CqFuncDef( Type_Color, "max", "cmax", "cc*" ),
                                 CqFuncDef( Type_Float, "clamp", "clamp", "fff" ),
                                 CqFuncDef( Type_Point, "clamp", "pclamp", "ppp" ),
                                 CqFuncDef( Type_Vector, "clamp", "vclamp", "vvv" ),
                                 CqFuncDef( Type_Normal, "clamp", "nclamp", "nnn" ),
                                 CqFuncDef( Type_Color, "clamp", "cclamp", "ccc" ),
                                 CqFuncDef( Type_Float, "floor", "floor", "f" ),
                                 CqFuncDef( Type_Float, "ceil", "ceil", "f" ),
                                 CqFuncDef( Type_Float, "round", "round", "f" ),
                                 CqFuncDef( Type_Float, "step", "step", "ff" ),
                                 CqFuncDef( Type_Float, "smoothstep", "smoothstep", "fff" ),
                                 CqFuncDef( Type_Float, "spline", "fspline", "fffff*" ),
                                 CqFuncDef( Type_Color, "spline", "cspline", "fcccc*" ),
                                 CqFuncDef( Type_Point, "spline", "pspline", "fpppp*" ),
                                 CqFuncDef( Type_Point, "spline", "vspline", "fvvvv*" ),
                                 CqFuncDef( Type_Float, "spline", "sfspline", "sfffff*" ),
                                 CqFuncDef( Type_Color, "spline", "scspline", "sfcccc*" ),
                                 CqFuncDef( Type_Point, "spline", "spspline", "sfpppp*" ),
                                 CqFuncDef( Type_Point, "spline", "svspline", "sfvvvv*" ),
                                 CqFuncDef( Type_Float, "Du", "fDu", "f", ( 1 << EnvVars_du ) ),
                                 CqFuncDef( Type_Float, "Dv", "fDv", "f", ( 1 << EnvVars_dv ) ),
                                 CqFuncDef( Type_Float, "Deriv", "fDeriv", "ff"),
                                 CqFuncDef( Type_Color, "Du", "cDu", "c", ( 1 << EnvVars_du ) ),
                                 CqFuncDef( Type_Color, "Dv", "cDv", "c", ( 1 << EnvVars_dv ) ),
                                 CqFuncDef( Type_Color, "Deriv", "cDeriv", "cf"),
                                 CqFuncDef( Type_Point, "Du", "pDu", "p", ( 1 << EnvVars_du ) ),
                                 CqFuncDef( Type_Point, "Dv", "pDv", "p", ( 1 << EnvVars_dv ) ),
                                 CqFuncDef( Type_Point, "Deriv", "pDeriv", "pf"),
                                 CqFuncDef( Type_hPoint, "Du", "hDu", "h", ( 1 << EnvVars_du ) ),
                                 CqFuncDef( Type_hPoint, "Dv", "hDv", "h", ( 1 << EnvVars_dv ) ),
                                 CqFuncDef( Type_hPoint, "Deriv", "hDeriv", "hf"),
                                 CqFuncDef( Type_Normal, "Du", "nDu", "n", ( 1 << EnvVars_du ) ),
                                 CqFuncDef( Type_Normal, "Dv", "nDv", "n", ( 1 << EnvVars_dv ) ),
                                 CqFuncDef( Type_Normal, "Deriv", "nDeriv", "nf"),
                                 CqFuncDef( Type_Vector, "Du", "vDu", "v", ( 1 << EnvVars_du ) ),
                                 CqFuncDef( Type_Vector, "Dv", "vDv", "v", ( 1 << EnvVars_dv ) ),
                                 CqFuncDef( Type_Vector, "Deriv", "vDeriv", "vf"),
                                 CqFuncDef( Type_Float, "random", "frandom", "" ),
                                 CqFuncDef( Type_Color, "random", "crandom", "" ),
                                 CqFuncDef( Type_Point, "random", "prandom", "" ),
                                 CqFuncDef( Type_Float, "noise", "noise1", "f" ),
                                 CqFuncDef( Type_Float, "noise", "noise2", "ff" ),
                                 CqFuncDef( Type_Float, "noise", "noise3", "p" ),
                                 CqFuncDef( Type_Float, "noise", "noise4", "pf" ),
                                 CqFuncDef( Type_Color, "noise", "cnoise1", "f" ),
                                 CqFuncDef( Type_Color, "noise", "cnoise2", "ff" ),
                                 CqFuncDef( Type_Color, "noise", "cnoise3", "p" ),
                                 CqFuncDef( Type_Color, "noise", "cnoise4", "pf" ),
                                 CqFuncDef( Type_Point, "noise", "pnoise1", "f" ),
                                 CqFuncDef( Type_Point, "noise", "pnoise2", "ff" ),
                                 CqFuncDef( Type_Point, "noise", "pnoise3", "p" ),
                                 CqFuncDef( Type_Point, "noise", "pnoise4", "pf" ),
                                 CqFuncDef( Type_Vector, "noise", "pnoise1", "f" ),
                                 CqFuncDef( Type_Vector, "noise", "pnoise2", "ff" ),
                                 CqFuncDef( Type_Vector, "noise", "pnoise3", "p" ),
                                 CqFuncDef( Type_Vector, "noise", "pnoise4", "pf" ),
                                 CqFuncDef( Type_Float, "xcomp", "xcomp", "p" ),
                                 CqFuncDef( Type_Float, "ycomp", "ycomp", "p" ),
                                 CqFuncDef( Type_Float, "zcomp", "zcomp", "p" ),
                                 CqFuncDef( Type_Float, "xcomp", "xcomp", "h" ),
                                 CqFuncDef( Type_Float, "ycomp", "ycomp", "h" ),
                                 CqFuncDef( Type_Float, "zcomp", "zcomp", "h" ),
                                 CqFuncDef( Type_Float, "xcomp", "xcomp", "n" ),
                                 CqFuncDef( Type_Float, "ycomp", "ycomp", "n" ),
                                 CqFuncDef( Type_Float, "zcomp", "zcomp", "n" ),
                                 CqFuncDef( Type_Float, "xcomp", "xcomp", "v" ),
                                 CqFuncDef( Type_Float, "ycomp", "ycomp", "v" ),
                                 CqFuncDef( Type_Float, "zcomp", "zcomp", "v" ),
                                 CqFuncDef( Type_Float, "length", "length", "p" ),
                                 CqFuncDef( Type_Float, "distance", "distance", "pp" ),
                                 CqFuncDef( Type_Float, "area", "area", "p"),
                                 CqFuncDef( Type_Float, "area", "area", "h"),
                                 CqFuncDef( Type_Point, "normalize", "normalize", "p" ),
                                 CqFuncDef( Type_Point, "faceforward", "faceforward", "pp", (1 << EnvVars_Ng) ),
                                 CqFuncDef( Type_Point, "faceforward", "faceforward2", "ppp" ),
                                 CqFuncDef( Type_Point, "reflect", "reflect", "pp" ),
                                 CqFuncDef( Type_Point, "refract", "refract", "ppf" ),
                                 CqFuncDef( Type_Void, "fresnel", "fresnel", "ppfFF" ),
                                 CqFuncDef( Type_Void, "fresnel", "fresnel2", "ppfFFPP" ),
                                 CqFuncDef( Type_Point, "transform", "transform2", "ssp" ),
                                 CqFuncDef( Type_Point, "transform", "transform", "sp" ),
                                 CqFuncDef( Type_Float, "depth", "depth", "p" ),
                                 CqFuncDef( Type_Point, "calculatenormal", "calculatenormal", "p"),
                                 CqFuncDef( Type_Point, "calculatenormal", "calculatenormal", "h"),
                                 CqFuncDef( Type_Float, "comp", "comp", "cf" ),
                                 CqFuncDef( Type_Void, "setcomp", "setcomp", "cff" ),
                                 CqFuncDef( Type_Color, "mix", "cmix", "ccf" ),
                                 CqFuncDef( Type_Color, "mix", "cmixc", "ccc" ),
                                 CqFuncDef( Type_Color, "ambient", "ambient", "" ),
                                 CqFuncDef( Type_Color, "diffuse", "diffuse", "p" ),
                                 CqFuncDef( Type_Color, "specular", "specular", "ppf" ),
                                 CqFuncDef( Type_Color, "phong", "phong", "ppf" ),
                                 CqFuncDef( Type_Color, "trace", "trace", "pp" ),
                                 CqFuncDef( Type_Float, "shadow", "shadow2", "sfpppp*" ),
                                 CqFuncDef( Type_Float, "shadow", "shadow", "sfp*" ),
                                 CqFuncDef( Type_Float, "texture", "ftexture3", "sfffffffff*" ),
                                 CqFuncDef( Type_Float, "texture", "ftexture2", "sfff*"),
                                 CqFuncDef( Type_Float, "texture", "ftexture1", "sf*", ( 1 << EnvVars_s ) | ( 1 << EnvVars_t ) ),
                                 CqFuncDef( Type_Color, "texture", "ctexture3", "sfffffffff*" ),
                                 CqFuncDef( Type_Color, "texture", "ctexture2", "sfff*" ),
                                 CqFuncDef( Type_Color, "texture", "ctexture1", "sf*", ( 1 << EnvVars_s ) | ( 1 << EnvVars_t ) ),
                                 CqFuncDef( Type_Float, "environment", "fenvironment3", "sfpppp*" ),
                                 CqFuncDef( Type_Float, "environment", "fenvironment2", "sfp*", ( 1 << EnvVars_dv ) | ( 1 << EnvVars_v ) | ( 1 << EnvVars_du ) | ( 1 << EnvVars_u ) ),
                                 CqFuncDef( Type_Color, "environment", "cenvironment3", "sfpppp*" ),
                                 CqFuncDef( Type_Color, "environment", "cenvironment2", "sfp*", ( 1 << EnvVars_dv ) | ( 1 << EnvVars_v ) | ( 1 << EnvVars_du ) | ( 1 << EnvVars_u ) ),
                                 CqFuncDef( Type_Point, "bump", "bump3", "sfpppffffffff*" ),
                                 CqFuncDef( Type_Point, "bump", "bump2", "sfpppff*" ),
                                 CqFuncDef( Type_Point, "bump", "bump1", "sfppp*" ),
                                 CqFuncDef( Type_Void, "printf", "printf", "s*" ),

                                 CqFuncDef( Type_Point, "transform", "transformm", "mp" ),
                                 CqFuncDef( Type_Vector, "vtransform", "vtransform2", "ssv" ),
                                 CqFuncDef( Type_Vector, "vtransform", "vtransform", "sv" ),
                                 CqFuncDef( Type_Vector, "vtransform", "vtransformm", "mv" ),
                                 CqFuncDef( Type_Normal, "ntransform", "ntransform2", "ssn" ),
                                 CqFuncDef( Type_Normal, "ntransform", "ntransform", "sn" ),
                                 CqFuncDef( Type_Normal, "ntransform", "ntransformm", "mn" ),
                                 CqFuncDef( Type_Color, "ctransform", "ctransform2", "ssc" ),
                                 CqFuncDef( Type_Color, "ctransform", "ctransform", "sc" ),
                                 CqFuncDef( Type_Float, "cellnoise", "fcellnoise1", "f" ),
                                 CqFuncDef( Type_Float, "cellnoise", "fcellnoise2", "ff" ),
                                 CqFuncDef( Type_Float, "cellnoise", "fcellnoise3", "p" ),
                                 CqFuncDef( Type_Float, "cellnoise", "fcellnoise4", "pf" ),
                                 CqFuncDef( Type_Color, "cellnoise", "ccellnoise1", "f" ),
                                 CqFuncDef( Type_Color, "cellnoise", "ccellnoise2", "ff" ),
                                 CqFuncDef( Type_Color, "cellnoise", "ccellnoise3", "p" ),
                                 CqFuncDef( Type_Color, "cellnoise", "ccellnoise4", "pf" ),
                                 CqFuncDef( Type_Point, "cellnoise", "pcellnoise1", "f" ),
                                 CqFuncDef( Type_Point, "cellnoise", "pcellnoise2", "ff" ),
                                 CqFuncDef( Type_Point, "cellnoise", "pcellnoise3", "p" ),
                                 CqFuncDef( Type_Point, "cellnoise", "pcellnoise4", "pf" ),
                                 CqFuncDef( Type_Vector, "cellnoise", "pcellnoise1", "f" ),
                                 CqFuncDef( Type_Vector, "cellnoise", "pcellnoise2", "ff" ),
                                 CqFuncDef( Type_Vector, "cellnoise", "pcellnoise3", "p" ),
                                 CqFuncDef( Type_Vector, "cellnoise", "pcellnoise4", "pf" ),
                                 CqFuncDef( Type_Float, "mix", "fmix", "fff" ),
                                 CqFuncDef( Type_Point, "mix", "pmix", "ppf" ),
                                 CqFuncDef( Type_Vector, "mix", "vmix", "vvf" ),
                                 CqFuncDef( Type_Normal, "mix", "nmix", "nnf" ),
                                 CqFuncDef( Type_Point, "mix", "pmixc", "ppc" ),
                                 CqFuncDef( Type_Vector, "mix", "vmixc", "vvc" ),
                                 CqFuncDef( Type_Normal, "mix", "nmixc", "nnc" ),

                                 CqFuncDef( Type_Void, "setcomp", "setcomp", "Cff" ),
                                 CqFuncDef( Type_Void, "setxcomp", "setxcomp", "Pf" ),
                                 CqFuncDef( Type_Void, "setycomp", "setycomp", "Pf" ),
                                 CqFuncDef( Type_Void, "setzcomp", "setzcomp", "Pf" ),
                                 CqFuncDef( Type_Void, "setxcomp", "setxcomp", "Nf" ),
                                 CqFuncDef( Type_Void, "setycomp", "setycomp", "Nf" ),
                                 CqFuncDef( Type_Void, "setzcomp", "setzcomp", "Nf" ),
                                 CqFuncDef( Type_Void, "setxcomp", "setxcomp", "Vf" ),
                                 CqFuncDef( Type_Void, "setycomp", "setycomp", "Vf" ),
                                 CqFuncDef( Type_Void, "setzcomp", "setzcomp", "Vf" ),

                                 CqFuncDef( Type_Float, "ptlined", "ptlined", "ppp" ),
                                 CqFuncDef( Type_Float, "inversesqrt", "inversesqrt", "f" ),
                                 CqFuncDef( Type_String, "concat", "concat", "ss*" ),
                                 CqFuncDef( Type_String, "format", "format", "s*" ),
                                 CqFuncDef( Type_Float, "match", "match", "ss" ),
                                 CqFuncDef( Type_Point, "rotate", "rotate", "pfpp" ),
                                 CqFuncDef( Type_Float, "filterstep", "filterstep2", "fff*"),
                                 CqFuncDef( Type_Float, "filterstep", "filterstep", "ff*"),
                                 CqFuncDef( Type_Color, "specularbrdf", "specularbrdf", "vnvf" ),

                                 CqFuncDef( Type_Float, "pnoise", "fpnoise1", "ff" ),
                                 CqFuncDef( Type_Float, "pnoise", "fpnoise2", "ffff" ),
                                 CqFuncDef( Type_Float, "pnoise", "fpnoise3", "pp" ),
                                 CqFuncDef( Type_Float, "pnoise", "fpnoise4", "pfpf" ),
                                 CqFuncDef( Type_Color, "pnoise", "cpnoise1", "ff" ),
                                 CqFuncDef( Type_Color, "pnoise", "cpnoise2", "ffff" ),
                                 CqFuncDef( Type_Color, "pnoise", "cpnoise3", "pp" ),
                                 CqFuncDef( Type_Color, "pnoise", "cpnoise4", "pfpf" ),
                                 CqFuncDef( Type_Point, "pnoise", "ppnoise1", "ff" ),
                                 CqFuncDef( Type_Point, "pnoise", "ppnoise2", "ffff" ),
                                 CqFuncDef( Type_Point, "pnoise", "ppnoise3", "pp" ),
                                 CqFuncDef( Type_Point, "pnoise", "ppnoise4", "pfpf" ),
                                 CqFuncDef( Type_Vector, "pnoise", "ppnoise1", "ff" ),
                                 CqFuncDef( Type_Vector, "pnoise", "ppnoise2", "ffff" ),
                                 CqFuncDef( Type_Vector, "pnoise", "ppnoise3", "pp" ),
                                 CqFuncDef( Type_Vector, "pnoise", "ppnoise4", "pfpf" ),

                                 CqFuncDef( Type_Matrix, "mtransform", "mtransform2", "ssm" ),
                                 CqFuncDef( Type_Matrix, "mtransform", "mtransform", "sm" ),
                                 CqFuncDef( Type_Float, "comp", "mcomp", "mff" ),
                                 CqFuncDef( Type_Void, "setcomp", "setmcomp", "mfff" ),
                                 CqFuncDef( Type_Float, "determinant", "determinant", "m" ),
                                 CqFuncDef( Type_Matrix, "translate", "mtranslate", "mv" ),
                                 CqFuncDef( Type_Matrix, "rotate", "mrotate", "mfv" ),
                                 CqFuncDef( Type_Matrix, "scale", "mscale", "mp" ),

                                 CqFuncDef( Type_Float, "spline", "fsplinea", "f[f]" ),
                                 CqFuncDef( Type_Color, "spline", "csplinea", "f[c]" ),
                                 CqFuncDef( Type_Point, "spline", "psplinea", "f[p]" ),
                                 CqFuncDef( Type_Point, "spline", "vsplinea", "f[v]" ),
                                 CqFuncDef( Type_Float, "spline", "sfsplinea", "sf[f]" ),
                                 CqFuncDef( Type_Color, "spline", "scsplinea", "sf[c]" ),
                                 CqFuncDef( Type_Point, "spline", "spsplinea", "sf[p]" ),
                                 CqFuncDef( Type_Point, "spline", "svsplinea", "sf[v]" ),

                                 CqFuncDef( Type_String, "shadername", "shadername2", "s" ),
                                 CqFuncDef( Type_String, "shadername", "shadername", "" ),
                                 CqFuncDef( Type_Void, "bake", "bake_f", "sfff" ),
                                 CqFuncDef( Type_Void, "bake", "bake_3c", "sffc" ),
                                 CqFuncDef( Type_Void, "bake", "bake_3p", "sffp" ),
                                 CqFuncDef( Type_Void, "bake", "bake_3v", "sffv" ),
                                 CqFuncDef( Type_Void, "bake", "bake_3n", "sffn" ),

                                 CqFuncDef( Type_Float, "occlusion", "occlusion", "sfpnf*" ),
                                 CqFuncDef( Type_Float, "occlusion", "occlusion_rt", "pnf*" ),
                                 CqFuncDef( Type_Float, "bake3d", "bake3d", "sspn*" ),
                                 CqFuncDef( Type_Float, "texture3d", "texture3d", "spn*" ),
                             };

TqUint	gcStandardFuncs = sizeof( gStandardFuncs ) / sizeof( gStandardFuncs[ 0 ] );



std::vector<CqFuncDef>	gLocalFuncs;


///---------------------------------------------------------------------
/** Constructor.
 */

CqFuncDef::CqFuncDef( TqInt Type, const char* strName, const char* strVMName, const char* strParams, CqParseNode* pDef, CqParseNode* pArgs ) :
		m_Type( Type ),
		m_strName( strName ),
		m_strVMName( strVMName ),
		m_strParamTypes( strParams ),
		m_fLocal( true ),
		m_pDef( pDef ),
		m_pArgs( pArgs ),
		m_fVarying( false )
{
	// Build the type array.
	TypeArray();

	if ( m_pDef )
		m_pDef->Optimise();
}


///---------------------------------------------------------------------
/** Find a function definition by searchin the standard and user definitions lists.
 */

bool CqFuncDef::FindFunction( const char* strName, std::vector<SqFuncRef>& Refs )
{
	SqFuncRef ref;
	// Clear any existing indexes.
	Refs.clear();

	// Search the standard definitions first.
	TqUint i;
	for ( i = 0; i < gcStandardFuncs; i++ )
	{
		if ( gStandardFuncs[ i ].m_strName == strName )
		{
			ref.m_Type = FuncTypeStandard;
			ref.m_Index = i;
			Refs.push_back( ref );
		}
	}

	// Search the local definitions next.
	for ( i = 0; i < gLocalFuncs.size(); i++ )
	{
		if ( gLocalFuncs[ i ].m_strName == strName )
		{
			ref.m_Type = FuncTypeLocal;
			ref.m_Index = i;
			Refs.push_back( ref );
		}
	}
	if ( !Refs.empty() )
		return ( true );
	else
		return ( false );
}


///---------------------------------------------------------------------
/** Add a new local function.
 */

TqInt CqFuncDef::AddFunction( CqFuncDef& Def )
{
	gLocalFuncs.push_back( Def );
	return ( gLocalFuncs.size() - 1 );
}


///---------------------------------------------------------------------
/** Fill in an array of types from a typespec string.
 */

int CqFuncDef::TypeArray()
{
	// Go through the type string parsing the types.
	unsigned int j = 0, cvars = 0;

	while ( j < m_strParamTypes.size() )
	{
		TqInt type = Type_Nil;
		int ctype = m_strParamTypes[ j++ ];
		switch ( tolower( ctype ) )
		{
				case '@':
				break;
				case 'f':
				type = ( ( type & ( ~Type_Mask ) ) | Type_Float );
				break;
				case 'i':
				type = ( ( type & ( ~Type_Mask ) ) | Type_Integer );
				break;
				case 'p':
				type = ( ( type & ( ~Type_Mask ) ) | Type_Point );
				break;
				case 's':
				type = ( ( type & ( ~Type_Mask ) ) | Type_String );
				break;
				case 'c':
				type = ( ( type & ( ~Type_Mask ) ) | Type_Color );
				break;
				case 't':
				type = ( ( type & ( ~Type_Mask ) ) | Type_Triple );
				break;
				case 'h':
				type = ( ( type & ( ~Type_Mask ) ) | Type_hPoint );
				break;
				case 'n':
				type = ( ( type & ( ~Type_Mask ) ) | Type_Normal );
				break;
				case 'v':
				type = ( ( type & ( ~Type_Mask ) ) | Type_Vector );
				break;
				case 'x':
				type = ( ( type & ( ~Type_Mask ) ) | Type_Void );
				break;
				case 'm':
				type = ( ( type & ( ~Type_Mask ) ) | Type_Matrix );
				break;
				case 'w':
				type = ( ( type & ( ~Type_Mask ) ) | Type_HexTuple );
				break;
				case '[':
				type = ( type | Type_Array );
				break;
				case ']':
				break;
				case '*':
				m_fVarying = true;
				break;
		}
		if ( isupper( ctype ) )
			type = ( type | Type_Variable );

		if ( ( type & Type_Mask ) != Type_Nil )
		{
			m_aTypeSpec.push_back( type );
			cvars++;
		}
	}
	return ( cvars );
}


const IqParseNode* CqFuncDef::pArgs() const
{
	return ( m_pArgs );
}


const IqParseNode* CqFuncDef::pDef() const
{
	return ( m_pDef );
}

IqParseNode* CqFuncDef::pDef()
{
	return ( m_pDef );
}


///---------------------------------------------------------------------
/** Return a temporary pointer to a function definition..
 */

CqFuncDef* CqFuncDef::GetFunctionPtr( const SqFuncRef& Ref )
{
	if ( Ref.m_Type == FuncTypeStandard && Ref.m_Index < gcStandardFuncs )
		return ( &gStandardFuncs[ Ref.m_Index ] );

	if ( Ref.m_Type == FuncTypeLocal && Ref.m_Index < gLocalFuncs.size() )
		return ( &gLocalFuncs[ Ref.m_Index ] );

	return ( 0 );
}


///---------------------------------------------------------------------
/// IqFuncDef::GetFunctionPtr
/// Return a temporary pointer to a function definition..

IqFuncDef* IqFuncDef::GetFunctionPtr( const SqFuncRef& Ref )
{
	return ( CqFuncDef::GetFunctionPtr( Ref ) );
}


} // namespace Aqsis
//---------------------------------------------------------------------
