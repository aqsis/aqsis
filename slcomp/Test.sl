void NestFunc(float zz)
{
	zz=zz+1;
}

void VoidFunc(float param)
{
	float f;
	float t=0;
	f=t;
	param=1;
	NestFunc(param);
}

float TestFunc(float param, param2; point PP;)
{
	float test=1;

	if(param==0)
		return(-test);
	else
		return(test);
}


matrix MatrixFunc(float mx[16])
{
    return matrix (mx[0], mx[1], mx[2], mx[3],
		   mx[4], mx[5], mx[6], mx[7],
		   mx[8], mx[9], mx[10], mx[11],
		   mx[12], mx[13], mx[14], mx[15]);
}


volume
Test(	
	uniform float Ka = 1;
	uniform float Kd = .5;
	uniform float Ks = .5;
	uniform float roughness = .1;
	uniform color specularcolor = 1;
)
{
	// Test variable declarations
	float f2,f3;
	float f1=f2=f3=0;
	point p=point "world" (0,0,0);
	normal n=normal "world" (0,1,0);
	vector vv=vector "shader" (0,0,0);
	color c=color "rgb" (0,0,0);
	matrix m1;
	matrix m2 = 1;
	matrix m3 = matrix(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);

//	matrix "world" m1;
//	matrix m2 = matrix "shader" 1;
//	matrix m3 = matrix "world" (1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);

	// Test function calls.
	f2=cos(Kd);
	f2=clamp(f2,Ka,Kd);

	f2=min(f2,f3,f1);
	point p2=min(p,p,p);
	normal n2=min(n,n,n);
	vector v2=min(vv,vv,vv);
	color c2=min(c,c,c);
	
	// Test variable length arguments nd spline functions.
	f2=spline(0.5, 1,2,3,4,5,6,7,8,9);
	f2=spline("bspline",0.5, 1,2,3,4,5,6,7,8,9);
	p2=spline(0.5, p,p,p,p2,p2,p2);
	p2=spline("bezier",0.5, p,p,p,p2,p2,p2);
	c2=spline(0.5, c,c,c,c2,c2,c2);
	c2=spline("bezier",0.5, c,c,c,c2,c2,c2);
	v2=spline(0.5, vv,vv,vv,v2,v2,v2);
	v2=spline("linear",0.5, vv,vv,vv,v2,v2,v2);

	// Test array spline functions.
	float fa1[9]={1,2,3,4,5,6,7,8,9};
	f2=spline(0.5, fa1);
	f2=spline("bspline",0.5, fa1);
	point pa1[6]={p,p,p,p2,p2,p2};
	p2=spline(0.5, pa1);
	p2=spline("bezier",0.5, pa1);
	color ca1[6]={c,c,c,c2,c2,c2};
	c2=spline(0.5, ca1);
	c2=spline("bezier",0.5, ca1);
	vector va1[6]={vv,vv,vv,v2,v2,v2};
	v2=spline(0.5, va1);
	v2=spline("linear",0.5, va1);
	
	// Test special functions
	N=calculatenormal(P); 
	f2=Du(f3); 
	f2=Dv(f3);
	Du(P);
	Dv(n);
	Du(vv);
	p=Du(P); 
	p=Dv(P); 
	c=Du(Ci); 
	c=Dv(Ci); 
	f2=Deriv(f3,f2); 
	p=Deriv(P,f2); 
	c=Deriv(Ci,f2); 
	// Special case, not handled yet, passing non variable parameter.
	N=calculatenormal(P+(normalize(N)*0.1));

	// Test operator precedence
	f1=2*f2+3*f2;

	// Test function return handling.
	f2=TestFunc(Ka,2,P);
	TestFunc(1,f2,N);
	VoidFunc(f2);


	// Test texture functions
	c=texture("texture.tif", s,t);
	c=texture("texture.tif"[2], s,t);
	c=texture("texture.tif", s,t,0,0,0,0,0,0);
	c=texture("texture.tif"[1], s,t,0,0,0,0,0,0);
	c=texture("texture.tif", s,t,"swidth",1,"twidth",1);
	c=texture("texture.tif", s,t,0,0,0,0,0,0,"swidth",1,"twidth",1);

	f1=texture("texture.tif", s,t);
	f1=texture("texture.tif"[2], s,t);
	f1=texture("texture.tif", s,t,0,0,0,0,0,0);
	f1=texture("texture.tif"[1], s,t,0,0,0,0,0,0);
	f1=texture("texture.tif", s,t,"swidth",1,"twidth",1);
	f1=texture("texture.tif", s,t,0,0,0,0,0,0,"swidth",1,"twidth",1);

	// Test environment functions
	c=environment("environment.tif", P);
	c=environment("environment.tif"[2], P);
	c=environment("environment.tif", P,P,p,p);
	c=environment("environment.tif"[1], P,P,p,p);
	c=environment("environment.tif", P,"swidth",1,"twidth",1);
	c=environment("environment.tif", P,P,p,p,"swidth",1,"twidth",1);

	f1=environment("environment.tif", P);
	f1=environment("environment.tif"[2], P);
	f1=environment("environment.tif", P,P,P,p);
	f1=environment("environment.tif"[1], P,P,p,p);
	f1=environment("environment.tif", P,"swidth",1,"twidth",1);
	f1=environment("environment.tif", P,P,p,p,"swidth",1,"twidth",1);

	// Test bump functions
//	p=bump("bump.tif",N, dPdu,dPdv);
//	p=bump("bump.tif"[2],N, dPdu,dPdv);
//	p=bump("bump.tif",N, dPdu,dPdv, "swidth",1,"twidth",1);
//	p=bump("bump.tif",N, dPdu,dPdv, s,t);
//	p=bump("bump.tif"[2],N, dPdu,dPdv, s,t);
//	p=bump("bump.tif",N, dPdu,dPdv, s,t,0,0,0,0,0,0);
//	p=bump("bump.tif"[1],N, dPdu,dPdv, s,t,0,0,0,0,0,0);
//	p=bump("bump.tif",N, dPdu,dPdv, s,t, "swidth",1,"twidth",1);
//	p=bump("bump.tif",N, dPdu,dPdv, s,t,0,0,0,0,0,0,"swidth",1,"twidth",1);

	f1=filterstep(1,0.5,"width",0.1);
	f1=filterstep(1,0.5,1.5,"width",0.1);

	float af[16];
	af[0]=1; af[1]=0; af[2]=0; af[3]=0;
	af[4]=0; af[5]=1; af[6]=0; af[7]=0;
	af[8]=0; af[9]=0; af[10]=1; af[11]=0;
	af[12]=0; af[13]=0; af[14]=0; af[15]=1;
	matrix m4=MatrixFunc(af);

	attribute("name",f1);
	f1=(f1==0)?p2:f1;
}