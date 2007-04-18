/* Simple image-based lighting shader */

light
imagelight(float intensity = 1; 
	color lightcolor = 1; 
	string texturename = "", 
	envspace = "world"; 
	float samples = 64, 
	width = 32, 
	__nondiffuse = 0, 
	__nonspecular = 0;)
{
	solar()

	vector Lt = normalize(vtransform(envspace, -L));

	if (texturename != "")
		Cl = intensity * (lightcolor * color environment(texturename, Lt, "samples", samples, "width", width));
	else
		Cl = 0;
}
