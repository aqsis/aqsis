light
bml(
    float  intensity = 1;
    color  lightcolor = 1;
    point  from = point "shader" (0,0,0);	/* light position */
    point  to = point "shader" (0,0,1);
    float  coneangle = radians(30);
    float  conedeltaangle = radians(5);
    float  beamdistribution = 2;
    string shadowname = "";
    float samples = 16;
    float blur = 0.01;
    float  shadowbias = -1e9;
)
{
    float  atten, cosangle;
    uniform vector A = (to - from) / length(to - from);
    uniform float cosoutside= cos(coneangle),
		  cosinside = cos(coneangle-conedeltaangle);

    illuminate( from, A, coneangle ) {
	cosangle = L.A / length(L);
	atten = pow(cosangle, beamdistribution) / (L.L);
	atten *= smoothstep( cosoutside, cosinside, cosangle );
	if (shadowname != "") 
	{
		atten *= 1-shadow(shadowname, Ps, "samples", samples, "blur", blur, "bias", shadowbias);
	}
	Cl = atten * intensity * lightcolor;
    }
}
