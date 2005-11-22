/*
  Apply a texture map without shading.
*/

surface constanttex(string mapname="")
{
  Ci = texture(mapname, s, t);
  Oi = 1;
}
