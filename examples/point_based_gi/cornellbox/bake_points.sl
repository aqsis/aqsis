surface bake_points(color Cemit = 0;
                    string pointCloudName = "")
{
    normal Nn = normalize(N);
    color col = Cemit + Cs*(diffuse(Nn) + ambient());
    if(pointCloudName != "")
    {
        bake3d(pointCloudName, "", P, Nn,
               "_area", area(P),
               "_radiosity", col,
               "interpolate", 1);
    }
    Oi = Os;
    Ci = col*Oi;
}
