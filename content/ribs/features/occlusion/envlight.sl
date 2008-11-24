/*
 * Simple ambient light with ambient occlusion support. It is also used
 * new dso shadeops bake3d() and texture3d().
 */


light envlight (
    float intensity = 1;
    color lightcolor = 1;
    string filename = "occlmap.sm";
    float samples = 8;
    float blur = 0.01;
    float bias = 0.01;
    color shadowcolor = color(0,0,0);
    float tobake = 1.0;
)
{
	float o;
	normal Nf = Ns;
	if (tobake == 1.0)
	{
		o = occlusion (filename, Ps, Ns, 0,
		               "blur", blur,
		               "samples", samples,
		               "bias", bias);
		Cl = intensity * mix(lightcolor, shadowcolor, o);
		bake3d("envlight.ptc", "rgba", Ps, Nf, "Cl", Cl);
		/* This could  save the result of 'o' instead to envlight.ptc
		* bake3d("envlight.ptc", "rgba", Ps, Nf, "o", o);
		 */
	}
	else
	{
		/* read back the previous generated envlight.ptc to get 'Cl' */
		texture3d("envlight.ptc", Ps, Nf, "Cl", Cl);
		/*
		* texture3d("envlight.ptc", Ps, Nf, "o", o);
		* Cl = intensity * mix(lightcolor, shadowcolor, o);
		*/
	}

}
