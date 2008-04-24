// Volume data shader
// Copyright (C) 2008 Christopher J. Foster
//
// This software is licensed under the GPLv2 - see the file COPYING for details.

//------------------------------------------------------------------------------
// Function to obtain a density inside a volume, given the distance, d to
// some surface; d > 0 is considered to be inside.  The density is increased
// for small d to accentuate the surface.
//
// d = signed distance to surface
float density(float d)
{
	float dens = 0;
	if(d > 0)
	  dens = 1/(0.5+d*d) + 2;
	return dens;
}

// Determine if the given point is inside the sample box.  For our purposes,
// the sample box is the cube [0,1)^3.
float inBox(point samplePos)
{
	float x = xcomp(samplePos);
	float y = ycomp(samplePos);
	float z = zcomp(samplePos);
	float inside = 0;
	if(x >= 0 && x < 1 && y >= 0 && y < 1 && z >= 0 && z < 1)
	  inside = 1;
	return inside;
}

//------------------------------------------------------------------------------
// A volumetric data shader.  This shader does explicit ray-marching through a
// volume; the volume is considered to both emit light (it glows) and attenuate
// light passing through.
volume boxed_volume_glow(
	// colour for the interior of the volume
	color interiorColor = color(0.5,0.5,1);
	// colour used near the surface of the volume
	color surfaceColor = color(1,0,0);
	// width of the surface near which to use surfaceColor.
	float surfaceDepth = 0.3;
	// The amount of attenuation caused by absorption within the medium
	float attenuation = 1;
	// Intensity of the glow
	float intensity = 1;
	// Number of samples in the stratified monte carlo integration.
	float numSamples = 100;
	// Amount to scale the isosurface distance function by before using it in
	// the density function
	float isoScale = 1;
	)
{
	float totalDens = 0;
	float i = 0;
	point Pcurr = transform("data_box", P);
	vector nIbox = normalize(vtransform("data_box", I));
	vector deltaI = -nIbox*2/numSamples;
	color c = 0;
	// Check we're inside the data box to begin with.  This hack is necessary,
	// since the front faces of the box also spawn atmosphere shader
	// evaluations (ie, RiAtmosphere and transparent surfaces are rather
	// incompatible).  However, aqsis doesn't have raytracing.
	if(inBox(Pcurr + 0.001*deltaI))
	{
		// Integrate from the back of the volume to the front.  This makes
		// attenuation extra easy to deal with, but may be inefficient in the
		// case that we can shortcut the loop when attenuation is very strong.
		//
		// We break the interval up into equal-sized pieces, and choose a
		// random sample position inside each one.  This jittered sampling
		// is very simple to perform, but has poor stratification which implies
		// relatively high noise.  It produces less annoying artefacts than
		// deterministic sampling however (no banding).
		for(i = 0; i <= numSamples; i+=1)
		{
			// Choose a sample position randomly within the current interval.
			point samplePos = Pcurr + deltaI*(float random());
			// Stop if we've gone outside the data box.
			if(inBox(samplePos) == 0)
				break;
			float d = isoScale * float sample_vol(samplePos);
			float dens = density(d)/numSamples;
			c *= (1-attenuation*dens);
			c += dens*intensity*mix(surfaceColor, interiorColor,
					smoothstep(0.0,surfaceDepth,d));
			totalDens += dens;
			Pcurr += deltaI;
		}
	}
	Ci = Ci*max(1-totalDens,0) + c;
	Oi = Oi + totalDens;
}
