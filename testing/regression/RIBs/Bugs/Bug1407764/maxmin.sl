surface maxmin()
{
float mi = min(u,v,s,t);
color ran[4] = { color(0,1,1), color(1,1,0), color(1,0,1), color(1,1,1)};
Ci = spline(mi, ran);
}

