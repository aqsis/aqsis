/* Simple SL include file with errors in it */

#define snoise(x)    (2*noise(x) - 1)
#define PULSE(a,b,x) (step((a),(x)) - step((b),(x)))
#define boxstep(a,b,x) clamp(((x)-(a))/((b)-(a)),0,1)


