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

#include <sys/stat.h>

#include	"aqsis.h"
#include	"sstring.h"
#include	"irenderer.h"
#include	"ishaderdata.h"
#include	"dsoshadeops.h"
#include	"file.h"
#include	"logging.h"

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
	if(m_itTypeNameMap != m_TypeNameMap.end())
	{
		strProt = (*m_itTypeNameMap).first + " ";
	}
	else
	{
		strProt += "Unkown ";
	};

	strProt += *strFuncName + " ( ";

	std::list<EqVariableType>::iterator it = pExtCall->arg_types.begin();
	while( it != pExtCall->arg_types.end())
	{
		type = (*it);
		m_itTypeNameMap = m_TypeNameMap.begin();
		while (m_itTypeNameMap != m_TypeNameMap.end() &&
		        (*m_itTypeNameMap).second != type)
		{
			m_itTypeNameMap++ ;
		};
		if(m_itTypeNameMap != m_TypeNameMap.end())
		{
			strProt += (*m_itTypeNameMap).first + " ";
		}
		else
		{
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
CqDSORepository::SetDSOPath(const char* pathStr)
{
	if ( pathStr == NULL )
		return;

	CqString Path(pathStr);
	CqString::size_type iLeft = 0;
	CqString::size_type iRight = iLeft ;

	// Split the string up into the components of the path;
	while(iRight <= Path.length())
	{
		if (    Path[iRight] == ';' ||  // completed a path element with ';'
		        ( Path[iRight] == ':' && ( iRight - iLeft ) > 1) || // completed a path element ':'
		        ( iRight+1 > Path.length() && iLeft != iRight) ) // hit end of list
		{
			CqString *element = new CqString(Path.substr(iLeft,iRight - iLeft));
			// Here, if element points to a directory, we can add each library in the
			// named directory which is not already in the path list

			struct stat s;
			if (!stat( element->c_str(), &s ))
			{
				if ( S_ISDIR(s.st_mode) )
				{
					// We have a directory, list all the libraries in that directory and add them to the path
					CqString wild = *element + CqString( DIRSEP ) + CqString( "*" ) + CqString ( SHARED_LIBRARY_SUFFIX );
					std::list<CqString*> files = Aqsis::CqFile::Glob(wild);
					if ( !files.empty() )
					{
						m_pDSOPathList.splice(m_pDSOPathList.end(), files);
					}
					delete(element);
				}
				else
				{
					m_pDSOPathList.push_back(element);
				}
			}
			else
				delete(element);
		}

		if (    Path[iRight] == ';' ||  // completed a path element with ';'
		        ( Path[iRight] == ':' && ( iRight - iLeft ) > 1) ) // completed a path element ':'
			iLeft = iRight + 1  ;
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
	CqString strTableSymbol = *pShadeOpName + "_shadeops" ;

	std::list<SqDSOExternalCall*>* oplist = new (std::list<SqDSOExternalCall*>);
	std::list<CqString*>::iterator itPathEntry;
	SqShadeOp *pTableSymbol = NULL;

	Aqsis::log() << debug << "Looking for DSO candidates for shadeop \"" << pShadeOpName->c_str() << "\"" << std::endl;
	for ( itPathEntry = m_pDSOPathList.begin() ; itPathEntry != m_pDSOPathList.end() ; itPathEntry++ )
	{
		Aqsis::log() << debug << "Looking in shared library : " << (*itPathEntry)->c_str() << std::endl;
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
			Aqsis::log() << error << "DLOpen: " << strError.c_str() << std::endl;
		};
	};
	std::stringstream resultStr;
	if(oplist->empty())
		resultStr << "(none found)";
	else
		resultStr << "(found " << oplist->size() << " possibilities)";
	Aqsis::log() << debug << "Finished looking for DSO candidates "<< resultStr.str().c_str() << std::endl;
	return ( oplist->empty() ? NULL : oplist );
};

//---------------------------------------------------------------------
/** We have found a plugins with the desried shadeop table in it
 * For each shaeop table entry we parse the prototype and validate all
 * symbols that are named by the shadeop entry.
 */

SqDSOExternalCall*
CqDSORepository::parseShadeOpTableEntry(void* handle, SqShadeOp* pShadeOpEntry)
{

	TqInt length = strlen(pShadeOpEntry->m_opspec)+1;
	char temp[1024];
	strncpy(temp, pShadeOpEntry->m_opspec,length);

	// We remove all the '(),' charachters, so we are left with
	//   returntype name argtype ...
	for (int x = 0; x < length; x++)
		if(temp[x]=='('||temp[x]==')'||temp[x]==',')
			temp[x]=' ';

	CqString strSpec(temp);

	// Get the return type of the function
	std::string strRetType;


	strRetType = strtok(temp, " ");
	m_itTypeNameMap = m_TypeNameMap.find(strRetType.c_str());

	// ERROR if we cant find this types name;
	if (m_itTypeNameMap == m_TypeNameMap.end())
	{
		Aqsis::log() << warning << "Discarding DSO Table entry due to unsupported return type: \"" << strRetType.c_str() << "\"" << std::endl;
		return NULL;
	}
	EqVariableType rettype = (*m_itTypeNameMap).second;


	// Get function name
	std::string strMethodName;

	strMethodName = strtok(NULL, " ");
	CqString s = strMethodName.c_str();
	DSOMethod method = (DSOMethod) DLSym (handle,&s);
	if(method == NULL)
	{
		Aqsis::log() << warning << "Discarding DSO Table entry due to unknown symbol for method: \"" << strMethodName.c_str() << "\"" << std::endl;
		return NULL;
	};


	// Parse each arg type, presumably we need to handle arrays here
	std::list<EqVariableType> arglist;
	char *nextarg = NULL;
	do
	{
		// Get the next arguments type
		std::string strArgType;

		nextarg = strtok(NULL, " ");
		if (nextarg == NULL)
			break;
		strArgType = nextarg;
		m_itTypeNameMap = m_TypeNameMap.find(strArgType.c_str());

		// ERROR if we cant find this arguments type name;
		if (m_itTypeNameMap == m_TypeNameMap.end())
		{
			Aqsis::log() << warning << "Discarding DSO Table entry due to unsupported argument type: \"" << strArgType.c_str() << "\"" << std::endl;
			return NULL;
		};
		arglist.push_back((*m_itTypeNameMap).second);

	}
	while(nextarg);

	// Check if there is a valid init function
	CqString strInit = pShadeOpEntry->m_init;
	DSOInit initfunc = NULL;
	if (strcmp(pShadeOpEntry->m_init,""))
	{
		initfunc = (DSOInit) DLSym(handle,&strInit);
		if (initfunc == NULL)
		{
			Aqsis::log() << warning << "Discarding DSO Table entry due to unknown symbol for init: \"" << strInit.c_str() << "\"" << std::endl;
			return NULL; // ERROR ;
		};
	}

	// Check if there is a valid shutdown function
	CqString strShutdown = pShadeOpEntry->m_shutdown;
	DSOShutdown shutdownfunc = NULL;
	if (strcmp(pShadeOpEntry->m_shutdown,""))
	{
		shutdownfunc = (DSOShutdown) DLSym(handle,&strShutdown);
		if (shutdownfunc == NULL)
		{
			Aqsis::log() << warning << "Discarding DSO Table entry due to unknown symbol for shutdown: \"" << strShutdown.c_str() << "\"" << std::endl;
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
