// Aqsis
// Copyright (C) 1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.org
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
 * \brief Declares macros used to ease the creation of CqShaderVM shadops
 *
 * These include macros for handling the stack, and macros for creating
 * shadeops which call through to the shader execution environment functions.
 *
 * \author Paul C. Gregory (pgregory@aqsis.org)
 */

#ifndef SHADEOPMACROS_H_INCLUDED
#define SHADEOPMACROS_H_INCLUDED

//----------------------------------------------------------------------
// Stack management
#define	POPV(A)			SqStackEntry _se_##A=Pop(__fVarying); \
						IqShaderData* A = _se_##A.m_Data
#define	POP				Pop(__fVarying)
#define	RELEASE(A)		Release( _se_##A )
#define	RESULT(t,c)		IqShaderData* pResult=GetNextTemp(t,c); \
						pResult->SetSize(m_shadingPointCount)

#define CONSTFUNC		// Uniform function
#define	VARFUNC			bool __fVarying=true;
#define	AUTOFUNC		bool __fVarying=false;

//----------------------------------------------------------------------
// Macros for declaring shadeops which call a function with arguments and a
// result output value
#define	FUNC(t,Func)	RESULT(t,__fVarying?class_varying:class_uniform); \
						if(m_pEnv->IsRunning()) \
							Func(pResult,this); \
						Push(pResult);
#define	FUNC1(t,Func)	POPV(ValA); \
						RESULT(t,__fVarying?class_varying:class_uniform); \
						if(m_pEnv->IsRunning()) \
							Func(ValA,pResult,this); \
						Push(pResult); \
						RELEASE(ValA);
#define	FUNC2(t,Func)	POPV(ValA); \
						POPV(ValB); \
						RESULT(t,__fVarying?class_varying:class_uniform); \
						if(m_pEnv->IsRunning()) \
							Func(ValA,ValB,pResult,this); \
						Push(pResult); \
						RELEASE(ValA); \
						RELEASE(ValB);
#define	FUNC3(t,Func)	POPV(ValA); \
						POPV(ValB); \
						POPV(ValC); \
						RESULT(t,__fVarying?class_varying:class_uniform); \
						if(m_pEnv->IsRunning()) \
							Func(ValA,ValB,ValC,pResult,this); \
						Push(pResult); \
						RELEASE(ValA); \
						RELEASE(ValB); \
						RELEASE(ValC);
#define	FUNC4(t,Func)	POPV(ValA); \
						POPV(ValB); \
						POPV(ValC); \
						POPV(ValD); \
						RESULT(t,__fVarying?class_varying:class_uniform); \
						if(m_pEnv->IsRunning()) \
							Func(ValA,ValB,ValC,ValD,pResult,this); \
						Push(pResult); \
						RELEASE(ValA); \
						RELEASE(ValB); \
						RELEASE(ValC); \
						RELEASE(ValD);
#define	FUNC5(t,Func)	POPV(ValA); \
						POPV(ValB); \
						POPV(ValC); \
						POPV(ValD); \
						POPV(ValE); \
						RESULT(t,__fVarying?class_varying:class_uniform); \
						if(m_pEnv->IsRunning()) \
							Func(ValA,ValB,ValC,ValD,ValE,pResult,this); \
						Push(pResult); \
						RELEASE(ValA); \
						RELEASE(ValB); \
						RELEASE(ValC); \
						RELEASE(ValD); \
						RELEASE(ValE);
#define	FUNC7(t,Func)	POPV(ValA); \
						POPV(ValB); \
						POPV(ValC); \
						POPV(ValD); \
						POPV(ValE); \
						POPV(ValF); \
						POPV(ValG); \
						RESULT(t,__fVarying?class_varying:class_uniform); \
						if(m_pEnv->IsRunning()) \
							Func(ValA,ValB,ValC,ValD,ValE,ValF,ValG,pResult,this); \
						Push(pResult); \
						RELEASE(ValA); \
						RELEASE(ValB); \
						RELEASE(ValC); \
						RELEASE(ValD); \
						RELEASE(ValE); \
						RELEASE(ValF); \
						RELEASE(ValG);
#define FUNC1PLUS(t,Func)	POPV(count);	/* Count of additional values.*/ \
						POPV(a);		/* first value */ \
						/* Read all the additional values. */ \
						TqFloat fc; \
						count->GetFloat( fc ); \
						TqInt cParams=static_cast<TqInt>( fc ); \
						IqShaderData** aParams=new IqShaderData*[cParams]; \
						SqStackEntry *stackitems = new SqStackEntry[cParams];\
						TqInt iP=0; \
						while(iP!=cParams)	{\
							stackitems[iP]=POP; \
							aParams[iP]=stackitems[iP].m_Data;\
							iP++;\
						}\
						RESULT(t,__fVarying?class_varying:class_uniform); \
						if(m_pEnv->IsRunning()) \
							Func(a,pResult,this, cParams, aParams); \
						delete[](aParams); \
						iP=0; \
						while(iP!=cParams)	{\
							Release( stackitems[iP]);\
							iP++;\
						}\
						delete[](stackitems); \
						Push(pResult); \
						RELEASE(count); \
						RELEASE(a);
#define FUNC2PLUS(t,Func)	POPV(count);	/* Count of additional values.*/ \
						POPV(a);		/* first value */ \
						POPV(b);		/* second value */ \
						/* Read all the additional values. */ \
						TqFloat fc; \
						count->GetFloat( fc ); \
						TqInt cParams=static_cast<TqInt>( fc ); \
						IqShaderData** aParams=new IqShaderData*[cParams]; \
						SqStackEntry *stackitems = new SqStackEntry[cParams];\
						TqInt iP=0; \
						while(iP!=cParams)	{\
							stackitems[iP]=POP; \
							aParams[iP]=stackitems[iP].m_Data;\
							iP++;\
						}\
						RESULT(t,__fVarying?class_varying:class_uniform); \
						if(m_pEnv->IsRunning()) \
							Func(a,b,pResult,this, cParams, aParams); \
						delete[](aParams); \
						iP=0; \
						while(iP!=cParams)	{\
							Release( stackitems[iP]);\
							iP++;\
						}\
						delete[](stackitems); \
						Push(pResult); \
						RELEASE(count); \
						RELEASE(a); \
						RELEASE(b);
#define FUNC3PLUS(t,Func)	POPV(count);	/* Count of additional values.*/ \
						POPV(a);		/* first value */ \
						POPV(b);		/* second value */ \
						POPV(c);		/* third value */ \
						/* Read all the additional values. */ \
						TqFloat fc; \
						count->GetFloat( fc ); \
						TqInt cParams=static_cast<TqInt>( fc ); \
						IqShaderData** aParams=new IqShaderData*[cParams]; \
						SqStackEntry *stackitems = new SqStackEntry[cParams];\
						TqInt iP=0; \
						while(iP!=cParams)	{\
							stackitems[iP]=POP; \
							aParams[iP]=stackitems[iP].m_Data;\
							iP++;\
						}\
						RESULT(t,__fVarying?class_varying:class_uniform); \
						if(m_pEnv->IsRunning()) \
							Func(a,b,c,pResult,this, cParams, aParams); \
						delete[](aParams); \
						iP=0; \
						while(iP!=cParams)	{\
							Release( stackitems[iP]);\
							iP++;\
						}\
						delete[](stackitems); \
						Push(pResult); \
						RELEASE(count); \
						RELEASE(a); \
						RELEASE(b); \
						RELEASE(c);

#define FUNC4PLUS(t,Func)	POPV(count);	/* Count of additional values.*/ \
						POPV(a);		/* first value */ \
						POPV(b);		/* second value */ \
						POPV(c);		/* third value */ \
                  POPV(d);    /* fourth value */ \
						/* Read all the additional values. */ \
						TqFloat fc; \
						count->GetFloat( fc ); \
						TqInt cParams=static_cast<TqInt>( fc ); \
						IqShaderData** aParams=new IqShaderData*[cParams]; \
						SqStackEntry *stackitems = new SqStackEntry[cParams];\
						TqInt iP=0; \
						while(iP!=cParams)	{\
							stackitems[iP]=POP; \
							aParams[iP]=stackitems[iP].m_Data;\
							iP++;\
						}\
						RESULT(t,__fVarying?class_varying:class_uniform); \
						if(m_pEnv->IsRunning()) \
							Func(a,b,c,d,pResult,this, cParams, aParams); \
						delete[](aParams); \
						iP=0; \
						while(iP!=cParams)	{\
							Release( stackitems[iP]);\
							iP++;\
						}\
						delete[](stackitems); \
						Push(pResult); \
						RELEASE(count); \
						RELEASE(a); \
						RELEASE(b); \
						RELEASE(c); \
                  RELEASE(d);

//----------------------------------------------------------------------
// Macros for declaring shadeops which call a function with arguments but no
// return value.
#define	VOIDFUNC(Func)	Func(this);
#define	VOIDFUNC1(Func)	POPV(ValA); \
						if(m_pEnv->IsRunning()) \
							Func(ValA,this); \
						RELEASE(ValA);
#define	VOIDFUNC2(Func)	POPV(ValA); \
						POPV(ValB); \
						if(m_pEnv->IsRunning()) \
							Func(ValA,ValB,this); \
						RELEASE(ValA); \
						RELEASE(ValB);
#define	VOIDFUNC3(Func)	POPV(ValA); \
						POPV(ValB); \
						POPV(ValC); \
						if(m_pEnv->IsRunning()) \
							Func(ValA,ValB,ValC,this); \
						RELEASE(ValA); \
						RELEASE(ValB); \
						RELEASE(ValC);
#define	VOIDFUNC4(Func)	POPV(ValA); \
						POPV(ValB); \
						POPV(ValC); \
						POPV(ValD); \
						if(m_pEnv->IsRunning()) \
							Func(ValA,ValB,ValC,ValD,this); \
						RELEASE(ValA); \
						RELEASE(ValB); \
						RELEASE(ValC); \
						RELEASE(ValD);
#define	VOIDFUNC5(Func)	POPV(ValA); \
						POPV(ValB); \
						POPV(ValC); \
						POPV(ValD); \
						POPV(ValE); \
						if(m_pEnv->IsRunning()) \
							Func(ValA,ValB,ValC,ValD,ValE,this); \
						RELEASE(ValA); \
						RELEASE(ValB); \
						RELEASE(ValC); \
						RELEASE(ValD); \
						RELEASE(ValE);
#define	VOIDFUNC7(Func)	POPV(ValA); \
						POPV(ValB); \
						POPV(ValC); \
						POPV(ValD); \
						POPV(ValE); \
						POPV(ValF); \
						POPV(ValG); \
						if(m_pEnv->IsRunning()) \
							Func(ValA,ValB,ValC,ValD,ValE,ValF,ValG,this); \
						RELEASE(ValA); \
						RELEASE(ValB); \
						RELEASE(ValC); \
						RELEASE(ValD); \
						RELEASE(ValE); \
						RELEASE(ValF); \
						RELEASE(ValG);
#define VOIDFUNC1PLUS(Func)	POPV(count);	/* Count of additional values.*/ \
						POPV(a);		/* first value */ \
						/* Read all the additional values. */ \
						TqFloat fc; \
						count->GetFloat( fc ); \
						TqInt cParams=static_cast<TqInt>( fc ); \
						IqShaderData** aParams=new IqShaderData*[cParams]; \
						SqStackEntry *stackitems = new SqStackEntry[cParams];\
						TqInt iP=0; \
						while(iP!=cParams)	{\
							stackitems[iP]=POP; \
							aParams[iP]=stackitems[iP].m_Data;\
							iP++;\
						}\
						if(m_pEnv->IsRunning()) \
							Func(a,this, cParams, aParams); \
						delete[](aParams); \
                  				iP=0; \
						while(iP!=cParams)	{\
							Release( stackitems[iP]);\
							iP++;\
						}\
                  				delete[](stackitems); \
						RELEASE(count); \
						RELEASE(a);
#define VOIDFUNC5PLUS(Func)	POPV(count);	/* Count of additional values.*/ \
						POPV(a); \
						POPV(b); \
						POPV(c); \
						POPV(d); \
						POPV(e); \
						/* Read all the additional values. */ \
						TqFloat fc; \
						count->GetFloat( fc ); \
						TqInt cParams=static_cast<TqInt>( fc ); \
						IqShaderData** aParams=new IqShaderData*[cParams]; \
						SqStackEntry *stackitems = new SqStackEntry[cParams];\
						TqInt iP=0; \
						while(iP!=cParams)	{\
							stackitems[iP]=POP; \
							aParams[iP]=stackitems[iP].m_Data;\
							iP++;\
						}\
						if(m_pEnv->IsRunning()) \
							Func(a,b,c,d,e, this, cParams, aParams); \
						delete[](aParams); \
                  				iP=0; \
						while(iP!=cParams)	{\
							Release( stackitems[iP]);\
							iP++;\
						}\
						delete[](stackitems); \
						RELEASE(count); \
						RELEASE(a); \
						RELEASE(b); \
						RELEASE(c); \
						RELEASE(d); \
						RELEASE(e);

//----------------------------------------------------------------------
#endif // SHADEOPMACROS_H_INCLUDED
