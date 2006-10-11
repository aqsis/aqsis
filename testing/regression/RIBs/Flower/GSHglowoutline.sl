surface GSHglowoutline(float glow = 0.01)
{
	float dp = (normalize (-N) . normalize(I));
	
	Oi = Os + (1-Os)*smoothstep(20,23,zcomp(P));
	Ci = Os*Cs + 0.32*smoothstep(-1,0.3,dp);
}
