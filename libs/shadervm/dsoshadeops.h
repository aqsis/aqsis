// Aqsis
// Copyright (C) 1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.org
// // This library is free software; you can redistribute it and/or // modify it under the terms of the GNU General Public // License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.  // // This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


/** \file
		\brief  Classes and structures for supporting DSOs
		\author Tristan Colgate <tristan@inuxtech.co.uk>
*/

#ifndef DSOSHADEOPS_H
#define DSOSHADEOPS_H

#include <aqsis/aqsis.h>

#include <list>
#include <map>

#include "idsoshadeops.h"
#include <aqsis/shadervm/ishader.h>
#include <aqsis/riutil/primvartype.h>
#include <aqsis/util/plugins.h>
#include <aqsis/ri/shadeop.h>

namespace Aqsis {

//---------------------------------------------------------------------
/**   CqDSORepository: This class is responsible for searching for shader
 * operations in extrnal shared libraries. 
 */
class AQSIS_SHADERVM_SHARE CqDSORepository: public IqDSORepository, private CqPluginBase
{

	private:
		//
		//---------------------------------------------------------------------
		/** Mapping of type names and ID's that we can handle in DSO's. The type
		 * ID's are as presented from the SL compiler, the type names are those that 
		 * may be used by DSO shadeop table function prototypes.
		 */
		void BuildTypeMaps(void)
		{
			/// \todo Code Review - use stuff from primvartype.h instead of m_TypeNameMap 
			m_TypeNameMap["invalid"] = type_invalid;
			m_TypeIdMap['@'] = type_invalid;
			m_TypeNameMap["integer"] = type_integer;
			m_TypeIdMap['i'] = type_integer;
			m_TypeNameMap["float"] = type_float;
			m_TypeIdMap['f'] = type_float;
			m_TypeNameMap["point"] = type_point;
			m_TypeIdMap['p'] = type_point;
			m_TypeNameMap["string"] = type_string;
			m_TypeIdMap['s'] = type_string;
			m_TypeNameMap["color"] = type_color;
			m_TypeIdMap['c'] = type_color;
			m_TypeNameMap["triple"] = type_triple;
			m_TypeIdMap['t'] = type_triple;
			m_TypeNameMap["hpoint"] = type_hpoint;
			m_TypeIdMap['h'] = type_hpoint;
			m_TypeNameMap["normal"] = type_normal;
			m_TypeIdMap['n'] = type_normal;
			m_TypeNameMap["vector"] = type_vector;
			m_TypeIdMap['v'] = type_vector;
			m_TypeNameMap["void"] = type_void;
			m_TypeIdMap['x'] = type_void;
			m_TypeNameMap["matrix"] = type_matrix;
			m_TypeIdMap['m'] = type_matrix;
			m_TypeNameMap["hextuple"] = type_sixteentuple; // ??
			m_TypeIdMap['w'] = type_sixteentuple;
		};

	public:
		virtual void SetDSOPath(const char*);
		CqString strPrototype(CqString*, SqDSOExternalCall*);

		CqDSORepository(CqString *searchpath): CqPluginBase()
		{
			BuildTypeMaps();
			SetDSOPath(searchpath->c_str());
		};

		CqDSORepository(): CqPluginBase()
		{
			BuildTypeMaps();
			if(getenv("AQSIS_SHADER_PATH"))
			{
				char *dsopath = getenv("AQSIS_SHADER_PATH");
				SetDSOPath(dsopath);
			}
		};


		//---------------------------------------------------------------------
		/**   The Destructor handles the calling of shutdown on any previously
		 * initialised DSO shadeops.
		 */
		virtual ~CqDSORepository()
		{
			// We should call shutdown for all init'd functions
			while( ( m_itActiveDSOMap = m_ActiveDSOMap.begin() ) != m_ActiveDSOMap.end())
			{

				std::list<SqDSOExternalCall *> *list = (*m_itActiveDSOMap).second;

				while( !list->empty() )
				{
					SqDSOExternalCall *item = list->front();

					if(item->shutdown != NULL &&
					        item->initialised)
						item->shutdown(item->initData);

					delete item;
					list->pop_front();
				};
				delete list;
				m_ActiveDSOMap.erase(m_itActiveDSOMap);
			};
		};

		// Utility to provide a formatted function prototype
		CqString strPtototype(SqDSOExternalCall*);

	protected:
		//A maps and iterators for the Type name/id mappings
		std::map<CqString,EqVariableType> m_TypeNameMap ;
		std::map<CqString,EqVariableType>::iterator m_itTypeNameMap ;
		std::map<TqChar,EqVariableType> m_TypeIdMap ;
		std::map<TqChar,EqVariableType>::iterator m_itTypeIdMap ;

		// A list of files (possibly also directories in future) to be searched for shadeops
		std::list<CqString> m_DSOPathList;
		// This is a map of shadeop names to descriptors of shadeop implementations that we
		// have already found, its for efficiency and to allow us to track initiliased shadeops
		// to be shutdown later.
		std::map<CqString,std::list<SqDSOExternalCall*> *> m_ActiveDSOMap;
		std::map<CqString,std::list<SqDSOExternalCall*> *>::iterator m_itActiveDSOMap;

		// Get Array of DSO entry points for a specific shadeop
		std::list<SqDSOExternalCall*>* getShadeOpMethods(CqString*);

		// Parse a single DSO SHADEOP_TABLE rntry
		SqDSOExternalCall* parseShadeOpTableEntry(void*, SqShadeOp*);
};

} // namespace Aqsis

#endif	// DSOSHADEOPS_H
