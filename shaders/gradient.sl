/*VSL 1.0.0*************************************************************
 *
 * IMAGER VBgGradient
 *
 * SYNOPSIS: Vertical background gradient between 2 colors
 *
 * AUTHOR: Scott Iverson
 *
 * COPYRIGHT: 2001 SiTex Graphics
 *
 * DESCRIPTION:
 *   This shader produces a gradient background, <b>TopColor</b> at the top of the 
 *   image, gradually changing to <b>BottomColor</b> at the bottom.  
 *   
 *   <b>Sharpness</b> controls the narrowness of the transition range.
 *   
 *   <b>MidPt</b> controls the approximate location of the midway point of the 
 *   transition between colors.
 *
 ***********************************************************************/





/*DESC
  Set global color (Ci) and alpha
  
  Also sets the output opacity to alpha.
DESC*/
void SetCiAlpha_0(
  color ci; 
  float Alpha; 
)
{
  extern color Ci, Oi;
  extern float alpha;
  
  alpha=Alpha; Ci=ci;
  Oi=alpha;
}



void CiAlpha_1(
  output color ci; 
  output float Alpha; 
)
{
  extern color Ci;
  extern float alpha;
  
  ci=Ci;
  Alpha=alpha;
}



/*DESC
  global point variable P (point being shaded)
DESC*/
void P_2(
  output point P_global; 
)
{
  extern point P;
  
  P_global=P;
}



/*DESC
  transforms the input point and multiplies by frequency
DESC*/
void TransformPt_3(
  point p; 
  string fromspace; 
  string tospace; 
  float frequency; 
  output point pt; 
  output float dpt; 
)
{
#if !defined(AQSIS)
  pt=transform(fromspace,tospace,p)*frequency;
  dpt=max(sqrt(area(pt)),1.0e-6);
#else
float lenx, leny;
float resolution[3];
  option("Format", resolution); 
  lenx = frequency * xcomp(p) / resolution[0]; 
  leny = frequency * ycomp(p) / resolution[1]; 
  setxcomp(pt, lenx); 
  setycomp(pt, leny); 
  setzcomp(pt, 0.0); 
  dpt = length(pt);
#endif
}



/*DESC
  Outputs the x, y, and z components of a point
DESC*/
void XYZ_4(
  point p; 
  output float x; 
  output float y; 
  output float z; 
)
{
  x=xcomp(p);
  y=ycomp(p);
  z=zcomp(p);
}



void Subtract_5(
  float A; 
  float B; 
  output float Diff; 
)
{
  Diff=A-B;
}



/*DESC
  Product of colors A and B
DESC*/
void Multiply_6(
  color A; 
  color B; 
  output color AB; 
)
{
  AB=A*B;
}



/*DESC
  Sum of colors A and B
DESC*/
void Add_7(
  color A; 
  color B; 
  output color Sum; 
)
{
  Sum=A+B;
}



void Gain_9(
  float sharpness; 
  float x; 
  output float f; 
)
{
  f=mix(x,smoothstep(sharpness*.495,1-sharpness*.495,x),sharpness);
}



void MixColors_10(
  color Color0; 
  color Color1; 
  float f; 
  output color Mix; 
)
{
  Mix=mix(Color0,Color1,f);
}



/*DESC
  Applies gamma correction to Craw using the gain and gamma set by the 
  Exposure for the scene.
  
  This function is useful in imager shaders, because the output of an imager 
  shader is not gamma-corrected.
DESC*/
void GammaCorrect_12(
  color Craw; 
  output color Cgc; 
)
{
  uniform float gg[2];
  
  gg[0]=1.0; 
  gg[1]=1.0;
  Cgc=Craw;
  
  if (option("Exposure",gg)==1) {
    setcomp(Cgc,0,pow(comp(Craw,0)*gg[0],1.0/gg[1]));
    setcomp(Cgc,1,pow(comp(Craw,1)*gg[0],1.0/gg[1]));
    setcomp(Cgc,2,pow(comp(Craw,2)*gg[0],1.0/gg[1]));
  }
}




/*DESC
  Bias function.
  x and bias are assumed to lie in the unit interval 0..1
  
  If bias<0.5, x is shifted toward the low end of the spectrum.
  If bias>0.5, x is shifted toward the high end of the spectrum.
DESC*/
void Bias_14(
  float x; 
  float bias; 
  output float f; 
)
{
  f=pow(x,log(clamp(bias,0.001,0.999))/log(0.5));
  
}



void Subtract_15(
  float A; 
  float B; 
  output float Diff; 
)
{
  Diff=A-B;
}

imager gradient (
  color TopColor=color(0.0,0.0,0.8);
  color BottomColor=color(0.023,0.260,0.090);
  float Sharpness=0;
  float MidPoint=0.5;
)
 {
  color ci_1;
  float Alpha_1;
  point P_global_2;
  point pt_3;
  float dpt_3;
  float x_4;
  float y_4;
  float z_4;
  float Diff_5;
  color AB_6;
  color Sum_7;
  float f_9;
  color Mix_10;
  color Cgc_12;
  float f_14;
  float Diff_15;

  CiAlpha_1(ci_1,Alpha_1);
  P_2(P_global_2);
  TransformPt_3(P_global_2,"raster","NDC",1,pt_3,dpt_3);
  XYZ_4(pt_3,x_4,y_4,z_4);
  Subtract_5(1,Alpha_1,Diff_5);
  Subtract_15(1,MidPoint,Diff_15);
  Bias_14(y_4,Diff_15,f_14);
  Gain_9(Sharpness,f_14,f_9);
  MixColors_10(TopColor,BottomColor,f_9,Mix_10);
  GammaCorrect_12(Mix_10,Cgc_12);
  /* Multiply_6(Diff_5,Cgc_12,AB_6); */
  AB_6 = Diff_5 * Cgc_12;
  Add_7(ci_1,AB_6,Sum_7);
  SetCiAlpha_0(Sum_7,1);
 }
