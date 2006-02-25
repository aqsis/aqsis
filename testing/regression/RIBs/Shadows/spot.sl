// Spot
        
light spot(float intensity = 1.0;
         uniform color lightcolor = color "rgb" (1, 1, 1);
         float falloff = 45.0;
         float hotspot = 43.0;
         float shadow_filter = 4.0;
         float shadow_bias = 0.01;
         float shadow_samples = 32;
         string shadowname = "")
{
  uniform float cosfalloff = cos(radians(0.5*falloff));
  uniform float coshotspot = cos(radians(0.5*hotspot));
  uniform vector axis = normalize(vector "shader" (0,0,1));
  float illuminate_angle = falloff*PI/360.0;
  
  illuminate(point "shader" (0,0,0), axis, illuminate_angle)
  {
    vector L0 = normalize(L);
    float dist = length(L);
    float ca = L0.axis;
    float att = smoothstep(cosfalloff, coshotspot, ca);

    Cl = att * intensity * lightcolor;

    if (shadowname!="")
      Cl *= 1 - shadow(shadowname, Ps, "width", shadow_filter, "bias", shadow_bias, "samples", shadow_samples);
    
  }
}        
        
