/*
 * dbo_plane.c - sample Dynamic Blob Op
 *
 * Description:  a unit square in the xy-plane, at z=0,
 *   centered at the origin
 *   The blob expects a single float argument that gives
 *   the height at which the field goes to 0
 * 
 * (c) 2003 SiTex Graphics, Inc.
 *
 */

#include "implicit.h"

EXPORT void ImplicitBound(State *s, float *bd,
                                          int niarg, int *iarg,
                                          int nfarg, float *farg,
                                          int nsarg, char **sarg)
{
  bd[0]=-0.5;
  bd[1]=0.5;
  bd[2]=-0.5; bd[3]=0.5;
  bd[4]=-farg[0]; bd[5]=farg[0];
}

float level(float v, float h)
{
  float r=(v/h+1.0f)*0.5f;
  if (r<0.0) r=0.0f;
  if (r>1.0) r=1.0f;
  return r;
}

EXPORT void ImplicitValue(State *s, float *result, float *p,
                                          int niarg, int *iarg,
                                          int nfarg, float *farg,
                                          int nsarg, char **sarg)
{
  if ((p[0]<-0.5)||(p[0]>0.5)||(p[1]<-0.5)||(p[1]>0.5)) {
    result[0]=1.0;
  } else {
    result[0] = level(p[2],farg[0]);
  }
}


EXPORT void ImplicitRange(State *s, float *rng,
                                          float *bd,
                                          int niarg, int *iarg,
                                          int nfarg, float *farg,
                                          int nsarg, char **sarg)
{
  if ((bd[0]>0.5)||(bd[1]<-0.5)||(bd[2]>0.5)||(bd[3]<-0.5)||(bd[4]>farg[0])) {
     rng[0]=1.0; rng[1]=1.0;
  } else {
    rng[0]=level(bd[4],farg[0]); rng[1]=level(bd[5],farg[0]);
  }
}

EXPORT void ImplicitFree(State *s)
{
}


