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
		\brief Implements the classes and support structures for the shader VM stack.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#include	<aqsis/aqsis.h>
#include	"shaderstack.h"
#include	<aqsis/shadervm/ishaderdata.h>

#undef SHADERSTACKSTATS /* define if you want to know at run-time the max. depth of stack */


namespace Aqsis {

TqUint   CqShaderStack::m_samples = 18;
TqUint   CqShaderStack::m_maxsamples = 18;

std::deque<CqShaderVariableUniformFloat*>			CqShaderStack::m_UFPool;
std::deque<CqShaderVariableUniformPoint*>			CqShaderStack::m_UPPool;
std::deque<CqShaderVariableUniformString*>			CqShaderStack::m_USPool;
std::deque<CqShaderVariableUniformColor*>			CqShaderStack::m_UCPool;
std::deque<CqShaderVariableUniformNormal*>			CqShaderStack::m_UNPool;
std::deque<CqShaderVariableUniformVector*>			CqShaderStack::m_UVPool;
std::deque<CqShaderVariableUniformMatrix*>			CqShaderStack::m_UMPool;

std::deque<CqShaderVariableVaryingFloat*>			CqShaderStack::m_VFPool;
std::deque<CqShaderVariableVaryingPoint*>			CqShaderStack::m_VPPool;
std::deque<CqShaderVariableVaryingString*>			CqShaderStack::m_VSPool;
std::deque<CqShaderVariableVaryingColor*>			CqShaderStack::m_VCPool;
std::deque<CqShaderVariableVaryingNormal*>			CqShaderStack::m_VNPool;
std::deque<CqShaderVariableVaryingVector*>			CqShaderStack::m_VVPool;
std::deque<CqShaderVariableVaryingMatrix*>			CqShaderStack::m_VMPool;


//----------------------------------------------------------------------
/** Returns the next shaderstack variable and allocates more if
 *  it needs to be
 */

IqShaderData* CqShaderStack::GetNextTemp( EqVariableType type, EqVariableClass _class )
{
	switch ( type )
	{
			case type_float:
			{
				if ( _class == class_uniform )
				{
					if( m_UFPool.empty() )
						return( new CqShaderVariableUniformFloat() );
					else
					{
						IqShaderData* ret = m_UFPool.front();
						m_UFPool.pop_front();
						return( ret );
					}
				}
				else
				{
					if( m_VFPool.empty() )
						return( new CqShaderVariableVaryingFloat() );
					else
					{
						IqShaderData* ret = m_VFPool.front();
						m_VFPool.pop_front();
						return( ret );
					}
				}
			}

			case type_point:
			{
				if ( _class == class_uniform )
				{
					if( m_UPPool.empty() )
						return( new CqShaderVariableUniformPoint() );
					else
					{
						IqShaderData* ret = m_UPPool.front();
						m_UPPool.pop_front();
						return( ret );
					}
				}
				else
				{
					if( m_VPPool.empty() )
						return( new CqShaderVariableVaryingPoint() );
					else
					{
						IqShaderData* ret = m_VPPool.front();
						m_VPPool.pop_front();
						return( ret );
					}
				}
			}

			case type_string:
			{
				if ( _class == class_uniform )
				{
					if( m_USPool.empty() )
						return( new CqShaderVariableUniformString() );
					else
					{
						IqShaderData* ret = m_USPool.front();
						m_USPool.pop_front();
						return( ret );
					}
				}
				else
				{
					if( m_VSPool.empty() )
						return( new CqShaderVariableVaryingString() );
					else
					{
						IqShaderData* ret = m_VSPool.front();
						m_VSPool.pop_front();
						return( ret );
					}
				}
			}

			case type_color:
			{
				if ( _class == class_uniform )
				{
					if( m_UCPool.empty() )
						return( new CqShaderVariableUniformColor() );
					else
					{
						IqShaderData* ret = m_UCPool.front();
						m_UCPool.pop_front();
						return( ret );
					}
				}
				else
				{
					if( m_VCPool.empty() )
						return( new CqShaderVariableVaryingColor() );
					else
					{
						IqShaderData* ret = m_VCPool.front();
						m_VCPool.pop_front();
						return( ret );
					}
				}
			}

			case type_normal:
			{
				if ( _class == class_uniform )
				{
					if( m_UNPool.empty() )
						return( new CqShaderVariableUniformNormal() );
					else
					{
						IqShaderData* ret = m_UNPool.front();
						m_UNPool.pop_front();
						return( ret );
					}
				}
				else
				{
					if( m_VNPool.empty() )
						return( new CqShaderVariableVaryingNormal() );
					else
					{
						IqShaderData* ret = m_VNPool.front();
						m_VNPool.pop_front();
						return( ret );
					}
				}
			}

			case type_vector:
			{
				if ( _class == class_uniform )
				{
					if( m_UVPool.empty() )
						return( new CqShaderVariableUniformVector() );
					else
					{
						IqShaderData* ret = m_UVPool.front();
						m_UVPool.pop_front();
						return( ret );
					}
				}
				else
				{
					if( m_VVPool.empty() )
						return( new CqShaderVariableVaryingVector() );
					else
					{
						IqShaderData* ret = m_VVPool.front();
						m_VVPool.pop_front();
						return( ret );
					}
				}
			}

			case type_matrix:
			{
				if ( _class == class_uniform )
				{
					if( m_UMPool.empty() )
						return( new CqShaderVariableUniformMatrix() );
					else
					{
						IqShaderData* ret = m_UMPool.front();
						m_UMPool.pop_front();
						return( ret );
					}
				}
				else
				{
					if( m_VMPool.empty() )
						return( new CqShaderVariableVaryingMatrix() );
					else
					{
						IqShaderData* ret = m_VMPool.front();
						m_VMPool.pop_front();
						return( ret );
					}
				}
			}

			default:
				break;
	}
	assert( false );
	return( NULL );
}

//----------------------------------------------------------------------
/** Release the stack value passed in, if it is a temporary, return it to the bucket.
 * \param s Stack entry to be released.
 */
void CqShaderStack::Release( SqStackEntry s )
{
	if( s.m_IsTemp )
	{
		switch( s.m_Data->Type() )
		{
				case type_float:
				{
					if ( s.m_Data->Class() == class_uniform )
						m_UFPool.push_back(reinterpret_cast<CqShaderVariableUniformFloat*>(s.m_Data) );
					else
						m_VFPool.push_back(reinterpret_cast<CqShaderVariableVaryingFloat*>(s.m_Data) );
					break;
				}

				case type_point:
				{
					if ( s.m_Data->Class() == class_uniform )
						m_UPPool.push_back(reinterpret_cast<CqShaderVariableUniformPoint*>(s.m_Data) );
					else
						m_VPPool.push_back(reinterpret_cast<CqShaderVariableVaryingPoint*>(s.m_Data) );
					break;
				}

				case type_string:
				{
					if ( s.m_Data->Class() == class_uniform )
						m_USPool.push_back(reinterpret_cast<CqShaderVariableUniformString*>(s.m_Data) );
					else
						m_VSPool.push_back(reinterpret_cast<CqShaderVariableVaryingString*>(s.m_Data) );
					break;
				}

				case type_color:
				{
					if ( s.m_Data->Class() == class_uniform )
						m_UCPool.push_back(reinterpret_cast<CqShaderVariableUniformColor*>(s.m_Data) );
					else
						m_VCPool.push_back(reinterpret_cast<CqShaderVariableVaryingColor*>(s.m_Data) );
					break;
				}

				case type_normal:
				{
					if ( s.m_Data->Class() == class_uniform )
						m_UNPool.push_back(reinterpret_cast<CqShaderVariableUniformNormal*>(s.m_Data) );
					else
						m_VNPool.push_back(reinterpret_cast<CqShaderVariableVaryingNormal*>(s.m_Data) );
					break;
				}

				case type_vector:
				{
					if ( s.m_Data->Class() == class_uniform )
						m_UVPool.push_back(reinterpret_cast<CqShaderVariableUniformVector*>(s.m_Data) );
					else
						m_VVPool.push_back(reinterpret_cast<CqShaderVariableVaryingVector*>(s.m_Data) );
					break;
				}

				case type_matrix:
				{
					if ( s.m_Data->Class() == class_uniform )
						m_UMPool.push_back(reinterpret_cast<CqShaderVariableUniformMatrix*>(s.m_Data) );
					else
						m_VMPool.push_back(reinterpret_cast<CqShaderVariableVaryingMatrix*>(s.m_Data) );
					break;
				}
				
				default:
					break;
		}
	}
}


//----------------------------------------------------------------------
/** Prints the max. depth stack for now to stdout.
 *  if SHADERSTACKSTATS is defined
 * TODO: maybe later prints outs statistics about the running of the VM.
 */

void CqShaderStack::Statistics()
{
#ifdef SHADERSTACKSTATS
	static TqInt done = 0;
	if (!done)
	{
		std::cout << "The shaderstack's max. depth was " << m_maxsamples << std::endl;
		done = 1;
	}
#endif
}
} // namespace Aqsis
//---------------------------------------------------------------------
