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
		\brief Implements the classes and support structures for the shader VM stack.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include	"aqsis.h"
#include	"shaderstack.h"
#include	"shadervariable.h"

START_NAMESPACE(Aqsis)


CqVMStackEntry::CqVMStackEntry(TqInt size)
{
	if(size>1)	
		m_aValues.resize(512); 

	m_Size=size;
	m_pVarRef=0;
}


CqVMStackEntry&	 CqVMStackEntry::operator=(CqShaderVariable* pv)
{
	//pv->GetValue(*this);
	m_pVarRef=pv;
	return(*this);
}


TqInt CqVMStackEntry::Size() const
{
	if(m_pVarRef!=0)	return(m_pVarRef->Size());
	else				return(m_Size);
}


END_NAMESPACE(Aqsis)
//---------------------------------------------------------------------
