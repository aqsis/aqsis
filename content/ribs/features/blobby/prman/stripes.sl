surface stripes(float Ka = 1, 
                      Kd = 0.5, 
                      Ks = 0.7, 
                      roughness = 0.1,
                      frequency = 8;
                              color hilite = 1)
{
normal  nf, n;
vector  i;
color   ambientcolor, diffusecolor, speccolor,
        surfcolor = Cs, surfopac = Os;
 float   ss = mod(s * frequency, 1.0);

n = normalize(N);
nf = faceforward(n, I);
  
if(ss < 0.5)
    Oi = 0;
else
    Oi = surfopac;
  
 ambientcolor = Ka * ambient();
 diffusecolor = Kd * diffuse(nf);
i = normalize(-I);
speccolor = specular(nf, v, roughness);
  
 Ci = Oi * surfcolor * (ambientcolor + diffusecolor + speccolor);
}
