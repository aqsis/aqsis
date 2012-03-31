/* -------------- declaration section -------------- */


%{

#include <cstring> // for strcspn, strchr
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "parsenode.h"

using namespace Aqsis;

#include "parser.hpp"

#ifdef	WIN32
//extern "C" TqInt isatty(TqInt);
#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif
#endif

namespace Aqsis
{

extern std::istream* ParseInputStream;
extern CqString		ParseStreamName;	
extern TqInt ParseLineNumber;

extern bool	FindVariable(const char* name, SqVarRef& ref);
extern bool	FindFunction(const char* name, std::vector<SqFuncRef>& Ref);
extern CqString strNameSpace();

extern std::vector<std::pair<bool,CqString> >	ParseNameSpaceStack;
}

static TqInt scannerinput(char* Buffer, TqInt MaxSize);
#undef YY_INPUT
#define YY_INPUT(buffer, result, max_size) (result = scannerinput(buffer, max_size))

%}

%option nounput

D			[0-9]
L			[a-zA-Z_]
H			[a-fA-F0-9]
E			[Ee][+-]?{D}+
FS			(f|F|l|L)
IS			(u|U|l|L)*
LF			(\r\n)|(\r)|(\n)|(\x0c)
WS			[ \t\h]
hashline	#{WS}?(line|{D}+)
hash		#{WS}?((pragma)).*{LF}
string		\"(\\.|[^\\"])*\"

%{
#define	YY_SKIP_YYWRAP
TqInt yywrap();
static TqInt check_type();
%}

%%

{LF}			{ ParseLineNumber+=+1; }

"break"			{ return(BREAK); }
"continue"		{ return(CONTINUE); }
"else"			{ return(ELSE); }
"float"			{ return(TYPE_FLOAT); }
"point"			{ return(TYPE_POINT); }
"vector"		{ return(TYPE_VECTOR); }
"normal"		{ return(TYPE_NORMAL); }
"string"		{ return(TYPE_STRING); }
"void"			{ return(TYPE_VOID); }
"matrix"		{ return(TYPE_MATRIX); }
"color"			{ return(TYPE_COLOR); }
"for"			{ return(FOR); }
"if"			{ return(IF); }
"return"		{ return(RETURN); }
"while"			{ return(WHILE); }
"uniform"		{ return(TYPE_UNIFORM); }
"varying"		{ return(TYPE_VARYING); }
"output"		{ return(OUTPUT); }
"extern"		{ return(EXTERN); }
"atmosphere"	{ return(SHADER_TYPE_ATMOSPHERE); }
"surface"		{ return(SHADER_TYPE_SURFACE); }
"volume"		{ return(SHADER_TYPE_VOLUME); }
"displacement"	{ return(SHADER_TYPE_DISPLACEMENT); }
"imager"		{ return(SHADER_TYPE_IMAGER); }
"attribute"		{ return(ATTRIBUTE);}
"option"		{ return(OPTION);}
"rendererinfo"	{ return(RENDERERINFO);}
"incident"		{ return(INCIDENT);}
"opposite"		{ return(OPPOSITE);}
"transformation" { return(SHADER_TYPE_TRANSFORMATION); }
"light"			{ return(SHADER_TYPE_LIGHT); }
"lightsource"	{ return(LIGHTSOURCE); }
"illuminate"	{ return(ILLUMINATE);}
"illuminance"	{ return(ILLUMINANCE);}
"solar"			{ return(SOLAR);}
"gather"	{ return(GATHER);}
"texture"		{ return(TEXTUREMAP);}
"environment"	{ return(ENVIRONMENT);}
"bump"			{ return(BUMP);}
"shadow"		{ return(SHADOW);}
"occlusion"		{ return(OCCLUSION);}
"textureinfo"	{ return(TEXTUREINFO);}
"rayinfo"	{ return(RAYINFO);}

{hashline}.*{LF}	{
					// Find the start of the line no.
					TqInt i=0, ln=0;
					i=strcspn((char*)yytext, "0123456789");
					char* endptr;
					ln=strtol((char*)yytext+i, &endptr, 10);
					ParseLineNumber=ln;
					// Now check if there is a new filename specified.
					char* fname;
					if((fname=strchr(endptr, '\"'))!=0)
					{
						fname++;
						if((i=strcspn(fname, "\""))>0)
						{
							CqString strfName(fname);
							ParseStreamName=strfName.substr(0,i).c_str();
						}
					}
				}
{hash}			{ ParseLineNumber+=1; }

{L}({L}|{D})*	{ yylval.m_Identifier=new CqString((char*)yytext); return(check_type()); }

0[xX]{H}+{IS}?	{ yylval.m_FloatConst=(TqFloat)atof((char*)yytext); return(FLOAT_CONSTANT); }
0{D}+{IS}?		{ yylval.m_FloatConst=(TqFloat)atof((char*)yytext); return(FLOAT_CONSTANT); }
{D}+{IS}?		{ yylval.m_FloatConst=(TqFloat)atof((char*)yytext); return(FLOAT_CONSTANT); }
'(\\.|[^\\'])+'	{ yylval.m_FloatConst=(TqFloat)atof((char*)yytext); return(FLOAT_CONSTANT); }

{D}+{E}{FS}?	{ yylval.m_FloatConst=(TqFloat)atof((char*)yytext); return(FLOAT_CONSTANT); }
{D}*"."{D}+({E})?{FS}?	{ yylval.m_FloatConst=(TqFloat)atof((char*)yytext); return(FLOAT_CONSTANT); }
{D}+"."{D}*({E})?{FS}?	{ yylval.m_FloatConst=(TqFloat)atof((char*)yytext); return(FLOAT_CONSTANT); }

{string}		{ 
					CqString strText((char*)yytext);
					yylval.m_Identifier=new CqString(strText.substr(1,strText.size()-2)); 
					return(STRING_LITERAL); 
				}

"+="			{ return(ADD_ASSIGN); }
"-="			{ return(SUB_ASSIGN); }
"*="			{ return(MUL_ASSIGN); }
"/="			{ return(DIV_ASSIGN); }
"&&"			{ return(AND_OP); }
"||"			{ return(OR_OP); }
"<="			{ return(LE_OP); }
">="			{ return(GE_OP); }
"=="			{ return(EQ_OP); }
"!="			{ return(NE_OP); }
";"				{ return(';'); }
"{"				{ return('{'); }
"}"				{ return('}'); }
","				{ return(','); }
":"				{ return(':'); }
"="				{ return('='); }
"("				{ return('('); }
")"				{ return(')'); }
"["				{ return('['); }
"]"				{ return(']'); }
"."				{ return('.'); }
"&"				{ return('&'); }
"!"				{ return('!'); }
"~"				{ return('~'); }
"-"				{ return('-'); }
"+"				{ return('+'); }
"*"				{ return('*'); }
"/"				{ return('/'); }
"%"				{ return('%'); }
"<"				{ return('<'); }
">"				{ return('>'); }
"^"				{ return('^'); }
"|"				{ return('|'); }
"?"				{ return('?'); }

[ \t]		{ }
.			{ /* ignore bad characters */ }

%%

TqInt yywrap()
{
	return(1);
}


TqInt check_type()
{
	// Check the type against known variables.
	TqInt Ret=IDENTIFIER;
	SqVarRef var;
	CqString strName(strNameSpace());
	strName+=(char*)yytext;
	CqVarDef* pVar = 0;
	
	yylval.m_pSymbol.eType=0;

	// First check for local variables in the nested namespaces.
	std::vector<std::pair<bool,CqString> >::reverse_iterator i=ParseNameSpaceStack.rbegin();
	bool fv = FindVariable((strNameSpace()+(char*)yytext).c_str(), var);
	while(!fv && i!=ParseNameSpaceStack.rend())
	{
		CqString strNS=i->second;
		fv=CqVarDef::FindVariable((strNS+(char*)yytext).c_str(), var);
		// Exit on finding a terminal scope (function).
		if(i->first)
			break;
		i++;
	}
	// If a local variable was found, it must hide any global variables 
	// of the same name.
	if(fv)
	{
		yylval.m_pSymbol.VarRef=var;
		yylval.m_pSymbol.eType=1;
		pVar=CqVarDef::GetVariablePtr(var);
	}
	// Otherwise, check for global variables.
	else
	{
		// Check the type against global variables.
		strName=(char*)yytext;
		if(FindVariable(strName.c_str(), var))
		{
			yylval.m_pSymbol.VarRef=var;
			yylval.m_pSymbol.eType=1;
			pVar=CqVarDef::GetVariablePtr(var);
		}
	}

	// Check the type against known functions.
	bool isAFunction = false;
	std::vector<SqFuncRef> func;
	if(FindFunction((char*)yytext, func))
	{
		yylval.m_pSymbol.FuncRef=func[0];
		yylval.m_pSymbol.eType|=2;
		isAFunction = true;
	}

	// Check the resulting type, and return the appropriate
	// identifier.
	if(pVar && pVar->Type()&Type_Array)
		Ret=ARRAY_SYMBOL;
	else if(pVar)
		Ret=SYMBOL;
	else if(isAFunction)
		Ret=SYMBOL;

	return(Ret);
}

static TqInt scannerinput(char* Buffer, TqInt MaxSize)
{
	// Sanity checks ...
	assert(ParseInputStream);
	assert(Buffer);
	assert(MaxSize);

	TqInt count = 0;

	if(!ParseInputStream->eof())
	{
		ParseInputStream->read(Buffer,MaxSize);
		count=ParseInputStream->gcount();
		ParseInputStream->clear(ParseInputStream->rdstate()&(~std::ios::failbit));
		if(ParseInputStream->bad())
			count= -1;
	}

	return count;
}

