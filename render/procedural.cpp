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
CqProcedural::CqProcedural( CqBound &B ) : CqSurface()
{
	m_Bound = B;	
	m_pconStored = QGetRenderContext()->pconCurrent();
	m_pconStored->AddRef();
//	m_pAttributes = m_pAttributes->Write();
//	m_pTransform = m_pTransform->Write();
}




TqInt CqProcedural::Split( std::vector<CqBasicSurface*>& aSplits )
{
	// Store current context, set current context to the stored one
	CqModeBlock *pconSave = QGetRenderContext()->pconCurrent( m_pconStored );
//	if( !m_pAttributes ) m_pAttributes = QGetRenderContext()->pattrWriteCurrent();
	CqAttributes *pattrsSave = const_cast<CqAttributes*> (QGetRenderContext()->pconCurrent()->pattrCurrent( m_pAttributes )); 
//	if( !m_pTransform ) m_pTransform = QGetRenderContext()->ptransWriteCurrent();
	CqTransform *ptransSave = const_cast<CqTransform*> (QGetRenderContext()->pconCurrent()->ptransCurrent( m_pTransform ));

	// Call the procedural secific Split()
	SplitProcedural();

	// restore saved context
	QGetRenderContext()->pconCurrent()->ptransCurrent( ptransSave );
	QGetRenderContext()->pconCurrent()->pattrCurrent( pattrsSave );
	QGetRenderContext()->pconCurrent( pconSave );
	return 0;
}


/**
 * CqProcedural destructor.
 */
CqProcedural::~CqProcedural()
{ 
	m_pconStored->Release();
};


/**
 * CqProcDelayedReadArchive constructor.
 */
CqProcDelayedReadArchive::CqProcDelayedReadArchive() : CqProcedural()
{
}

/* CqProcDelayedReadArchive.
 */
CqProcDelayedReadArchive::CqProcDelayedReadArchive( CqBound &B , CqString &filename) : CqProcedural(B)
{
	m_strFileName = filename;	
}


/**
 * Procedural Specific Split function implementing ReadArchive
 **/
void CqProcDelayedReadArchive::SplitProcedural(void)
{
	RiReadArchive( const_cast<char*> (m_strFileName.c_str()), (RtArchiveCallback) NULL );
}


/**
 * CqProcDelayedReadArchive destructor.
 */
CqProcDelayedReadArchive::~CqProcDelayedReadArchive()
{ 

}


END_NAMESPACE( Aqsis )
