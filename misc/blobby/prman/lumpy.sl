displacement
lumpy( float a=0.5; output varying float height=0 )
{
    float h = a * noise(P);
    P += h * normalize(N);
    N = calculatenormal(N);
    height = h;
}
