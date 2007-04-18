/* @(#)wood.sl1.5 (Pixar - RenderMan Division) 7/31/90 */

/*-______________________________________________________________________
** 
** Copyright (c) 1989 PIXAR.  All rights reserved.  This program or
** documentation contains proprietary confidential information and trade
** secrets of PIXAR.  Reverse engineering of object code is prohibited.
** Use of copyright notice is precautionary and does not imply
** publication.
** 
**                      RESTRICTED RIGHTS NOTICE
** 
** Use, duplication, or disclosure by the Government is subject to the 
** following restrictions:  For civilian agencies, subparagraphs (a) through
** (d) of the Commercial Computer Software--Restricted Rights clause at
** 52.227-19 of the FAR; and, for units of the Department of Defense, DoD
** Supplement to the FAR, clause 52.227-7013 (c)(1)(ii), Rights in
** Technical Data and Computer Software.
** 
** Pixar
** 3240 Kerner Blvd.
** San Rafael, CA  94901
** 
** ______________________________________________________________________
*/

/*----------------------------------------------------------------------------
 * wood - solid-wood shader.
 *
 * Ka, Kd, Ks, specularcolor, roughness - the standard meaning.
 * grain - controls the distance between growth rings (as grain gets
 *larger, the rings get closer).
 * swirl - the amount of turbulence in the wood pattern.
 * swirlfreq - the relative frequency of the turbulence (i.e. waves vs. noise).
 * c0, c1 - two points on the center line of the tree from which the
 *wood was taken.  This shader creates concentric rings out from
 *this line.
 * darkcolor - the color of the dark grain bands.  By default, this is a
 *fraction (.3) of the surface color Cs.
 *--------------------------------------------------------------------------*/
surface
wood(float Ka=1, Kd=.6, Ks=0.4, roughness=.2, grain=5, swirl=0.25, swirlfreq=1;
point c0=point "shader" (0,0,0), c1=point "shader" (0,0,1);
color specularcolor = 1, darkcolor = -1;/* -1 means depend on Cs */)
{
    point cP, C1, C0, PP, Nf, V, newP;
    float dd, pd, alpha, nn;
    float defaultweight = .3;
    color Cwood, mixcolor;

    Nf = faceforward( normalize(N), I );
    V = -normalize(I);

    /* Compute the distance from P to the line (c0, c1). */
    C1 = transform("shader",c1);
    C0 = transform("shader",c0);
    cP = normalize(C1 - C0);
    newP = transform("shader",P);
    PP = newP - C0;
    pd = PP.cP;
    dd = sqrt(abs(PP.PP - pd*pd));

    /* add some turbulence */
    nn = swirl*noise(swirlfreq*newP);
    nn += 0.5*swirl*noise(2.0*swirlfreq*newP);
    nn += 0.25*swirl*noise(4.0*swirlfreq*newP);
    nn += 0.125*swirl*noise(8.0*swirlfreq*newP);
    nn += 0.0625*swirl*noise(16.0*swirlfreq*newP);
    nn += 0.03125*swirl*noise(32.0*swirlfreq*newP);
    dd += nn;

    /* Compute the scale factor to be applied to the color to generate the
       grain.  The factor will vary between .5 and 1. */
    alpha = mod(grain*dd, 1);
    alpha *= alpha;
    alpha = (1-alpha)/5 + .8 - 0.3*noise(10.0*grain*dd);
    /* force alpha between 0 and 1 */
    alpha = (alpha-0.5)/0.5;

    /* Finally, compute the output color.  It is a specular surface scaled by
       the grain pattern. The specularity also varies with the grain:
       darker wood is more specular */

    /* if darkcolor < 0, we are using default, which is a fraction of Cs */
    if (comp(darkcolor,0)<0) mixcolor = defaultweight * Cs;
    else mixcolor = darkcolor;
    Cwood = mix(mixcolor,Cs,alpha);

    Oi = Os;
    Ci = Os * (Cwood * ( Ka*ambient() + Kd*diffuse(Nf) ) +
(1-alpha+0.75) * Ks * specularcolor * specular(Nf,V,roughness) );
}