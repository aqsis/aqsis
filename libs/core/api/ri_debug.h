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
		\brief Declares the functions used for debug printing RI interface calls.
		\author Paul Gregory (pgregory@aqsis.org)
*/

//? Is .h included already?
#ifndef RI_DEBUG_H_INCLUDED
#define RI_DEBUG_H_INCLUDED 1

#include	<aqsis/aqsis.h>
#include	"renderer.h"

namespace Aqsis {


void DebugPlist(RtInt count, RtToken tokens[], RtPointer values[],
		const SqInterpClassCounts& interpClassCounts, std::stringstream& _message)
{
	RtInt i;
	for(i=0; i<count; i++)
	{
		RtToken	token = tokens[ i ];
		RtPointer	value = values[ i ];

		CqPrimvarToken tok = QGetRenderContext()->tokenDict().parseAndLookup(token);

		int size = tok.storageCount(interpClassCounts);

		int j;
		_message << "\"" << token << "\" ["; 
		switch( tok.storageType() )
		{
			case type_integer:
				for(j=0; j<size; j++)
					_message << reinterpret_cast<RtInt*>(values[i])[j] << " ";
				break;
			case type_float:
				for(j=0; j<size; j++)
					_message << reinterpret_cast<RtFloat*>(values[i])[j] << " ";
				break;
			case type_string:
				{
					for(j=0; j<size; j++)
					{
						RtString item = new char[strlen(reinterpret_cast<RtString*>(value)[j])+1];
						strcpy(item, reinterpret_cast<RtString*>(value)[j]);
						_message << "\"" << item << "\" ";
						delete[](item);
					}
				}
				break;
			default:
				break;
		}
		_message << "] "; 
	}
}

#include	"ri_debug.inl"


//-----------------------------------------------------------------------

} // namespace Aqsis

#endif // RI_DEBUG_H_INCLUDED

