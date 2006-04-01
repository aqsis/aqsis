// Aqsis
// Copyright © 2001, Paul C. Gregory
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
		\brief Implements RiBlobbyV option.
	
*/
/*    References:
 *          K-3d.org
 *
 */

//? Is .h included already?
#ifndef BLOBBY_H_INCLUDED
#define BLOBBY_H_INCLUDED

#include	"aqsis.h"
#include	"matrix.h"
#include	"surface.h"
#include	"vector4d.h"

#include	"ri.h"
#include	"vector3d.h"

#include	"jules_bloomenthal.h"

START_NAMESPACE( Aqsis )

// CqBlobby 
class CqBlobby : public implicit_functor
{
public:
	CqBlobby(TqInt nleaf, TqInt ncode, TqInt* code, TqInt nflt, TqFloat* flt) ;

	inline TqFloat implicit_value(const CqVector3D& Point);
	void polygonize(std::vector<CqVector3D>& Vertices, std::vector<CqVector3D>& Normals, std::vector<std::vector<TqInt> >& Polygons, TqFloat ShadingRate);

private:
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
		IDEMPOTENTATE
	} EqOpcodeName;

	class CqOpcode
	{
	public:
		CqOpcode(EqOpcodeName Name, const TqInt Index1, const TqInt Index2) :
			name(Name), index1(Index1), index2(Index2)
		{
		}

      CqOpcode(EqOpcodeName Name, const TqInt Index1) :
			name(Name), index1(Index1), index2(0)
		{
		}

      CqOpcode(EqOpcodeName Name) :
			name(Name), index1(0), index2(0)
		{
		}
		EqOpcodeName name;
		TqInt index1;
		TqInt index2;
	};

	union instruction
	{
	public:
		instruction(const EqOpcodeName OpCode) : opcode(OpCode) {}
		instruction(const TqInt Count) : count(Count) {}
		instruction(const TqFloat Value) : value(Value) {}
		instruction(const CqVector3D& Vector) { vector[0] = Vector[0]; vector[1] = Vector[1]; vector[2] = Vector[2]; }
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

	void build_stack(CqOpcode op);

	void grow_bound(const CqMatrix& transformation, const TqFloat radius = 1.0f);

	void grow_bound(const CqVector3D& vector, const TqFloat radius = 1.0f);

	TqInt* Codes;
	TqFloat* Floats;

	std::vector<CqOpcode> opcodes;
	std::vector<instruction> instructions;

	std::vector<CqVector3D> origins;
	CqBound bbox; // m_vecMin m_vecMax
   TqBool m_IsComplex;
};

//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

#endif	// !BLOBBY_H_INCLUDED
