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
		\brief Template class to handle Bezier forward differencing
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

//? Is .h included already?
#ifndef FORWARDDIFF_H_INCLUDED
#define FORWARDDIFF_H_INCLUDED 1

#include	<aqsis/aqsis.h>

namespace Aqsis {


template <class T>
class CqForwardDiffBezier
{
	public:

		CqForwardDiffBezier( TqFloat dt )
		{
			InitPreCalcMatrix( dt );
		}


		/**
		 * Initializes the PRECALCULATED matrix (M). Needs to be reset any time the
		 * the parametric "step-size" changes (same as Uincr in direct eval version).
		 */
		void InitPreCalcMatrix( TqFloat dt )
		{
			// store repeated calculations
			TqFloat dt2 = dt * dt, dt3 = dt2 * dt, dt3_3 = 3 * dt3, dt3_6 = 6 * dt3,
			                             dt3_18 = 18 * dt3, dt2_3 = 3 * dt2, dt2_6 = 6 * dt2, dt_3 = 3 * dt;
			M00 = -dt3_6;
			M01 = dt3_18;
			M02 = -dt3_18;
			M03 = dt3_6;
			M10 = dt2_6 - dt3_6;
			M11 = dt3_18 - 2 * dt2_6;
			M12 = dt2_6 - dt3_18;
			M13 = dt3_6;
			M20 = dt2_3 - dt_3 - dt3;
			M21 = dt3_3 - dt2_6 + dt_3;
			M22 = dt2_3 - dt3_3;
			M23 = dt3;
		};

		/**
		 * Applies PRECALCULATED matrix to the four bezier control points to calculate
		 * the FORWARD-DIFFERENCE increments matrix (FD).
		 */
		void CalcForwardDiff( T& A, T& B, T& C, T& D )
		{
			f = A;                             // STORE THE START VALUE
			df = static_cast<T>( A * M20 + B * M21 + C * M22 + D * M23 ); // CALC CHANGE IN START
			ddf = static_cast<T>( A * M10 + B * M11 + C * M12 + D * M13 ); // CALC CHANGE IN "CHANGE IN START"
			dddf = static_cast<T>( A * M00 + B * M01 + C * M02 + D * M03 ); // CALC CHANGE IN CHANGE IN "CHANGE IN START"
		};

		/**
		 * Returns the current value (f). Increments to the next step.
		 */
		T GetValue()
		{
			T res = f;							// store current value
			f = f + df;
			df = df + ddf;
			ddf = ddf + dddf;	// go to next step
			return ( res );						// return previous
		}
		;

	private:
		// Precalculated forward-difference matrix (applied to control points to
		// find actual forward difference increments).
		TqFloat M00, M01, M02, M03,
		M10, M11, M12, M13,
		M20, M21, M22, M23;

		// Forward difference vector increments (and start point f)
		T f, df, ddf, dddf;
};


//-----------------------------------------------------------------------

} // namespace Aqsis

#endif // MOTION_H_INCLUDED
