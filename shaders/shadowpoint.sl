/* $Id: shadowpoint.sl,v 1.1 2002/08/12 15:18:34 pgregory Exp $  (Pixar - RenderMan Division)  $Date: 2002/08/12 15:18:34 $ */
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
light
shadowpoint(
    float intensity = 1;
    color lightcolor = 1;
    point from = point "shader" (0,0,0);	/* light position */
    string    sfpx    = "";
    string    sfnx    = "";
    string    sfpy    = "";
    string    sfny    = "";
    string    sfpz    = "";
    string    sfnz    = "";
    uniform float width      = 1.0;
    uniform float samples    = 16.0;
)
{
    float    attenuation = 0.0;
    float    Lx, Ly, Lz, AbsLx, AbsLy, AbsLz;
    vector   Lrel;

    illuminate( from ) {

        Lrel = vtransform("world", L);

        Lx = xcomp(Lrel);
        AbsLx = abs(Lx);
        Ly = ycomp(Lrel);
        AbsLy = abs(Ly);
        Lz = zcomp(Lrel);
        AbsLz = abs(Lz);

        if((AbsLx > AbsLy) && (AbsLx > AbsLz)) {
            if((Lx > 0.0)&&(sfpx != ""))
		attenuation = shadow( sfpx, Ps, "samples", samples,
                                       "twidth", width, "swidth", width );
            else if (sfnx != "")
		attenuation = shadow( sfnx, Ps, "samples", samples,
                                      "twidth", width, "swidth", width );
	}
        else if((AbsLy > AbsLx) && (AbsLy > AbsLz)) {
            if((Ly > 0.0)&&(sfpy != ""))
		attenuation = shadow( sfpy, Ps, "samples", samples,
                                      "twidth", width, "swidth", width );
            else if (sfny != "")
		attenuation = shadow( sfny, Ps, "samples", samples,
                                      "twidth", width, "swidth", width );
	}
        else if((AbsLz > AbsLy) && (AbsLz > AbsLx)) {
            if((Lz > 0.0)&&(sfpz != ""))
		attenuation = shadow( sfpz, Ps, "samples", samples,
                                      "twidth", width, "swidth", width );
            else if (sfnz != "")
		attenuation = shadow( sfnz, Ps, "samples", samples,
                                      "twidth", width, "swidth", width );
	}

        /* calculate light contribution  distance square fall off */
        Cl = (1.0 - attenuation) * intensity * lightcolor / L.L;
    }
}
