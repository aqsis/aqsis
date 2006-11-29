/*
**
** The contents of this file are subject to the Mozilla Public License Version
** 1.1 (the "License"); you may not use this file except in compliance with
** the License. You may obtain a copy of the License at
** http://www.mozilla.org/MPL/
**
** Software distributed under the License is distributed on an "AS IS" basis,
** WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
** for the specific language governing rights and limitations under the
** License.
**
** The Original Code is the Liquid Rendering Toolkit.
**
** The Initial Developer of the Original Code is Colin Doncaster. Portions
** created by Colin Doncaster are Copyright (C) 2002. All Rights Reserved.
**
** Contributor(s): Philippe Leprince.
**
**
** The RenderMan (R) Interface Procedures and Protocol are:
** Copyright 1988, 1989, Pixar
** All Rights Reserved
**
**
** RenderMan (R) is a registered trademark of Pixar
*/

/* ______________________________________________________________________
**
** Liquid Point Light Shader Source
** ______________________________________________________________________
*/



light
liquidpoint(
      uniform float  intensity              = 1;
      uniform color  lightcolor             = 1;
      uniform float decay                   = 0;

      uniform string shadownamepx           = "";
      uniform string shadownamenx           = "";
      uniform string shadownamepy           = "";
      uniform string shadownameny           = "";
      uniform string shadownamepz           = "";
      uniform string shadownamenz           = "";

      uniform float  shadowbias             = 0.01;
      uniform float  shadowblur             = 0.0;
      uniform float  shadowsamples          = 16;
      uniform float  shadowfiltersize       = 1;
      uniform color  shadowcolor            = 0;

      uniform float  lightID                = 0;
      uniform string __category             = "";

      output varying float __shadowF        = 0;
      output varying color __shadowC        = 1.0;
      output varying color __unshadowed_Cl  = 0;
      output float __nondiffuse             = 0;
      output float __nonspecular            = 0;
)
{
  illuminate( point "shader" ( 0, 0, 0 ) ) {

    if ( shadownamepx == "raytrace" ) {
      __shadowC = shadow( shadownamepx, Ps, "samples", shadowsamples, "blur", shadowfiltersize*0.2+shadowblur, "bias", shadowbias, "width", 1 );
    } else {
      vector Lworld = vtransform( "world", L );

      float Lx = xcomp( Lworld );
      float LxAbs = abs( Lx );
      float Ly = ycomp( Lworld );
      float LyAbs = abs( Ly );
      float Lz = zcomp( Lworld );
      float LzAbs = abs( Lz );

      if( ( LxAbs > LyAbs ) && ( LxAbs > LzAbs ) ) {
        if( ( Lx > 0 ) && ( shadownamepx != "" ) ) {
          uniform float shadowsize[2];
          textureinfo( shadownamepx, "resolution", shadowsize );
          __shadowF = shadow( shadownamepx, Ps, "samples", shadowsamples, "blur", shadowfiltersize*1/shadowsize[0]+shadowblur, "bias", shadowbias, "width", 1 );
        } else if( shadownamenx != "") {
          uniform float shadowsize[2];
          textureinfo( shadownamenx, "resolution", shadowsize );
          __shadowF = shadow( shadownamenx, Ps, "samples", shadowsamples, "blur", shadowfiltersize*1/shadowsize[0]+shadowblur, "bias", shadowbias, "width", 1 );
        }
      } else if( (LyAbs > LxAbs) && ( LyAbs > LzAbs ) ) {
        if( ( Ly > 0 ) && ( shadownamepy != "" ) ) {
          uniform float shadowsize[2];
          textureinfo( shadownamepy, "resolution", shadowsize );
          __shadowF = shadow( shadownamepy, Ps, "samples", shadowsamples, "blur", shadowfiltersize*1/shadowsize[0]+shadowblur, "bias", shadowbias, "width", 1 );
        } else if( shadownameny != "" ) {
          uniform float shadowsize[2];
          textureinfo( shadownameny, "resolution", shadowsize );
          __shadowF = shadow( shadownameny, Ps, "samples", shadowsamples, "blur", shadowfiltersize*1/shadowsize[0]+shadowblur, "bias", shadowbias, "width", 1 );
        }
      } else if( ( LzAbs > LyAbs ) && ( LzAbs > LxAbs ) ) {
        if( ( Lz > 0 ) && ( shadownamepz != "" ) ) {
          uniform float shadowsize[2];
          textureinfo( shadownamepz, "resolution", shadowsize );
          __shadowF = shadow( shadownamepz, Ps, "samples", shadowsamples, "blur", shadowfiltersize*1/shadowsize[0]+shadowblur, "bias", shadowbias, "width", 1 );
        } else if( shadownamenz != "") {
          uniform float shadowsize[2];
          textureinfo( shadownamenz, "resolution", shadowsize );
          __shadowF = shadow( shadownamenz, Ps, "samples", shadowsamples, "blur", shadowfiltersize*1/shadowsize[0]+shadowblur, "bias", shadowbias, "width", 1 );
        }
      } else
        __shadowF = 0.0;
    }

#if defined ( AIR ) || defined ( AQSIS )
      __shadowC = color( mix( comp( lightcolor, 0 ), comp(shadowcolor,0), __shadowF ),
                         mix( comp( lightcolor, 1 ), comp(shadowcolor,1), __shadowF ),
                         mix( comp( lightcolor, 2 ), comp(shadowcolor,2), __shadowF )	);
#else
      __shadowC = mix( lightcolor, shadowcolor, __shadowF );
#endif



    Cl = intensity * pow( 1 / length( L ), decay );
    __unshadowed_Cl = Cl;
    Cl *= __shadowC;
  }
}
