/************************************************************************
 * noises.h - various noise-based patterns
 *
 * $Revision: 1.1 $    $Date: 1998-04-05 15:53:48-07 $
 *
 ************************************************************************/



#ifndef filterwidth
/*
 * Define metrics for estimating filter widths, if none has already
 * been defined.  This is crucial for antialiasing.
 */
#define MINFILTWIDTH 1.0e-6
#define filterwidth(x)  max (abs(Du(x)*du) + abs(Dv(x)*dv), MINFILTWIDTH)
#define filterwidthp(p) max (sqrt(area(p)), MINFILTWIDTH)
#endif



#ifndef snoise
/*
 * Signed noise -- the original Perlin kind with range (-1,1) We prefer
 * signed noise to regular noise mostly because its average is zero.
 * We define three simple macros:
 *   snoise(p) - Perlin noise on either a 1-D (float) or 3-D (point) domain.
 *   snoisexy(x,y) - Perlin noise on a 2-D domain.
 *   vsnoise(p) - vector-valued Perlin noise on either 1-D or 3-D domain.
 */
#define snoise(p) (2 * (float noise(p)) - 1)
#define snoisexy(x,y) (2 * (float noise(x,y)) - 1)
#define vsnoise(p) (2 * (vector noise(p)) - 1)
#endif


/* If we know the filter size, we can crudely antialias snoise by fading
 * to its average value at approximately the Nyquist limit.
 */
#define filteredsnoise(p,width) (snoise(p) * (1 - smoothstep (0.2,0.6,width)))
#define filteredvsnoise(p,width) (vsnoise(p) * (1-smoothstep (0.2,0.6,width)))



/* Having filteredsnoise leads easily to an antialiased version of fBm. */
float fBm (point p; float filtwidth;
           uniform float maxoctaves, lacunarity, gain)
{
    uniform float i;
    uniform float amp = 1;
    varying point pp = p;
    varying float sum = 0, fw = filtwidth;

    for (i = 0;  i < maxoctaves && fw < 1.0;  i += 1) {
#pragma nolint
	sum += amp * filteredsnoise (pp, fw);
	amp *= gain;  pp *= lacunarity;  fw *= lacunarity;
    }
    return sum;
}


/* Typical use of fBm: */
#define fBm_default(p)  fBm (p, sqrt(area(p)), 4, 2, 0.5)





/* A vector-valued antialiased fBm. */
vector
vfBm (point p; float filtwidth;
      uniform float maxoctaves, lacunarity, gain)
{
    uniform float i;
    uniform float amp = 1;
    varying point pp = p;
    varying vector sum = 0;
    varying float fw = filtwidth;

    for (i = 0;  i < maxoctaves && fw < 1.0;  i += 1) {
	sum += amp * filteredvsnoise (pp, fw);
	amp *= gain;  pp *= lacunarity;  fw *= lacunarity;
    }
    return sum;
}


/* Typical use of vfBm: */
#define vfBm_default(p)  vfBm (p, sqrt(area(p)), 4, 2, 0.5)



/* The stuff that Ken Musgrave calls "VLNoise" */
#define VLNoise(Pt,scale) (snoise(vsnoise(Pt)*scale+Pt))
#define filteredVLNoise(Pt,fwidth,scale) \
            (filteredsnoise(filteredvsnoise(Pt,fwidth)*scale+Pt,fwidth))


float
VLfBm (point p; float filtwidth;
       uniform float maxoctaves, lacunarity, gain, scale)
{
    uniform float i;
    uniform float amp = 1;
    varying point pp = p;
    varying float sum = 0;
    varying float fw = filtwidth;

    for (i = 0;  i < maxoctaves && fw < 1.0;  i += 1) {
#pragma nolint
	sum += amp * filteredVLNoise (pp, fw, scale);
	amp *= gain;  pp *= lacunarity;  fw *= lacunarity;
    }
    return sum;
}


/* Typical use of vfBm: */
#define VLfBm_default(p)      VLfBm (p, sqrt(area(p)), 4, 2, 0.5, 1.0)




/* Antialiased turbulence.  Watch out -- the abs() call introduces infinite
 * frequency content, which makes our antialiasing efforts much trickier!
 */
float turbulence (point p; float filtwidth;
                  uniform float maxoctaves, lacunarity, gain)
{
    uniform float i;
    uniform float amp = 1;
    varying point pp = p;
    varying float sum = 0, fw = filtwidth;

    for (i = 0;  i < maxoctaves && fw < 1.0;  i += 1) {
#pragma nolint
	sum += amp * abs(filteredsnoise (pp, fw));
	amp *= gain;  pp *= lacunarity;  fw *= lacunarity;
    }
    return sum;
}


/* Typical use of turbulence: */
#define turbulence_default(p)  turbulence (p, sqrt(area(p)), 4, 2, 0.5)
