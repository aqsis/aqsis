// Aqsis
// Copyright (C) 1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.org
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
		\brief Declares the classes used for caching RI calls for later retrieval.
		\author Paul Gregory (pgregory@aqsis.org)
*/

//? Is .h included already?
#ifndef OBJECTINSTANCE_H_INCLUDED
#define OBJECTINSTANCE_INCLUDED 1

#include	<aqsis/aqsis.h>
#include	"renderer.h"
#include	<aqsis/ri/ri.h>
#include	"ri_cache.h"

#include	<vector>

namespace Aqsis {


class CqObjectInstance
{
	public:
		CqObjectInstance()
		{}
		~CqObjectInstance()
		{
			std::vector<RiCacheBase*>::iterator i;
			for(i=m_CachedCommands.begin(); i!=m_CachedCommands.end(); i++)
				delete((*i));
		}

		void AddCacheCommand(RiCacheBase* pCacheCommand)
		{
			m_CachedCommands.push_back(pCacheCommand);
		}

		void RecallInstance()
		{
			std::vector<RiCacheBase*>::iterator i;
			for(i=m_CachedCommands.begin(); i!=m_CachedCommands.end(); i++)
				(*i)->ReCall();
		}
	private:
		std::vector<RiCacheBase*>	m_CachedCommands;
};


//-----------------------------------------------------------------------

} // namespace Aqsis

#endif // OBJECTINSTANCE_H_INCLUDED
