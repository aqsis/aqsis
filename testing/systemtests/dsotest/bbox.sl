/*
 * bbox() -- give the bounding box for the object(s) that use
 *   this function.
 *
 * This is a simple example of using a shader DSO.
 *
 * AUTHOR: Tal Lancaster
 *
 * HISTORY:
 *  Created: Oct 2/1998
 */

surface bbox (
  float Ka=1, Kd=1;
)
{
  normal Nf;
  float discard;

  point Praster = transform ("raster", P);
  point Pworld = transform ("world", P);

  Nf = faceforward(normalize(N),I);

#if 1
 discard = bbox (Praster);
#else
 discard = bbox (Pworld);
#endif

  Ci =  Cs * ( Ka*ambient() + Kd*diffuse(Nf) ) ;
}

