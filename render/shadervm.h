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
		\brief Declares classes and support functionality for the shader virtual machine.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is .h included already?
#ifndef SHADERVM_H_INCLUDED
#define SHADERVM_H_INCLUDED 1

#include	<vector>

#include	"specific.h"	// Needed for namespace macros.

#include	"ri.h"
#include	"bitvector.h"
#include	"vector3d.h"
#include	"color.h"
#include	"exception.h"
#include	"sstring.h"
#include	"matrix.h"
#include	"noise.h"
#include	"shadervariable.h"
#include	"shaderexecenv.h"
#include	"shaderstack.h"

#define		_qShareName	CORE
#include	"share.h"

START_NAMESPACE(Aqsis)

//----------------------------------------------------------------------
/** \enum EqShaderType
 * Shader type identifier.
 */

enum EqShaderType
{
	Type_Surface,			///< Surface shader
	Type_Lightsource,		///< Lightsource shader.
	Type_Volume,			///< Volume shader.
	Type_Displacement,		///< Displacement shader.
	Type_Transformation,	///< Transformation shader.
	Type_Imager,			///< Image shader.
};


class CqShaderVM;

//----------------------------------------------------------------------
/** \struct SqOpCodeTrans
 * Structure for translating an opcode in an slx file.
 */

struct SqOpCodeTrans
{
	char*	m_strName;					///< Name of the opcode.
	void(CqShaderVM::*m_pCommand)();	///< Member pointer to the function.
	TqInt	m_cParams;					///< Number of expected parameters.
	EqVariableType m_aParamTypes[10];	///< Array of parameter types (up to ten, can be extended if required).
};


_qShareM	extern char*	gShaderTypeNames[];
_qShareM	extern TqInt		gcShaderTypeNames;

extern	CqVMStackEntry		gUniformResult;	///< Global stackentry for storing the result of an opcode prior to putting it onto the stack.
extern	CqVMStackEntry		gVaryingResult; ///< Global stackentry for storing the result of an opcode prior to putting it onto the stack.

#define	USES(a,b)	((a)&(0x00000001<<(b))?TqTrue:TqFalse)

class CqShaderRegister;

//----------------------------------------------------------------------
/** \class CqShader
 * Abstract base class from which all shaders must be defined.
 */

class _qShareC  CqShader
{
	public:
			_qShareM 	CqShader() : m_Uses(0xFFFFFFFF)	{}
	virtual	_qShareM 	~CqShader()	{}

	static	_qShareM 	void	RegisterShader(const char* strName, EqShaderType type, CqShader* pShader);
	static	_qShareM 	CqShaderRegister* FindShader(const char* strName, EqShaderType type);
	static	_qShareM 	CqShader* CreateShader(const char* strName, EqShaderType type);
						/** Get the shader matrix, the transformation at the time this shader was instantiated.
						 */
			_qShareM 	CqMatrix&	matCurrent()	{return(m_matCurrent);}
						/** Set the naem of the shader.
						 */
			_qShareM	void	SetstrName(const char* strName)
													{m_strName=strName;}
						/** Get the name of this shader.
						 */
			_qShareM	const CqString& strName() const	{return(m_strName);}
	
						/** Set a named shader paramter to the specified value.
						 * \param name Character pointer to parameter name.
						 * \param val Character pointer to the vale storage, will be cast to a pointer of the appropriate type.
						 */
	virtual	_qShareM 	void	SetValue(const char* name, TqPchar val)			{}
						/** Get the value of a named shader paramter.
						 * \param name The name of the shader paramter.
						 * \param res CqShaderVariable pointer to store the result in, will be typechecked for suitability.
						 * \return Boolean indicating the parameter existed and res was of an appropriate type.
						 */
	virtual	_qShareM 	TqBool	GetValue(const char* name, CqShaderVariable* res)	{return(TqFalse);}
						/** Evaluate the shader code.
						 * \param Env The shader execution environment to evaluate within.
						 */
	virtual	_qShareM 	void	Evaluate(CqShaderExecEnv& Env)			{}
						/** Initialise the state of any arguments with default values.
						 */
	virtual	_qShareM	void	PrepareDefArgs()						{}
						/** Prepare the shader for evaluation.
						 * \param uGridRes The resolution of the grid being shaded in u.
						 * \param vGridRes The resolution of the grid being shaded in v
						 * \param Env Pointer to the CqShaderExecEnv to evaluate within.
						 */
	virtual _qShareM 	void	Initialise(const TqInt uGridRes, const TqInt vGridRes, CqShaderExecEnv& Env)	{}
						/** Determine whether this shader is an aambient ligthsource shader.
						 * i.e. A lightsource shader with no Illuminate or Solar constructs.
						 */
	virtual	_qShareM 	TqBool	fAmbient() const{return(TqFalse);}
						/** Duplicate this shader.
						 * \return A pointer to a new shader.
						 */
	virtual _qShareM 	CqShader*	Clone() const	{return(new CqShader);}
						/** Determine whether this shader uses the specified system variable.
						 * \param Var ID of the variable from EqEnvVars.
						 */
			TqBool		Uses(EqEnvVars Var) const
										{
											return(USES(m_Uses,Var));
										}
						/** Determine whether this shader uses the specified system variable.
						 * \param Var ID of the variable from EqEnvVars.
						 */
			TqBool		Uses(TqInt Var) const
										{
											assert(Var>=0 && Var<EnvVars_Last);
											return(Uses(static_cast<EqEnvVars>(Var)));
										}
						/** Get a bit vector representing the system variables needed by this shader.
						 */
			TqInt		Uses() const	{
											return(m_Uses);
										}

	protected:
			TqInt		m_Uses;			///< Bit vector representing the system variables used by this shader.
	private:
			CqMatrix	m_matCurrent;	///< Transformation matrix to world coordinates in effect at the time this shader was instantiated.
			CqString	m_strName;		///< The name of this shader.
};


class CqShaderVM;
union UsProgramElement;

/** \struct SqLabel
 * Storege for label during slx file reading.
 */

struct SqLabel
{
	UsProgramElement* m_pAddress;	///< The absolute address of the label.
	TqInt			  m_Offset;		///< The index offset fromt he start of the program.
};


//----------------------------------------------------------------------
/** \union UsProgramElement
 * Union describing the possible types of a single program element.
 */

union UsProgramElement
{
	void(CqShaderVM::*m_Command)();		///< Pointer to a function.
	TqFloat		m_FloatVal;				///< Absolute float value.
	CqString*	m_pString;				///< Absolute string value.
	TqInt		m_iVariable;			///< Shader variable index.
	SqLabel		m_Label;				///< Program label.
};



//----------------------------------------------------------------------
// Macros used in the VM shadeops
//

#define	POPV(A)			CqVMStackEntry& A=Pop(__fVarying)
#define	POP				Pop(__fVarying)
#define	RESULT			CqVMStackEntry& Result=__fVarying?gVaryingResult:gUniformResult;

#define	VARFUNC			TqBool __fVarying=TqTrue;
#define	AUTOFUNC		TqBool __fVarying=TqFalse;

#define	FUNC(Func)		RESULT; \
						Func(Result,this); \
						Push(Result);
#define	FUNC1(Func)		POPV(ValA); \
						RESULT; \
						Func(ValA,Result,this); \
						Push(Result);
#define	FUNC2(Func)		POPV(ValA); \
						POPV(ValB); \
						RESULT; \
						Func(ValA,ValB,Result,this); \
						Push(Result);
#define	FUNC3(Func)		POPV(ValA); \
						POPV(ValB); \
						POPV(ValC); \
						RESULT; \
						Func(ValA,ValB,ValC,Result,this); \
						Push(Result);
#define	FUNC4(Func)		POPV(ValA); \
						POPV(ValB); \
						POPV(ValC); \
						POPV(ValD); \
						RESULT; \
						Func(ValA,ValB,ValC,ValD,Result,this); \
						Push(Result);
#define	FUNC5(Func)		POPV(ValA); \
						POPV(ValB); \
						POPV(ValC); \
						POPV(ValD); \
						POPV(ValE); \
						RESULT; \
						Func(ValA,ValB,ValC,ValD,ValE,Result,this); \
						Push(Result);
#define	FUNC7(Func)		POPV(ValA); \
						POPV(ValB); \
						POPV(ValC); \
						POPV(ValD); \
						POPV(ValE); \
						POPV(ValF); \
						POPV(ValG); \
						RESULT; \
						Func(ValA,ValB,ValC,ValD,ValE,ValF,ValG,Result,this); \
						Push(Result);
#define FUNC1PLUS(Func)	POPV(count);	/* Count of additional values.*/ \
						POPV(a);		/* first value */ \
						/* Read all the additional values. */ \
						static TqInt i=0; \
						TqInt cParams=static_cast<TqInt>(FLOAT(count)); \
						CqVMStackEntry** aParams=new CqVMStackEntry*[cParams]; \
						TqInt iP=0; \
						while(iP!=cParams)	aParams[iP++]=&POP; \
						RESULT; \
						Func(a,Result,this, cParams, aParams); \
						Push(Result);
#define FUNC2PLUS(Func)	POPV(count);	/* Count of additional values.*/ \
						POPV(a);		/* first value */ \
						POPV(b);		/* second value */ \
						/* Read all the additional values. */ \
						static TqInt i=0; \
						TqInt cParams=static_cast<TqInt>(FLOAT(count)); \
						CqVMStackEntry** aParams=new CqVMStackEntry*[cParams]; \
						TqInt iP=0; \
						while(iP!=cParams)	aParams[iP++]=&POP; \
						RESULT; \
						Func(a,b,Result,this, cParams, aParams); \
						Push(Result);
#define FUNC3PLUS(Func)	POPV(count);	/* Count of additional values.*/ \
						POPV(a);		/* first value */ \
						POPV(b);		/* second value */ \
						POPV(c);		/* third value */ \
						/* Read all the additional values. */ \
						static TqInt i=0; \
						TqInt cParams=static_cast<TqInt>(FLOAT(count)); \
						CqVMStackEntry** aParams=new CqVMStackEntry*[cParams]; \
						TqInt iP=0; \
						while(iP!=cParams)	aParams[iP++]=&POP; \
						RESULT; \
						Func(a,b,c,Result,this, cParams, aParams); \
						Push(Result);


#define	VOIDFUNC(Func)	Func(this);
#define	VOIDFUNC1(Func)	POPV(ValA); \
						Func(ValA,this); 
#define	VOIDFUNC2(Func)	POPV(ValA); \
						POPV(ValB); \
						Func(ValA,ValB,this); 
#define	VOIDFUNC3(Func)	POPV(ValA); \
						POPV(ValB); \
						POPV(ValC); \
						Func(ValA,ValB,ValC,this); 
#define	VOIDFUNC4(Func)	POPV(ValA); \
						POPV(ValB); \
						POPV(ValC); \
						POPV(ValD); \
						Func(ValA,ValB,ValC,ValD,this); 
#define	VOIDFUNC5(Func)	POPV(ValA); \
						POPV(ValB); \
						POPV(ValC); \
						POPV(ValD); \
						POPV(ValE); \
						Func(ValA,ValB,ValC,ValD,ValE,this); 
#define	VOIDFUNC7(Func)	POPV(ValA); \
						POPV(ValB); \
						POPV(ValC); \
						POPV(ValD); \
						POPV(ValE); \
						POPV(ValF); \
						POPV(ValG); \
						Func(ValA,ValB,ValC,ValD,ValE,ValF,ValG,this); 
#define VOIDFUNC1PLUS(Func)	POPV(count);	/* Count of additional values.*/ \
						POPV(a);		/* first value */ \
						/* Read all the additional values. */ \
						static TqInt i=0; \
						TqInt cParams=FLOAT(count); \
						CqVMStackEntry** aParams=new CqVMStackEntry*[cParams]; \
						TqInt iP=0; \
						while(iP!=cParams)	aParams[iP++]=&POP; \
						Func(a,this, cParams, aParams); 

#define	SPLINE(func)	POPV(count);	\
						POPV(value); \
						POPV(vala);	\
						POPV(valb);	\
						POPV(valc);	\
						POPV(vald);	\
						TqInt i=0; \
						TqInt cParams=static_cast<TqInt>(FLOAT(count))+4; \
						CqVMStackEntry** apSplinePoints=new CqVMStackEntry*[cParams]; \
						apSplinePoints[0]=&vala; apSplinePoints[1]=&valb; apSplinePoints[2]=&valc; apSplinePoints[3]=&vald; \
						TqInt iSP; \
						for(iSP=4; iSP<cParams; iSP++) \
							apSplinePoints[iSP]=&POP; \
						RESULT; \
						func(value,Result,this,cParams,apSplinePoints); \
						Push(Result);

#define	SSPLINE(func)	POPV(count); \
						POPV(basis); \
						POPV(value); \
						POPV(vala);	\
						POPV(valb);	\
						POPV(valc);	\
						POPV(vald);	\
						TqInt i=0; \
						TqInt cParams=static_cast<TqInt>(FLOAT(count))+4; \
						CqVMStackEntry** apSplinePoints=new CqVMStackEntry*[cParams]; \
						apSplinePoints[0]=&vala; apSplinePoints[1]=&valb; apSplinePoints[2]=&valc; apSplinePoints[3]=&vald; \
						TqInt iSP; \
						for(iSP=4; iSP<cParams; iSP++) \
							apSplinePoints[iSP]=&POP; \
						RESULT; \
						func(basis,value,Result,this,cParams,apSplinePoints); \
						Push(Result);

#define	TEXTURE(func)	POPV(count); /* additional parameter count */\
						POPV(name); /* texture name */\
						POPV(channel); /* channel */\
						TqInt i=0; \
						TqInt cParams=static_cast<TqInt>(FLOAT(count)); \
						CqVMStackEntry** aParams=new CqVMStackEntry*[cParams]; \
						TqInt iP=0; \
						while(iP!=cParams)	aParams[iP++]=&POP; \
						RESULT; \
						func(name,channel,Result,this,cParams,aParams); \
						Push(Result);
#define	TEXTURE1(func)	POPV(count); /* additional parameter count */\
						POPV(name); /* texture name */\
						POPV(channel); /* channel */\
						POPV(R); /* point */\
						TqInt i=0; \
						TqInt cParams=static_cast<TqInt>(FLOAT(count)); \
						CqVMStackEntry** aParams=new CqVMStackEntry*[cParams]; \
						TqInt iP=0; \
						while(iP!=cParams)	aParams[iP++]=&POP; \
						RESULT; \
						func(name,channel,R,Result,this, cParams, aParams); \
						Push(Result);
#define	TEXTURE2(func)	POPV(count); /* additional parameter count */\
						POPV(name); /* texture name */\
						POPV(channel); /* channel */\
						POPV(s); /* s */\
						POPV(t); /* t */\
						TqInt i=0; \
						TqInt cParams=static_cast<TqInt>(FLOAT(count)); \
						CqVMStackEntry** aParams=new CqVMStackEntry*[cParams]; \
						TqInt iP=0; \
						while(iP!=cParams)	aParams[iP++]=&POP; \
						RESULT; \
						func(name,channel,s,t,Result,this, cParams, aParams); \
						Push(Result);
#define	TEXTURE4(func)	POPV(count); /* additional parameter count */\
						POPV(name); /* texture name */\
						POPV(channel); /* channel */\
						POPV(R1); /* R1 */\
						POPV(R2); /* R2 */\
						POPV(R3); /* R3 */\
						POPV(R4); /* R4 */\
						TqInt i=0; \
						TqInt cParams=static_cast<TqInt>(FLOAT(count)); \
						CqVMStackEntry** aParams=new CqVMStackEntry*[cParams]; \
						TqInt iP=0; \
						while(iP!=cParams)	aParams[iP++]=&POP; \
						RESULT; \
						func(name,channel,R1,R2,R3,R4,Result,this,cParams,aParams); \
						Push(Result);
#define	TEXTURE8(func)	POPV(count); /* additional parameter count */\
						POPV(name); /* texture name */\
						POPV(channel); /* channel */\
						POPV(s1); /* s1 */\
						POPV(t1); /* t1 */\
						POPV(s2); /* s2 */\
						POPV(t2); /* t2 */\
						POPV(s3); /* s3 */\
						POPV(t3); /* t3 */\
						POPV(s4); /* s4 */\
						POPV(t4); /* t4 */\
						TqInt i=0; \
						TqInt cParams=static_cast<TqInt>(FLOAT(count)); \
						CqVMStackEntry** aParams=new CqVMStackEntry*[cParams]; \
						TqInt iP=0; \
						while(iP!=cParams)	aParams[iP++]=&POP; \
						RESULT; \
						func(name,channel,s1,t1,s2,t2,s3,t3,s4,t4,Result,this,cParams,aParams); \
						Push(Result);

#define	TRIPLE(T)		POPV(ValA); \
						POPV(ValB); \
						POPV(ValC); \
						RESULT; \
						Result.OpTRIPLE_C(ValA,ValB,ValC); \
						Push(Result);

static TqFloat temp_float;
static CqVector3D temp_point;
static CqColor temp_color;
static CqString temp_string;
static bool temp_bool;
static CqMatrix temp_matrix;

//----------------------------------------------------------------------
/** \class CqShaderVM
 * Main class handling the execution of a program in shader language bytecodes.
 */

class _qShareC CqShaderVM : public CqShaderStack, public CqShader
{
	public:
	_qShareM 	CqShaderVM() : CqShaderStack(), m_PC(0)
								{}
	_qShareM	CqShaderVM(const CqShaderVM& From) : CqShader(), m_PC(0)
								{
									*this=From;
								}
	virtual _qShareM 	~CqShaderVM()	
								{
									// Delete the local variables.
									TqInt i;
									for(i=0; i<m_LocalVars.size(); i++)
									{
										if(m_LocalVars[i]!=NULL)	delete(m_LocalVars[i]);
										m_LocalVars[i]=0;
									}
								}

	_qShareM	void	LoadProgram(std::istream* pFile);
	_qShareM 	void	Execute(CqShaderExecEnv& Env);
	_qShareM	void	ExecuteInit();

	virtual	void		SetValue(const char* name, TqPchar val);
	virtual	TqBool		GetValue(const char* name, CqShaderVariable* res);
	virtual	void		Evaluate(CqShaderExecEnv& Env)
									{Execute(Env);};			
	virtual	_qShareM	void		PrepareDefArgs()						{ExecuteInit();}
	virtual void		Initialise(const TqInt uGridRes, const TqInt vGridRes, CqShaderExecEnv& Env);
	virtual	CqShader*	Clone() const	{
											CqShaderVM* pShader=new CqShaderVM(*this);
											pShader->ExecuteInit();
											return(pShader);
										}

						/** Assignment operator.
						 */
			CqShaderVM&	operator=(const CqShaderVM& From);

	private:
				CqShaderExecEnv*	m_pEnv;							///< Pointer to the current excution environment.
				std::vector<CqShaderVariable*>	m_LocalVars;		///< Array of local variables.
				std::vector<UsProgramElement>	m_ProgramInit;		///< Bytecodes of the intialisation program.
				std::vector<UsProgramElement>	m_Program;			///< Bytecodes of the main program.
				UsProgramElement*	m_PC;							///< Current program pointer.
				TqInt				m_PO;							///< Current program offset.
				TqInt				m_PE;							///< Offset of the end of the program.

									/** Determine whether the program execution has finished.
									 */
				TqBool				fDone()		{return(m_PO>=m_PE);}
									/** Get the next program element from storage.
									 * \return Reference to the next program element.
									 */
				UsProgramElement&	ReadNext()	{m_PO++; return(*m_PC++);}
									/** Get a shader variable by index.
									 * \param Index Integer index, top bit indicates system variable.
									 * \return Pointer to a CqShaderVariable derived class.
									 */
				CqShaderVariable*	GetVar(TqInt Index)
												{
													if(Index&0x8000)	
														return(m_pEnv->pVar(Index&0x7FFF));
													else
														return(m_LocalVars[Index]);
												}
									/** Add a variable to the list of local ones.
									 * \param pVar Pointer to a CqShaderVariable derived class.
									 */
				void				AddLocalVariable(CqShaderVariable* pVar)
												{m_LocalVars.push_back(pVar);}
									/** Find the index of a named shader variable.
									 * \param strName Character pointer to the name.
									 * \return Index of local variable or -1.
									 */
				TqInt				FindLocalVarIndex(const char* strName)
												{
													TqInt i;
													for(i=0; i<m_LocalVars.size(); i++)
														if(m_LocalVars[i]->strName().compare(strName)==0)	return(i);
													return(-1);
												}
				void				GetToken(char* token, TqInt l, std::istream* pFile);

									/** Add a command to the program data area.
									 * \param pCommand Pointer to the opcode function.
									 * \param pProgramArea Pointer to the program area, either init, or main code areas.
									 */
				void				AddCommand(void(CqShaderVM::*pCommand)(), std::vector<UsProgramElement>* pProgramArea)
												{
													UsProgramElement E;
													E.m_Command=pCommand;
													pProgramArea->push_back(E);
												}
									/** Add an absolute float value to the program data area.
									 * \param f Float value to add.
									 * \param pProgramArea Pointer to the program area, either init, or main code areas.
									 */
				void				AddFloat(TqFloat f, std::vector<UsProgramElement>* pProgramArea)
												{
													UsProgramElement E;
													E.m_FloatVal=f;
													pProgramArea->push_back(E);
												}
									/** Add an absolute string value to the program data area.
									 * \param s Character pointer value to add.
									 * \param pProgramArea Pointer to the program area, either init, or main code areas.
									 */
				void				AddString(const char* s, std::vector<UsProgramElement>* pProgramArea)
												{
													CqString* ps=new CqString(s);
													UsProgramElement E;
													E.m_pString=ps;
													pProgramArea->push_back(E);
												}
									/** Add an variable index value to the program area.
									 * \param iVar Integer variable index to add, top bit indicates system variable.
									 * \param pProgramArea Pointer to the program area, either init, or main code areas.
									 */
				void				AddVariable(TqInt iVar, std::vector<UsProgramElement>* pProgramArea)
												{
													UsProgramElement E;
													E.m_iVariable=iVar;
													pProgramArea->push_back(E);
												}
				
				void	SO_nop();
				void	SO_dup();
				void	SO_drop();
				void	SO_debug_break();
				void	SO_pushif();
				void	SO_puship();
				void	SO_pushis();
				void	SO_pushv();
				void	SO_ipushv();
				void	SO_pop();
				void	SO_ipop();
				void	SO_mergef();
				void	SO_merges();
				void	SO_mergep();
				void	SO_mergec();
				void	SO_setfc();
				void	SO_setfp();
				void	SO_setfm();
				void	SO_settc();
				void	SO_settp();
				void	SO_setpc();
				void	SO_setcp();
				void	SO_setwm();
				void	SO_RS_PUSH();
				void	SO_RS_POP();
				void	SO_RS_GET();
				void	SO_RS_INVERSE();
				void	SO_S_CLEAR();
				void	SO_S_GET();
				void	SO_RS_JZ();
				void	SO_RS_JNZ();
				void	SO_S_JZ();
				void	SO_S_JNZ();
				void	SO_jnz();
				void	SO_jz();
				void	SO_jmp();
				void	SO_lsff();
				void	SO_lspp();
				void	SO_lscc();													
				void	SO_gtff();
				void	SO_gtpp();
				void	SO_gtcc();													
				void	SO_geff();
				void	SO_gepp();
				void	SO_gecc();													
				void	SO_leff();
				void	SO_lepp();
				void	SO_lecc();
				void	SO_eqff();
				void	SO_eqpp();
				void	SO_eqcc();
				void	SO_eqss();												
				void	SO_eqmm();												
				void	SO_neff();
				void	SO_nepp();
				void	SO_necc();
				void	SO_ness();													
				void	SO_nemm();													
				void	SO_mulff();
				void	SO_divff();
				void	SO_addff();
				void	SO_subff();
				void	SO_negf();													
				void	SO_mulpp();
				void	SO_divpp();
				void	SO_addpp();
				void	SO_subpp();
				void	SO_crspp();
				void	SO_dotpp();
				void	SO_negp();													
				void	SO_mulcc();
				void	SO_divcc();
				void	SO_addcc();
				void	SO_subcc();
				void	SO_crscc();
				void	SO_dotcc();
				void	SO_negc();													
				void	SO_mulfp();
				void	SO_divfp();
				void	SO_addfp();
				void	SO_subfp();													
				void	SO_mulfc();
				void	SO_divfc();
				void	SO_addfc();
				void	SO_subfc();													
				void	SO_mulmm();
				void	SO_divmm();													
				void	SO_land();
				void	SO_lor();
				void	SO_radians();
				void	SO_degrees();
				void	SO_sin();
				void	SO_asin();
				void	SO_cos();
				void	SO_acos();
				void	SO_tan();
				void	SO_atan();
				void	SO_atan2();
				void	SO_pow();
				void	SO_exp();
				void	SO_sqrt();
				void	SO_log();
				void	SO_log2();
				void	SO_mod();
				void	SO_abs();
				void	SO_sign();
				void	SO_min();
				void	SO_max();
				void	SO_pmin();
				void	SO_pmax();
				void	SO_vmin();
				void	SO_vmax();
				void	SO_nmin();
				void	SO_nmax();
				void	SO_cmin();
				void	SO_cmax();
				void	SO_clamp();
				void	SO_pclamp();
				void	SO_cclamp();
				void	SO_floor();
				void	SO_ceil();
				void	SO_round();
				void	SO_step();
				void	SO_smoothstep();
				void	SO_fspline();
				void	SO_cspline();
				void	SO_pspline();				
				void	SO_sfspline();
				void	SO_scspline();
				void	SO_spspline();				
				void	SO_fDu();
				void	SO_fDv();
				void	SO_fDeriv();
				void	SO_cDu();
				void	SO_cDv();
				void	SO_cDeriv();
				void	SO_pDu();
				void	SO_pDv();
				void	SO_pDeriv();				
				void	SO_frandom();
				void	SO_crandom();
				void	SO_prandom();
				void	SO_noise1();
				void	SO_noise2();
				void	SO_noise3();
				void	SO_noise4();
				void	SO_cnoise1();
				void	SO_cnoise2();
				void	SO_cnoise3();
				void	SO_cnoise4();
				void	SO_pnoise1();
				void	SO_pnoise2();
				void	SO_pnoise3();
				void	SO_pnoise4();
				void	SO_xcomp();
				void	SO_ycomp();
				void	SO_zcomp();
				void	SO_setxcomp();
				void	SO_setycomp();
				void	SO_setzcomp();
				void	SO_length();
				void	SO_distance();
				void	SO_area();
				void	SO_normalize();
				void	SO_faceforward();
				void	SO_reflect();
				void	SO_refract();
				void	SO_fresnel();
				void	SO_fresnel2();
				void	SO_transform2();
				void	SO_transform();
				void	SO_transformm();
				void	SO_vtransform2();
				void	SO_vtransform();
				void	SO_vtransformm();
				void	SO_ntransform2();
				void	SO_ntransform();
				void	SO_ntransformm();
				void	SO_depth();
				void	SO_calculatenormal();
				void	SO_comp();
				void	SO_setcomp();				
				void	SO_cmix();
				void	SO_fmix();
				void	SO_pmix();
				void	SO_vmix();
				void	SO_nmix();
				void	SO_ambient();
				void	SO_diffuse();
				void	SO_specular();
				void	SO_phong();
				void	SO_trace();				
				void	SO_shadow();
				void	SO_shadow1();
				void	SO_ftexture1();
				void	SO_ftexture2();
				void	SO_ftexture3();
				void	SO_ctexture1();
				void	SO_ctexture2();
				void	SO_ctexture3();
				void	SO_fenvironment2();
				void	SO_fenvironment3();
				void	SO_cenvironment2();
				void	SO_cenvironment3();
				void	SO_bump1();
				void	SO_bump2();
				void	SO_bump3();				
				void	SO_illuminate();
				void	SO_illuminate2();
				void	SO_init_illuminance();
				void	SO_advance_illuminance();
				void	SO_illuminance();
				void	SO_illuminance2();
				void	SO_solar();
				void	SO_solar2();
				void	SO_printf();
				void	SO_atmosphere();
				void	SO_displacement();
				void	SO_lightsource();
				void	SO_surface();
				void	SO_attribute();
				void	SO_option();
				void	SO_rendererinfo();
				void	SO_incident();
				void	SO_opposite();
				void	SO_fcellnoise1();
				void	SO_fcellnoise2();
				void	SO_fcellnoise3();
				void	SO_fcellnoise4();
				void	SO_ccellnoise1();
				void	SO_ccellnoise2();
				void	SO_ccellnoise3();
				void	SO_ccellnoise4();
				void	SO_pcellnoise1();
				void	SO_pcellnoise2();
				void	SO_pcellnoise3();
				void	SO_pcellnoise4();
				void	SO_fpnoise1();
				void	SO_fpnoise2();
				void	SO_fpnoise3();
				void	SO_fpnoise4();
				void	SO_cpnoise1();
				void	SO_cpnoise2();
				void	SO_cpnoise3();
				void	SO_cpnoise4();
				void	SO_ppnoise1();
				void	SO_ppnoise2();
				void	SO_ppnoise3();
				void	SO_ppnoise4();													
				void	SO_ctransform2();
				void	SO_ctransform();													
				void	SO_ptlined();
				void	SO_inversesqrt();
				void	SO_concat();
				void	SO_format();
				void	SO_match();
				void	SO_rotate();
				void	SO_filterstep();
				void	SO_filterstep2();
				void	SO_specularbrdf();
				void	SO_mcomp();
				void	SO_setmcomp();
				void	SO_determinant();
				void	SO_mtranslate();
				void	SO_mrotate();
				void	SO_mscale();
				void	SO_fsplinea();
				void	SO_csplinea();
				void	SO_psplinea();				
				void	SO_sfsplinea();
				void	SO_scsplinea();
				void	SO_spsplinea();				
				void	SO_shadername();
				void	SO_shadername2();


		static	SqOpCodeTrans	m_TransTable[];		///< Static opcode translation table.
		static	TqInt			m_cTransSize;		///< Size of translation table.
};


//-----------------------------------------------------------------------

END_NAMESPACE(Aqsis)

#endif	// !SHADERVM_H_INCLUDED
