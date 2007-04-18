surface cell( float Ka = 1;
       float Ks = 1;
       float roughness = .1;)
{
point ramp_p1= cellnoise(u * 10, v * 10);
color ramp_f1= cellnoise(u * 10, v * 10);

   float amplitude = comp(ramp_p1,0);
  
   N = calculatenormal(P + amplitude);
   Ci = ramp_f1;
   point Nf = faceforward (normalize(N),I);
   point V = -normalize(I);
   Oi = Os;
   Ci = Os * ramp_f1 * (Ka*ambient() + Ks*specular(Nf,V,roughness));
} 
