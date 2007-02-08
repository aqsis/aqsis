/* $Id: //depot/branches/rmanprod/rman-11.0/shaders/cmarble.sl#1 $  (Pixar - RenderMan Division)  $Date: 2002/10/21 $ */
/*
** Copyright (c) 1999 PIXAR.  All rights reserved.  This program or
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
** 1001 West Cutting Blvd.
** Richmond, CA  94804
*/
/*-----------------------------------------------------------------------
 * cmarble - a surface shader producing white marble with colored veins
 *		whose color is determined from Cs.
 *
 * Ks, Ks, Ka, roughness, specularcolor - the usual meaning
 * veining - frequency of veins
 *-----------------------------------------------------------------------*/
surface
cmarble(
	float Ks=.4,Kd=.6, Ka=.1, roughness=.1;
	float veining = 1;
	color specularcolor=1)
{
	varying point PP;
	varying float cmi;
	varying normal Nf;
	varying vector V;
	color diffusecolor ;
	float pixelsize, twice, scale, weight, turbulence;
	/* variables for rgb to hsv conversion ... */
	string spoke;	/* part of color wheel it's in */
	float x;	/* smallest RGB component */
	float hue, sat, val;
	float red, grn, blu;


	Nf = faceforward( normalize(N), I);
	V = -normalize(I);

	PP = transform("shader",P) * veining;
	PP = PP/2;	/* frequency adjustment (S-shaped curve) */
	pixelsize = sqrt(area(PP)); twice = 2 * pixelsize;

	/* compute turbulence */
	turbulence = 0;
	for (scale = 1; scale > twice; scale /= 2) {
		turbulence += scale * abs(noise(PP/scale)-0.5);
	}
	/* gradual fade out of highest freq component near visibility limit */
	if (scale > pixelsize) {
		weight = (scale / pixelsize) - 1;
		weight = clamp(weight, 0, 1);
		turbulence += weight * scale * abs(noise(PP/scale)-0.5);
	}
	/*
	 * turbulence now has a range of 0:2, but its values actually
	 * tend strongly to lie around 0.75 to 1.
	 */

	cmi = clamp(turbulence, 0, 1);

	/*
	 * Assuming that Cs is in RGB space, convert to HSV space so
	 * that we can modulate saturation and value without changing hue.
	 */
	red = comp(Cs, 0); grn = comp(Cs, 1); blu = comp(Cs, 2);

	/* set val to largest rgb component, x to smallest */
	if (red >= grn && red >= blu) {
		/* red largest */
		val = red;
		if (grn > blu) {
			x = blu;
			spoke = "Rb";
		} else {
			x = grn;
			spoke = "Rg";
		}
	} else if (grn >= red && grn >= blu) {
		/* green largest */
		val = grn;
		if (red > blu) {
			x = blu;
			spoke = "Gb";
		} else {
			x = red;
			spoke = "Gr";
		}
	} else {
		/* blue largest */
		val = blu;
		if (grn > red) {
			x = red;
			spoke = "Br";
		} else {
			x = grn;
			spoke = "Bg";
		}
	}
	hue = 0.;		/* default hue is red */
	sat = 0.;		/* default saturation is unsaturated (gray) */

	if (val > 0.0) {	/* not black */
		sat = (val - x)/val;	/* actual saturation */
		if (sat > 0.0) {	/* not a gray, so hue matters */
			/* now compute actual hue */
			if (spoke == "Rb") { /* red largest, blu smallest */
				hue = 1 - (val - grn)/(val - x);
			} else if (spoke == "Rg") {
				hue = 5 + (val - blu)/(val - x);
			} else if (spoke == "Gr") {
				hue = 3 - (val - blu)/(val - x);
			} else if (spoke == "Gb") {
				hue = 1 + (val - red)/(val - x);
			} else if (spoke == "Br") {
				hue = 3 + (val - grn)/(val - x);
			} else if (spoke == "Bg") {
				hue = 5 - (val - red)/(val - x);
			}
			hue *= 1/6.0;
		}
	}

	/*
	 * Now we have Cs in hue,sat,val (HSV) form.  Proceed to modulate
	 * saturation and value based on the turbulence computed earlier.
	 * Value is blended between the value of Cs and white (value = 1).
	 * So a "black" marble has black veins and a "white" marble has
	 * white veins outlined in dark gray.
	 */

	sat *= float spline(turbulence,
		0.999, 0.999, 0.200, 0.060, 0.030, 0.020,
		0.010, 0.010, 0.000);
	val = val * float spline(turbulence,
		0.999, 0.999, 0.400, 0.000,
		0.000, 0.000, 0.000,
		0.000, 0.000, 0.000, 0.000
		) + float spline(turbulence,
		0.000, 0.000, 0.000, 0.800,
		0.800, 0.800, 0.800,
		0.999, 0.999, 0.999, 0.999);

	/*
	 * Now reconstruct an RGB color from the hue,sat,val and use
	 * that as the diffuse surface color in a "plastic" shading formula.
	 */

	diffusecolor = color "hsv" (hue, sat, val);

	Oi = Os;
	Ci = diffusecolor * (Ka*ambient() + Kd*diffuse(Nf));
	/* add in specular component */
	Ci = Os * (Ci + specularcolor * Ks * specular(Nf,V,roughness));
}


