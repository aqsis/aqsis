surface mxmatch(color c=color(0.2, 0.5, 0.4);)
{
color a = color (0.1, 0.4, 1.0);
color b = color (0.4, 0.3, 1.0);

	color res = mix(a,b,c);

	Ci = color(res);
}
