surface map(string map = "")
{
  Ci = texture(map, s, t);
/*  printf("s - %f\tt - %f\tduds - %f\tdvdt = %f\nCi - %c\n", s, t, Du(s), Dv(t), Ci);
*/
  Oi = 1;
}        
        
