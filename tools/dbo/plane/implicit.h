/*
 * Header for user-defined implicit surface
 *
 * (c) 2003 SiTex Graphics, Inc.
 *
 *
 * 
 * Permission was granted to aqsis 
 * 
 * Yes, you are allowed to implement AIR's interface for DBOs in aqsis.  You may also include the implicit.h header with aqsis if you wish.
 * Regards,
 * Scott Iverson
 * SiTex Graphics Support
 * support@sitexgraphics.com
 * http://www.sitexgraphics.com
 */

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__BORLANDC__) || defined(_MSC_VER)
#define EXPORT __declspec (dllexport)
#else
#define EXPORT extern
#endif

typedef struct {
	int BlobbyId;
	int OpId;
} State;

EXPORT void ImplicitBound(State *s, float *bd,
                                          int niarg, int *iarg,
                                          int nfarg, float *farg,
																					int nsarg, char **sarg);

/* ImplicitValue returns a scaled distance in *result.
   The blob's field of influence is 0 for distances greater than 1.
   The surface is located at distance=0.5.
 */

EXPORT void ImplicitValue(State *s, float *result,
                                          float *p,
                                          int niarg, int *iarg,
                                          int nfarg, float *farg,
																					int nsarg, char **sarg);


/* ImplicitRange returns a range as two floats in rng, giving
   the possible range of values for all points in bd.
   If the bound is completely outside the field of the blob, the
   function should return [1 1]
 */

EXPORT void ImplicitRange(State *s, float *rng,
                                          float *bd,
                                          int niarg, int *iarg,
                                          int nfarg, float *farg,
																					int nsarg, char **sarg);

/* ImplicitFree (optional) called once after processing each Blobby.
   NB:  ImplicitFree may be called for a Blobby even though none of the
   other functions were queried.
 */

EXPORT void ImplicitFree(State *s);


EXPORT void ImplicitMotion(State *s, float *movec,
                           float *pt, float *times,
                           int niarg, int *iarg,
                           int nfarg, float *farg,
                           int nsarg, char **sarg);


#ifdef __cplusplus
}
#endif

