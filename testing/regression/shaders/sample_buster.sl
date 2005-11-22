/*******************************************
** sample_buster RenderMan imager shader
**
** By Tim Shead, 2004
**
** This shader is useful for identifying sampling
** errors and/or cracks in the output of a RenderMan
** render engine, by forcing the value of a pixel
** to a given color if the pixel alpha is less
** than one.  Render your geometry single-sided with
** the constant shader and a constrasting color,
** and sample_buster will force sampling errors
** and cracks to stand-out.
*/

imager sample_buster(color bgcolor = 1)
{
	if(alpha < 1)
		Ci = bgcolor;
	Oi = 1;
	alpha = 1;
}


