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
		\brief Implements the entrypoint for the shader compiler.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include	"aqsis.h"

#ifdef	AQSIS_SYSTEM_WIN32
	#include	<windows.h>
#endif // AQSIS_SYSTEM_WIN32

#include	<iostream>
#include	<fstream>

#include	"parsenode.h"
#include	"ri.h"
#include	"Parser.h"
#include	"scanner.h"
#include	"shadervariable.h"
#include	"version.h"

class SLCompiler : public SLParser
{
	public:
			SLCompiler(std::istream& file, char* pName) :	
									m_Scanner(&file),
									m_strName(pName),
									m_Line(1)
				{
					m_Scanner.m_Parser=this;
				};

	virtual int yylex();
	virtual void yyerror(char *m);
	virtual	void yydebugout(char *m);
	
	virtual	const char* FileName() const		{return(m_strName.c_str());}
	virtual	void SetFileName(const char* strName)	{m_strName=strName;}
	virtual void SetLineNo(TqInt i)				{m_Line=i;}
	virtual TqInt LineNo() const				{return(m_Line);}
			
	private:
			SLScanner	m_Scanner;
			CqString	m_strName;
			TqInt		m_Line;
};

int SLCompiler::yylex()
{
	yylloc.first_line=m_Line;
	int token=m_Scanner.yylex(&yylval,&yylloc);
	yylloc.last_line=m_Line;
	yylloc.text=(char *)m_Scanner.yytext;
	return(token);
}

void SLCompiler::yyerror(char *m)
{
	char String[100];
	// C:\Projects\Renderer\slcomp\slcomp.cpp(37) : error C2065: 'tchar' : undeclared identifier
	sprintf(String,"%s(%d) : error C0001: '%s' : %s\n",m_strName.c_str(),yylloc. first_line, yylloc.text, m);
	std::cout << String << std::flush;	
}

void SLCompiler::yydebugout(char *m)
{
	printf(m);	
}


int main(int argc, char* argv[])
{
	std::cout << "slcomp V" << VERSION_STR << std::endl;

	if(argc<2)
	{
		std::cout << "Usage: slcomp [options] file.sl" << std::endl;
		return(0);
	}
	char iName[256];

	CqString strOpts("-v -d PI=3.1412 -c 6 ");	// Default options to slpp.

	int i;
	for(i=1; i<argc-1; i++)
	{		
		// TODO: Parse any options we recognise here when there are any slcomp options.
		strOpts+=argv[i];
		strOpts+=" ";
	}
	strcpy(iName,argv[i]);

	std::cout << "Compiling : " << iName << std::endl;

	// Call slpp to preprocess the source file.
	PROCESS_INFORMATION ProcInf;
	STARTUPINFO	StartInf;
	GetStartupInfo(&StartInf);
	CqString strCmd("slpp ");
	strCmd+=strOpts;
	strCmd+="\"";
	strCmd+=iName;
	strCmd+="\" ";
	strcat(iName,"p");
	strCmd+="-o \"";
	strCmd+=iName;
	strCmd+="\"";
	//printf(strCmd.c_str());
	BOOL res=CreateProcess(NULL, (char*)strCmd.c_str(), NULL, NULL, false, 0, NULL, NULL, &StartInf, &ProcInf);
	// Wait for slpp to finish
	DWORD ExitCode;
	while(GetExitCodeProcess(ProcInf.hProcess, &ExitCode) && ExitCode==STILL_ACTIVE);

	if(ExitCode)
	{
		std::cout << "slpp Error" << std::endl;
		return(ExitCode);
	}

#ifdef	_DEBUG
	FILE *stream = freopen( "slparse.out", "w", stderr );
#endif

	std::ifstream ifile(iName, std::ios_base::in);
	SLCompiler theCompiler(ifile,iName);
#ifdef _DEBUG
	theCompiler.yydebug=1;
#endif
	int result=theCompiler.yyparse();

	if(result==0)
	{
		char oName[256];
		strcpy(oName,theCompiler.Shader().strName());
		strcat(oName,RI_SHADER_EXTENSION);
		std::ofstream ofile(oName);

#ifdef _DEBUG
		std::ofstream dofile("tree.out");
		theCompiler.pParseTree()->OutTree(dofile);				
#endif
		theCompiler.TypeCheck();
 		theCompiler.Optimise();
		theCompiler.Output(ofile);
	}

#ifdef	_DEBUG	
	fclose(stream);
#endif

	// Delete the temporary preprocessed source file
	ifile.close();
	res=DeleteFile(iName);

	return(0);
}

