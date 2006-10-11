
displacement GSHsinedisp( float amplitude = 0.002, wavelength = 0.0025 )
{
#if defined(STYLE1)
        float amp = amplitude * pow(noise( u / wavelength, v / wavelength),6) 
	 	+ amplitude*sin((v/(wavelength*40)) * 2 * PI);
#else
	float uc = u + 0.1*noise(u*10,v*10);
	float vc = v + 0.1*noise(u*11,v*9);
	float amp = amplitude*sin((uc/(wavelength*10.0)) * 2 * PI) 
 		+ amplitude*sin((vc/(wavelength*10.0)) * 2 * PI);
#endif
	
 	P += amp * N;

 	N = calculatenormal(P);
}
