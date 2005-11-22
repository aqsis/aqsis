/*
  Show the sides of a shape.

  The surface on the outside (front facing) is rendered using the outsidecol
  (default: green) and the inside (back facing) uses insidecol (default: red).
*/

surface show_side(color outsidecol=color "rgb" (0,1,0);
		  color insidecol=color "rgb" (1,0,0))
{
  Ci = (I.N<0)? outsidecol : insidecol;
  Oi = 1;
}
