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
        \brief Declares the classes and support structures for 
                handling RenderMan Curves primitives.
        \author Jonathan Merritt (j.merritt@pgrad.unimelb.edu.au)
*/

#ifndef CURVES_H_INCLUDED
#define CURVES_H_INCLUDED

#include        "aqsis.h"
#include        "matrix.h"
#include        "surface.h"
#include        "vector4d.h"
#include        "patch.h"
#define         _qShareName	CORE
#include        "share.h"

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
		CqCurve( const CqCurve &from );
		virtual ~CqCurve();
		virtual void AddPrimitiveVariable( CqParameter* pParam );
		virtual	CqBound	Bound() const;
		CqCurve& operator=( const CqCurve& from );
		virtual void SetDefaultPrimitiveVariables( TqBool bUseDef_st = TqTrue );
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
		virtual TqBool Diceable()
		{
			return TqFalse;
		}
		/** Returns a normal to the curve. */
		TqBool GetNormal( TqInt index, CqVector3D& normal ) const
		{
			if ( N() != NULL )
			{
				normal = ( *N() ) [ index ];
				return TqTrue;
			}
			else
			{
				normal = CqVector3D( 0, 0, -1 );  // default camera normal
				return TqFalse;
			}
		}
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
		//--------------------------------------------------- Protected Members
	protected:
		/** Index of the width parameter within the m_aUserParams array of
		 * user parameters. */
		TqInt m_widthParamIndex;
		/** Index of the constantwidth parameter within the m_aUserParams array
		 * of user parameters. */
		TqInt m_constantwidthParamIndex;

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
		CqLinearCurveSegment( const CqLinearCurveSegment &from );
		virtual ~CqLinearCurveSegment();
		CqLinearCurveSegment& operator=( const CqLinearCurveSegment& from );
		void NaturalSubdivide(
		    CqParameter* pParam,
		    CqParameter* pParam1, CqParameter* pParam2,
		    TqBool u
		);
		virtual TqInt Split( std::vector<CqBasicSurface*>& aSplits );
		TqInt SplitToCurves( std::vector<CqBasicSurface*>& aSplits );
		TqInt SplitToPatch( std::vector<CqBasicSurface*>& aSplits );
		//---------------------------------------------- Inlined Public Methods
	public:
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
		template <class T, class SLT> void TypedNaturalSubdivide(
		    CqParameterTyped<T, SLT>* pParam,
		    CqParameterTyped<T, SLT>* pResult1,
		    CqParameterTyped<T, SLT>* pResult2,
		    TqBool u
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
		CqCubicCurveSegment( const CqCubicCurveSegment &from );
		virtual ~CqCubicCurveSegment();
		CqCubicCurveSegment& operator=( const CqCubicCurveSegment& from );
		//---------------------------------------------- Inlined Public Methods
	public:
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
		CqCurvesGroup( const CqCurvesGroup& from );
		virtual ~CqCurvesGroup();
		CqCurvesGroup& operator=( const CqCurvesGroup& from );
		//--------------------------------------------------- Protected Members
	protected:
		TqInt m_ncurves;       ///< Number of curves in the group.
		std::vector<TqInt> m_nvertices;  ///< Number of vertices in each curve.
		TqBool m_periodic;      ///< true if the curves specified are periodic
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
		CqLinearCurvesGroup(
		    TqInt ncurves, TqInt nvertices[], TqBool periodic = TqFalse
		);
		CqLinearCurvesGroup( const CqLinearCurvesGroup &from );
		virtual ~CqLinearCurvesGroup();
		CqLinearCurvesGroup& operator=( const CqLinearCurvesGroup& from );
		virtual	TqInt Split( std::vector<CqBasicSurface*>& aSplits );
		virtual void Transform(
		    const CqMatrix& matTx,
		    const CqMatrix& matITTx,
		    const CqMatrix& matRTx,
			TqInt iTime = 0
		);
		//---------------------------------------------- Inlined Public Methods
	public:
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
		CqCubicCurvesGroup(
		    TqInt ncurves, TqInt nvertices[], TqBool periodic = TqFalse
		);
		CqCubicCurvesGroup( const CqCubicCurvesGroup &from );
		virtual ~CqCubicCurvesGroup();
		virtual	TqUint cVarying() const;
		CqCubicCurvesGroup& operator=( const CqCubicCurvesGroup& from );
		virtual TqInt Split( std::vector<CqBasicSurface*>& aSplits );
		virtual void Transform(
		    const CqMatrix& matTx,
		    const CqMatrix& matITTx,
		    const CqMatrix& matRTx,
			TqInt iTime = 0
		);
		//---------------------------------------------- Inlined Public Methods
	public:
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
};



END_NAMESPACE( Aqsis )
#endif
