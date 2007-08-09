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
		\brief Declares the functions used for debug printing RI interface calls.
		\author Paul Gregory (pgregory@aqsis.org)
*/

//? Is .h included already?
#ifndef RI_DEBUG_H_INCLUDED
#define RI_DEBUG_H_INCLUDED 1

#include	"aqsis.h"
#include	"symbols.h"
#include	"renderer.h"

START_NAMESPACE( Aqsis )


void DebugPlist(RtInt count, RtToken tokens[], RtPointer values[],
			int constant_size, int uniform_size, int varying_size, int vertex_size, int facevarying_size, std::stringstream& _message)
{
	RtInt i;
	for(i=0; i<count; i++)
	{
		RtToken	token = tokens[ i ];
		RtPointer	value = values[ i ];

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

			default:
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
		_message << "["; 
		switch( Decl.m_Type )
		{
			case type_integer:
				for(j=0; j<size; j++)
					_message << reinterpret_cast<RtInt*>(values[i])[j];
				break;

			case type_point:
			case type_color:
			case type_normal:
			case type_vector:
			case type_hpoint:
			case type_matrix:
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

END_NAMESPACE( Aqsis )

#endif // RI_DEBUG_H_INCLUDED

