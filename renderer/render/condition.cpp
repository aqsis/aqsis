// Aqsis
// Copyright 1997 - 2001, Paul C. Gregory
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
		\brief Implement the small set of the condition testing for RiElseIf() RiIfBegin().
            It take the left parameter in form of $user:test; look to the current list of 
            options and determine its type. Later it looks for the second paramter and determines
            the relation operation to be performed; the third parameter is scanned based on the
            type of the first parameter.
		\author Michel Joron (joron@sympatico.ca)
*/

#include	"aqsis.h"
#include	"renderer.h"
#include	"logging.h"


START_NAMESPACE( Aqsis )

// Logic relation
static TqUlong RIH_eq = CqString::hash("==");
static TqUlong RIH_ne = CqString::hash("!=");
static TqUlong RIH_le = CqString::hash("<=");
static TqUlong RIH_lt = CqString::hash("<");
static TqUlong RIH_ge = CqString::hash(">=");
static TqUlong RIH_gt = CqString::hash(">");

// Math relation
static TqUlong RIH_mul = CqString::hash("*");
static TqUlong RIH_div = CqString::hash("/");
static TqUlong RIH_add = CqString::hash("+");
static TqUlong RIH_sub = CqString::hash("-");


inline TqBool IsMath(TqUlong comp)
{
	if ((comp == RIH_mul) || (comp == RIH_div) || (comp == RIH_add) || (comp == RIH_sub))
		return TqTrue;
	return TqFalse;
}

template <class T>
inline TqBool MathCondition(const T& A, const T& B, TqUlong comp)
{

	if (comp == RIH_mul)
		return ((A * B) != (T) 0);
	if (comp == RIH_div)
	{
		if (B == (T) 0)
			return TqFalse; // It maybe TqTrue this is a division by zero ??
		if (A == (T) 0)
			return TqFalse;
		return ((A / B) != (T) 0);
	}
	if (comp == RIH_add)
		return ((A + B) != (T) 0);
	if (comp == RIH_sub)
		return ((A - B) != (T) 0);
	return TqTrue;
}

template <class T>
inline TqBool LogicCondition(const T& A, const T& B, TqUlong comp)
{
	if (comp == RIH_eq )
		return (A == B);
	if (comp == RIH_ne )
		return (A != B);
	if (comp == RIH_le )
		return (A <= B);
	if (comp == RIH_lt )
		return (A < B);
	if (comp == RIH_ge )
		return (A >= B);
	if (comp == RIH_gt )
		return (A > B);
	return TqFalse;
}

template <class T>
inline TqBool Condition(const T& A, const T& B, TqUlong comp)
{

	if (IsMath(comp))
		return MathCondition(A, B, comp);
	else
		return LogicCondition(A, B, comp);
}

//----------------------------------------------------------------------
/** Conditional handler for 3.04 file format. It support ==, !=, < , <=,
 *  >, >= on float, integer, point, normal, color, vector, normal parameters 
 *  and == for string parameter.
 *
 *	\param	condition	Pointer to the full condition something likes
 *                IfBegin "$user:pass == 'beauty'" or 
 *                ElseIf  "$user:pass != 'shadow'" or
 *                IfBegin "$limits:bgcolor == [0.2 0.2 0.4]"
 * \param   where  it is from IfBegin, ElseIf for printfs debugging.
 */
TqBool TestCondition(RtString condition, RtString where)
{

	char StringA[80];
	char Compare[80];


	TqBool Ok = TqTrue;
	TqInt n;

	// If the left side is not the right parameter than return TqTrue
	if ((strstr(condition, "$") == 0) || (strstr(condition, ":") == 0))
		return TqTrue;

	n = sscanf(condition,"$%s %s", StringA, Compare);

	// If the left side is ill formed return TqTrue
	if (n != 2)
		return TqTrue;


	TqUlong comp = CqString::hash(Compare);

	char *A, *B;
	A = strtok(StringA, ":");
	B = strtok(NULL, ":");

	const CqParameter *pParam = QGetRenderContext() ->optCurrent().pParameter(A,B);

	// If the left side is not known for now return TqTrue
	if (!pParam)
	{
		Aqsis::log() << warning << "Unknown parameter: " << A << ":" << B << std::endl;
		return TqTrue;
	}

	// At this point the condition must be true; otherwise the return value will be TqFalse
	Ok = TqFalse;
	switch (pParam->Type())
	{
			case type_integer:
			{
				TqInt IntC;

				const TqInt *pInt = QGetRenderContext() ->optCurrent().GetIntegerOption( A, B);

				n = sscanf(condition,"$%s %s %d", StringA, Compare, &IntC);
				if (pInt && (n == 3) )
				{
					Ok = Condition(*pInt, IntC, comp);
				}

			}
			break;

			case type_float:
			{
				TqFloat FloatC;

				const TqFloat *pFloat = QGetRenderContext() ->optCurrent().GetFloatOption( A, B);

				n = sscanf(condition,"$%s %s %f", StringA, Compare, &FloatC);
				if (pFloat && (n == 3) )
				{
					Ok = Condition(*pFloat, FloatC, comp);
				}

			}
			break;
			case type_point:
			case type_normal:
			case type_color:
			case type_vector:
			{
				TqFloat ArrayA[3], ArrayC[3];


				const TqFloat *pVector = QGetRenderContext() ->optCurrent().GetFloatOption( A, B);

				n = sscanf(condition,"$%s %s [%f %f %f]", StringA, Compare, &ArrayC[0], &ArrayC[1], &ArrayC[2]);
				if (pVector && (n == 5) )
				{
					ArrayA[0] = pVector[0];
					ArrayA[1] = pVector[1];
					ArrayA[2] = pVector[2];
					Ok = (Condition(ArrayA[0], ArrayC[0], comp) &&
					      Condition(ArrayA[1], ArrayC[1], comp) &&
					      Condition(ArrayA[2], ArrayC[2], comp));
				}
			}
			break;

			case type_string:
			{
				char StringC[80];

				const CqString *pString = QGetRenderContext() ->optCurrent().GetStringOption( A, B);

				n = sscanf(condition,"$%s %s %s", StringA, Compare, StringC);
				if ((pString) && (n == 3) )
				{
					// Only two cases "==" and "!=" are supported for string comparaison
					if (comp == RIH_eq )
					{

						if (strstr(StringC, pString->c_str()) != 0)
						{
							Ok = TqTrue;
						}
					}
					else if (comp == RIH_ne )
					{

						if (!strstr(StringC, pString->c_str()) != 0)
						{
							Ok = TqTrue;
						}
					}
				}
			}
			break;

			default:
			{
				// It is not supported than make sure we return TqTrue
				Aqsis::log() << warning << "this type of " << A << ":" << B << " are not supported yet!" << std::endl;
				Ok = TqTrue;
			}
			break;
	}

#ifdef _DEBUG
	if (Ok == TqFalse  )
		Aqsis::log() << info << where << ": " << condition << " is false" << std::endl;
	else
		Aqsis::log() << info << where << ": " << condition << " is true" << std::endl;
#endif

	return Ok;

}


END_NAMESPACE( Aqsis )

