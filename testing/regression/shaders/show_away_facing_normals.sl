surface
show_away_facing_normals()
{
  // Show normals facing away in a light blue.
  Ci = Cs;
  if( N.P > 0 )
  {
	Ci = color(0.8,0.8,1);
  }
  Oi = 1;
}
