surface show_dpdu()
{
  vector d = 0.5*normalize(dPdu)+vector(0.5,0.5,0.5);
  Ci = color "rgb" (xcomp(d), ycomp(d), zcomp(d));
  Oi = 1;
}
