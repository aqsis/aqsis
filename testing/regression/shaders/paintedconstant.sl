/* paintedconstant
   A constant shader that uses a texture map for the color.
 */

surface paintedconstant(string texturename="";)
{
  Oi = Os;

  if (texturename!="")
       Ci = Os * color texture (texturename);
  else 
       Ci = Os * Cs;
}

