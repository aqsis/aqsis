/* debug.sl - Show micropolygrids by coloring with a random color.
 *
 * The RenderMan (R) Interface Procedures and RIB Protocol are:
 *     Copyright 1988, 1989, Pixar.  All rights reserved.
 * RenderMan (R) is a registered trademark of Pixar.
 */

color testfunc(float test[5])
{
	return(color(test[0],test[1],test[2]));
}

surface
debug (float test[5]={1,0,0.5,0.5,0.5})
{
  float some[3]={1,1,0};
  color colarray[3]={color(random(),random(),random()),
					 color(1,1,0),
					 color(1,0,0)};
  some[0]=random();
  some[1]=random();
  some[2]=random();
  float ff=some[0];

  color c = testfunc(some);
  c = colarray[2];
  Oi = Os;
  Ci = Os * c;
}

