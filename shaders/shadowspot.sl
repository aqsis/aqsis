light shadowspot(
    float intensity = 1;
    color lightcolor = 1;
    point from = point "shader" (0, 0, 0);
    point to =   point "shader" (0, 0, 1);
    float coneangle = radians(30);
    float conedeltaangle = radians(5);
    float beamdistribution = 2;
    string shadowname = "";
    float samples = 16;
    float width = 1;)
{
    float atten, cosangle;
    uniform point A = (to - from) / length(to - from);
    uniform float cosoutside = cos(coneangle);
    uniform float cosinside  = cos(coneangle - conedeltaangle);

    illuminate (from, A, coneangle) {
        cosangle = (L . A) / length(L);
        atten = pow(cosangle, beamdistribution) / (L . L);
        atten *= smoothstep(cosoutside, cosinside, cosangle);
        Cl = atten * intensity * lightcolor;
        if (shadowname != "")
            Cl *= 1 - shadow(shadowname, Ps);
    }
}
