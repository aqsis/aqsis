/** hair.sl from 
 *
 * "Advanced RenderMan: Beyond the Companion"
 * by Larry Gritz and Tony Apodaca,  course notes from Siggraph 1999 Course 25
 */

surface hair_gritz (
		float Ka = 1;
		float Kd = 0.6;
		float Ks = 0.35;
		float roughness = 0.15;
		color rootcolor = color(0.109, 0.037, 0.007);
		color tipcolor = color(0.519, 0.325, 0.125);
		color specularcolor = (color(1) + tipcolor)/2;
	)
{
	vector T = normalize(dPdv); /* tangent along lenght of hair */
	vector V = -normalize(I);   /* V is the view vector */
	color Cspec = 0, Cdiff = 0; /* collect specular & diffuse light */
	float cosang;
	/* Loop over lights, catch highlights as if this was a thin cylinder */
	illuminance(P)
	{
		cosang = cos(abs(acos(T.normalize(L)) - acos(-T.V)));
		Cspec += Cl * v * pow(cosang, 1/roughness);
		Cdiff += Cl * v;
		/* We multipled by v to make it darker at the roots.  This assumes v=0
		 * at the root, v=1 at the tip.
		 */
	}
	Oi = Os;
	Ci = Oi * (mix(rootcolor, tipcolor, v) * (Ka*ambient() + Kd*Cdiff)
			+ (Ks * Cspec * specularcolor));
}

