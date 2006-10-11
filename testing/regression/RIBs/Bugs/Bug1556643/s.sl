color fun ()
{
uniform matrix m = 1;
textureinfo("foo2", "viewingmatrix", m);
float z =
texture("foo",0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5);

return color(1, 1, 1);
}

surface s ()
{
normal Nn = (1, 1, 1);
normal Nf = faceforward(Nn, I, Nn);

illuminance("foo1",P, Nf, PI/2 )
{
color c = fun();
}
}

