/* I took wave's lead and renamed proctexh.h to DPProctext.h -- tal@cs.caltech.edu */

/*
 * Preprocessor macros for use in RenderMan shaders.
 * Darwyn Peachey, June, 1994.
 */

#define snoise(x)    (2*noise(x) - 1)
#define PULSE(a,b,x) (step((a),(x)) - step((b),(x)))
#define boxstep(a,b,x) clamp(((x)-(a))/((b)-(a)),0,1)
