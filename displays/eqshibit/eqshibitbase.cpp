// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
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
		\brief Implements the common eqshibit functionality.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#include "eqshibitbase.h"
#include <sstream>
#include <iostream>


START_NAMESPACE( Aqsis )


void CqEqshibitBase::addNewCatalog(std::string name)
{
	m_catalogs[name] = TqEqshibitCatalog();
	m_currentCatalogName = name;
}

void CqEqshibitBase::setCurrentCatalog(std::string name)
{
	if(m_catalogs.find(name) != m_catalogs.end())
		m_currentCatalogName = name;
}


TqEqshibitCatalog& CqEqshibitBase::currentCatalog()
{
	return(m_catalogs.find(m_currentCatalogName)->second);
}


std::string CqEqshibitBase::currentCatalogName()
{
	return( m_currentCatalogName );
}

void CqEqshibitBase::addImageToCurrentCatalog(std::string name)
{
	if(m_currentCatalogName.empty())
	{
		std::cout << "Adding image" << std::endl;
		std::map<std::string, TqEqshibitCatalog>::size_type numCatalogs = m_catalogs.size();
		std::stringstream strCatName;
		strCatName << "Catalog" << numCatalogs+1;
		addNewCatalog(strCatName.str());
	}
	currentCatalog().push_back(name);	
}

//---------------------------------------------------------------------

END_NAMESPACE( Aqsis )
