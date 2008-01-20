// Volume data shader
// Copyright (C) 2008 Christopher J. Foster
//
// This software is licensed under the GPLv2 - see the file COPYING for details.

//------------------------------------------------------------------------------
// A surface shader producing dotted lines along the v-direction.  Useful for
// dotted axes made out of RiCurves.
surface boundingbox_shader(
		float numDashes = 20;
		)
{
	float d = u - 0.5;
	d *= d*2;
	float showLine = step(1,mod(v*(2*numDashes+1)-1,2));
	Ci = Cs*(1 - d*d)*showLine;
	Oi = Os*showLine;
}
