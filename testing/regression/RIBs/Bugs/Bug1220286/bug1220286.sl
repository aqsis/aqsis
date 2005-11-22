/**
 * Takes two varying float values called fs and ft, and colours the surface so
 * that fs controls the amount of red and ft controls the amount of green.
 */
surface bug1220286 (
	varying float fs = 0.0;
	varying float ft = 0.0;
) {
	Oi = 1;
	Ci = fs * color(1,0,0) + ft * color(0,1,0);
}
