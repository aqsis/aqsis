// Aqsis
// Copyright c 1997 - 2001, Paul C. Gregory
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
		\brief Implements RiBlobbyV
		\author Romain Behar (romainbehar@yahoo.com)
		\author Michel Joron (rearrange a bit)
*/
/*    References:
 *          k-3d 
 *
 */

#include	<math.h>
#include	<vector>
#include	<list>
#include <limits>

#include	"aqsis.h"
#include	"ri.h"
#include	"vector4d.h"
#include	"matrix.h"
#include	"blobby.h"


START_NAMESPACE( Aqsis )


CqVector3D nearest_segment_point(const CqVector3D& Point, const CqVector3D& S1,
                                 const CqVector3D& S2)
{
        const CqVector3D vector = S2 - S1;
        const CqVector3D w = Point - S1;

        const CqVector3D c1 = w * vector;
        TqFloat mag_c1 = c1.Magnitude();
        if(mag_c1 <= 0)
                return S1;

        const CqVector3D c2 = (vector * vector);
        TqFloat mag_c2 = c2.Magnitude();
        if(mag_c2 <= mag_c1)
                return S2;

        const CqVector3D b = c1 / c2;
        const CqVector3D middlepoint = S1 + b.Magnitude() * vector;
        return middlepoint;
}

inline const CqMatrix translation3D(const CqVector3D& v)
{
        return CqMatrix( 1, 0, 0, v.x(),
                0, 1, 0, v.y(),
                0, 0, 1, v.z(),
                0, 0, 0, 1);
}

inline const CqMatrix scaling3D(const CqVector3D& v)
{
        return CqMatrix(
                v.x(), 0, 0, 0,
                0, v.y(), 0, 0,
                0, 0, v.z(), 0,
                0, 0, 0, 1);
}

CqBlobby::CqBlobby(TqInt nleaf, TqInt ncode, TqInt* code, TqInt nflt, TqFloat* flt) : Codes(code), Floats(flt)
{
    //
    bbox = CqBound(-0.5, -0.5, -0.5, 0.5, 0.5, 0.5);
    m_IsComplex = TqFalse;

    // Build CqOpcode array
    EqOpcodeName current_opcode;
    TqInt int_index = 0;
    TqInt float_index = 0;
    for(TqInt c = 0; c < ncode;)
    {
	    switch(code[c++])
	    {
		case 0: current_opcode = ADD; break;
		case 1: current_opcode = MULTIPLY; break;
		case 2: current_opcode = MAX; break;
		case 3: current_opcode = MIN; break;
		case 4: current_opcode = DIVIDE; break;
		case 5: current_opcode = SUBTRACT; break;
		case 6: current_opcode = NEGATE; break;
		case 7: current_opcode = IDEMPOTENTATE; break;

		case 1000: current_opcode = CONSTANT; break;
		case 1001: current_opcode = ELLIPSOID; break;
		case 1002: current_opcode = SEGMENT; break;
		case 1003: current_opcode = PLANE; break;
	    }

	switch(current_opcode)
	{
	    case CONSTANT:
		Aqsis::log() << warning << "Blobby constant not implemented" << std::endl;
		opcodes.push_back(CqOpcode(current_opcode, code[c++]));
	    break;

	    case ELLIPSOID:
	    case SEGMENT:
		  opcodes.push_back(CqOpcode(current_opcode, code[c++]));
	    break;

	    case PLANE:
		Aqsis::log()  << warning << "Blobby plane not implemented" << std::endl;
		opcodes.push_back(CqOpcode(current_opcode, code[c], code[c+1]));
		c += 2;
	    break;

	    case ADD:
	    case MULTIPLY:
	    case MAX:
	    case MIN:
	    {
		TqInt n_ops = code[c];
		opcodes.push_back(CqOpcode(current_opcode, c));
		c += n_ops + 1;
	    }
	    break;

	    case DIVIDE:
	    case SUBTRACT:
       	    {
		opcodes.push_back(CqOpcode(current_opcode, code[c], code[c+1]));
		c += 2;
                m_IsComplex = TqTrue;
       	    }
            break;

	    case NEGATE:
		opcodes.push_back(CqOpcode(current_opcode, code[c++]));
	    break;

	    case IDEMPOTENTATE:
		c++;
	    break;
	}
    }

    // Prepare stack for implicit function
    build_stack(opcodes.back());
}

TqFloat CqBlobby::implicit_value(const CqVector3D& Point)
{
	std::stack<TqFloat> stack;
	stack.push(0);

	for(unsigned long pc = 0; pc < instructions.size(); )
		{
			switch(instructions[pc++].opcode)
				{
					case CONSTANT:
						stack.push(instructions[pc++].value);
						break;
					case ELLIPSOID:
						{
							const TqFloat r2 = (instructions[pc++].get_matrix() * Point).Magnitude2();
							stack.push(r2 <= 1 ? 1 - 3*r2 + 3*r2*r2 - r2*r2*r2 : 0);
						}
						break;
					case SEGMENT:
						{
							const CqMatrix m = instructions[pc++].get_matrix();
							const CqVector3D start = instructions[pc++].get_vector();
							const CqVector3D end = instructions[pc++].get_vector();
							const TqFloat radius = instructions[pc++].value;

                     // I don't think I convert the equation correctly...
                     //
                     //
		               //TqFloat r2 = (k3d::inverse(k3d::translation3D(nearest_segment_point(Point, start, end)) *
                     //              k3d::scaling3D(radius) * m) * Point).Magnitude2();
                     //
                     CqVector3D nearpt = nearest_segment_point(Point, start, end);
                     CqMatrix neareast =   (radius * translation3D(nearpt)  * m).Inverse();
                     CqVector3D result  = neareast * Point;
                     
                     TqFloat r2 = result.Magnitude2();
                     
		     Aqsis::log() << info << "Point " << Point << " nearpt " << nearpt << " r2 " << r2 << std::endl;
							
                     stack.push(r2 <= 1 ? (1 - 3*r2 + 3*r2*r2 - r2*r2*r2) : 0);
						}
						break;
					case SUBTRACT:
						{
							TqFloat a = stack.top();
							stack.pop();
							TqFloat b = stack.top();
							stack.pop();
							
							stack.push(b - a);
						}break;
					case DIVIDE:
						{
							TqFloat a = stack.top();
							stack.pop();
							TqFloat b = stack.top();
							stack.pop();
		     					Aqsis::log() << info << "idex a " << a << " idex b " << b << std::endl;
							if (a != 0.0)
								stack.push(b / a);
							else
								stack.push(0.0f);
						}break;
					case ADD:
						{
							const TqInt count = instructions[pc++].count;
							TqFloat sum = 0;
							for(TqInt i = 0; i != count; ++i)
								{
									sum += stack.top();
									stack.pop();
								}
							stack.push(sum);
						}
						break;
					case MULTIPLY:
						{
							const TqInt count = instructions[pc++].count;
							TqFloat product = 1;
							for(TqInt i = 0; i != count; ++i)
								{
									product *= stack.top();
									stack.pop();
								}
							stack.push(product);
						}
						break;
					case MIN:
						{
							const TqInt count = instructions[pc++].count;
							TqFloat minimum = std::numeric_limits<TqFloat>::max();
							for(TqInt i = 0; i != count; ++i)
								{
									minimum = std::min(minimum, stack.top());
									stack.pop();
								}
							stack.push(minimum);
						}
						break;
					case MAX:
						{
							const TqInt count = instructions[pc++].count;
							TqFloat maximum = -std::numeric_limits<TqFloat>::max();
							for(TqInt i = 0; i != count; ++i)
								{
									maximum = std::max(maximum, stack.top());
									stack.pop();
								}
							stack.push(maximum);
						}
						break;
				}
		}

	return stack.top();
}

//---------------------------------------------------------------------
/** polygonize this is where everything is going on..
 */

void CqBlobby::polygonize(std::vector<CqVector3D>& Vertices, std::vector<CqVector3D>& Normals, std::vector<std::vector<TqInt> >& Polygons, TqFloat ShadingRate)
{
   // Set up polygonizer

   TqInt experimental = 10;

   if (m_IsComplex)
       experimental = 12;


   bloomenthal_polygonizer polygonizer(
		ShadingRate, // Voxel size
		0.421875, // Threshold (blobby specific)
		-experimental, experimental,// Lattice X min-max
		-experimental, experimental, // Lattice Y min-max
		-experimental, experimental, // Lattice Z min-max
		bbox.vecCross() / 2, // Lattice center
		static_cast<implicit_functor&>(*this),
		Vertices, Normals, Polygons);

	polygonizer.cross_limits();

	// Polygonize blobbies
	TqBool missed_blobbies = TqFalse;
	for(std::vector<CqVector3D>::const_iterator p = origins.begin(); p != origins.end(); ++p)
	{
		if(!polygonizer.polygonize_from_inside_point(*p))
		{
			missed_blobbies = TqTrue;
			break;
		}
	}

	// Second chance for missed blobbies
	if(missed_blobbies == TqTrue)
		polygonizer.polygonize_whole_grid();
}

void CqBlobby::build_stack(CqOpcode op)
{
	switch(op.name)
	{
	    case CONSTANT:
	    break;

	    case ELLIPSOID:
	    {
		TqInt f = op.index1;
		CqMatrix transformation(
		    Floats[f], Floats[f+1], Floats[f+2], Floats[f+3],
		    Floats[f+4], Floats[f+5], Floats[f+6], Floats[f+7],
		    Floats[f+8], Floats[f+9], Floats[f+10], Floats[f+11],
		    Floats[f+12], Floats[f+13], Floats[f+14], Floats[f+15]);

		grow_bound(transformation);
		origins.push_back(CqVector3D(Floats[f+12], Floats[f+13], Floats[f+14]));

		instructions.push_back(instruction(ELLIPSOID));
		instructions.push_back(instruction(transformation.Inverse()));
	    }
	    break;

	    case SEGMENT:
	    {
		TqInt f = op.index1;
		CqVector3D start(Floats[f], Floats[f+1], Floats[f+2]);
		f += 3;
		CqVector3D end(Floats[f], Floats[f+1], Floats[f+2]);
		f += 3;
		TqFloat radius = Floats[f];
		f++;
		CqMatrix transformation(
		    Floats[f], Floats[f+1], Floats[f+2], Floats[f+3],
		    Floats[f+4], Floats[f+5], Floats[f+6], Floats[f+7],
		    Floats[f+8], Floats[f+9], Floats[f+10], Floats[f+11],
		    Floats[f+12], Floats[f+13], Floats[f+14], Floats[f+15]);

		grow_bound(transformation * start);
		grow_bound(end * transformation);
		origins.push_back(start);

		instructions.push_back(instruction(SEGMENT));
		instructions.push_back(instruction(transformation));
		instructions.push_back(instruction(start));
		instructions.push_back(instruction(end));
		instructions.push_back(instruction(radius));
	    }
	    break;

	    case ADD:
	    case MULTIPLY:
	    case MIN:
	    case MAX:
	    {
		TqInt operands = op.index1;
		TqInt n = Codes[operands];
		for(TqInt i = 1; i <= n; ++i)
		{
		    build_stack(opcodes[Codes[operands + i]]);
		}

		instructions.push_back(instruction(op.name));
		instructions.push_back(instruction(n));
	    }
	    break;
	    case SUBTRACT:
	    case DIVIDE:
	    {
		TqInt operands = op.index2;
		TqInt n = 2;
	        build_stack(opcodes[operands]);
                operands = op.index1;
	        build_stack(opcodes[operands]);

		instructions.push_back(instruction(op.name));
	    }

	}
}

void CqBlobby::grow_bound(const CqMatrix& transformation, const TqFloat radius )
{
	CqBound unit_box(- 0.5, - 0.5, - 0.5,  0.5,  0.5,  0.5);
	unit_box.Transform(transformation);

	bbox.Encapsulate( unit_box);
}

void CqBlobby::grow_bound(const CqVector3D& vector, const TqFloat radius )
{
	bbox.Encapsulate(vector);
}

END_NAMESPACE( Aqsis )
//---------------------------------------------------------------------


