surface indirect(float Kd = 1;
                 float Ka = 1;
                 float Ki = 0.5;
                 float Ke = 0.5;
                 color Cemit = 0;
                 float microbufres = 10;
                 float maxsolidangle = 0.03;
                 string pointCloudName = "")
{
    normal Nn = normalize(N);
    color indirect = 0;
    if(pointCloudName != "")
    {
        indirect = indirectdiffuse(P, Nn, 0,
                                   "filename", pointCloudName,
                                   "maxsolidangle", maxsolidangle,
                                   "microbufres", microbufres);
    }
    Oi = Os;
    Ci = Oi*(Ke*Cemit + Cs*(Kd*diffuse(Nn) + Ka*ambient() + Ki*indirect));
}
