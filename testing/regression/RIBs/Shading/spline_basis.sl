/*
  The the spline() function with different basis settings.
 */ 
surface spline_basis()
{
  color map[10] = { color "rgb" (1,1,1), 
		   color "rgb" (1,1,1),
		   color "rgb" (1,0,0),
		   color "rgb" (0,1,0),
		   color "rgb" (0,0,1),
		   color "rgb" (0,1,1),
		   color "rgb" (1,0,1),
		   color "rgb" (1,1,0),
		   color "rgb" (1,1,1), 
		   color "rgb" (1,1,1) };

  float column = floor(6.0*s);

  if (column==0)
  {
    Ci = spline(v, map);
  }
  else if (column==1)
  {
    Ci = spline("catmull-rom", v, map);
  }
  else if (column==2)
  {
    Ci = spline("linear", v, map);
  }
  else if (column==3)
  {
    Ci = spline("bspline", v, map);
  }
  else if (column==4)
  {
    Ci = spline("bezier", v, map);
  }
  else
  {
    Ci = spline("hermite", v, map);
  }

  Oi = 1;
}
