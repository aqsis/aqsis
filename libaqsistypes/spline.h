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
		\brief Declares the CqSplineCubic class for generic spline functionality.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is .h included already?
#ifndef SPLINE_H_INCLUDED
#define SPLINE_H_INCLUDED 1

#include	<vector>

#include	"aqsis.h"

#include	"matrix.h"
#include	"vector4d.h"
#include	"sstring.h"

START_NAMESPACE( Aqsis )


typedef	TqFloat	__Basis[ 4 ][ 4 ];

extern __Basis	gBezierBasis;
extern __Basis	gBSplineBasis;
extern __Basis	gCatmullRomBasis;
extern __Basis	gHermiteBasis;
extern __Basis	gPowerBasis;


//----------------------------------------------------------------------
/** \class CqSplineCubic
 * Cubic spline curve
 */

class CqSplineCubic
{
public:
    CqSplineCubic( TqInt cu = 4 );
    virtual	~CqSplineCubic()
    {}

    virtual	CqVector4D	Evaluate( TqFloat t ) const;
    virtual	void	InitFD( TqInt n );
    virtual	CqVector4D	EvaluateFD();
    virtual	TqInt	cSections() const;

    /** Get a reference to the array of control points.
     */
    std::vector<CqVector4D>& aControlPoints()
    {
        return ( m_aControlPoints );
    }
    /** Get a reference to the cubic spline basis matrix.
     */
    CqMatrix&	matBasis()
    {
        return ( m_matBasis );
    }
    /** Set the cubic spline basis matrix.
     * \param mat Basis matrix.
     */
    void	SetmatBasis( CqMatrix& mat )
    {
        m_matBasis = mat;
    }
    /** Set the cubic spline basis matrix.
     * \param strName Basis name.
     */
    void	SetBasis( const CqString& strName )
    {
        __Basis * pVals = 0;
		TqInt step = 3;
        if ( strName.compare( "bezier" ) == 0 )
		{
            pVals = &gBezierBasis;
			step = 3;
		}
        else if ( strName.compare( "bspline" ) == 0 )
		{
            pVals = &gBSplineBasis;
			step = 1;
		}
        else if ( strName.compare( "catmull-rom" ) == 0 )
		{
            pVals = &gCatmullRomBasis;
			step = 1;
		}
        else if ( strName.compare( "hermite" ) == 0 )
		{
            pVals = &gHermiteBasis;
			step = 2;
		}
        else if ( strName.compare( "power" ) == 0 )
		{
            pVals = &gPowerBasis;
			step =4;
		}

        if ( pVals )
        {
            CqMatrix m;
            m = *pVals;
            SetmatBasis( m );
			SetStep( step );
        }
    }

    /** Get the control point step size for the evaluation window.
     * \return Integer step size.
     */
    TqInt	Step() const
    {
        return ( m_Step );
    }
    /** Set the control point step size for the evaluation window.
     * \param Step Integer step size.
     */
    void	SetStep( const TqInt Step )
    {
        m_Step = Step;
    }

    /** Indexed access to the control points.
     * \param i Integer index.
     */
    CqVector4D&	operator[] ( TqInt i )
    {
        assert( i<m_cu ); return ( m_aControlPoints[ i ] );
    }
    /** Indexed access to the control points.
     * \param i Integer index.
     */
    const CqVector4D&	operator[] ( TqInt i ) const
    {
        assert( i<m_cu ); return ( m_aControlPoints[ i ] );
    }

protected:
    CqMatrix	m_matBasis;						///< Basis matrix.
    TqInt	m_Step;							///< Evaluation window step size.
    TqInt	m_cu;							///< Number of control points.
    std::vector<CqVector4D>	m_aControlPoints;		///< Array of 4D control points.

    CqVector4D	m_vecFDPoint;					///< Forward difference evaluation parameters.
    CqVector4D	m_vecFDDelta;					///< Forward difference evaluation parameters.
    CqVector4D	m_vecFDDelta2;					///< Forward difference evaluation parameters.
    CqVector4D	m_vecFDDelta3;					///< Forward difference evaluation parameters.
}
;


//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

#endif	// !SPLINE_H_INCLUDED
