/* -------------- declaration section -------------- */

%expect 26

%{
#ifdef	WIN32
#include <malloc.h>
#if _MSC_VER
#pragma warning(disable : 4786)
#endif
#include <cstdio>
#include <memory>
namespace std
{ using ::size_t; 
  using ::malloc;
  using ::free;
}
#endif

#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <stack>
#include <cassert>
#include <cstring>

#include	"parsenode.h"
#include	<aqsis/util/logging.h>
#include	<aqsis/util/exception.h>
#include	<aqsis/math/math.h>

# define YYMAXDEPTH 100000
# define YYINITDEPTH  2000

using namespace Aqsis;


namespace Aqsis
{

extern CqString ParseStreamName;
extern std::ostream* ParseErrorStream;
extern TqInt ParseLineNumber;
extern TqInt	iArrayAccess;
CqParseNode*	ParseTreePointer;
std::vector<std::pair<bool,CqString> >	ParseNameSpaceStack;
int scopeID = 0;
std::stack<TqInt> functionReturnCountStack;
std::stack<CqParseNodeDeclaration*> currentFunctionStack;
EqShaderType gShaderType;

bool	FindVariable(const char* name, SqVarRef& ref);
bool	FindFunction(const char* name, std::vector<SqFuncRef>& Ref);
CqString strNameSpace();
void	pushScope(CqString name, bool terminal=false);
CqString	popScope();
void	TypeCheck();
void	Optimise();
void	InitStandardNamespace();
void	ProcessShaderArguments( CqParseNode* pArgs );

void Error(const CqString& message, TqInt lineNumber);

}

%}


%union{
	CqParseNode::Pos m_Pos;
	CqParseNode*	m_pParseNode;
	TqInt			m_VarType;
	EqShaderType	m_ShaderType;
	TqFloat			m_FloatConst;
	CqString*	m_Identifier;
	struct{
		SqVarRef		VarRef;
		SqFuncRef		FuncRef;
		TqInt				eType;
	}				m_pSymbol;
	struct{
		TqInt				Type;
		CqString*	Space;
	}				m_TypeAndSpace;
	EqCommType		m_CommType;
}

%{
extern TqInt yylex();
static void yyerror(const CqString& Message);
%}


%token <m_Identifier>	IDENTIFIER 
%token <m_pSymbol>		SYMBOL ARRAY_SYMBOL

%token <m_tok>	TYPE_FLOAT TYPE_POINT TYPE_STRING TYPE_COLOR TYPE_NORMAL TYPE_VECTOR TYPE_VOID TYPE_MATRIX
%token <m_tok>	TYPE_UNIFORM TYPE_VARYING
%token <m_tok>	SHADER_TYPE_SURFACE SHADER_TYPE_VOLUME SHADER_TYPE_IMAGER SHADER_TYPE_TRANSFORMATION SHADER_TYPE_DISPLACEMENT SHADER_TYPE_LIGHT SHADER_TYPE_ATMOSPHERE 
%token <m_tok>	ATTRIBUTE OPTION RENDERERINFO INCIDENT OPPOSITE LIGHTSOURCE
%token <m_tok>	EXTERN TEXTUREINFO RAYINFO OUTPUT

%token <m_tok>	IF ELSE WHILE FOR CONTINUE BREAK RETURN
%token <m_tok>	ILLUMINATE ILLUMINANCE SOLAR GATHER

%token <m_tok>	TEXTUREMAP ENVIRONMENT BUMP SHADOW OCCLUSION

%token <m_tok>	SETXCOMP SETYCOMP SETZCOMP SETCOMP


/* NOTE: These are priorities in ascending precedence, operators on the same line have the same precedence. */
%right	<m_tok>	'=' ADD_ASSIGN SUB_ASSIGN MUL_ASSIGN DIV_ASSIGN
%right	<m_tok>	'?' ':'
%left	<m_tok> OR_OP
%left	<m_tok>	AND_OP
%left	<m_tok>	'>' GE_OP '<' LE_OP EQ_OP NE_OP
%left	<m_tok>	'+' '-'
%left	<m_tok>	'^'
%left	<m_tok>	'/' '*'
%left	<m_tok>	'.'
%right	<m_tok>	NEG	'!'
%left	<m_tok>	'(' ')'

%type	<m_pParseNode> cast_expr

%token <m_FloatConst>	FLOAT_CONSTANT
%token <m_Identifier>	STRING_LITERAL
%type  <m_pParseNode>	file
%type  <m_pParseNode>	definitions
%type  <m_pParseNode>	shader_definition
%type  <m_pParseNode>	function_definition
%type  <m_pParseNode>	function_declaration
%type  <m_ShaderType>	shader_type
%type  <m_pParseNode>	formals
%type  <m_pParseNode>	variable_definitions
%type  <m_pParseNode>	formal_variable_definitions
%type  <m_TypeAndSpace>	typespec
%type  <m_pParseNode>	def_expressions
%type  <m_pParseNode>	def_expression
%type  <m_pParseNode>	def_init
%type  <m_pParseNode>	array_initialisers
%type  <m_pParseNode>	def_array_initialisers
%type  <m_VarType>		detail
%type  <m_TypeAndSpace>	type
%type  <m_TypeAndSpace>	pspace
%type  <m_TypeAndSpace>	cspace
%type  <m_TypeAndSpace>	vspace
%type  <m_TypeAndSpace>	nspace
%type  <m_TypeAndSpace> mspace
%type  <m_Identifier>	spacetype
%type  <m_pParseNode>	statements
%type  <m_pParseNode>	statement
%type  <m_pParseNode>	loop_control
%type  <m_pParseNode>	loop_modstmt
%type  <m_pParseNode>	loop_mod
%type  <m_pParseNode>	expression
%type  <m_pParseNode>	primary
%type  <m_pParseNode>	relation
%type  <m_pParseNode>	relational_operator
%type  <m_pParseNode>	assignexpression
%type  <m_pParseNode>	procedurecall
%type  <m_pParseNode>	unresolvedcall
%type  <m_pParseNode>	proc_argument
%type  <m_pParseNode>	proc_arguments
%type  <m_pParseNode>	texture_type
%type  <m_pParseNode>	texture_arguments
%type  <m_FloatConst>	number
%type  <m_CommType>		comm_type
%type  <m_pParseNode>	comm_function
%type  <m_Pos>          get_filepos

%start file
%%

file
	: definitions	{ParseTreePointer=$1;}
	| file definitions
					{ParseTreePointer->AddLastChild($2);}
	;

definitions
	:	shader_definition
	|	function_definition
	;

get_filepos
	:
		{
			$$.m_LineNo = ParseLineNumber;
			$$.m_strFileName = ParseStreamName.c_str();
		}
	;

shader_definition
	:	shader_type IDENTIFIER get_filepos '(' formals ')' '{' statements '}'
							{
								// Store a pointer to the actual shader.
								$$=new CqParseNodeShader($2->c_str(),$1);
								$$->SetPos($3);
								$$->AddLastChild($8);

								// Now copy any initialisers from the formals list to their respective
								// local variable definitions.
								CqParseNode* pArgs=$5;
								ProcessShaderArguments( pArgs );
								$$->AddLastChild( $5 );
							}
	|	shader_type SYMBOL get_filepos '(' formals ')' '{' statements '}'
							{
								// Store a pointer to the actual shader.
								CqString strName;
								if($2.eType&1)	strName=CqVarDef::GetVariablePtr($2.VarRef)->strName();
								else			strName=CqFuncDef::GetFunctionPtr($2.FuncRef)->strName();
								$$=new CqParseNodeShader(strName.c_str(),$1);
								$$->SetPos($3);
								$$->AddLastChild($8);

								// Now copy any initialisers from the formals list to their respective
								// local variable definitions.
								CqParseNode* pArgs=$5;
								ProcessShaderArguments( pArgs );
								$$->AddLastChild( $5 );
							}
	|	shader_type IDENTIFIER get_filepos '(' ')' '{' statements '}'
							{
								// Store a pointer to the actual shader.
								$$=new CqParseNodeShader($2->c_str(), $1);
								$$->SetPos($3);
								$$->AddLastChild($7);
							}
	|	shader_type SYMBOL get_filepos '(' ')' '{' statements '}'
							{
								// Store a pointer to the actual shader.
								CqString strName;
								if($2.eType&1)	strName=CqVarDef::GetVariablePtr($2.VarRef)->strName();
								else			strName=CqFuncDef::GetFunctionPtr($2.FuncRef)->strName();
								$$=new CqParseNodeShader(strName.c_str(),$1);
								$$->SetPos($3);
								$$->AddLastChild($7);
							}
	;
 
function_definition
	:	function_declaration formals ')'
							{
								// Push a new level onto the return count stack.
								functionReturnCountStack.push(0);
								currentFunctionStack.push(static_cast<CqParseNodeDeclaration*>($1));
							}
		'{' statements '}'
							{
								CqParseNodeDeclaration* pDecl=static_cast<CqParseNodeDeclaration*>($1);

								// If no return/or multiple returns, then this is not a valid function.
								if(pDecl->Type() != Type_Void &&
								    functionReturnCountStack.top() != 1)
									yyerror("Must have one return in function");
								
								// Make a string of the parameter types.
								CqParseNode* pArgs=$2;
								CqString strArgTypes("");
								if(pArgs)
								{
									// Each child of the variable_definitions is a CqParseNodeVariable
									CqParseNodeVariable* pVar=static_cast<CqParseNodeVariable*>(pArgs->pFirstChild());
									while(pVar)
									{
										// If storage is not specified, a function parameter defaults to varying.
										pVar->SetDefaultStorage(Type_Varying);
										// Get the type from the variable
										strArgTypes+=CqParseNode::TypeIdentifier(pVar->ResType());
										// TODO: Find out if local function arguments can have default values.
										// If so, they can be found as the child of pVar.
										pVar=static_cast<CqParseNodeVariable*>(pVar->pNext());
									}
								}

								// Add the function declaration to the list of local functions.
								CqFuncDef funcdef(pDecl->Type(), pDecl->strName(), pDecl->strName(), strArgTypes.c_str(), $6, pArgs);
								CqFuncDef::AddFunction(funcdef);
								$$=new CqParseNode();
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
								delete(pDecl);
								// Function level namespace is now defunct.
								popScope();
								functionReturnCountStack.pop();
								currentFunctionStack.pop();
							}
	|	function_declaration ')'
							{
								// Push a new level onto the functionReturnCountStack.
								functionReturnCountStack.push(0);
								currentFunctionStack.push(static_cast<CqParseNodeDeclaration*>($1));
							}
		'{' statements '}'
							{
								CqParseNodeDeclaration* pDecl=static_cast<CqParseNodeDeclaration*>($1);

								// If no return/or multiple returns, then this is not a valid function.
								if(pDecl->Type() != Type_Void &&
								    functionReturnCountStack.top() != 1)
									yyerror("Must have one return in function");

								// Add the function declaration to the list of local functions.
								CqFuncDef funcdef(pDecl->Type(), pDecl->strName(), pDecl->strName(), "", $5, 0);
								CqFuncDef::AddFunction(funcdef);
								$$=new CqParseNode();
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
								delete(pDecl);
								// Function level namespace is now defunct.
								popScope();
								functionReturnCountStack.pop();
								currentFunctionStack.pop();
							}
	;

function_declaration
	:	typespec IDENTIFIER '(' {
								$$=new CqParseNodeDeclaration((strNameSpace()+*$2).c_str(),$1.Type);
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
								// Store the name of the function being defined for use in variable namespacing.
								pushScope(*$2,true);
							}
	|	IDENTIFIER '('		{	
								$$=new CqParseNodeDeclaration((strNameSpace()+*$1).c_str(),Type_Void);
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
								// Store the name of the function being defined for use in variable namespacing.
								pushScope(*$1,true);
							}
	|	typespec SYMBOL '(' {
								// TODO: Should warn about duplicate declarations.
								CqString strName(strNameSpace());
								if($2.eType&1)	strName+=CqVarDef::GetVariablePtr($2.VarRef)->strName();
								else			strName+=CqFuncDef::GetFunctionPtr($2.FuncRef)->strName();

								$$=new CqParseNodeDeclaration(strName.c_str(),$1.Type);
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
								// Store the name of the function being defined for use in variable namespacing.
								pushScope(strName,true);
							}
	|	SYMBOL '('			{	
								// TODO: Should warn about duplicate declarations.
								CqString strName(strNameSpace());
								if($1.eType&1)	strName+=CqVarDef::GetVariablePtr($1.VarRef)->strName();
								else			strName+=CqFuncDef::GetFunctionPtr($1.FuncRef)->strName();

								$$=new CqParseNodeDeclaration(strName.c_str(),Type_Void);
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
								// Store the name of the function being defined for use in variable namespacing.
								pushScope(strName,true);
							}
	;

shader_type
	:	SHADER_TYPE_LIGHT	{ 
								$$ = Type_Lightsource;
							}
	|	SHADER_TYPE_SURFACE	{ 
								$$ = Type_Surface;
							}
	|	SHADER_TYPE_VOLUME	{ 
								$$ = Type_Volume;
							}
	|	SHADER_TYPE_DISPLACEMENT	{ 
								$$ = Type_Displacement;
							}
	|	SHADER_TYPE_TRANSFORMATION	{ 
								$$ = Type_Transformation;
							}
	|	SHADER_TYPE_IMAGER	{ 
								$$ = Type_Imager;
							}
	;

formals
	:	formal_variable_definitions
							{
								// Create a list header, and add the first entry to it.
								$$=new CqParseNode();
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
								CqParseNodeVariable* pVarNode=static_cast<CqParseNodeVariable*>($1->pFirstChild());
								while(pVarNode!=0)
								{
									CqParseNodeVariable* pVarNext=static_cast<CqParseNodeVariable*>(pVarNode->pNext());
									$$->AddLastChild(pVarNode);
									pVarNode=pVarNext;
								}
							}
	|	formals ';' formal_variable_definitions
							{
								// Add this one to the list.
								CqParseNodeVariable* pVarNode=static_cast<CqParseNodeVariable*>($3->pFirstChild());
								while(pVarNode!=0)
								{
									CqParseNodeVariable* pVarNext=static_cast<CqParseNodeVariable*>(pVarNode->pNext());
									$$->AddLastChild(pVarNode);
									pVarNode=pVarNext;
								}
							}
	|	formals ';'
	;

formal_variable_definitions
	:	typespec def_expressions
							{
								$$=new CqParseNode();
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
								CqParseNodeDeclaration* pDecl=static_cast<CqParseNodeDeclaration*>($2->pFirstChild());
								while(pDecl)
								{
									TqInt Type=($1.Type);
									TqInt arrayLength = 0;
									
									SqVarRef var;
									bool fv=CqVarDef::FindVariable((strNameSpace()+pDecl->strName()).c_str(), var);
									if(fv)
									{
										CqVarDef* pVar=CqVarDef::GetVariablePtr(var);
										// Check if the declaration marked it as an arry
										if(pVar->Type()&Type_Array)
											Type=(TqInt)(Type|Type_Array);
										arrayLength = pVar->ArrayLength();

										pVar->SetType(Type);
										// Create a variable node, in the case of local variable definition, these nodes will be removed, 
										// and only the intitialisers kept.
										// In the case of function parameters, the variables will be needed for type string construction.
										CqParseNode* pVarNode=new CqParseNodeVariable(var);
										pVarNode->SetPos(ParseLineNumber,ParseStreamName.c_str());
										$$->AddLastChild(pVarNode);

										// Copy any initialisers
										if(pDecl->pFirstChild())
										{
											if(Type&Type_Array)
											{
												CqParseNode* pArrayInit=new CqParseNode();

												CqParseNode* pInitList=pDecl->pFirstChild();
												CqParseNode* pInit=pInitList->pFirstChild();
												TqInt i=0;
												while(pInit!=0)
												{
													CqParseNode* pInit2=pInit->pNext();
													CqParseNodeAssignArray* pInitFunc=new CqParseNodeAssignArray(var);
													pInitFunc->NoDup();
													CqParseNodeFloatConst* pIndex=new CqParseNodeFloatConst(i);

													pInitFunc->AddLastChild(pInit);
													pInitFunc->AddLastChild(pIndex);
													pArrayInit->AddLastChild(pInitFunc);

													i++;
													pInit=pInit2;
												}
												// Check if the right number of initialisers has been specified.
												if(i != arrayLength)
												{
													CqString strError("Error: expected exactly ");
													strError += arrayLength;
													strError += " initialisers, got ";
													strError += i;
													yyerror(strError);
												}
												pVarNode->AddLastChild(pArrayInit);
											}
											else
											{
												// Create an assign operator to initialise the variable.
												CqParseNode* pV=new CqParseNodeAssign(var);
												pV->SetPos(ParseLineNumber,ParseStreamName.c_str());
												pV->NoDup();
												pV->AddLastChild(pDecl->pFirstChild());

												pVarNode->AddLastChild(pV);
												pDecl->ClearChild();
											}
										}
									}

									CqParseNode* pTemp=pDecl;
									pDecl=static_cast<CqParseNodeDeclaration*>(pDecl->pNext());
									delete(pTemp);
								}
							}
	|	OUTPUT typespec def_expressions
							{
								$$=new CqParseNode();
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
								CqParseNodeDeclaration* pDecl=static_cast<CqParseNodeDeclaration*>($3->pFirstChild());
								while(pDecl)
								{
									TqInt Type=($2.Type);
									TqInt arrayLength = 0;
									
									SqVarRef var;
									bool fv=CqVarDef::FindVariable((strNameSpace()+pDecl->strName()).c_str(), var);
									if(fv)
									{
										CqVarDef* pVar=CqVarDef::GetVariablePtr(var);
										// Check if the declaration marked it as an arry
										if(pVar->Type()&Type_Array)
											Type=(TqInt)(Type|Type_Array);
										arrayLength = pVar->ArrayLength();

										pVar->SetType(Type);
										// Create a variable node, in the case of local variable definition, these nodes will be removed, 
										// and only the intitialisers kept.
										// In the case of function parameters, the variables will be needed for type string construction.
										CqParseNodeVariable* pVarNode=new CqParseNodeVariable(var);
										pVarNode->SetOutput();
										pVarNode->SetPos(ParseLineNumber,ParseStreamName.c_str());
										$$->AddLastChild(pVarNode);

										// Copy any initialisers
										if(pDecl->pFirstChild())
										{
											if(Type&Type_Array)
											{
												CqParseNode* pArrayInit=new CqParseNode();

												CqParseNode* pInitList=pDecl->pFirstChild();
												CqParseNode* pInit=pInitList->pFirstChild();
												TqInt i=0;
												while(pInit!=0)
												{
													CqParseNode* pInit2=pInit->pNext();
													CqParseNodeAssignArray* pInitFunc=new CqParseNodeAssignArray(var);
													pInitFunc->NoDup();
													CqParseNodeFloatConst* pIndex=new CqParseNodeFloatConst(i);

													pInitFunc->AddLastChild(pInit);
													pInitFunc->AddLastChild(pIndex);
													pArrayInit->AddLastChild(pInitFunc);

													i++;
													pInit=pInit2;
												}
												// Check if the right number of initialisers has been specified.
												if(i != arrayLength)
												{
													CqString strError("Error: expected exactly ");
													strError += arrayLength;
													strError += " initialisers, got ";
													strError += i;
													yyerror(strError);
												}
												pVarNode->AddLastChild(pArrayInit);
											}
											else
											{
												// Create an assign operator to initialise the variable.
												CqParseNode* pV=new CqParseNodeAssign(var);
												pV->SetPos(ParseLineNumber,ParseStreamName.c_str());
												pV->NoDup();
												pV->AddLastChild(pDecl->pFirstChild());

												pVarNode->AddLastChild(pV);
												pDecl->ClearChild();
											}
										}
									}

									CqParseNode* pTemp=pDecl;
									pDecl=static_cast<CqParseNodeDeclaration*>(pDecl->pNext());
									delete(pTemp);
								}
							}
	;


variable_definitions
	:	typespec def_expressions
							{
								$$=new CqParseNode();
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
								CqParseNodeDeclaration* pDecl=static_cast<CqParseNodeDeclaration*>($2->pFirstChild());
								while(pDecl)
								{
									TqInt Type=($1.Type);
									TqInt arrayLength = 0;
									
									SqVarRef var;
									bool fv=CqVarDef::FindVariable((strNameSpace()+pDecl->strName()).c_str(), var);
									if(fv)
									{
										CqVarDef* pVar=CqVarDef::GetVariablePtr(var);
										// Check if the declaration marked it as an arry
										if(pVar->Type()&Type_Array)
										{
											Type=(TqInt)(Type|Type_Array);
											if( pVar->ArrayLength() <= 0 )
												yyerror("Array length must be specified.");
										}
										arrayLength = pVar->ArrayLength();

										pVar->SetType(Type);
										// Create a variable node, in the case of local variable definition, these nodes will be removed, 
										// and only the intitialisers kept.
										// In the case of function parameters, the variables will be needed for type string construction.
										CqParseNode* pVarNode=new CqParseNodeVariable(var);
										pVarNode->SetPos(ParseLineNumber,ParseStreamName.c_str());
										$$->AddLastChild(pVarNode);

										// Copy any initialisers
										if(pDecl->pFirstChild())
										{
											if(Type&Type_Array)
											{
												CqParseNode* pArrayInit=new CqParseNode();

												CqParseNode* pInitList=pDecl->pFirstChild();
												CqParseNode* pInit=pInitList->pFirstChild();
												TqInt i=0;
												while(pInit!=0)
												{
													CqParseNode* pInit2=pInit->pNext();
													CqParseNodeAssignArray* pInitFunc=new CqParseNodeAssignArray(var);
													pInitFunc->NoDup();
													CqParseNodeFloatConst* pIndex=new CqParseNodeFloatConst(i);

													pInitFunc->AddLastChild(pInit);
													pInitFunc->AddLastChild(pIndex);
													pArrayInit->AddLastChild(pInitFunc);

													i++;
													pInit=pInit2;
												}
												// Check if the right number of initialisers has been specified.
												if(i != arrayLength)
												{
													CqString strError("Error: expected exactly ");
													strError += arrayLength;
													strError += " initialisers, got ";
													strError += i;
													yyerror(strError);
												}
												pVarNode->AddLastChild(pArrayInit);
											}
											else
											{
												// Create an assign operator to initialise the variable.
												CqParseNode* pV=new CqParseNodeAssign(var);
												pV->SetPos(ParseLineNumber,ParseStreamName.c_str());
												pV->NoDup();
												pV->AddLastChild(pDecl->pFirstChild());

												// Make sure that any change in the storage type is communicated to the initialiser
												pV->UpdateStorageStatus();

												pVarNode->AddLastChild(pV);
												pDecl->ClearChild();
											}
										}
									}

									CqParseNode* pTemp=pDecl;
									pDecl=static_cast<CqParseNodeDeclaration*>(pDecl->pNext());
									delete(pTemp);
								}
							}
	|	EXTERN typespec def_expressions
							{
								$$=new CqParseNode();
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
								CqParseNodeDeclaration* pDecl=static_cast<CqParseNodeDeclaration*>($3->pFirstChild());
								while(pDecl)
								{
									SqVarRef varLocal, varExtern;
									bool fvl=CqVarDef::FindVariable((strNameSpace()+pDecl->strName()).c_str(), varLocal);

									// As this is an extern, we need to repeatedly check in the previous namespaces
									// until we find the variable they are referring to.
									bool fve=false;
									if(!ParseNameSpaceStack.empty())
									{
										std::vector<std::pair<bool,CqString> >::reverse_iterator i=ParseNameSpaceStack.rbegin()+1;
										while(!fve && i!=ParseNameSpaceStack.rend())
										{
											CqString strNS=i->second;
											fve=CqVarDef::FindVariable((strNS+pDecl->strName()).c_str(), varExtern);
											i++;
										}
									}
									if(!fve)
									{
										// If not found in the namespaces defined in the code, check the global namespace.
										fve=CqVarDef::FindStandardVariable(pDecl->strName(), varExtern);
									}

									// If we found a candidate...
									if(fve && fvl)
									{
										CqVarDef* pvarLocal=CqVarDef::GetVariablePtr(varLocal);
										CqVarDef* pvarExtern=CqVarDef::GetVariablePtr(varExtern);
										CqParseNode* pVarNode=new CqParseNodeVariable(varLocal);
										pvarLocal->SetExtern(true, varExtern);
										pvarLocal->SetType(pvarExtern->Type());
										pVarNode->SetPos(ParseLineNumber,ParseStreamName.c_str());
										$$->AddLastChild(pVarNode);
									}
									else
										yyerror("extern not found");

									CqParseNode* pTemp=pDecl;
									pDecl=static_cast<CqParseNodeDeclaration*>(pDecl->pNext());
									delete(pTemp);
								}
							}
	;

typespec
	:	detail type			{
								$$.Type=(TqInt)($2.Type|$1);
								$$.Space=$2.Space;
							}
	|	type
	;

def_expressions
	:	def_expression		{
								// Create a list header and add the first element.
								$$=new CqParseNode();
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
								$$->AddLastChild($1);
							}
	|	def_expressions ',' def_expression
							{
								// This one to the list.
								$$->AddLastChild($3);
							}
	;

def_expression
	:	IDENTIFIER def_init	{
								// Create a new variable declaration based on the specified name.
								$$=new CqParseNodeDeclaration($1->c_str());
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
								CqVarDef vardef(Type_Void, (strNameSpace()+*$1).c_str());
								CqVarDef::AddVariable(vardef);
								// Add the initialiser as the first child of the declaration.
								$$->AddLastChild($2);
							}
	|	IDENTIFIER			{
								// Create a new variable declaration based on the specified name.
								$$=new CqParseNodeDeclaration($1->c_str());
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
								CqVarDef vardef(Type_Void, (strNameSpace()+*$1).c_str());
								CqVarDef::AddVariable(vardef);
							}
	|	IDENTIFIER '[' number ']' {
								// Create a new variable declaration based on the specified name.
								$$=new CqParseNodeDeclaration($1->c_str());
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
								CqVarDef vardef(Type_Array, (strNameSpace()+*$1).c_str(), static_cast<TqInt>($3));
								CqVarDef::AddVariable(vardef);
							}
	|	IDENTIFIER '[' number ']' def_array_initialisers
							{
								// Create a new variable declaration based on the specified name.
								$$=new CqParseNodeDeclaration($1->c_str());
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
								CqVarDef vardef(Type_Array, (strNameSpace()+*$1).c_str(), static_cast<TqInt>($3));
								CqVarDef::AddVariable(vardef);
								$$->AddLastChild($5);
							}
	|	IDENTIFIER '[' ']' {
								// Create a new variable declaration based on the specified name.
								$$=new CqParseNodeDeclaration($1->c_str());
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
								CqVarDef vardef(Type_Array, (strNameSpace()+*$1).c_str(), 0);
								CqVarDef::AddVariable(vardef);
							}
	|	SYMBOL def_init		{
								// Create a new declaration based on the name.
								// TODO: Should warn about duplicate declarations.
								CqString strName("");
								if($1.eType&1)	strName=CqVarDef::GetVariablePtr($1.VarRef)->strName();
								else			strName=CqFuncDef::GetFunctionPtr($1.FuncRef)->strName();

								$$=new CqParseNodeDeclaration(strName.c_str());
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
								CqVarDef vardef(Type_Void, (strNameSpace()+strName).c_str());
								CqVarDef::AddVariable(vardef);
								// Add the initialiser as the first child of the declaration.
								$$->AddLastChild($2);
							}
	|	SYMBOL				{
								CqString strName("");
								if($1.eType&1)	strName=CqVarDef::GetVariablePtr($1.VarRef)->strName();
								else			strName=CqFuncDef::GetFunctionPtr($1.FuncRef)->strName();

								$$=new CqParseNodeDeclaration(strName.c_str());
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
								CqVarDef vardef(Type_Void, (strNameSpace()+strName).c_str());
								CqVarDef::AddVariable(vardef);
							}
	|	SYMBOL '[' number ']' {
								CqString strName("");
								if($1.eType&1)	strName=CqVarDef::GetVariablePtr($1.VarRef)->strName();
								else			strName=CqFuncDef::GetFunctionPtr($1.FuncRef)->strName();

								$$=new CqParseNodeDeclaration(strName.c_str());
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
								CqVarDef vardef(Type_Array, (strNameSpace()+strName).c_str(), static_cast<TqInt>($3));
								CqVarDef::AddVariable(vardef);
							}
	|	SYMBOL '[' number ']' def_array_initialisers
							{
								CqString strName("");
								if($1.eType&1)	strName=CqVarDef::GetVariablePtr($1.VarRef)->strName();
								else			strName=CqFuncDef::GetFunctionPtr($1.FuncRef)->strName();

								$$=new CqParseNodeDeclaration(strName.c_str());
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
								CqVarDef vardef(Type_Array, (strNameSpace()+strName).c_str(), static_cast<TqInt>($3));
								CqVarDef::AddVariable(vardef);
								$$->AddLastChild($5);
							}
	|	SYMBOL '[' ']' {
								CqString strName("");
								if($1.eType&1)	strName=CqVarDef::GetVariablePtr($1.VarRef)->strName();
								else			strName=CqFuncDef::GetFunctionPtr($1.FuncRef)->strName();

								$$=new CqParseNodeDeclaration(strName.c_str());
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
								CqVarDef vardef(Type_Array, (strNameSpace()+strName).c_str(), 0);
								CqVarDef::AddVariable(vardef);
							}
	;

def_init
	:	'=' expression		{
								$$=$2;
							}
	;

def_array_initialisers
	:	'=' '{' array_initialisers '}'
							{
								CqParseNode* pArrayInit=new CqParseNode();
								CqParseNode* pInit=$3->pFirstChild();
								while(pInit)
								{
									CqParseNode* pInit2=pInit->pNext();
									pArrayInit->AddLastChild(pInit);
									pInit=pInit2;
								}
								$$=pArrayInit;
							}
	;

array_initialisers
	:	expression			{
								// Create a list header and add the first element.
								$$=new CqParseNode();
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
								$$->AddLastChild($1);
							}
	|	array_initialisers ',' expression
							{
								// This one to the list.
								$$->AddLastChild($3);
							}
	;

detail
	:	TYPE_VARYING		{	$$=Type_Varying;	}
	|	TYPE_UNIFORM		{	$$=Type_Uniform;	}
	;

type
	:	TYPE_FLOAT				{	
								$$.Type=Type_Float;		
								$$.Space=0;
							}
	|	TYPE_STRING				{
								$$.Type=Type_String;	
								$$.Space=0;
							}
	|	TYPE_VOID			{
								$$.Type=Type_Void;
								$$.Space=0;
							}
	|	pspace
	|	cspace
	|	vspace
	|	nspace
	|	mspace
	;

pspace
	:	TYPE_POINT spacetype {
								$$.Type=Type_Point;		
								$$.Space=$2;
							}
	|	TYPE_POINT			{
								$$.Type=Type_Point;		
								$$.Space=0;
							}
	;

cspace
	:	TYPE_COLOR spacetype {
								$$.Type=Type_Color;		
								$$.Space=$2;
							}
	|	TYPE_COLOR			{
								$$.Type=Type_Color;		
								$$.Space=0;
							}
	;

vspace
	:	TYPE_VECTOR spacetype	{
								$$.Type=Type_Vector;		
								$$.Space=$2;
							}
	|	TYPE_VECTOR			{
								$$.Type=Type_Vector;		
								$$.Space=0;
							}
	;

nspace
	:	TYPE_NORMAL spacetype	{
								$$.Type=Type_Normal;		
								$$.Space=$2;
							}
	|	TYPE_NORMAL			{
								$$.Type=Type_Normal;		
								$$.Space=0;
							}
	;

mspace
	:	TYPE_MATRIX spacetype	{
								$$.Type=Type_Matrix;		
								$$.Space=$2;
							}
	|	TYPE_MATRIX			{
								$$.Type=Type_Matrix;		
								$$.Space=0;
							}
	;

spacetype
	:	STRING_LITERAL
	;

statements
	:	statement			{
								// Create a list header and add the first element.
								$$=new CqParseNode();
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
								$$->AddFirstChild($1);
							}
	|	statements statement
							{
								// Add this one to the list.
								$$->AddLastChild($2);
							}
	;

statement
	:	assignexpression ';'{$$->NoDup();}
	|	procedurecall ';'	{
								if($$->ResType()!=Type_Void)
								{
									$$=new CqParseNodeDrop();
									$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
									$$->AddLastChild($1);
								}
							}
	|	unresolvedcall ';'	{
								if($$->ResType()!=Type_Void)
								{
									$$=new CqParseNodeDrop();
									$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
									$$->AddLastChild($1);
								}
							}
	|	comm_function ';'	{
								$$=new CqParseNodeDrop();
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
								$$->AddLastChild($1);
							}
	|	RETURN expression ';'
							{
								// Check that the return is valid
								if(functionReturnCountStack.empty())
									yyerror("return statement outside function scope");
								else
								{
									functionReturnCountStack.top()++;
									if(currentFunctionStack.top()->Type() != Type_Void)
									{
										if(functionReturnCountStack.top() > 1)
											yyerror("more than one return in function");
									}
									else if(functionReturnCountStack.top() > 0)
										yyerror("void functions cannot have a return");
								}
								// Create a cast node casting to the function return type.
								$$=new CqParseNodeCast(currentFunctionStack.top()->Type());
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
								$$->AddLastChild($2);
							}
	|	loop_modstmt ';'
	|	loop_control		
	|	variable_definitions ';'
							{
								// Remove the variable nodes from these leaving just the 
								// intialisation code.
								// Each child of the variable_definitions is a CqParseNodeVariable
								CqParseNodeVariable* pVar=static_cast<CqParseNodeVariable*>($1->pFirstChild());
								while(pVar)
								{
									CqParseNodeVariable* pTemp=static_cast<CqParseNodeVariable*>(pVar->pNext());

									// If storage is not specified, local variable defaults to varying.
									pVar->SetDefaultStorage(Type_Varying);
									
									// Relink the initialisers, if any, after the variable.
									if(pVar->pFirstChild())
									{
										// Make sure that any change in the storage type is communicated to the initialiser
										pVar->pFirstChild()->UpdateStorageStatus();
										CqParseNode* pVarying=pVar->pFirstChild();
										pVarying->LinkAfter(pVar);
									}
									pVar->UnLink();
									delete(pVar);
									pVar=pTemp;
								}
							}
	|	function_definition
	|	'{' 
							{
								// Introduce a new scope for the block
								pushScope("b");
							}
		statements 
							{
								popScope();
							}
		'}'				{	
								$$=$3;	
							}
	|	'{' '}'		{	$$=new CqParseNode(); }
	|	';'				{	$$=new CqParseNode(); }
	|	IF relation statement
							{
								CqParseNode* pNew=new CqParseNodeConditional();
								pNew->SetPos(ParseLineNumber,ParseStreamName.c_str());
								pNew->AddLastChild($2);
								pNew->AddLastChild($3);
								$$=pNew;
							}
	|	IF expression statement			
							{
								CqParseNode* relation=new CqParseNodeRelOp(Op_NE);
								relation->SetPos(ParseLineNumber,ParseStreamName.c_str());
								relation->AddFirstChild($2);
								CqParseNode* pcomp=new CqParseNodeFloatConst(0);
								relation->AddLastChild(pcomp);

								CqParseNode* pNew=new CqParseNodeConditional();
								pNew->SetPos(ParseLineNumber,ParseStreamName.c_str());
								pNew->AddLastChild(relation);
								pNew->AddLastChild($3);
								$$=pNew;

							}
	|	IF relation statement ELSE statement
							{
								CqParseNode* pNew=new CqParseNodeConditional();
								pNew->SetPos(ParseLineNumber,ParseStreamName.c_str());
								pNew->AddLastChild($2);
								pNew->AddLastChild($3);
								pNew->AddLastChild($5);
								$$=pNew;
							}
	|	IF expression statement ELSE statement	{
								CqParseNode* relation=new CqParseNodeRelOp(Op_NE);
								relation->SetPos(ParseLineNumber,ParseStreamName.c_str());
								relation->AddFirstChild($2);
								CqParseNode* pcomp=new CqParseNodeFloatConst(0);
								relation->AddLastChild(pcomp);
								CqParseNode* pNew=new CqParseNodeConditional();
								
								pNew->SetPos(ParseLineNumber,ParseStreamName.c_str());
								pNew->AddLastChild(relation);
								pNew->AddLastChild($3);
								pNew->AddLastChild($5);
								$$=pNew;
							}
	;



loop_control
	:	WHILE relation statement		 {
								$$=new CqParseNodeWhileConstruct();
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
								$$->AddLastChild($2);
								$$->AddLastChild($3);
							}
	|	WHILE expression statement		 {
								CqParseNode* relation=new CqParseNodeRelOp(Op_NE);
								relation->SetPos(ParseLineNumber,ParseStreamName.c_str());
								relation->AddFirstChild($2);
								CqParseNode* pcomp=new CqParseNodeFloatConst(0);
								relation->AddLastChild(pcomp);

								$$=new CqParseNodeWhileConstruct();
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
								$$->AddLastChild(relation);
								$$->AddLastChild($3);
							}
	|	FOR '(' expression ';' relation ';' expression ')' statement
							{
								$$=new CqParseNode();
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
								$$->AddLastChild($3);
								CqParseNode* pW=new CqParseNodeWhileConstruct();
								pW->SetPos(ParseLineNumber,ParseStreamName.c_str());
								$$->AddLastChild(pW);
								pW->AddLastChild($5);
								pW->AddLastChild($9);
								pW->AddLastChild($7);
								// Make sure that any assigns in the two expressions don't dup
								$3->NoDup();
								$7->NoDup();
							}
	|	FOR '(' expression ';' expression ';' expression ')' statement
							{
								CqParseNode* relation=new CqParseNodeRelOp(Op_NE);
								relation->SetPos(ParseLineNumber,ParseStreamName.c_str());
								relation->AddFirstChild($5);
								CqParseNode* pcomp=new CqParseNodeFloatConst(0);
								relation->AddLastChild(pcomp);

								$$=new CqParseNode();
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
								$$->AddLastChild($3);
								CqParseNode* pW=new CqParseNodeWhileConstruct();
								pW->SetPos(ParseLineNumber,ParseStreamName.c_str());
								$$->AddLastChild(pW);
								pW->AddLastChild(relation);
								pW->AddLastChild($9);
								pW->AddLastChild($7);
								// Make sure that any assigns in the two expressions don't dup
								$3->NoDup();
								$7->NoDup();
							}
	|	SOLAR '(' ')' statement
							{
								$$=new CqParseNodeSolarConstruct();
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
								$$->AddLastChild($4);
							}
	|	SOLAR '(' expression ',' expression ')' statement
							{
								$$=new CqParseNodeSolarConstruct(true);
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
								CqParseNode* pArg=new CqParseNode();
								pArg->SetPos(ParseLineNumber,ParseStreamName.c_str());
								pArg->AddFirstChild($3);
								pArg->AddFirstChild($5);
								$$->AddLastChild(pArg);
								$$->AddLastChild($7);
								// Make sure that any assigns in the two expressions don't dup
								$3->NoDup();
								$5->NoDup();
							}
	|	ILLUMINATE '(' expression ')' statement
							{
								$$=new CqParseNodeIlluminateConstruct();
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
								$$->AddLastChild($3);
								$$->AddLastChild($5);
								// Make sure that any assigns in the two expressions don't dup
								$3->NoDup();
							}
	|	ILLUMINATE '(' expression ',' expression ',' expression ')' statement
							{
								$$=new CqParseNodeIlluminateConstruct(true);
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
								CqParseNode* pArg=new CqParseNode();
								pArg->SetPos(ParseLineNumber,ParseStreamName.c_str());
								pArg->AddFirstChild($3);
								pArg->AddFirstChild($5);
								pArg->AddFirstChild($7);
								$$->AddLastChild(pArg);
								$$->AddLastChild($9);
								// Make sure that any assigns in the two expressions don't dup
								$3->NoDup();
								$5->NoDup();
								$7->NoDup();
							}
	|	ILLUMINANCE '(' expression ')' statement
							{
								$$=new CqParseNodeIlluminanceConstruct();
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
								CqParseNode* pArg=new CqParseNode();
								pArg->SetPos(ParseLineNumber,ParseStreamName.c_str());
								pArg->AddFirstChild(new CqParseNodeStringConst(""));	// [category]
								pArg->AddFirstChild($3);
								$$->AddLastChild(pArg);
								$$->AddLastChild($5);
								// Make sure that any assigns in the two expressions don't dup
								$3->NoDup();
							}
	|	ILLUMINANCE '(' expression ',' expression ')' statement
							{
								$$=new CqParseNodeIlluminanceConstruct();
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
								CqParseNode* pArg=new CqParseNode();
								pArg->SetPos(ParseLineNumber,ParseStreamName.c_str());
								pArg->AddFirstChild($3);
								pArg->AddFirstChild($5);
								$$->AddLastChild(pArg);
								$$->AddLastChild($7);
								// Make sure that any assigns in the two expressions don't dup
								$3->NoDup();
								$5->NoDup();
							}
	|	ILLUMINANCE '(' expression ',' expression ',' expression ')' statement
							{
								$$=new CqParseNodeIlluminanceConstruct(true);
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
								CqParseNode* pArg=new CqParseNode();
								pArg->SetPos(ParseLineNumber,ParseStreamName.c_str());
								pArg->AddFirstChild(new CqParseNodeStringConst(""));	// [category]
								pArg->AddFirstChild($3);
								pArg->AddFirstChild($5);
								pArg->AddFirstChild($7);
								$$->AddLastChild(pArg);
								$$->AddLastChild($9);
								// Make sure that any assigns in the two expressions don't dup
								$3->NoDup();
								$5->NoDup();
								$7->NoDup();
							}
	| ILLUMINANCE '(' expression ',' expression ',' expression ',' expression ')' statement
							{
								$$=new CqParseNodeIlluminanceConstruct(true);
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
								CqParseNode* pArg=new CqParseNode();
								pArg->SetPos(ParseLineNumber,ParseStreamName.c_str());
								pArg->AddFirstChild($3);
								pArg->AddFirstChild($5);
								pArg->AddFirstChild($7);
								pArg->AddFirstChild($9);
								$$->AddLastChild(pArg);
								$$->AddLastChild($11);
								// Make sure that any assigns in the two expressions don't dup
								$3->NoDup();
								$5->NoDup();
								$7->NoDup();
								$9->NoDup();
							}
	| GATHER '(' expression ',' expression ',' expression ',' expression ',' expression ')' statement
							{
								$$=new CqParseNodeGatherConstruct(true);
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
								CqParseNode* pArg=new CqParseNode();
								pArg->SetPos(ParseLineNumber,ParseStreamName.c_str());
								pArg->AddLastChild($3);
								pArg->AddLastChild($5);
								pArg->AddLastChild($7);
								pArg->AddLastChild($9);
								pArg->AddLastChild($11);
								$$->AddLastChild(pArg);
								$$->AddLastChild($13);
								// Make sure that any assigns in the two expressions don't dup
								$3->NoDup();
								$5->NoDup();
								$7->NoDup();
								$9->NoDup();
								$11->NoDup();
							}
	| GATHER '(' expression ',' expression ',' expression ',' expression ',' expression ')' statement ELSE statement
							{
								$$=new CqParseNodeGatherConstruct(true);
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
								CqParseNode* pArg=new CqParseNode();
								pArg->SetPos(ParseLineNumber,ParseStreamName.c_str());
								pArg->AddLastChild($3);
								pArg->AddLastChild($5);
								pArg->AddLastChild($7);
								pArg->AddLastChild($9);
								pArg->AddLastChild($11);
								$$->AddLastChild(pArg);
								$$->AddLastChild($13);
								$$->AddLastChild($15);
								// Make sure that any assigns in the two expressions don't dup
								$3->NoDup();
								$5->NoDup();
								$7->NoDup();
								$9->NoDup();
								$11->NoDup();
							}
	| GATHER '(' expression ',' expression ',' expression ',' expression ',' expression texture_arguments ')' statement
							{
								$$=new CqParseNodeGatherConstruct(true);
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
								CqParseNode* pArg=new CqParseNode();
								pArg->SetPos(ParseLineNumber,ParseStreamName.c_str());
								pArg->AddLastChild($3);
								pArg->AddLastChild($5);
								pArg->AddLastChild($7);
								pArg->AddLastChild($9);
								pArg->AddLastChild($11);
								// Add all extra arguments as further arguments to the function.
								CqParseNode* pParam=$12->pFirstChild();
								while(pParam!=0)
								{
									CqParseNode* pTemp=pParam->pNext();
									pArg->AddLastChild(pParam);
									pParam=pTemp;
								}
								$$->AddLastChild(pArg);
								$$->AddLastChild($14);
								// Make sure that any assigns in the two expressions don't dup
								$3->NoDup();
								$5->NoDup();
								$7->NoDup();
								$9->NoDup();
								$11->NoDup();
							}
	| GATHER '(' expression ',' expression ',' expression ',' expression ',' expression texture_arguments ')' statement ELSE statement
							{
								$$=new CqParseNodeGatherConstruct(true);
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
								CqParseNode* pArg=new CqParseNode();
								pArg->SetPos(ParseLineNumber,ParseStreamName.c_str());
								pArg->AddLastChild($3);
								pArg->AddLastChild($5);
								pArg->AddLastChild($7);
								pArg->AddLastChild($9);
								pArg->AddLastChild($11);
								// Add all extra arguments as further arguments to the function.
								CqParseNode* pParam=$12->pFirstChild();
								while(pParam!=0)
								{
									CqParseNode* pTemp=pParam->pNext();
									pArg->AddLastChild(pParam);
									pParam=pTemp;
								}
								$$->AddLastChild(pArg);
								$$->AddLastChild($14);
								$$->AddLastChild($16);
								// Make sure that any assigns in the two expressions don't dup
								$3->NoDup();
								$5->NoDup();
								$7->NoDup();
								$9->NoDup();
								$11->NoDup();
							}
	;

loop_modstmt
	:	loop_mod number		{
								$$=$1;
								// \todo: "number" should really be an integer
								// - it would better to encode this into the
								// grammer.
								TqFloat breakDepth = $2;
								TqInt breakDepthRounded = lround(breakDepth);
								if(breakDepthRounded <= 0)
									yyerror("Invalid break or continue - depth argument must be greater than 0");
								else if(breakDepthRounded != breakDepth)
									yyerror("Invalid break or continue - depth argument must be an integer");
								CqParseNode* pArg = new CqParseNodeFloatConst($2);
								pArg->SetPos(ParseLineNumber,ParseStreamName.c_str());
								$$->AddLastChild(pArg);
							}
	|	loop_mod
	;

loop_mod
	:	BREAK				{
								$$=new CqParseNodeLoopMod(LoopMod_Break);
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
							}
	|	CONTINUE			{
								$$=new CqParseNodeLoopMod(LoopMod_Continue);
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
							}

	;

expression
	:	primary
	|	expression '.' expression
							{
								std::vector<SqFuncRef> func;
								CqFuncDef::FindFunction("operator.", func);
								CqParseNodeFunctionCall* pFunc=new CqParseNodeFunctionCall(func);
								pFunc->SetPos(ParseLineNumber,ParseStreamName.c_str());
								pFunc->AddLastChild($1);
								pFunc->AddLastChild($3);

								$$=pFunc;
							}
	|	expression '/' expression
							{
								std::vector<SqFuncRef> func;
								CqFuncDef::FindFunction("operator/", func);
								CqParseNodeFunctionCall* pFunc=new CqParseNodeFunctionCall(func);
								pFunc->SetPos(ParseLineNumber,ParseStreamName.c_str());
								pFunc->AddLastChild($1);
								pFunc->AddLastChild($3);

								$$=pFunc;
							}
	|	expression '*' expression
							{
								std::vector<SqFuncRef> func;
								CqFuncDef::FindFunction("operator*", func);
								CqParseNodeFunctionCall* pFunc=new CqParseNodeFunctionCall(func);
								pFunc->SetPos(ParseLineNumber,ParseStreamName.c_str());
								pFunc->AddLastChild($1);
								pFunc->AddLastChild($3);

								$$=pFunc;
							}
	|	expression '^' expression
							{
								std::vector<SqFuncRef> func;
								CqFuncDef::FindFunction("operator^", func);
								CqParseNodeFunctionCall* pFunc=new CqParseNodeFunctionCall(func);
								pFunc->SetPos(ParseLineNumber,ParseStreamName.c_str());
								pFunc->AddLastChild($1);
								pFunc->AddLastChild($3);

								$$=pFunc;
							}
	|	expression '+' expression
							{
								std::vector<SqFuncRef> func;
								CqFuncDef::FindFunction("operator+", func);
								CqParseNodeFunctionCall* pFunc=new CqParseNodeFunctionCall(func);
								pFunc->SetPos(ParseLineNumber,ParseStreamName.c_str());
								pFunc->AddLastChild($1);
								pFunc->AddLastChild($3);

								$$=pFunc;
							}
	|	expression '-' expression
							{
								std::vector<SqFuncRef> func;
								CqFuncDef::FindFunction("operator-", func);
								CqParseNodeFunctionCall* pFunc=new CqParseNodeFunctionCall(func);
								pFunc->SetPos(ParseLineNumber,ParseStreamName.c_str());
								pFunc->AddLastChild($1);
								pFunc->AddLastChild($3);

								$$=pFunc;
							}
	|	'-' expression	%prec NEG
							{
								std::vector<SqFuncRef> func;
								CqFuncDef::FindFunction("operatorneg", func);
								CqParseNodeFunctionCall* pFunc=new CqParseNodeFunctionCall(func);
								pFunc->SetPos(ParseLineNumber,ParseStreamName.c_str());
								pFunc->AddFirstChild($2);

								$$=pFunc;
							}
	|	cast_expr
	|	relation '?' expression ':' expression
							{
								CqParseNode* pNew=new CqParseNodeQCond();
								pNew->SetPos(ParseLineNumber,ParseStreamName.c_str());
								pNew->AddLastChild($1);
								pNew->AddLastChild($3);
								pNew->AddLastChild($5);
								$$=pNew;
							}
	;


cast_expr
	:	type expression	 %prec '('	{
								$$=new CqParseNode();
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
								CqParseNode* pCast=new CqParseNodeCast($1.Type);
								pCast->SetPos(ParseLineNumber,ParseStreamName.c_str());
								pCast->AddFirstChild($2);
								
								TqInt Type=(TqInt)($1.Type&Type_Mask);
								CqString* pSpace=$1.Space;
								// Check if the type has a valid space associated
								if(pSpace!=0 && pSpace->compare("")!=0 && 
								  ((Type==Type_Point) ||
								   (Type==Type_Normal) ||
								   (Type==Type_Vector) ||
								   (Type==Type_Matrix) ||
								   (Type==Type_Color) ))
								{
									// Create a transform function.
									std::vector<SqFuncRef> funcTrans;
									CqString strTrans("transform");;
									if(Type==Type_Normal)	strTrans="ntransform";
									else if(Type==Type_Vector)	strTrans="vtransform";
									else if(Type==Type_Matrix)	strTrans="mtransform";
									else if(Type==Type_Color)	strTrans="ctransform";
									if(FindFunction(strTrans.c_str(), funcTrans))
									{
										$$=new CqParseNodeFunctionCall(funcTrans);
										$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
									
										// And create a holder for the arguments.
										CqParseNode* pFromSpace=new CqParseNodeStringConst((*$1.Space).c_str());
										CqParseNode* pToSpace;
										if(Type != Type_Color)	pToSpace = new CqParseNodeStringConst("current");
										else					pToSpace = new CqParseNodeStringConst("rgb");
										$$->AddLastChild(pFromSpace);
										$$->AddLastChild(pToSpace);
										$$->AddLastChild(pCast);
									}
								}
								else
									$$->AddFirstChild(pCast);
							}

	;

primary
	:	number				{	
								$$=new CqParseNodeFloatConst($1);
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
							}
	|	SYMBOL				{
								$$=new CqParseNodeVariable($1.VarRef);	
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
							}
	|	ARRAY_SYMBOL		{
								$$=new CqParseNodeVariable($1.VarRef);	
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
							}
	|	ARRAY_SYMBOL '[' expression ']' {
								$$=new CqParseNodeVariableArray($1.VarRef);
								$$->AddLastChild($3);
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
							}
	|	STRING_LITERAL		{
								$$=new CqParseNodeStringConst($1->c_str());
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
							}
	|	procedurecall		
	|	unresolvedcall		
	|	comm_function
	|	assignexpression
	|	'(' expression ')'	{	
								$$=$2;
							}
	|	'(' expression ',' expression ',' expression ')'
							{
								$$=new CqParseNodeTriple();
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
								$$->AddLastChild($2);
								$$->AddLastChild($4);
								$$->AddLastChild($6);
							}
	|	'(' expression ',' expression ',' expression ',' expression ',' expression ',' expression ',' expression ',' expression ',' expression ',' expression ',' expression ',' expression ',' expression ',' expression ',' expression ',' expression ')'
							{
								$$=new CqParseNodeHexTuple();
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
								$$->AddLastChild($2);
								$$->AddLastChild($4);
								$$->AddLastChild($6);
								$$->AddLastChild($8);
								$$->AddLastChild($10);
								$$->AddLastChild($12);
								$$->AddLastChild($14);
								$$->AddLastChild($16);
								$$->AddLastChild($18);
								$$->AddLastChild($20);
								$$->AddLastChild($22);
								$$->AddLastChild($24);
								$$->AddLastChild($26);
								$$->AddLastChild($28);
								$$->AddLastChild($30);
								$$->AddLastChild($32);
							}
	;

relational_operator
	:	'>'					{	$$=new CqParseNodeRelOp(Op_G);	}
	|	GE_OP				{	$$=new CqParseNodeRelOp(Op_GE);	}
	|	'<'					{	$$=new CqParseNodeRelOp(Op_L);	}
	|	LE_OP				{	$$=new CqParseNodeRelOp(Op_LE);	}
	|	EQ_OP				{	$$=new CqParseNodeRelOp(Op_EQ);	}
	|	NE_OP				{	$$=new CqParseNodeRelOp(Op_NE);	}
	;

relation
	:	'(' relation ')'	{
								$$=$2;
							}
	|	expression relational_operator expression
							{
								$$=$2;
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
								$$->AddFirstChild($1);
								$$->AddFirstChild($3);
							}
	|	relation AND_OP relation
							{
								$$=new CqParseNodeLogicalOp(Op_LogAnd);
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
								$$->AddFirstChild($1);
								$$->AddLastChild($3);
							}
	|	relation OR_OP relation
							{
								$$=new CqParseNodeLogicalOp(Op_LogOr);
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
								$$->AddFirstChild($1);
								$$->AddLastChild($3);
							}
	|	'!' relation		{
								$$=new CqParseNodeUnaryOp(Op_LogicalNot);
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
								$$->AddLastChild($2);
							}
	;

assignexpression
	:	SYMBOL '=' expression
							{
								$$=new CqParseNodeAssign($1.VarRef);
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
								$$->AddLastChild($3);
							}
	|	SYMBOL ADD_ASSIGN expression
							{
								$$=new CqParseNodeAssign($1.VarRef);
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
								
								std::vector<SqFuncRef> func;
								CqFuncDef::FindFunction("operator+", func);
								CqParseNodeFunctionCall* pFunc=new CqParseNodeFunctionCall(func);
								pFunc->SetPos(ParseLineNumber,ParseStreamName.c_str());
								pFunc->AddLastChild(new CqParseNodeVariable($1.VarRef));
								pFunc->AddLastChild($3);

								$$->AddLastChild(pFunc);
							}
	|	SYMBOL SUB_ASSIGN expression
							{
								$$=new CqParseNodeAssign($1.VarRef);
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
								
								std::vector<SqFuncRef> func;
								CqFuncDef::FindFunction("operator-", func);
								CqParseNodeFunctionCall* pFunc=new CqParseNodeFunctionCall(func);
								pFunc->SetPos(ParseLineNumber,ParseStreamName.c_str());
								pFunc->AddLastChild(new CqParseNodeVariable($1.VarRef));
								pFunc->AddLastChild($3);

								$$->AddLastChild(pFunc);
							}
	|	SYMBOL MUL_ASSIGN expression
							{
								$$=new CqParseNodeAssign($1.VarRef);
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
								
								std::vector<SqFuncRef> func;
								CqFuncDef::FindFunction("operator*", func);
								CqParseNodeFunctionCall* pFunc=new CqParseNodeFunctionCall(func);
								pFunc->SetPos(ParseLineNumber,ParseStreamName.c_str());
								pFunc->AddLastChild(new CqParseNodeVariable($1.VarRef));
								pFunc->AddLastChild($3);

								$$->AddLastChild(pFunc);
							}
	|	SYMBOL DIV_ASSIGN expression
							{
								$$=new CqParseNodeAssign($1.VarRef);
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
								
								std::vector<SqFuncRef> func;
								CqFuncDef::FindFunction("operator/", func);
								CqParseNodeFunctionCall* pFunc=new CqParseNodeFunctionCall(func);
								pFunc->SetPos(ParseLineNumber,ParseStreamName.c_str());
								pFunc->AddLastChild(new CqParseNodeVariable($1.VarRef));
								pFunc->AddLastChild($3);

								$$->AddLastChild(pFunc);
							}
	|	ARRAY_SYMBOL '[' expression ']' '=' expression
							{
								$$=new CqParseNodeAssignArray($1.VarRef);
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
								$$->AddLastChild($6);
								$$->AddLastChild($3);
							}
	|	ARRAY_SYMBOL  '[' expression ']' ADD_ASSIGN expression
							{
								$$=new CqParseNodeAssignArray($1.VarRef);
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
								
								std::vector<SqFuncRef> func;
								CqFuncDef::FindFunction("operator+", func);
								CqParseNodeFunctionCall* pFunc=new CqParseNodeFunctionCall(func);
								pFunc->SetPos(ParseLineNumber,ParseStreamName.c_str());
								CqParseNodeVariableArray* pVar=new CqParseNodeVariableArray($1.VarRef);
								pVar->AddLastChild($3->Clone());
								pFunc->AddLastChild(pVar);

								pFunc->AddLastChild($6);

								$$->AddLastChild(pFunc);
								$$->AddLastChild($3);
							}
	|	ARRAY_SYMBOL  '[' expression ']' SUB_ASSIGN expression
							{
								$$=new CqParseNodeAssignArray($1.VarRef);
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
								
								std::vector<SqFuncRef> func;
								CqFuncDef::FindFunction("operator-", func);
								CqParseNodeFunctionCall* pFunc=new CqParseNodeFunctionCall(func);
								pFunc->SetPos(ParseLineNumber,ParseStreamName.c_str());
								CqParseNodeVariableArray* pVar=new CqParseNodeVariableArray($1.VarRef);
								pVar->AddLastChild($3->Clone());
								pFunc->AddLastChild(pVar);

								pFunc->AddLastChild($6);

								$$->AddLastChild(pFunc);
								$$->AddLastChild($3);
							}
	|	ARRAY_SYMBOL  '[' expression ']' MUL_ASSIGN expression
							{
								$$=new CqParseNodeAssignArray($1.VarRef);
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
								
								std::vector<SqFuncRef> func;
								CqFuncDef::FindFunction("operator*", func);
								CqParseNodeFunctionCall* pFunc=new CqParseNodeFunctionCall(func);
								pFunc->SetPos(ParseLineNumber,ParseStreamName.c_str());
								CqParseNodeVariableArray* pVar=new CqParseNodeVariableArray($1.VarRef);
								pVar->AddLastChild($3->Clone());
								pFunc->AddLastChild(pVar);

								pFunc->AddLastChild($6);

								$$->AddLastChild(pFunc);
								$$->AddLastChild($3);
							}
	|	ARRAY_SYMBOL  '[' expression ']' DIV_ASSIGN expression
							{
								$$=new CqParseNodeAssignArray($1.VarRef);
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
								
								std::vector<SqFuncRef> func;
								CqFuncDef::FindFunction("operator/", func);
								CqParseNodeFunctionCall* pFunc=new CqParseNodeFunctionCall(func);
								pFunc->SetPos(ParseLineNumber,ParseStreamName.c_str());
								CqParseNodeVariableArray* pVar=new CqParseNodeVariableArray($1.VarRef);
								pVar->AddLastChild($3->Clone());
								pFunc->AddLastChild(pVar);

								pFunc->AddLastChild($6);

								$$->AddLastChild(pFunc);
								$$->AddLastChild($3);
							}
	;

procedurecall
	:	SYMBOL '(' proc_arguments ')'
							{
								std::vector<SqFuncRef> func;
								CqFuncDef::FindFunction(CqFuncDef::GetFunctionPtr($1.FuncRef)->strName(), func);
								CqParseNodeFunctionCall* pFunc=new CqParseNodeFunctionCall(func);
								pFunc->SetPos(ParseLineNumber,ParseStreamName.c_str());
								while($3->pFirstChild()!=0)	pFunc->AddLastChild($3->pFirstChild());

								$$=pFunc;
							}
	|	SYMBOL '(' ')'		{
								std::vector<SqFuncRef> func;
								CqFuncDef::FindFunction(CqFuncDef::GetFunctionPtr($1.FuncRef)->strName(), func);
								CqParseNodeFunctionCall* pFunc=new CqParseNodeFunctionCall(func);
								pFunc->SetPos(ParseLineNumber,ParseStreamName.c_str());

								$$=pFunc;
							}
	|	texture_type '(' proc_arguments ')'
							{
								$$ = $1;
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
								CqParseNode* pArg = $3->pFirstChild();
								CqParseNode* pNameWithChannel = 0;

								while(pArg!=0)
								{
									if(pArg->NodeType() == ParseNode_TextureNameWithChannel)
									{
										pNameWithChannel = pArg;
										$$->AddLastChild(pArg->pFirstChild());
										pArg = static_cast<CqParseNode*>(pArg->pNextSibling());
									}
									else
									{
										CqParseNode *pNextArg = static_cast<CqParseNode*>(pArg->pNextSibling());
										$$->AddLastChild(pArg);
										pArg = pNextArg;
									}
								}
								if(NULL != pNameWithChannel)
								{
									CqParseNodeStringConst *cname = new CqParseNodeStringConst("channel");
									$$->AddLastChild(cname);
									$$->AddLastChild(pNameWithChannel->pFirstChild());
								}
							}
	;


unresolvedcall
	:	IDENTIFIER '(' proc_arguments ')'
							{
								Aqsis::log() << warning << "Unresolved function " << $1->c_str() << " will be treated as a DSO at runtime" << std::endl;
								CqParseNode* pArgs=$3;
								CqString strArgTypes("");
								if(pArgs)
								{
									CqParseNode* pArg=static_cast<CqParseNode*>(pArgs->pFirstChild());
									while(pArg)
									{
										strArgTypes+=CqParseNode::TypeIdentifier(pArg->ResType());
										pArg=static_cast<CqParseNode*>(pArg->pNext());
									}
								};

								CqFuncDef func_spec(Type_Nil,$1->c_str(),"unresolved",strArgTypes.c_str(), (CqParseNode*)NULL, pArgs);
								CqParseNodeUnresolvedCall* pUFunc=new CqParseNodeUnresolvedCall(func_spec);
								pUFunc->SetPos(ParseLineNumber,ParseStreamName.c_str());
								while($3->pFirstChild()!=0)	pUFunc->AddLastChild($3->pFirstChild());

								$$=pUFunc;
							}
	|	IDENTIFIER '(' ')'
							{
								// Need to emit a warning.
								// This should in theory be the eaiest case to handle
								// since there are no arguments that might need casting
								// later.
								Aqsis::log() << warning << "Unresolved function " << $1->c_str() << " will be treated as a DSO at runtime" << std::endl;
								CqFuncDef func_spec(Type_Nil, $1->c_str(), "unresolved","");
								CqParseNodeUnresolvedCall* pUFunc=new CqParseNodeUnresolvedCall(func_spec);
								pUFunc->SetPos(ParseLineNumber,ParseStreamName.c_str());

								$$=pUFunc;
							}
	;

proc_argument
	:	expression		
	|	expression '[' expression ']'
							{
								CqParseNodeTextureNameWithChannel *pNameChannel = new CqParseNodeTextureNameWithChannel();
								pNameChannel->AddFirstChild($1);
								pNameChannel->AddLastChild($3);
								$$ = pNameChannel;
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
							}
	;
		
proc_arguments
	:	proc_argument		{
								// Create a list header, and add the first entry.
								$$=new CqParseNode();
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
								$$->AddLastChild($1);
							}
	|	proc_arguments ',' proc_argument
							{
								// Add this entry to the list.
								$$->AddLastChild($3);
							}
	;

texture_type
	:	TEXTUREMAP			{
								std::vector<SqFuncRef> func;
								CqFuncDef::FindFunction("texture", func);
								$$=new CqParseNodeFunctionCall(func);
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
							}
	|	ENVIRONMENT			{
								std::vector<SqFuncRef> func;
								CqFuncDef::FindFunction("environment", func);
								$$=new CqParseNodeFunctionCall(func);
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
							}
	|	BUMP				{
								std::vector<SqFuncRef> func;
								CqFuncDef::FindFunction("bump", func);
								$$=new CqParseNodeFunctionCall(func);
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
							}
	|	SHADOW				{
								std::vector<SqFuncRef> func;
								CqFuncDef::FindFunction("shadow", func);
								$$=new CqParseNodeFunctionCall(func);
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
							}
	|	OCCLUSION			{
								std::vector<SqFuncRef> func;
								CqFuncDef::FindFunction("occlusion", func);
								$$=new CqParseNodeFunctionCall(func);
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
							}
	;

texture_arguments
	:	',' expression		{
								// Create a list header and add the first entry.
								$$=new CqParseNode();
								$$->SetPos(ParseLineNumber,ParseStreamName.c_str());
								$$->AddFirstChild($2);
							}
	|	texture_arguments ',' expression
							{
								// Add this entry to the list.
								$$->AddLastChild($3);
							}
		
	;

number
	:	FLOAT_CONSTANT
	;


comm_type
	:	SHADER_TYPE_ATMOSPHERE		{$$=CommTypeAtmosphere;}
	|	SHADER_TYPE_DISPLACEMENT	{$$=CommTypeDisplacement;}
	|	LIGHTSOURCE		{$$=CommTypeLightsource;}
	|	SHADER_TYPE_SURFACE			{$$=CommTypeSurface;}
	|	ATTRIBUTE		{$$=CommTypeAttribute;}
	|	OPTION			{$$=CommTypeOption;}
	|	RENDERERINFO	{$$=CommTypeRendererInfo;}
	|	INCIDENT		{$$=CommTypeIncident;}
	|	OPPOSITE		{$$=CommTypeOpposite;}
    |	TEXTUREINFO	    {$$=CommTypeTextureInfo;}
    |	RAYINFO	    {$$=CommTypeRayInfo;}

	;

comm_function
	:	comm_type '(' expression ',' SYMBOL ')'
							{
								CqVarDef* pVD=0;
								bool fError=false;
								
								if(($3->ResType()&Type_Mask)!=Type_String)	fError=true;

								// Get the variable, error if not a variable.
								if($5.eType&1)	pVD=CqVarDef::GetVariablePtr($5.VarRef);
								
								if(pVD!=0 && !fError)
								{
									$$=new CqParseNodeCommFunction($1, $5.VarRef);
									$$->AddLastChild($3);
								}
								else
								{
									yyerror("invalid variable reference");
									$$=new CqParseNode();
								}
							}
    | comm_type '(' expression ',' expression ',' SYMBOL ')'

                            {
								CqVarDef* pVD=0;
								bool fError=false;
								
                                if(($3->ResType()&Type_Mask)!=Type_String)	fError=true;

							    if(($5->ResType()&Type_Mask)!=Type_String)	fError=true;

								// Get the variable, error if not a variable.
								if($7.eType&1)	pVD=CqVarDef::GetVariablePtr($7.VarRef);
								
								if(pVD!=0 && !fError)
								{
									$$=new CqParseNodeCommFunction($1, $7.VarRef);
								    $$->AddLastChild($5);
								    $$->AddLastChild($3);
								}
								else
								{
									yyerror("invalid variable reference");
									$$=new CqParseNode();
								}
							}
	|	comm_type '(' expression ',' ARRAY_SYMBOL ')'
							{
								CqVarDef* pVD=0;
								bool fError=false;
								
								if(($3->ResType()&Type_Mask)!=Type_String)	fError=true;

								// Get the variable, error if not a variable.
								if($5.eType&1)	pVD=CqVarDef::GetVariablePtr($5.VarRef);
								
								if(pVD!=0 && !fError)
								{
									$$=new CqParseNodeCommFunction($1, $5.VarRef);
									$$->AddLastChild($3);
								}
								else
								{
									yyerror("invalid variable reference");
									$$=new CqParseNode();
								}
							}
    | comm_type '(' expression ',' expression ',' ARRAY_SYMBOL ')'

                            {
								CqVarDef* pVD=0;
								bool fError=false;
								
                                if(($3->ResType()&Type_Mask)!=Type_String)	fError=true;

							    if(($5->ResType()&Type_Mask)!=Type_String)	fError=true;

								// Get the variable, error if not a variable.
								if($7.eType&1)	pVD=CqVarDef::GetVariablePtr($7.VarRef);
								
								if(pVD!=0 && !fError)
								{
									$$=new CqParseNodeCommFunction($1, $7.VarRef);
								    $$->AddLastChild($5);
								    $$->AddLastChild($3);
								}
								else
								{
									yyerror("invalid variable reference");
									$$=new CqParseNode();
								}
							}
	;


%%

namespace Aqsis
{

bool FindVariable(const char* name, SqVarRef& Ref)
{
	// First search in the current namespace, then in the global namespace.
	CqString strLocalVar(strNameSpace()+name);
	
	if(CqVarDef::FindVariable(strLocalVar.c_str(), Ref))	return(true);
	else	return(CqVarDef::FindVariable(name, Ref));

	
}


bool FindFunction(const char* name, std::vector<SqFuncRef>& Ref)
{
	// Search in the namespaces from local to global in order.
	CqString strNS(strNameSpace());
	
	do
	{
		CqString strLocalFunc(strNS+name);
		if(CqFuncDef::FindFunction(strLocalFunc.c_str(), Ref))
			return(true);

		// Extract the next namespace up.
		if( ( strNS.size() > 2 ) && ( strNS.substr( strNS.size()-2 ) == "::" ) )
		{
			strNS = strNS.substr( 0, strNS.size()-2 );
			strNS = strNS.substr(0, strNS.rfind("::")+strlen("::"));
		}

	}while( strNS.find_last_of("::") != std::string::npos );
	
	return(CqFuncDef::FindFunction(name, Ref));
}


void TypeCheck()
{
	// Typecheck any declared variables.
	TqUint i;
	for(i=0; i<gLocalVars.size(); i++)
	{
		bool needsCast = false;
		if(gLocalVars[i].pDefValue()!=0)
			gLocalVars[i].pDefValue()->TypeCheck(CqParseNode::pAllTypes(), Type_Last-1, needsCast, false);
	}

	// Typecheck any local functions.
	for(i=0; i<gLocalFuncs.size(); i++)
	{
		if(gLocalFuncs[i].pDef()!=0)
		{
			bool needsCast = false;
//			TqInt RetType=gLocalFuncs[i].Type();
			gLocalFuncs[i].pDefNode()->TypeCheck(CqParseNode::pAllTypes(), Type_Last-1, needsCast, false);
		}
	}

	bool needsCast = false;
	if(ParseTreePointer)
		ParseTreePointer->TypeCheck(CqParseNode::pAllTypes(), Type_Last-1, needsCast, false);
}


void Optimise()
{
	// Optimise any local functions.
	TqUint i;
	for(i=0; i<gLocalFuncs.size(); i++)
	{
		if(gLocalFuncs[i].pDef()!=0)
			gLocalFuncs[i].pDefNode()->Optimise();
	}

	if(ParseTreePointer)
		ParseTreePointer->Optimise();
}


CqString strNameSpace()	
{
	CqString strRes("");

	if(!ParseNameSpaceStack.empty())
		strRes=ParseNameSpaceStack.back().second;

	return(strRes);
}

void pushScope(CqString name, bool terminal)
{
	std::pair<bool,CqString> n;
	n.first = terminal;
	n.second = strNameSpace() + name + CqString(scopeID++) + "::";
	ParseNameSpaceStack.push_back(n);
}

CqString popScope()
{
	CqString old = ParseNameSpaceStack.back().second;
	ParseNameSpaceStack.erase(ParseNameSpaceStack.end()-1);
	return old;
}

void InitStandardNamespace()
{
	std::pair<bool,CqString> n;
	n.first = false;
	n.second = "";
	ParseNameSpaceStack.push_back(n);
}

void ProcessShaderArguments( CqParseNode* pArgs )
{
	if(pArgs)
	{
		// Each child of the variable_definitions is a CqParseNodeVariable
		CqParseNodeVariable* pVar=static_cast<CqParseNodeVariable*>(pArgs->pFirstChild());

		while(pVar)
		{
			// If storage is not specified, a shader parameter defaults to uniform.
			pVar->SetDefaultStorage(Type_Uniform);
			// Force the variable to be a parameter.
			pVar->SetParam();

			// Check if a default value has been specified.
			CqParseNode* pDefValue=pVar->pFirstChild();
			if(pDefValue!=0)
			{
				// Get a pointer to the local variable.
				CqVarDef* pVarDef=CqVarDef::GetVariablePtr(pVar->VarRef());
				if(pVarDef!=0)
				{
					CqParseNode Node;
					pDefValue->UnLink();
					CqParseNodeCast* pCast=new CqParseNodeCast(pVarDef->Type());
					Node.AddLastChild(pCast);
					pCast->AddLastChild(pDefValue);
					Node.Optimise();
					pVarDef->SetpDefValue(Node.pFirstChild());
					pVar->AddFirstChild( Node.pFirstChild() );
				}
			}
			else
			{
				Error(CqString("missing default value for shader instance variable \"")
						+ pVar->strName() + "\"",  pVar->LineNo());
			}
			pVar=static_cast<CqParseNodeVariable*>(pVar->pNext());
		}
	}
}

void Error(const CqString& message, TqInt lineNumber)
{
	AQSIS_THROW_XQERROR(XqParseError, EqE_Syntax,
		ParseStreamName.c_str() << " : " << lineNumber << " : " << message.c_str());
}

} // End Namespace

static void yyerror(const CqString& message)
{
	Error(message, ParseLineNumber);
}
