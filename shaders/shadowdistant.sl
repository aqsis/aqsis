/* $Id: shadowdistant.sl,v 1.1 2002/08/12 15:18:34 pgregory Exp $  (Pixar - RenderMan Division)  $Date: 2002/08/12 15:18:34 $ */
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
shadowdistant(
    float  intensity=1 ;
    color  lightcolor=1 ;
    point from = point "shader" (0,0,0) ;
    point to   = point "shader" (0,0,1) ;
    string shadowname="";
    float samples=16;
    float width=1;
)
{
    solar( to - from, 0.0 ) {
	Cl = intensity * lightcolor;
	if (shadowname != "") {
		Cl *= 1 - shadow(shadowname, Ps, "samples", samples,
			"swidth", width, "twidth", width);
	}
    }
}
