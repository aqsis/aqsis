//------------------------------------------------------------------------------
/**
 *	@file	trimcurve.h
 *	@author	Paul Gregory
 *	@brief	NURB based trim cureve class.
 *
 *	Last change by:		$Author: pgregory $
 *	Last change date:	$Date: 2001/11/09 08:56:51 $
 */
//------------------------------------------------------------------------------

#ifndef	___trimcurve_Loaded___
#define	___trimcurve_Loaded___


#include	<vector>

#include	"aqsis.h"

#include	"nurbs.h"
#include	"vector3d.h"

START_NAMESPACE(Aqsis)

//----------------------------------------------------------------------
/** \class CqTrimCurve
 * Trim curve, based on NURBS surface, with control to restrict to 2d.
 */

class CqTrimCurve : public CqSurfaceNURBS
{
	private:
	public:
				CqTrimCurve() : CqSurfaceNURBS()	{}
				CqTrimCurve(const CqTrimCurve& From) : CqSurfaceNURBS(From)	{}
		virtual	~CqTrimCurve()				{}

							/** Get the order of the NURBS surface in the u direction.
							 */
				TqUint		Order() const		{return(CqSurfaceNURBS::uOrder());}
							/** Get the order of the NURBS surface in the v direction.
							 */
				TqUint		Degree() const		{return(CqSurfaceNURBS::uDegree());}
							/** Get the number of control points in the u direction.
							 */
				TqUint		cVerts() const		{return(CqSurfaceNURBS::cuVerts());}
							/** Get the length of the knot vector for the u direction.
							 */
				TqUint		cKnots() const		{return(CqSurfaceNURBS::cuKnots());}
							/** Get a reference to the knot vector for the u direction.
							 */
				std::vector<TqFloat>& aKnots()	{return(CqSurfaceNURBS::auKnots());}

				void		operator=(const CqTrimCurve& From)	{CqSurfaceNURBS::operator=(From);}
				TqInt		operator==(const CqTrimCurve& from)	{return(CqSurfaceNURBS::operator==(from));}
							/** Get the control point at the specified u,v index.
							 * \param u Index in the u direction.
							 * \param v Index in the v direction.
							 * \return Reference to the 4D homogenous control point.
							 */
				CqVector4D&	CP(const TqUint u)					{return(CqSurfaceNURBS::CP(u,0));}
							/** Get the control point at the specified u,v index.
							 * \param u Index in the u direction.
							 * \param v Index in the v direction.
							 * \return Reference to the 4D homogenous control point.
							 */
		const	CqVector4D&	CP(const TqUint u) const			{return(CqSurfaceNURBS::CP(u,0));}

							/** Initialise the NURBS structures to take a NURBS surfafe of the specified dimensions.
							 * \param uOrder The required order in the u direction.
							 * \param vOrder The required order in the v direction.
							 * \param cuVerts The required control point count in the u direction.
							 * \param cvVerts The required control point count in the v direction.
							 */
				void		Init(TqUint Order, TqUint cVerts)	{CqSurfaceNURBS::Init(Order,1,cVerts,1);}
				CqVector4D	Evaluate(TqFloat u)					{return(CqSurfaceNURBS::Evaluate(u,0));}

	protected:
};



class CqTrimLoop
{
	public:
			CqTrimLoop()	{}
			~CqTrimLoop()	{}

				
				std::vector<CqTrimCurve>& aCurves()	{return(m_aCurves);}

				void		Prepare();
				TqBool		TrimPoint(CqVector2D& v);

	private:
				std::vector<CqTrimCurve>	m_aCurves;
				std::vector<CqVector2D>		m_aCurvePoints;
};


class CqTrimLoopArray
{
	public:
			CqTrimLoopArray()	{}
			~CqTrimLoopArray()	{}

				std::vector<CqTrimLoop>& aLoops()	{return(m_aLoops);}

				void		Prepare();
				TqBool		TrimPoint(CqVector2D& v);
	private:
				std::vector<CqTrimLoop>	m_aLoops;
};


//-----------------------------------------------------------------------

END_NAMESPACE(Aqsis)


#endif	//	___trimcurve_Loaded___
