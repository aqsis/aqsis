float findme(float m[4])
{
 float idx = 4.0 * random();
 if (idx >= 4.0) idx = 3.0;
 if (idx < 0.0) idx = 0.0;
 return m[ idx ];
}

surface array_cer()
{
  float array[4] = { 1.0, 0.2, 0.3, 0.4};
  array[0] = s;
  array[1] = t;

  float m = findme(array);
  Oi = 1;
  Ci = noise(m);
}
