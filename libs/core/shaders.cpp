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
		\brief Implements support structures for registering shaders, and any built in shaders.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#include	<string.h>

#include	<aqsis/aqsis.h>
#include	"lights.h"
#include	"shaders.h"
#include	<aqsis/util/file.h>

namespace Aqsis {



/** Add a new layer to this layered shader.
 */

void CqLayeredShader::AddLayer(const CqString& layername, const boost::shared_ptr<IqShader> &layer)
{
	// Add it to the list of shaders first
	m_Layers.push_back(std::pair<CqString, boost::shared_ptr<IqShader> >(layername, layer));
	m_LayerMap[layername] = m_Layers.size()-1;

	// Now combine the new layer's "Uses" data into ours.
	m_Uses |= layer->Uses();
}


void CqLayeredShader::AddConnection(const CqString& layer1, const CqString& variable1, const CqString& layer2, const CqString& variable2)
{
	// Add the connection.
	/// \todo should check that the layers exist.
	SqLayerConnection conn;
	conn.m_layer2Name = layer2;
	conn.m_variable1Name = variable1;
	conn.m_variable2Name = variable2;
	m_Connections.insert(std::pair<CqString, SqLayerConnection>(layer1, conn));
}


bool LayerNameMatch(std::pair<CqString, boost::shared_ptr<IqShader> >& elem1, std::pair<CqString, boost::shared_ptr<IqShader> >& elem2 )
{
	return(elem1.first.compare(elem2.first) == 0);
}



void CqLayeredShader::Evaluate( IqShaderExecEnv* pEnv )
{
	if(!m_Layers.empty())
	{
		TqInt index = 1;

		std::vector<std::pair<CqString, boost::shared_ptr<IqShader> > >::iterator i = m_Layers.begin();
		while( i != m_Layers.end() )
		{
			i->second->Evaluate(pEnv);
			std::vector<std::pair<CqString, boost::shared_ptr<IqShader> > >::iterator j = i;
			++i;
			if(m_Connections.count(j->first) > 0)
			{
				// Iterate across all map entries with the previous layer as it's layer1 name
				// and complete the connections.
				std::multimap<CqString, SqLayerConnection>::iterator start = m_Connections.lower_bound(j->first);
				std::multimap<CqString, SqLayerConnection>::iterator end = m_Connections.upper_bound(j->first);
				while( start != end )
				{
					// Find the target layer in the map, if it exists.
					if(m_LayerMap.count(start->second.m_layer2Name) > 0)
					{
						IqShaderData* pOut = j->second->FindArgument(start->second.m_variable1Name);
						if(!pOut)
							pOut = pEnv->FindStandardVar(start->second.m_variable1Name.c_str());
						boost::shared_ptr<IqShader> targetlayer = m_Layers[m_LayerMap[start->second.m_layer2Name]].second;
						IqShaderData* pIn = targetlayer->FindArgument(start->second.m_variable2Name);
						if(!pIn)
							pIn = pEnv->FindStandardVar(start->second.m_variable2Name.c_str());
						if(pOut && pIn)
							pIn->SetValueFromVariable(pOut);
					}
					++start;
				}
			}
			++index;
		}
	}
}

const std::vector<IqShaderData*>& CqLayeredShader::GetArguments() const
{
	// Implemented only so we can return something (ugh, fat interface; needs
	// review).  Should only be needed by libslxargs...
	assert(!m_Layers.empty());
	return m_Layers.front().second->GetArguments();
}

void CqLayeredShader::SetType( EqShaderType type)
{
}

EqShaderType CqLayeredShader::Type() const
{
	// Implemented for completeness.  Probably only needed in libslxargs.
	if(!m_Layers.empty())
		return m_Layers.front().second->Type();
	else
	{
		assert(0 && "Type not well-defined");
		return Type_Surface;
	}
}

//---------------------------------------------------------------------

} // namespace Aqsis


