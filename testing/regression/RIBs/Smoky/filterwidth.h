/************************************************************************
 * filterwidth.h - Some handy macros for filter size estimation
 *                 for antialiasing of shaders.
 *
 * Author: Larry Gritz (lg AT larrygritz DOT com)
 *
 * Reference:
 *   _Advanced RenderMan: Creating CGI for Motion Picture_, 
 *   by Anthony A. Apodaca and Larry Gritz, Morgan Kaufmann, 1999.
 *
 * $Revision: 1.2 $    $Date: 2003/12/24 06:18:06 $
 *
 ************************************************************************/



#ifndef FILTERWIDTH_H
#define FILTERWIDTH_H 1

/* Define metrics for estimating filter widths, if none has already
 * been defined.  This is crucial for antialiasing.
 */
#ifndef MINFILTWIDTH
#  define MINFILTWIDTH 1.0e-6
#endif


/* The filterwidth macro takes a float and returns the approximate 
 * amount that the float changes from pixel to adjacent pixel.
 */
#define filterwidth(x)  max (abs(Du(x)*du) + abs(Dv(x)*dv), MINFILTWIDTH)


/* The filterwidthp macro is similar to filterwidth, but is for 
 * point data.
 */
#define filterwidthp(p) max (sqrt(area(p)), MINFILTWIDTH)



/* Given a function g, its known average g_avg, its high frequency
 * feature size, and a filter width, fade to the average as we approach
 * the Nyquist limit.
 */
#define fadeout(g,g_avg,featuresize,fwidth) \
        mix (g, g_avg, smoothstep(.2,.6,fwidth/featuresize))


#endif /* FILTERWIDTH_H */
