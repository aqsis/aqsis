// cribber.cpp : Defines the entry point for the console application.
//

#include	<iostream>
#include	<fstream>

#include	<windows.h>

#include	<conio.h>
#include	<stdio.h>

#pragma warning (disable : 4100)

#include	"ri.h"
#include	"arg.h"
#include	"iribcompiler.h"

void RenderFile(std::istream& file, const char* name);
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
		RenderFile(std::cin, "stdin");
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


void RenderFile(std::istream& file, const char* name)
{
	IqRIBCompiler* pRibber=IqRIBCompiler::Create();

	// Store the current working directory for later use
	char strCurrWD[255];
	GetCurrentDirectory(255,strCurrWD);

	RiBegin("CRIBBER");

	// Read config file name out of the ini file.
	char strExe[255];
	char strDrive[10];
	char strPath[255];
	char strCFGFile[255];
	GetModuleFileName(NULL, strExe, 255);
	_splitpath(strExe,strDrive,strPath,NULL,NULL);
	_makepath(strCFGFile,strDrive,strPath,"ribber",".cfg");

	std::ifstream cfgfile(strCFGFile);
	if(cfgfile.is_open())
	{
		pRibber->SetFile(cfgfile, "config gile");
		pRibber->Parse();
		cfgfile.close();
	}
	else
		std::cerr << "Config file not found " << strCFGFile << std::endl;

	char strDir[255];
	_splitpath(name,strDrive,strPath,NULL,NULL);
	_makepath(strDir, strDrive, strPath, NULL, NULL);
	SetCurrentDirectory(strDir);

	RiOption("statistics", "endofframe", &g_Verbose, RI_NULL);
	pRibber->SetFile(file,name);
	pRibber->Parse();

	SetCurrentDirectory(strCurrWD);

	RiEnd();

	pRibber->Destroy();
}

static void arg_filename(int argc, char**argv)
{
    int i;
	g_cFiles++;

	for(i=0; i<argc; i++)
	{
		std::ifstream file(argv[i]);
		RenderFile(file,argv[i]);
	}
}
