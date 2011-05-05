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

/**
        \file
        \brief Declares the classes and support structures for 
                handling RenderMan Curves primitives.
        \author Jonathan Merritt (j.merritt@pgrad.unimelb.edu.au)
*/

#ifndef CURVES_H_INCLUDED
#define CURVES_H_INCLUDED

#include        <aqsis/aqsis.h>
#include        <aqsis/math/matrix.h>
#include        "surface.h"
#include        "patch.h"

namespace Aqsis {


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
		virtual	void Bound(CqBound* bound) const;
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
		/** \brief Returns whether the curve is diceable
		 *
		 * At the moment, no Curves are directly diceable since they're
		 * converted to patches just prior to rendering.
		 */
		virtual bool Diceable(const CqMatrix& matCtoR);

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
		/** \brief Natural subdivision for this curve segment.
		 *
		 * \param pParam - Original parameter.
		 * \param pParam1 - First new parameter.
		 * \param pParam2 - Second new parameter.
		 * \param u - true if the split is along u (should always be false!)
		 */
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
		/** \brief Natural subdivision of parameters on cubic curves
		 *
		 * \param pParam - Original parameter.
		 * \param pParam1 - First new parameter.
		 * \param pParam2 - Second new parameter.
		 * \param u - true if the split is along u (should always be false!)
		 */
		void NaturalSubdivide(
		    CqParameter* pParam,
		    CqParameter* pParam1, CqParameter* pParam2,
		    bool u
		);
		/** \brief Natural subdivision for varying parameters on cubic curves
		 *
		 * \param pParam - Original parameter.
		 * \param pParam1 - First new parameter.
		 * \param pParam2 - Second new parameter.
		 * \param u - true if the split is along u (should always be false!)
		 */
		void VaryingNaturalSubdivide(
		    CqParameter* pParam,
		    CqParameter* pParam1, CqParameter* pParam2,
		    bool u
		);
		virtual TqInt Split( std::vector<boost::shared_ptr<CqSurface> >& aSplits );
		TqInt SplitToCurves( std::vector<boost::shared_ptr<CqSurface> >& aSplits );
		TqInt SplitToPatch( std::vector<boost::shared_ptr<CqSurface> >& aSplits );

		/** \brief Calculate the tangent at a given u along the curve
		 *
		 * The algorithm uses the analytical form for the tangent vector at
		 * points inside the curve.  For the curve endpoints (u == 0 or u ==
		 * 1), a numerically stable algorithm is used which works even in the
		 * cases where some control points are repeated.
		 *
		 * \param u - curve parameter
		 * \return the tangent vector at u.
		 */
		CqVector3D	CalculateTangent(TqFloat u);
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
		virtual void Transform(
		    const CqMatrix& matTx,
		    const CqMatrix& matITTx,
		    const CqMatrix& matRTx,
		    TqInt iTime = 0
		);
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
		virtual void AddPrimitiveVariable( CqParameter* pParam );
		virtual	TqUint cVarying() const;
		virtual TqInt Split( std::vector<boost::shared_ptr<CqSurface> >& aSplits );
		virtual	void Bound(CqBound* bound) const;
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
		}
		/** Returns a string name of the class. */
		virtual CqString strName() const
		{
			return "CqCubicCurvesGroup";
		}
		virtual CqSurface* Clone() const;
	private:
		/** \brief Convert a vertex parameter from the current to the bezier basis.
		 *
		 * \param param - parameter to convert.  Must be vertex storage class.
		 * \return new converted parameter in the bezier basis.
		 */
		template<typename DataT, typename SLDataT>
		CqParameter* convertToBezierBasis(CqParameter* param);

		/// Total number of verts for vertex data after Bezier basis transform.
		TqInt m_nVertsBezier;
		/** \brief Transformation taking parameters in the current basis into
		 * parameters in the Bezier basis.
		 */
		CqMatrix m_basisTrans;
};



} // namespace Aqsis
#endif
