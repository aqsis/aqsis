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
** Liquid Spot Light Shader Source
** ______________________________________________________________________
*/



light
liquidspot(
      uniform float intensity               = 1;
      uniform color lightcolor              = 1;
      uniform float coneangle               = radians( 40 );
      uniform float penumbraangle           = radians( 0 );
      uniform float dropoff                 = 0;
      uniform float decay                   = 0;

      uniform float barndoors               = 0;
      uniform float leftbarndoor            = 10;
      uniform float rightbarndoor           = 10;
      uniform float topbarndoor             = 10;
      uniform float bottombarndoor          = 10;

      uniform float decayRegions            = 0;
      uniform float startDistance1          = 1.0;
      uniform float endDistance1            = 2.0;
      uniform float startDistance2          = 3.0;
      uniform float endDistance2            = 6.0;
      uniform float startDistance3          = 8.0;
      uniform float endDistance3            = 10.0;
      uniform float startDistanceIntensity1 = 1.0;
      uniform float endDistanceIntensity1   = 1.0;
      uniform float startDistanceIntensity2 = 1.0;
      uniform float endDistanceIntensity2   = 1.0;
      uniform float startDistanceIntensity3 = 1.0;
      uniform float endDistanceIntensity3   = 1.0;

      uniform string shadowname       = "";
      uniform float  shadowbias       = 0.01;
      uniform float  shadowblur       = 0.0;
      uniform float  shadowsamples    = 32;
      uniform float  shadowfiltersize = 1;
      uniform color  shadowcolor      = 0;
      varying color  shadowcolorSurf  = 0;
      uniform float  shadowcolorMix  = -1;

      uniform float  lightID          = 0;
      uniform string __category       = "";

      output varying color __shadowC        = 0;
      output varying float __shadowF        = 0;
      output varying float __barn           = 0;
      output varying color __unshadowed_Cl  = 0;
      output float         __nondiffuse     = 0;
      output float         __nonspecular    = 0;
)
{
  varying float atten = 0, cosangle = 0;
  uniform float cosoutside, cosinside, angle;
  uniform float lbn, rbn, tbn, bbn, penumbraRatio;
  varying point Pshad;
  varying float Pz, Pz_width;

  if( penumbraangle < 0 ) {
    angle = coneangle;
    cosoutside = cos( coneangle );
    cosinside = cos( coneangle + penumbraangle );
    //printf("o: %f, i: %f \n", cosoutside, cosinside );
  } else {
    angle = coneangle + penumbraangle;
    cosoutside = cos( angle );
    cosinside = cos( coneangle );
    //printf("o: %f, i: %f \n", cosoutside, cosinside );
  }

  __barn = 1.0;
  if( barndoors != 0 ) {
    lbn = -leftbarndoor / angle;
    rbn = rightbarndoor / angle;
    tbn = topbarndoor / angle;
    bbn = -bottombarndoor / angle;
    penumbraRatio = penumbraangle / angle * 0.5;
  }

  if ( decayRegions != 0 || barndoors != 0 ) {
    Pshad = transform("shader", Ps);
    Pz = zcomp(Pshad);
    Pz_width = max (abs(Du(Pz)*du) + abs(Dv(Pz)*dv), 1.0e-6);
  }

  varying color finalShadowColor = shadowcolor;
  if ( shadowcolorMix != -1 ) {
    finalShadowColor = mix( shadowcolor, shadowcolorSurf, shadowcolorMix );
  }



  illuminate( point "shader" ( 0, 0, 0 ), vector "shader" ( 0, 0, 1), angle ) {


    float distance = length( L );
    cosangle = ( L . vector "shader" ( 0, 0, 1) ) / distance;


    varying float pwc = max (abs(Du(cosangle)*du) + abs(Dv(cosangle)*dv), 1.0e-6);

    atten = 1 / pow( distance, decay );
    atten *= pow( cosangle, dropoff );
    atten *= smoothstep( cosoutside + pwc, cosinside + pwc*2, cosangle );

    // decay regions support
    if ( decayRegions != 0 ) {

      varying float region = 0, mask = 0;

      mask =  smoothstep( startDistance1 - Pz_width, startDistance1 + Pz_width, Pz ) - smoothstep( endDistance1 - Pz_width, endDistance1 + Pz_width, Pz );
      if ( startDistanceIntensity1 != 1 || endDistanceIntensity1 != 1 )
        mask *= mix( startDistanceIntensity1, endDistanceIntensity1, smoothstep( startDistance1, endDistance1, Pz ) );
      region = mask;

      mask = smoothstep( startDistance2 - Pz_width, startDistance2 + Pz_width, Pz ) - smoothstep( endDistance2 - Pz_width, endDistance2 + Pz_width, Pz );
      if ( startDistanceIntensity2 != 1 || endDistanceIntensity2 != 1 )
        mask *= mix( startDistanceIntensity2, endDistanceIntensity2, smoothstep( startDistance2, endDistance2, Pz ) );
      region += mask;

      mask = smoothstep( startDistance3 - Pz_width, startDistance3 + Pz_width, Pz ) - smoothstep( endDistance3 - Pz_width, endDistance3 + Pz_width, Pz );
      if ( startDistanceIntensity3 != 1 || endDistanceIntensity3 != 1 )
        mask *= mix( startDistanceIntensity3, endDistanceIntensity3, smoothstep( startDistance3, endDistance3, Pz ) );
      region += mask;

      atten *= region;

    }

    // barn doors
    if( barndoors != 0 ) {

      varying point PP = Pshad / Pz;
      varying float x = xcomp( PP ) / tan(angle);
      varying float y = ycomp( PP ) / tan(angle);

      varying float pwx = max (abs(Du(x)*du) + abs(Dv(x)*dv), 1.0e-6);
      varying float pwy = max (abs(Du(y)*du) + abs(Dv(y)*dv), 1.0e-6);

      __barn = smoothstep( lbn - pwx - penumbraRatio, lbn + pwx + penumbraRatio, x ) *
              ( 1- smoothstep( rbn - pwx - penumbraRatio, rbn + pwx + penumbraRatio, x ) ) *
              ( 1 - smoothstep( tbn - pwy - penumbraRatio, tbn + pwy + penumbraRatio, y ) ) *
              smoothstep( bbn - pwy - penumbraRatio, bbn + pwy + penumbraRatio, y );

    }

    // shadows
    if( shadowname != "" ) {

      uniform float shadowsize[2];
      if ( shadowname == "raytrace" ) shadowsize[0] = 5;
      else textureinfo( shadowname, "resolution", shadowsize );
      __shadowF = shadow( shadowname, Ps, "samples", shadowsamples, "bias", shadowbias, "blur", shadowfiltersize*1/shadowsize[0] + shadowblur );
#if defined ( AIR ) || defined ( AQSIS )
      __shadowC = color( mix( comp( lightcolor, 0 ), comp(finalShadowColor,0), __shadowF ),
                         mix( comp( lightcolor, 1 ), comp(finalShadowColor,1), __shadowF ),
                         mix( comp( lightcolor, 2 ), comp(finalShadowColor,2), __shadowF )	);
#else
      __shadowC = mix( lightcolor, finalShadowColor, __shadowF );
#endif
    } else {

      __shadowF = 0.0;
      __shadowC = lightcolor;

    }

    // output
    Cl = intensity * atten * __barn;
    __unshadowed_Cl = Cl;
    Cl *= __shadowC;

  }
}


