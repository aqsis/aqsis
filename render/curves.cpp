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
                handling RenderMan Curves primitives.
        \author Jonathan Merritt (j.merritt@pgrad.unimelb.edu.au)
*/
#if 0

#include <stdio.h>
#include <string.h>
#include "aqsis.h"
#include "imagebuffer.h"
#include "micropolygon.h"
#include "renderer.h"
#include "patch.h"
#include "vector2d.h"
#include "vector3d.h"
#include "curves.h"
START_NAMESPACE(Aqsis)



/**
 * CqCurve constructor.
 */
CqCurve::CqCurve() : CqSurface() {
        SetSplitDir(SplitDir_V);
        m_widthParamIndex = -1;
        m_constantwidthParamIndex = -1;
}



/**
 * CqCurve copy constructor.
 */
CqCurve::CqCurve(const CqCurve &from) : CqSurface() {
        (*this) = from;
}



/**
 * CqCurve destructor.
 */
CqCurve::~CqCurve() { }



/**
 * Adds a primitive variable to the list of user parameters.  This method
 * caches the indexes of the "width" and "constantwidth" parameters within
 * the array of user parameters for later access.
 *
 * @param pParam        Pointer to the parameter to add.
 */
void CqCurve::AddPrimitiveVariable(CqParameter* pParam) {
        
        // add the primitive variable using the superclass method
        CqSurface::AddPrimitiveVariable(pParam);
        
        // trap the indexes of "width" and "constantwidth" parameters
        if (pParam->strName() == "width")
        {
                assert(m_widthParamIndex == -1);
                m_widthParamIndex = m_aUserParams.size() - 1;
        }
        else if (pParam->strName() == "constantwidth")
        {
                assert(m_constantwidthParamIndex == -1);
                m_constantwidthParamIndex = m_aUserParams.size() - 1;
        }
        
}



/**
 * Calculates bounds for a CqCurve.
 *
 * NOTE: This method makes the same assumptions as 
 * CqSurfacePatchBicubic::Bound() does about the convex-hull property of the
 * curve.  This is fine most of the time, but the user can specify basis
 * matrices like Catmull-Rom, which are non-convex.
 *
 * FIXME: Handle non-convex hulls, or make sure that all hulls which reach
 * us are convex!
 *
 * @return CqBound object containing the bounds.
 */
CqBound CqCurve::Bound() const {
        
        // Get the boundary in camera space.
        CqVector3D vecA( FLT_MAX, FLT_MAX, FLT_MAX );
	CqVector3D vecB( -FLT_MAX, -FLT_MAX, -FLT_MAX );
        TqFloat maxCameraSpaceWidth = 0;
        TqInt nWidthParams = cVarying();
        for (TqInt i = 0; i < (*P()).Size(); i++ )
        {
                // expand the boundary if necessary to accomodate the
                //  current vertex
                CqVector3D vecV = (*P())[i];
                if ( vecV.x() < vecA.x() ) vecA.x( vecV.x() );
                if ( vecV.y() < vecA.y() ) vecA.y( vecV.y() );
                if ( vecV.x() > vecB.x() ) vecB.x( vecV.x() );
                if ( vecV.y() > vecB.y() ) vecB.y( vecV.y() );
                if ( vecV.z() < vecA.z() ) vecA.z( vecV.z() );
                if ( vecV.z() > vecB.z() ) vecB.z( vecV.z() );
                        
                // increase the maximum camera space width of the curve if 
                //  necessary
                if (i < nWidthParams)
                {
                        TqFloat camSpaceWidth = GetWidthInSpace("camera", i);
                        if (camSpaceWidth > maxCameraSpaceWidth)
                        {
                                maxCameraSpaceWidth = camSpaceWidth;
                        }
                }
                
        }
        
        // increase the size of the boundary by half the width of the 
        //  curve in camera space
        vecA -= (maxCameraSpaceWidth/2.0);
        vecB += (maxCameraSpaceWidth/2.0);

        // return the boundary
        CqBound	B;
        B.vecMin() = vecA;
        B.vecMax() = vecB;
        return ( B );

}



/**
 * CqCurve assignment operator.
 *
 * @param from  CqCurve to make this one equal to.
 *
 * @return Reference to (*this).
 */
CqCurve& CqCurve::operator=(const CqCurve& from) {
        CqSurface::operator=(from);
        return (*this);
}



/**
 * Returns the approximate "length" of a grid in raster space.
 *
 * @return Approximate grid length.
 */
TqFloat CqCurve::GetGridLength() const {
        
        const TqInt* poptGridSize = 
                QGetRenderContext()->optCurrent().GetIntegerOption( 
                        "limits", "gridsize" 
                );
        TqInt m_XBucketSize = 16;
        TqInt m_YBucketSize = 16;
        const TqInt* poptBucketSize = 
                QGetRenderContext()->optCurrent().GetIntegerOption( 
                        "limits", "bucketsize" 
                );
        if (poptBucketSize != 0)
        {
                m_XBucketSize = poptBucketSize[0];
                m_YBucketSize = poptBucketSize[1];
        }
        TqFloat ShadingRate = pAttributes()->GetFloatAttribute(
                "System", "ShadingRate"
        )[0];
        TqFloat gridsize;
        if (poptGridSize)
        {
                gridsize = static_cast<TqFloat>(poptGridSize[0]) / 
                        ShadingRate;
        }
        else
        {
                gridsize = m_XBucketSize * m_YBucketSize / ShadingRate;
        }
        return sqrt(gridsize);

}



/**
 * Returns the width of the curve at a specified index.  The index is an index
 * into the "width" parameter, if present.  If the "width" parameter was not
 * specified then the "constantwidth" parameter is used.  If "constantwidth"
 * was not specified either then the default width of 1.0 is used.  The width 
 * is returned as an object space dimension.
 *
 * @param index         Index at which to return the width.
 *
 * @return Width of the curve in object space.
 */
TqFloat CqCurve::GetWidth(TqInt index) const {
        
        // first try returning the width
        const CqParameterTypedVarying<TqFloat, type_float, TqFloat>* widthP =
                width();
        if (widthP != NULL) 
        {
                return (*widthP)[index];
        }
        
        // next, try constantwidth
        const CqParameterTypedConstant<TqFloat, type_float, TqFloat>* cwidthP =
                constantwidth();
        if (cwidthP != NULL)
        {
                printf ("Returning constantwidth of %f\n", *(*cwidthP).pValue());
                return *(*cwidthP).pValue();
        }
        
        // finally, just return the default
        return static_cast<TqFloat>(1.0);
        
}



/**
 * Returns the width of the curve in a certain space (NB: cannot be shader
 * space yet, since I don't look up shader shace!).
 *
 * See CqCurve::GetWidth().
 *
 * @param toSpace       Space in which to return the width (cannot be shader
 *                              space!).
 * @param index         Index at which to return the width.
 *
 * @return Width of the curve in toSpace space.
 */
TqFloat CqCurve::GetWidthInSpace(
        const char *toSpace, TqInt index
) const {
        
        // fetch the current width in object space
        TqFloat objSpaceWidth = GetWidth(index);
        
        // fetch the object->world transform for this CqCurve, and
        //  then arrange to translate it to the position of the point in
        //  question
        CqMatrix objToWorld = pTransform()->matObjectToWorld();
        objToWorld.Translate((*P())[index]);
        
        // To convert the widths to world space, what we do is create a
        //  vector in object space that represents the width of the
        //  curve, and transform this vector into the required space,
        //  using the modified object to world transform
        CqVector4D wv0 = CqVector4D(0,0,0,1);
        CqVector4D wv1 = CqVector4D(objSpaceWidth,0,0,1);
        CqMatrix objToCamera = QGetRenderContext()->matSpaceToSpace(
                "object", toSpace, CqMatrix(), objToWorld
        );
        wv0 = objToCamera * wv0;
        wv1 = objToCamera * wv1;
        printf("wv0 = [ %f %f %f ]\n", wv0.x(), wv0.y(), wv0.z());
        printf("wv1 = [ %f %f %f ]\n", wv1.x(), wv1.y(), wv1.z());
        CqVector2D lengthVector = CqVector3D(wv0 - wv1);

        return lengthVector.Magnitude();
        
}



/**
 * Populates the "width" parameter if it is not already present (ie supplied
 * by the user).  The "width" is populated either by the value of
 * "constantwidth" or by the default object-space width 1.0.
 */
void CqCurve::PopulateWidth() {
        
        // if the width parameter has been supplied then bail immediately
        if (width() != NULL)
                return;
        
        // otherwise, find the value to fill the width array with
        TqFloat widthvalue = 1.0;
        if (constantwidth() != NULL)
        {
                widthvalue = *(constantwidth()->pValue());
        }
        
        // create and fill in the width array
        CqParameterTypedVarying<TqFloat, type_float, TqFloat>* widthP = 
                new CqParameterTypedVarying<TqFloat, type_float, TqFloat>(
                        "width"
                );
        widthP->SetSize(cVarying());
        for (TqInt i=0; i < cVarying(); i++)
        {
                (*widthP)[i] = widthvalue;
        }
        
        // add the width array to the curve as a parameter
        AddPrimitiveVariable(widthP);
}



/**
 * Sets the default primitive variables.
 *
 * @param bUseDef_st
 */
void CqCurve::SetDefaultPrimitiveVariables(TqBool bUseDef_st)
{
	TqInt bUses = Uses();

	// Set default values for all of our parameters

	// s and t default to four values, if the particular surface type requires different it is up
	// to the surface to override or change this after the fact.
        /*
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
        */

	if ( USES( bUses, EnvVars_v ) )
	{
		AddPrimitiveVariable(new CqParameterTypedVarying<TqFloat, type_float, TqFloat>("v") );
		v()->SetSize( 2 );
		v()->pValue() [ 0 ] = 0.0;
                v()->pValue() [ 1 ] = 1.0;
	}
}



/**
 * CqLinearCurveSegment constructor.
 */
CqLinearCurveSegment::CqLinearCurveSegment() : CqCurve() { }



/**
 * CqLinearCurveSegment copy constructor.
 */
CqLinearCurveSegment::CqLinearCurveSegment(const CqLinearCurveSegment &from) : 
CqCurve() {
        (*this) = from;
}



/**
 * CqLinearCurveSegment destructor.
 */
CqLinearCurveSegment::~CqLinearCurveSegment() { }



/**
 * Assignment operator.
 *
 * @param from  CqLinearCurveSegment to make this one equal to.
 *
 * @return Reference to *this.
 */
CqLinearCurveSegment& CqLinearCurveSegment::operator=(
        const CqLinearCurveSegment& from
) {
        CqCurve::operator=(from);
        return (*this);
}



/**
 * Implements natural subdivision for this curve segment.
 *
 * @param pParam        Original parameter.
 * @param pParam1       First new parameter.
 * @param pParam2       Second new parameter.
 * @param u             true if the split is along u (should
 *                              always be false!)
 */
void CqLinearCurveSegment::NaturalSubdivide(
        CqParameter* pParam, 
        CqParameter* pParam1, CqParameter* pParam2, 
        TqBool u
) {
        
        assert(u == false);
	switch( pParam->Type() )
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
//			CqParameterTyped<CqMatrix, CqMatrix>* pTParam = static_cast<CqParameterTyped<CqMatrix, CqMatrix>*>( pParam );
//			CqParameterTyped<CqMatrix, CqMatrix>* pTResult1 = static_cast<CqParameterTyped<CqMatrix, CqMatrix>*>( pParam1 );
//			CqParameterTyped<CqMatrix, CqMatrix>* pTResult2 = static_cast<CqParameterTyped<CqMatrix, CqMatrix>*>( pParam2 );
//			TypedNaturalSubdivide( pTParam, pTResult1, pTResult2, u );
//			break;
		}
	}
        
}



/**
 * Called before the curve segment is subdivided by splitting.
 *
 * @param aSplits       Vector to contain the splitting results.
 * @param u             true if the curve should be split in u
 *                              (should always be false!)
 */
TqInt CqLinearCurveSegment::PreSubdivide(
        std::vector<CqBasicSurface*>& aSplits, TqBool u
) {
        
        assert(u == false);     // we can only split curves along v!
        
        aSplits.push_back(new CqLinearCurveSegment);
        aSplits.push_back(new CqLinearCurveSegment);
        
        return 2;
        
}



/**
 * Splits a CqLinearCurveSegment into either two smaller segments or a
 * patch.
 *
 * @param aSplits       Vector to store the split objects in.
 *
 * @return      The number of objects we've created.
 */
TqInt CqLinearCurveSegment::Split(std::vector<CqBasicSurface*>& aSplits) {
 
        // OK, here the CqSingleCurveLinear line has two options:
        //  1. split into two more lines
        //  2. turn into a bilinear patch for rendering
        // We don't want to go turning into a patch unless absolutely
        // necessary, since patches cost more.  We only want to become a patch
        // if the current curve is "best handled" as a patch.  For now, I'm
        // choosing to define that the curve is best handled as a patch under
        // one or both of the following two conditions:
        //  1. If the maximum width is a significant fraction of the length of 
        //      the line (width greater than 0.75 x length; ignoring normals).
        //  2. If the length of the line (ignoring the width; cos' it's
        //      covered by point 1) is such that it's likely a bilinear
        //      patch would be diced immediately if we created one (so that
        //      patches don't have to get split!).

        // find the length and maximum width of the line in raster space
	const CqMatrix &matCtoR = QGetRenderContext()->matSpaceToSpace( 
                "camera", "raster" 
        );
        (*P())[0].Homogenize();
        (*P())[1].Homogenize();
        CqVector2D hull[2];
        hull[0] = matCtoR * (*P())[0];
        hull[1] = matCtoR * (*P())[1];
        CqVector2D lengthVector = hull[1] - hull[0];
        TqFloat lengthraster = lengthVector.Magnitude();
        TqFloat width0 = GetWidthInSpace("raster", 0);
        TqFloat width1 = GetWidthInSpace("raster", 1);
        TqFloat maxwidthraster = (width0 > width1) ? width0 : width1;
        
        // find the approximate "length" of a diced patch in raster space
        TqFloat gridlength = GetGridLength();
        
        printf("(*P())[0]      = [ %f %f %f ]\n", (*P())[0].x(), (*P())[0].y(), (*P())[0].z());
        printf("(*P())[1]      = [ %f %f %f ]\n", (*P())[1].x(), (*P())[1].y(), (*P())[1].z());
        printf("(*v())[0]      = %f\n", (*v())[0]);
        printf("(*v())[1]      = %f\n", (*v())[1]);
        printf("maxwidthraster = %f\n", maxwidthraster);
        printf("lengthraster   = %f\n", lengthraster);
        printf("gridlength     = %f\n", gridlength);
        
        // decide whether to split into a more curve segments or a patch
        if (
                (maxwidthraster > (0.75 * lengthraster)) ||
                (gridlength >= lengthraster) ||
                (!m_fDiceable)
        ) 
        {
                printf("Splitting into a patch.\n");
                
                // split into a patch
                return SplitToPatch(aSplits);
        }
        else
        {
                
                printf("Splitting into two curves.\n");
                
                // split into more curves
                //  This bit right here looks a lot like CqSurface::Split().
                //  The difference is that we *don't* want the default splitter
                //  to handle varying class variables because it inconveniently
                //  sets them up to have 4 elements.
                
                TqInt cSplits = PreSubdivide(aSplits, false);

                assert( aSplits.size() == 2 );
        
                aSplits[0]->SetSurfaceParameters(*this);
                aSplits[0]->SetEyeSplitCount(EyeSplitCount());
                aSplits[0]->AddRef();

                aSplits[1]->SetSurfaceParameters(*this);
                aSplits[1]->SetEyeSplitCount(EyeSplitCount());
                aSplits[1]->AddRef();
        
                // Iterate through any user parameters, subdividing and storing
                //  the second value in the target surface.
                std::vector<CqParameter*>::iterator iUP;
                for (
                        iUP = m_aUserParams.begin(); 
                        iUP != m_aUserParams.end(); 
                        iUP++ 
                )
                {
                
                        // clone the parameters
                        CqParameter* pNewA = (*iUP)->Clone();
                        CqParameter* pNewB = (*iUP)->Clone();
                        
                        // let the standard system handle all but varying class
                        //  primitive variables
                        if ((*iUP)->Class() == class_varying) 
                        {
                                // for varying class variables, we want to 
                                //  handle them the same way as vertex class 
                                //  variables for the simple case of a 
                                //  CqSingleCurveLinear
                                NaturalSubdivide(
                                        (*iUP), pNewA, pNewB, TqFalse
                                );
                        } 
                        else 
                        {                                
                                (*iUP)->Subdivide(
                                        pNewA, pNewB, false, this 
                                );
                        }
                        
                        static_cast<CqSurface*>(aSplits[0])->
                                AddPrimitiveVariable(pNewA);
                        static_cast<CqSurface*>(aSplits[1])->
                                AddPrimitiveVariable(pNewB);
                
                }

                /*
                if (!m_fDiceable)
                {
                        std::vector<CqBasicSurface*> aSplits0;
                        std::vector<CqBasicSurface*> aSplits1;

                        cSplits = aSplits[0]->Split(aSplits0);
                        cSplits += aSplits[1]->Split(aSplits1);
                        // Release the old ones.
                        aSplits[0]->Release();
                        aSplits[1]->Release();

                        aSplits.clear();
                        aSplits.swap(aSplits0);
                        aSplits.insert( 
                                aSplits.end(), aSplits1.begin(), aSplits1.end()
                        );
                }
                */
        
                PostSubdivide();

                return(cSplits);
                
        }
        
        return -1;
        
}



/**
 * Converts a linear curve segment into a patch for rendering.
 *
 * @param aSplits       Vector of split surfaces to add the segment to.
 *
 * @return Number of create objects.
 */
TqInt CqLinearCurveSegment::SplitToPatch(
        std::vector<CqBasicSurface*>& aSplits
) {
        
        // first, we find the following vectors:
        //  direction     - from the first point to the second along the line 
        //                      segment
        //  normal0       - normal at the first point
        //  normal1       - normal at the second point
        //  widthOffset0  - offset to account for the width of the patch at 
        //                      the first point 
        //  widthOffset1  - offset to account for the width of the patch at
        //                      the second point
        // also, we find a value for userN, which indicates whether the user
        // specified the normals, or whether they're the default ones
        CqVector3D direction = (*P())[1] - (*P())[0];
        CqVector3D normal0, normal1;
        TqBool userN = GetNormal(0, normal0);  GetNormal(1, normal1);
        CqVector3D widthOffset0 = normal0 % direction;
        CqVector3D widthOffset1 = normal1 % direction;
        widthOffset0 *= 
                GetWidthInSpace("camera", 0) / widthOffset0.Magnitude() / 2.0;
        widthOffset1 *=
                GetWidthInSpace("camera", 1) / widthOffset1.Magnitude() / 2.0;
                
        // next, we create the bilinear patch
        CqSurfacePatchBilinear *pPatch = new CqSurfacePatchBilinear();
        pPatch->SetSurfaceParameters(*this);        
        pPatch->AddRef();
        pPatch->SetDefaultPrimitiveVariables();
        
        // set the points on the patch
        pPatch->AddPrimitiveVariable(
                new CqParameterTypedVertex<
                        CqVector4D, type_hpoint, CqVector3D
                >("P",0)
        );
        pPatch->P()->SetSize(4);
        (*pPatch->P())[0] = static_cast<CqVector3D>((*P())[0]) + widthOffset0;
        (*pPatch->P())[1] = static_cast<CqVector3D>((*P())[0]) - widthOffset0;
        (*pPatch->P())[2] = static_cast<CqVector3D>((*P())[1]) + widthOffset1;
        (*pPatch->P())[3] = static_cast<CqVector3D>((*P())[1]) - widthOffset1;

        // set u, v coordinates of the patch
        
        // add the patch to the split surfaces vector
        aSplits.push_back(pPatch);
        
        return 1;
}



/**
 * CqCubicCurveSegment constructor.
 */
CqCubicCurveSegment::CqCubicCurveSegment() : CqCurve() { }



/**
 * CqCubicCurveSegment copy constructor.
 */
CqCubicCurveSegment::CqCubicCurveSegment(const CqCubicCurveSegment &from) 
: CqCurve() {
        (*this) = from;
}



/**
 * CqCubicCurveSegment destructor.
 */
CqCubicCurveSegment::~CqCubicCurveSegment() { }



/**
 * Assignment operator.
 *
 * @param from  CqCubicCurveSegment to make this one equal to.
 *
 * @return Reference to *this.
 */
CqCubicCurveSegment& CqCubicCurveSegment::operator=(
        const CqCubicCurveSegment& from
) {
        CqCurve::operator=(from);
        return (*this);
}



/**
 * CqCurvesGroup constructor.
 */
CqCurvesGroup::CqCurvesGroup() : CqCurve() { }



/**
 * CqCurvesGroup copy constructor.
 */
CqCurvesGroup::CqCurvesGroup(const CqCurvesGroup& from) : CqCurve() {
        (*this) = from;
}



/**
 * CqCurvesGroup destructor.
 */
CqCurvesGroup::~CqCurvesGroup() { }



/**
 * Assignment operator.
 *
 * @param from  CqCurvesGroup to set this one equal to.
 */
CqCurvesGroup& CqCurvesGroup::operator=(const CqCurvesGroup& from) {
        CqCurve::operator=(from);
        return (*this);
}



/**
 * Constructor for a CqLinearCurvesGroup.
 *
 * @param ncurves       Number of curves in the group.
 * @param nvertices     Number of vertices per curve.
 * @param periodic      true if the curves in the group are periodic.
 */
CqLinearCurvesGroup::CqLinearCurvesGroup(
        RtInt ncurves, RtInt nvertices[], TqBool periodic
) : CqCurvesGroup(), m_ncurves(ncurves), m_periodic(periodic) {
        
        assert(nvertices != NULL);     
        
        // it makes no sense to have a periodic curve group with a segment
        //  that has only two vertices - check for this just in case
        //  because the cVarying equations don't work; also add up the total
        //  number of vertices
        m_nTotalVerts = 0;
        for (TqInt i=0; i < m_ncurves; i++) 
        {
                m_nTotalVerts += nvertices[i];
                if ((nvertices[i] <= 2) && m_periodic)
                {
                        CqBasicError(
                        0, Severity_Normal, 
                        "Encountered a periodic group of " \
                        "linear curves with at least one " \
                        "curve that has only two vertices."
                        );
                }
        }
        
        // copy the array of numbers of vertices
        m_nvertices = new RtInt[m_ncurves];
        memcpy(m_nvertices, nvertices, sizeof(RtInt)*m_ncurves);
        
}



/**
 * CqLinearCurvesGroup copy constructor.
 */
CqLinearCurvesGroup::CqLinearCurvesGroup(const CqLinearCurvesGroup &from)
: CqCurve() {
        (*this) = from;
}


/**
 * CqLinearCurvesGroup destructor.
 */
CqLinearCurvesGroup::~CqLinearCurvesGroup() { 
        delete m_nvertices;
}



/**
 * Returns the number of parameters of varying storage class that this curve
 * group has.
 *
 * @return      Number of varying parameters.
 */
TqUint CqLinearCurvesGroup::cVarying() const {
        
        // this formula for cVarying is a slight modification of the one found
        //  in section 5.5 of RiSpec ver. 3.2
        TqUint nsegments = 0;
        for (TqInt i=0; i < m_ncurves; i++) 
        {
                nsegments += m_nvertices[i];
        }
        if (m_periodic)
        {
                nsegments -= m_ncurves;
                return nsegments;
        }
        else
        {
                return nsegments-1;
        }
        
}



/**
 * Assignment operator.
 *
 * @param from  CqCubicCurveSegment to set this one equal to.
 *
 * @return Reference to *this.
 */
CqLinearCurvesGroup& CqLinearCurvesGroup::operator=(
        const CqLinearCurvesGroup& from
) {
        // base class assignment
        CqCurvesGroup::operator=(from);
        
        // copy members
        m_ncurves = from.m_ncurves;
        m_periodic = from.m_periodic;
        memcpy(m_nvertices, from.m_nvertices, sizeof(TqInt)*m_ncurves);
}



/**
 * Splits a CqLinearCurvesGroup object.
 *
 * The initial, naiive implementation here is immediately to split the group of
 * curves into CqLinearCurveSegment objects.  Perhaps a better way would be to
 * manage splitting of curve groups into other curve groups until they're of a
 * small enough size to become curve segments... ?
 *
 * @param aSplits       Vector of split objects.
 */
TqInt CqLinearCurvesGroup::Split(std::vector<CqBasicSurface*>& aSplits) {
        
        TqInt nSplits = 0;      // number of splits we've done
        
        // create each linear curve, filling in its variables as we go
        TqInt vertexI = 0;      // keeps track of the current vertex index
        TqInt uniformI = 0;     // keeps track of the uniform param index
        // we process all the curves in the group...
        for (TqInt curveI = 0; curveI < m_ncurves; curveI++)
        {
                // for each curve, we then process all its segments
                for (TqInt segI = 0; segI < m_nvertices[curveI]-1; segI++)
                {
                        
                        // create the new CqLinearCurveSegment for the current
                        //  curve segment
                        CqLinearCurveSegment *pSeg = 
                                new CqLinearCurveSegment();
                        pSeg->AddRef();
                        pSeg->SetSurfaceParameters(*this);
                        
                        // process user parameters
			std::vector<CqParameter*>::iterator iUP;
			for(
                                iUP = aUserParams().begin(); 
                                iUP != aUserParams().end(); 
                                iUP++
                        )
			{
                                if (
                                        ((*iUP)->Class() == class_varying) ||
                                        ((*iUP)->Class() == class_vertex)
                                )
                                {
                                        // copy "varying" or "vertex" class 
                                        //  variables
                                        CqParameter* pNewUP = 
                                        (*iUP)->CloneType(
                                                (*iUP)->strName().c_str(),
                                                (*iUP)->Count()
                                        );
                                        assert(
                                                pSeg->cVarying() == 
                                                pSeg->cVertex()
                                        );
                                        pNewUP->SetSize(pSeg->cVarying());
                                        
                                        pNewUP->SetValue((*iUP), 0, vertexI);
                                        pNewUP->SetValue((*iUP), 1, vertexI+1);
                                        pSeg->AddPrimitiveVariable(pNewUP);
                                        
                                }
                                else if ((*iUP)->Class() == class_uniform)
                                {
                                        // copy "uniform" class variables
                                        CqParameter* pNewUP = 
                                        (*iUP)->CloneType(
                                                (*iUP)->strName().c_str(),
                                                (*iUP)->Count()
                                        );
                                        pNewUP->SetSize(pSeg->cUniform());
                                        
                                        pNewUP->SetValue((*iUP), 0, uniformI);
                                        pSeg->AddPrimitiveVariable(pNewUP);
                                }
                                else if ((*iUP)->Class() == class_constant)
                                {
                                        // copy "constant" class variables
                                        CqParameter* pNewUP = 
                                        (*iUP)->CloneType(
                                                (*iUP)->strName().c_str(),
                                                (*iUP)->Count()
                                        );
                                        pNewUP->SetSize(1);
                                        
                                        pNewUP->SetValue((*iUP), 0, 0);
                                        pSeg->AddPrimitiveVariable(pNewUP);
                                } // if
                                
                        } // for each parameter
                        
                        ++vertexI;
                        ++uniformI;
                        aSplits.push_back(pSeg);
                        ++nSplits;
                        
                } // for each curve segment
                
                ++vertexI;
                
        } // for each curve
        
        return nSplits;
        
}



/**
 * Constructor for a CqCubicCurvesGroup.
 *
 * @param ncurves       Number of curves in the group.
 * @param nvertices     Number of vertices per curve.
 * @param periodic      true if curves in the group are periodic.
 */
CqCubicCurvesGroup::CqCubicCurvesGroup(
        RtInt ncurves, RtInt nvertices[], TqBool periodic
) : CqCurvesGroup(), m_ncurves(ncurves), m_periodic(periodic) {
        
        // add up the total number of vertices
        m_nTotalVerts = 0;
        for (TqInt i=0; i < ncurves; i++) 
        {
                m_nTotalVerts += nvertices[i];
        }
        
        // copy the array of numbers of vertices
        assert(nvertices != NULL);
        m_nvertices = new RtInt[m_ncurves];
        memcpy(m_nvertices, nvertices, sizeof(RtInt)*m_ncurves);
        
}



/**
 * CqCubicCurvesGroup copy constructor.
 */
CqCubicCurvesGroup::CqCubicCurvesGroup(const CqCubicCurvesGroup &from) :
CqCurve() {
        (*this) = from;
}



/**
 * CqCubicCurvesGroup destructor.
 */
CqCubicCurvesGroup::~CqCubicCurvesGroup() { 
        delete m_nvertices;
}



/**
 * Returns the number of parameters of varying storage class that this curve
 * group has.
 *
 * @return      Number of varying parameters.
 */
TqUint CqCubicCurvesGroup::cVarying() const {
        
        TqInt nsegments = 0;        
        TqInt vStep = 
                pAttributes()->GetIntegerAttribute("System", "BasisStep")[1];
        TqInt i;
        if (m_periodic)
        {
                for (i=0; i < m_ncurves; i++)
                {
                        nsegments += m_nvertices[i] / vStep;
                }
        }
        else
        {
                for (i=0; i < m_ncurves; i++)
                {
                        nsegments += (m_nvertices[i]-4) / vStep + 1;
                }
        }
        
        if (m_periodic)
        {
                return nsegments;
        }
        else
        {
                return nsegments + 1;
        }
        
}



/**
 * Assignment operator.
 *
 * @param from  CqCubicCurvesGroup to make this one equal to.
 */
CqCubicCurvesGroup& CqCubicCurvesGroup::operator=(
        const CqCubicCurvesGroup& from
) {
        // base class assignment
        CqCurvesGroup::operator=(from);
        
        // copy members
        m_ncurves = from.m_ncurves;
        m_periodic = from.m_periodic;
        memcpy(m_nvertices, from.m_nvertices, sizeof(TqInt)*m_ncurves);
}



END_NAMESPACE(Aqsis)

#endif
