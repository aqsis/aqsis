displacement endrough(
	/* Amplitude of roughness at the hair end */
	float amount = 0.1;
	/* "Shape" of the end rough.  1 == linear, >1 == curve near tip,
	 * <1 == curve near base */
	float shape = 1;
	/* Randomness to add to the end of the hair.  This should be attached to
	 * the hairs as uniform parameter the components of endRoughRand should be
	 * between -1 and 1. */
	vector endRoughRand = (0,0,0);
	)
{
	P += amount*pow(v, shape)*endRoughRand;
	N = calculatenormal(P);
}
