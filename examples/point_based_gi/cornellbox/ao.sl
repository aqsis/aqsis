// Test of occlusion() shadeop
surface ao(float microbufres = 10;
           string pointCloudName = "")
{
    float occl = 0;
    if(pointCloudName != "")
    {
        normal Nn = normalize(N);
        occl = occlusion(P, Nn, 0,
                         "microbufres", microbufres,
                         "filename", pointCloudName);
    }
    Oi = Os;
    Ci = Oi*(1 - occl);
}

