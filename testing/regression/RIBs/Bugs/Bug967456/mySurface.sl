/* mySurface */

surface 
mySurface(float roughness = 0.1)
{
  normal Nf = faceforward(normalize(N), I);

  Oi = color 1;
  Ci = (color(1, 0, 0) * ambient() + 
        color(0, 1, 0) * diffuse(Nf) + 
        color(0, 0, 1) * specular(Nf, normalize(-I), roughness));
}


