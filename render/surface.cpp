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
		\brief Implements the base GPrim handling classes.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include	"aqsis.h"
#include	"renderer.h"
#include	"micropolygon.h"
#include	"surface.h"
#include	"vector2d.h"
#include	"imagebuffer.h"

START_NAMESPACE( Aqsis )


//---------------------------------------------------------------------
/** Default constructor
 */

CqBasicSurface::CqBasicSurface() : CqListEntry<CqBasicSurface>(), m_fDiceable( TqTrue ), m_fDiscard( TqFalse ), m_EyeSplitCount( 0 ),
		m_pAttributes( 0 ), m_pTransform( 0 ), m_SplitDir( SplitDir_U ), m_pCSGNode( NULL )
{
	// Set a refernce with the current attributes.
	m_pAttributes = const_cast<CqAttributes*>( QGetRenderContext() ->pattrCurrent() );
	m_pAttributes->AddRef();

	m_pTransform = const_cast<CqTransform*>( QGetRenderContext() ->ptransCurrent() );
	m_pTransform->AddRef();

	m_CachedBound = TqFalse;

	// If the current context is a solid node, and is a 'primitive', attatch this surface to the node.
	if ( QGetRenderContext() ->pconCurrent() ->isSolid() )
	{
		CqContext * pSolid = QGetRenderContext() ->pconCurrent();
		if ( pSolid->pCSGNode() ->NodeType() == CqCSGTreeNode::CSGNodeType_Primitive )
		{
			m_pCSGNode = pSolid->pCSGNode();
			m_pCSGNode->AddRef();
		}
		else
			CqAttributeError error(RIE_BADSOLID, Severity_Normal, "Primitive defined when not in 'Primitive' solid block", m_pAttributes, TqTrue);
	}
}


//---------------------------------------------------------------------
/** Copy constructor
 */

CqBasicSurface::CqBasicSurface( const CqBasicSurface& From ) : m_fDiceable( TqTrue ), m_SplitDir( SplitDir_U )
{
	*this = From;

	// Set a reference with the donors attributes.
	m_pAttributes = From.m_pAttributes;
	m_pAttributes->AddRef();

	m_pTransform = From.m_pTransform;
	m_pTransform->AddRef();

	m_CachedBound = From.m_CachedBound;
	m_Bound = From.m_Bound;
}


//---------------------------------------------------------------------
/** Assignement operator
 */

CqBasicSurface& CqBasicSurface::operator=( const CqBasicSurface& From )
{
	m_fDiceable = From.m_fDiceable;
	m_EyeSplitCount = From.m_EyeSplitCount;
	m_fDiscard = From.m_fDiscard;

	SetSurfaceParameters(From);

	return ( *this );
}


//---------------------------------------------------------------------
/** Copy the local surface parameters from the donor surface.
 */

void CqBasicSurface::SetSurfaceParameters( const CqBasicSurface& From )
{
	// If we already have attributes, unreference them now as we don't need them anymore.
	if ( m_pAttributes ) m_pAttributes->Release();
	if ( m_pTransform ) m_pTransform->Release();
	if ( m_pCSGNode ) m_pCSGNode->Release();

	// Now store and reference our new attributes.
	m_pAttributes = From.m_pAttributes;
	m_pAttributes->AddRef();

	m_pTransform = From.m_pTransform;
	m_pTransform->AddRef();

	m_pCSGNode = From.m_pCSGNode;
	if ( m_pCSGNode ) m_pCSGNode->AddRef();
}


//---------------------------------------------------------------------
/** Return the name of this primitive surface if specified as a "identifier" "name" attribute,
 * otherwise return "not named"
 */

CqString CqBasicSurface::strName() const
{
	const CqString * pattrLightName = pAttributes() ->GetStringAttribute( "identifier", "name" );
	CqString strName( "not named" );
	if ( pattrLightName != 0 ) strName = pattrLightName[ 0 ];

	return ( strName );
}


//---------------------------------------------------------------------
/** Work out which standard shader variables this surface requires by looking at the shaders.
 */

TqInt CqBasicSurface::Uses() const
{
	TqInt Uses = gDefUses;
	IqShader* pshadSurface = pAttributes() ->pshadSurface();
	IqShader* pshadDisplacement = pAttributes() ->pshadDisplacement();
	IqShader* pshadAtmosphere = pAttributes() ->pshadAtmosphere();

	if ( NULL == pshadSurface && NULL == pshadDisplacement && NULL == pshadAtmosphere )
		return( 0 );
	
	if ( pshadSurface ) Uses |= pshadSurface->Uses();
	if ( pshadDisplacement ) Uses |= pshadDisplacement->Uses();
	if ( pshadAtmosphere ) Uses |= pshadAtmosphere->Uses();

	// Just a quick check, if it uses dPdu/dPdv must also use du/dv
	if ( USES( Uses, EnvVars_dPdu ) ) Uses |= ( 1 << EnvVars_du );
	if ( USES( Uses, EnvVars_dPdv ) ) Uses |= ( 1 << EnvVars_dv );
	// Just a quick check, if it uses du/dv must also use u/v
	if ( USES( Uses, EnvVars_du ) ) Uses |= ( 1 << EnvVars_u );
	if ( USES( Uses, EnvVars_dv ) ) Uses |= ( 1 << EnvVars_v );

	return ( Uses );
}


//---------------------------------------------------------------------
/** Default constructor
 */

CqSurface::CqSurface() : CqBasicSurface(),
		m_P( "P" ),
		m_N( "N" )
{
	// Nullify the standard primitive variables index table.
	TqInt i;
	for( i = 0; i < EnvVars_Last; i++ )
		m_aiStdPrimitiveVars[ i ] = -1;	
}

//---------------------------------------------------------------------
/** Copy constructor
 */

CqSurface::CqSurface( const CqSurface& From ) :
		m_P( "P" ),
		m_N( "N" )
{
	*this = From;
}


//---------------------------------------------------------------------
/** Assignement operator
 */

CqSurface& CqSurface::operator=( const CqSurface& From )
{
	CqBasicSurface::operator=( From );
	return ( *this );
}

//---------------------------------------------------------------------
/** Copy all the primitive variables from the donor to this.
 */

void CqSurface::ClonePrimitiveVariables( const CqSurface& From )
{
	// Copy primitive variables.
	m_P = From.m_P;
	m_N = From.m_N;

	// Clone any user parameters.
	m_aUserParams.clear();
	std::vector<CqParameter*>::const_iterator iUP;
	for( iUP = From.m_aUserParams.begin(); iUP != From.m_aUserParams.end(); iUP++ )
		AddPrimitiveVariable( (*iUP)->Clone() );

	// Copy the standard primitive variables index table.
	TqInt i;
	for( i = 0; i < EnvVars_Last; i++ )
		m_aiStdPrimitiveVars[ i ] = From.m_aiStdPrimitiveVars[ i ];	
}

//---------------------------------------------------------------------
/** Set the default values (where available) from the attribute state for all standard
 * primitive variables.
 */

void CqSurface::SetDefaultPrimitiveVariables( TqBool bUseDef_st )
{
	TqInt bUses = Uses();

	// Set default values for all of our parameters

	// s and t default to four values, if the particular surface type requires different it is up
	// to the surface to override or change this after the fact.
	if ( USES( bUses, EnvVars_s ) && bUseDef_st )
	{
		AddPrimitiveVariable(new CqParameterTypedVarying<TqFloat, type_float, TqFloat>("s") );
		s()->SetSize( 4 );
		TqInt i;
		for ( i = 0; i < 4; i++ )
			s()->pValue() [ i ] = m_pAttributes->GetFloatAttribute("System", "TextureCoordinates") [ i*2 ];
	}

	if ( USES( bUses, EnvVars_t ) && bUseDef_st )
	{
		AddPrimitiveVariable(new CqParameterTypedVarying<TqFloat, type_float, TqFloat>("t") );
		t()->SetSize( 4 );
		TqInt i;
		for ( i = 0; i < 4; i++ )
			t()->pValue() [ i ] = m_pAttributes->GetFloatAttribute("System", "TextureCoordinates") [ (i*2)+1 ];
	}

	if ( USES( bUses, EnvVars_u ) )
	{
		AddPrimitiveVariable(new CqParameterTypedVarying<TqFloat, type_float, TqFloat>("u") );
		u()->SetSize( 4 );
		u()->pValue() [ 0 ] = u()->pValue() [ 2 ] = 0.0;
		u()->pValue() [ 1 ] = u()->pValue() [ 3 ] = 1.0;
	}

	if ( USES( bUses, EnvVars_v ) )
	{
		AddPrimitiveVariable(new CqParameterTypedVarying<TqFloat, type_float, TqFloat>("v") );
		v()->SetSize( 4 );
		v()->pValue() [ 0 ] = v()->pValue() [ 1 ] = 0.0;
		v()->pValue() [ 2 ] = v()->pValue() [ 3 ] = 1.0;
	}
}


//---------------------------------------------------------------------
/** Dice the patch into a mesh of micropolygons.
 */

CqMicroPolyGridBase* CqSurface::Dice()
{
	// Create a new CqMicorPolyGrid for this patch
	CqMicroPolyGrid * pGrid = new CqMicroPolyGrid( m_uDiceSize, m_vDiceSize, this );

	TqInt lUses = Uses();

	// Dice the primitive variables.
	if ( USES( lUses, EnvVars_Cs ) ) 
	{
		if( bHasCs() )
			Cs()->BilinearDice( m_uDiceSize, m_vDiceSize, pGrid->Cs() );
		else if( NULL != pAttributes()->GetColorAttribute("System", "Color") )
			pGrid->Cs()->SetColor( pAttributes()->GetColorAttribute("System", "Color")[0]);
		else
			pGrid->Cs()->SetColor( CqColor( 1,1,1 ) );
	}

	if ( USES( lUses, EnvVars_Os ) ) 
	{
		if( bHasOs() )
			Os()->BilinearDice( m_uDiceSize, m_vDiceSize, pGrid->Os() );
		else if( NULL != pAttributes()->GetColorAttribute("System", "Opacity") )
			pGrid->Os()->SetColor( pAttributes()->GetColorAttribute("System", "Opacity")[0]);
		else
			pGrid->Os()->SetColor( CqColor( 1,1,1 ) );
	}

	if ( USES( lUses, EnvVars_s ) ) 
	{
		if( bHass() )
			s()->BilinearDice( m_uDiceSize, m_vDiceSize, pGrid->s() );
	}

	if ( USES( lUses, EnvVars_t ) ) 
	{
		if( bHast() )
			t()->BilinearDice( m_uDiceSize, m_vDiceSize, pGrid->t() );
	}

	if ( USES( lUses, EnvVars_u ) ) 
	{
		if( bHasu() )
			u()->BilinearDice( m_uDiceSize, m_vDiceSize, pGrid->u() );
	}

	if ( USES( lUses, EnvVars_v ) ) 
	{
		if( bHasv() )
			v()->BilinearDice( m_uDiceSize, m_vDiceSize, pGrid->v() );
	}
	

	NaturalInterpolate( &P(), m_uDiceSize, m_vDiceSize, pGrid->P() );

	// If the shaders need N and they have been explicitly specified, then bilinearly interpolate them.
	if ( USES( lUses, EnvVars_N ) )
	{
		if( bHasN() )
		{
			N().BilinearDice( m_uDiceSize, m_vDiceSize, pGrid->N() );
			pGrid->SetbShadingNormals( TqTrue );
		}
	}

	if( CanGenerateNormals() && USES( lUses, EnvVars_N ) )
	{
		GenerateGeometricNormals( m_uDiceSize, m_vDiceSize, pGrid->Ng() );
		pGrid->SetbGeometricNormals( TqTrue );
	}

	// Now we need to dice the user specified parameters as appropriate.
	std::vector<CqParameter*>::iterator iUP;
	for( iUP = m_aUserParams.begin(); iUP != m_aUserParams.end(); iUP++ )
	{
		/// \todo: Must transform point/vector/normal/matrix parameter variables from 'object' space to current before setting.
		if( NULL != pGrid->pAttributes()->pshadSurface() )
			pGrid->pAttributes()->pshadSurface()->SetArgument( (*iUP) );

		if( NULL != pGrid->pAttributes()->pshadDisplacement() )
			pGrid->pAttributes()->pshadDisplacement()->SetArgument( (*iUP) );

		if( NULL != pGrid->pAttributes()->pshadAtmosphere() )
			pGrid->pAttributes()->pshadAtmosphere()->SetArgument( (*iUP) );
	}

	return ( pGrid );
}



//---------------------------------------------------------------------
/** uSubdivide any user defined parameter variables.
 */

void CqSurface::uSubdivideUserParameters( CqSurface* pA, CqSurface* pB )
{
	// Iterate through any use parameters subdividing and storing the second value in the target surface.
	std::vector<CqParameter*>::iterator iUP;
	for( iUP = m_aUserParams.begin(); iUP != m_aUserParams.end(); iUP++ )
	{
		CqParameter* pNewA = (*iUP)->Clone();
		CqParameter* pNewB = (*iUP)->Clone();
		pNewA->uSubdivide( pNewB );
		pA->AddPrimitiveVariable( pNewA );
		pB->AddPrimitiveVariable( pNewB );
	}
}


//---------------------------------------------------------------------
/** vSubdivide any user defined parameter variables.
 */

void CqSurface::vSubdivideUserParameters( CqSurface* pA, CqSurface* pB )
{
	// Iterate through any use parameters subdividing and storing the second value in the target surface.
	std::vector<CqParameter*>::iterator iUP;
	for( iUP = m_aUserParams.begin(); iUP != m_aUserParams.end(); iUP++ )
	{
		CqParameter* pNewA = (*iUP)->Clone();
		CqParameter* pNewB = (*iUP)->Clone();
		pNewA->vSubdivide( pNewB );
		pA->AddPrimitiveVariable( pNewA );
		pB->AddPrimitiveVariable( pNewB );
	}
}

//---------------------------------------------------------------------

END_NAMESPACE( Aqsis )


