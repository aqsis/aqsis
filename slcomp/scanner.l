/* -------------- declaration section -------------- */
%name SLScanner

%define	IOSTREAM
%define LEX_PARAM YY_SLParser_STYPE *val,YY_SLParser_LTYPE *loc
%define MEMBERS public: std::istream* m_pfile; \
						SLParser* m_Parser; \
						void comment(); \
						int check_type();
%define CONSTRUCTOR_INIT : m_pfile(&std::cin)
%define	CONSTRUCTOR_PARAM	std::istream* pfile
%define	CONSTRUCTOR_CODE	m_pfile=pfile; \
							yyin=pfile;
%define DEBUG 0
%header{ 
%}

D			[0-9]
L			[a-zA-Z_]
H			[a-fA-F0-9]
E			[Ee][+-]?{D}+
FS			(f|F|l|L)
IS			(u|U|l|L)*
LF			(\r\n)|(\r)|(\n)|(\x0c)
WS			[ \t\h]
hashline	#{WS}?line
hash		#{WS}?((pragma)).*{LF}
string		\"(\\.|[^\\"])*\"

%{
int yywrap();
%}

%%
"/*"			{ comment(); }

{LF}			{ m_Parser->SetLineNo(m_Parser->LineNo()+1); }

"break"			{ return(SLParser::BREAK); }
"continue"		{ return(SLParser::CONTINUE); }
"else"			{ return(SLParser::ELSE); }
"float"			{ return(SLParser::FLOAT); }
"point"			{ return(SLParser::POINT); }
"vector"		{ return(SLParser::VECTOR); }
"normal"		{ return(SLParser::NORMAL); }
"string"		{ return(SLParser::STRING); }
"void"			{ return(SLParser::TYPEVOID); }
"matrix"		{ return(SLParser::MATRIX); }
"color"			{ return(SLParser::COLOR); }
"for"			{ return(SLParser::FOR); }
"if"			{ return(SLParser::IF); }
"return"		{ return(SLParser::RETURN); }
"while"			{ return(SLParser::WHILE); }
"uniform"		{ return(SLParser::UNIFORM); }
"varying"		{ return(SLParser::VARYING); }
"output"		{}
"extern"		{ return(SLParser::EXTERN); }
"atmosphere"	{ return(SLParser::ATMOSPHERE); }
"surface"		{ return(SLParser::SURFACE); }
"volume"		{ return(SLParser::VOLUME); }
"displacement"	{ return(SLParser::DISPLACEMENT); }
"imager"		{ return(SLParser::IMAGER); }
"attribute"		{ return(SLParser::ATTRIBUTE);}
"option"		{ return(SLParser::OPTION);}
"rendererinfo"	{ return(SLParser::RENDERERINFO);}
"incident"		{ return(SLParser::INCIDENT);}
"opposite"		{ return(SLParser::OPPOSITE);}
"transformation" { return(SLParser::TRANSFORMATION); }
"light"			{ return(SLParser::LIGHT); }
"lightsource"	{ return(SLParser::LIGHTSOURCE); }
"illuminate"	{ return(SLParser::ILLUMINATE);}
"illuminance"	{ return(SLParser::ILLUMINANCE);}
"solar"			{ return(SLParser::SOLAR);}
"texture"		{ return(SLParser::TEXTURE);}
"environment"	{ return(SLParser::ENVIRONMENT);}
"bump"			{ return(SLParser::BUMP);}
"shadow"		{ return(SLParser::SHADOW);}

{hashline}.*{D}.*{LF}	{
					// Find the start of the line no.
					TqInt i=0, ln=0;
					i=strcspn((char*)yytext, "0123456789");
					char* endptr;
					ln=strtol((char*)yytext+i, &endptr, 10);
					m_Parser->SetLineNo(ln);
					// Now check if there is a new filename specified.
					char* fname;
					if((fname=strchr(endptr, '\"'))!=0)
					{
						fname++;
						if((i=strcspn(fname, "\""))>0)
						{
							CqString strfName(fname);
							m_Parser->SetFileName(strfName.substr(0,i).c_str());
						}
					}
				}
{hash}			{ m_Parser->SetLineNo(m_Parser->LineNo()+1); }

{L}({L}|{D})*	{ val->m_Identifier=new CqString((char*)yytext); return(check_type()); }

0[xX]{H}+{IS}?	{ val->m_FloatConst=(TqFloat)atof((char*)yytext); return(SLParser::FLOAT_CONSTANT); }
0{D}+{IS}?		{ val->m_FloatConst=(TqFloat)atof((char*)yytext); return(SLParser::FLOAT_CONSTANT); }
{D}+{IS}?		{ val->m_FloatConst=(TqFloat)atof((char*)yytext); return(SLParser::FLOAT_CONSTANT); }
'(\\.|[^\\'])+'	{ val->m_FloatConst=(TqFloat)atof((char*)yytext); return(SLParser::FLOAT_CONSTANT); }

{D}+{E}{FS}?	{ val->m_FloatConst=(TqFloat)atof((char*)yytext); return(SLParser::FLOAT_CONSTANT); }
{D}*"."{D}+({E})?{FS}?	{ val->m_FloatConst=(TqFloat)atof((char*)yytext); return(SLParser::FLOAT_CONSTANT); }
{D}+"."{D}*({E})?{FS}?	{ val->m_FloatConst=(TqFloat)atof((char*)yytext); return(SLParser::FLOAT_CONSTANT); }

{string}		{ 
					CqString strText((char*)yytext);
					val->m_Identifier=new CqString(strText.substr(1,strText.size()-2)); 
					return(SLParser::STRING_LITERAL); 
				}

"+="			{ return(SLParser::ADD_ASSIGN); }
"-="			{ return(SLParser::SUB_ASSIGN); }
"*="			{ return(SLParser::MUL_ASSIGN); }
"/="			{ return(SLParser::DIV_ASSIGN); }
"&&"			{ return(SLParser::AND_OP); }
"||"			{ return(SLParser::OR_OP); }
"<="			{ return(SLParser::LE_OP); }
">="			{ return(SLParser::GE_OP); }
"=="			{ return(SLParser::EQ_OP); }
"!="			{ return(SLParser::NE_OP); }
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

#include <vector>
#include "Parser.h"

int yywrap()
{
	return(1);
}


void SLScanner::comment()
{
	char c, c1;

loop:
	while ((c = input()) != '*' && c != 0)
	{
		if(c=='\n' || c=='\r')
		{
			m_Parser->SetLineNo(m_Parser->LineNo()+1);
			c1=input();
			if((c=='\r' && c1!='\n') || (c=='\n' && c1!='\r'))
				unput(c1);
		}
	}

	if ((c1 = input()) != '/' && c != 0)
	{
		unput(c1);
		goto loop;
	}
}


int SLScanner::check_type()
{
	// Check the type against known variables.
	int Ret=SLParser::IDENTIFIER;
	SqVarRef var;
	CqString strName(m_Parser->strNameSpace());
	strName+=(char*)yytext;
	
	m_Parser->yylval.m_pSymbol.eType=0;

	if(m_Parser->FindVariable(strName.c_str(), var))
	{
		m_Parser->yylval.m_pSymbol.VarRef=var;
		m_Parser->yylval.m_pSymbol.eType=1;
		Ret=SLParser::SYMBOL;
	}

	// Check the type against global variables.
	strName=(char*)yytext;
	if(m_Parser->FindVariable(strName.c_str(), var))
	{
		m_Parser->yylval.m_pSymbol.VarRef=var;
		m_Parser->yylval.m_pSymbol.eType=1;
		Ret=SLParser::SYMBOL;
	}

	// Check the type against known functions.
	std::vector<SqFuncRef> func;
	if(m_Parser->FindFunction((char*)yytext, func))
	{
		m_Parser->yylval.m_pSymbol.FuncRef=func[0];
		m_Parser->yylval.m_pSymbol.eType|=2;
		Ret=SLParser::SYMBOL;
	}

	return(Ret);
}

