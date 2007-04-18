surface showN()
{
point PP;
normal Nf;
vector V;
color Ct, Ot;
normal NN = normalize(N);
vector posNorm = 0.5*(vector(1,1,1)+NN);
color posCol =
color(comp(posNorm,0),comp(posNorm,1),comp(posNorm,2));
color posGCol =
0.25*color(comp(posNorm,1),comp(posNorm,1),comp(posNorm,1));
Oi = Os;
Ci = posCol;
}

