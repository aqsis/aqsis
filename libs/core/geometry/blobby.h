// Aqsis
// Copyright (C) 2006, Paul C. Gregory
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
    \brief Declares Blobby polygonizer.

*/
/*    References:
*          K-3d.org
*
*/

//? Is .h included already?
#ifndef BLOBBY_H_INCLUDED
#define BLOBBY_H_INCLUDED

#include <aqsis/aqsis.h>
#include <aqsis/math/matrix.h>
#include "surface.h"
#include <aqsis/math/vector4d.h>

#include <aqsis/ri/ri.h>
#include <aqsis/math/vector3d.h>

namespace Aqsis {

// CqBlobby
class CqBlobby : public CqSurface
{
	public:
		CqBlobby(TqInt nleaf, TqInt ncode, TqInt* code, TqInt nfloats, TqFloat* floats, TqInt nstrings, char** strings);

		/** Determine whether the passed surface is valid to be used as a
		 *  frame in motion blur for this surface (CqSurface implementation).
		 */
		virtual bool IsMotionBlurMatch( CqSurface* pSurf )
		{
			return( false );
		}

		/** \todo Find out the correct values for the 4 following functions */
		virtual	TqUint	cUniform() const
		{
			return ( 1 );
		}
		virtual	TqUint	cVarying() const
		{
			return ( 4 );
		}
		virtual	TqUint	cVertex() const
		{
			return ( 16 );
		}
		virtual	TqUint	cFaceVarying() const
		{
			return ( 1 );
		}

		/** Clone all CqBlobby data (CqSurface implementation).
		 */
		virtual CqSurface* Clone() const;

		// Overrides from CqSurface
		virtual TqInt Split( std::vector<boost::shared_ptr<CqSurface> >& aSplits );
		virtual void	Bound(CqBound* bound) const
		{
			bound->vecMin() = m_bbox.vecMin();
			bound->vecMax() = m_bbox.vecMax();
		}

		/** Return CqBlobby's implicit value.
		 * \param World position to compute implicit value from.
		 */
		TqFloat implicit_value(const CqVector3D& Point, TqInt n, std::vector <TqFloat>& splits);

		TqFloat implicit_value(const CqVector3D& Point);

		TqInt polygonize(TqInt PixelsWidth, TqInt PixelsHeight, TqInt& NPoints, TqInt& NPolys, TqInt*& NVertices, TqInt*& Vertices, TqFloat*& Points);

		//! Enumeration of the blobby opcodes
		typedef enum
		{
			CONSTANT,
			ELLIPSOID,
			SEGMENT,
			PLANE,
			ADD,
			MULTIPLY,
			MIN,
			MAX,
			SUBTRACT,
			DIVIDE,
			NEGATE,
			IDEMPOTENTATE,
			AIR,
		} EqOpcodeName;

		// Blobby virtual machine instruction
		union instruction
		{
			instruction(const EqOpcodeName OpCode) : opcode(OpCode)
			{}
			instruction(const TqInt Count) : count(Count)
			{}
			instruction(const TqFloat Value) : value(Value)
			{}
			instruction(const CqVector3D& Vector)
			{
				vector[0] = Vector[0];
				vector[1] = Vector[1];
				vector[2] = Vector[2];
			}
			instruction(const CqMatrix& Matrix)
			{
				matrix[0] = Matrix[0][0];
				matrix[1] = Matrix[0][1];
				matrix[2] = Matrix[0][2];
				matrix[3] = Matrix[0][3];

				matrix[4] = Matrix[1][0];
				matrix[5] = Matrix[1][1];
				matrix[6] = Matrix[1][2];
				matrix[7] = Matrix[1][3];

				matrix[8] = Matrix[2][0];
				matrix[9] = Matrix[2][1];
				matrix[10] = Matrix[2][2];
				matrix[11] = Matrix[2][3];

				matrix[12] = Matrix[3][0];
				matrix[13] = Matrix[3][1];
				matrix[14] = Matrix[3][2];
				matrix[15] = Matrix[3][3];
			}

			EqOpcodeName opcode;
			TqInt count;
			TqInt children_index;
			TqFloat value;
			TqFloat vector[3];
			TqFloat matrix[16];

			CqVector3D get_vector() const
			{
				return CqVector3D(vector[0], vector[1], vector[2]);
			}

			CqMatrix get_matrix() const
			{
				return CqMatrix(
					matrix[0], matrix[1], matrix[2], matrix[3],
					matrix[4], matrix[5], matrix[6], matrix[7],
					matrix[8], matrix[9], matrix[10], matrix[11],
					matrix[12], matrix[13], matrix[14], matrix[15]);
			}
		};

		typedef std::vector<instruction> instructions_t;

	private:
		// Program (list of instructions) that computes implicit values
		instructions_t m_instructions;

		// Bounding-box
		CqBound m_bbox;

		// RenderMan primitive storage
		TqInt m_nleaf;
		TqInt m_ncode;
		TqInt* m_code;
		TqInt m_nfloats;
		TqFloat* m_floats;
		TqInt m_nstrings;
		char** m_strings;
};

//-----------------------------------------------------------------------

} // namespace Aqsis

#endif  // !BLOBBY_H_INCLUDED
