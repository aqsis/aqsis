////---------------------------------------------------------------------
////    Class definition file:  IBUFFER.CPP
////    Associated header file: IBUFFER.H
////
////    Author:					Paul C. Gregory
////    Creation date:			29/03/99
////---------------------------------------------------------------------

#include	<stdio.h>

#include	"shadowmap.h"

BOOL APIENTRY DllMain( HINSTANCE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
   	return TRUE;
}

extern "C"
_qShareM Aqsis::CqImageBuffer* CreateImage(const char* strName)
{
	return(new Aqsis::CqShadowMap(strName));
}

START_NAMESPACE(Aqsis)

//---------------------------------------------------------------------

END_NAMESPACE(Aqsis)
