surface showuser (varying color myvcolor=0)
{
	normal Nf = normalize(Ng);

	Oi = Os;
    Ci = Os * myvcolor * (diffuse(Nf));
}
