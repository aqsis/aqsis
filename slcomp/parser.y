/* -------------- declaration section -------------- */

%name SLParser

%expect 14
%define LSP_NEEDED
%define ERROR_BODY =0
%define	ERROR_VERBOSE =1
%define LEX_BODY =0
%define	MEMBERS public: \
						CqParseNode* pParseTree()	{return(m_ParseTree);} \
						TqBool	 FindVariable(const char* name, SqVarRef& ref); \
						TqBool	 FindFunction(const char* name, std::vector<SqFuncRef>& Ref); \
						void		 Output(std::ostream& Out); \
						void		 TypeCheck(); \
						void		 Optimise(); \
						CqString	 strNameSpace(); \
						CqFuncDef&	 Shader()		{return(m_Shader);} \
						virtual	const char* FileName() const=0; \
						virtual	void SetFileName(const char* strName)=0; \
						virtual void SetLineNo(TqInt i)=0; \
						virtual TqInt LineNo() const=0; \
						void		 InitStandardNamespace(); \
				protected: \
						CqFuncDef	m_Shader; \
						CqParseNode* m_ParseTree; \
						std::vector<CqString>	m_stkstrNameSpace;
%define CONSTRUCTOR_INIT : m_ParseTree(0)
%define CONSTRUCTOR_CODE { \
						m_stkstrNameSpace.clear(); \
						InitStandardNamespace(); \
					}
%define DEBUG 1

%header{
using namespace Aqsis;
%}

%union{
	CqParseNode*	m_pParseNode;
	EqVariableType	m_VarType;
	EqShaderType	m_ShaderType;
	TqFloat			m_FloatConst;
	CqString*	m_Identifier;
	struct{
		SqVarRef		VarRef;
		SqFuncRef		FuncRef;
		int				eType;
	}				m_pSymbol;
	struct{
		EqVariableType	Type;
		CqString*	Space;
	}				m_TypeAndSpace;
	EqCommType		m_CommType;
}

%token <m_Identifier>	IDENTIFIER 
%token <m_pSymbol>		SYMBOL

%token <m_tok>	FLOAT POINT STRING COLOR NORMAL VECTOR TYPEVOID MATRIX
%token <m_tok>	UNIFORM VARYING
%token <m_tok>	SURFACE VOLUME IMAGER TRANSFORMATION DISPLACEMENT LIGHT LIGHTSOURCE ATMOSPHERE ATTRIBUTE OPTION RENDERERINFO INCIDENT OPPOSITE
%token <m_tok>	EXTERN

%token <m_tok>	IF ELSE WHILE FOR CONTINUE BREAK RETURN
%token <m_tok>	ILLUMINATE ILLUMINANCE SOLAR

%token <m_tok>	TEXTURE ENVIRONMENT BUMP SHADOW

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
%type  <m_pParseNode>	proc_arguments
%type  <m_pParseNode>	texture
%type  <m_pParseNode>	texture_type
%type  <m_pParseNode>	texture_filename
%type  <m_pParseNode>	channel
%type  <m_pParseNode>	texture_arguments
%type  <m_FloatConst>	number
%type  <m_CommType>		comm_type
%type  <m_pParseNode>	comm_function

%start file
%%

file
	: definitions	{m_ParseTree=$1;}
	| file definitions
					{m_ParseTree->AddLastChild($2);}
	;

definitions
	:	shader_definition
	|	function_definition
	;

shader_definition
	:	shader_type IDENTIFIER '(' formals ')' '{' statements '}'
							{
								// There should only be one of these, so store the shader type.
								m_Shader.SetSType($1);

								// Store a pointer to the actual shader.
								m_Shader=CqFuncDef(Type_Nil, $2->c_str(), $2->c_str(), "", 0, TqFalse, $1);
								$$=new CqParseNode();
								$$->SetPos(LineNo(),FileName());
								$$->AddLastChild($7);

								// Now copy any initialisers from the formals list to their respective
								// local variable definitions.
								CqParseNode* pArgs=$4;
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
												pDefValue->UnLink();
												pVarDef->SetpDefValue(pDefValue);
											}
										}
										pVar=static_cast<CqParseNodeVariable*>(pVar->pNext());
									}
								}

							}
	|	shader_type IDENTIFIER '(' ')' '{' statements '}'
							{
								// There should only be one of these, so store the shader type.
								m_Shader.SetSType($1);

								// Store a pointer to the actual shader.
								m_Shader=CqFuncDef(Type_Nil, $2->c_str(), $2->c_str(), "", 0, TqFalse, $1);
								$$=new CqParseNode();
								$$->SetPos(LineNo(),FileName());
								$$->AddLastChild($6);
							}
	;
 
function_definition
	:	function_declaration formals ')' '{' statements '}'
							{
								CqParseNodeDeclaration* pDecl=static_cast<CqParseNodeDeclaration*>($1);
								
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
								CqFuncDef::AddFunction(CqFuncDef(pDecl->Type(), pDecl->strName(), pDecl->strName(), strArgTypes.c_str(), $5, pArgs));
								$$=new CqParseNode();
								$$->SetPos(LineNo(),FileName());
								delete(pDecl);
								// Function level namespace is now defunct.
								m_stkstrNameSpace.erase(m_stkstrNameSpace.end()-1);
							}
	|	function_declaration ')' '{' statements '}'
							{
								CqParseNodeDeclaration* pDecl=static_cast<CqParseNodeDeclaration*>($1);
								
								// Add the function declaration to the list of local functions.
								CqFuncDef::AddFunction(CqFuncDef(pDecl->Type(), pDecl->strName(), pDecl->strName(), "", $4, 0));
								$$=new CqParseNode();
								$$->SetPos(LineNo(),FileName());
								delete(pDecl);
								// Function level namespace is now defunct.
								m_stkstrNameSpace.erase(m_stkstrNameSpace.end()-1);
							}
	;

function_declaration
	:	typespec IDENTIFIER '(' {
								$$=new CqParseNodeDeclaration((strNameSpace()+*$2).c_str(),$1.Type);
								$$->SetPos(LineNo(),FileName());
								// Store the name of the function being defined for use in variable namespacing.
								m_stkstrNameSpace.push_back(strNameSpace()+*$2+"::");
							}
	|	IDENTIFIER '('		{	
								$$=new CqParseNodeDeclaration((strNameSpace()+*$1).c_str(),Type_Void);
								$$->SetPos(LineNo(),FileName());
								// Store the name of the function being defined for use in variable namespacing.
								m_stkstrNameSpace.push_back(strNameSpace()+*$1+"::");
							}
	|	typespec SYMBOL '(' {
								// TODO: Should warn about duplicate declarations.
								CqString strName(strNameSpace());
								if($2.eType&1)	strName+=CqVarDef::GetVariablePtr($2.VarRef)->strName();
								else			strName+=CqFuncDef::GetFunctionPtr($2.FuncRef)->strName();

								$$=new CqParseNodeDeclaration(strName.c_str(),$1.Type);
								$$->SetPos(LineNo(),FileName());
								// Store the name of the function being defined for use in variable namespacing.
								m_stkstrNameSpace.push_back(strName+"::");
							}
	|	SYMBOL '('			{	
								// TODO: Should warn about duplicate declarations.
								CqString strName(strNameSpace());
								if($1.eType&1)	strName+=CqVarDef::GetVariablePtr($1.VarRef)->strName();
								else			strName+=CqFuncDef::GetFunctionPtr($1.FuncRef)->strName();

								$$=new CqParseNodeDeclaration(strName.c_str(),Type_Void);
								$$->SetPos(LineNo(),FileName());
								// Store the name of the function being defined for use in variable namespacing.
								m_stkstrNameSpace.push_back(strName+"::");
							}
	;

shader_type
	:	LIGHT				{ $$=Type_Lightsource;}
	|	SURFACE				{ $$=Type_Surface;}
	|	VOLUME				{ $$=Type_Volume;}
	|	DISPLACEMENT		{ $$=Type_Displacement;}
	|	TRANSFORMATION		{ $$=Type_Transformation;}
	|	IMAGER				{ $$=Type_Imager;}
	;

formals
	:	variable_definitions
							{
								// Create a list header, and add the first entry to it.
								$$=new CqParseNode();
								$$->SetPos(LineNo(),FileName());
								CqParseNodeVariable* pVarNode=static_cast<CqParseNodeVariable*>($1->pFirstChild());
								while(pVarNode!=0)
								{
									CqParseNodeVariable* pVarNext=static_cast<CqParseNodeVariable*>(pVarNode->pNext());
									$$->AddLastChild(pVarNode);
									pVarNode=pVarNext;
								}
							}
	|	formals ';' variable_definitions
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

variable_definitions
	:	typespec def_expressions
							{
								$$=new CqParseNode();
								$$->SetPos(LineNo(),FileName());
								CqParseNodeDeclaration* pDecl=static_cast<CqParseNodeDeclaration*>($2->pFirstChild());
								while(pDecl)
								{
									EqVariableType Type=($1.Type);
									
									SqVarRef var;
									TqBool fv=CqVarDef::FindVariable((strNameSpace()+pDecl->strName()).c_str(), var);
									if(fv)
									{
										CqVarDef* pVar=CqVarDef::GetVariablePtr(var);
										// Check if the declaration marked it as an arry
										if(pVar->Type()&Type_Array)
											Type=(EqVariableType)(Type|Type_Array);

										pVar->SetType(Type);
										// Create a variable node, in the case of local variable definition, these nodes will be removed, 
										// and only the intitialisers kept.
										// In the case of function parameters, the variables will be needed for type string construction.
										CqParseNode* pVarNode=new CqParseNodeVariable(var);
										pVarNode->SetPos(LineNo(),FileName());
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
												pVarNode->AddLastChild(pArrayInit);
											}
											else
											{
												// Create an assign operator to initialise the variable.
												CqParseNode* pV=new CqParseNodeAssign(var);
												pV->SetPos(LineNo(),FileName());
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
	|	EXTERN typespec def_expressions
							{
								$$=new CqParseNode();
								$$->SetPos(LineNo(),FileName());
								CqParseNodeDeclaration* pDecl=static_cast<CqParseNodeDeclaration*>($3->pFirstChild());
								while(pDecl)
								{
									SqVarRef varLocal, varExtern;
									TqBool fvl=CqVarDef::FindVariable((strNameSpace()+pDecl->strName()).c_str(), varLocal);

									// As this is an extern, we need to repeatedly check in the previous namespaces
									// until we find the variable they are referring to.
									TqBool fve=TqFalse;
									if(!m_stkstrNameSpace.empty())
									{
										std::vector<CqString>::reverse_iterator i=m_stkstrNameSpace.rbegin()+1;
										while(!fve && i!=m_stkstrNameSpace.rend())
										{
											CqString strNS=*i;
											fve=CqVarDef::FindVariable((strNS+pDecl->strName()).c_str(), varExtern);
											i++;
										}
									}

									// If we found a candidate...
									if(fve && fvl)
									{
										CqVarDef* pvarLocal=CqVarDef::GetVariablePtr(varLocal);
										CqVarDef* pvarExtern=CqVarDef::GetVariablePtr(varExtern);
										CqParseNode* pVarNode=new CqParseNodeVariable(varLocal);
										pvarLocal->SetExtern(TqTrue, varExtern);
										pvarLocal->SetType(pvarExtern->Type());
										pVarNode->SetPos(LineNo(),FileName());
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
								$$.Type=(EqVariableType)($2.Type|$1);
								$$.Space=$2.Space;
							}
	|	type
	;

def_expressions
	:	def_expression		{
								// Create a list header and add the first element.
								$$=new CqParseNode();
								$$->SetPos(LineNo(),FileName());
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
								$$->SetPos(LineNo(),FileName());
								CqVarDef::AddVariable(CqVarDef(Type_Void, (strNameSpace()+*$1).c_str()));
								// Add the initialiser as the first child of the declaration.
								$$->AddLastChild($2);
							}
	|	IDENTIFIER			{
								// Create a new variable declaration based on the specified name.
								$$=new CqParseNodeDeclaration($1->c_str());
								$$->SetPos(LineNo(),FileName());
								CqVarDef::AddVariable(CqVarDef(Type_Void, (strNameSpace()+*$1).c_str()));
							}
	|	IDENTIFIER '[' number ']' {
								// Create a new variable declaration based on the specified name.
								$$=new CqParseNodeDeclaration($1->c_str());
								$$->SetPos(LineNo(),FileName());
								CqVarDef::AddVariable(CqVarDef(Type_Array, (strNameSpace()+*$1).c_str(),$3));
							}
	|	IDENTIFIER '[' number ']' def_array_initialisers
							{
								// Create a new variable declaration based on the specified name.
								$$=new CqParseNodeDeclaration($1->c_str());
								$$->SetPos(LineNo(),FileName());
								CqVarDef::AddVariable(CqVarDef(Type_Array, (strNameSpace()+*$1).c_str(),$3));
								$$->AddLastChild($5);
							}
	|	SYMBOL def_init		{
								// Create a new declaration based on the name.
								// TODO: Should warn about duplicate declarations.
								CqString strName("");
								if($1.eType&1)	strName=CqVarDef::GetVariablePtr($1.VarRef)->strName();
								else			strName=CqFuncDef::GetFunctionPtr($1.FuncRef)->strName();

								$$=new CqParseNodeDeclaration(strName.c_str());
								$$->SetPos(LineNo(),FileName());
								CqVarDef::AddVariable(CqVarDef(Type_Void, (strNameSpace()+strName).c_str()));
								// Add the initialiser as the first child of the declaration.
								$$->AddLastChild($2);
							}
	|	SYMBOL				{
								CqString strName("");
								if($1.eType&1)	strName=CqVarDef::GetVariablePtr($1.VarRef)->strName();
								else			strName=CqFuncDef::GetFunctionPtr($1.FuncRef)->strName();

								$$=new CqParseNodeDeclaration(strName.c_str());
								$$->SetPos(LineNo(),FileName());
								CqVarDef::AddVariable(CqVarDef(Type_Void, (strNameSpace()+strName).c_str()));
							}
	|	SYMBOL '[' number ']' {
								CqString strName("");
								if($1.eType&1)	strName=CqVarDef::GetVariablePtr($1.VarRef)->strName();
								else			strName=CqFuncDef::GetFunctionPtr($1.FuncRef)->strName();

								$$=new CqParseNodeDeclaration(strName.c_str());
								$$->SetPos(LineNo(),FileName());
								CqVarDef::AddVariable(CqVarDef(Type_Array, (strNameSpace()+strName).c_str(),$3));
							}
	|	SYMBOL '[' number ']' def_array_initialisers
							{
								CqString strName("");
								if($1.eType&1)	strName=CqVarDef::GetVariablePtr($1.VarRef)->strName();
								else			strName=CqFuncDef::GetFunctionPtr($1.FuncRef)->strName();

								$$=new CqParseNodeDeclaration(strName.c_str());
								$$->SetPos(LineNo(),FileName());
								CqVarDef::AddVariable(CqVarDef(Type_Array, (strNameSpace()+strName).c_str(),$3));
								$$->AddLastChild($5);
							}
	;

def_init
	:	'=' expression		{
								$$=$2;
							}

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
								$$->SetPos(LineNo(),FileName());
								$$->AddLastChild($1);
							}
	|	array_initialisers ',' expression
							{
								// This one to the list.
								$$->AddLastChild($3);
							}

detail
	:	VARYING				{	$$=Type_Varying;	}
	|	UNIFORM				{	$$=Type_Uniform;	}
	;

type
	:	FLOAT				{	
								$$.Type=Type_Float;		
								$$.Space=0;
							}
	|	STRING				{
								$$.Type=Type_String;	
								$$.Space=0;
							}
	|	TYPEVOID			{
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
	:	POINT spacetype		{
								$$.Type=Type_Point;		
								$$.Space=$2;
							}
	|	POINT				{
								$$.Type=Type_Point;		
								$$.Space=0;
							}
	;

cspace
	:	COLOR spacetype		{
								$$.Type=Type_Color;		
								$$.Space=$2;
							}
	|	COLOR				{
								$$.Type=Type_Color;		
								$$.Space=0;
							}
	;

vspace
	:	VECTOR spacetype	{
								$$.Type=Type_Vector;		
								$$.Space=$2;
							}
	|	VECTOR				{
								$$.Type=Type_Vector;		
								$$.Space=0;
							}
	;

nspace
	:	NORMAL spacetype	{
								$$.Type=Type_Normal;		
								$$.Space=$2;
							}
	|	NORMAL				{
								$$.Type=Type_Normal;		
								$$.Space=0;
							}
	;

mspace
	:	MATRIX spacetype	{
								$$.Type=Type_Matrix;		
								$$.Space=$2;
							}
	|	MATRIX				{
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
								$$->SetPos(LineNo(),FileName());
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
									$$->SetPos(LineNo(),FileName());
									$$->AddLastChild($1);
								}
							}
	|	comm_function ';'	{
								$$=new CqParseNodeDrop();
								$$->SetPos(LineNo(),FileName());
								$$->AddLastChild($1);
							}
	|	RETURN expression ';'
							{
								$$=new CqParseNode();
								$$->SetPos(LineNo(),FileName());
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
	|	'{' statements '}'	{	$$=$2;	}
	|	'{' '}'				{	$$=new CqParseNode(); }
	|	';'					{	$$=new CqParseNode(); }
	|	IF relation statement
							{
								CqParseNode* pNew=new CqParseNodeConditional();
								pNew->SetPos(LineNo(),FileName());
								pNew->AddLastChild($2);
								pNew->AddLastChild($3);
								$$=pNew;
							}
	|	IF relation statement ELSE statement
							{
								CqParseNode* pNew=new CqParseNodeConditional();
								pNew->SetPos(LineNo(),FileName());
								pNew->AddLastChild($2);
								pNew->AddLastChild($3);
								pNew->AddLastChild($5);
								$$=pNew;
							}
	;



loop_control
	:	WHILE relation statement
							{
								$$=new CqParseNodeWhileConstruct();
								$$->SetPos(LineNo(),FileName());
								$$->AddLastChild($2);
								$$->AddLastChild($3);
							}
	|	FOR '(' expression ';' relation ';' expression ')' statement
							{
								$$=new CqParseNode();
								$$->SetPos(LineNo(),FileName());
								$$->AddLastChild($3);
								CqParseNode* pW=new CqParseNodeWhileConstruct();
								pW->SetPos(LineNo(),FileName());
								$$->AddLastChild(pW);
								pW->AddLastChild($5);
								pW->AddLastChild($9);
								pW->AddLastChild($7);
								// Make sure that any assigns in the two expressions don't dup
								$3->NoDup();
								$7->NoDup();
							}
	|	SOLAR '(' ')' statement
							{
								$$=new CqParseNodeSolarConstruct();
								$$->SetPos(LineNo(),FileName());
								$$->AddLastChild($4);
							}
	|	SOLAR '(' expression ',' expression ')' statement
							{
								$$=new CqParseNodeSolarConstruct(TqTrue);
								$$->SetPos(LineNo(),FileName());
								CqParseNode* pArg=new CqParseNode();
								pArg->SetPos(LineNo(),FileName());
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
								$$->SetPos(LineNo(),FileName());
								$$->AddLastChild($3);
								$$->AddLastChild($5);
								// Make sure that any assigns in the two expressions don't dup
								$3->NoDup();
							}
	|	ILLUMINATE '(' expression ',' expression ',' expression ')' statement
							{
								$$=new CqParseNodeIlluminateConstruct(TqTrue);
								$$->SetPos(LineNo(),FileName());
								CqParseNode* pArg=new CqParseNode();
								pArg->SetPos(LineNo(),FileName());
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
								$$->SetPos(LineNo(),FileName());
								CqParseNode* pArg=new CqParseNode();
								pArg->SetPos(LineNo(),FileName());
								pArg->AddFirstChild($3);
								pArg->AddFirstChild(new CqParseNodeFloatConst(0));	// [nsamples]
								$$->AddLastChild(pArg);
								$$->AddLastChild($5);
								// Make sure that any assigns in the two expressions don't dup
								$3->NoDup();
							}
	|	ILLUMINANCE '(' expression ',' expression ')' statement
							{
								$$=new CqParseNodeIlluminanceConstruct();
								$$->SetPos(LineNo(),FileName());
								CqParseNode* pArg=new CqParseNode();
								pArg->SetPos(LineNo(),FileName());
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
								$$=new CqParseNodeIlluminanceConstruct(TqTrue);
								$$->SetPos(LineNo(),FileName());
								CqParseNode* pArg=new CqParseNode();
								pArg->SetPos(LineNo(),FileName());
								pArg->AddFirstChild($3);
								pArg->AddFirstChild($5);
								pArg->AddFirstChild($7);
								pArg->AddFirstChild(new CqParseNodeFloatConst(0));	// [nsamples]
								$$->AddLastChild(pArg);
								$$->AddLastChild($9);
								// Make sure that any assigns in the two expressions don't dup
								$3->NoDup();
								$5->NoDup();
								$7->NoDup();
							}
	| ILLUMINANCE '(' expression ',' expression ',' expression ',' expression ')' statement
							{
								$$=new CqParseNodeIlluminanceConstruct(TqTrue);
								$$->SetPos(LineNo(),FileName());
								CqParseNode* pArg=new CqParseNode();
								pArg->SetPos(LineNo(),FileName());
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
	;

loop_modstmt
	:	loop_mod number
	|	loop_mod
	;

loop_mod
	:	BREAK				{	
								$$=new CqParseNode();	
								$$->SetPos(LineNo(),FileName());
							}
	|	CONTINUE			{
								$$=new CqParseNode();	
								$$->SetPos(LineNo(),FileName());
							}

	;

expression
	:	primary				
	|	expression '.' expression
							{
								std::vector<SqFuncRef> func;
								CqFuncDef::FindFunction("operator.", func);
								CqParseNodeFunction* pFunc=new CqParseNodeFunction(func);
								pFunc->SetPos(LineNo(),FileName());
								pFunc->AddLastChild($1);
								pFunc->AddLastChild($3);

								$$=pFunc;
							}
	|	expression '/' expression
							{
								std::vector<SqFuncRef> func;
								CqFuncDef::FindFunction("operator/", func);
								CqParseNodeFunction* pFunc=new CqParseNodeFunction(func);
								pFunc->SetPos(LineNo(),FileName());
								pFunc->AddLastChild($1);
								pFunc->AddLastChild($3);

								$$=pFunc;
							}
	|	expression '*' expression
							{
								std::vector<SqFuncRef> func;
								CqFuncDef::FindFunction("operator*", func);
								CqParseNodeFunction* pFunc=new CqParseNodeFunction(func);
								pFunc->SetPos(LineNo(),FileName());
								pFunc->AddLastChild($1);
								pFunc->AddLastChild($3);

								$$=pFunc;
							}
	|	expression '^' expression
							{
								std::vector<SqFuncRef> func;
								CqFuncDef::FindFunction("operator^", func);
								CqParseNodeFunction* pFunc=new CqParseNodeFunction(func);
								pFunc->SetPos(LineNo(),FileName());
								pFunc->AddLastChild($1);
								pFunc->AddLastChild($3);

								$$=pFunc;
							}
	|	expression '+' expression
							{
								std::vector<SqFuncRef> func;
								CqFuncDef::FindFunction("operator+", func);
								CqParseNodeFunction* pFunc=new CqParseNodeFunction(func);
								pFunc->SetPos(LineNo(),FileName());
								pFunc->AddLastChild($1);
								pFunc->AddLastChild($3);

								$$=pFunc;
							}
	|	expression '-' expression
							{
								std::vector<SqFuncRef> func;
								CqFuncDef::FindFunction("operator-", func);
								CqParseNodeFunction* pFunc=new CqParseNodeFunction(func);
								pFunc->SetPos(LineNo(),FileName());
								pFunc->AddLastChild($1);
								pFunc->AddLastChild($3);

								$$=pFunc;
							}
	|	'-' expression	%prec NEG
							{
								std::vector<SqFuncRef> func;
								CqFuncDef::FindFunction("operator-", func);
								CqParseNodeFunction* pFunc=new CqParseNodeFunction(func);
								pFunc->SetPos(LineNo(),FileName());
								pFunc->AddFirstChild($2);

								$$=pFunc;
							}
	|	cast_expr
	|	relation '?' expression ':' expression
							{
//								if($3->ResType() == $5->ResType())
//								{
									CqParseNode* pNew=new CqParseNodeQCond();
									pNew->SetPos(LineNo(),FileName());
									pNew->AddLastChild($1);
									pNew->AddLastChild($3);
									pNew->AddLastChild($5);
									$$=pNew;
//								}
//								else
//								{
//									yyerror("expressions don't match");
//									$$=new CqParseNode();
//								}
							}
	;


cast_expr
	:	type expression	 %prec '('	{
								$$=new CqParseNode();
								$$->SetPos(LineNo(),FileName());
								CqParseNode* pCast=new CqParseNodeCast($1.Type);
								pCast->SetPos(LineNo(),FileName());
								pCast->AddFirstChild($2);
								
								EqVariableType Type=(EqVariableType)($1.Type&Type_Mask);
								CqString* pSpace=$1.Space;
								// Check if the type has a valid space associated
								if(pSpace!=0 && pSpace->compare("")!=0 && 
								  ((Type==Type_Point) ||
								   (Type==Type_Normal) ||
								   (Type==Type_Vector) ||
								   (Type==Type_Matrix) ))
								{
									// Create a transform function.
									std::vector<SqFuncRef> funcTrans;
									CqString strTrans("transform");;
									if(Type==Type_Normal)	strTrans="ntransform";
									if(Type==Type_Vector)	strTrans="vtransform";
									if(Type==Type_Matrix)	strTrans="mtransform";
									if(FindFunction(strTrans.c_str(), funcTrans))
									{
										$$=new CqParseNodeFunction(funcTrans);
										$$->SetPos(LineNo(),FileName());
									
										// And create a holder for the arguments.
										CqParseNode* pFromSpace=new CqParseNodeStringConst((*$1.Space).c_str());
										CqParseNode* pToSpace=new CqParseNodeStringConst("current");
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
								$$->SetPos(LineNo(),FileName());
							}
	|	texture
	|	SYMBOL				{
								$$=new CqParseNodeVariable($1.VarRef);	
								$$->SetPos(LineNo(),FileName());
							}
	|	SYMBOL '[' expression ']' {
								$$=new CqParseNodeVariableArray($1.VarRef);
								$$->AddLastChild($3);
								$$->SetPos(LineNo(),FileName());
							}
	|	STRING_LITERAL		{
								$$=new CqParseNodeStringConst($1->c_str());
								$$->SetPos(LineNo(),FileName());
							}
	|	procedurecall		
	|	comm_function
	|	assignexpression
	|	'(' expression ')'	{	
								$$=$2;
							}
	|	'(' expression ',' expression ',' expression ')'
							{
								$$=new CqParseNodeTriple();
								$$->SetPos(LineNo(),FileName());
								$$->AddLastChild($2);
								$$->AddLastChild($4);
								$$->AddLastChild($6);
							}
	|	'(' expression ',' expression ',' expression ',' expression ',' expression ',' expression ',' expression ',' expression ',' expression ',' expression ',' expression ',' expression ',' expression ',' expression ',' expression ',' expression ')'
							{
								$$=new CqParseNodeHexTuple();
								$$->SetPos(LineNo(),FileName());
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
								$$->SetPos(LineNo(),FileName());
								$$->AddFirstChild($1);
								$$->AddFirstChild($3);
							}
	|	relation AND_OP relation
							{
								$$=new CqParseNodeLogicalOp(Op_LogAnd);
								$$->SetPos(LineNo(),FileName());
								$$->AddFirstChild($1);
								$$->AddLastChild($3);
							}
	|	relation OR_OP relation
							{
								$$=new CqParseNodeLogicalOp(Op_LogOr);
								$$->SetPos(LineNo(),FileName());
								$$->AddFirstChild($1);
								$$->AddLastChild($3);
							}
	|	'!' relation		{
								$$=new CqParseNodeUnaryOp(Op_LogicalNot);
								$$->SetPos(LineNo(),FileName());
								$$->AddLastChild($2);
							}
	;

assignexpression
	:	SYMBOL '=' expression
							{
								$$=new CqParseNodeAssign($1.VarRef);
								$$->SetPos(LineNo(),FileName());
								$$->AddLastChild($3);
							}
	|	SYMBOL ADD_ASSIGN expression
							{
								$$=new CqParseNodeAssign($1.VarRef);
								$$->SetPos(LineNo(),FileName());
								
								std::vector<SqFuncRef> func;
								CqFuncDef::FindFunction("operator+", func);
								CqParseNodeFunction* pFunc=new CqParseNodeFunction(func);
								pFunc->SetPos(LineNo(),FileName());
								pFunc->AddLastChild(new CqParseNodeVariable($1.VarRef));
								pFunc->AddLastChild($3);

								$$->AddLastChild(pFunc);
							}
	|	SYMBOL SUB_ASSIGN expression
							{
								$$=new CqParseNodeAssign($1.VarRef);
								$$->SetPos(LineNo(),FileName());
								
								std::vector<SqFuncRef> func;
								CqFuncDef::FindFunction("operator-", func);
								CqParseNodeFunction* pFunc=new CqParseNodeFunction(func);
								pFunc->SetPos(LineNo(),FileName());
								pFunc->AddLastChild(new CqParseNodeVariable($1.VarRef));
								pFunc->AddLastChild($3);

								$$->AddLastChild(pFunc);
							}
	|	SYMBOL MUL_ASSIGN expression
							{
								$$=new CqParseNodeAssign($1.VarRef);
								$$->SetPos(LineNo(),FileName());
								
								std::vector<SqFuncRef> func;
								CqFuncDef::FindFunction("operator*", func);
								CqParseNodeFunction* pFunc=new CqParseNodeFunction(func);
								pFunc->SetPos(LineNo(),FileName());
								pFunc->AddLastChild(new CqParseNodeVariable($1.VarRef));
								pFunc->AddLastChild($3);

								$$->AddLastChild(pFunc);
							}
	|	SYMBOL DIV_ASSIGN expression
							{
								$$=new CqParseNodeAssign($1.VarRef);
								$$->SetPos(LineNo(),FileName());
								
								std::vector<SqFuncRef> func;
								CqFuncDef::FindFunction("operator/", func);
								CqParseNodeFunction* pFunc=new CqParseNodeFunction(func);
								pFunc->SetPos(LineNo(),FileName());
								pFunc->AddLastChild(new CqParseNodeVariable($1.VarRef));
								pFunc->AddLastChild($3);

								$$->AddLastChild(pFunc);
							}
	|	SYMBOL '[' expression ']' '=' expression
							{
								$$=new CqParseNodeAssignArray($1.VarRef);
								$$->SetPos(LineNo(),FileName());
								$$->AddLastChild($6);
								$$->AddLastChild($3);
							}
	|	SYMBOL  '[' expression ']' ADD_ASSIGN expression
							{
								$$=new CqParseNodeAssignArray($1.VarRef);
								$$->SetPos(LineNo(),FileName());
								
								std::vector<SqFuncRef> func;
								CqFuncDef::FindFunction("operator+", func);
								CqParseNodeFunction* pFunc=new CqParseNodeFunction(func);
								pFunc->SetPos(LineNo(),FileName());
								CqParseNodeVariableArray* pVar=new CqParseNodeVariableArray($1.VarRef);
								pVar->AddLastChild($3->Clone());
								pFunc->AddLastChild(pVar);

								pFunc->AddLastChild($6);

								$$->AddLastChild(pFunc);
								$$->AddLastChild($3);
							}
	|	SYMBOL  '[' expression ']' SUB_ASSIGN expression
							{
								$$=new CqParseNodeAssignArray($1.VarRef);
								$$->SetPos(LineNo(),FileName());
								
								std::vector<SqFuncRef> func;
								CqFuncDef::FindFunction("operator-", func);
								CqParseNodeFunction* pFunc=new CqParseNodeFunction(func);
								pFunc->SetPos(LineNo(),FileName());
								CqParseNodeVariableArray* pVar=new CqParseNodeVariableArray($1.VarRef);
								pVar->AddLastChild($3->Clone());
								pFunc->AddLastChild(pVar);

								pFunc->AddLastChild($6);

								$$->AddLastChild(pFunc);
								$$->AddLastChild($3);
							}
	|	SYMBOL  '[' expression ']' MUL_ASSIGN expression
							{
								$$=new CqParseNodeAssignArray($1.VarRef);
								$$->SetPos(LineNo(),FileName());
								
								std::vector<SqFuncRef> func;
								CqFuncDef::FindFunction("operator*", func);
								CqParseNodeFunction* pFunc=new CqParseNodeFunction(func);
								pFunc->SetPos(LineNo(),FileName());
								CqParseNodeVariableArray* pVar=new CqParseNodeVariableArray($1.VarRef);
								pVar->AddLastChild($3->Clone());
								pFunc->AddLastChild(pVar);

								pFunc->AddLastChild($6);

								$$->AddLastChild(pFunc);
								$$->AddLastChild($3);
							}
	|	SYMBOL  '[' expression ']' DIV_ASSIGN expression
							{
								$$=new CqParseNodeAssignArray($1.VarRef);
								$$->SetPos(LineNo(),FileName());
								
								std::vector<SqFuncRef> func;
								CqFuncDef::FindFunction("operator/", func);
								CqParseNodeFunction* pFunc=new CqParseNodeFunction(func);
								pFunc->SetPos(LineNo(),FileName());
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
								CqParseNodeFunction* pFunc=new CqParseNodeFunction(func);
								pFunc->SetPos(LineNo(),FileName());
								while($3->pFirstChild()!=0)	pFunc->AddLastChild($3->pFirstChild());

								$$=pFunc;
							}
	|	SYMBOL '(' ')'		{
								std::vector<SqFuncRef> func;
								CqFuncDef::FindFunction(CqFuncDef::GetFunctionPtr($1.FuncRef)->strName(), func);
								CqParseNodeFunction* pFunc=new CqParseNodeFunction(func);
								pFunc->SetPos(LineNo(),FileName());

								$$=pFunc;
							}
	;

proc_arguments
	:	expression			{
								// Create a list header, and add the first entry.
								$$=new CqParseNode();
								$$->SetPos(LineNo(),FileName());
								$$->AddLastChild($1);
							}
	|	proc_arguments ',' expression
							{
								// Add this entry to the list.
								$$->AddLastChild($3);
							}
	;

texture
	:	texture_type '(' texture_filename channel texture_arguments ')'
							{	
								// Add the texture_filename as the first argument
								$$->AddFirstChild($3);
								$$->AddLastChild($4);
								// Add all texture_arguments as further arguments to the function.
								CqParseNode* pParam=$5->pFirstChild();
								while(pParam!=0)
								{
									CqParseNode* pTemp=pParam->pNext();
									$$->AddLastChild(pParam);
									pParam=pTemp;
								}
							}
	|	texture_type '(' texture_filename texture_arguments ')'
							{	
								// Add the texture_filename as the first argument
								$$->AddFirstChild($3);
								$$->AddLastChild(new CqParseNodeFloatConst(0));
								// Add all texture_arguments as further arguments to the function.
								CqParseNode* pParam=$4->pFirstChild();
								while(pParam!=0)
								{
									CqParseNode* pTemp=pParam->pNext();
									$$->AddLastChild(pParam);
									pParam=pTemp;
								}
							}
	|	texture_type '(' texture_filename channel ')'
							{	
								// Add the texture_filename as the first argument
								$$->AddFirstChild($3);
								$$->AddLastChild($4);
							}
	|	texture_type '(' texture_filename ')'
							{	
								// Add the texture_filename as the first argument
								$$->AddFirstChild($3);
								$$->AddLastChild(new CqParseNodeFloatConst(0));
							}
	;

texture_type
	:	TEXTURE				{
								std::vector<SqFuncRef> func;
								CqFuncDef::FindFunction("texture", func);
								$$=new CqParseNodeFunction(func);
								$$->SetPos(LineNo(),FileName());
							}
	|	ENVIRONMENT			{
								std::vector<SqFuncRef> func;
								CqFuncDef::FindFunction("environment", func);
								$$=new CqParseNodeFunction(func);
								$$->SetPos(LineNo(),FileName());
							}
	|	BUMP				{
								std::vector<SqFuncRef> func;
								CqFuncDef::FindFunction("bump", func);
								$$=new CqParseNodeFunction(func);
								$$->SetPos(LineNo(),FileName());
							}
	|	SHADOW				{
								std::vector<SqFuncRef> func;
								CqFuncDef::FindFunction("shadow", func);
								$$=new CqParseNodeFunction(func);
								$$->SetPos(LineNo(),FileName());
							}
	;

texture_filename
	:	SYMBOL				{	
								$$=new CqParseNodeVariable($1.VarRef);	
								$$->SetPos(LineNo(),FileName());
							}
	|	STRING_LITERAL		{	
								$$=new CqParseNodeStringConst($1->c_str());	
								$$->SetPos(LineNo(),FileName());
							}
	;

channel
	:	'[' expression ']'	{	$$=$2; }
	;

texture_arguments
	:	',' expression		{
								// Create a list header and add the first entry.
								$$=new CqParseNode();
								$$->SetPos(LineNo(),FileName());
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
	:	ATMOSPHERE		{$$=CommTypeAtmosphere;}
	|	DISPLACEMENT	{$$=CommTypeDisplacement;}
	|	LIGHTSOURCE		{$$=CommTypeLightsource;}
	|	SURFACE			{$$=CommTypeSurface;}
	|	ATTRIBUTE		{$$=CommTypeAttribute;}
	|	OPTION			{$$=CommTypeOption;}
	|	RENDERERINFO	{$$=CommTypeRendererInfo;}
	|	INCIDENT		{$$=CommTypeIncident;}
	|	OPPOSITE		{$$=CommTypeOpposite;}
	;

comm_function
	:	comm_type '(' expression ',' SYMBOL ')'
							{
								CqVarDef* pVD=0;
								TqBool fError=TqFalse;
								
								if(($3->ResType()&Type_Mask)!=Type_String)	fError=TqTrue;

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
	;


%%

TqBool SLParser::FindVariable(const char* name, SqVarRef& Ref)
{
	// First search in the current namespace, then in the global namespace.
	CqString strLocalVar(strNameSpace()+name);
	
	if(CqVarDef::FindVariable(strLocalVar.c_str(), Ref))	return(TqTrue);
	else	return(CqVarDef::FindVariable(name, Ref));

	
}


TqBool SLParser::FindFunction(const char* name, std::vector<SqFuncRef>& Ref)
{
	// First search in the current namespace, then in the global namespace.
	CqString strLocalFunc(strNameSpace()+name);
	
	if(CqFuncDef::FindFunction(strLocalFunc.c_str(), Ref))	return(TqTrue);
	else	return(CqFuncDef::FindFunction(name, Ref));
}


void SLParser::Output(std::ostream& Out)
{
	TqInt i=0;
	// Output the shader type.
	Out << m_Shader.strSType() << std::endl;

	// Output version information.
	Out << "AQSIS_V " << VERSION_STR << std::endl;

	Out << std::endl << std::endl << "segment Data" << std::endl;

	// Do a first pass output to find out which variables are used.
	if(m_ParseTree)
	{
		gInternalFunctionUsage=0;
		std::ostrstream strNull;
		m_ParseTree->Output(strNull);
	}

	// Now that we have this information, work out which standard vars are used.
	TqInt Use=gInternalFunctionUsage;
	for(i=0; i<EnvVars_Last; i++)
	{
		if(gStandardVars[i].UseCount()>0)
			Use|=(0x00000001<<i);
	}
	Out << std::endl << "USES " << Use << std::endl << std::endl;
	
	// Output any declared variables.
	for(i=0; i<gLocalVars.size(); i++)
		Out << gLocalVars[i];

	Out << std::endl << std::endl << "segment Init" << std::endl;
	for(i=0; i<gLocalVars.size(); i++)
		gLocalVars[i].OutputInit(Out);

	Out << std::endl << std::endl << "segment Code" << std::endl;
	
	if(m_ParseTree)
		m_ParseTree->Output(Out);
}


void SLParser::TypeCheck()
{
	// Typecheck any declared variables.
	TqInt i;
	for(i=0; i<gLocalVars.size(); i++)
	{
		if(gLocalVars[i].pDefValue()!=0)
			gLocalVars[i].pDefValue()->TypeCheck(CqParseNode::pAllTypes(), Type_Last-1);
	}

	// Typecheck any local functions.
	for(i=0; i<gLocalFuncs.size(); i++)
	{
		if(gLocalFuncs[i].pDef()!=0)
		{
			EqVariableType RetType=gLocalFuncs[i].Type();
			gLocalFuncs[i].pDef()->TypeCheck(CqParseNode::pAllTypes(), Type_Last-1);
		}
	}

	if(m_ParseTree)
		m_ParseTree->TypeCheck(CqParseNode::pAllTypes(), Type_Last-1);
}


void SLParser::Optimise()
{
	// Optimise any local functions.
	TqInt i;
	for(i=0; i<gLocalFuncs.size(); i++)
	{
		if(gLocalFuncs[i].pDef()!=0)
			gLocalFuncs[i].pDef()->Optimise();
	}

	if(m_ParseTree)
		m_ParseTree->Optimise();
}


CqString SLParser::strNameSpace()	
{
	CqString strRes("");

	if(!m_stkstrNameSpace.empty())
		strRes=m_stkstrNameSpace.back();

	return(strRes);
}

void SLParser::InitStandardNamespace()
{
	m_stkstrNameSpace.push_back("");
}