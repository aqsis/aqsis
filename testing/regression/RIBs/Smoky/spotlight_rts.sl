light
  spotlight_rts(
    float  intensity = 1;
    color  lightcolor = 1;
    point  from = point "shader" (0,0,0);
    point  to = point "shader" (0,0,1);
    float  coneangle = radians(30);
    float  conedeltaangle = radians(5);
    float  beamdistrib = 2;
    float samples = 1;
    float width = 1;
    float blur = 0;
    float bias = -1;
    float falloff=2.0;
  )
  {
    color t;
    float cosangle;
    uniform vector A;
    uniform float outc, inc;

    A = (to - from) / length(to - from);
    outc = cos(coneangle);
    inc  = cos(coneangle-conedeltaangle);

    illuminate( from, A, coneangle ) {
	float ldist = length(L);
	cosangle = L.A / ldist;

	float dim;
	if(ldist>=1.0)
	    dim = pow(ldist,falloff);
	else
	    dim=1.0;

	t = pow(cosangle, beamdistrib) / dim;
	t *= smoothstep(outc, inc, cosangle);

#if !defined(AQSIS) && !defined(AIR) && !defined(RDC)
	t *= transmission(Ps, from,
	    	"samples", samples,
		"samplebase", width,
		"samplecone", blur,
		"bias", bias);
#endif
	Cl = t * intensity * lightcolor;
    }
  }
