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
		\brief Implements the CqAttributes class for handling RenderMan attributes.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#include	"attributes.h"
#include	"renderer.h"
#include	"shaders.h"
#include	"trimcurve.h"
#include	"lights.h"

namespace Aqsis {


std::list<CqAttributes*>	Attribute_stack;


const TqUlong CqAttributes::CqHashTable::tableSize = 127;


/** A macro to take care of adding a system attribute given a name.
 *  Creates a new CqParameter derived class, initialises it to the given default value and 
 *  adds it to the default attributes member.
 */
#define	ADD_SYSTEM_ATTR(name, type, sltype, id, def) \
	CqParameterTypedUniform<type,id, sltype>* p##name = new CqParameterTypedUniform<type,id, sltype>(#name); \
	p##name->pValue()[0] = ( def ); \
	pdefattrs->AddParameter(p##name);

/** A macro to take care of adding a system attribute given a name.
 *  Creates a new CqParameter derived class, initialises it to the given default value and 
 *  adds it to the default attributes member.
 */
#define	ADD_SYSTEM_ATTR2(name, type, sltype, id, def0, def1) \
	CqParameterTypedUniformArray<type,id, sltype>* p##name = new CqParameterTypedUniformArray<type,id, sltype>(#name,2); \
	p##name->pValue()[0] = ( def0 ); \
	p##name->pValue()[1] = ( def1 ); \
	pdefattrs->AddParameter(p##name);

/** A macro to take care of adding a system attribute given a name.
 *  Creates a new CqParameter derived class, initialises it to the given default value and 
 *  adds it to the default attributes member.
 */
#define	ADD_SYSTEM_ATTR4(name, type, sltype, id, def0, def1, def2, def3) \
	CqParameterTypedUniformArray<type,id, sltype>* p##name = new CqParameterTypedUniformArray<type,id, sltype>(#name,4); \
	p##name->pValue()[0] = ( def0 ); \
	p##name->pValue()[1] = ( def1 ); \
	p##name->pValue()[2] = ( def2 ); \
	p##name->pValue()[3] = ( def3 ); \
	pdefattrs->AddParameter(p##name);

/** A macro to take care of adding a system attribute given a name.
 *  Creates a new CqParameter derived class, initialises it to the given default value and 
 *  adds it to the default attributes member.
 */
#define	ADD_SYSTEM_ATTR6(name, type, sltype, id, def0, def1, def2, def3, def4, def5) \
	CqParameterTypedUniformArray<type,id, sltype>* p##name = new CqParameterTypedUniformArray<type,id, sltype>(#name,8); \
	p##name->pValue()[0] = ( def0 ); \
	p##name->pValue()[1] = ( def1 ); \
	p##name->pValue()[2] = ( def2 ); \
	p##name->pValue()[3] = ( def3 ); \
	p##name->pValue()[4] = ( def4 ); \
	p##name->pValue()[5] = ( def5 ); \
	pdefattrs->AddParameter(p##name);


/** A macro to take care of adding a system attribute given a name.
 *  Creates a new CqParameter derived class, initialises it to the given default value and 
 *  adds it to the default attributes member.
 */
#define	ADD_SYSTEM_ATTR8(name, type, sltype, id, def0, def1, def2, def3, def4, def5, def6, def7) \
	CqParameterTypedUniformArray<type,id, sltype>* p##name = new CqParameterTypedUniformArray<type,id, sltype>(#name,8); \
	p##name->pValue()[0] = ( def0 ); \
	p##name->pValue()[1] = ( def1 ); \
	p##name->pValue()[2] = ( def2 ); \
	p##name->pValue()[3] = ( def3 ); \
	p##name->pValue()[4] = ( def4 ); \
	p##name->pValue()[5] = ( def5 ); \
	p##name->pValue()[6] = ( def6 ); \
	p##name->pValue()[7] = ( def7 ); \
	pdefattrs->AddParameter(p##name);

//---------------------------------------------------------------------
/** Constructor.
 */

CqAttributes::CqAttributes()
{
	Attribute_stack.push_front( this );
	m_StackIterator = Attribute_stack.begin();

	boost::shared_ptr<CqNamedParameterList> pdefattrs( new CqNamedParameterList( "System" ) );

	ADD_SYSTEM_ATTR( Color, CqColor, CqColor, type_color, CqColor( 1.0f, 1.0f, 1.0f ) );		// the current color attribute.
	ADD_SYSTEM_ATTR( Opacity, CqColor, CqColor, type_color, CqColor( 1.0f, 1.0f, 1.0f ) );	// the current opacity attribute.
	ADD_SYSTEM_ATTR8( TextureCoordinates, TqFloat, TqFloat, type_float, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f );	// an array of 2D vectors representing the coordinate space.
	ADD_SYSTEM_ATTR( ShadingRate, TqFloat, TqFloat, type_float, 1.0f );					// the current effective shading rate.
	ADD_SYSTEM_ATTR( ShadingInterpolation, TqInt, TqFloat, type_integer, ShadingInterp_Smooth );	// the current shading interpolation mode.
	ADD_SYSTEM_ATTR( GeometricFlatness, TqFloat, TqFloat, type_float, 1.0f );				// the current screen-space geometric flatness.
	ADD_SYSTEM_ATTR( GeometricFocusFactor, TqFloat, TqFloat, type_float, 1.0f );		// multiplier for CoC area to multiply the shading rate for depth of field
	ADD_SYSTEM_ATTR( GeometricMotionFactor, TqFloat, TqFloat, type_float, 1.0f );
	ADD_SYSTEM_ATTR( Matte, TqInt, TqFloat, type_integer, 0 );				// the current state of the matte flag.
	ADD_SYSTEM_ATTR2( Basis, CqMatrix, CqMatrix, type_matrix, RiBezierBasis, RiBezierBasis );	// the basis matrix for the u direction.
	ADD_SYSTEM_ATTR2( BasisStep, TqInt, TqFloat, type_integer, 3, 3 );	// the steps to advance the evaluation window in the u direction.
	ADD_SYSTEM_ATTR( Orientation, TqInt, TqFloat, type_integer, 0 );	// the orientation associated primitives are described in.
	ADD_SYSTEM_ATTR( Sides, TqInt, TqFloat, type_integer, 2 );		// the number of visible sides associated primitives have.
	ADD_SYSTEM_ATTR2( LevelOfDetailBounds, TqFloat, TqFloat, type_float, 0.0f, 1.0f );	// relative importance bounds for this LOD representation
	ADD_SYSTEM_ATTR6( LODBound, TqFloat, TqFloat, type_float, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f );	// object space bounds for level of detail calculation of surfaces in this attribute scope.
	ADD_SYSTEM_ATTR4( LODRanges, TqFloat, TqFloat, type_float, 0.0f, 0.0f, 0.0f, 0.0f );	// Range values minvisible, lowertransition, uppertransition, maxvisible
	AddAttribute( pdefattrs );
}


//---------------------------------------------------------------------
/** Copy constructor.
 */

CqAttributes::CqAttributes( const CqAttributes& From )
{
	*this = From;

	// Register ourself with the global attribute stack.
	Attribute_stack.push_front( this );
	m_StackIterator = Attribute_stack.begin();
}


//---------------------------------------------------------------------
/** Destructor.
 */

CqAttributes::~CqAttributes()
{
	// Unreference the system attributes.
	//	TqInt i = m_aAttributes.size();
	//	while ( i-- > 0 )
	//	{
	//		m_aAttributes[ i ] ->Release();
	//		m_aAttributes[ i ] = 0;
	//	}

	// Remove ourself from the stack
	Attribute_stack.erase( m_StackIterator );
}

//---------------------------------------------------------------------
/** Copy function.
 */

CqAttributes& CqAttributes::operator=( const CqAttributes& From )
{
	// Copy the system attributes.
	//	m_aAttributes.resize( From.m_aAttributes.size() );
	//	TqInt i = From.m_aAttributes.size();
	//	while ( i-- > 0 )
	//	{
	//		m_aAttributes[ i ] = From.m_aAttributes[ i ];
	//		m_aAttributes[ i ] ->AddRef();
	//	}
	m_aAttributes = From.m_aAttributes;

	m_apLightsources = From.m_apLightsources;

	m_pshadDisplacement = From.m_pshadDisplacement;
	m_pshadAreaLightSource = From.m_pshadAreaLightSource;
	m_pshadSurface = From.m_pshadSurface;
	m_pshadAtmosphere = From.m_pshadAtmosphere;
	m_pshadInteriorVolume = From.m_pshadInteriorVolume;
	m_pshadExteriorVolume = From.m_pshadExteriorVolume;

	return ( *this );
}


//---------------------------------------------------------------------
/** Get a system attribute parameter.
 * \param strName The name of the attribute.
 * \param strParam The name of the paramter on the attribute.
 * \return CqParameter pointer or 0 if not found.
 */

const CqParameter* CqAttributes::pParameter( const char* strName, const char* strParam ) const
{
	const CqNamedParameterList* pList = pAttribute( strName ).get();
	if ( pList )
	{
		return ( pList->pParameter( strParam ) );
	}
	return ( 0 );
}


//---------------------------------------------------------------------
/** Get a system attribute parameter.
 * \param strName The name of the attribute.
 * \param strParam The name of the paramter on the attribute.
 * \return CqParameter pointer or 0 if not found.
 */

CqParameter* CqAttributes::pParameterWrite( const char* strName, const char* strParam )
{
	CqNamedParameterList * pList = pAttributeWrite( strName ).get();
	if ( pList )
	{
		return (pList->pParameter( strParam ) );
	}
	return ( 0 );
}


//---------------------------------------------------------------------
/** Get a float system attribute parameter.
 * \param strName The name of the attribute.
 * \param strParam The name of the paramter on the attribute.
 * \return Float pointer 0 if not found.
 */

TqFloat* CqAttributes::GetFloatAttributeWrite( const char* strName, const char* strParam )
{
	CqParameter * pParam = pParameterWrite( strName, strParam );
	if ( pParam != 0 && pParam->Type() == type_float)
		return ( static_cast<CqParameterTyped<TqFloat, TqFloat>*>( pParam ) ->pValue() );
	else
		return ( 0 );
}


//---------------------------------------------------------------------
/** Get an integer system attribute parameter.
 * \param strName The name of the attribute.
 * \param strParam The name of the paramter on the attribute.
 * \return Integer pointer 0 if not found.
 */

TqInt* CqAttributes::GetIntegerAttributeWrite( const char* strName, const char* strParam )
{
	CqParameter * pParam = pParameterWrite( strName, strParam );
	if ( pParam != 0 && pParam->Type() == type_integer )
		return ( static_cast<CqParameterTyped<TqInt, TqFloat>*>( pParam ) ->pValue() );
	else
		return ( 0 );
}


//---------------------------------------------------------------------
/** Get a string system attribute parameter.
 * \param strName The name of the attribute.
 * \param strParam The name of the paramter on the attribute.
 * \return CqString pointer 0 if not found.
 */

CqString* CqAttributes::GetStringAttributeWrite( const char* strName, const char* strParam )
{
	CqParameter * pParam = pParameterWrite( strName, strParam );
	if ( pParam != 0 && pParam->Type() == type_string )
		return ( static_cast<CqParameterTyped<CqString, CqString>*>( pParam ) ->pValue() );
	else
		return ( 0 );
}


//---------------------------------------------------------------------
/** Get a point system attribute parameter.
 * \param strName The name of the attribute.
 * \param strParam The name of the paramter on the attribute.
 * \return CqVetor3D pointer 0 if not found.
 */

CqVector3D* CqAttributes::GetPointAttributeWrite( const char* strName, const char* strParam )
{
	CqParameter * pParam = pParameterWrite( strName, strParam );
	if ( pParam != 0 && pParam->Type() == type_point )
		return ( static_cast<CqParameterTyped<CqVector3D, CqVector3D>*>( pParam ) ->pValue() );
	else
		return ( 0 );
}


//---------------------------------------------------------------------
/** Get a vector system attribute parameter.
 * \param strName The name of the attribute.
 * \param strParam The name of the paramter on the attribute.
 * \return CqVetor3D pointer 0 if not found.
 */

CqVector3D* CqAttributes::GetVectorAttributeWrite( const char* strName, const char* strParam )
{
	CqParameter * pParam = pParameterWrite( strName, strParam );
	if ( pParam != 0 && pParam->Type() == type_vector )
		return ( static_cast<CqParameterTyped<CqVector3D, CqVector3D>*>( pParam ) ->pValue() );
	else
		return ( 0 );
}


//---------------------------------------------------------------------
/** Get a normal system attribute parameter.
 * \param strName The name of the attribute.
 * \param strParam The name of the paramter on the attribute.
 * \return CqVetor3D pointer 0 if not found.
 */

CqVector3D* CqAttributes::GetNormalAttributeWrite( const char* strName, const char* strParam )
{
	CqParameter * pParam = pParameterWrite( strName, strParam );
	if ( pParam != 0 && pParam->Type() == type_normal )
		return ( static_cast<CqParameterTyped<CqVector3D, CqVector3D>*>( pParam ) ->pValue() );
	else
		return ( 0 );
}


//---------------------------------------------------------------------
/** Get a color system attribute parameter.
 * \param strName The name of the attribute.
 * \param strParam The name of the paramter on the attribute.
 * \return CqColor pointer 0 if not found.
 */

CqColor* CqAttributes::GetColorAttributeWrite( const char* strName, const char* strParam )
{
	CqParameter * pParam = pParameterWrite( strName, strParam );
	if ( pParam != 0 && pParam->Type() == type_color )
		return ( static_cast<CqParameterTyped<CqColor, CqColor>*>( pParam ) ->pValue() );
	else
		return ( 0 );
}


//---------------------------------------------------------------------
/** Get a matrix system attribute parameter.
 * \param strName The name of the attribute.
 * \param strParam The name of the paramter on the attribute.
 * \return CqMatrix pointer 0 if not found.
 */

CqMatrix* CqAttributes::GetMatrixAttributeWrite( const char* strName, const char* strParam )
{
	CqParameter * pParam = pParameterWrite( strName, strParam );
	if ( pParam != 0 && pParam->Type() == type_matrix )
		return ( static_cast<CqParameterTyped<CqMatrix, CqMatrix>*>( pParam ) ->pValue() );
	else
		return ( 0 );
}


//---------------------------------------------------------------------
/** Get a float system attribute parameter.
 * \param strName The name of the attribute.
 * \param strParam The name of the paramter on the attribute.
 * \return Float pointer 0 if not found.
 */

const TqFloat* CqAttributes::GetFloatAttribute( const char* strName, const char* strParam ) const
{
	const CqParameter * pParam = pParameter( strName, strParam );
	if ( pParam != 0 && pParam->Type() == type_float )
		return ( static_cast<const CqParameterTyped<TqFloat, TqFloat>*>( pParam ) ->pValue() );
	else
		return ( 0 );
}


//---------------------------------------------------------------------
/** Get an integer system attribute parameter.
 * \param strName The name of the attribute.
 * \param strParam The name of the paramter on the attribute.
 * \return Integer pointer 0 if not found.
 */

const TqInt* CqAttributes::GetIntegerAttribute( const char* strName, const char* strParam ) const
{
	const CqParameter * pParam = pParameter( strName, strParam );
	if ( pParam != 0 && pParam->Type() == type_integer )
		return ( static_cast<const CqParameterTyped<TqInt, TqFloat>*>( pParam ) ->pValue() );
	else
		return ( 0 );
}

const TqInt CqAttributes::GetIntegerAttributeDef( const char* strName, const char* strParam, TqInt defaultVal ) const
{
	const TqInt* attr = GetIntegerAttribute(strName, strParam);
	if(attr)
		return *attr;
	return defaultVal;
}


//---------------------------------------------------------------------
/** Get a string system attribute parameter.
 * \param strName The name of the attribute.
 * \param strParam The name of the paramter on the attribute.
 * \return CqString pointer 0 if not found.
 */

const CqString* CqAttributes::GetStringAttribute( const char* strName, const char* strParam ) const
{
	const CqParameter * pParam = pParameter( strName, strParam );
	if ( pParam != 0 && pParam->Type() == type_string )
		return ( static_cast<const CqParameterTyped<CqString, CqString>*>( pParam ) ->pValue() );
	else
		return ( 0 );
}


//---------------------------------------------------------------------
/** Get a point system attribute parameter.
 * \param strName The name of the attribute.
 * \param strParam The name of the paramter on the attribute.
 * \return CqVector3D pointer 0 if not found.
 */

const CqVector3D* CqAttributes::GetPointAttribute( const char* strName, const char* strParam ) const
{
	const CqParameter * pParam = pParameter( strName, strParam );
	if ( pParam != 0 && pParam->Type() == type_point )
		return ( static_cast<const CqParameterTyped<CqVector3D, CqVector3D>*>( pParam ) ->pValue() );
	else
		return ( 0 );
}


//---------------------------------------------------------------------
/** Get a vector system attribute parameter.
 * \param strName The name of the attribute.
 * \param strParam The name of the paramter on the attribute.
 * \return CqVector3D pointer 0 if not found.
 */

const CqVector3D* CqAttributes::GetVectorAttribute( const char* strName, const char* strParam ) const
{
	const CqParameter * pParam = pParameter( strName, strParam );
	if ( pParam != 0 && pParam->Type() == type_vector )
		return ( static_cast<const CqParameterTyped<CqVector3D, CqVector3D>*>( pParam ) ->pValue() );
	else
		return ( 0 );
}


//---------------------------------------------------------------------
/** Get a normal system attribute parameter.
 * \param strName The name of the attribute.
 * \param strParam The name of the paramter on the attribute.
 * \return CqVector3D pointer 0 if not found.
 */

const CqVector3D* CqAttributes::GetNormalAttribute( const char* strName, const char* strParam ) const
{
	const CqParameter * pParam = pParameter( strName, strParam );
	if ( pParam != 0 && pParam->Type() == type_normal )
		return ( static_cast<const CqParameterTyped<CqVector3D, CqVector3D>*>( pParam ) ->pValue() );
	else
		return ( 0 );
}


//---------------------------------------------------------------------
/** Get a color system attribute parameter.
 * \param strName The name of the attribute.
 * \param strParam The name of the paramter on the attribute.
 * \return CqColor pointer 0 if not found.
 */

const CqColor* CqAttributes::GetColorAttribute( const char* strName, const char* strParam ) const
{
	const CqParameter * pParam = pParameter( strName, strParam );
	if ( pParam != 0 && pParam->Type() == type_color )
		return ( static_cast<const CqParameterTyped<CqColor, CqColor>*>( pParam ) ->pValue() );
	else
		return ( 0 );
}


//---------------------------------------------------------------------
/** Get a matrix system attribute parameter.
 * \param strName The name of the attribute.
 * \param strParam The name of the paramter on the attribute.
 * \return CqMatrix pointer 0 if not found.
 */

const CqMatrix* CqAttributes::GetMatrixAttribute( const char* strName, const char* strParam ) const
{
	const CqParameter * pParam = pParameter( strName, strParam );
	if ( pParam != 0 && pParam->Type() == type_matrix )
		return ( static_cast<const CqParameterTyped<CqMatrix, CqMatrix>*>( pParam ) ->pValue() );
	else
		return ( 0 );
}


IqLightsource*	CqAttributes::pLight( TqInt index ) const
{
	return ( boost::shared_ptr<CqLightsource>(m_apLightsources[index]).get() );
}

//---------------------------------------------------------------------

} // namespace Aqsis


