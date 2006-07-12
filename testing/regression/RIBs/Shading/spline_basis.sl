/*
  The the spline() function with different basis settings.
 */ 
surface spline_basis()
{
  color white = color "rgb" (1,1,1); 
  color red = color "rgb" (1,0,0); 
  color green = color "rgb" (0,1,0); 
  color blue = color "rgb" (0,0,1); 
  color color1 = color "rgb" (0,1,1);
  color color2 = color "rgb" (1,0,1);
  color color3 = color "rgb" (1,1,0);

  float column = floor(6.0*s);

  if (column==0)
  {
    Ci = spline(v, white, white, red, green, blue, color1, color2, color3, white, white);
  }
  else if (column==1)
  {
    Ci = spline("catmull-rom", v, white, white, red, green, blue, color1, color2, color3, white, white);
  }
  else if (column==2)
  {
    Ci = spline("linear", v, white, white, red, green, blue, color1, color2, color3, white, white);
  }
  else if (column==3)
  {
    Ci = spline("bspline", v, white, white, red, green, blue, color1, color2, color3, white, white);
  }
  else if (column==4)
  {
    Ci = spline("bezier", v, white, white, red, green, blue, color1, color2, color3, white, white);
  }
  else
  {
    Ci = spline("hermite", v, white, white, red, green, blue, color1, color2, color3, white, white);
  }

  Oi = 1;
}
