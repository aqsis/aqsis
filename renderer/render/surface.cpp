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

TqFloat CqBasicSurface::m_fGridSize = sqrt(256.0);

//---------------------------------------------------------------------
/** Default constructor
 */

CqBasicSurface::CqBasicSurface() : m_fDiceable( TqTrue ), m_fDiscard( TqFalse ), m_EyeSplitCount( 0 ),
		m_pAttributes( 0 ), m_SplitDir( SplitDir_U )
{
	// Set a refernce with the current attributes.
	m_pAttributes = const_cast<CqAttributes*>( QGetRenderContext() ->pattrCurrent() );
	ADDREF( m_pAttributes );

	m_pTransform = QGetRenderContext() ->ptransCurrent();

	m_CachedBound = TqFalse;

	// If the current context is a solid node, and is a 'primitive', attatch this surface to the node.
	if ( QGetRenderContext() ->pconCurrent() ->isSolid() )
	{
		CqModeBlock * pSolid = QGetRenderContext() ->pconCurrent().get();
		if ( pSolid->pCSGNode() ->NodeType() == CqCSGTreeNode::CSGNodeType_Primitive )
		{
			m_pCSGNode = pSolid->pCSGNode();
		}
		else
		{
			CqString objname( "unnamed" );
			const CqString* pattrName = m_pAttributes->GetStringAttribute( "identifier", "name" );
			if ( pattrName != 0 )
				objname = pattrName[ 0 ];
			std::cerr << warning << "Primitive \"" << objname.c_str() << "\" defined when not in 'Primitive' solid block" << std::endl;
		}
	}

	STATS_INC( GPR_allocated );
	STATS_INC( GPR_current );
	TqInt cGprim = STATS_GETI( GPR_current );
	TqInt cPeak = STATS_GETI( GPR_peak );
	STATS_SETI( GPR_peak, cGprim > cPeak ? cGprim : cPeak );
}


//---------------------------------------------------------------------
/** Copy constructor
 */

CqBasicSurface::CqBasicSurface( const CqBasicSurface& From ) : m_fDiceable( TqTrue ), m_SplitDir( SplitDir_U )
{
	*this = From;

	// Set a reference with the donors attributes.
	m_pAttributes = From.m_pAttributes;
	ADDREF( m_pAttributes );

	m_pTransform = From.m_pTransform;

	m_CachedBound = From.m_CachedBound;
	m_Bound = From.m_Bound;

	STATS_INC( GPR_allocated );
	STATS_INC( GPR_current );
	TqInt cGprim = STATS_GETI( GPR_current );
	TqInt cPeak = STATS_GETI( GPR_peak );
	STATS_SETI( GPR_peak, cGprim > cPeak ? cGprim : cPeak );
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
	if ( m_pAttributes )
		RELEASEREF( m_pAttributes );

	// Now store and reference our new attributes.
	m_pAttributes = From.m_pAttributes;
	ADDREF( m_pAttributes );

	m_pTransform = From.m_pTransform;

	m_pCSGNode = From.m_pCSGNode;
}


//---------------------------------------------------------------------
/** Return the name of this primitive surface if specified as a "identifier" "name" attribute,
 * otherwise return "not named"
 */

CqString CqBasicSurface::strName() const
{
	const CqString * pattrLightName = pAttributes() ->GetStringAttribute( "identifier", "name" );
	CqString strName( "not named" );
	if ( pattrLightName != 0 )
		strName = pattrLightName[ 0 ];

	return ( strName );
}


//---------------------------------------------------------------------
/** Work out which standard shader variables this surface requires by looking at the shaders.
 */

TqInt CqBasicSurface::Uses() const
{
	TqInt Uses = gDefUses | QGetRenderContext()->pDDmanager()->Uses();
	boost::shared_ptr<IqShader> pshadSurface = pAttributes() ->pshadSurface(QGetRenderContextI()->Time());
	boost::shared_ptr<IqShader> pshadDisplacement = pAttributes() ->pshadDisplacement(QGetRenderContextI()->Time());
	boost::shared_ptr<IqShader> pshadAtmosphere = pAttributes() ->pshadAtmosphere(QGetRenderContextI()->Time());

	if ( pshadSurface && pshadDisplacement && pshadAtmosphere )
		return ( 0 );

	if ( pshadSurface )
		Uses |= pshadSurface->Uses();
	if ( pshadDisplacement )
		Uses |= pshadDisplacement->Uses();
	if ( pshadAtmosphere )
		Uses |= pshadAtmosphere->Uses();

	// Just a quick check, if it uses dPdu/dPdv must also use du/dv
	if ( USES( Uses, EnvVars_dPdu ) )
		Uses |= ( 1 << EnvVars_du );
	if ( USES( Uses, EnvVars_dPdv ) )
		Uses |= ( 1 << EnvVars_dv );
	// Just a quick check, if it uses du/dv must also use u/v
	if ( USES( Uses, EnvVars_du ) )
		Uses |= ( 1 << EnvVars_u );
	if ( USES( Uses, EnvVars_dv ) )
		Uses |= ( 1 << EnvVars_v );

	return ( Uses );
}


//---------------------------------------------------------------------
/** Adjust the bound of the quadric taking into account transformation motion blur.
 */

CqBound	CqBasicSurface::AdjustBoundForTransformationMotion( const CqBound& B ) const
{
	CqBound Bm( B );

	if( pTransform()->cTimes() > 1 )
	{
		CqMatrix matCameraToObject0 = QGetRenderContext() ->matSpaceToSpace( "camera", "object", CqMatrix(), pTransform()->matObjectToWorld( pTransform()->Time( 0 ) ), pTransform()->Time( 0 ) );
		CqBound B0( B );
		B0.Transform( matCameraToObject0 );

		TqInt i;
		for( i = 1; i < pTransform()->cTimes(); i++ )
		{
			CqBound Btx( B0 );
			CqMatrix matObjectToCameraT = QGetRenderContext() ->matSpaceToSpace( "object", "camera", CqMatrix(), pTransform()->matObjectToWorld( pTransform()->Time( i ) ), pTransform()->Time( i ) );
			Btx.Transform( matObjectToCameraT );
			Bm.Encapsulate( Btx );
		}
	}

	return ( Bm );
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
	std::vector<CqParameter*>::const_iterator end = From.m_aUserParams.end() ;
	for ( iUP = From.m_aUserParams.begin(); iUP != end; iUP++ )
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
	if ( USES( bUses, EnvVars_s ) && bUseDef_st && !bHasVar(EnvVars_s) )
	{
		AddPrimitiveVariable( new CqParameterTypedVarying<TqFloat, type_float, TqFloat>( "s" ) );
		s() ->SetSize( 4 );
		TqInt i;
		for ( i = 0; i < 4; i++ )
			s() ->pValue() [ i ] = m_pAttributes->GetFloatAttribute( "System", "TextureCoordinates" ) [ i * 2 ];
	}

	if ( USES( bUses, EnvVars_t ) && bUseDef_st && !bHasVar(EnvVars_t))
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

			default:
			{
				// blank to avoid compiler warnings about unhandled cases
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

			default:
			{
				// left blank to avoid compiler warnings about unhandled types
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
	CqMicroPolyGrid* pGrid = new CqMicroPolyGrid( m_uDiceSize, m_vDiceSize, shared_from_this() );

	TqInt lUses = Uses();

	// Allow the surface to fill in as much as possible on the grid in one go for speed.
	TqInt lDone = DiceAll( pGrid );

	// Dice the primitive variables.

	// Special cases for s and t if "st" exists, it should override s and t.
	CqParameter* pParam;
	if( ( pParam = FindUserParam("st") ) != NULL )
	{
		if ( !isDONE( lDone, EnvVars_s ) && USES( lUses, EnvVars_s ) && ( NULL != pGrid->pVar(EnvVars_s) ) )
			pParam ->DiceOne( m_uDiceSize, m_vDiceSize, pGrid->pVar(EnvVars_s), this, 0 );
		if ( !isDONE( lDone, EnvVars_t ) && USES( lUses, EnvVars_t ) && ( NULL != pGrid->pVar(EnvVars_t) ) )
			pParam ->DiceOne( m_uDiceSize, m_vDiceSize, pGrid->pVar(EnvVars_t), this, 1 );
		DONE( lDone, EnvVars_s);
		DONE( lDone, EnvVars_t);
	}

	// Loop over all the variables checking if they have been specified in the scene, and
	// if they are needed by the shaders, and id the grid can accept them.
	// If all the tests pass, dice them into the grid based on their type.
	TqInt varID;
	for( varID = EnvVars_Cs; varID != EnvVars_Last; varID++ )
	{
		if ( !isDONE( lDone, varID ) && USES( lUses, varID ) && ( NULL != pGrid->pVar(varID) ) )
		{
			// Check if Cs has been specified by the user.
			if ( bHasVar(varID) )
			{
				if( pVar(varID)->Class() == class_vertex || pVar(varID)->Class() == class_facevarying )
					// "vertex" and "facevarying" need to be dealt with by the surface as they are diced using the
					// natural subdivision algorithms for that particular surface.
					NaturalDice( pVar(varID), m_uDiceSize, m_vDiceSize, pGrid->pVar(varID) );
				else
					// "varying" are just bilinearly interpolated, so can be handled by the primitive variable.
					pVar(varID) ->Dice( m_uDiceSize, m_vDiceSize, pGrid->pVar(varID), this );

				// Mark this as done, so that the special case default handlers later don't need to worry about it.
				DONE(lDone, varID);
			}
		}
	}

	// Special case handlers for primitive variables that have defaults.
	if ( !isDONE( lDone, EnvVars_Cs ) && USES( lUses, EnvVars_Cs ) && ( NULL != pGrid->pVar(EnvVars_Cs) ) )
	{
		if ( NULL != pAttributes() ->GetColorAttribute( "System", "Color" ) )
			pGrid->pVar(EnvVars_Cs) ->SetColor( pAttributes() ->GetColorAttribute( "System", "Color" ) [ 0 ] );
		else
			pGrid->pVar(EnvVars_Cs) ->SetColor( CqColor( 1, 1, 1 ) );
	}

	if ( !isDONE( lDone, EnvVars_Os ) && USES( lUses, EnvVars_Os ) && ( NULL != pGrid->pVar(EnvVars_Os) ) )
	{
		if ( NULL != pAttributes() ->GetColorAttribute( "System", "Opacity" ) )
			pGrid->pVar(EnvVars_Os) ->SetColor( pAttributes() ->GetColorAttribute( "System", "Opacity" ) [ 0 ] );
		else
			pGrid->pVar(EnvVars_Os) ->SetColor( CqColor( 1, 1, 1 ) );
	}

	// If the shaders need N and they have been explicitly specified, then bilinearly interpolate them.
	if ( isDONE( lDone, EnvVars_N ) )
		pGrid->SetbShadingNormals( TqTrue );

	if ( !isDONE( lDone, EnvVars_Ng ) && CanGenerateNormals() && USES( lUses, EnvVars_Ng ) )
	{
		GenerateGeometricNormals( m_uDiceSize, m_vDiceSize, pGrid->pVar(EnvVars_Ng) );
		pGrid->SetbGeometricNormals( TqTrue );
	}

	// Now we need to dice the user specified parameters as appropriate.
	std::vector<CqParameter*>::iterator iUP;
	std::vector<CqParameter*>::iterator end = m_aUserParams.end();
	for ( iUP = m_aUserParams.begin(); iUP != end ; iUP++ )
	{
		boost::shared_ptr<IqShader> pShader;
		if ( pShader=pGrid->pAttributes() ->pshadSurface(QGetRenderContext()->Time()) )
			pShader->SetArgument( ( *iUP ), this );

		if ( pShader=pGrid->pAttributes() ->pshadDisplacement(QGetRenderContext()->Time()) )
			pShader->SetArgument( ( *iUP ), this );

		if ( pShader=pGrid->pAttributes() ->pshadAtmosphere(QGetRenderContext()->Time()) )
			pShader->SetArgument( ( *iUP ), this );
	}

	PostDice( pGrid );

	return ( pGrid );
}


TqInt CqSurface::Split( std::vector<boost::shared_ptr<CqBasicSurface> >& aSplits )
{
	TqInt cSplits = PreSubdivide( aSplits, m_SplitDir == SplitDir_U );

	assert( aSplits.size() == 2 );

	aSplits[ 0 ] ->SetSurfaceParameters( *this );
	aSplits[ 0 ] ->SetSplitDir( ( SplitDir() == SplitDir_U ) ? SplitDir_V : SplitDir_U );
	aSplits[ 0 ] ->SetEyeSplitCount( EyeSplitCount() );
	aSplits[ 0 ] ->m_fDiceable = TqTrue;
	//ADDREF( aSplits[ 0 ] );

	aSplits[ 1 ] ->SetSurfaceParameters( *this );
	aSplits[ 1 ] ->SetSplitDir( ( SplitDir() == SplitDir_U ) ? SplitDir_V : SplitDir_U );
	aSplits[ 1 ] ->SetEyeSplitCount( EyeSplitCount() );
	aSplits[ 1 ] ->m_fDiceable = TqTrue;
	//ADDREF( aSplits[ 1 ] );

	// Iterate through any use parameters subdividing and storing the second value in the target surface.
	std::vector<CqParameter*>::iterator iUP;
	std::vector<CqParameter*>::iterator end = m_aUserParams.end();
	TqBool direction = SplitDir() == SplitDir_U;

	for ( iUP = m_aUserParams.begin(); iUP != end; iUP++ )
	{
		CqParameter* pNewA = ( *iUP ) ->Clone();
		CqParameter* pNewB = ( *iUP ) ->Clone();
		( *iUP ) ->Subdivide( pNewA, pNewB, direction , this );
		static_cast<CqSurface*>( aSplits[ 0 ].get() ) ->AddPrimitiveVariable( pNewA );
		static_cast<CqSurface*>( aSplits[ 1 ].get() ) ->AddPrimitiveVariable( pNewB );
	}

	if ( !m_fDiceable )
	{
		std::vector<boost::shared_ptr<CqBasicSurface> > aSplits0;
		std::vector<boost::shared_ptr<CqBasicSurface> > aSplits1;

		cSplits = aSplits[ 0 ] ->Split( aSplits0 );
		cSplits += aSplits[ 1 ] ->Split( aSplits1 );

		aSplits.clear();
		aSplits.swap( aSplits0 );
		aSplits.insert( aSplits.end(), aSplits1.begin(), aSplits1.end() );
	}

	PostSubdivide( aSplits );

	return ( aSplits.size() );
}


//---------------------------------------------------------------------
/** uSubdivide any user defined parameter variables.
 */

void CqSurface::uSubdivideUserParameters( CqSurface* pA, CqSurface* pB )
{
	// Iterate through any use parameters subdividing and storing the second value in the target surface.
	std::vector<CqParameter*>::iterator iUP;
	std::vector<CqParameter*>::iterator end = m_aUserParams.end();
	for ( iUP = m_aUserParams.begin(); iUP != end; iUP++ )
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
	std::vector<CqParameter*>::iterator end = m_aUserParams.end();
	for ( iUP = m_aUserParams.begin(); iUP != end; iUP++ )
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

void	CqSurface::Transform( const CqMatrix& matTx, const CqMatrix& matITTx, const CqMatrix& matRTx, TqInt iTime )
{
	// Tansform the control hull by the specified matrix.
	std::vector<CqParameter*>::iterator iUP;
	std::vector<CqParameter*>::iterator end = m_aUserParams.end();
	for ( iUP = m_aUserParams.begin(); iUP != end; iUP++ )
	{
		TqInt i;

		if ( ( *iUP ) ->Type() == type_point )
		{
			CqParameterTyped<CqVector3D, CqVector3D>* pTPV = static_cast<CqParameterTyped<CqVector3D, CqVector3D>*>( ( *iUP ) );
			TqInt size = ( *iUP ) ->Size();
			for ( i = 0; i < size; i++ )
				pTPV->pValue() [ i ] = matTx * pTPV->pValue() [ i ];
		}
		else if ( ( *iUP ) ->Type() == type_normal )
		{
			CqParameterTyped<CqVector3D, CqVector3D>* pTPV = static_cast<CqParameterTyped<CqVector3D, CqVector3D>*>( ( *iUP ) );
			TqInt size = ( *iUP ) ->Size();
			for ( i = 0; i < size; i++ )
				pTPV->pValue() [ i ] = matITTx * pTPV->pValue() [ i ];
		}
		if ( ( *iUP ) ->Type() == type_vector )
		{
			CqParameterTyped<CqVector3D, CqVector3D>* pTPV = static_cast<CqParameterTyped<CqVector3D, CqVector3D>*>( ( *iUP ) );
			TqInt size = ( *iUP ) ->Size();
			for ( i = 0; i < size; i++ )
				pTPV->pValue() [ i ] = matRTx * pTPV->pValue() [ i ];
		}
		if ( ( *iUP ) ->Type() == type_hpoint )
		{
			CqParameterTyped<CqVector4D, CqVector3D>* pTPV = static_cast<CqParameterTyped<CqVector4D, CqVector3D>*>( ( *iUP ) );
			TqInt size = ( *iUP ) ->Size();
			for ( i = 0; i < size; i++ )
				pTPV->pValue() [ i ] = matTx * pTPV->pValue() [ i ];
		}
	}
}



/** Find out if a named user parameter exists on this surface.
 */
CqParameter* CqSurface::FindUserParam( const char* name ) const
{
	TqUlong strName = CqString::hash(name );
	std::vector<CqParameter*>::const_iterator iUP;
	std::vector<CqParameter*>::const_iterator end = m_aUserParams.end();
	for ( iUP = m_aUserParams.begin(); iUP != end ; iUP++ )
	{
		if( ( *iUP )->hash() == strName )
			return( *iUP );
	}
	return( NULL );
}


//---------------------------------------------------------------------

END_NAMESPACE( Aqsis )


