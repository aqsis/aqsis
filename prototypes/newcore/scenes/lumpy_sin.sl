displacement lumpy_sin()
{
    float amp = 0.05*(sin(10*xcomp(P)) + sin(30*ycomp(P)) + sin(20*zcomp(P)));
    P += amp*normalize(N);
    N = calculatenormal(P);
}
