// cribber.cpp : Defines the entry point for the console application.
//

#include	<windows.h>

#include	<conio.h>
#include	<stdio.h>

#pragma warning (disable : 4100)

#include	"aqsis.h"
#include	"file.h"
#include	"ri.h"
#include	"arg.h"
#include	"share.h"
#include	"iribcompiler.h"
#include	"irenderer.h"

using namespace Aqsis;

void RenderFile(CqFile& file);
static void arg_filename(int argc, char**argv);

int g_nowait;
int g_cFiles=0;
int g_Verbose=0;


int main(int argc, char* argv[])
{
    if (arg_parse(argc, argv,
		"", "Usage: %s [options]", argv[0],
		"", ARG_SUBR(arg_filename), "file(s) to render",
		"-nowait", ARG_FLAG(&g_nowait), "don't wait for a keypress",
		"-endofframe %d", &g_Verbose, "equivalent to \"endofframe\" option",  
		0) < 0)
	{
		RiEnd();
		exit(1);
	}

	if(g_cFiles==0)	// If no files specified, take input from stdin.
	{
		CqFile file(&std::cin, "stdin");
		RenderFile(file);
	}

	// Wait for a keypress
	if(!g_nowait)
	{
		std::cout << "Press any key to continue..." << std::endl;
		while(!kbhit());
		getche();
	}

	return(0);
}


void RenderFile(CqFile& file)
{
	IsRIBCompiler* pRibber=IsRIBCompiler::Create();
	IsRenderer* pRenderer=IsRenderer::Create();

	// Store the current working directory for later use
	char strCurrWD[255];
	GetCurrentDirectory(255,strCurrWD);

	if(file.IsValid())
	{
		RiBegin("CRIBBER");

		// Read config file name out of the ini file.
		char strExe[255];
		char strDrive[10];
		char strPath[255];
		char strCFGFile[255];
		GetModuleFileName(NULL, strExe, 255);
		_splitpath(strExe,strDrive,strPath,NULL,NULL);
		_makepath(strCFGFile,strDrive,strPath,"ribber",".cfg");

		CqString strCFG(strCFGFile);

		CqFile cfgfile(strCFG.c_str());
		if(cfgfile.IsValid())
		{
			char tbuffer[256];
			tbuffer[0]='\0';
			bool ff=static_cast<std::istream*>(cfgfile)->eof();
			static_cast<std::istream*>(cfgfile)->getline(tbuffer,255);
			pRibber->SetFile(cfgfile);
			pRibber->Parse();
			cfgfile.Close();
		}
		else
		{
			CqString strErr("Config file not found");
			strErr+=strCFGFile;
			std::cerr << strErr.c_str() << std::endl;
		}
	
		char strDir[255];
		_splitpath(file.strRealName().c_str(),strDrive,strPath,NULL,NULL);
		_makepath(strDir, strDrive, strPath, NULL, NULL);
		SetCurrentDirectory(strDir);


		if(g_Verbose!=0)	RiOption("statistics", "endofframe", &g_Verbose, RI_NULL);

		pRibber->SetFile(file);

		pRibber->Parse();
		SetCurrentDirectory(strCurrWD);

		RiEnd();

	}
	pRibber->Destroy();
	pRenderer->Destroy();
}

static void arg_filename(int argc, char**argv)
{
    int i;
	g_cFiles++;

	for(i=0; i<argc; i++)
	{
	    CqString strFname(argv[i]);
		CqFile file;
		file.Open(strFname.c_str());
		RenderFile(file);
	}
}
