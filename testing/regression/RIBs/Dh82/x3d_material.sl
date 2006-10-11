/*
 * x3d_material - emulation of 3d Exploration material
 *
 * AUTHOR: Scott Iverson
 *
 * COPYRIGHT: 2001 SiTex Graphics
 */

surface x3d_material(
  color ambientcolor=color(1.0, 1.0, 1.0);
  color emissioncolor=color(0.0, 0.0, 0.0);
  color specularcolor=color(1.0, 1.0, 1.0);
  float roughness=0.1;
  float reflectivity=0.0;
  string diffusemap="";
  string shinemap="";
  string bumpmap="";
  string specularmap="";
  string opacitymap="";
  string reflectionmap="";
  float modulatebump=1.0;
  float modulatediffuse=0.0;)
{
  float shine=1.0;
  Oi=Os;
  color Cdiff=Cs;
  color Ct=color(1.0, 1.0, 1.0);

  normal NN=N;
  if (bumpmap!="") {
    float bumpM=float texture(bumpmap);
    bumpM = bumpM * modulatebump;
    point PP=P+normalize(N)*bumpM;
    NN=calculatenormal(PP);
  }
  normal Nf=faceforward(normalize(NN),I);
  if (diffusemap!="") {
    Ct=texture(diffusemap);
    if (modulatediffuse==0.0) Cdiff=color(1.0, 1.0, 1.0);
  }

  Oi=Os;
  Ci=Oi*(emissioncolor+(ambientcolor*ambient()+Cdiff * diffuse(Nf))*Ct+
     specularcolor*(shine*specular(Nf,-normalize(I),roughness)));

}


