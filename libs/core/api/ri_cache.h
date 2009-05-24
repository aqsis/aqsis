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
#ifndef RI_CACHE_H_INCLUDED
#define RI_CACHE_H_INCLUDED 1

#include <aqsis/aqsis.h>

#include <cstring>

#include <aqsis/ri/ri.h>
#include "renderer.h"
#include <aqsis/util/logging.h>

namespace Aqsis {


class RiCacheBase
{
	public:
		RiCacheBase()	:	m_count(0), m_tokens(0), m_values(0), m_interpClassCounts()
		{}
		virtual ~RiCacheBase()
		{
			// Delete the plist
			int i;
			for(i=0; i<m_count; i++)
			{
				CqPrimvarToken tok = QGetRenderContext()->
					tokenDict().parseAndLookup(m_tokens[i]);
				// Delete i'th value based on type.
				RtPointer value = m_values[i];
				switch(tok.storageType())
				{
					case type_integer:
						delete[] reinterpret_cast<RtInt*>(value);
						break;
					case type_float:
						delete[] reinterpret_cast<RtFloat*>(value);
						break;
					case type_string:
						{
							int size = tok.storageCount(m_interpClassCounts);
							RtString* strArray = reinterpret_cast<RtString*>(value);
							for(int j=0; j<size; j++)
								delete[] strArray[j];
						}
						delete[] reinterpret_cast<RtString*>(m_values[i]);
						break;
					default:
						break;
				}
				// Delete i'th token
				delete[] m_tokens[i];
			}

			// Delete token and value arrays.
			delete[] m_tokens;
			delete[] m_values;
		}

		virtual void ReCall()=0;

	protected:
		virtual	void	CachePlist(RtInt count, RtToken tokens[], RtPointer values[],
				const SqInterpClassCounts& interpClassCounts)
		{
			m_interpClassCounts = interpClassCounts;
			m_count = count;
			m_tokens = new RtToken[count];
			m_values = new RtPointer[count];

			RtInt i;
			for(i=0; i<count; i++)
			{
				RtToken	token = tokens[ i ];
				RtPointer	value = values[ i ];

				RtToken newtoken = new char[strlen(token) + 1];
				strcpy(newtoken, token);
				m_tokens[i] = newtoken;

				CqPrimvarToken tok;
				try
				{
					tok = QGetRenderContext()->tokenDict().parseAndLookup(token);
				}
				catch(XqValidation& e)
				{
					Aqsis::log() << error << e.what() << "\n";
				}

				int size = tok.storageCount(m_interpClassCounts);

				int j;
				switch( tok.storageType() )
				{
					case type_integer:
						m_values[i] = CopyAtomicValue(size,
								reinterpret_cast<RtInt*>(values[i]));
						break;
					case type_float:
						m_values[i] = CopyAtomicValue(size,
								reinterpret_cast<RtFloat*>(values[i]));
						break;
					case type_string:
						{
							RtString* copyvalue = new RtString[size];
							for(j=0; j<size; j++)
							{
								RtString item = new char[strlen(reinterpret_cast<RtString*>(value)[j])];
								strcpy(item, reinterpret_cast<RtString*>(value)[j]);
								copyvalue[j] = item;
							}
							m_values[i] = reinterpret_cast<RtPointer>(copyvalue);
						}
						break;
					default:
						m_values[i] = 0;
						break;
				}
			}
		}

		template <class T>
		RtPointer CopyAtomicValue(RtInt size, T* value)
		{
			T* copyvalue = new T[size];
			int j;
			for(j=0; j<size; j++)
				copyvalue[j] = value[j];
			return((RtPointer)copyvalue);
		}

		RtInt		m_count;
		RtToken*	m_tokens;
		RtPointer*	m_values;
		SqInterpClassCounts m_interpClassCounts;
};


#include	"ri_cache.inl"


//-----------------------------------------------------------------------

} // namespace Aqsis

#endif // RI_CACHE_H_INCLUDED

