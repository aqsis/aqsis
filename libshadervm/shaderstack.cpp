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
#include	"ishaderdata.h"

#undef SHADERSTACKSTATS /* define if you want to know at run-time the max. depth of stack */


START_NAMESPACE( Aqsis )

std::deque<CqShaderVariableUniformFloat>	CqShaderStack::m_aUFPool;
// Integer
std::deque<CqShaderVariableUniformPoint>	CqShaderStack::m_aUPPool;
std::deque<CqShaderVariableUniformString>	CqShaderStack::m_aUSPool;
std::deque<CqShaderVariableUniformColor>	CqShaderStack::m_aUCPool;
// Triple
// hPoint
std::deque<CqShaderVariableUniformNormal>	CqShaderStack::m_aUNPool;
std::deque<CqShaderVariableUniformVector>	CqShaderStack::m_aUVPool;
// Void
std::deque<CqShaderVariableUniformMatrix>	CqShaderStack::m_aUMPool;
// SixteenTuple

std::deque<CqShaderVariableVaryingFloat>	CqShaderStack::m_aVFPool;
// Integer
std::deque<CqShaderVariableVaryingPoint>	CqShaderStack::m_aVPPool;
std::deque<CqShaderVariableVaryingString>	CqShaderStack::m_aVSPool;
std::deque<CqShaderVariableVaryingColor>	CqShaderStack::m_aVCPool;
// Triple
// hPoint
std::deque<CqShaderVariableVaryingNormal>	CqShaderStack::m_aVNPool;
std::deque<CqShaderVariableVaryingVector>	CqShaderStack::m_aVVPool;
// Void
std::deque<CqShaderVariableVaryingMatrix>	CqShaderStack::m_aVMPool;

TqInt	CqShaderStack::m_iUPoolTops[ type_last ];
TqInt	CqShaderStack::m_iVPoolTops[ type_last ];
TqInt   CqShaderStack::m_samples = 18;
TqInt   CqShaderStack::m_maxsamples = 0;

//----------------------------------------------------------------------
/** Returns the next shaderstack variable and allocates more if 
 *  it needs to be
 */

IqShaderData* CqShaderStack::GetNextTemp( EqVariableType type, EqVariableClass _class )
{
	switch ( type )
	{
	case type_point:
	if ( _class == class_uniform )
	{
		while ( m_iUPoolTops[ type_point ] >= m_aUPPool.size() )
			m_aUPPool.resize( m_aUPPool.size() + 1 );
		return ( &m_aUPPool[ m_iUPoolTops[ type_point ] ] );
	}
	else
	{
		while ( m_iVPoolTops[ type_point ] >= m_aVPPool.size() )
			m_aVPPool.resize( m_aVPPool.size() + 1 );
		return ( &m_aVPPool[ m_iVPoolTops[ type_point ] ] );
	}

	case type_string:
	if ( _class == class_uniform )
	{
		while ( m_iUPoolTops[ type_string ] >= m_aUSPool.size() )
			m_aUSPool.resize( m_aUSPool.size() + 1 );
		return ( &m_aUSPool[ m_iUPoolTops[ type_string ] ] );
	}
	else
	{
		while ( m_iVPoolTops[ type_string ] >= m_aVSPool.size() )
			m_aVSPool.resize( m_aVSPool.size() + 1 );
		return ( &m_aVSPool[ m_iVPoolTops[ type_string ] ] );
	}

	case type_color:
	if ( _class == class_uniform )
	{
		while ( m_iUPoolTops[ type_color ] >= m_aUCPool.size() )
			m_aUCPool.resize( m_aUCPool.size() + 1 );
		return ( &m_aUCPool[ m_iUPoolTops[ type_color ] ] );
	}
	else
	{
		while ( m_iVPoolTops[ type_color ] >= m_aVCPool.size() )
			m_aVCPool.resize( m_aVCPool.size() + 1 );
		return ( &m_aVCPool[ m_iVPoolTops[ type_color ] ] );
	}

	case type_normal:
	if ( _class == class_uniform )
	{
		while ( m_iUPoolTops[ type_normal ] >= m_aUNPool.size() )
			m_aUNPool.resize( m_aUNPool.size() + 1 );
		return ( &m_aUNPool[ m_iUPoolTops[ type_normal ] ] );
	}
	else
	{
		while ( m_iVPoolTops[ type_normal ] >= m_aVNPool.size() )
			m_aVNPool.resize( m_aVNPool.size() + 1 );
		return ( &m_aVNPool[ m_iVPoolTops[ type_normal ] ] );
	}

	case type_vector:
	if ( _class == class_uniform )
	{
		while ( m_iUPoolTops[ type_vector ] >= m_aUVPool.size() )
			m_aUVPool.resize( m_aUVPool.size() + 1 );
		return ( &m_aUVPool[ m_iUPoolTops[ type_vector ] ] );
	}
	else
	{
		while ( m_iVPoolTops[ type_vector ] >= m_aVVPool.size() )
			m_aVVPool.resize( m_aVVPool.size() + 1 );
		return ( &m_aVVPool[ m_iVPoolTops[ type_vector ] ] );
	}

	case type_matrix:
	if ( _class == class_uniform )
	{
		while ( m_iUPoolTops[ type_matrix ] >= m_aUMPool.size() )
			m_aUMPool.resize( m_aUMPool.size() + 1 );
		return ( &m_aUMPool[ m_iUPoolTops[ type_matrix ] ] );
	}
	else
	{
		while ( m_iVPoolTops[ type_matrix ] >= m_aVMPool.size() )
			m_aVMPool.resize( m_aVMPool.size() + 1 );
		return ( &m_aVMPool[ m_iVPoolTops[ type_matrix ] ] );
	}

	default:
	if ( type == type_float )
	{
		if ( _class == class_uniform )
		{
			while ( m_iUPoolTops[ type_float ] >= m_aUFPool.size() )
				m_aUFPool.resize( m_aUFPool.size() + 1 );
			return ( &m_aUFPool[ m_iUPoolTops[ type_float ] ] );
		}
		else
		{
			while ( m_iVPoolTops[ type_float ] >= m_aVFPool.size() )
				m_aVFPool.resize( m_aVFPool.size() + 1 );
			return ( &m_aVFPool[ m_iVPoolTops[ type_float ] ] );
		}
	}
	return NULL;
	}
}

//----------------------------------------------------------------------
/** Push a new shader variable reference onto the stack.
 */
void	CqShaderStack::Push( IqShaderData* pv )
{
	while ( m_iTop >= m_Stack.size() )
	{
		TqInt n = m_Stack.size() + 1;
		m_Stack.resize( n );
		m_Stack.reserve( n );
	}

	m_Stack[ m_iTop ] = pv;
	m_iTop ++;
	if ( pv->Class() == class_uniform )
		m_iUPoolTops[ pv->Type() ] ++;
	else
		m_iVPoolTops[ pv->Type() ] ++;
	m_maxsamples = MAX(m_maxsamples, m_iTop);
}

//----------------------------------------------------------------------
/** Pop the top stack entry.
 * \param f Boolean value to update if this is varying. If not varying, leaves f unaltered.
 * \return Reference to the top stack entry.
 */
IqShaderData* CqShaderStack::Pop( TqBool& f )
{
	if ( m_iTop ) m_iTop--;

	IqShaderData* pVal = m_Stack[ m_iTop ];

	f = pVal->Size() > 1 || f;

	if ( pVal->Class() == class_uniform )
	{
		m_iUPoolTops[ pVal->Type() ] --;
		assert( m_iUPoolTops[ pVal->Type() ] >= 0 );
	}
	else
	{
		m_iVPoolTops[ pVal->Type() ] --;
		assert( m_iVPoolTops[ pVal->Type() ] >= 0 );
	}

	return ( pVal );
}

//----------------------------------------------------------------------
/** Duplicate the top stack entry.
 */
void	CqShaderStack::Dup()
{
	TqInt iTop = m_iTop-1;

	IqShaderData* s = GetNextTemp( m_Stack[ iTop ] ->Type(), m_Stack[ iTop ] ->Class() );
	s->SetValueFromVariable( m_Stack[ iTop ] );
	Push( s );
}

//----------------------------------------------------------------------
/** Drop the top stack entry.
 */
void	CqShaderStack::Drop()
{
	TqBool f = TqFalse;
	Pop( f );
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
END_NAMESPACE( Aqsis )
//---------------------------------------------------------------------
