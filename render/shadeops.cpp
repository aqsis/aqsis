// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.com
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
// 
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
// 
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


/** \file
		\brief Implements the basic shader operations.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include	<math.h>

#include	"aqsis.h"
#include	"lights.h"
#include	"shadervm.h"
#include	"shaderexecenv.h"
#include	"texturemap.h"
#include	"spline.h"
#include	"surface.h"

START_NAMESPACE(Aqsis)

//----------------------------------------------------------------------
// SO_sprintf
// Helper function to process a string inserting variable, used in printf and format.

static	CqString	SO_sprintf(const char* str, int cParams, CqVMStackEntry** apParams, int varyingindex)
{
	CqString strRes("");
	CqString strTrans=str; //str.TranslateEqcapes();

	int i=0;
	int ivar=0;
	while(i<strTrans.size())
	{
		switch(strTrans[i])
		{
			case '%':	// Insert variable.
			{
				i++;
				switch(strTrans[i])
				{
					case 'f':
					{
						TqFloat f=apParams[ivar++]->Value(f,varyingindex);
						strRes+=f;
					}
					break;

					case 'p':
					{
						CqVector3D vec=apParams[ivar++]->Value(vec,varyingindex);
						strRes+=vec.x();	strRes+=",";
						strRes+=vec.y();	strRes+=",";
						strRes+=vec.z();
					}
					break;

					case 'c':
					{
						CqColor col=apParams[ivar++]->Value(col,varyingindex);
						strRes+=col.fRed();		strRes+=",";
						strRes+=col.fGreen();	strRes+=",";
						strRes+=col.fBlue();
					}
					break;

					case 'm':
					{
						CqMatrix mat=apParams[ivar++]->Value(mat,varyingindex);
						strRes+=mat.Element(0,0);	strRes+=","; strRes+=mat.Element(0,1);	strRes+=","; strRes+=mat.Element(0,2);	strRes+=","; strRes+=mat.Element(0,3);	strRes+=",";
						strRes+=mat.Element(1,0);	strRes+=","; strRes+=mat.Element(1,1);	strRes+=","; strRes+=mat.Element(1,2);	strRes+=","; strRes+=mat.Element(1,3);	strRes+=",";
						strRes+=mat.Element(2,0);	strRes+=","; strRes+=mat.Element(2,1);	strRes+=","; strRes+=mat.Element(2,2);	strRes+=","; strRes+=mat.Element(2,3);	strRes+=",";
						strRes+=mat.Element(3,0);	strRes+=","; strRes+=mat.Element(3,1);	strRes+=","; strRes+=mat.Element(3,2);	strRes+=","; strRes+=mat.Element(3,3);	strRes+=",";
					}
					break;

					case 's':
					{
						CqString stra=apParams[ivar++]->Value(stra,varyingindex);
						strRes+=stra;
					}
					break;

					default:
					{
						strRes+=strTrans[i];
					}
					break;
				}
				i++;
			}
			break;

			default:
			{
				strRes+=strTrans[i];
				i++;
			}
			break;
		}
	}
	return(strRes);
}

//----------------------------------------------------------------------
// init_illuminance()
TqBool CqShaderExecEnv::SO_init_illuminance()
{
	m_li=-1;
	return(SO_advance_illuminance());
}


//----------------------------------------------------------------------
// advance_illuminance()
TqBool CqShaderExecEnv::SO_advance_illuminance()
{
	m_li++;
	while(m_li<m_pSurface->pAttributes()->apLights().size() && 
	   m_pSurface->pAttributes()->apLights()[m_li]->pShader()->fAmbient())
	{
		m_li++;
	}
	if(m_li<m_pSurface->pAttributes()->apLights().size())	return(TqTrue);
	else														return(TqFalse);
}


void CqShaderExecEnv::ValidateIlluminanceCache(CqVMStackEntry& P, CqShader* pShader)
{
	// If this is the first call to illuminance this time round, call all lights and setup the Cl and L caches.
	if(!m_IlluminanceCacheValid)
	{
		// Fist cache the shaderexecenv state for later retrieval.
		TqInt	cacheGridI=m_GridI;

		TqInt li=0;
		while(li<m_pSurface->pAttributes()->apLights().size())
		{
			CqLightsource* lp=m_pSurface->pAttributes()->apLights()[li];
			// Initialise the lightsource
			lp->Initialise(uGridRes(), vGridRes());
			m_Illuminate=0;
			// Evaluate the lightsource
			lp->Evaluate(P);
			li++;
		}
		m_IlluminanceCacheValid=TqTrue;;

		// Restore the state prior to executing the lightsources.
		m_GridI=cacheGridI;
	}
}


STD_SOIMPL	CqShaderExecEnv::SO_radians(FLOATVAL degrees, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(degrees)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,FLOAT(degrees)/(180.0f/RI_PI));
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_degrees(FLOATVAL radians, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(radians)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,(FLOAT(radians)/180.0f)*RI_PI);
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_sin(FLOATVAL a, DEFPARAMIMPL)					
{
	INIT_SOR
	CHECKVARY(a)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,static_cast<TqFloat>(sin(FLOAT(a))));
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_asin(FLOATVAL a, DEFPARAMIMPL)					
{
	INIT_SOR
	CHECKVARY(a)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,static_cast<TqFloat>(asin(FLOAT(a))));
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_cos(FLOATVAL a, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(a)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,static_cast<TqFloat>(cos(FLOAT(a))));
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_acos(FLOATVAL a, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(a)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,static_cast<TqFloat>(acos(FLOAT(a))));
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_tan(FLOATVAL a, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(a)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,static_cast<TqFloat>(tan(FLOAT(a))));
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_atan(FLOATVAL yoverx, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(yoverx)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,static_cast<TqFloat>(atan(FLOAT(yoverx))));
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_atan(FLOATVAL y, FLOATVAL x, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(x)
	CHECKVARY(y)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,static_cast<TqFloat>(atan2(FLOAT(y),FLOAT(x))));
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_pow(FLOATVAL x, FLOATVAL y, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(x)
	CHECKVARY(y)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,static_cast<TqFloat>(pow(FLOAT(x),FLOAT(y))));
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_exp(FLOATVAL x, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(x)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,static_cast<TqFloat>(exp(FLOAT(x))));
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_sqrt(FLOATVAL x, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(x)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,static_cast<TqFloat>(sqrt(FLOAT(x))));
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_log(FLOATVAL x, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(x)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,static_cast<TqFloat>(log(FLOAT(x))));
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_mod(FLOATVAL a, FLOATVAL b, DEFPARAMIMPL)		
{
	INIT_SOR
	CHECKVARY(a)
	CHECKVARY(b)
	CHECKVARY(Result)
	FOR_EACHR
		TqInt n=static_cast<TqInt>(FLOAT(a)/FLOAT(b));
		TqFloat a2=FLOAT(a)-n*FLOAT(b);
		if(a2<0.0f)
			a2+=FLOAT(b);
		Result.SetValue(i,a2);
	END_FORR
}	

//----------------------------------------------------------------------
// log(x,base)
STD_SOIMPL	CqShaderExecEnv::SO_log(FLOATVAL x, FLOATVAL base, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(x)
	CHECKVARY(base)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,static_cast<TqFloat>(log(FLOAT(x))/log(FLOAT(base))));
	END_FORR
}


STD_SOIMPL	CqShaderExecEnv::SO_abs(FLOATVAL x, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(x)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,static_cast<TqFloat>(fabs(FLOAT(x))));
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_sign(FLOATVAL x, DEFPARAMIMPL)					
{
	INIT_SOR
	CHECKVARY(x)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,(FLOAT(x)<0.0f)?-1.0f:1.0f);
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_min(FLOATVAL a, FLOATVAL b, DEFPARAMVARIMPL)		
{
	INIT_SOR
	CHECKVARY(a)
	CHECKVARY(b)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,MIN(FLOAT(a),FLOAT(b)));
		while(cParams-->0)
			Result.SetValue(i,MIN(FLOAT(Result),FLOAT(*apParams[cParams])));
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_max(FLOATVAL a, FLOATVAL b, DEFPARAMVARIMPL)		
{
	INIT_SOR
	CHECKVARY(a)
	CHECKVARY(b)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,MAX(FLOAT(a),FLOAT(b)));
		while(cParams-->0)
			Result.SetValue(i,MAX(FLOAT(Result),FLOAT(*apParams[cParams])));
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_pmin(POINTVAL a, POINTVAL b, DEFPARAMVARIMPL)		
{
	INIT_SOR
	CHECKVARY(a)
	CHECKVARY(b)
	CHECKVARY(Result)
	FOR_EACHR
		const CqVector3D& vA=POINT(a);
		const CqVector3D& vB=POINT(b);
		CqVector3D res=CqVector3D(MIN(vA.x(),vB.x()),MIN(vA.y(),vB.y()),MIN(vA.z(),vB.z()));
		while(cParams-->0)
		{
			const CqVector3D& vN=POINT(*apParams[cParams]);
			res=CqVector3D(MIN(res.x(),vN.x()),MIN(res.y(),vN.y()),MIN(res.z(),vN.z()));
		}
		Result.SetValue(i,res);
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_pmax(POINTVAL a, POINTVAL b, DEFPARAMVARIMPL)		
{
	INIT_SOR
	CHECKVARY(a)
	CHECKVARY(b)
	CHECKVARY(Result)
	FOR_EACHR
		const CqVector3D& vA=POINT(a);
		const CqVector3D& vB=POINT(b);
		CqVector3D res=CqVector3D(MAX(vA.x(),vB.x()),MAX(vA.y(),vB.y()),MAX(vA.z(),vB.z()));
		while(cParams-->0)
		{
			const CqVector3D& vN=POINT(*apParams[cParams]);
			res=CqVector3D(MAX(res.x(),vN.x()),MAX(res.y(),vN.y()),MAX(res.z(),vN.z()));
		}
		Result.SetValue(i,res);
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_cmin(COLORVAL a, COLORVAL b, DEFPARAMVARIMPL)		
{
	INIT_SOR
	CHECKVARY(a)
	CHECKVARY(b)
	CHECKVARY(Result)
	FOR_EACHR
		const CqColor& cA=COLOR(a);
		const CqColor& cB=COLOR(b);
		CqColor res=CqColor(MIN(cA.fRed(),cB.fRed()),MIN(cA.fGreen(),cB.fGreen()),MIN(cA.fBlue(),cB.fBlue()));
		while(cParams-->0)
		{
			const CqColor& cN=COLOR(*apParams[cParams]);
			res=CqColor(MIN(res.fRed(),cN.fRed()),MIN(res.fGreen(),cN.fGreen()),MIN(res.fBlue(),cN.fBlue()));
		}
		Result.SetValue(i,res);
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_cmax(COLORVAL a, COLORVAL b, DEFPARAMVARIMPL)		
{
	INIT_SOR
	CHECKVARY(a)
	CHECKVARY(b)
	CHECKVARY(Result)
	FOR_EACHR
		const CqColor& cA=COLOR(a);
		const CqColor& cB=COLOR(b);
		CqColor res=CqColor(MAX(cA.fRed(),cB.fRed()),MAX(cA.fGreen(),cB.fGreen()),MAX(cA.fBlue(),cB.fBlue()));
		while(cParams-->0)
		{
			const CqColor& cN=COLOR(*apParams[cParams]);
			res=CqColor(MAX(res.fRed(),cN.fRed()),MAX(res.fGreen(),cN.fGreen()),MAX(res.fBlue(),cN.fBlue()));
		}
		Result.SetValue(i,res);
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_clamp(FLOATVAL a, FLOATVAL min, FLOATVAL max, DEFPARAMIMPL)	
{
	INIT_SOR
	CHECKVARY(a)
	CHECKVARY(min)
	CHECKVARY(max)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,CLAMP(FLOAT(a),FLOAT(min),FLOAT(max)));
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_pclamp(POINTVAL a, POINTVAL min, POINTVAL max, DEFPARAMIMPL)	
{
	INIT_SOR
	CHECKVARY(a)
	CHECKVARY(min)
	CHECKVARY(max)
	CHECKVARY(Result)
	FOR_EACHR
		const CqVector3D& vA=POINT(a);
		const CqVector3D& vMin=POINT(min);
		const CqVector3D& vMax=POINT(max);
		Result.SetValue(i,CqVector3D(CLAMP(vA.x(),vMin.x(),vMax.x()),
									 CLAMP(vA.y(),vMin.y(),vMax.y()),
									 CLAMP(vA.z(),vMin.z(),vMax.z())));
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_cclamp(COLORVAL a, COLORVAL min, COLORVAL max, DEFPARAMIMPL)	
{
	INIT_SOR
	CHECKVARY(a)
	CHECKVARY(min)
	CHECKVARY(max)
	CHECKVARY(Result)
	FOR_EACHR
		const CqColor& cA=COLOR(a);
		const CqColor& cMin=COLOR(min);
		const CqColor& cMax=COLOR(max);
		Result.SetValue(i,CqColor(CLAMP(cA.fRed(),cMin.fRed(),cMax.fRed()),
								   CLAMP(cA.fGreen(),cMin.fGreen(),cMax.fGreen()),
								   CLAMP(cA.fBlue(),cMin.fBlue(),cMax.fBlue())));
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_floor(FLOATVAL x, DEFPARAMIMPL)				
{
	INIT_SOR
	CHECKVARY(x)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,static_cast<TqFloat>(floor(FLOAT(x))));
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_ceil(FLOATVAL x, DEFPARAMIMPL)					
{
	INIT_SOR
	CHECKVARY(x)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,static_cast<TqFloat>(ceil(FLOAT(x))));
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_round(FLOATVAL x, DEFPARAMIMPL)				
{
	double v;
	INIT_SOR
	CHECKVARY(x)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,(modf(FLOAT(x),&v)>0.5f)?static_cast<TqFloat>(v)+1.0f:static_cast<TqFloat>(v));
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_step(FLOATVAL min, FLOATVAL value, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(min)
	CHECKVARY(value)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,(FLOAT(value)<FLOAT(min))?0.0f:1.0f);
	END_FORR
}


//----------------------------------------------------------------------
// smoothstep(min,max,value)
STD_SOIMPL	CqShaderExecEnv::SO_smoothstep(FLOATVAL min, FLOATVAL max, FLOATVAL value, DEFPARAMIMPL)
{
	// TODO: Check this against Eberts code
	INIT_SOR
	CHECKVARY(value)
	CHECKVARY(min)
	CHECKVARY(max)
	CHECKVARY(Result)
	FOR_EACHR
		if(FLOAT(value)<FLOAT(min))
			Result.SetValue(i,0.0f);
		else if(FLOAT(value)>=FLOAT(max))
			Result.SetValue(i,1.0f);
		else
		{
			TqFloat v=(FLOAT(value)-FLOAT(min))/(FLOAT(max)-FLOAT(min));
			Result.SetValue(i,v*v*(3.0f-2.0f*v));
		}
	END_FORR
}


//----------------------------------------------------------------------
// spline(value, f1,f2,...,fn)
STD_SOIMPL	CqShaderExecEnv::SO_fspline(FLOATVAL value, DEFPARAMVARIMPL)
{
	INIT_SOR
	CHECKVARY(value)
	TqInt v;
	for(v=0; v<cParams; v++)	CHECKVARY((*apParams[v]))
	CHECKVARY(Result)

	CqSplineCubic spline(cParams);

	FOR_EACHR
		if(FLOAT(value)>=1.0f)	Result.SetValue(i,FLOAT((*apParams[cParams-2])));
		else if(FLOAT(value)<=0.0f)	Result.SetValue(i,FLOAT((*apParams[1])));
		else
		{
			TqInt j;
			for(j=0; j<cParams; j++)
				spline[j]=CqVector4D(FLOAT((*apParams[j])),0.0f,0.0f,1.0f);
			
			CqVector4D	res=spline.Evaluate(FLOAT(value));
			Result.SetValue(i,res.x());
		}
	END_FORR
}


//----------------------------------------------------------------------
// spline(value, f1,f2,...,fn)
STD_SOIMPL	CqShaderExecEnv::SO_cspline(FLOATVAL value, DEFPARAMVARIMPL)
{
	INIT_SOR
	CHECKVARY(value)
	TqInt v;
	for(v=0; v<cParams; v++)	CHECKVARY((*apParams[v]))
	CHECKVARY(Result)

	CqSplineCubic spline(cParams);

	FOR_EACHR
		if(FLOAT(value)>=1.0f)	Result.SetValue(i,COLOR((*apParams[cParams-2])));
		else if(FLOAT(value)<=0.0f)	Result.SetValue(i,COLOR((*apParams[1])));
		else
		{
			TqInt j;
			for(j=0; j<cParams; j++)
				spline[j]=CqVector4D(COLOR((*apParams[j])).fRed(),COLOR((*apParams[j])).fGreen(),COLOR((*apParams[j])).fBlue(),1.0f);
			
			CqVector4D	res=spline.Evaluate(FLOAT(value));
			Result.SetValue(i,CqColor(res.x(),res.y(),res.z()));
		}
	END_FORR
}


//----------------------------------------------------------------------
// spline(value, f1,f2,...,fn)
STD_SOIMPL	CqShaderExecEnv::SO_pspline(FLOATVAL value, DEFPARAMVARIMPL)
{
	INIT_SOR
	CHECKVARY(value)
	TqInt v;
	for(v=0; v<cParams; v++)	CHECKVARY((*apParams[v]))
	CHECKVARY(Result)

	CqSplineCubic spline(cParams);

	FOR_EACHR
		if(FLOAT(value)>=1.0f)	Result.SetValue(i,POINT((*apParams[cParams-2])));
		else if(FLOAT(value)<=0.0f)	Result.SetValue(i,POINT((*apParams[1])));
		else
		{
			TqInt j;
			for(j=0; j<cParams; j++)
				spline[j]=POINT((*apParams[j]));
			
			CqVector3D	res=spline.Evaluate(FLOAT(value));
			Result.SetValue(i,res);
		}
	END_FORR
}


//----------------------------------------------------------------------
// spline(value, f1,f2,...,fn)
STD_SOIMPL	CqShaderExecEnv::SO_sfspline(STRINGVAL basis, FLOATVAL value, DEFPARAMVARIMPL)
{
	INIT_SOR
	CHECKVARY(value)
	TqInt v;
	for(v=0; v<cParams; v++)	CHECKVARY((*apParams[v]))
	CHECKVARY(Result)

	CqSplineCubic spline(cParams);
	spline.SetmatBasis(basis.Value(temp_string,0));

	FOR_EACHR
		if(FLOAT(value)>=1.0f)	Result.SetValue(i,FLOAT((*apParams[cParams-2])));
		else if(FLOAT(value)<=0.0f)	Result.SetValue(i,FLOAT((*apParams[1])));
		else
		{
			TqInt j;
			for(j=0; j<cParams; j++)
				spline[j]=CqVector4D(FLOAT((*apParams[j])),0.0f,0.0f,1.0f);
			
			CqVector4D	res=spline.Evaluate(FLOAT(value));
			Result.SetValue(i,res.x());
		}
	END_FORR
}


//----------------------------------------------------------------------
// spline(value, f1,f2,...,fn)
STD_SOIMPL	CqShaderExecEnv::SO_scspline(STRINGVAL basis, FLOATVAL value, DEFPARAMVARIMPL)
{
	INIT_SOR
	CHECKVARY(value)
	TqInt v;
	for(v=0; v<cParams; v++)	CHECKVARY((*apParams[v]))
	CHECKVARY(Result)

	CqSplineCubic spline(cParams);
	spline.SetmatBasis(basis.Value(temp_string,0));

	FOR_EACHR
		if(FLOAT(value)>=1.0f)	Result.SetValue(i,COLOR((*apParams[cParams-2])));
		else if(FLOAT(value)<=0.0f)	Result.SetValue(i,COLOR((*apParams[1])));
		else
		{
			TqInt j;
			for(j=0; j<cParams; j++)
				spline[j]=CqVector4D(COLOR((*apParams[j])).fRed(),COLOR((*apParams[j])).fGreen(),COLOR((*apParams[j])).fBlue(),1.0f);
			
			CqVector4D	res=spline.Evaluate(FLOAT(value));
			Result.SetValue(i,CqColor(res.x(),res.y(),res.z()));
		}
	END_FORR
}


//----------------------------------------------------------------------
// spline(value, f1,f2,...,fn)
STD_SOIMPL	CqShaderExecEnv::SO_spspline(STRINGVAL basis, FLOATVAL value, DEFPARAMVARIMPL)
{
	INIT_SOR
	CHECKVARY(value)
	TqInt v;
	for(v=0; v<cParams; v++)	CHECKVARY((*apParams[v]))
	CHECKVARY(Result)

	CqSplineCubic spline(cParams);
	spline.SetmatBasis(basis.Value(temp_string,0));

	FOR_EACHR
		if(FLOAT(value)>=1.0f)	Result.SetValue(i,POINT((*apParams[cParams-2])));
		else if(FLOAT(value)<=0.0f)	Result.SetValue(i,POINT((*apParams[1])));
		else
		{
			TqInt j;
			for(j=0; j<cParams; j++)
				spline[j]=POINT((*apParams[j]));
			
			CqVector3D	res=spline.Evaluate(FLOAT(value));
			Result.SetValue(i,res);
		}
	END_FORR
}


STD_SOIMPL	CqShaderExecEnv::SO_fDu(FLOATVAL p, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(p)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,SO_DuType<TqFloat>(p,i,*this));
	END_FORR
}


STD_SOIMPL	CqShaderExecEnv::SO_fDv(FLOATVAL p, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(p)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,SO_DvType<TqFloat>(p,i,*this));
	END_FORR
}


STD_SOIMPL	CqShaderExecEnv::SO_fDeriv(FLOATVAL p, FLOATVAL den, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(p)
	CHECKVARY(den)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,SO_DerivType<TqFloat>(p,den,i,*this));
	END_FORR
}


STD_SOIMPL	CqShaderExecEnv::SO_cDu(COLORVAL p, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(p)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,SO_DuType<CqColor>(p,i,*this));
	END_FORR
}


STD_SOIMPL	CqShaderExecEnv::SO_cDv(COLORVAL p, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(p)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,SO_DvType<CqColor>(p,i,*this));
	END_FORR
}


STD_SOIMPL	CqShaderExecEnv::SO_cDeriv(COLORVAL p, FLOATVAL den, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(p)
	CHECKVARY(den)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,SO_DerivType<CqColor>(p,den,i,*this));
	END_FORR
}


STD_SOIMPL	CqShaderExecEnv::SO_pDu(POINTVAL p, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(p)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,SO_DuType<CqVector3D>(p,i,*this));
	END_FORR
}


STD_SOIMPL	CqShaderExecEnv::SO_pDv(POINTVAL p, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(p)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,SO_DvType<CqVector3D>(p,i,*this));
	END_FORR
}


STD_SOIMPL	CqShaderExecEnv::SO_pDeriv(POINTVAL p, FLOATVAL den, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(p)
	CHECKVARY(den)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,SO_DerivType<CqVector3D>(p,den,i,*this));
	END_FORR
}


STD_SOIMPL	CqShaderExecEnv::SO_frandom(DEFPARAMIMPL)
{
	static CqRandom randomiser;

	INIT_SOR
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,m_random.RandomFloat());
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_crandom(DEFPARAMIMPL)							
{
	INIT_SOR
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,CqColor(m_random.RandomFloat(),m_random.RandomFloat(),m_random.RandomFloat()));
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_prandom(DEFPARAMIMPL)							
{
	INIT_SOR
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,CqVector3D(m_random.RandomFloat(),m_random.RandomFloat(),m_random.RandomFloat()));
	END_FORR
}


//----------------------------------------------------------------------
// noise(v)
STD_SOIMPL	CqShaderExecEnv::SO_fnoise1(FLOATVAL v, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(v)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,(m_noise.FGNoise1(FLOAT(v))+1)/2.0f);
	END_FORR
}

//----------------------------------------------------------------------
// noise(u,v)
STD_SOIMPL CqShaderExecEnv::SO_fnoise2(FLOATVAL u, FLOATVAL v, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(u)
	CHECKVARY(v)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,(m_noise.FGNoise2(FLOAT(u),FLOAT(v))+1)/2.0f);
	END_FORR
}

//----------------------------------------------------------------------
// noise(p)
STD_SOIMPL CqShaderExecEnv::SO_fnoise3(POINTVAL p, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(p)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,(m_noise.FGNoise3(POINT(p))+1)/2.0f);
	END_FORR
}

//----------------------------------------------------------------------
// noise(p,t)
STD_SOIMPL CqShaderExecEnv::SO_fnoise4(POINTVAL p, FLOATVAL t, DEFPARAMIMPL)
{
	// TODO: Do proper 4D noise.
	INIT_SOR
	CHECKVARY(p)
	CHECKVARY(t)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,(m_noise.FGNoise3(POINT(p))+1)/2.0f);
	END_FORR
}

//----------------------------------------------------------------------
// noise(v)
STD_SOIMPL	CqShaderExecEnv::SO_cnoise1(FLOATVAL v, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(v)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,(m_noise.CGNoise1(FLOAT(v))+1)/2.0f);
	END_FORR
}

//----------------------------------------------------------------------
// noise(u,v)
STD_SOIMPL CqShaderExecEnv::SO_cnoise2(FLOATVAL u, FLOATVAL v, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(u)
	CHECKVARY(v)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,(m_noise.CGNoise2(FLOAT(u),FLOAT(v))+1)/2.0f);
	END_FORR
}

//----------------------------------------------------------------------
// noise(p)
STD_SOIMPL CqShaderExecEnv::SO_cnoise3(POINTVAL p, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(p)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,(m_noise.CGNoise3(POINT(p))+1)/2.0f);
	END_FORR
}

//----------------------------------------------------------------------
// noise(p,t)
STD_SOIMPL CqShaderExecEnv::SO_cnoise4(POINTVAL p, FLOATVAL t, DEFPARAMIMPL)
{
	// TODO: Do proper 4D noise.
	INIT_SOR
	CHECKVARY(p)
	CHECKVARY(t)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,(m_noise.CGNoise3(POINT(p))+1)/2.0f);
	END_FORR
}

//----------------------------------------------------------------------
// noise(v)
STD_SOIMPL CqShaderExecEnv::SO_pnoise1(FLOATVAL v, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(v)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,(m_noise.PGNoise1(FLOAT(v))+1)/2.0f);
	END_FORR
}

//----------------------------------------------------------------------
// noise(u,v)
STD_SOIMPL CqShaderExecEnv::SO_pnoise2(FLOATVAL u, FLOATVAL v, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(u)
	CHECKVARY(v)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,(m_noise.PGNoise2(FLOAT(u),FLOAT(v))+1)/2.0f);
	END_FORR
}

//----------------------------------------------------------------------
// noise(p)
STD_SOIMPL CqShaderExecEnv::SO_pnoise3(POINTVAL p, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(p)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,(m_noise.PGNoise3(POINT(p))+1)/2.0f);
	END_FORR
}

//----------------------------------------------------------------------
// noise(p,t)
STD_SOIMPL CqShaderExecEnv::SO_pnoise4(POINTVAL p, FLOATVAL t, DEFPARAMIMPL)
{
	// TODO: Do proper 4D noise.
	INIT_SOR
	CHECKVARY(p)
	CHECKVARY(t)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,(m_noise.PGNoise3(POINT(p))+1)/2.0f);
	END_FORR
}

//----------------------------------------------------------------------
// setcomp(c,i,v)
STD_SOIMPL	CqShaderExecEnv::SO_setcomp(COLORVAL p, FLOATVAL i, FLOATVAL v, DEFVOIDPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(p)
	CHECKVARY(v)
	CHECKVARY(i)
	FOR_EACHR
		CqColor t=COLOR(p);
		t[i]=FLOAT(v);
		p.SetValue(i,t);
	END_FORR
}

//----------------------------------------------------------------------
// setxcomp(p,v)
STD_SOIMPL	CqShaderExecEnv::SO_setxcomp(POINTVAL p, FLOATVAL v, DEFVOIDPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(p)
	CHECKVARY(v)
	FOR_EACHR
		CqVector3D t=POINT(p);
		t.x(FLOAT(v));
		p.SetValue(i,t);
	END_FORR
}

//----------------------------------------------------------------------
// setycomp(p,v)
STD_SOIMPL	CqShaderExecEnv::SO_setycomp(POINTVAL p, FLOATVAL v, DEFVOIDPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(p)
	CHECKVARY(v)
	FOR_EACHR
		CqVector3D t=POINT(p);
		t.y(FLOAT(v));
		p.SetValue(i,t);
	END_FORR
}

//----------------------------------------------------------------------
// setzcomp(p,v)
STD_SOIMPL	CqShaderExecEnv::SO_setzcomp(POINTVAL p, FLOATVAL v, DEFVOIDPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(p)
	CHECKVARY(v)
	FOR_EACHR
		CqVector3D t=POINT(p);
		t.z(FLOAT(v));
		p.SetValue(i,t);
	END_FORR
}



STD_SOIMPL	CqShaderExecEnv::SO_length(VECTORVAL V, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(V)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,VECTOR(V).Magnitude());
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_distance(POINTVAL P1, POINTVAL P2, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(P1)
	CHECKVARY(P2)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,(POINT(P1)-POINT(P2)).Magnitude());
	END_FORR
}


//----------------------------------------------------------------------
// area(P)
STD_SOIMPL CqShaderExecEnv::SO_area(POINTVAL p, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(p)
	CHECKVARY(Result)
	FOR_EACHR
		CqVector3D	vecR=(SO_DuType<CqVector3D>(p,m_GridI,*this)*du())%(SO_DvType<CqVector3D>(p,m_GridI,*this)*dv());
		Result.SetValue(i,vecR.Magnitude());
	END_FORR
}


STD_SOIMPL	CqShaderExecEnv::SO_normalize(VECTORVAL V, DEFPARAMIMPL)	
{
	INIT_SOR
	CHECKVARY(V)
	CHECKVARY(Result)
	FOR_EACHR
		CqVector3D r(VECTOR(V)); 
		r.Unit(); 
		Result.SetValue(i,r);
	END_FORR
}


//----------------------------------------------------------------------
// faceforward(N,I,[Nref])
STD_SOIMPL CqShaderExecEnv::SO_faceforward(NORMALVAL N, VECTORVAL I /* [Nref] */, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(N)
	CHECKVARY(I)
	CHECKVARY(Result)
	FOR_EACHR
		CqVector3D vN=NORMAL(N);
		CqVector3D vI=VECTOR(I);
		//CqVector3D vNg=Ng();
		TqFloat s=(((-vI)*vN)<0.0f)?-1.0f:1.0f;
		Result.SetValue(i,vN*s);
	END_FORR
}


//----------------------------------------------------------------------
// reflect(I,N)
STD_SOIMPL CqShaderExecEnv::SO_reflect(VECTORVAL I, NORMALVAL N, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(I)
	CHECKVARY(N)
	CHECKVARY(Result)
	FOR_EACHR
		CqVector3D	vI=VECTOR(I);
		CqVector3D	vN=NORMAL(N);
		TqFloat idn=2.0f*(vI*vN);
		CqVector3D res=vI-(idn*vN);
		Result.SetValue(i,res);
	END_FORR
}


//----------------------------------------------------------------------
// reftact(I,N,eta)
STD_SOIMPL CqShaderExecEnv::SO_refract(VECTORVAL I, NORMALVAL N, FLOATVAL eta, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(I)
	CHECKVARY(N)
	CHECKVARY(eta)
	CHECKVARY(Result)
	FOR_EACHR
		TqFloat IdotN=VECTOR(I)*NORMAL(N);
		TqFloat feta=FLOAT(eta);
		TqFloat k=1-feta*feta*(1-IdotN*IdotN);
		Result.SetValue(i,(k<0.0f)? CqVector3D(0,0,0):CqVector3D(feta*VECTOR(I)-(feta*IdotN+sqrt(k))*NORMAL(N)));
	END_FORR
}


//----------------------------------------------------------------------
// fresnel(I,N,eta,Kr,Kt)
#define SQR(A)	((A)*(A))
STD_SOIMPL CqShaderExecEnv::SO_fresnel(VECTORVAL I, NORMALVAL N, FLOATVAL eta, FLOATVAL Kr, FLOATVAL Kt, DEFVOIDPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(I)
	CHECKVARY(N)
	CHECKVARY(eta)
	CHECKVARY(Kr)
	CHECKVARY(Kt)
	FOR_EACHR
		TqFloat cos_theta = -VECTOR(I)*NORMAL(N);
		TqFloat fuvA = SQR(1.0f/FLOAT(eta)) - (1.0f-SQR(cos_theta));
		TqFloat fuvB = sqrt(SQR(fuvA));
		TqFloat fu2 = (fuvA + fuvB)/2;
		TqFloat fv2 = (-fuvA + fuvB)/2;
		TqFloat fperp2 = (SQR(cos_theta-sqrt(fu2)) + fv2) / (SQR(cos_theta+sqrt(fu2)) + fv2);
		TqFloat feta=FLOAT(eta);
		TqFloat fpara2 = (SQR(SQR(1.0f/feta)*cos_theta - sqrt(fu2)) + SQR(-sqrt(fv2))) /
						 (SQR(SQR(1.0f/feta)*cos_theta + sqrt(fu2)) + SQR(sqrt(fv2)));

		Kr.SetValue(i,0.5f*(fperp2 + fpara2));
		Kt.SetValue(i,1.0f-FLOAT(Kr));
	END_FORR
}

//----------------------------------------------------------------------
// fresnel(I,N,eta,Kr,Kt,R,T)
STD_SOIMPL CqShaderExecEnv::SO_fresnel(VECTORVAL I, NORMALVAL N, FLOATVAL eta, FLOATVAL Kr, FLOATVAL Kt, VECTORVAL R, VECTORVAL T, DEFVOIDPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(I)
	CHECKVARY(N)
	CHECKVARY(eta)
	CHECKVARY(Kr)
	CHECKVARY(Kt)
	CHECKVARY(R)
	CHECKVARY(T)
	FOR_EACHR
		TqFloat cos_theta = -VECTOR(I)*NORMAL(N);
		TqFloat fuvA = SQR(1.0f/FLOAT(eta)) - (1.0f-SQR(cos_theta));
		TqFloat fuvB = sqrt(SQR(fuvA));
		TqFloat fu2 = (fuvA + fuvB)/2;
		TqFloat fv2 = (-fuvA + fuvB)/2;
		TqFloat feta = FLOAT(eta);
		TqFloat fperp2 = (SQR(cos_theta-sqrt(fu2)) + fv2) / (SQR(cos_theta+sqrt(fu2)) + fv2);
		TqFloat fpara2 = (SQR(SQR(1.0f/feta)*cos_theta - sqrt(fu2)) + SQR(-sqrt(fv2))) /
						 (SQR(SQR(1.0f/feta)*cos_theta + sqrt(fu2)) + SQR(sqrt(fv2)));
		Kr.SetValue(i,0.5f*(fperp2 + fpara2));
		Kt.SetValue(i,1.0f-FLOAT(Kr));
	END_FORR
	SO_reflect(I,N,R);
	SO_refract(I,N,eta,T);
}


//----------------------------------------------------------------------
// transform(s,s,P)
STD_SOIMPL CqShaderExecEnv::SO_transform(STRINGVAL fromspace, STRINGVAL tospace, POINTVAL p, DEFPARAMIMPL)
{
	assert(pShader!=0);

	const CqMatrix& mat=pCurrentRenderer()->matSpaceToSpace(fromspace.Value(temp_string,0).c_str(), tospace.Value(temp_string,0).c_str(), pShader->matCurrent(), matObjectToWorld());
	INIT_SOR
	CHECKVARY(p)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,mat*POINT(p));
	END_FORR
}


//----------------------------------------------------------------------
// transform(s,P)
STD_SOIMPL CqShaderExecEnv::SO_transform(STRINGVAL tospace, POINTVAL p, DEFPARAMIMPL)
{
	assert(pShader!=0);

	const CqMatrix& mat=pCurrentRenderer()->matSpaceToSpace("current", tospace.Value(temp_string,0).c_str(), pShader->matCurrent(), matObjectToWorld());
	INIT_SOR
	CHECKVARY(p)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,mat*POINT(p));
	END_FORR
}


//----------------------------------------------------------------------
// transform(m,P)
STD_SOIMPL CqShaderExecEnv::SO_transformm(MATRIXVAL tospace, POINTVAL p, DEFPARAMIMPL)
{
	assert(pShader!=0);

	INIT_SOR
	CHECKVARY(p)
	CHECKVARY(Result)
	FOR_EACHR
		const CqMatrix& mat=MATRIX(tospace);
		Result.SetValue(i,mat*POINT(p));
	END_FORR
}


//----------------------------------------------------------------------
// vtransform(s,s,P)
STD_SOIMPL CqShaderExecEnv::SO_vtransform(STRINGVAL fromspace, STRINGVAL tospace, VECTORVAL p, DEFPARAMIMPL)
{
	assert(pShader!=0);

	const CqMatrix& mat=pCurrentRenderer()->matVSpaceToSpace(fromspace.Value(temp_string,0).c_str(), tospace.Value(temp_string,0).c_str(), pShader->matCurrent(), matObjectToWorld());
	INIT_SOR
	CHECKVARY(p)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,mat*POINT(p));
	END_FORR
}


//----------------------------------------------------------------------
// vtransform(s,P)
STD_SOIMPL CqShaderExecEnv::SO_vtransform(STRINGVAL tospace, VECTORVAL p, DEFPARAMIMPL)
{
	assert(pShader!=0);

	const CqMatrix& mat=pCurrentRenderer()->matVSpaceToSpace("current", tospace.Value(temp_string,0).c_str(), pShader->matCurrent(), matObjectToWorld());
	INIT_SOR
	CHECKVARY(p)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,mat*POINT(p));
	END_FORR
}


//----------------------------------------------------------------------
// vtransform(m,P)
STD_SOIMPL CqShaderExecEnv::SO_vtransformm(MATRIXVAL tospace, VECTORVAL p, DEFPARAMIMPL)
{
	assert(pShader!=0);

	INIT_SOR
	CHECKVARY(p)
	CHECKVARY(Result)
	FOR_EACHR
		const CqMatrix& mat=MATRIX(tospace);
		Result.SetValue(i,mat*POINT(p));
	END_FORR
}


//----------------------------------------------------------------------
// ntransform(s,s,P)
STD_SOIMPL CqShaderExecEnv::SO_ntransform(STRINGVAL fromspace, STRINGVAL tospace, NORMALVAL p, DEFPARAMIMPL)
{
	assert(pShader!=0);

	const CqMatrix& mat=pCurrentRenderer()->matNSpaceToSpace(fromspace.Value(temp_string,0).c_str(), tospace.Value(temp_string,0).c_str(), pShader->matCurrent(), matObjectToWorld());
	INIT_SOR
	CHECKVARY(p)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,mat*POINT(p));
	END_FORR
}


//----------------------------------------------------------------------
// ntransform(s,P)
STD_SOIMPL CqShaderExecEnv::SO_ntransform(STRINGVAL tospace, NORMALVAL p, DEFPARAMIMPL)
{
	assert(pShader!=0);

	const CqMatrix& mat=pCurrentRenderer()->matNSpaceToSpace("current", tospace.Value(temp_string,0).c_str(), pShader->matCurrent(), matObjectToWorld());
	INIT_SOR
	CHECKVARY(p)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,mat*POINT(p));
	END_FORR
}


//----------------------------------------------------------------------
// ntransform(m,P)
STD_SOIMPL CqShaderExecEnv::SO_ntransformm(MATRIXVAL tospace, NORMALVAL p, DEFPARAMIMPL)
{
	assert(pShader!=0);

	INIT_SOR
	CHECKVARY(p)
	CHECKVARY(Result)
	FOR_EACHR
		const CqMatrix& mat=MATRIX(tospace);
		Result.SetValue(i,mat*POINT(p));
	END_FORR
}


//----------------------------------------------------------------------
// depth(P)
STD_SOIMPL CqShaderExecEnv::SO_depth(POINTVAL p, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(p)
	CHECKVARY(Result)
	FOR_EACHR
		TqFloat d=POINT(p).z();
		d=(d-pCurrentRenderer()->optCurrent().fClippingPlaneNear())/
			(pCurrentRenderer()->optCurrent().fClippingPlaneFar()-pCurrentRenderer()->optCurrent().fClippingPlaneNear());
		Result.SetValue(i,d);
	END_FORR
}


//----------------------------------------------------------------------
// calculatenormal(P)
STD_SOIMPL CqShaderExecEnv::SO_calculatenormal(POINTVAL p, DEFPARAMIMPL)
{
	// Find out if the orientation is inverted.
	EqOrientation O=pSurface()->pAttributes()->eOrientation();
	float neg=1;
	if(O!=OrientationLH)	neg=-1;

	INIT_SOR
	CHECKVARY(p)
	CHECKVARY(Result)
	FOR_EACHR
		CqVector3D	dPdu=SO_DuType<CqVector3D>(p,m_GridI,*this);
		CqVector3D	dPdv=SO_DvType<CqVector3D>(p,m_GridI,*this);
		CqVector3D	N=dPdu%dPdv;
		N.Unit();
		N*=neg;
		Result.SetValue(i,N);
	END_FORR
}

STD_SOIMPL CqShaderExecEnv::SO_cmix(COLORVAL color0, COLORVAL color1, FLOATVAL value, DEFPARAMIMPL)
{
	//assert(value>=0.0 && value<=1.0);
	INIT_SOR
	CHECKVARY(color0)
	CHECKVARY(color1)
	CHECKVARY(value)
	CHECKVARY(Result)
	FOR_EACHR
		CqColor c((1.0f-FLOAT(value))*COLOR(color0)+FLOAT(value)*COLOR(color1));
		Result.SetValue(i,c);
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_fmix(FLOATVAL f0, FLOATVAL f1, FLOATVAL value, DEFPARAMIMPL)
{
	//assert(value>=0.0 && value<=1.0);
	INIT_SOR
	CHECKVARY(f0)
	CHECKVARY(f1)
	CHECKVARY(value)
	FOR_EACHR
		TqFloat f((1.0f-FLOAT(value))*FLOAT(f0)+FLOAT(value)*FLOAT(f1));
		Result.SetValue(i,f);
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_pmix(POINTVAL p0, POINTVAL p1, FLOATVAL value, DEFPARAMIMPL)
{
	//assert(value>=0.0 && value<=1.0);
	INIT_SOR
	CHECKVARY(p0)
	CHECKVARY(p1)
	CHECKVARY(value)
	FOR_EACHR
		CqVector3D p((1.0f-FLOAT(value))*POINT(p0)+FLOAT(value)*POINT(p1));
		Result.SetValue(i,p);
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_vmix(VECTORVAL v0, VECTORVAL v1, FLOATVAL value, DEFPARAMIMPL)
{
	//assert(value>=0.0 && value<=1.0);
	INIT_SOR
	CHECKVARY(v0)
	CHECKVARY(v1)
	CHECKVARY(value)
	FOR_EACHR
		CqVector3D v((1.0f-FLOAT(value))*VECTOR(v0)+FLOAT(value)*VECTOR(v1));
		Result.SetValue(i,v);
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_nmix(NORMALVAL n0, NORMALVAL n1, FLOATVAL value, DEFPARAMIMPL)
{
	//assert(value>=0.0 && value<=1.0);
	INIT_SOR
	CHECKVARY(n0)
	CHECKVARY(n1)
	CHECKVARY(value)
	FOR_EACHR
		CqVector3D n((1.0f-FLOAT(value))*NORMAL(n0)+FLOAT(value)*NORMAL(n1));
		Result.SetValue(i,n);
	END_FORR
}


//----------------------------------------------------------------------
// texture(S)
STD_SOIMPL CqShaderExecEnv::SO_ftexture1(STRINGVAL name, FLOATVAL channel, DEFPARAMVARIMPL)
{
	GET_TEXTURE_PARAMS;
	
	TqInt i=0;
	CqTextureMap* pTMap=CqTextureMap::GetTextureMap(STRING(name).c_str());
	INIT_SOR
	__fVarying=TqTrue;
	if(pTMap!=0 && pTMap->IsValid())
	{
		float fdu=du();
		float fdv=dv();
		FOR_EACHR
			TqFloat swidth=0.0f,twidth=0.0f;
			if(fdu!=0.0f && fdv!=0.0f)
			{
				TqFloat dsdu=SO_DuType<TqFloat>(s(),m_GridI,*this);
				swidth=fabs(dsdu*fdu);	
				TqFloat dtdu=SO_DuType<TqFloat>(t(),m_GridI,*this);
				twidth=fabs(dtdu*fdu);	

				TqFloat dsdv=SO_DvType<TqFloat>(s(),m_GridI,*this);
				swidth+=fabs(dsdv*fdv);	
				TqFloat dtdv=SO_DvType<TqFloat>(t(),m_GridI,*this);
				twidth+=fabs(dtdv*fdv);	
			}

			swidth*=_pswidth;
			twidth*=_ptwidth;

			// Sample the texture.
			std::valarray<float> val;
			pTMap->SampleSATMap(s(),t(),swidth,twidth,_psblur,_ptblur,val);

			// Grab the appropriate channel.
			float fchan=FLOAT(channel);
			if(fchan>=val.size())
				Result.SetValue(i,_pfill);
			else
				Result.SetValue(i,val[fchan]);
		END_FORR
	}
	else
		Result=0.0f;
}

//----------------------------------------------------------------------
// texture(S,F,F)
STD_SOIMPL CqShaderExecEnv::SO_ftexture2(STRINGVAL name, FLOATVAL channel, FLOATVAL s, FLOATVAL t, DEFPARAMVARIMPL)
{
	GET_TEXTURE_PARAMS;

	TqInt i=0;
	TqFloat f;
	CqTextureMap* pTMap=CqTextureMap::GetTextureMap(STRING(name).c_str());
	INIT_SOR
	__fVarying=TqTrue;
	if(pTMap!=0 && pTMap->IsValid())
	{
		float fdu=du();
		float fdv=dv();
		FOR_EACHR
			TqFloat swidth=0.0f,twidth=0.0f;
			if(fdu!=0.0f && fdv!=0.0f)
			{
				TqFloat dsdu=SO_DuType<TqFloat>(s,m_GridI,*this);
				swidth=fabs(dsdu*fdu);	
				TqFloat dtdu=SO_DuType<TqFloat>(t,m_GridI,*this);
				twidth=fabs(dtdu*fdu);	

				TqFloat dsdv=SO_DvType<TqFloat>(s,m_GridI,*this);
				swidth+=fabs(dsdv*fdv);	
				TqFloat dtdv=SO_DvType<TqFloat>(t,m_GridI,*this);
				twidth+=fabs(dtdv*fdv);	
			}

			swidth*=_pswidth;
			twidth*=_ptwidth;
		
			// Sample the texture.
			std::valarray<float> val;
			pTMap->SampleSATMap(s.Value(f,m_GridI),t.Value(f,m_GridI),swidth,twidth,_psblur,_ptblur,val);

			// Grab the appropriate channel.
			float fchan=FLOAT(channel);
			if(fchan>=val.size())
				Result.SetValue(i,_pfill);
			else
				Result.SetValue(i,val[fchan]);
		END_FORR
	}
	else
		Result=0.0f;
}

//----------------------------------------------------------------------
// texture(S,F,F,F,F,F,F,F,F)
STD_SOIMPL CqShaderExecEnv::SO_ftexture3(STRINGVAL name, FLOATVAL channel, FLOATVAL s1, FLOATVAL t1, FLOATVAL s2, FLOATVAL t2, FLOATVAL s3, FLOATVAL t3, FLOATVAL s4, FLOATVAL t4, DEFPARAMVARIMPL)
{
	GET_TEXTURE_PARAMS;

	TqInt i=0;
	TqFloat f;
	CqTextureMap* pTMap=CqTextureMap::GetTextureMap(STRING(name).c_str());
	INIT_SOR
	__fVarying=TqTrue;
	if(pTMap!=0 && pTMap->IsValid())
	{
		FOR_EACHR
			// Sample the texture.
			std::valarray<float> val;
			pTMap->SampleSATMap(s1.Value(f,m_GridI),t1.Value(f,m_GridI),s2.Value(f,m_GridI),t2.Value(f,m_GridI),s3.Value(f,m_GridI),t3.Value(f,m_GridI),s4.Value(f,m_GridI),t4.Value(f,m_GridI),_psblur,_ptblur,val);

			// Grab the appropriate channel.
			float fchan=FLOAT(channel);
			if(fchan>=val.size())
				Result.SetValue(i,_pfill);
			else
				Result.SetValue(i,val[fchan]);
		END_FORR
	}
	else
		Result=0.0f;
}

//----------------------------------------------------------------------
// texture(S)
STD_SOIMPL CqShaderExecEnv::SO_ctexture1(STRINGVAL name, FLOATVAL channel, DEFPARAMVARIMPL)
{
	GET_TEXTURE_PARAMS;

	TqInt i=0;
	CqTextureMap* pTMap=CqTextureMap::GetTextureMap(STRING(name).c_str());
	INIT_SOR
	__fVarying=TqTrue;
	if(pTMap!=0 && pTMap->IsValid())
	{
		float fdu=du();
		float fdv=dv();
		FOR_EACHR
			TqFloat swidth=0.0f,twidth=0.0f;
			if(fdu!=0.0f && fdv!=0.0f)
			{
				TqFloat dsdu=SO_DuType<TqFloat>(s(),m_GridI,*this);
				swidth=fabs(dsdu*fdu);	
				TqFloat dsdv=SO_DvType<TqFloat>(s(),m_GridI,*this);
				swidth+=fabs(dsdv*fdv);	

				TqFloat dtdu=SO_DuType<TqFloat>(t(),m_GridI,*this);
				twidth=fabs(dtdu*fdu);	
				TqFloat dtdv=SO_DvType<TqFloat>(t(),m_GridI,*this);
				twidth+=fabs(dtdv*fdv);	
			}

			swidth*=_pswidth;
			twidth*=_ptwidth;

			// Sample the texture.
			std::valarray<float> val;
			pTMap->SampleSATMap(s(),t(),swidth,twidth,_psblur,_ptblur,val);

			// Grab the appropriate channel.
			float fchan=FLOAT(channel);
			if(fchan+2>=val.size())
				Result.SetValue(i,CqColor(_pfill,_pfill,_pfill));
			else
				Result.SetValue(i,CqColor(val[fchan],val[fchan+1],val[fchan+2]));
		END_FORR
	}
	else
		Result=CqColor(0,0,0);
}

//----------------------------------------------------------------------
// texture(S,F,F)
STD_SOIMPL CqShaderExecEnv::SO_ctexture2(STRINGVAL name, FLOATVAL channel, FLOATVAL s, FLOATVAL t, DEFPARAMVARIMPL)
{
	GET_TEXTURE_PARAMS;

	TqInt i=0;
	CqTextureMap* pTMap=CqTextureMap::GetTextureMap(STRING(name).c_str());
	TqFloat f;
	INIT_SOR
	__fVarying=TqTrue;
	if(pTMap!=0 && pTMap->IsValid())
	{
		float fdu=du();
		float fdv=dv();
		FOR_EACHR
			TqFloat swidth=0.0f,twidth=0.0f;
			if(fdu!=0.0f && fdv!=0.0f)
			{
				TqFloat dsdu=SO_DuType<TqFloat>(s,m_GridI,*this);
				swidth=fabs(dsdu*fdu);	
				TqFloat dsdv=SO_DvType<TqFloat>(s,m_GridI,*this);
				swidth+=fabs(dsdv*fdv);	

				TqFloat dtdu=SO_DuType<TqFloat>(t,m_GridI,*this);
				twidth=fabs(dtdu*fdu);	
				TqFloat dtdv=SO_DvType<TqFloat>(t,m_GridI,*this);
				twidth+=fabs(dtdv*fdv);	
			}

			swidth*=_pswidth;
			twidth*=_ptwidth;

			// Sample the texture.
			std::valarray<float> val;
			pTMap->SampleSATMap(s.Value(f,m_GridI),t.Value(f,m_GridI),swidth,twidth,_psblur,_ptblur,val);

			// Grab the appropriate channel.
			float fchan=FLOAT(channel);
			if(fchan+2>=val.size())
				Result.SetValue(i,CqColor(_pfill,_pfill,_pfill));
			else
				Result.SetValue(i,CqColor(val[fchan],val[fchan+1],val[fchan+2]));
		END_FORR
	}
	else
		Result=CqColor(0,0,0);
}

//----------------------------------------------------------------------
// texture(S,F,F,F,F,F,F,F,F)
STD_SOIMPL CqShaderExecEnv::SO_ctexture3(STRINGVAL name, FLOATVAL channel, FLOATVAL s1, FLOATVAL t1, FLOATVAL s2, FLOATVAL t2, FLOATVAL s3, FLOATVAL t3, FLOATVAL s4, FLOATVAL t4, DEFPARAMVARIMPL)
{
	GET_TEXTURE_PARAMS;

	TqInt i=0;
	TqFloat f;
	CqTextureMap* pTMap=CqTextureMap::GetTextureMap(STRING(name).c_str());
	INIT_SOR
	__fVarying=TqTrue;
	if(pTMap!=0 && pTMap->IsValid())
	{
		FOR_EACHR
			// Sample the texture.
			std::valarray<float> val;
			pTMap->SampleSATMap(s1.Value(f,m_GridI),t1.Value(f,m_GridI),s2.Value(f,m_GridI),t2.Value(f,m_GridI),s3.Value(f,m_GridI),t3.Value(f,m_GridI),s4.Value(f,m_GridI),t4.Value(f,m_GridI),_psblur,_ptblur,val);

			// Grab the appropriate channel.
			float fchan=FLOAT(channel);
			if(fchan+2>=val.size())
				Result.SetValue(i,CqColor(_pfill,_pfill,_pfill));
			else
				Result.SetValue(i,CqColor(val[fchan],val[fchan+1],val[fchan+2]));
		END_FORR
	}
	else
		Result=CqColor(0,0,0);
}


//----------------------------------------------------------------------
// environment(S,P)
STD_SOIMPL CqShaderExecEnv::SO_fenvironment2(STRINGVAL name, FLOATVAL channel, VECTORVAL R, DEFPARAMVARIMPL)
{
	GET_TEXTURE_PARAMS;

	TqInt i=0;
	CqTextureMap* pTMap=CqTextureMap::GetEnvironmentMap(STRING(name).c_str());
	CqVector3D f;
	INIT_SOR
	__fVarying=TqTrue;
	if(pTMap!=0 && pTMap->IsValid())
	{
		float fdu=du();
		float fdv=dv();
		FOR_EACHR
			CqVector3D swidth=0.0f,twidth=0.0f;
			if(fdu!=0.0f)
			{
				CqVector3D dRdu=SO_DuType<CqVector3D>(R,m_GridI,*this);
				swidth=dRdu*fdu;	
			}
			if(fdv!=0.0f)
			{
				CqVector3D dRdv=SO_DvType<CqVector3D>(R,m_GridI,*this);
				twidth=dRdv*fdv;	
			}

			swidth*=_pswidth;
			twidth*=_ptwidth;

			// Sample the texture.
			std::valarray<float> val;
			pTMap->SampleSATMap(R.Value(f,m_GridI),swidth,twidth,_psblur,_ptblur,val);

			// Grab the appropriate channel.
			float fchan=FLOAT(channel);
			if(fchan>=val.size())
				Result.SetValue(i,_pfill);
			else
				Result.SetValue(i,val[fchan]);
		END_FORR
	}
	else
		Result=0.0f;
}

//----------------------------------------------------------------------
// environment(S,P,P,P,P)
STD_SOIMPL CqShaderExecEnv::SO_fenvironment3(STRINGVAL name, FLOATVAL channel, VECTORVAL R1, VECTORVAL R2, VECTORVAL R3, VECTORVAL R4, DEFPARAMVARIMPL)
{
	GET_TEXTURE_PARAMS;

	TqInt i=0;
	CqTextureMap* pTMap=CqTextureMap::GetEnvironmentMap(STRING(name).c_str());
	CqVector3D f;
	INIT_SOR
	__fVarying=TqTrue;
	if(pTMap!=0 && pTMap->IsValid())
	{
		FOR_EACHR
			// Sample the texture.
			std::valarray<float> val;
			pTMap->SampleSATMap(R1.Value(f,m_GridI),R2.Value(f,m_GridI),R3.Value(f,m_GridI),R4.Value(f,m_GridI),_psblur,_ptblur,val);

			// Grab the appropriate channel.
			float fchan=FLOAT(channel);
			if(fchan>=val.size())
				Result.SetValue(i,_pfill);
			else
				Result.SetValue(i,val[fchan]);
		END_FORR
	}
	else
		Result=0.0f;
}


//----------------------------------------------------------------------
// environment(S,P)
STD_SOIMPL CqShaderExecEnv::SO_cenvironment2(STRINGVAL name, FLOATVAL channel, VECTORVAL R, DEFPARAMVARIMPL)
{
	GET_TEXTURE_PARAMS;

	TqInt i=0;
	CqTextureMap* pTMap=CqTextureMap::GetEnvironmentMap(STRING(name).c_str());
	CqVector3D f;
	INIT_SOR
	__fVarying=TqTrue;
	if(pTMap!=0 && pTMap->IsValid())
	{
		float fdu=du();
		float fdv=dv();
		FOR_EACHR
			CqVector3D swidth=0.0f,twidth=0.0f;
			if(fdu!=0.0f)
			{
				CqVector3D dRdu=SO_DuType<CqVector3D>(R,m_GridI,*this);
				swidth=dRdu*fdu;	
			}
			if(fdv!=0.0f)
			{
				CqVector3D dRdv=SO_DvType<CqVector3D>(R,m_GridI,*this);
				twidth=dRdv*fdv;	
			}

			swidth*=_pswidth;
			twidth*=_ptwidth;

			// Sample the texture.
			std::valarray<float> val;
			pTMap->SampleSATMap(R.Value(f,m_GridI),swidth,twidth,_psblur,_ptblur,val);

			// Grab the appropriate channel.
			float fchan=FLOAT(channel);
			if(fchan+2>=val.size())
				Result.SetValue(i,CqColor(_pfill,_pfill,_pfill));
			else
				Result.SetValue(i,CqColor(val[fchan],val[fchan+1],val[fchan+2]));
		END_FORR
	}
	else
		Result=CqColor(1,1,0);
}

//----------------------------------------------------------------------
// environment(S,P,P,P,P)
STD_SOIMPL CqShaderExecEnv::SO_cenvironment3(STRINGVAL name, FLOATVAL channel, VECTORVAL R1, VECTORVAL R2, VECTORVAL R3, VECTORVAL R4, DEFPARAMVARIMPL)
{
	GET_TEXTURE_PARAMS;

	TqInt i=0;
	CqTextureMap* pTMap=CqTextureMap::GetEnvironmentMap(STRING(name).c_str());
	CqVector3D f;
	INIT_SOR
	__fVarying=TqTrue;
	if(pTMap!=0 && pTMap->IsValid())
	{
		FOR_EACHR
			// Sample the texture.
			// TODO: need to get and pass width,blur etc. values.
			std::valarray<float> val;
			pTMap->SampleSATMap(R1.Value(f,m_GridI),R2.Value(f,m_GridI),R3.Value(f,m_GridI),R4.Value(f,m_GridI),_psblur,_ptblur,val);

			// Grab the appropriate channel.
			float fchan=FLOAT(channel);
			if(fchan+2>=val.size())
				Result.SetValue(i,CqColor(_pfill,_pfill,_pfill));
			else
				Result.SetValue(i,CqColor(val[fchan],val[fchan+1],val[fchan+2]));
		END_FORR
	}
	else
		Result=CqColor(0,0,0);
}

//----------------------------------------------------------------------
// bump(S)
STD_SOIMPL CqShaderExecEnv::SO_bump1(STRINGVAL name, FLOATVAL channel, DEFPARAMVARIMPL)
{
	INIT_SOR
	__fVarying=TqTrue;
	FOR_EACHR
		Result.SetValue(i,CqVector3D(0,0,0));
	END_FORR
}

//----------------------------------------------------------------------
// bump(S,F,F)
STD_SOIMPL CqShaderExecEnv::SO_bump2(STRINGVAL name, FLOATVAL channel, FLOATVAL s, FLOATVAL t, DEFPARAMVARIMPL)
{
	INIT_SOR
	__fVarying=TqTrue;
	FOR_EACHR
		Result.SetValue(i,CqVector3D(0,0,0));
	END_FORR
}

//----------------------------------------------------------------------
// bump(S,F,F,F,F,F,F,F,F)
STD_SOIMPL CqShaderExecEnv::SO_bump3(STRINGVAL name, FLOATVAL channel, FLOATVAL s1, FLOATVAL t1, FLOATVAL s2, FLOATVAL t2, FLOATVAL s3, FLOATVAL t3, FLOATVAL s4, FLOATVAL t4, DEFPARAMVARIMPL)
{
	INIT_SOR
	__fVarying=TqTrue;
	FOR_EACHR
		Result.SetValue(i,CqVector3D(0,0,0));
	END_FORR
}

//----------------------------------------------------------------------
// shadow(S,P)
STD_SOIMPL CqShaderExecEnv::SO_shadow(STRINGVAL name, FLOATVAL channel, POINTVAL P, DEFPARAMVARIMPL)
{
	GET_TEXTURE_PARAMS;

	TqInt i=0;
	CqVector3D	p;
	static CqVMStackEntry den;
	den=1.0f;
	CqShadowMap* pMap=static_cast<CqShadowMap*>(CqShadowMap::GetShadowMap(STRING(name).c_str()));
	INIT_SOR
	__fVarying=TqTrue;
	if(pMap!=0 && pMap->IsValid())
	{
		FOR_EACHR
			CqVector3D swidth=0.0f,twidth=0.0f;
			
			//swidth=SO_DerivType<CqVector3D>(P,den,m_GridI,*this);
			//twidth=SO_DerivType<CqVector3D>(P,den,m_GridI,*this);
			swidth=0.0f;
			twidth=0.0f;

			swidth*=_pswidth;
			twidth*=_ptwidth;
			float fv;
			pMap->SampleMap(P.Value(p,m_GridI),swidth,twidth,_psblur,_ptblur,fv);	
			Result.SetValue(i,fv);
		END_FORR
	}
	else
		Result.SetValue(i,0);	// Default, completely lit
}

//----------------------------------------------------------------------
// shadow(S,P,P,P,P)
STD_SOIMPL CqShaderExecEnv::SO_shadow1(STRINGVAL name, FLOATVAL channel, POINTVAL P1, POINTVAL P2, POINTVAL P3, POINTVAL P4, DEFPARAMVARIMPL)
{
	GET_TEXTURE_PARAMS;

	TqInt i=0;
	CqVector3D	p;
	CqShadowMap* pMap=static_cast<CqShadowMap*>(CqShadowMap::GetShadowMap(STRING(name).c_str()));
	INIT_SOR
	__fVarying=TqTrue;
	if(pMap!=0 && pMap->IsValid())
	{
		FOR_EACHR
			float fv;
			pMap->SampleMap(P1.Value(p,m_GridI),P2.Value(p,m_GridI),P3.Value(p,m_GridI),P4.Value(p,m_GridI),_psblur,_ptblur,fv);	
			Result.SetValue(i,fv);
		END_FORR
	}
	else
		Result.SetValue(i,0);	// Default, completely lit
}


//----------------------------------------------------------------------
// ambient()
STD_SOIMPL CqShaderExecEnv::SO_ambient(DEFPARAMIMPL)
{
	static CqVMStackEntry Point;

	// Use the lightsource stack on the current surface
	if(m_pSurface!=0)
	{
		// If this is the first call to illuminance this time round, call all lights and setup the Cl and L caches.
		ValidateIlluminanceCache(Point,pShader);

		for(TqInt light_index=0; light_index<m_pSurface->pAttributes()->apLights().size(); light_index++)
		{
			CqLightsource* lp=m_pSurface->pAttributes()->apLights()[light_index];
			INIT_SOR
			__fVarying=TqTrue;
			FOR_EACHR
				Cl()[i]=lp->Cl()[i];
				if(light_index==0)	Result.SetValue(i,CqColor(0,0,0));
				if(lp->pShader()->fAmbient())
					Result.SetValue(i,COLOR(Result)+Cl());
			END_FORR
		}
	}
}


//----------------------------------------------------------------------
// diffuse(N)
STD_SOIMPL CqShaderExecEnv::SO_diffuse(NORMALVAL N, DEFPARAMIMPL)
{
	static CqVMStackEntry Tempangle;
	static CqVMStackEntry Tempnsamples;
	static CqVMStackEntry Point;

	Point=&P();
	// If the illuminance cache is already OK, then we don't need to bother filling in the illuminance parameters.
	if(!m_IlluminanceCacheValid)
	{
		ValidateIlluminanceCache(Point,pShader);
	}

	Tempangle=RI_PIO2;
	Tempnsamples=0.0f;

	// Setup the return value.
	INIT_SOR
	Result=CqColor(0,0,0);
	__fVarying=TqTrue;

	// SO_init_illuminance returns TRUE if there are any non ambient ligthsources available.
	if(SO_init_illuminance())
	{
		do
		{
			// SO_illuminance sets the current state to whether the lightsource illuminates the points or not.
			SO_illuminance(Point,N,Tempangle,Tempnsamples);
			PushState();
			GetCurrentState();
			FOR_EACHR
				CqVector3D	Ln=L();
				Ln.Unit();
				Result.SetValue(i,COLOR(Result)+Cl()*(Ln*NORMAL(N)));
			END_FORR
			PopState();
		// SO_advance_illuminance returns TRUE if there are any more non ambient lightsources.
		}while(SO_advance_illuminance());	
	}
	else
	{
		INIT_SOR
		__fVarying=TqTrue;
		FOR_EACHR
			Result.SetValue(i,CqColor(0,0,0));
		END_FORR
	}
}


//----------------------------------------------------------------------
// specular(N,V,roughness)
STD_SOIMPL CqShaderExecEnv::SO_specular(NORMALVAL N, VECTORVAL V, FLOATVAL roughness, DEFPARAMIMPL)
{
	static CqVMStackEntry Tempangle;
	static CqVMStackEntry Tempnsamples;
	static CqVMStackEntry Point;

	Point=&P();
	// If the illuminance cache is already OK, then we don't need to bother filling in the illuminance parameters.
	if(!m_IlluminanceCacheValid)
	{
		ValidateIlluminanceCache(Point,pShader);
	}

	Tempangle=RI_PIO2;
	Tempnsamples=0.0f;

	// Setup the return value.
	INIT_SOR
	Result=CqColor(0,0,0);
	__fVarying=TqTrue;

	// SO_init_illuminance returns TRUE if there are any non ambient ligthsources available.
	if(SO_init_illuminance())
	{
		do
		{
			// SO_illuminance sets the current state to whether the lightsource illuminates the points or not.
			SO_illuminance(Point,N,Tempangle,Tempnsamples);
			PushState();
			GetCurrentState();
			FOR_EACHR
				CqVector3D	Ln=L();	Ln.Unit();
				CqVector3D	H=Ln+VECTOR(V);	H.Unit();
				// NOTE: The (roughness/8) term emulates the BMRT behaviour for prmanspecular.
				Result.SetValue(i,COLOR(Result)+Cl()*pow(MAX(0.0f,NORMAL(N)*H),1.0f/(FLOAT(roughness)/8.0f)));
			END_FORR
			PopState();
		// SO_advance_illuminance returns TRUE if there are any more non ambient lightsources.
		}while(SO_advance_illuminance());	
	}
	else
	{
		INIT_SOR
		__fVarying=TqTrue;
		FOR_EACHR
			Result.SetValue(i,CqColor(0,0,0));
		END_FORR
	}
}


//----------------------------------------------------------------------
// phong(N,V,size)
STD_SOIMPL CqShaderExecEnv::SO_phong(NORMALVAL N, VECTORVAL V, FLOATVAL size, DEFPARAMIMPL)
{
	static CqVMStackEntry nV;
	static CqVMStackEntry nN;
	static CqVMStackEntry R;

	static CqVMStackEntry Tempangle;
	static CqVMStackEntry Tempnsamples;
	static CqVMStackEntry Point;

	SO_normalize(V,nV);
	SO_normalize(N,nN);

	INIT_SOR
	__fVarying=TqTrue;
	{	
		FOR_EACHR
			nV.SetValue(i,-VECTOR(nV));
		END_FORR
	}
	SO_reflect(nV,nN,R);

	Point=&P();
	// If the illuminance cache is already OK, then we don't need to bother filling in the illuminance parameters.
	if(!m_IlluminanceCacheValid)
	{
		ValidateIlluminanceCache(Point,pShader);
	}

	Tempangle=RI_PIO2;
	Tempnsamples=0.0f;

	// Initialise the return value
	Result=CqColor(0,0,0);

	// SO_init_illuminance returns TRUE if there are any non ambient ligthsources available.
	if(SO_init_illuminance())
	{
		do
		{
			// SO_illuminance sets the current state to whether the lightsource illuminates the points or not.
			SO_illuminance(Point,N,Tempangle,Tempnsamples);
			PushState();
			GetCurrentState();
			FOR_EACHR
				CqVector3D Ln=L();	Ln.Unit();
				Result.SetValue(i,COLOR(Result)+Cl()*pow(MAX(0.0f,VECTOR(R)*Ln), FLOAT(size)));
			END_FORR
			PopState();
		// SO_advance_illuminance returns TRUE if there are any more non ambient lightsources.
		}while(SO_advance_illuminance());	
	}
	else
	{
		INIT_SOR
		__fVarying=TqTrue;
		FOR_EACHR
			Result.SetValue(i,CqColor(0,0,0));
		END_FORR
	}
}


//----------------------------------------------------------------------
// trace(P,R)
STD_SOIMPL CqShaderExecEnv::SO_trace(POINTVAL P, VECTORVAL R, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(P)
	CHECKVARY(R)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,CqColor(0,0,0));
	END_FORR
}


//----------------------------------------------------------------------
// illuminance(P,nsamples)
STD_SOIMPL CqShaderExecEnv::SO_illuminance(POINTVAL P, VECTORVAL Axis, FLOATVAL Angle, FLOATVAL nsamples, DEFVOIDPARAMIMPL)
{
	// Fill in the lightsource information, and transfer the results to the shader variables,
	if(m_pSurface!=0)
	{
		CqLightsource* lp=m_pSurface->pAttributes()->apLights()[m_li];

		INIT_SOR
		CHECKVARY(P)
		CHECKVARY(Axis)
		CHECKVARY(Angle)
		CHECKVARY(nsamples)

		FOR_EACHR
			L()[i]=-(lp->L()[i]);
			Cl()[i]=lp->Cl()[i];
			// Check if its within the cone.
			if(FLOAT(Angle)!=-1)
			{
				CqVector3D nL=L();	nL.Unit();
				TqFloat cosangle=nL*VECTOR(Axis);
				if(acos(cosangle)>FLOAT(Angle))
					m_CurrentState.SetValue(i,TqFalse);
				else
					m_CurrentState.SetValue(i,TqTrue);
			}
			else
				m_CurrentState.SetValue(i,TqTrue);
		END_FORR		
	}
}


STD_SOIMPL	CqShaderExecEnv::SO_illuminance(POINTVAL P, FLOATVAL nsamples, DEFVOIDPARAMIMPL)
{
	static CqVMStackEntry Axis;
	static CqVMStackEntry Angle;

	Axis=CqVector3D(0,1,0);
	Angle=RI_PI*2;

	SO_illuminance(P,Axis,Angle,nsamples);
}


//----------------------------------------------------------------------
// illuminate(P)
STD_SOIMPL CqShaderExecEnv::SO_illuminate(POINTVAL P, VECTORVAL Axis, FLOATVAL Angle, DEFVOIDPARAMIMPL)
{
	TqBool res=TqTrue;
	if(m_Illuminate>0)	res=TqFalse;
	// TODO: Check light cone, and exclude points outside.
	INIT_SOR
	__fVarying=TqTrue;
	FOR_EACHR
		if(res)	L()=Ps()-POINT(P);
		m_CurrentState.SetValue(i,res);
	END_FORR
	m_Illuminate++;
}


STD_SOIMPL	CqShaderExecEnv::SO_illuminate(POINTVAL P, DEFVOIDPARAMIMPL)
{
	static CqVMStackEntry Axis;
	static CqVMStackEntry Angle;

	Axis=CqVector3D(0,0,0);
	Angle=(TqFloat)-1.0f;

	SO_illuminate(P, Axis, Angle, pShader);
}


//----------------------------------------------------------------------
// solar()
STD_SOIMPL CqShaderExecEnv::SO_solar(VECTORVAL Axis, FLOATVAL Angle, DEFVOIDPARAMIMPL)
{
	TqBool res=TqTrue;
	if(m_Illuminate>0)	res=TqFalse;
	// TODO: Check light cone, and exclude points outside.
	INIT_SOR
	__fVarying=TqTrue;
	FOR_EACHR
		if(res)	L()=VECTOR(Axis);
		m_CurrentState.SetValue(i,res);
	END_FORR
	m_Illuminate++;
}


STD_SOIMPL	CqShaderExecEnv::SO_solar(DEFVOIDPARAMIMPL)
{
	static CqVMStackEntry Axis;
	static CqVMStackEntry Angle;

	Axis=CqVector3D(0,0,0);
	Angle=(TqFloat)-1.0f;

	SO_solar(Axis, Angle, pShader);
}


//----------------------------------------------------------------------
// printf(s,...)

STD_SOIMPL	CqShaderExecEnv::SO_printf(STRINGVAL str, DEFVOIDPARAMVARIMPL)
{
	INIT_SOR
	CHECKVARY(str)
	int ii;
	for(ii=0; ii<cParams; ii++)	CHECKVARY(*apParams[ii]);
	FOR_EACHR
		CqString strA=SO_sprintf(STRING(str).c_str(),cParams,apParams,i);
		pCurrentRenderer()->PrintMessage(SqMessage(0,0,strA.c_str()));
	END_FORR
}


//----------------------------------------------------------------------
// format(s,...)

STD_SOIMPL	CqShaderExecEnv::SO_format(STRINGVAL str, DEFPARAMVARIMPL)
{
	INIT_SOR
	CHECKVARY(str)
	int ii;
	for(ii=0; ii<cParams; ii++)	CHECKVARY(*apParams[ii]);
	CHECKVARY(Result);
	FOR_EACHR
		CqString strA=SO_sprintf(STRING(str).c_str(),cParams,apParams,i);
		Result.SetValue(i,strA);
	END_FORR
}


//----------------------------------------------------------------------
// concat(s,s,...)

STD_SOIMPL	CqShaderExecEnv::SO_concat(STRINGVAL stra,STRINGVAL strb, DEFPARAMVARIMPL)
{
	INIT_SOR
	CHECKVARY(stra)
	CHECKVARY(strb)
	int ii;
	for(ii=0; ii<cParams; ii++)	CHECKVARY(*apParams[ii]);
	CHECKVARY(Result);
	FOR_EACHR
		CqString strRes=STRING(stra);
		strRes+=STRING(strb);
		for(ii=0; ii<cParams; ii++)
			strRes+=STRING(*apParams[ii]);
		Result.SetValue(i,strRes);
	END_FORR
}


//----------------------------------------------------------------------
// noise(v)
STD_SOIMPL CqShaderExecEnv::SO_fcellnoise1(FLOATVAL v, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(v)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,m_cellnoise.FCellNoise1(FLOAT(v)));
	END_FORR
}
STD_SOIMPL CqShaderExecEnv::SO_ccellnoise1(FLOATVAL v, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(v)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,CqColor(m_cellnoise.PCellNoise1(FLOAT(v))));
	END_FORR
}
STD_SOIMPL CqShaderExecEnv::SO_pcellnoise1(FLOATVAL v, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(v)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,m_cellnoise.PCellNoise1(FLOAT(v)));
	END_FORR
}

//----------------------------------------------------------------------
// noise(u,v)
STD_SOIMPL CqShaderExecEnv::SO_fcellnoise2(FLOATVAL u, FLOATVAL v, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(u)
	CHECKVARY(v)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,m_cellnoise.FCellNoise2(FLOAT(u),FLOAT(v)));
	END_FORR
}
STD_SOIMPL CqShaderExecEnv::SO_ccellnoise2(FLOATVAL u, FLOATVAL v, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(u)
	CHECKVARY(v)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,CqColor(m_cellnoise.PCellNoise2(FLOAT(u),FLOAT(v))));
	END_FORR
}
STD_SOIMPL CqShaderExecEnv::SO_pcellnoise2(FLOATVAL u, FLOATVAL v, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(u)
	CHECKVARY(v)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,m_cellnoise.PCellNoise2(FLOAT(u),FLOAT(v)));
	END_FORR
}

//----------------------------------------------------------------------
// noise(p)
STD_SOIMPL CqShaderExecEnv::SO_fcellnoise3(POINTVAL p, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(p)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,m_cellnoise.FCellNoise3(POINT(p)));
	END_FORR
}
STD_SOIMPL CqShaderExecEnv::SO_ccellnoise3(POINTVAL p, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(p)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,CqColor(m_cellnoise.PCellNoise3(POINT(p))));
	END_FORR
}
STD_SOIMPL CqShaderExecEnv::SO_pcellnoise3(POINTVAL p, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(p)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,m_cellnoise.PCellNoise3(POINT(p)));
	END_FORR
}

//----------------------------------------------------------------------
// noise(p,f)
STD_SOIMPL CqShaderExecEnv::SO_fcellnoise4(POINTVAL p, FLOATVAL v, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(p)
	CHECKVARY(v)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,m_cellnoise.FCellNoise4(POINT(p),FLOAT(v)));
	END_FORR
}
STD_SOIMPL CqShaderExecEnv::SO_ccellnoise4(POINTVAL p, FLOATVAL v, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(p)
	CHECKVARY(v)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,CqColor(m_cellnoise.PCellNoise4(POINT(p),FLOAT(v))));
	END_FORR
}
STD_SOIMPL CqShaderExecEnv::SO_pcellnoise4(POINTVAL p, FLOATVAL v, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(p)
	CHECKVARY(v)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,m_cellnoise.PCellNoise4(POINT(p),FLOAT(v)));
	END_FORR
}



//----------------------------------------------------------------------
// atmosphere
//

STD_SOIMPL CqShaderExecEnv::SO_atmosphere(STRINGVAL name, CqShaderVariable* pV, DEFPARAMIMPL)
{
	CqShader* pAtmosphere=m_pSurface->pAttributes()->pshadAtmosphere();
	TqInt i=0;
	if(pAtmosphere)	Result.SetValue(0,pAtmosphere->GetValue(STRING(name).c_str(), pV)?1.0f:0.0f);
	else			Result.SetValue(0,0.0f);
}


//----------------------------------------------------------------------
// displacement
//

STD_SOIMPL CqShaderExecEnv::SO_displacement(STRINGVAL name, CqShaderVariable* pV, DEFPARAMIMPL)
{
	CqShader* pDisplacement=m_pSurface->pAttributes()->pshadDisplacement();
	TqInt i=0;
	if(pDisplacement)	Result.SetValue(0,pDisplacement->GetValue(STRING(name).c_str(), pV)?1.0f:0.0f);
	else				Result.SetValue(0,0.0f);
}


//----------------------------------------------------------------------
// lightsource
//

STD_SOIMPL CqShaderExecEnv::SO_lightsource(STRINGVAL name, CqShaderVariable* pV, DEFPARAMIMPL)
{
	// This should only be called within an Illuminance construct, so m_li should be valid.
	TqInt i=0;
	CqShader* pLightsource=0;
	if(m_li<m_pSurface->pAttributes()->apLights().size())
		pLightsource=m_pSurface->pAttributes()->apLights()[m_li]->pShader();
	if(pLightsource)	Result.SetValue(0,pLightsource->GetValue(STRING(name).c_str(), pV)?1.0f:0.0f);
	else				Result.SetValue(0,0.0f);
}


//----------------------------------------------------------------------
// surface
//

STD_SOIMPL CqShaderExecEnv::SO_surface(STRINGVAL name, CqShaderVariable* pV, DEFPARAMIMPL)
{
	CqShader* pSurface=m_pSurface->pAttributes()->pshadSurface();
	TqInt i=0;
	if(pSurface)	Result.SetValue(0,pSurface->GetValue(STRING(name).c_str(), pV)?1.0f:0.0f);
	else			Result.SetValue(0,0.0f);
}


//----------------------------------------------------------------------
// attribute
//

STD_SOIMPL CqShaderExecEnv::SO_attribute(STRINGVAL name, CqShaderVariable* pV, DEFPARAMIMPL)
{
	//Find out if it is a specific attribute request
	CqString strName=name.Value(strName,0);
	TqFloat Ret=0.0f;

	if(strName.compare("ShadingRate")==0)
	{
		if((pV->Type()&Type_Mask)==Type_Float)
		{
			CqVMStackEntry SE(m_pSurface->pAttributes()->fEffectiveShadingRate());
			pV->SetValue(SE);
			Ret=1.0f;
		}
	}
	else if(strName.compare("Sides")==0)
	{
		if((pV->Type()&Type_Mask)==Type_Float)
		{
			CqVMStackEntry SE(m_pSurface->pAttributes()->iNumberOfSides());
			pV->SetValue(SE);
			Ret=1.0f;
		}
	}
	else if(strName.compare("Matte")==0)
	{
		if((pV->Type()&Type_Mask)==Type_Float)
		{
			CqVMStackEntry SE(m_pSurface->pAttributes()->bMatteSurfaceFlag());
			pV->SetValue(SE);
			Ret=1.0f;
		}
	}
	else
	{
		int iColon=strName.find_first_of(':');
		if(iColon>=0)
		{
			CqString strParam=strName.substr(iColon+1,strName.size()-iColon-1);
			strName=strName.substr(0,iColon);
			const CqParameter* pParam=m_pSurface->pAttributes()->pParameter(strName.c_str(), strParam.c_str());
			if(pParam!=0)
			{
				// Should only be able to query uniform parameters here, varying ones should be handled
				// by passing as shader paramters.
				// Types must match, although storage doesn't have to
				if((pParam->Type()&Type_Uniform) &&
				   ((pParam->Type()&Type_Mask)==(pV->Type()&Type_Mask)))
				{
					CqVMStackEntry se;
					switch(pParam->Type()&Type_Mask)
					{
						case Type_Integer:
						{
							se=static_cast<TqFloat>(*(static_cast<const CqParameterTyped<TqInt>*>(pParam)->pValue()));
							break;
						}

						case Type_Float:
						{
							se=*(static_cast<const CqParameterTyped<TqFloat>*>(pParam)->pValue());
							break;
						}

						case Type_String:
						{
							se=*(static_cast<const CqParameterTyped<CqString>*>(pParam)->pValue());
							break;
						}

						case Type_Point:
						case Type_Vector:
						case Type_Normal:
						{
							se=*(static_cast<const CqParameterTyped<CqVector3D>*>(pParam)->pValue());
							break;
						}

						case Type_Color:
						{
							se=*(static_cast<const CqParameterTyped<CqColor>*>(pParam)->pValue());
							break;
						}

						case Type_Matrix:
						{
							se=*(static_cast<const CqParameterTyped<CqMatrix>*>(pParam)->pValue());
							break;
						}
					}
					pV->SetValue(se);
					Ret=1.0f;
				}
			}
		}
	}

	Result.SetValue(0,Ret);
}


//----------------------------------------------------------------------
// option
//

STD_SOIMPL CqShaderExecEnv::SO_option(STRINGVAL name, CqShaderVariable* pV, DEFPARAMIMPL)
{
	//Find out if it is a specific option request
	CqString strName=name.Value(strName,0);
	TqFloat Ret=0.0f;
	CqVMStackEntry se;

	if(strName.compare("Format")==0)
	{
		if((pV->Type()&Type_Mask)==Type_Float &&
		   (pV->Type()&Type_Array))
		{			
			CqShaderVariableArray* paV=static_cast<CqShaderVariableArray*>(pV);
			if(paV->ArrayLength()>=3)
			{
				se=static_cast<TqFloat>(pCurrentRenderer()->optCurrent().iXResolution());
				(*paV)[0]->SetValue(se);
				se=static_cast<TqFloat>(pCurrentRenderer()->optCurrent().iYResolution());
				(*paV)[1]->SetValue(se);
				se=static_cast<TqFloat>(pCurrentRenderer()->optCurrent().fPixelAspectRatio());
				(*paV)[2]->SetValue(se);
				Ret=1.0f;
			}
		}
	}
	else if(strName.compare("CropWindow")==0)
	{
		if((pV->Type()&Type_Mask)==Type_Float &&
		   (pV->Type()&Type_Array))
		{			
			CqShaderVariableArray* paV=static_cast<CqShaderVariableArray*>(pV);
			if(paV->ArrayLength()>=4)
			{
				se=static_cast<TqFloat>(pCurrentRenderer()->optCurrent().fCropWindowXMin());
				(*paV)[0]->SetValue(se);
				se=static_cast<TqFloat>(pCurrentRenderer()->optCurrent().fCropWindowXMax());
				(*paV)[1]->SetValue(se);
				se=static_cast<TqFloat>(pCurrentRenderer()->optCurrent().fCropWindowYMin());
				(*paV)[2]->SetValue(se);
				se=static_cast<TqFloat>(pCurrentRenderer()->optCurrent().fCropWindowYMax());
				(*paV)[3]->SetValue(se);
				Ret=1.0f;
			}
		}
	}
	else if(strName.compare("FrameAspectRatio")==0)
	{
		if((pV->Type()&Type_Mask)==Type_Float)
		{
			se=static_cast<TqFloat>(pCurrentRenderer()->optCurrent().fFrameAspectRatio());
			pV->SetValue(se);
			Ret=1.0f;
		}
	}
	else if(strName.compare("DepthOfField")==0)
	{
		if((pV->Type()&Type_Mask)==Type_Float &&
		   (pV->Type()&Type_Array))
		{			
			CqShaderVariableArray* paV=static_cast<CqShaderVariableArray*>(pV);
			if(paV->ArrayLength()>=3)
			{
				se=static_cast<TqFloat>(pCurrentRenderer()->optCurrent().ffStop());
				(*paV)[0]->SetValue(se);
				se=static_cast<TqFloat>(pCurrentRenderer()->optCurrent().fFocalLength());
				(*paV)[1]->SetValue(se);
				se=static_cast<TqFloat>(pCurrentRenderer()->optCurrent().fFocalDistance());
				(*paV)[2]->SetValue(se);
				Ret=1.0f;
			}
		}
	}
	else if(strName.compare("Shutter")==0)
	{
		if((pV->Type()&Type_Mask)==Type_Float &&
		   (pV->Type()&Type_Array))
		{			
			CqShaderVariableArray* paV=static_cast<CqShaderVariableArray*>(pV);
			if(paV->ArrayLength()>=2)
			{
				se=static_cast<TqFloat>(pCurrentRenderer()->optCurrent().fShutterOpen());
				(*paV)[0]->SetValue(se);
				se=static_cast<TqFloat>(pCurrentRenderer()->optCurrent().fShutterClose());
				(*paV)[1]->SetValue(se);
				Ret=1.0f;
			}
		}
	}
	else if(strName.compare("Clipping")==0)
	{
		if((pV->Type()&Type_Mask)==Type_Float &&
		   (pV->Type()&Type_Array))
		{			
			CqShaderVariableArray* paV=static_cast<CqShaderVariableArray*>(pV);
			if(paV->ArrayLength()>=2)
			{
				se=static_cast<TqFloat>(pCurrentRenderer()->optCurrent().fClippingPlaneNear());
				(*paV)[0]->SetValue(se);
				se=static_cast<TqFloat>(pCurrentRenderer()->optCurrent().fClippingPlaneFar());
				(*paV)[1]->SetValue(se);
				Ret=1.0f;
			}
		}
	}
	else
	{
		int iColon=strName.find_first_of(':');
		if(iColon>=0)
		{
			CqString strParam=strName.substr(iColon+1,strName.size()-iColon-1);
			strName=strName.substr(0,iColon);
			const CqParameter* pParam=pCurrentRenderer()->optCurrent().pParameter(strName.c_str(), strParam.c_str());
			if(pParam!=0)
			{
				// Should only be able to query uniform parameters here, varying ones should be handled
				// by passing as shader paramters.
				// Types must match, although storage doesn't have to
				if((pParam->Type()&Type_Uniform) &&
				   ((pParam->Type()&Type_Mask)==(pV->Type()&Type_Mask)))
				{
					switch(pParam->Type()&Type_Mask)
					{
						case Type_Integer:
						{
							se=static_cast<TqFloat>(*(static_cast<const CqParameterTyped<TqInt>*>(pParam)->pValue()));
							break;
						}

						case Type_Float:
						{
							se=*(static_cast<const CqParameterTyped<TqFloat>*>(pParam)->pValue());
							break;
						}

						case Type_String:
						{
							se=*(static_cast<const CqParameterTyped<CqString>*>(pParam)->pValue());
							break;
						}

						case Type_Point:
						case Type_Vector:
						case Type_Normal:
						{
							se=*(static_cast<const CqParameterTyped<CqVector3D>*>(pParam)->pValue());
							break;
						}

						case Type_Color:
						{
							se=*(static_cast<const CqParameterTyped<CqColor>*>(pParam)->pValue());
							break;
						}

						case Type_Matrix:
						{
							se=*(static_cast<const CqParameterTyped<CqMatrix>*>(pParam)->pValue());
							break;
						}
					}
					pV->SetValue(se);
					Ret=1.0f;
				}
			}
		}
	}

	Result.SetValue(0,Ret);
}


//----------------------------------------------------------------------
// rendererinfo
//

STD_SOIMPL CqShaderExecEnv::SO_rendererinfo(STRINGVAL name, CqShaderVariable* pV, DEFPARAMIMPL)
{
	CqString strName=name.Value(strName,0);
	TqFloat Ret=0.0f;
	CqVMStackEntry se;

	if(strName.compare("renderer")==0)
	{
		if((pV->Type()&Type_Mask)==Type_String)
		{			
			se=STRNAME;
			pV->SetValue(se);
			Ret=1.0f;
		}
	}
	else if(strName.compare("version")==0)
	{
		if((pV->Type()&Type_Mask)==Type_Float &&
		   (pV->Type()&Type_Array))
		{			
			CqShaderVariableArray* paV=static_cast<CqShaderVariableArray*>(pV);
			if(paV->ArrayLength()>=4)
			{
				se=static_cast<TqFloat>(VERMAJOR);
				(*paV)[0]->SetValue(se);
				se=static_cast<TqFloat>(VERMINOR);
				(*paV)[1]->SetValue(se);
				se=static_cast<TqFloat>(BUILD);
				(*paV)[2]->SetValue(se);
				se=static_cast<TqFloat>(0.0f);
				(*paV)[3]->SetValue(se);
				Ret=1.0f;
			}
		}
	}
	else if(strName.compare("versionstring")==0)
	{
		if((pV->Type()&Type_Mask)==Type_String)
		{
			se=VERSION_STR;
			pV->SetValue(se);
			Ret=1.0f;
		}
	}

	Result.SetValue(0,Ret);
}


//----------------------------------------------------------------------
// incident

STD_SOIMPL CqShaderExecEnv::SO_incident(STRINGVAL name, CqShaderVariable* pV, DEFPARAMIMPL)
{
	TqFloat Ret=0.0f;
	Result.SetValue(0,Ret);
}


//----------------------------------------------------------------------
// opposite

STD_SOIMPL CqShaderExecEnv::SO_opposite(STRINGVAL name, CqShaderVariable* pV, DEFPARAMIMPL)
{
	TqFloat Ret=0.0f;
	Result.SetValue(0,Ret);
}


//----------------------------------------------------------------------
// ctransform(s,s,c)
STD_SOIMPL CqShaderExecEnv::SO_ctransform(STRINGVAL fromspace, STRINGVAL tospace, COLORVAL c, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(c)
	CHECKVARY(Result)
	FOR_EACHR
		CqColor res(COLOR(c));
		if(STRING(fromspace).compare("hsv"))		res=COLOR(c).hsvtorgb();
		else if(STRING(fromspace).compare("hsl"))	res=COLOR(c).hsltorgb();
		else if(STRING(fromspace).compare("XYZ"))	res=COLOR(c).XYZtorgb();
		else if(STRING(fromspace).compare("xyY"))	res=COLOR(c).xyYtorgb();
		else if(STRING(fromspace).compare("YIQ"))	res=COLOR(c).YIQtorgb();

		if(STRING(tospace).compare("hsv"))		res=COLOR(c).rgbtohsv();
		else if(STRING(tospace).compare("hsl"))	res=COLOR(c).rgbtohsl();
		else if(STRING(tospace).compare("XYZ"))	res=COLOR(c).rgbtoXYZ();
		else if(STRING(tospace).compare("xyY"))	res=COLOR(c).rgbtoxyY();
		else if(STRING(tospace).compare("YIQ"))	res=COLOR(c).rgbtoYIQ();

		Result.SetValue(i,res);
	END_FORR
}


//----------------------------------------------------------------------
// ctransform(s,c)
STD_SOIMPL CqShaderExecEnv::SO_ctransform(STRINGVAL tospace, COLORVAL c, DEFPARAMIMPL)
{
	static CqVMStackEntry fromspace;
	fromspace="rgb";

	assert(pShader!=0);
	SO_ctransform(fromspace, tospace, c, Result, pShader);
}


//----------------------------------------------------------------------
// ctransform(s,c)
STD_SOIMPL CqShaderExecEnv::SO_ptlined(POINTVAL P0, POINTVAL P1, POINTVAL Q, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(P0)
	CHECKVARY(P1)
	CHECKVARY(Q)
	CHECKVARY(Result)
	FOR_EACHR
		CqVector3D kDiff=POINT(Q)-POINT(P0);
		CqVector3D vecDir=POINT(P1)-POINT(P0);
		TqFloat fT=kDiff*vecDir;

		if(fT<=0.0f)
			fT=0.0f;
		else
		{
			TqFloat fSqrLen=vecDir.Magnitude2();
			if(fT>=fSqrLen)
			{
				fT=1.0f;
				kDiff-=vecDir;
			}
			else
			{
				fT/=fSqrLen;
				kDiff-=fT*vecDir;
			}
		}
		Result.SetValue(i,kDiff.Magnitude());
	END_FORR
}


STD_SOIMPL	CqShaderExecEnv::SO_inversesqrt(FLOATVAL x, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(x)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,1.0f/static_cast<TqFloat>(sqrt(FLOAT(x))));
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_match(STRINGVAL a, STRINGVAL b, DEFPARAMIMPL)
{
	// TODO: Do this properly.
	INIT_SOR
	FOR_EACHR
		Result.SetValue(i,0);
	END_FORR
}


//----------------------------------------------------------------------
// pnoise(u,period)
STD_SOIMPL CqShaderExecEnv::SO_fpnoise1(FLOATVAL v, FLOATVAL period, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(v)
	CHECKVARY(period)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,(m_noise.FGNoise1(fmod(FLOAT(v),FLOAT(period)))+1)/2.0f);
	END_FORR
}

//----------------------------------------------------------------------
// pnoise(u,v,uperiod,vperiod)
STD_SOIMPL CqShaderExecEnv::SO_fpnoise2(FLOATVAL u, FLOATVAL v, FLOATVAL uperiod, FLOATVAL vperiod, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(u)
	CHECKVARY(uperiod)
	CHECKVARY(v)
	CHECKVARY(vperiod)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,( m_noise.FGNoise2( fmod(FLOAT(u),FLOAT(uperiod)),
											  fmod(FLOAT(v),FLOAT(vperiod)) )+1)/2.0f);
	END_FORR
}

//----------------------------------------------------------------------
// pnoise(p,pperiod)
STD_SOIMPL CqShaderExecEnv::SO_fpnoise3(POINTVAL p, POINTVAL pperiod, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(p)
	CHECKVARY(pperiod)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,(m_noise.FGNoise3(CqVector3D(fmod(POINT(p).x(),POINT(pperiod).x()), 
													   fmod(POINT(p).y(),POINT(pperiod).y()), 
													   fmod(POINT(p).z(),POINT(pperiod).z())) )+1)/2.0f);
	END_FORR
}

//----------------------------------------------------------------------
// pnoise(p,t,pperiod,tperiod)
STD_SOIMPL CqShaderExecEnv::SO_fpnoise4(POINTVAL p, FLOATVAL t, POINTVAL pperiod, FLOATVAL tperiod, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(p)
	CHECKVARY(pperiod)
	CHECKVARY(t)
	CHECKVARY(tperiod)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,(m_noise.FGNoise3(CqVector3D(fmod(POINT(p).x(),POINT(pperiod).x()), 
													   fmod(POINT(p).y(),POINT(pperiod).y()), 
													   fmod(POINT(p).z(),POINT(pperiod).z())) )+1)/2.0f);
	END_FORR
}

//----------------------------------------------------------------------
// pnoise(u,period)
STD_SOIMPL CqShaderExecEnv::SO_cpnoise1(FLOATVAL v, FLOATVAL period, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(v)
	CHECKVARY(period)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,(m_noise.CGNoise1(fmod(FLOAT(v),FLOAT(period)))+1)/2.0f);
	END_FORR
}

//----------------------------------------------------------------------
// pnoise(u,v,uperiod,vperiod)
STD_SOIMPL CqShaderExecEnv::SO_cpnoise2(FLOATVAL u, FLOATVAL v, FLOATVAL uperiod, FLOATVAL vperiod, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(u)
	CHECKVARY(uperiod)
	CHECKVARY(v)
	CHECKVARY(vperiod)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,(m_noise.CGNoise2( fmod(FLOAT(u),FLOAT(uperiod)),
											 fmod(FLOAT(v),FLOAT(vperiod)) )+1)/2.0f);
	END_FORR
}

//----------------------------------------------------------------------
// pnoise(p,pperiod)
STD_SOIMPL CqShaderExecEnv::SO_cpnoise3(POINTVAL p, POINTVAL pperiod, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(p)
	CHECKVARY(pperiod)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,(m_noise.CGNoise3(CqVector3D(fmod(POINT(p).x(),POINT(pperiod).x()), 
													   fmod(POINT(p).y(),POINT(pperiod).y()), 
													   fmod(POINT(p).z(),POINT(pperiod).z())) )+1)/2.0f);
	END_FORR
}

//----------------------------------------------------------------------
// pnoise(p,t,pperiod,tperiod)
STD_SOIMPL CqShaderExecEnv::SO_cpnoise4(POINTVAL p, FLOATVAL t, POINTVAL pperiod, FLOATVAL tperiod, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(p)
	CHECKVARY(pperiod)
	CHECKVARY(t)
	CHECKVARY(tperiod)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,(m_noise.CGNoise3(CqVector3D(fmod(POINT(p).x(),POINT(pperiod).x()), 
													   fmod(POINT(p).y(),POINT(pperiod).y()), 
													   fmod(POINT(p).z(),POINT(pperiod).z())) )+1)/2.0f);
	END_FORR
}

//----------------------------------------------------------------------
// pnoise(u,period)
STD_SOIMPL CqShaderExecEnv::SO_ppnoise1(FLOATVAL v, FLOATVAL period, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(v)
	CHECKVARY(period)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,(m_noise.PGNoise1(fmod(FLOAT(v),FLOAT(period)))+1)/2.0f);
	END_FORR
}

//----------------------------------------------------------------------
// pnoise(u,v,uperiod,vperiod)
STD_SOIMPL CqShaderExecEnv::SO_ppnoise2(FLOATVAL u, FLOATVAL v, FLOATVAL uperiod, FLOATVAL vperiod, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(u)
	CHECKVARY(uperiod)
	CHECKVARY(v)
	CHECKVARY(vperiod)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,(m_noise.PGNoise2( fmod(FLOAT(u),FLOAT(uperiod)),
											 fmod(FLOAT(v),FLOAT(vperiod)) )+1)/2.0f);
	END_FORR
}

//----------------------------------------------------------------------
// pnoise(p,pperiod)
STD_SOIMPL CqShaderExecEnv::SO_ppnoise3(POINTVAL p, POINTVAL pperiod, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(p)
	CHECKVARY(pperiod)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,(m_noise.PGNoise3(CqVector3D(fmod(POINT(p).x(),POINT(pperiod).x()), 
													   fmod(POINT(p).y(),POINT(pperiod).y()), 
													   fmod(POINT(p).z(),POINT(pperiod).z())) )+1)/2.0f);
	END_FORR
}

//----------------------------------------------------------------------
// pnoise(p,t,pperiod,tperiod)
STD_SOIMPL CqShaderExecEnv::SO_ppnoise4(POINTVAL p, FLOATVAL t, POINTVAL pperiod, FLOATVAL tperiod, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(p)
	CHECKVARY(pperiod)
	CHECKVARY(t)
	CHECKVARY(tperiod)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,(m_noise.PGNoise3(CqVector3D(fmod(POINT(p).x(),POINT(pperiod).x()), 
													   fmod(POINT(p).y(),POINT(pperiod).y()), 
													   fmod(POINT(p).z(),POINT(pperiod).z())) )+1)/2.0f);
	END_FORR
}


//----------------------------------------------------------------------
// rotate(Q,angle,P0,P1)
STD_SOIMPL CqShaderExecEnv::SO_rotate(VECTORVAL Q, FLOATVAL angle, POINTVAL P0, POINTVAL P1, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(Q)
	CHECKVARY(angle)
	CHECKVARY(P0)
	CHECKVARY(P1)
	CHECKVARY(Result)
	FOR_EACHR
		CqMatrix matR(FLOAT(angle), POINT(P1)-POINT(P0));

		CqVector3D	Res(VECTOR(Q));
		Res=matR*Res;

		Result.SetValue(i,Res);
	END_FORR
}

//----------------------------------------------------------------------
// filterstep(edge,s1)
STD_SOIMPL CqShaderExecEnv::SO_filterstep(FLOATVAL edge, FLOATVAL s1, DEFPARAMVARIMPL)
{
	GET_FILTER_PARAMS;

	INIT_SOR
	CHECKVARY(edge)
	CHECKVARY(s1)
	CHECKVARY(Result)
	FOR_EACHR
		TqFloat dsdu=SO_DuType<TqFloat>(s1,m_GridI,*this);
		TqFloat dsdv=SO_DvType<TqFloat>(s1,m_GridI,*this);
		
		TqFloat w=fabs(dsdu*du()*dsdv*dv());	
		w*=_pswidth;

		Result.SetValue(i,CLAMP((FLOAT(s1)+w/2.0f-FLOAT(edge))/w,0,1));
	END_FORR
}

//----------------------------------------------------------------------
// filterstep(edge,s1,s2)
STD_SOIMPL CqShaderExecEnv::SO_filterstep2(FLOATVAL edge, FLOATVAL s1, FLOATVAL s2, DEFPARAMVARIMPL)
{
	GET_FILTER_PARAMS;

	INIT_SOR
	CHECKVARY(edge)
	CHECKVARY(s1)
	CHECKVARY(s2)
	CHECKVARY(Result)
	FOR_EACHR
		TqFloat w=FLOAT(s2)-FLOAT(s1);
		w*=_pswidth;
		Result.SetValue(i,CLAMP((FLOAT(s1)+w/2.0f-FLOAT(edge))/w,0,1));
	END_FORR
}

//----------------------------------------------------------------------
// specularbrdf(L,N,V,rough)
STD_SOIMPL CqShaderExecEnv::SO_specularbrdf(VECTORVAL L, NORMALVAL N, VECTORVAL V, FLOATVAL rough, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(L)
	CHECKVARY(N)
	CHECKVARY(V)
	CHECKVARY(rough)
	CHECKVARY(Result)
	FOR_EACHR
		CqVector3D	Ln=VECTOR(L);	Ln.Unit();
		CqVector3D	H=Ln+VECTOR(V);	H.Unit();
		// NOTE: The (roughness/8) term emulates the BMRT behaviour for prmanspecular.
		Result.SetValue(i,Cl()*pow(MAX(0.0f,NORMAL(N)*H),1.0f/(FLOAT(rough)/8.0f)));
	END_FORR
}


//----------------------------------------------------------------------
// determinant(m)
STD_SOIMPL CqShaderExecEnv::SO_determinant(MATRIXVAL M, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(M)
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,MATRIX(M).Determinant());
	END_FORR
}


//----------------------------------------------------------------------
// translate(m,v)
STD_SOIMPL CqShaderExecEnv::SO_mtranslate(MATRIXVAL M, VECTORVAL V, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(M)
	CHECKVARY(V)
	CHECKVARY(Result)
	FOR_EACHR
		CqMatrix mat=MATRIX(M);
		mat.Translate(VECTOR(V));
		Result.SetValue(i,mat);
	END_FORR
}

//----------------------------------------------------------------------
// rotate(m,v)
STD_SOIMPL CqShaderExecEnv::SO_mrotate(MATRIXVAL M, FLOATVAL angle, VECTORVAL axis, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(M)
	CHECKVARY(angle)
	CHECKVARY(axis)
	CHECKVARY(Result)
	FOR_EACHR
		CqMatrix mat=MATRIX(M);
		mat.Rotate(FLOAT(angle),VECTOR(axis));
		Result.SetValue(i,mat);
	END_FORR
}

//----------------------------------------------------------------------
// scale(m,p)
STD_SOIMPL CqShaderExecEnv::SO_mscale(MATRIXVAL M, POINTVAL S, DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(M)
	CHECKVARY(S)
	CHECKVARY(Result)
	FOR_EACHR
		CqVector3D& vs=POINT(S);
		CqMatrix mat=MATRIX(M);
		mat.Scale(vs.x(),vs.y(),vs.z());
		Result.SetValue(i,mat);
	END_FORR
}


//----------------------------------------------------------------------
// setmcomp(p,v)
STD_SOIMPL	CqShaderExecEnv::SO_setmcomp(POINTVAL m, FLOATVAL r,FLOATVAL c, FLOATVAL v, DEFVOIDPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(m)
	CHECKVARY(r)
	CHECKVARY(c)
	CHECKVARY(v)
	FOR_EACHR
		CqMatrix t=MATRIX(m);
		t[static_cast<TqInt>(FLOAT(r))][static_cast<TqInt>(FLOAT(c))]=FLOAT(v);
		m.SetValue(i,t);
	END_FORR
}


//----------------------------------------------------------------------
// spline(value, f1,f2,...,fn)
STD_SOIMPL	CqShaderExecEnv::SO_fsplinea(FLOATVAL value, FLOATARRAYVAL a, DEFPARAMIMPL)
{
	assert(a.fVariable());
	assert(a.pVariable()->Type()&Type_Array);
	assert(a.pVariable()->Type()&Type_Float);

	CqShaderVariableArray* pArray=static_cast<CqShaderVariableArray*>(a.pVariable());
	TqInt	cParams=pArray->aVariables().size();
	CqSplineCubic spline(cParams);

	INIT_SOR
	CHECKVARY(value)
	CHECKVARY(a)
	CHECKVARY(Result)
	FOR_EACHR
		if(FLOAT(value)>=1.0f)	pArray->aVariables()[cParams-2]->GetValue(i,Result);
		else if(FLOAT(value)<=0.0f)	pArray->aVariables()[1]->GetValue(i,Result);
		else
		{
			TqInt j;
			for(j=0; j<cParams; j++)
				spline[j]=CqVector4D(static_cast<CqShaderVariableTyped<TqFloat>*>(pArray->aVariables()[j])->GetValue(),0.0f,0.0f,1.0f);
			
			CqVector4D	res=spline.Evaluate(FLOAT(value));
			Result.SetValue(i,res.x());
		}
	END_FORR
}


//----------------------------------------------------------------------
// spline(value, f1,f2,...,fn)
STD_SOIMPL	CqShaderExecEnv::SO_csplinea(FLOATVAL value, COLORARRAYVAL a, DEFPARAMIMPL)
{
	assert(a.fVariable());
	assert(a.pVariable()->Type()&Type_Array);
	assert(a.pVariable()->Type()&Type_Color);

	CqShaderVariableArray* pArray=static_cast<CqShaderVariableArray*>(a.pVariable());
	TqInt	cParams=pArray->aVariables().size();
	CqSplineCubic spline(cParams);

	INIT_SOR
	CHECKVARY(value)
	CHECKVARY(a)
	CHECKVARY(Result)
	FOR_EACHR
		if(FLOAT(value)>=1.0f)	pArray->aVariables()[cParams-2]->GetValue(i,Result);
		else if(FLOAT(value)<=0.0f)	pArray->aVariables()[1]->GetValue(i,Result);
		else
		{
			TqInt j;
			for(j=0; j<cParams; j++)
			{
				CqShaderVariableTyped<CqColor>* pVar=static_cast<CqShaderVariableTyped<CqColor>*>(pArray->aVariables()[j]);
				spline[j]=CqVector4D(pVar->GetValue().fRed(),pVar->GetValue().fGreen(),pVar->GetValue().fBlue(),1.0f);
			}
			
			CqVector4D	res=spline.Evaluate(FLOAT(value));
			Result.SetValue(i,CqColor(res.x(),res.y(),res.z()));
		}
	END_FORR
}


//----------------------------------------------------------------------
// spline(value, f1,f2,...,fn)
STD_SOIMPL	CqShaderExecEnv::SO_psplinea(FLOATVAL value, POINTARRAYVAL a, DEFPARAMIMPL)
{
	assert(a.fVariable());
	assert(a.pVariable()->Type()&Type_Array);
	assert(a.pVariable()->Type()&Type_Point);

	CqShaderVariableArray* pArray=static_cast<CqShaderVariableArray*>(a.pVariable());
	TqInt	cParams=pArray->aVariables().size();
	CqSplineCubic spline(cParams);

	INIT_SOR
	CHECKVARY(value)
	CHECKVARY(a)
	CHECKVARY(Result)
	FOR_EACHR
		if(FLOAT(value)>=1.0f)	pArray->aVariables()[cParams-2]->GetValue(i,Result);
		else if(FLOAT(value)<=0.0f)	pArray->aVariables()[1]->GetValue(i,Result);
		else
		{
			TqInt j;
			for(j=0; j<cParams; j++)
			{
				CqShaderVariableTyped<CqVector3D>* pVar=static_cast<CqShaderVariableTyped<CqVector3D>*>(pArray->aVariables()[j]);
				spline[j]=pVar->GetValue();
			}
			
			CqVector4D	res=spline.Evaluate(FLOAT(value));
			Result.SetValue(i,res);
		}
	END_FORR
}


//----------------------------------------------------------------------
// spline(value, f1,f2,...,fn)
STD_SOIMPL	CqShaderExecEnv::SO_sfsplinea(STRINGVAL basis, FLOATVAL value, FLOATARRAYVAL a, DEFPARAMIMPL)
{
	assert(a.fVariable());
	assert(a.pVariable()->Type()&Type_Array);
	assert(a.pVariable()->Type()&Type_Float);

	CqShaderVariableArray* pArray=static_cast<CqShaderVariableArray*>(a.pVariable());
	TqInt	cParams=pArray->aVariables().size();
	CqSplineCubic spline(cParams);
	spline.SetmatBasis(basis.Value(temp_string,0));

	INIT_SOR
	CHECKVARY(value)
	CHECKVARY(a)
	CHECKVARY(Result)
	FOR_EACHR
		if(FLOAT(value)>=1.0f)	pArray->aVariables()[cParams-2]->GetValue(i,Result);
		else if(FLOAT(value)<=0.0f)	pArray->aVariables()[1]->GetValue(i,Result);
		else
		{
			TqInt j;
			for(j=0; j<cParams; j++)
				spline[j]=CqVector4D(static_cast<CqShaderVariableTyped<TqFloat>*>(pArray->aVariables()[j])->GetValue(),0.0f,0.0f,1.0f);
			
			CqVector4D	res=spline.Evaluate(FLOAT(value));
			Result.SetValue(i,res.x());
		}
	END_FORR
}


//----------------------------------------------------------------------
// spline(value, f1,f2,...,fn)
STD_SOIMPL	CqShaderExecEnv::SO_scsplinea(STRINGVAL basis, FLOATVAL value, COLORARRAYVAL a, DEFPARAMIMPL)
{
	assert(a.fVariable());
	assert(a.pVariable()->Type()&Type_Array);
	assert(a.pVariable()->Type()&Type_Color);

	CqShaderVariableArray* pArray=static_cast<CqShaderVariableArray*>(a.pVariable());
	TqInt	cParams=pArray->aVariables().size();
	CqSplineCubic spline(cParams);
	spline.SetmatBasis(basis.Value(temp_string,0));

	INIT_SOR
	CHECKVARY(value)
	CHECKVARY(a)
	CHECKVARY(Result)
	FOR_EACHR
		if(FLOAT(value)>=1.0f)	pArray->aVariables()[cParams-2]->GetValue(i,Result);
		else if(FLOAT(value)<=0.0f)	pArray->aVariables()[1]->GetValue(i,Result);
		else
		{
			TqInt j;
			for(j=0; j<cParams; j++)
			{
				CqShaderVariableTyped<CqColor>* pVar=static_cast<CqShaderVariableTyped<CqColor>*>(pArray->aVariables()[j]);
				spline[j]=CqVector4D(pVar->GetValue().fRed(),pVar->GetValue().fGreen(),pVar->GetValue().fBlue(),1.0f);
			}
			
			CqVector4D	res=spline.Evaluate(FLOAT(value));
			Result.SetValue(i,CqColor(res.x(),res.y(),res.z()));
		}
	END_FORR
}


//----------------------------------------------------------------------
// spline(value, f1,f2,...,fn)
STD_SOIMPL	CqShaderExecEnv::SO_spsplinea(STRINGVAL basis, FLOATVAL value, POINTARRAYVAL a, DEFPARAMIMPL)
{
	assert(a.fVariable());
	assert(a.pVariable()->Type()&Type_Array);
	assert(a.pVariable()->Type()&Type_Point);

	CqShaderVariableArray* pArray=static_cast<CqShaderVariableArray*>(a.pVariable());
	TqInt	cParams=pArray->aVariables().size();
	CqSplineCubic spline(cParams);
	spline.SetmatBasis(basis.Value(temp_string,0));

	INIT_SOR
	CHECKVARY(value)
	CHECKVARY(a)
	CHECKVARY(Result)
	FOR_EACHR
		if(FLOAT(value)>=1.0f)	pArray->aVariables()[cParams-2]->GetValue(i,Result);
		else if(FLOAT(value)<=0.0f)	pArray->aVariables()[1]->GetValue(i,Result);
		else
		{
			TqInt j;
			for(j=0; j<cParams; j++)
			{
				CqShaderVariableTyped<CqVector3D>* pVar=static_cast<CqShaderVariableTyped<CqVector3D>*>(pArray->aVariables()[j]);
				spline[j]=pVar->GetValue();
			}
			
			CqVector4D	res=spline.Evaluate(FLOAT(value));
			Result.SetValue(i,res);
		}
	END_FORR
}


//----------------------------------------------------------------------
// shadername()
STD_SOIMPL	CqShaderExecEnv::SO_shadername(DEFPARAMIMPL)
{
	INIT_SOR
	CHECKVARY(Result)
	FOR_EACHR
		Result.SetValue(i,pShader->strName());
	END_FORR
}


//----------------------------------------------------------------------
// shadername(s)
STD_SOIMPL	CqShaderExecEnv::SO_shadername2(STRINGVAL shader, DEFPARAMIMPL)
{
	CqString strName("");
	CqString strShader;
	CqShader* pSurface=m_pSurface->pAttributes()->pshadSurface();
	CqShader* pDisplacement=m_pSurface->pAttributes()->pshadDisplacement();
	CqShader* pAtmosphere=m_pSurface->pAttributes()->pshadAtmosphere();

	INIT_SOR
	CHECKVARY(Result)
	FOR_EACHR
		strName="";
		strShader=shader.Value(strShader,0);
		if(strShader.compare("surface")==0 && pSurface!=0)	strName=pSurface->strName();
		else if(strShader.compare("displacement")==0 && pDisplacement!=0)	strName=pDisplacement->strName();
		else if(strShader.compare("atmosphere")==0 && pAtmosphere!=0)	strName=pAtmosphere->strName();
		Result.SetValue(i,strName);
	END_FORR
}


END_NAMESPACE(Aqsis)
//---------------------------------------------------------------------
