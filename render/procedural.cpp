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

/**
        \file
        \brief Implements the classes and support structures for 
                handling RenderMan Procedural primitives.
        \author Jonathan Merritt (j.merritt@pgrad.unimelb.edu.au)
*/

#include <stdio.h>
#include <string.h>
#include "aqsis.h"
#include "imagebuffer.h"
#include "micropolygon.h"
#include "renderer.h"
#include "procedural.h"

START_NAMESPACE( Aqsis )


/**
 * CqProcedural constructor.
 */
CqProcedural::CqProcedural() : CqSurface()
{
}

/**
 * CqProcedural copy constructor.
 */
CqProcedural::CqProcedural(RtPointer data, CqBound &B, RtProcSubdivFunc subfunc, RtProcFreeFunc freefunc ) : CqSurface()
{
	m_pData = data;
	m_Bound = B;	
	m_pSubdivFunc = subfunc;
	m_pFreeFunc = freefunc;

	m_pconStored = QGetRenderContext()->pconCurrent();
	m_pconStored->AddRef();
}




TqInt CqProcedural::Split( std::vector<CqBasicSurface*>& aSplits )
{
	// Store current context, set current context to the stored one
	CqModeBlock *pconSave = QGetRenderContext()->pconCurrent( m_pconStored );

	// Call the procedural secific Split()
	RiAttributeBegin();

	int detail = 1;// Need to work out what this should be
	m_pSubdivFunc(m_pData, detail);

	RiAttributeEnd();

	// restore saved context
	QGetRenderContext()->pconCurrent( pconSave );
	return 0;
}


/**
 * CqProcedural destructor.
 */
CqProcedural::~CqProcedural()
{ 
	m_pconStored->Release();
	if( m_pFreeFunc ) m_pFreeFunc( m_pData );
}


END_NAMESPACE( Aqsis )

