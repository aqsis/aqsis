/*
 * show_N(): color surface point according to its normal
 */
surface
show_N(float use_Ng = 0)
{
	if(use_Ng!=0)
		Ci = (color(normalize(Ng))+1)/2;
	else
		Ci = (color(normalize(N))+1)/2;
	Oi = 1;
}

