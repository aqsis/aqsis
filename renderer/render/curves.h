// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
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

/**
        \file
        \brief Declares the classes and support structures for 
                handling RenderMan Curves primitives.
        \author Jonathan Merritt (j.merritt@pgrad.unimelb.edu.au)
*/

#ifndef CURVES_H_INCLUDED
#define CURVES_H_INCLUDED

#include        "aqsis.h"
#include        "matrix.h"
#include        "surface.h"
#include        "patch.h"

START_NAMESPACE( Aqsis )


/**
 * \class CqCurve
 * 
 * Abstract base class for all curve objects.  This class provides facilities
 * for accessing information common to all curves, such as width information.
 */
class CqCurve : public CqSurface
{
		//------------------------------------------------------ Public Methods
	public:
		CqCurve();
		virtual ~CqCurve();
		virtual void AddPrimitiveVariable( CqParameter* pParam );
		virtual	CqBound	Bound() const;
		virtual void SetDefaultPrimitiveVariables( bool bUseDef_st = true );
#ifdef _DEBUG

		CqString className() const
		{
			return CqString("CqCurve");
		}
#endif
		//--------------------------------------------------- Protected Methods
	protected:
		TqFloat GetGridLength() const;
		void PopulateWidth();
		//---------------------------------------------- Inlined Public Methods
	public:
		/** Returns a const reference to the "constantwidth" parameter, or
		 * NULL if the parameter is not present. */
		const CqParameterTypedConstant <
		TqFloat, type_float, TqFloat
		> * constantwidth() const
		{
			if ( m_constantwidthParamIndex >= 0 )
			{
				return static_cast <
				       const CqParameterTypedConstant <
				       TqFloat, type_float, TqFloat
				       > *
				       > ( m_aUserParams[ m_constantwidthParamIndex ] );
			}
			else
			{
				return ( NULL );
			}
		}
		/** Returns whether the curve is diceable - at the moment, no Curves
		 * are directly diceable since they're converted to patches just prior
		 * to rendering. */
		virtual bool Diceable()
		{
			// OK, here the CqCubicCurveSegment line has two options:
			//  1. split into two more lines
			//  2. turn into a bilinear patch for rendering
			// We don't want to go turning into a patch unless absolutely
			// necessary, since patches cost more.  We only want to become a patch
			// if the current curve is "best handled" as a patch.  For now, I'm
			// choosing to define that the curve is best handled as a patch under
			// one or more of the following two conditions:
			//  1. If the maximum width is a significant fraction of the length of
			//      the line (width greater than 0.75 x length; ignoring normals).
			//  2. If the length of the line (ignoring the width; cos' it's
			//      covered by point 1) is such that it's likely a bilinear
			//      patch would be diced immediately if we created one (so that
			//      patches don't have to get split!).
			//  3. If the curve crosses the eye plane (m_fDiceable == false).

			// find the length of the CqLinearCurveSegment line in raster space
			if( m_splitDecision == Split_Undecided )
			{
				// AGG - 31/07/04
				// well, if we follow the above statagy we end up splitting into
				// far too many grids (with roughly 1 mpg per grid). so after
				// profiling a few scenes, the fastest method seems to be just
				// to convert to a patch immediatly.
				// we really need a native dice for curves but until that time
				// i reckon this is best.
				//m_splitDecision = Split_Patch;


				     const CqMatrix & matCtoR = QGetRenderContext() ->matSpaceToSpace(
				                                    "camera", "raster",
								NULL, NULL,
								QGetRenderContextI()->Time()
				                                );
				     CqVector2D hull[ 2 ];     // control hull
				     hull[ 0 ] = matCtoR * P()->pValue( 0 )[0];
				     hull[ 1 ] = matCtoR * P()->pValue( 1 )[0];
				     CqVector2D lengthVector = hull[ 1 ] - hull[ 0 ];
				     TqFloat lengthraster = lengthVector.Magnitude();

				     // find the approximate "length" of a diced patch in raster space
				     TqFloat gridlength = GetGridLength();

				     // decide whether to split into more curve segments or a patch
				     if (
					 ( lengthraster < gridlength ) ||
				         ( !m_fDiceable )
				     )
				     {
				         // split into a patch
				         m_splitDecision = Split_Patch;
				     }
				     else
				     {
				         // split into smaller curves
				         m_splitDecision = Split_Curve;
				     }
				
			}

			return false;
		}

		/** Determine whether the passed surface is valid to be used as a
		 *  frame in motion blur for this surface.
		 */
		virtual bool	IsMotionBlurMatch( CqSurface* pSurf )
		{
			return( false );
		}
		/** Copy the information about splitting and dicing from the specified GPrim.
		 * \param From A CqSurface reference to copy the information from.
		 */
		virtual void CopySplitInfo( const CqSurface* From )
		{
			CqSurface::CopySplitInfo( From );
			const CqCurve* pCurve = dynamic_cast<const CqCurve*>(From);
			if( NULL != pCurve )
				m_splitDecision = pCurve->m_splitDecision;
		}

		/** Returns a normal to the curve. */
		bool GetNormal( TqInt index, CqVector3D& normal ) const;

		/** Returns a string name of the class. */
		virtual CqString strName() const
		{
			return "CqCurve";
		}
		/** Returns a const reference to the "width" parameter, or NULL if
		 * the parameter is not present. */
		const CqParameterTypedVarying <
		TqFloat, type_float, TqFloat
		> * width() const
		{
			if ( m_widthParamIndex >= 0 )
			{
				return static_cast <
				       const CqParameterTypedVarying <
				       TqFloat, type_float, TqFloat
				       > *
				       > ( m_aUserParams[ m_widthParamIndex ] );
			}
			else
			{
				return ( NULL );
			}

		}
		/** Returns a reference to the "width" parameter, or NULL if
		 * the parameter is not present. */
		CqParameterTypedVarying <
		TqFloat, type_float, TqFloat
		> * width()
		{
			if ( m_widthParamIndex >= 0 )
			{
				return static_cast <
				       CqParameterTypedVarying <
				       TqFloat, type_float, TqFloat
				       > *
				       > ( m_aUserParams[ m_widthParamIndex ] );
			}
			else
			{
				return ( NULL );
			}

		}
		void CloneData(CqCurve* clone) const;
		//--------------------------------------------------- Protected Members
	protected:
		/** Index of the width parameter within the m_aUserParams array of
		 * user parameters. */
		TqInt m_widthParamIndex;
		/** Index of the constantwidth parameter within the m_aUserParams array
		 * of user parameters. */
		TqInt m_constantwidthParamIndex;

		enum EssSplitDecision
		{
		    Split_Undecided = 0,
		    Split_Curve,
		    Split_Patch,
	};
		/** Stored decision about split to curves or patches.
		 */
		TqInt m_splitDecision;
};



/**
 * \class CqLinearCurveSegment
 *
 * A single segment or sub-segment from a linear curve.
 */
class CqLinearCurveSegment : public CqCurve
{
		//------------------------------------------------------ Public Methods
	public:
		CqLinearCurveSegment();
		virtual ~CqLinearCurveSegment();
		void NaturalSubdivide(
		    CqParameter* pParam,
		    CqParameter* pParam1, CqParameter* pParam2,
		    bool u
		);
		virtual TqInt Split( std::vector<boost::shared_ptr<CqSurface> >& aSplits );
		TqInt SplitToCurves( std::vector<boost::shared_ptr<CqSurface> >& aSplits );
		TqInt SplitToPatch( std::vector<boost::shared_ptr<CqSurface> >& aSplits );
		//---------------------------------------------- Inlined Public Methods
	public:
#ifdef _DEBUG

		CqString className() const
		{
			return CqString("CqLinearCurveSegment");
		}
#endif
		/** Returns the number of facevarying class parameters. */
		virtual	TqUint	cFaceVarying() const
		{
			return 0;
		}
		/** Returns the number of uniform class parameters. */
		virtual	TqUint cUniform() const
		{
			return 1;
		}
		/** Returns the number of varying class parameters. */
		virtual	TqUint cVarying() const
		{
			return 2;
		}
		/** Returns the number of vertex class parameters. */
		virtual	TqUint	cVertex() const
		{
			return 2;
		}
		/** Returns a string name of the class. */
		virtual CqString strName() const
		{
			return "CqLinearCurveSegment";
		}
		/** Typed natural subdivision for the surface. */
		template <class T, class SLT>
		void TypedNaturalSubdivide(
		    CqParameterTyped<T, SLT>* pParam,
		    CqParameterTyped<T, SLT>* pResult1,
		    CqParameterTyped<T, SLT>* pResult2,
		    bool u
		)
		{
			// we can only split curves along v, so enforce this
			assert( u == false );

			CqParameterTyped<T, SLT>* pTParam = static_cast<CqParameterTyped<T, SLT>*>( pParam );
			CqParameterTyped<T, SLT>* pTResult1 = static_cast<CqParameterTyped<T, SLT>*>( pResult1 );
			CqParameterTyped<T, SLT>* pTResult2 = static_cast<CqParameterTyped<T, SLT>*>( pResult2 );

			pTResult1->pValue() [ 0 ] = pTParam->pValue() [ 0 ];
			pTResult1->pValue() [ 1 ] = pTResult2->pValue() [ 0 ] = static_cast<T>( ( pTParam->pValue() [ 0 ] + pTParam->pValue() [ 1 ] ) * 0.5f );
			pTResult2->pValue() [ 1 ] = pTParam->pValue() [ 1 ];
		}
		virtual CqSurface* Clone() const;
};



/**
 * \class CqCubicCurveSegment
 *
 * A single segment or sub-segment from a cubic curve.
 */
class CqCubicCurveSegment : public CqCurve
{
		//------------------------------------------------------ Public Methods
	public:
		CqCubicCurveSegment();
		virtual ~CqCubicCurveSegment();
		void NaturalSubdivide(
		    CqParameter* pParam,
		    CqParameter* pParam1, CqParameter* pParam2,
		    bool u
		);
		void VaryingNaturalSubdivide(
		    CqParameter* pParam,
		    CqParameter* pParam1, CqParameter* pParam2,
		    bool u
		);
		virtual TqInt Split( std::vector<boost::shared_ptr<CqSurface> >& aSplits );
		TqInt SplitToCurves( std::vector<boost::shared_ptr<CqSurface> >& aSplits );
		TqInt SplitToPatch( std::vector<boost::shared_ptr<CqSurface> >& aSplits );

		void ConvertToBezierBasis( CqMatrix& matBasis );
		//---------------------------------------------- Inlined Public Methods
	public:
#ifdef _DEBUG

		CqString className() const
		{
			return CqString("CqCubicCurveSegment");
		}
#endif
		/** Returns the number of facevarying class parameters. */
		virtual	TqUint	cFaceVarying() const
		{
			return 0;
		}
		/** Returns the number of uniform class parameters. */
		virtual	TqUint cUniform() const
		{
			return 1;
		}
		/** Returns the number of varying class parameters. */
		virtual	TqUint cVarying() const
		{
			return 2;
		}
		/** Returns the number of vertex class parameters. */
		virtual	TqUint	cVertex() const
		{
			return 4;
		}
		/** Returns a string name of the class. */
		virtual CqString strName() const
		{
			return "CqCubicCurveSegment";
		}
		/** Calculate the tangent at a given u on the curve segment using de Casteljau */
		CqVector3D	CalculateTangent(TqFloat u);

		/** Typed natural subdivision for the surface. */
		template <class T, class SLT>
		void TypedNaturalSubdivide(
		    CqParameterTyped<T, SLT>* pParam,
		    CqParameterTyped<T, SLT>* pResult1,
		    CqParameterTyped<T, SLT>* pResult2,
		    bool u
		)
		{
			// we can only split curves along v, so enforce this
			assert( u == false );

			CqParameterTyped<T, SLT>* pTParam = static_cast<CqParameterTyped<T, SLT>*>( pParam );
			CqParameterTyped<T, SLT>* pTResult1 = static_cast<CqParameterTyped<T, SLT>*>( pResult1 );
			CqParameterTyped<T, SLT>* pTResult2 = static_cast<CqParameterTyped<T, SLT>*>( pResult2 );

			pTResult1->pValue() [ 0 ] = pTParam->pValue() [ 0 ];
			pTResult1->pValue() [ 1 ] = static_cast<T>( ( pTParam->pValue() [ 0 ] + pTParam->pValue() [ 1 ] ) / 2.0f );
			pTResult1->pValue() [ 2 ] = static_cast<T>( pTResult1->pValue() [ 1 ] / 2.0f + ( pTParam->pValue() [ 1 ] + pTParam->pValue() [ 2 ] ) / 4.0f );

			pTResult2->pValue() [ 3 ] = pTParam->pValue() [ 3 ];
			pTResult2->pValue() [ 2 ] = static_cast<T>( ( pTParam->pValue() [ 2 ] + pTParam->pValue() [ 3 ] ) / 2.0f );
			pTResult2->pValue() [ 1 ] = static_cast<T>( pTResult2->pValue() [ 2 ] / 2.0f + ( pTParam->pValue() [ 1 ] + pTParam->pValue() [ 2 ] ) / 4.0f );

			pTResult1->pValue() [ 3 ] = static_cast<T>( ( pTResult1->pValue() [ 2 ] + pTResult2->pValue() [ 1 ] ) / 2.0f );
			pTResult2->pValue() [ 0 ] = pTResult1->pValue() [ 3 ];
		}

		/** Typed natural subdivision for the surface. */
		template <class T, class SLT>
		void VaryingTypedNaturalSubdivide(
		    CqParameterTyped<T, SLT>* pParam,
		    CqParameterTyped<T, SLT>* pResult1,
		    CqParameterTyped<T, SLT>* pResult2,
		    bool u
		)
		{
			// we can only split curves along v, so enforce this
			assert( u == false );

			CqParameterTyped<T, SLT>* pTParam = static_cast<CqParameterTyped<T, SLT>*>( pParam );
			CqParameterTyped<T, SLT>* pTResult1 = static_cast<CqParameterTyped<T, SLT>*>( pResult1 );
			CqParameterTyped<T, SLT>* pTResult2 = static_cast<CqParameterTyped<T, SLT>*>( pResult2 );

			pTResult1->pValue() [ 0 ] = pTParam->pValue() [ 0 ];
			pTResult1->pValue() [ 1 ] = pTResult2->pValue() [ 0 ] = static_cast<T>( ( pTParam->pValue() [ 0 ] + pTParam->pValue() [ 1 ] ) * 0.5f );
			pTResult2->pValue() [ 1 ] = pTParam->pValue() [ 1 ];
		}
		virtual CqSurface* Clone() const;
};



/**
 * \class CqCurvesGroup
 *
 * Base class for a group of curves.
 */
class CqCurvesGroup : public CqCurve
{
		//------------------------------------------------------ Public Methods
	public:
		CqCurvesGroup();
		virtual ~CqCurvesGroup();
#ifdef _DEBUG

		CqString className() const
		{
			return CqString("CqCurvesGroup");
		}
#endif
		void CloneData(CqCurvesGroup* clone) const;
		//--------------------------------------------------- Protected Members
	protected:
		TqInt m_ncurves;       ///< Number of curves in the group.
		std::vector<TqInt> m_nvertices;  ///< Number of vertices in each curve.
		bool m_periodic;      ///< true if the curves specified are periodic
		TqInt m_nTotalVerts;   ///< total number of vertices
}
;



/**
 * \class CqLinearCurvesGroup
 *
 * A (possibly disconnected) group of linear curves, such as those specified in
 * a RIB call to Curves "linear".
 */
class CqLinearCurvesGroup : public CqCurvesGroup
{
		//------------------------------------------------------ Public Methods
	public:
		CqLinearCurvesGroup()
		{}
		CqLinearCurvesGroup(
		    TqInt ncurves, TqInt nvertices[], bool periodic = false
		);
		virtual ~CqLinearCurvesGroup();
		virtual	TqInt Split( std::vector<boost::shared_ptr<CqSurface> >& aSplits );
		virtual void Transform(
		    const CqMatrix& matTx,
		    const CqMatrix& matITTx,
		    const CqMatrix& matRTx,
		    TqInt iTime = 0
		);
		//---------------------------------------------- Inlined Public Methods
	public:
#ifdef _DEBUG

		CqString className() const
		{
			return CqString("CqLinearCurvesGroup");
		}
#endif
		/** Returns the number of facevarying class parameters. */
		virtual	TqUint cFaceVarying() const
		{
			return 0;
		}
		/** Returns the number of uniform class parameters. */
		virtual	TqUint cUniform() const
		{
			return m_ncurves;
		}
		/** Returns the number of varying class parameters. */
		virtual TqUint cVarying() const
		{
			return m_nTotalVerts;
		}
		/** Returns the number of vertex class parameters. */
		virtual	TqUint cVertex() const
		{
			return m_nTotalVerts;
		}
		/** Returns a string name of the class. */
		virtual CqString strName() const
		{
			return "CqLinearCurvesGroup";
		}
		virtual CqSurface* Clone() const;
};



/**
 * \class CqCubicCurvesGroup
 *
 * A (possibly disconnected) group of cubic curves, such as those specified in
 * a RIB call to Curves "cubic".
 */
class CqCubicCurvesGroup : public CqCurvesGroup
{
		//------------------------------------------------------ Public Methods
	public:
		CqCubicCurvesGroup() {}
		CqCubicCurvesGroup(
		    TqInt ncurves, TqInt nvertices[], bool periodic = false
		);
		virtual ~CqCubicCurvesGroup();
		virtual	TqUint cVarying() const;
		virtual TqInt Split( std::vector<boost::shared_ptr<CqSurface> >& aSplits );
		virtual	CqBound	Bound() const;
		virtual void Transform(
		    const CqMatrix& matTx,
		    const CqMatrix& matITTx,
		    const CqMatrix& matRTx,
		    TqInt iTime = 0
		);
		//---------------------------------------------- Inlined Public Methods
	public:
#ifdef _DEBUG

		CqString className() const
		{
			return CqString("CqCubicCurvesGroup");
		}
#endif
		/** Returns the number of facevarying class parameters. */
		virtual	TqUint cFaceVarying() const
		{
			return 0;
		}
		/** Returns the number of uniform class parameters. */
		virtual	TqUint cUniform() const
		{
			return m_ncurves;
		}
		/** Returns the number of vertex class parameters. */
		virtual	TqUint cVertex() const
		{
			return m_nTotalVerts;
		};
		/** Returns a string name of the class. */
		virtual CqString strName() const
		{
			return "CqCubicCurvesGroup";
		}
		virtual CqSurface* Clone() const;
};



END_NAMESPACE( Aqsis )
#endif
