/**************************************************************************
 * patterns.h - various "helper" routines for making patterns, including
 *              antialiased versions of some useful functions.
 *
 * Author: Larry Gritz (gritzl@acm.org), though most are obvious to any
 *         experienced shader writer.
 *
 * $Revision: 1.1 $    $Date: 2001/04/27 20:36:12 $
 *
 **************************************************************************/


#ifndef PATTERNS_H
#define PATTERNS_H 1


/*
 * Define metrics for estimating filter widths, if none has already
 * been defined.  This is crucial for antialiasing.
 */
#ifndef MINFILTWIDTH
#  define MINFILTWIDTH 1.0e-6
#endif
#ifndef filterwidth
#  define filterwidth(x)  max (abs(Du(x)*du) + abs(Dv(x)*dv), MINFILTWIDTH)
#endif
#ifndef filterwidthp
#  define filterwidthp(p) max (sqrt(area(p)), MINFILTWIDTH)
#endif



/* A 1-D pulse pattern:  return 1 if edge0 <= x <= edge1, otherwise 0 */
float pulse (float edge0, edge1, x)
{
    return step(edge0,x) - step(edge1,x);
}


/* A filtered version of pulse:  this is just an analytic solution to
 * the convolution of pulse, above, with a box filter of a particular
 * width.  Derivation is left to the reader.
 */
float filteredpulse (float edge0, edge1, x, width)
{
    float x0 = x - width*0.5;
    float x1 = x0 + width;
    return max (0, (min(x1,edge1)-max(x0,edge0)) / width);
}




/* A pulse train: a signal that repeats with a given period, and is
 * 0 when 0 <= mod(x,period) < edge, and 1 when mod(x,period) > edge.
 */
float pulsetrain (float period, duty, x)
{
    return pulse (duty*period, 1, mod(x,period));
}



/* Filtered pulse train:  it's not as simple as just returning the mod
 * of filteredpulse -- you have to take into account that the filter may
 * cover multiple pulses in the train.
 * Strategy: consider the function INTFPT, which is the integral of the
 * train from 0 to x.  Just subtract!
 */
float filteredpulsetrain (float period, duty, x, width)
{
    /* First, normalize so period == 1 and our domain of interest is > 0 */
    float w = width/period;
    float x0 = x/period - w/2;
    float x1 = x0+w;
    /* Now we want to integrate the normalized pulsetrain over [x0,x1] where
     * 0 <= x0 < 1 and x0 < x1. 
     */
#define INTFPT(x) ((1-duty)*floor(x) + max(0,x-floor(x)-duty))
    return (INTFPT(x1) - INTFPT(x0)) / (x1-x0);
#undef INTFPT    
}





/* basic tiling pattern --
 *   inputs:
 *      x, y                    positions on a 2-D surface
 *      tilewidth, tileheight   dimensions of each tile
 *      rowstagger              how much does each row stagger relative to
 *                                   the previous row
 *      rowstaggervary          how much should rowstagger randomly vary
 *      jaggedfreq, jaggedamp   adds noise to the edge between the tiles
 *   outputs:
 *      row, column             index which tile the sample is in
 *      xtile, ytile            position within this tile (0-1)
 */
void basictile (float x, y;
		uniform float tilewidth, tileheight;
		uniform float rowstagger, rowstaggervary;
		uniform float jaggedfreq, jaggedamp;
		output float column, row;
		output float xtile, ytile;
    )
{
    point PP;
    float scoord = x, tcoord = y;

    if (jaggedamp != 0.0) {
	/* Make the shapes of the bricks vary just a bit */
	PP = point noise (x*jaggedfreq/tilewidth, y*jaggedfreq/tileheight);
	scoord += jaggedamp * xcomp (PP);
	tcoord += jaggedamp * ycomp (PP);
    }

    xtile = scoord / tilewidth;
    ytile = tcoord / tileheight;
    row = floor (ytile);   /* which brick row? */

    /* Shift the columns randomly by row */
    xtile += mod (rowstagger * row, 1);
    xtile += rowstaggervary * (noise (row+0.5) - 0.5);

    column = floor (xtile);
    xtile -= column;
    ytile -= row;
}



#endif /* defined(PATTERNS_H) */
