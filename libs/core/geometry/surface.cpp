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
		\brief Implements the base GPrim handling classes.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#include	<aqsis/aqsis.h>
#include	"renderer.h"
#include	"micropolygon.h"
#include	"surface.h"
#include	<aqsis/math/vector2d.h>
#include	"imagebuffer.h"

namespace Aqsis {

//TqFloat CqSurface::m_fGridSize = sqrt(256.0);


//---------------------------------------------------------------------
/** Copy the local surface parameters from the donor surface.
 */

void CqSurface::SetSurfaceParameters( const CqSurface& From )
{
	// Now store and reference our new attributes.
	m_pAttributes = From.m_pAttributes;

	m_pTransform = From.m_pTransform;

	m_pCSGNode = From.m_pCSGNode;
}


//---------------------------------------------------------------------
/** Return the name of this primitive surface if specified as a "identifier" "name" attribute,
 * otherwise return "not named"
 */

CqString CqSurface::strName() const
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

TqInt CqSurface::Uses() const
{
	TqInt Uses = gDefUses | QGetRenderContext()->pDDmanager()->Uses();
	boost::shared_ptr<IqShader> pshadSurface = pAttributes() ->pshadSurface(QGetRenderContextI()->Time());
	boost::shared_ptr<IqShader> pshadDisplacement = pAttributes() ->pshadDisplacement(QGetRenderContextI()->Time());
	boost::shared_ptr<IqShader> pshadAtmosphere = pAttributes() ->pshadAtmosphere(QGetRenderContextI()->Time());

	if ( !pshadSurface && !pshadDisplacement && !pshadAtmosphere )
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

void CqSurface::AdjustBoundForTransformationMotion( CqBound* B ) const
{
	// Create a map of transformation keyframes, taking into account both object and camera motion.
	TqInt iTime;
	IqConstTransformPtr objectTransform = pTransform();
	CqTransformPtr cameraTransform = QGetRenderContext()->GetCameraTransform();
	TqInt objectTimes = objectTransform->cTimes();
	TqInt cameraTimes = cameraTransform->cTimes();
	std::map<TqFloat, TqFloat> keyframeTimes;
	// Add all the object transformation times to the list of keyframe points.
	for(iTime = 0; iTime < objectTimes; iTime++)
		keyframeTimes[objectTransform->Time(iTime)] = objectTransform->Time(iTime);
	for(iTime = 0; iTime < cameraTimes; iTime++)
		keyframeTimes[cameraTransform->Time(iTime)] = cameraTransform->Time(iTime);

	if( keyframeTimes.size() > 1 )
	{
		CqMatrix matCameraToObject0;
		QGetRenderContext() ->matSpaceToSpace( "camera", "object", NULL, pTransform().get(), 0, matCameraToObject0 );
		CqBound B0;
		B0.vecMin() = B->vecMin();
		B0.vecMax() = B->vecMax();
		B0.Transform( matCameraToObject0 );

		std::map<TqFloat, TqFloat>::iterator keyFrame;
		for( keyFrame = keyframeTimes.begin(); keyFrame != keyframeTimes.end(); keyFrame++)
		{
			CqBound Btx( B0 );
			CqMatrix matObjectToCameraT;
			QGetRenderContext() ->matSpaceToSpace( "object", "camera", NULL, pTransform().get(), keyFrame->second, matObjectToCameraT );
			Btx.Transform( matObjectToCameraT );
			B->Encapsulate( &Btx );
		}
	}
}


//---------------------------------------------------------------------
/** Default constructor
 */

CqSurface::CqSurface()
	: m_fDiceable(true),
	m_fDiscard(false),
	m_SplitCount(0),
	m_aUserParams(),
	m_pAttributes(),
	m_pTransform(QGetRenderContext()->ptransCurrent()),
	m_uDiceSize(1),
	m_vDiceSize(1),
	m_SplitDir(SplitDir_U),
	m_CachedBound(false),
	m_Bound(),
	m_pCSGNode()
{
	// Set a refernce with the current attributes.
	m_pAttributes = QGetRenderContext() ->pattrCurrent();

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
			Aqsis::log() << warning << "Primitive \"" << objname.c_str() << "\" defined when not in 'Primitive' solid block" << std::endl;
		}
	}

	// Nullify the standard primitive variables index table.
	TqInt i;
	for ( i = 0; i < EnvVars_Last; i++ )
		m_aiStdPrimitiveVars[ i ] = -1;

	STATS_INC( GPR_allocated );
	STATS_INC( GPR_current );
	TqInt cGprim = STATS_GETI( GPR_current );
	TqInt cPeak = STATS_GETI( GPR_peak );
	STATS_SETI( GPR_peak, cGprim > cPeak ? cGprim : cPeak );
}


//---------------------------------------------------------------------
/** Clone the data on this CqSurface class onto the (possibly derived) clone
 *  passed in.
 */

void CqSurface::CloneData( CqSurface* clone ) const
{
	clone->m_fDiceable = m_fDiceable;
	clone->m_SplitCount = m_SplitCount;
	clone->m_fDiscard = m_fDiscard;

	clone->SetSurfaceParameters( *this );

	// Nullify the standard primitive variables index table.
	TqInt i;
	for ( i = 0; i < EnvVars_Last; i++ )
		clone->m_aiStdPrimitiveVars[ i ] = -1;

	clone->ClonePrimitiveVariables(*this);
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

void CqSurface::SetDefaultPrimitiveVariables( bool bUseDef_st )
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



namespace {

/** \brief Helper function for generic natural subdivision of surface parameters.
 *
 * Uses linear interpolation to split the parameters along either the u or v
 * directions.
 *
 * \param pParam - pointer to source parameter data
 * \param pResult1 - pointer to output parameter for first result curve
 * \param pResult1 - pointer to output parameter for second result curve
 * \param u - if true, split along the u-direction, otherwise split along v.
 */
template <class T, class SLT>
void surfaceNaturalSubdivide(CqParameter* pParam, CqParameter* pResult1,
		CqParameter* pResult2, bool u )
{
	CqParameterTyped<T, SLT>* pTParam = static_cast<CqParameterTyped<T, SLT>*>( pParam );
	CqParameterTyped<T, SLT>* pTResult1 = static_cast<CqParameterTyped<T, SLT>*>( pResult1 );
	CqParameterTyped<T, SLT>* pTResult2 = static_cast<CqParameterTyped<T, SLT>*>( pResult2 );

	for(TqInt i = 0; i < pParam->Count(); i++)
	{
		if ( u )
		{
			pTResult2->pValue( 1 ) [ i ] = pTParam->pValue( 1 ) [ i ];
			pTResult2->pValue( 3 ) [ i ] = pTParam->pValue( 3 ) [ i ];
			pTResult1->pValue( 1 ) [ i ] = pTResult2->pValue( 0 ) [ i ] = static_cast<T>( ( pTParam->pValue( 0 ) [ i ] + pTParam->pValue( 1 ) [ i ] ) * 0.5 );
			pTResult1->pValue( 3 ) [ i ] = pTResult2->pValue( 2 ) [ i ] = static_cast<T>( ( pTParam->pValue( 2 ) [ i ] + pTParam->pValue( 3 ) [ i ] ) * 0.5 );
		}
		else
		{
			pTResult2->pValue( 2 ) [ i ] = pTParam->pValue( 2 ) [ i ];
			pTResult2->pValue( 3 ) [ i ] = pTParam->pValue( 3 ) [ i ];
			pTResult1->pValue( 2 ) [ i ] = pTResult2->pValue( 0 ) [ i ] = static_cast<T>( ( pTParam->pValue( 0 ) [ i ] + pTParam->pValue( 2 ) [ i ] ) * 0.5 );
			pTResult1->pValue( 3 ) [ i ] = pTResult2->pValue( 1 ) [ i ] = static_cast<T>( ( pTParam->pValue( 1 ) [ i ] + pTParam->pValue( 3 ) [ i ] ) * 0.5 );
		}
	}
}

} // unnamed namespace

void CqSurface::NaturalSubdivide( CqParameter* pParam, CqParameter* pParam1, CqParameter* pParam2, bool u )
{
	switch(pParam->Type())
	{
		case type_float:
			surfaceNaturalSubdivide<TqFloat, TqFloat>(pParam, pParam1, pParam2, u);
			break;
		case type_integer:
			surfaceNaturalSubdivide<TqInt, TqFloat>(pParam, pParam1, pParam2, u);
			break;
		case type_point:
		case type_vector:
		case type_normal:
			surfaceNaturalSubdivide<CqVector3D, CqVector3D>(pParam, pParam1, pParam2, u);
			break;
		case type_hpoint:
			surfaceNaturalSubdivide<CqVector4D, CqVector3D>(pParam, pParam1, pParam2, u);
			break;
		case type_color:
			surfaceNaturalSubdivide<CqColor, CqColor>(pParam, pParam1, pParam2, u);
			break;
		case type_string:
			surfaceNaturalSubdivide<CqString, CqString>(pParam, pParam1, pParam2, u);
			break;
		case type_matrix:
			surfaceNaturalSubdivide<CqMatrix, CqMatrix>(pParam, pParam1, pParam2, u);
			break;
		default:
			// blank to avoid compiler warnings about unhandled cases
			break;
	}
}


namespace {

template <class T, class SLT>
void surfaceNaturalDice(TqFloat uSize, TqFloat vSize, CqParameter* pParam,
		IqShaderData* pData)
{
	CqParameterTyped<T, SLT>* pTParam = static_cast<CqParameterTyped<T, SLT>*>(pParam);
	TqInt iv, iu;
	for ( iv = 0; iv <= vSize; iv++ )
	{
		TqFloat v = ( 1.0f / vSize ) * iv;
		for ( iu = 0; iu <= uSize; iu++ )
		{
			TqFloat u = ( 1.0f / uSize ) * iu;
			IqShaderData* arrayValue;
			TqInt i;
			for(i = 0; i<pTParam->Count(); i++)
			{
				arrayValue = pData->ArrayEntry(i);
				T vec = BilinearEvaluate( pTParam->pValue(0) [ i ], pTParam->pValue(1) [ i ], pTParam->pValue(2) [ i ], pTParam->pValue(3) [ i ], u, v );
				TqInt igrid = static_cast<TqInt>( ( iv * ( uSize + 1 ) ) + iu );
				arrayValue->SetValue( paramToShaderType<SLT, T>( vec ), igrid );
			}
		}
	}
}

} // unnamed namespace

void CqSurface::NaturalDice( CqParameter* pParam, TqInt uDiceSize,
		TqInt vDiceSize, IqShaderData* pData )
{
	switch(pParam->Type())
	{
		case type_float:
			surfaceNaturalDice<TqFloat, TqFloat>(uDiceSize, vDiceSize, pParam, pData);
			break;
		case type_integer:
			surfaceNaturalDice<TqInt, TqFloat>(uDiceSize, vDiceSize, pParam, pData);
			break;
		case type_point:
		case type_vector:
		case type_normal:
			surfaceNaturalDice<CqVector3D, CqVector3D>(uDiceSize, vDiceSize, pParam, pData);
			break;
		case type_hpoint:
			surfaceNaturalDice<CqVector4D, CqVector3D>(uDiceSize, vDiceSize, pParam, pData);
			break;
		case type_color:
			surfaceNaturalDice<CqColor, CqColor>(uDiceSize, vDiceSize, pParam, pData);
			break;
		case type_string:
			surfaceNaturalDice<CqString, CqString>(uDiceSize, vDiceSize, pParam, pData);
			break;
		case type_matrix:
			surfaceNaturalDice<CqMatrix, CqMatrix>(uDiceSize, vDiceSize, pParam, pData);
			break;
		default:
			// left blank to avoid compiler warnings about unhandled types
			break;
	}
}


//---------------------------------------------------------------------
/** Dice the patch into a mesh of micropolygons.
 */

CqMicroPolyGridBase* CqSurface::Dice()
{
	PreDice( m_uDiceSize, m_vDiceSize );

	// Create a new CqMicorPolyGrid for this patch
	CqMicroPolyGrid* pGrid = new CqMicroPolyGrid();
	pGrid->Initialise( m_uDiceSize, m_vDiceSize, shared_from_this() );

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
				if( pVar(varID)->Class() == class_vertex || pVar(varID)->Class() == class_facevertex )
					// "vertex" and "facevertex" need to be dealt with by the surface as they are diced using the
					// natural subdivision algorithms for that particular surface.
					NaturalDice( pVar(varID), m_uDiceSize, m_vDiceSize, pGrid->pVar(varID) );
				else
					// "varying" and "facevarying" are just bilinearly interpolated, so can be handled by the primitive variable.
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
		pGrid->SetbShadingNormals( true );

	if ( isDONE( lDone, EnvVars_Ng ) )
		pGrid->SetbGeometricNormals( true );

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


TqInt CqSurface::Split( std::vector<boost::shared_ptr<CqSurface> >& aSplits )
{
	TqInt cSplits = PreSubdivide( aSplits, m_SplitDir == SplitDir_U );

	assert( aSplits.size() == 2 );

	aSplits[ 0 ] ->SetSurfaceParameters( *this );
	aSplits[ 0 ] ->SetSplitDir( ( SplitDir() == SplitDir_U ) ? SplitDir_V : SplitDir_U );
	aSplits[ 0 ] ->SetSplitCount( SplitCount() + 1 );
	aSplits[ 0 ] ->m_fDiceable = true;
	//ADDREF( aSplits[ 0 ] );

	aSplits[ 1 ] ->SetSurfaceParameters( *this );
	aSplits[ 1 ] ->SetSplitDir( ( SplitDir() == SplitDir_U ) ? SplitDir_V : SplitDir_U );
	aSplits[ 1 ] ->SetSplitCount( SplitCount() + 1 );
	aSplits[ 1 ] ->m_fDiceable = true;
	//ADDREF( aSplits[ 1 ] );

	// Iterate through any use parameters subdividing and storing the second value in the target surface.
	std::vector<CqParameter*>::iterator iUP;
	std::vector<CqParameter*>::iterator end = m_aUserParams.end();
	bool direction = SplitDir() == SplitDir_U;

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
		std::vector<boost::shared_ptr<CqSurface> > aSplits0;
		std::vector<boost::shared_ptr<CqSurface> > aSplits1;

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
		( *iUP ) ->Subdivide( pNewA, pNewB, true, this );
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
		( *iUP ) ->Subdivide( pNewA, pNewB, false, this );
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


/// Transform the bound cuboid in into out via the transformation trans.
static void transformCuboid(CqVector3D out[8], const CqVector3D in[8],
							const CqMatrix& trans)
{
	for(int i = 0; i < 8; ++i)
		out[i] = trans*in[i];
}

TqFloat CqSurface::AdjustedShadingRate() const
{
	TqFloat shadingRate =
		m_pAttributes->GetFloatAttribute("System", "ShadingRate")[0];
	CqRenderer* context = QGetRenderContext();
	if(context->UsingDepthOfField())
	{
		// Adjust dice size with the CoC *area*.  This ensures that very blurry
		// parts of a scene only have relatively few micropolys and allows the
		// number of sample-in-micropolygon tests per pixel to be constant as
		// the image size is increased.
		//
		// If this isn't included then render time increases roughly
		// quadratically with number of pixels which makes things very slow.
		const TqFloat focusFactor =
			m_pAttributes->GetFloatAttribute("System", "GeometricFocusFactor")[0];
		const TqFloat minCoC = context->MinCoCForBound(m_Bound);

		// We need a factor which decides the desired ratio of the area of the
		// circle of confusion to the area of a micropolygon.  The factor
		// areaRatio = 0.025 was chosen by some experiments demanding that
		// using focusFactor = 1 yield results almost visually indistingushable
		// from focusFactor = 0.
		//
		// Two main experiments were used to get areaRatio:
		//   1) Randomly coloured micropolygons: with an input of ShadingRate = 1
		//      and focusFactor = 1, randomly coloured micropolys (Ci = random())
		//      should all blend together with no large regions of colour, even
		//      in image regions with lots of focal blur.
		//   2) A scene with multiple strong specular highlights (a bilinear
		//      patch with displacement shader
		//      P += 0.1*sin(40*v)*cos(20*u) * normalize(N); ):
		//      This should look indistingushable from the result obtained with
		//      focusFactor = 1, regardless of the amount of focal blur.
		//
		const TqFloat areaRatio = 0.025;
		shadingRate *= max(1.0f, areaRatio*focusFactor*minCoC*minCoC);
	}

	// Adjust shadingRate based on motionfactor

	//get motionfactor variable from rib, camera transform
	const TqFloat* motionFactor = m_pAttributes->GetFloatAttribute("System", "GeometricMotionFactor");
	TqFloat motionFac = motionFactor[0];
	CqTransformPtr cameraTransform = context->GetCameraTransform();

	if (motionFac > 0.0 && (isMoving() || cameraTransform->isMoving() ) )
	{
		// get the exposure-time (Time of shutter close - Time of shutter open)
		const TqFloat* shutterTimes = context->GetFloatOption( "System", "Shutter" );
		assert(shutterTimes);
		TqFloat exposureTime = shutterTimes[1] - shutterTimes[0];

		// get object transform, and keyframe time values
		IqConstTransformPtr objectTransform = pTransform();
		TqInt objectTimes = objectTransform->cTimes();
		TqInt cameraTimes = cameraTransform->cTimes();
		std::vector<TqFloat>keyframeTimes;

		// store the object+camera keyframe time values in one vector
		TqInt iTime;
		for(iTime = 0; iTime < objectTimes; iTime++)
			keyframeTimes.push_back(objectTransform->Time(iTime));
		for (iTime = 0; iTime < cameraTimes; iTime++)
			keyframeTimes.push_back(cameraTransform->Time(iTime));

		// sort and get iterator over unique time-values
		std::sort(keyframeTimes.begin(), keyframeTimes.end());
		std::vector<TqFloat>::iterator timesIter = std::unique(keyframeTimes.begin(), keyframeTimes.end());
		keyframeTimes.resize(timesIter - keyframeTimes.begin());

		// only proceed if object is moving
		if (keyframeTimes.size() > 1)
		{
			// Get the bound for the object, in object space.  TODO: This is
			// potentially inefficient, because the bound as calculated in
			// PostSurface() is calculated again.  However, the cached m_Bound
			// is the bound in hybrid camera/raster space which isn't very
			// helpful for us.
			//
			// TODO: avoid using the expanded bound which has been adjusted for
			// MB, as returned by the Bound() function.
			CqBound bound;
			Bound(&bound);
			CqVector3D boundVerts[8];
			bound.getBoundCuboid(boundVerts);
			// Now get the cuboid corresponding to the bound (in camera space
			// at time = 0), and transform to object space.  In object space,
			// the bound is fixed in time by definition.
			CqMatrix camToObj;
			context->matSpaceToSpace("camera", "object", NULL, objectTransform.get(),
									 0, camToObj);
			transformCuboid(boundVerts, boundVerts, camToObj);

			// TODO: Here we've got to do a composite transform:
			//
			// object->camera (time t) * camera->raster (time 0)
			//
			// due to the fact that direct object->raster doesn't seem to pick
			// up the correct camera motion blur.  This should be fixed by
			// fixing matSpaceToSpace() properly.
			CqMatrix camToRast;
			context->matSpaceToSpace("camera", "raster", NULL, objectTransform.get(),
									 0, camToRast);

			TqFloat minSpeed = FLT_MAX;
			CqVector3D prevVerts[8];
			CqVector3D currVerts[8];

			// Get raster space bound at start of first motion segment.
			CqMatrix objToCam;
			context->matSpaceToSpace("object", "camera", NULL,
					objectTransform.get(), keyframeTimes[0], objToCam);
			transformCuboid(prevVerts, boundVerts, camToRast*objToCam);

			for (int t = 1; t < static_cast<int>(keyframeTimes.size()); t++)
			{
				// get cuboid at time t
				context->matSpaceToSpace("object", "camera", NULL,
						pTransform().get(), keyframeTimes[t], objToCam);
				transformCuboid(currVerts, boundVerts, camToRast*objToCam);

				// take the differences between verts of cuboids at the current
				// and previous times, and find the min distance^2
				TqFloat minDist = FLT_MAX;
				for (int i = 0; i < 8; i++)
				{
					TqFloat dist = (currVerts[i] - prevVerts[i]).Magnitude2();
					if (dist < minDist)
						minDist = dist;
				}

				// calculate speed of primitive, keep track of min-speed
				float speed = std::sqrt(minDist) / (keyframeTimes[t] - keyframeTimes[t-1]);
				if (speed < minSpeed)
					minSpeed = speed;

				// update prevCuboid
				for(int i = 0; i < 8; ++i)
					prevVerts[i] = currVerts[i];
			}

			// TODO: find optimal scalingConst.  This one seems to work well,
			// as it produces about the level of quality as a non-motionfactor
			// optimized render which uses a ShadingRate (approx)= average of
			// the adjusted-shading-rates this algorithm produces.
			const TqFloat scalingConst = 0.0833;
			// The shading rate is adjusted according to the estimated distance
			// travelled by a micropolygon across the screen given by
			// minSpeed*exposureTime.
			TqFloat shadingRateAdj = max<float>(1.0, scalingConst * exposureTime * minSpeed * motionFac);

			shadingRate *= shadingRateAdj;
		}
	}

	return shadingRate;
}

//---------------------------------------------------------------------

} // namespace Aqsis


