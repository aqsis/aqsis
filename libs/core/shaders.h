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
		\brief Declares support structures for registering shaders, and any built in shaders.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

//? Is .h included already?
#ifndef SHADERS_H_INCLUDED
#define SHADERS_H_INCLUDED 1

#include	<aqsis/aqsis.h>

#include <aqsis/math/color.h>
#include <aqsis/util/exception.h>
#include <aqsis/math/matrix.h>
#include <aqsis/util/sstring.h>
#include <aqsis/math/vector3d.h>
#include <aqsis/util/list.h>
#include <aqsis/shadervm/ishader.h>
#include <aqsis/shadervm/ishaderexecenv.h>
#include <aqsis/core/irenderer.h>
#include <aqsis/core/itransform.h>

namespace Aqsis {

//----------------------------------------------------------------------
/**
 * \class CqShaderKey
 * Key class for keeping track of shaders.
 */
class CqShaderKey
{
	public:
		CqShaderKey( const char* strName, EqShaderType type ) :
				m_type( type )
		{
			m_name = CqString::hash( strName );
		}
		CqShaderKey( const CqShaderKey &other ) :
				m_name( other.m_name ),
				m_type( other.m_type )
		{}
		virtual ~CqShaderKey()
		{}
		CqShaderKey &operator=( const CqShaderKey &other )
		{
			m_name = other.m_name;
			m_type = other.m_type;
			return (*this);
		}
		bool operator==( const CqShaderKey &other ) const
		{
			return (
			           ( m_name == other.m_name ) &&
			           ( m_type == other.m_type )
			       );
		}
		bool operator<( const CqShaderKey &other ) const
		{
			if ( m_name < other.m_name )
				return true;
			else if ( m_name > other.m_name )
				return false;
			else
				return m_type < other.m_type;
		}
	private:
		TqUlong        m_name; ///< Name of the shader.
		EqShaderType    m_type; ///< Type of the shader.
};

#if 0
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
#endif


//----------------------------------------------------------------------
/** \class CqShader
 * Abstract base class from which all shaders must be defined.
 */

class CqLayeredShader : public IqShader
{
	public:
		CqLayeredShader() : m_Uses( 0xFFFFFFFF )
		{
			// Find out if this shader is being declared outside the world construct. If so
			// if is effectively being defined in 'camera' space, which will affect the 
			// transformation of parameters. Should only affect lightsource shaders as these
			// are the only ones valid outside the world.
			m_outsideWorld = !QGetRenderContextI()->IsWorldBegin();
		}
		CqLayeredShader(const CqLayeredShader& from)
		{
			/// \todo Need to implement a proper copy constrctor, and operator=.
		}
		virtual	~CqLayeredShader()
		{}

		// Overidden from IqShader

		virtual const CqMatrix&	matCurrent()
		{
			return ( m_pTransform->matObjectToWorld(0) );
		}
		virtual const IqTransform*	getTransform() const
		{
			return ( m_pTransform.get() );
		}
		virtual void SetTransform(IqTransformPtr pTrans)
		{
			m_pTransform = pTrans;
		}
		virtual void	SetstrName( const char* strName )
		{
			m_strName = strName;
		}
		virtual const CqString& strName() const
		{
			return ( m_strName );
		}
		virtual	void	PrepareShaderForUse( )
		{
			if(!m_outsideWorld)
				InitialiseParameters();
		}
		virtual	void	InitialiseParameters( )
		{
			// Pass the argument on to the shader for each layer in the list.
			std::vector<std::pair<CqString, boost::shared_ptr<IqShader> > >::iterator i = m_Layers.begin();
			while( i != m_Layers.end() )
			{
				i->second->PrepareShaderForUse();
				++i;
			}
		}
		virtual	void	SetArgument( const CqString& name, EqVariableType type, const CqString& space, void* val )
		{
			// Pass the argument on to the shader for each layer in the list.
			std::vector<std::pair<CqString, boost::shared_ptr<IqShader> > >::iterator i = m_Layers.begin();
			while( i != m_Layers.end() )
			{
				i->second->SetArgument(name, type, space, val);
				++i;
			}
		}
		virtual	void	SetArgument( IqParameter* pParam, IqSurface* pSurface )
		{
			// Pass the argument on to the shader for each layer in the list.
			std::vector<std::pair<CqString, boost::shared_ptr<IqShader> > >::iterator i = m_Layers.begin();
			while( i != m_Layers.end() )
			{
				i->second->SetArgument(pParam, pSurface);
				++i;
			}
		}
		virtual	IqShaderData*	FindArgument( const CqString& name )
		{
			// Need to search for the output var backwards in the list, i.e. find the last shader layer
			// that will output the variable.
			std::vector<std::pair<CqString, boost::shared_ptr<IqShader> > >::reverse_iterator i = m_Layers.rbegin();
			while( i != m_Layers.rend() )
			{
				IqShaderData* result;
				if((result = i->second->FindArgument(name)) != NULL)
					return(result);
				++i;
			}
			return ( NULL );
		}
		virtual const std::vector<IqShaderData*>& GetArguments() const;
		virtual	bool	GetVariableValue( const char* name, IqShaderData* res ) const
		{
			// Again, need to search backwards through the list, as the last layer to affect this value will
			// be the one that matters.
			std::vector<std::pair<CqString, boost::shared_ptr<IqShader> > >::const_reverse_iterator i = m_Layers.rbegin();
			while( i != m_Layers.rend() )
			{
				if(i->second->GetVariableValue(name, res))
					return(true);
				++i;
			}
			return ( false );
		}
		virtual	void	Evaluate( IqShaderExecEnv* pEnv );
		virtual	void	PrepareDefArgs()
		{
			// Call PrepareDefArgs on all layers
			std::vector<std::pair<CqString, boost::shared_ptr<IqShader> > >::iterator i = m_Layers.begin();
			while( i != m_Layers.end() )
			{
				i->second->PrepareDefArgs();
				++i;
			}
		}
		virtual void	Initialise( const TqInt uGridRes, const TqInt vGridRes, const TqInt shadingPointCount, IqShaderExecEnv* pEnv )
		{
			// Call Initialise on all layers.
			std::vector<std::pair<CqString, boost::shared_ptr<IqShader> > >::iterator i = m_Layers.begin();
			while( i != m_Layers.end() )
			{
				i->second->Initialise(uGridRes, vGridRes, shadingPointCount, pEnv);
				++i;
			}
		}
		virtual	bool	fAmbient() const
		{
			// Not sure, probably always return false for now.
			return ( false );
		}
		virtual boost::shared_ptr<IqShader> Clone() const
		{
			return boost::shared_ptr<IqShader>(new CqLayeredShader(*this));
		}
		virtual bool	Uses( TqInt Var ) const
		{
			assert( Var >= 0 && Var < EnvVars_Last );
			return ( Uses( static_cast<EqEnvVars>( Var ) ) );
		}
		virtual TqInt	Uses() const
		{
			// Gather the uses from all layers as they are added.
			return ( m_Uses );
		}
		virtual IqShaderData* CreateVariable( EqVariableType Type, EqVariableClass Class, const CqString& name, IqShaderData::EqStorage storage )
		{
			// Call CreateVariable on the first shader in the list, all layers must be the same type.
			if(!m_Layers.empty())
				return(m_Layers.front().second->CreateVariable(Type, Class, name, storage));
			return ( NULL );
		}
		virtual IqShaderData* CreateVariableArray( EqVariableType Type, EqVariableClass Class, const CqString& name, TqInt Count, IqShaderData::EqStorage storage )
		{
			// Call CreateVariableArray on the first shader in the list, all layers must be the same type.
			if(!m_Layers.empty())
				return(m_Layers.front().second->CreateVariableArray(Type, Class, name, Count, storage));
			return ( NULL );
		}
		virtual IqShaderData* CreateTemporaryStorage( EqVariableType type, EqVariableClass _class )
		{
			// Call CreateTemporaryStorage on the first shader in the list, all layers must be the same type.
			if(!m_Layers.empty())
				return(m_Layers.front().second->CreateTemporaryStorage(type, _class));
			return ( NULL );
		}
		virtual void DeleteTemporaryStorage( IqShaderData* pData )
		{
			// Call DeleteTemporaryStorage on the first shader in the list, all layers must be the same type.
			if(!m_Layers.empty())
				m_Layers.front().second->DeleteTemporaryStorage(pData);
		}
		virtual void DefaultSurface()
	{}
		virtual bool IsLayered()
		{
			return(true);
		}

		virtual void AddLayer(const CqString& layername, const boost::shared_ptr<IqShader>& layer);
		virtual void AddConnection(const CqString& layer1, const CqString& variable1, const CqString& layer2, const CqString& variable2);

		virtual void SetType(EqShaderType type);
		virtual EqShaderType Type() const;

	protected:
		TqInt	m_Uses;			///< Bit vector representing the system variables used by this shader.
	private:
		IqTransformPtr	m_pTransform;	///< Transformation transformation to world coordinates in effect at the time this shader was instantiated.
		CqString	m_strName;		///< The name of this shader.
		bool		m_outsideWorld;

		std::vector<std::pair<CqString, boost::shared_ptr<IqShader> > >	m_Layers;
		std::map<CqString, TqInt> m_LayerMap;

		struct SqLayerConnection
		{
			CqString m_layer2Name;
			CqString m_variable1Name;
			CqString m_variable2Name;
		};
		std::multimap<CqString, SqLayerConnection> m_Connections;
}
;


//-----------------------------------------------------------------------

} // namespace Aqsis

//}  // End of #ifdef SHADERS_H_INCLUDED
#endif
