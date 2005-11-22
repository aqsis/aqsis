/*
  This shader is used to test various SL functions.
  You select a function with the func parameter. The selected function
  is evaluated at the given position (x). The result is scaled by yscale
  and used to displace an object (a cylinder which will then show the
  function graph).

  (Matthias Baas, baas@ira.uka.de)
*/

displacement funcs(varying float x=0.0; 
		   uniform float yscale=1.0;
		   uniform float func=0;)
{
  vector ydir = vector "world" (0,1,0);
  float y;

  if (func==0)      y = x;         /* identity function */
  else if (func==1) y = sin(x);
  else if (func==2) y = cos(x);
  else if (func==3) y = tan(x);
  else if (func==4) y = ceil(x);
  else if (func==5) y = sqrt(x);
  else if (func==6) y = mod(x,2);
  else if (func==7) y = floor(x);
  else if (func==8) y = abs(x);
  else if (func==9) y = inversesqrt(x);
  else if (func==10) y = exp(x);
  else if (func==11) y = log(x);
  else if (func==12) y = round(x);
  else if (func==13) y = asin(x);
  else if (func==14) y = acos(x);
  else if (func==15) y = atan(x);
  else if (func==16) y = sign(x);
  else if (func==17) y = pow(x,2);
  else if (func==18) y = max(x,-1.5);
  else if (func==19) y = smoothstep(-1,1,x);
  else if (func==20) y = min(x,1.5);
  else if (func==21) y = noise(x);
  else if (func==22) y = step(1.5, x);
  else if (func==23) y = clamp(x, -1.5, 1.5);
  else if (func==24) y = pnoise(x, 2);
  else if (func==25) y = cellnoise(x);
  else if (func==26) y = step(-1.5, x);
  else if (func==27) y = spline((x+4)/8, 2,2,2,-2,-2,-2);
  else if (func==28) y = filterstep(1.5,x);
  else if (func==29) y = spline((x+4)/8, 2,2,-2,-2);
  else if (func==30) y = mix(-1,1,x);
  else if (func==31) y = degrees(x*PI)/360;
  else if (func==32) y = random();
  else if (func==33) y = PI/2;
  else if (func==34) y = radians(x*180)/(2*PI);
  else if (func==35) y = Deriv(x*x, x);
  else if (func==36) y = Deriv(2*x, x);
  else if (func==37) y = 8*(pnoise(x+3, 3.0)-0.5)+0.5;

  P = P + y*yscale*ydir;
  N = calculatenormal (P);  
}
