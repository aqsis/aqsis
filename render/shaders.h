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
		\brief Declares support structires for registering shaders, and any built in shaders.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is .h included already?
#ifndef SHADERS_H_INCLUDED
#define SHADERS_H_INCLUDED 1

#include	"aqsis.h"

#include "color.h"
#include "exception.h"
#include "matrix.h"
#include "sstring.h"
#include "vector3d.h"
#include "list.h"
#include "ishader.h"
#include "ishaderexecenv.h"

START_NAMESPACE( Aqsis )

//----------------------------------------------------------------------
/** \class CqShaderRegister
 * Class for registering shaders.
 */

class CqShaderRegister : public CqListEntry<CqShaderRegister>
{
	public:
		CqShaderRegister( const char* strName, EqShaderType type, IqShader* pShader ) :
				m_strName( strName ),
				m_Type( type ),
				m_pShader( pShader )
		{}
		virtual	~CqShaderRegister()
		{
			delete( m_pShader );
		}

		/** Get the name of the shader.
		 * \return Constant CqString reference containing the name.
		 */
		const CqString& strName()
		{
			return ( m_strName );
		}
		/** Get the shader type.
		 * \return Shader type as a member of EqShaderType.
		 */
		EqShaderType	Type()
		{
			return ( m_Type );
		}
		/** Create an instance of this shader.
		 * \return A pointer to the new instance of the shader.
		 */
		IqShader*	Create()
		{
			return ( m_pShader->Clone() );
		}

	private:
		CqString	m_strName;		///< The registered name of the shader.
		EqShaderType m_Type;		///< The type of the shader from EqShaderType.
		IqShader*	m_pShader;		///< Pointer to the shader class.
}
;


//----------------------------------------------------------------------
/** \class CqShader
 * Abstract base class from which all shaders must be defined.
 */

class CqShader : public IqShader
{
	public:
		CqShader() : m_Uses( 0xFFFFFFFF )
		{}
		virtual	~CqShader()
		{}

		// Overidden from IqShader
		virtual CqMatrix&	matCurrent()
		{
			return ( m_matCurrent );
		}
		virtual void	SetstrName( const char* strName )
		{
			m_strName = strName;
		}
		virtual const CqString& strName() const
		{
			return ( m_strName );
		}
		virtual	void	SetArgument( const CqString& name, EqVariableType type, const CqString& space, void* val )
		{}
		virtual	TqBool	GetValue( const char* name, IqShaderData* res )
		{
			return ( TqFalse );
		}
		virtual	void	Evaluate( IqShaderExecEnv* pEnv )
		{}
		virtual	void	PrepareDefArgs()
		{}
		virtual void	Initialise( const TqInt uGridRes, const TqInt vGridRes, IqShaderExecEnv* pEnv )
		{}
		virtual	TqBool	fAmbient() const
		{
			return ( TqFalse );
		}
		virtual IqShader*	Clone() const
		{
			return ( new CqShader );
		}
		virtual TqBool	Uses( TqInt Var ) const
		{
			assert( Var >= 0 && Var < EnvVars_Last );
			return ( Uses( static_cast<EqEnvVars>( Var ) ) );
		}
		virtual TqInt	Uses() const
		{
			return ( m_Uses );
		}
		virtual IqShaderData* CreateVariable(EqVariableType Type, EqVariableClass Class, const CqString& name, TqBool fArgument = TqFalse)
		{
			return( NULL );
		}
		virtual IqShaderData* CreateVariableArray(EqVariableType Type, EqVariableClass Class, const CqString& name, TqInt Count, TqBool fArgument = TqFalse)
		{
			return( NULL );
		}
		virtual IqShaderData* CreateTemporaryStorage()
		{
			return( NULL );
		}
		virtual void DeleteTemporaryStorage( IqShaderData* pData )
		{
		}

	protected:
		TqInt		m_Uses;			///< Bit vector representing the system variables used by this shader.
	private:
		CqMatrix	m_matCurrent;	///< Transformation matrix to world coordinates in effect at the time this shader was instantiated.
		CqString	m_strName;		///< The name of this shader.
};


//---------------------------------------------------------------------
//---------------------------------------------------------------------
//---------------------------------------------------------------------
// These are the built in shaders, they will be registered as "builtin_<name>"
// these should be used where speed is an issue.


//-----------------------------------------------------------------------

END_NAMESPACE( Sappire )

//}  // End of #ifdef SHADERS_H_INCLUDED
#endif
