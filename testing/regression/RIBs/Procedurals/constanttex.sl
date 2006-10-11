/*
  Apply a texture map without shading.
*/

surface constanttex(string mapname="tmp_tex.tif")
{
  Ci = texture(mapname, s, t);
  Oi = 1;
}
