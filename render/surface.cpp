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
		CqModeBlock * pSolid = QGetRenderContext() ->pconCurrent();
		if ( pSolid->pCSGNode() ->NodeType() == CqCSGTreeNode::CSGNodeType_Primitive )
		{
			m_pCSGNode = pSolid->pCSGNode();
			m_pCSGNode->AddRef();
		}
		else
			CqAttributeError error( RIE_BADSOLID, Severity_Normal, "Primitive defined when not in 'Primitive' solid block", m_pAttributes, TqTrue );
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

	SetSurfaceParameters( From );

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
		return ( 0 );

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

CqSurface::CqSurface() : CqBasicSurface()
{
	// Nullify the standard primitive variables index table.
	TqInt i;
	for ( i = 0; i < EnvVars_Last; i++ )
		m_aiStdPrimitiveVars[ i ] = -1;
}

//---------------------------------------------------------------------
/** Copy constructor
 */

CqSurface::CqSurface( const CqSurface& From )
{
	*this = From;
}


//---------------------------------------------------------------------
/** Assignement operator
 */

CqSurface& CqSurface::operator=( const CqSurface& From )
{
	CqBasicSurface::operator=( From );
	// Nullify the standard primitive variables index table.
	TqInt i;
	for ( i = 0; i < EnvVars_Last; i++ )
		m_aiStdPrimitiveVars[ i ] = -1;
	return ( *this );
}

//---------------------------------------------------------------------
/** Copy all the primitive variables from the donor to this.
 */

void CqSurface::ClonePrimitiveVariables( const CqSurface& From )
{
	// Clone any primitive variables.
	m_aUserParams.clear();
	std::vector<CqParameter*>::const_iterator iUP;
	for ( iUP = From.m_aUserParams.begin(); iUP != From.m_aUserParams.end(); iUP++ )
		AddPrimitiveVariable( ( *iUP ) ->Clone() );

	// Copy the standard primitive variables index table.
	TqInt i;
	for ( i = 0; i < EnvVars_Last; i++ )
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
		AddPrimitiveVariable( new CqParameterTypedVarying<TqFloat, type_float, TqFloat>( "s" ) );
		s() ->SetSize( 4 );
		TqInt i;
		for ( i = 0; i < 4; i++ )
			s() ->pValue() [ i ] = m_pAttributes->GetFloatAttribute( "System", "TextureCoordinates" ) [ i * 2 ];
	}

	if ( USES( bUses, EnvVars_t ) && bUseDef_st )
	{
		AddPrimitiveVariable( new CqParameterTypedVarying<TqFloat, type_float, TqFloat>( "t" ) );
		t() ->SetSize( 4 );
		TqInt i;
		for ( i = 0; i < 4; i++ )
			t() ->pValue() [ i ] = m_pAttributes->GetFloatAttribute( "System", "TextureCoordinates" ) [ ( i * 2 ) + 1 ];
	}

	if ( USES( bUses, EnvVars_u ) )
	{
		AddPrimitiveVariable( new CqParameterTypedVarying<TqFloat, type_float, TqFloat>( "u" ) );
		u() ->SetSize( 4 );
		u() ->pValue() [ 0 ] = u() ->pValue() [ 2 ] = 0.0;
		u() ->pValue() [ 1 ] = u() ->pValue() [ 3 ] = 1.0;
	}

	if ( USES( bUses, EnvVars_v ) )
	{
		AddPrimitiveVariable( new CqParameterTypedVarying<TqFloat, type_float, TqFloat>( "v" ) );
		v() ->SetSize( 4 );
		v() ->pValue() [ 0 ] = v() ->pValue() [ 1 ] = 0.0;
		v() ->pValue() [ 2 ] = v() ->pValue() [ 3 ] = 1.0;
	}
}


void CqSurface::NaturalSubdivide( CqParameter* pParam, CqParameter* pParam1, CqParameter* pParam2, TqBool u )
{
	switch ( pParam->Type() )
	{
			case type_float:
			{
				CqParameterTyped<TqFloat, TqFloat>* pTParam = static_cast<CqParameterTyped<TqFloat, TqFloat>*>( pParam );
				CqParameterTyped<TqFloat, TqFloat>* pTResult1 = static_cast<CqParameterTyped<TqFloat, TqFloat>*>( pParam1 );
				CqParameterTyped<TqFloat, TqFloat>* pTResult2 = static_cast<CqParameterTyped<TqFloat, TqFloat>*>( pParam2 );
				TypedNaturalSubdivide( pTParam, pTResult1, pTResult2, u );
				break;
			}

			case type_integer:
			{
				CqParameterTyped<TqInt, TqFloat>* pTParam = static_cast<CqParameterTyped<TqInt, TqFloat>*>( pParam );
				CqParameterTyped<TqInt, TqFloat>* pTResult1 = static_cast<CqParameterTyped<TqInt, TqFloat>*>( pParam1 );
				CqParameterTyped<TqInt, TqFloat>* pTResult2 = static_cast<CqParameterTyped<TqInt, TqFloat>*>( pParam2 );
				TypedNaturalSubdivide( pTParam, pTResult1, pTResult2, u );
				break;
			}

			case type_point:
			case type_vector:
			case type_normal:
			{
				CqParameterTyped<CqVector3D, CqVector3D>* pTParam = static_cast<CqParameterTyped<CqVector3D, CqVector3D>*>( pParam );
				CqParameterTyped<CqVector3D, CqVector3D>* pTResult1 = static_cast<CqParameterTyped<CqVector3D, CqVector3D>*>( pParam1 );
				CqParameterTyped<CqVector3D, CqVector3D>* pTResult2 = static_cast<CqParameterTyped<CqVector3D, CqVector3D>*>( pParam2 );
				TypedNaturalSubdivide( pTParam, pTResult1, pTResult2, u );
				break;
			}

			case type_hpoint:
			{
				CqParameterTyped<CqVector4D, CqVector3D>* pTParam = static_cast<CqParameterTyped<CqVector4D, CqVector3D>*>( pParam );
				CqParameterTyped<CqVector4D, CqVector3D>* pTResult1 = static_cast<CqParameterTyped<CqVector4D, CqVector3D>*>( pParam1 );
				CqParameterTyped<CqVector4D, CqVector3D>* pTResult2 = static_cast<CqParameterTyped<CqVector4D, CqVector3D>*>( pParam2 );
				TypedNaturalSubdivide( pTParam, pTResult1, pTResult2, u );
				break;
			}


			case type_color:
			{
				CqParameterTyped<CqColor, CqColor>* pTParam = static_cast<CqParameterTyped<CqColor, CqColor>*>( pParam );
				CqParameterTyped<CqColor, CqColor>* pTResult1 = static_cast<CqParameterTyped<CqColor, CqColor>*>( pParam1 );
				CqParameterTyped<CqColor, CqColor>* pTResult2 = static_cast<CqParameterTyped<CqColor, CqColor>*>( pParam2 );
				TypedNaturalSubdivide( pTParam, pTResult1, pTResult2, u );
				break;
			}

			case type_string:
			{
				CqParameterTyped<CqString, CqString>* pTParam = static_cast<CqParameterTyped<CqString, CqString>*>( pParam );
				CqParameterTyped<CqString, CqString>* pTResult1 = static_cast<CqParameterTyped<CqString, CqString>*>( pParam1 );
				CqParameterTyped<CqString, CqString>* pTResult2 = static_cast<CqParameterTyped<CqString, CqString>*>( pParam2 );
				TypedNaturalSubdivide( pTParam, pTResult1, pTResult2, u );
				break;
			}

			case type_matrix:
			{
				CqParameterTyped<CqMatrix, CqMatrix>* pTParam = static_cast<CqParameterTyped<CqMatrix, CqMatrix>*>( pParam );
				CqParameterTyped<CqMatrix, CqMatrix>* pTResult1 = static_cast<CqParameterTyped<CqMatrix, CqMatrix>*>( pParam1 );
				CqParameterTyped<CqMatrix, CqMatrix>* pTResult2 = static_cast<CqParameterTyped<CqMatrix, CqMatrix>*>( pParam2 );
				TypedNaturalSubdivide( pTParam, pTResult1, pTResult2, u );
				break;
			}
	}
}


void CqSurface::NaturalDice( CqParameter* pParameter, TqInt uDiceSize, TqInt vDiceSize, IqShaderData* pData )
{
	switch ( pParameter->Type() )
	{
			case type_float:
			{
				CqParameterTyped<TqFloat, TqFloat>* pTParam = static_cast<CqParameterTyped<TqFloat, TqFloat>*>( pParameter );
				TypedNaturalDice( uDiceSize, vDiceSize, pTParam, pData );
				break;
			}

			case type_integer:
			{
				CqParameterTyped<TqInt, TqFloat>* pTParam = static_cast<CqParameterTyped<TqInt, TqFloat>*>( pParameter );
				TypedNaturalDice( uDiceSize, vDiceSize, pTParam, pData );
				break;
			}

			case type_point:
			case type_vector:
			case type_normal:
			{
				CqParameterTyped<CqVector3D, CqVector3D>* pTParam = static_cast<CqParameterTyped<CqVector3D, CqVector3D>*>( pParameter );
				TypedNaturalDice( uDiceSize, vDiceSize, pTParam, pData );
				break;
			}

			case type_hpoint:
			{
				CqParameterTyped<CqVector4D, CqVector3D>* pTParam = static_cast<CqParameterTyped<CqVector4D, CqVector3D>*>( pParameter );
				TypedNaturalDice( uDiceSize, vDiceSize, pTParam, pData );
				break;
			}

			case type_color:
			{
				CqParameterTyped<CqColor, CqColor>* pTParam = static_cast<CqParameterTyped<CqColor, CqColor>*>( pParameter );
				TypedNaturalDice( uDiceSize, vDiceSize, pTParam, pData );
				break;
			}

			case type_string:
			{
				CqParameterTyped<CqString, CqString>* pTParam = static_cast<CqParameterTyped<CqString, CqString>*>( pParameter );
				TypedNaturalDice( uDiceSize, vDiceSize, pTParam, pData );
				break;
			}

			case type_matrix:
			{
				CqParameterTyped<CqMatrix, CqMatrix>* pTParam = static_cast<CqParameterTyped<CqMatrix, CqMatrix>*>( pParameter );
				TypedNaturalDice( uDiceSize, vDiceSize, pTParam, pData );
				break;
			}
	}
}



//---------------------------------------------------------------------
/** Dice the patch into a mesh of micropolygons.
 */

CqMicroPolyGridBase* CqSurface::Dice()
{
	PreDice( m_uDiceSize, m_vDiceSize );

	// Create a new CqMicorPolyGrid for this patch
	CqMicroPolyGrid * pGrid = new CqMicroPolyGrid( m_uDiceSize, m_vDiceSize, this );

	TqInt lUses = Uses();

	// Dice the primitive variables.
	if ( USES( lUses, EnvVars_Cs ) && ( NULL != pGrid->Cs() ) )
	{
		if ( bHasCs() )
			Cs() ->Dice( m_uDiceSize, m_vDiceSize, pGrid->Cs(), this );
		else if ( NULL != pAttributes() ->GetColorAttribute( "System", "Color" ) )
			pGrid->Cs() ->SetColor( pAttributes() ->GetColorAttribute( "System", "Color" ) [ 0 ] );
		else
			pGrid->Cs() ->SetColor( CqColor( 1, 1, 1 ) );
	}

	if ( USES( lUses, EnvVars_Os ) && ( NULL != pGrid->Os() ) )
	{
		if ( bHasOs() )
			Os() ->Dice( m_uDiceSize, m_vDiceSize, pGrid->Os(), this );
		else if ( NULL != pAttributes() ->GetColorAttribute( "System", "Opacity" ) )
			pGrid->Os() ->SetColor( pAttributes() ->GetColorAttribute( "System", "Opacity" ) [ 0 ] );
		else
			pGrid->Os() ->SetColor( CqColor( 1, 1, 1 ) );
	}

	if ( USES( lUses, EnvVars_s ) && ( NULL != pGrid->s() ) && bHass() )
		s() ->Dice( m_uDiceSize, m_vDiceSize, pGrid->s(), this );

	if ( USES( lUses, EnvVars_t ) && ( NULL != pGrid->t() ) && bHast() )
		t() ->Dice( m_uDiceSize, m_vDiceSize, pGrid->t(), this );

	if ( USES( lUses, EnvVars_u ) && ( NULL != pGrid->u() ) && bHasu() )
		u() ->Dice( m_uDiceSize, m_vDiceSize, pGrid->u(), this );

	if ( USES( lUses, EnvVars_v ) && ( NULL != pGrid->v() ) && bHasv() )
		v() ->Dice( m_uDiceSize, m_vDiceSize, pGrid->v(), this );


	if ( NULL != pGrid->P() )
		NaturalDice( P(), m_uDiceSize, m_vDiceSize, pGrid->P() );

	// If the shaders need N and they have been explicitly specified, then bilinearly interpolate them.
	if ( USES( lUses, EnvVars_N ) && ( NULL != pGrid->N() ) && bHasN() )
	{
		N() ->Dice( m_uDiceSize, m_vDiceSize, pGrid->N(), this );
		pGrid->SetbShadingNormals( TqTrue );
	}

	if ( CanGenerateNormals() && USES( lUses, EnvVars_Ng ) )
	{
		GenerateGeometricNormals( m_uDiceSize, m_vDiceSize, pGrid->Ng() );
		pGrid->SetbGeometricNormals( TqTrue );
	}

	// Now we need to dice the user specified parameters as appropriate.
	std::vector<CqParameter*>::iterator iUP;
	for ( iUP = m_aUserParams.begin(); iUP != m_aUserParams.end(); iUP++ )
	{
		/// \todo: Must transform point/vector/normal/matrix parameter variables from 'object' space to current before setting.
		if ( NULL != pGrid->pAttributes() ->pshadSurface() )
			pGrid->pAttributes() ->pshadSurface() ->SetArgument( ( *iUP ), this );

		if ( NULL != pGrid->pAttributes() ->pshadDisplacement() )
			pGrid->pAttributes() ->pshadDisplacement() ->SetArgument( ( *iUP ), this );

		if ( NULL != pGrid->pAttributes() ->pshadAtmosphere() )
			pGrid->pAttributes() ->pshadAtmosphere() ->SetArgument( ( *iUP ), this );
	}

	PostDice( pGrid );

	return ( pGrid );
}


TqInt CqSurface::Split( std::vector<CqBasicSurface*>& aSplits )
{
	TqInt cSplits = PreSubdivide( aSplits, m_SplitDir == SplitDir_U );

	assert( aSplits.size() == 2 );

	aSplits[ 0 ] ->SetSurfaceParameters( *this );
	aSplits[ 0 ] ->SetSplitDir( ( SplitDir() == SplitDir_U ) ? SplitDir_V : SplitDir_U );
	aSplits[ 0 ] ->SetEyeSplitCount( EyeSplitCount() );
	aSplits[ 0 ] ->m_fDiceable = TqTrue;
	aSplits[ 0 ] ->AddRef();

	aSplits[ 1 ] ->SetSurfaceParameters( *this );
	aSplits[ 1 ] ->SetSplitDir( ( SplitDir() == SplitDir_U ) ? SplitDir_V : SplitDir_U );
	aSplits[ 1 ] ->SetEyeSplitCount( EyeSplitCount() );
	aSplits[ 1 ] ->m_fDiceable = TqTrue;
	aSplits[ 1 ] ->AddRef();

	// Iterate through any use parameters subdividing and storing the second value in the target surface.
	std::vector<CqParameter*>::iterator iUP;
	for ( iUP = m_aUserParams.begin(); iUP != m_aUserParams.end(); iUP++ )
	{
		CqParameter* pNewA = ( *iUP ) ->Clone();
		CqParameter* pNewB = ( *iUP ) ->Clone();
		( *iUP ) ->Subdivide( pNewA, pNewB, SplitDir() == SplitDir_U, this );
		static_cast<CqSurface*>( aSplits[ 0 ] ) ->AddPrimitiveVariable( pNewA );
		static_cast<CqSurface*>( aSplits[ 1 ] ) ->AddPrimitiveVariable( pNewB );
	}

	if ( !m_fDiceable )
	{
		std::vector<CqBasicSurface*> aSplits0;
		std::vector<CqBasicSurface*> aSplits1;

		cSplits = aSplits[ 0 ] ->Split( aSplits0 );
		cSplits += aSplits[ 1 ] ->Split( aSplits1 );
		// Release the old ones.
		aSplits[ 0 ] ->Release();
		aSplits[ 1 ] ->Release();

		aSplits.clear();
		aSplits.swap( aSplits0 );
		aSplits.insert( aSplits.end(), aSplits1.begin(), aSplits1.end() );
	}

	PostSubdivide(aSplits);

	return ( aSplits.size() );
}


//---------------------------------------------------------------------
/** uSubdivide any user defined parameter variables.
 */

void CqSurface::uSubdivideUserParameters( CqSurface* pA, CqSurface* pB )
{
	// Iterate through any use parameters subdividing and storing the second value in the target surface.
	std::vector<CqParameter*>::iterator iUP;
	for ( iUP = m_aUserParams.begin(); iUP != m_aUserParams.end(); iUP++ )
	{
		CqParameter* pNewA = ( *iUP ) ->Clone();
		CqParameter* pNewB = ( *iUP ) ->Clone();
		( *iUP ) ->Subdivide( pNewA, pNewB, TqTrue, this );
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
	for ( iUP = m_aUserParams.begin(); iUP != m_aUserParams.end(); iUP++ )
	{
		CqParameter* pNewA = ( *iUP ) ->Clone();
		CqParameter* pNewB = ( *iUP ) ->Clone();
		( *iUP ) ->Subdivide( pNewA, pNewB, TqFalse, this );
		pA->AddPrimitiveVariable( pNewA );
		pB->AddPrimitiveVariable( pNewB );
	}
}


//---------------------------------------------------------------------
/** Transform the surface by the specified matrix.
 */

void	CqSurface::Transform( const CqMatrix& matTx, const CqMatrix& matITTx, const CqMatrix& matRTx )
{
	// Tansform the control hull by the specified matrix.
	std::vector<CqParameter*>::iterator iUP;
	for ( iUP = m_aUserParams.begin(); iUP != m_aUserParams.end(); iUP++ )
	{
		TqInt i;

		if ( ( *iUP ) ->Type() == type_point )
		{
			CqParameterTyped<CqVector3D, CqVector3D>* pTPV = static_cast<CqParameterTyped<CqVector3D, CqVector3D>*>( ( *iUP ) );
			for ( i = 0; i < ( *iUP ) ->Size(); i++ )
				pTPV->pValue() [ i ] = matTx * pTPV->pValue() [ i ];
		}
		else if ( ( *iUP ) ->Type() == type_normal )
		{
			CqParameterTyped<CqVector3D, CqVector3D>* pTPV = static_cast<CqParameterTyped<CqVector3D, CqVector3D>*>( ( *iUP ) );
			for ( i = 0; i < ( *iUP ) ->Size(); i++ )
				pTPV->pValue() [ i ] = matITTx * pTPV->pValue() [ i ];
		}
		if ( ( *iUP ) ->Type() == type_vector )
		{
			CqParameterTyped<CqVector3D, CqVector3D>* pTPV = static_cast<CqParameterTyped<CqVector3D, CqVector3D>*>( ( *iUP ) );
			for ( i = 0; i < ( *iUP ) ->Size(); i++ )
				pTPV->pValue() [ i ] = matRTx * pTPV->pValue() [ i ];
		}
		if ( ( *iUP ) ->Type() == type_hpoint )
		{
			CqParameterTyped<CqVector4D, CqVector3D>* pTPV = static_cast<CqParameterTyped<CqVector4D, CqVector3D>*>( ( *iUP ) );
			for ( i = 0; i < ( *iUP ) ->Size(); i++ )
				pTPV->pValue() [ i ] = matTx * pTPV->pValue() [ i ];
		}
	}
}

//---------------------------------------------------------------------

END_NAMESPACE( Aqsis )


