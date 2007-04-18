surface equalng()
{
   color ct = color "rgb" (1.0, 0.0, 0.0);
   if (Ng == faceforward(N,I))
	ct = color "rgb" (0.0, 1.0, 0.0);
    Ci = ct;
    Oi = 1.0;
}
