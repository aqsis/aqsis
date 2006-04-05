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

#include <math.h>
#include <vector>
#include <list>
#include <limits>

#include "aqsis.h"
#include "ri.h"
#include "vector4d.h"
#include "matrix.h"
#include "blobby.h"
#include "itexturemap.h"

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

/*
* When 0<=r<=2, bump(r) is the polynomial of lowest degree with
*      bump(0)=0
*      bump'(0)=0
*      bump"(0)=0
*      bump(1)=1
*      bump(2)=0
*      bump'(2)=0
*      bump"(2)=0
*/
inline const TqFloat bump(TqFloat r)
{
	if(r<=0.0f || r>=2.0f)
		return 0.0f;
	return (((6.-r)*r-12.)*r+8.)*r*r*r;
}
/*
* When 0<=r<=1, ease(r) is the polynomial of lowest degree with
*      ease(0)=0
*      ease'(0)=0
*      ease(1)=1
*      ease'(1)=0
*/
inline const TqFloat ease(TqFloat r)
{
	if(r<=0.0f)
		return 0.0f;
	if(r>=1.0f)
		return 1.0f;
	return r*r*(3.0f-2.0f*r);
}

#define ZCLAMP  1e-6
TqFloat repulsion(TqFloat z, TqFloat A, TqFloat B, TqFloat C, TqFloat D)
{
	if(z>=A)
		return 0.0f;
	if(z<=ZCLAMP)
		z=ZCLAMP;
	return (D*bump(z/C)-B/z)*(1.0f-ease(z/A));
}
inline const CqMatrix scaling3D(const CqVector3D& v)
{
	return CqMatrix(
	           v.x(), 0, 0, 0,
	           0, v.y(), 0, 0,
	           0, 0, v.z(), 0,
	           0, 0, 0, 1);
}

CqBlobby::CqBlobby(TqInt nleaf, TqInt ncode, TqInt* code, TqInt nflt, TqFloat* flt, TqInt nStr, char **str) : Codes(code), Floats(flt), Strings(str)
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
				case 0:
				current_opcode = ADD;
				break;
				case 1:
				current_opcode = MULTIPLY;
				break;
				case 2:
				current_opcode = MAX;
				break;
				case 3:
				current_opcode = MIN;
				break;
				case 4:
				current_opcode = DIVIDE;
				break;
				case 5:
				current_opcode = SUBTRACT;
				break;
				case 6:
				current_opcode = NEGATE;
				break;
				case 7:
				current_opcode = IDEMPOTENTATE;
				break;

				case 1000:
				current_opcode = CONSTANT;
				break;
				case 1001:
				current_opcode = ELLIPSOID;
				break;
				case 1002:
				current_opcode = SEGMENT;
				break;
				case 1003:
				current_opcode = PLANE;
				break;
				default:
				Aqsis::log() << warning << "Unknown Opcode for Blobby " << code[c-1] << std::endl;
				current_opcode =  UNKNOWN;
				break;
		}

		switch(current_opcode)
		{
				case CONSTANT:
				opcodes.push_back(CqOpcode(current_opcode, code[c++]));
				break;

				case ELLIPSOID:
				case SEGMENT:
				opcodes.push_back(CqOpcode(current_opcode, code[c++]));
				break;

				case PLANE:
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
				default:
				// Hopefull the following number is to skipped the unknown opcode
				TqInt n_ops = code[c];
				c += n_ops + 1;
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
				case PLANE:
				{
					TqInt which = instructions[pc++].value;
					TqInt n = instructions[pc++].value;

					CqString depthname = Strings[which];
					IqTextureMap* pMap = QGetRenderContextI() ->GetShadowMap( depthname );

					TqFloat A, B, C, D;
					std::valarray<TqFloat> fv;
					fv.resize(1);
					fv[0]= 0.0f;


					A = Floats[n];
					B = Floats[n+1];
					C = Floats[n+2];
					D = Floats[n+3];

					if ( pMap != 0 && pMap->IsValid() )
					{
						CqVector3D swidth = 0.0f, twidth = 0.0f;
						CqVector3D _aq_P = Point;
						pMap->SampleMap( _aq_P, swidth, twidth, fv, 0 );
					}
					else
					{
						Aqsis::log() << warning << depthname << " is not found " << std::endl;
						fv[0] = A + 1.0;
					}

					Aqsis::log() << info << "A " << A << " B " << B << " C " << C << " D " << D << std::endl;
					TqFloat result = repulsion(fv[0], A, B, C, D);
					stack.push(result);
				}
				break;
				case SEGMENT:
				{

					const CqMatrix m = instructions[pc++].get_matrix();
					const CqVector3D start = instructions[pc++].get_vector();
					const CqVector3D end = instructions[pc++].get_vector();
					const TqFloat radius = instructions[pc++].value;

					if ((SegPoint == Point) &&
					    (SegRadius == radius) &&
					    (SegMatrix == m))
					{
						stack.push(SegRes);
						break;
					}
					SegMatrix = m;
					SegRadius = radius;
					// I don't think I convert the equation correctly...
					//
					//
					//TqFloat r2 = (k3d::inverse(k3d::translation3D(nearest_segment_point(Point, start, end)) *
					//              k3d::scaling3D(radius) * m) * Point).Magnitude2();
					//
					CqVector3D nearpt = nearest_segment_point(Point, start, end);
					CqMatrix neareast =   (radius * translation3D(nearpt)  * m);
					CqVector3D probably  = neareast * Point;

					TqFloat r2 = probably.Magnitude2();

					Aqsis::log() << info << "Point " << Point << " nearpt " << nearpt << " r2 " << r2 << std::endl;
					TqFloat result = r2 <= 1 ? (1 - 3*r2 + 3*r2*r2 - r2*r2*r2) : 0;
					SegRes = result;
					SegPoint = Point;
					stack.push(result);
				}
				break;
				case SUBTRACT:
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
				}
				break;
				case DIVIDE:
				{
					TqFloat a = stack.top();
					stack.pop();
					TqFloat b = stack.top();
					stack.pop();

					stack.push(b - a);
				}
				break;
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

	const CqVector3D mid = (bbox.vecMax() + bbox.vecMin())/2.0;
	const CqVector3D length =  (bbox.vecMax() - bbox.vecMin());

	TqInt n_x_over_2 = static_cast<TqInt>((fabs(length.x())+0.5) / ShadingRate / 2) + 1;
	TqInt n_y_over_2 = static_cast<TqInt>((fabs(length.y())+0.5) / ShadingRate / 2) + 1;
	TqInt n_z_over_2 = static_cast<TqInt>((fabs(length.z())+0.5) / ShadingRate / 2) + 1;

	bloomenthal_polygonizer polygonizer(
	    ShadingRate, // Voxel size
	    0.421875, // Threshold (blobby specific)
	    -n_x_over_2, n_x_over_2,// Lattice X min-max
	    -n_y_over_2, n_y_over_2, // Lattice Y min-max
	    -n_z_over_2, n_z_over_2, // Lattice Z min-max
	    bbox.vecCross() / 2, // Lattice center
	    static_cast<implicit_functor&>(*this),
	    Vertices, Normals, Polygons);

	polygonizer.cross_limits();

	// Polygonize blobbies
	TqBool missed_blobbies = TqFalse;
	std::vector<CqVector3D>::const_iterator p;
	for(p = origins.begin(); p != origins.end(); ++p)
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

	missed_blobbies = TqFalse;
	for( p = origins.begin(); p != origins.end(); ++p)
		if(!polygonizer.polygonize_from_inside_point(*p))
			missed_blobbies = TqTrue;

	if (missed_blobbies)
		polygonizer.polygonize_whole_grid();
}

void CqBlobby::build_stack(CqOpcode op)
{
	switch(op.name)
	{
			case CONSTANT:
			break;

			case PLANE:
			{
				instructions.push_back(instruction(PLANE));
				instructions.push_back(instruction(op.index1));
				instructions.push_back(instruction(op.index2));
				Aqsis::log() << log << "id1 " << op.index1 << " id2 " << op.index2 << std::endl;
			}
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

				grow_bound(transformation * start, radius);
				grow_bound(transformation * end, radius);
				origins.push_back(start);

				instructions.push_back(instruction(SEGMENT));
				instructions.push_back(instruction(transformation.Inverse()));
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
				build_stack(opcodes[op.index1]);
				build_stack(opcodes[op.index2]);

				instructions.push_back(instruction(op.name));
			}

	}
}

void CqBlobby::grow_bound(const CqMatrix& transformation, const TqFloat radius )
{
	TqFloat r = 0.5 * radius;
	CqBound unit_box(- r, - r, - r, r, r, r);
	unit_box.Transform(transformation);

	bbox.Encapsulate( unit_box);
}

void CqBlobby::grow_bound(const CqVector3D& vector, const TqFloat radius )
{
	bbox.Encapsulate(vector);
}




END_NAMESPACE( Aqsis )
//---------------------------------------------------------------------


