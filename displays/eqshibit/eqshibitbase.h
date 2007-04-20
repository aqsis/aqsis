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
		\brief Implements the default display devices for Aqsis.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#ifndef	___eqshibitbase_Loaded___
#define	___eqshibitbase_Loaded___

#include <aqsis.h>

#include <map>
#include <string>
#include <list>

START_NAMESPACE( Aqsis )

typedef	std::list<std::string> TqEqshibitCatalog;

class CqEqshibitBase
{
public:
	CqEqshibitBase()		{}
	virtual ~CqEqshibitBase()	{}

	virtual void	addNewCatalog(std::string name);
	void	setCurrentCatalog(std::string name);
	TqEqshibitCatalog& currentCatalog();
	std::string currentCatalogName();
	virtual void	addImageToCurrentCatalog(std::string name);

private:
	std::map<std::string, TqEqshibitCatalog>	m_catalogs;
	std::string	m_currentCatalogName;
};


//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

#endif	//	___display_Loaded___
