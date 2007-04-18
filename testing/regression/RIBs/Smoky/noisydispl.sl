

displacement noisydispl(float ampl=1,freq=1)
{
  P = P + ampl*noise(freq*P);
  N = calculatenormal(P);
}
