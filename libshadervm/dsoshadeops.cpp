// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory // // Contact: pgregory@aqsis.com //
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful, // but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

/** \file
		\brief Implements the classes and support structures for DSO shadeops
		\author Tristan Colgate <tristan@inuxtech.co.uk>
*/

#include	"aqsis.h"
#include	"sstring.h"
#include	"log.h"
#include	"irenderer.h"
#include	"ishaderdata.h"
#include	"dsoshadeops.h"
#include	<sstream>

START_NAMESPACE( Aqsis )

//---------------------------------------------------------------------
/** This does replicate effort from CqFile and at present doesnt handle NT either 
 ** There is a distinction in that we would like to handle directories here which CqFile doesnt
 */

CqString
CqDSORepository::strPrototype(CqString *strFuncName, SqDSOExternalCall *pExtCall)
{
	CqString strProt;
	EqVariableType type;
	
	type = pExtCall->return_type;
	m_itTypeNameMap = m_TypeNameMap.begin();
	while (m_itTypeNameMap != m_TypeNameMap.end() &&
	       (*m_itTypeNameMap).second != type)
	{
		m_itTypeNameMap++ ;
	};
	if(m_itTypeNameMap != m_TypeNameMap.end()){
		strProt = (*m_itTypeNameMap).first + " ";
	}else{
	  	strProt += "Unkown ";
	};

	strProt += *strFuncName + " ( ";
	
	std::list<EqVariableType>::iterator it = pExtCall->arg_types.begin();
	while( it != pExtCall->arg_types.end()){
		type = (*it);
		m_itTypeNameMap = m_TypeNameMap.begin();
		while (m_itTypeNameMap != m_TypeNameMap.end() &&
	       		(*m_itTypeNameMap).second != type)
		{
			m_itTypeNameMap++ ;
		};
		if(m_itTypeNameMap != m_TypeNameMap.end()){
			strProt += (*m_itTypeNameMap).first + " ";
		}else{
	  		strProt += "Unkown ";
		};

		it++ ;
	};
	
	strProt += ")";

	return strProt;
};

//---------------------------------------------------------------------
/** This does replicate effort from CqFile and at present doesnt handle NT either 
 ** There is a distinction in that we would like to handle directories here which CqFile doesnt
 */
void
CqDSORepository::SetDSOPath(const CqString* pPath)
{
	CqString::size_type iLeft = 0;
	CqString::size_type iRight = iLeft ;

	// Split the string up into the components of the path;
	while(iRight <= pPath->length())
	{
	  	if ( ( *pPath)[iRight] == ';' || ( ( *pPath)[iRight] == ':' && ( iRight - iLeft ) > 1) )
		{
	  		CqString *element = new CqString(pPath->substr(iLeft, iRight - iLeft));
			m_pDSOPathList.push_back(element);
			iLeft = iRight + 1  ; 
		} else if ( iRight+1 > pPath->length() && iLeft != iRight) 
		{
			CqString *element = new CqString(pPath->substr(iLeft,iRight - iLeft));
			// Here, if element points to a directory, we can add each library in the
			// named directory which is not already in the path list 
			m_pDSOPathList.push_back(element);
		} ;
		iRight ++ ;
	};
};


//---------------------------------------------------------------------
/**  This returns a list of descriptors of calls to external DSO functions that
 * implement a named shadeop, entries are returned for each polymorphic function
 */
std::list<SqDSOExternalCall*>*
CqDSORepository::getShadeOpMethods(CqString* pShadeOpName)
{
	IqLog *logger = QGetRenderContextI()->Logger();
	CqString strTableSymbol = *pShadeOpName + "_shadeops" ;
	std::list<SqDSOExternalCall*>* oplist = new (std::list<SqDSOExternalCall*>);
	std::list<CqString*>::iterator itPathEntry;
	SqShadeOp *pTableSymbol = NULL;


	for ( itPathEntry = m_pDSOPathList.begin() ; itPathEntry != m_pDSOPathList.end() ; itPathEntry++ )
	{
		void *handle = DLOpen( (*itPathEntry) );

		if( handle != NULL )
		{
			pTableSymbol = (SqShadeOp*) DLSym( handle, &strTableSymbol );

			if ( pTableSymbol != NULL )
			{
				//We have an appropriate named shadeop table
				SqShadeOp *pShadeOp = (SqShadeOp*) pTableSymbol;
				while( ( pShadeOp->m_opspec )[0] != (char) NULL )
				{
				  	SqDSOExternalCall *pDSOCall = NULL;
					pDSOCall = parseShadeOpTableEntry( handle, pShadeOp );
					if ( pDSOCall != NULL )
						oplist->push_back( pDSOCall );	

					pShadeOp++;
				};
			};

			// Failure here does not neccesarily mean anything since 
		} 
		else 
		{
			CqString strError = DLError();
			logger->error("DLOpen: %s\n" , strError.c_str() );
		};
	};
	return ( oplist->empty() ? NULL : oplist );
};

//---------------------------------------------------------------------
/** We have found a plugins with the desried shadeop table in it
 * For each shaeop table entry we parse the prototype and validate all
 * symbols that are named by the shadeop entry.
 */

SqDSOExternalCall*
CqDSORepository::parseShadeOpTableEntry(void* handle, SqShadeOp* pShadeOpEntry){
	
  	IqLog *logger = QGetRenderContextI()->Logger();
	TqInt length = strlen(pShadeOpEntry->m_opspec)+1;
	char *temp = new (char [length]);
	strncpy(temp, pShadeOpEntry->m_opspec,length);

	// We remove all the '(),' charachters, so we are left with 
	//   returntype name argtype ...
	for (int x = 0; x < length; x++)
		if(temp[x]=='('||temp[x]==')'||temp[x]==',')temp[x]=' ';

	CqString strSpec(temp);
	std::istringstream strOpSpec(strSpec, std::ios::in);

	// Get the return type of the function
	std::stringbuf strRetType;
	strOpSpec.get(strRetType,' ') ;
	m_itTypeNameMap = m_TypeNameMap.find(strRetType.str());
	// ERROR if we cant find this types name;
	if (m_itTypeNameMap == m_TypeNameMap.end())
	{
	  	logger->warn( "Discarding DSO Table entry due to unsupported return type: %s\n" , strRetType.str().c_str() );
		return NULL;
	}
	EqVariableType rettype = (*m_itTypeNameMap).second;
	strOpSpec >> std::ws;

	// Get function name
	std::stringbuf strMethodName;
	strOpSpec.get(strMethodName,' ') ;
	CqString s = strMethodName.str();
	DSOMethod method = (DSOMethod) DLSym (handle,&s);
	if(method == NULL) 
	{
	  	logger->warn( "Discarding DSO Table entry due to unknown symbol for method: %s\n" , strMethodName.str().c_str() );
	 	return NULL;
	};
	strOpSpec >> std::ws;
	
	// Parse each arg type, presumably we need to handle arrays here
	std::list<EqVariableType> arglist;
	while(!strOpSpec.eof())
	{
		// Get the next arguments type 
		std::stringbuf strArgType;
		strOpSpec.get(strArgType,' ') ;
		m_itTypeNameMap = m_TypeNameMap.find(strArgType.str());
		// ERROR if we cant find this arguments type name;
		if (m_itTypeNameMap == m_TypeNameMap.end())
		{
	  		logger->warn( "Discarding DSO Table entry due to unsupported argumetn type: %s\n", strArgType.str().c_str());
			return NULL;
		}; 
		arglist.push_back((*m_itTypeNameMap).second);
		strOpSpec >> std::ws;
	};

	// Check if there is a valid init function
	CqString strInit(pShadeOpEntry->m_init);
	DSOInit initfunc = NULL;
	if (strcmp(pShadeOpEntry->m_init,""))
	{
		initfunc = (DSOInit) DLSym(handle,&strInit);
		if (initfunc == NULL)
		{
	  		logger->warn( "Discarding DSO Table entry dut to unknown symbol for init: %s\n" , strInit.c_str()); 
			return NULL; // ERROR ;
		};
	} 
	
	// Check if there is a valid shutdown function
	CqString strShutdown(pShadeOpEntry->m_shutdown);
	DSOShutdown shutdownfunc = NULL;
	if (strcmp(pShadeOpEntry->m_shutdown,""))
	{
		shutdownfunc = (DSOShutdown) DLSym(handle,&strShutdown);
		if (shutdownfunc == NULL)
		{
 			logger->warn( "Discarding DSO Table entry dut to unknown symbol for shutdown: %s\n" , strShutdown.c_str()); 
			return NULL; // ERROR ;
		};
	};


	// We have a valid shadeop implementation
	SqDSOExternalCall *ret = new SqDSOExternalCall;
	ret->method = method;
	ret->init = initfunc;
	ret->shutdown = shutdownfunc;
	ret->return_type = rettype; 
	ret->arg_types = arglist;
	ret->initData = NULL;
	ret->initialised = false;

	return ret;
};


END_NAMESPACE( Aqsis )
//---------------------------------------------------------------------
