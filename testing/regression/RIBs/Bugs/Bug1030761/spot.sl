// Spot light
        
light spot(float falloff = 45.0;
	   float width = 4.0;
	   float samples = 64;
	   string shadowname = "")
{
  float illuminate_angle = falloff*PI/360.0;
  uniform vector axis = normalize(vector "shader" (0,0,1));
  
  illuminate(point "shader" (0,0,0), axis, illuminate_angle)
  {
    Cl = 1;

    if (shadowname!="")
      Cl *= 1 - shadow(shadowname, Ps, "width", width, "samples", samples);
  }
}        
        
