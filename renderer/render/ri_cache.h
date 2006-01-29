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
		\brief Declares the classes used for caching RI calls for later retrieval.
		\author Paul Gregory (pgregory@aqsis.com)
*/

//? Is .h included already?
#ifndef RI_CACHE_H_INCLUDED
#define RI_CACHE_H_INCLUDED 1

#include	"aqsis.h"
#include	"symbols.h"
#include	"ri.h"
#include	"renderer.h"

START_NAMESPACE( Aqsis )


class RiCacheBase
{
	public:
		RiCacheBase()	:	m_count(0), m_tokens(0), m_values(0)
		{}
		virtual ~RiCacheBase()
		{
			// Delete the plist
			int i;
			for(i=0; i<m_count; i++)
			{
				SqParameterDeclaration Decl = QGetRenderContext()->FindParameterDecl( m_tokens[i] );
				if(Decl.m_Type == type_string)
				{
					int size = 1;
					switch( Decl.m_Class )
					{
							case class_constant:
							size = m_constant_size;
							break;

							case class_uniform:
							size = m_uniform_size;
							break;

							case class_varying:
							size = m_varying_size;
							break;

							case class_vertex:
							size = m_vertex_size;
							break;

							case class_facevarying:
							size = m_facevarying_size;
							break;
					}
					int j;
					for(j=0; j<size; j++)
						delete[](reinterpret_cast<RtString*>(m_values[i])[j]);
				}
				delete[](m_tokens[i]);
				delete[](m_values[i]);
			}

			delete[] m_tokens;
			delete[] m_values;
		}

		virtual void ReCall()=0;

	protected:
		virtual	void	CachePlist(RtInt count, RtToken tokens[], RtPointer values[],
		                        int constant_size, int uniform_size, int varying_size, int vertex_size, int facevarying_size )
		{
			// Cache the sizes as we need them during destruction.
			m_constant_size = constant_size;
			m_uniform_size = uniform_size;
			m_varying_size = varying_size;
			m_vertex_size = vertex_size;
			m_facevarying_size = facevarying_size;

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

				SqParameterDeclaration Decl = QGetRenderContext()->FindParameterDecl( token );

				// Work out the amount of data to copy determined by the
				// class, the type and the array size.
				int size = 1;
				switch( Decl.m_Class )
				{
						case class_constant:
						size = constant_size;
						break;

						case class_uniform:
						size = uniform_size;
						break;

						case class_varying:
						size = varying_size;
						break;

						case class_vertex:
						size = vertex_size;
						break;

						case class_facevarying:
						size = facevarying_size;
						break;
				}

				// If it is a compound type, increase the length by the number of elements.
				if( Decl.m_Type == type_point ||
				        Decl.m_Type == type_normal ||
				        Decl.m_Type == type_color ||
				        Decl.m_Type == type_vector)
					size *= 3;
				else if( Decl.m_Type == type_hpoint)
					size *= 4;
				else if( Decl.m_Type == type_matrix)
					size *= 16;

				// If it is an array, increase the size by the number of elements in the array.
				size *= Decl.m_Count;

				int j;
				switch( Decl.m_Type )
				{
						case type_integer:
						m_values[i] = CopyAtomicValue(size, reinterpret_cast<RtInt*>(values[i]));
						break;

						case type_point:
						case type_color:
						case type_normal:
						case type_vector:
						case type_hpoint:
						case type_matrix:
						case type_float:
						m_values[i] = CopyAtomicValue(size, reinterpret_cast<RtFloat*>(values[i]));
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
		int			m_constant_size;
		int			m_uniform_size;
		int			m_varying_size;
		int			m_vertex_size;
		int			m_facevarying_size;
};


#include	"ri_cache.inl"


//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

#endif // RI_CACHE_H_INCLUDED
