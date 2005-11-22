surface show_sphere_st()
{
  point Pobj = transform("object", P);
  float sig = asin(zcomp(Pobj));
  float th = acos(xcomp(Pobj)/cos(sig));
  float tt = (sig + PI/2)/PI;
  float ss = th/(2*PI);
  
  Ci = color "rgb" (clamp(1-10*(ss-s),0,1) ,clamp(1-10*(tt-t),0,1) ,0);
  Oi = 1;
}