//------------------------------------------------------------------------------
/**
 *	@file	trimcurve.cpp
 *	@author	Paul Gregory
 *	@brief	Implementation of trimcurce functionality.
 *
 *	Last change by:		$Author: pgregory $
 *	Last change date:	$Date: 2001/11/09 08:56:51 $
 */
//------------------------------------------------------------------------------


#include	"trimcurve.h"

START_NAMESPACE(Aqsis)


void CqTrimLoop::Prepare()
{
	TqInt cPoints=200;

	std::vector<CqTrimCurve>::iterator iCurve;
	TqInt iPoint;
	TqFloat u=0.0f;
	TqFloat du=1.0f/cPoints;

	for(iCurve=m_aCurves.begin(); iCurve!=m_aCurves.end(); iCurve++)
	{
		for(iPoint=0; iPoint<cPoints; iPoint++)
		{
			m_aCurvePoints.push_back(CqVector2D(iCurve->Evaluate(u)));
			u+=du;
		}
	}
}


TqBool	CqTrimLoop::TrimPoint(CqVector2D& v)
{
	TqFloat x=v.x();
	TqFloat y=v.y();
	TqInt i, j, c = 0;
	for (i = 0, j = m_aCurvePoints.size()-1; i < m_aCurvePoints.size(); j = i++)
	{
		if ((((m_aCurvePoints[i].y() <= y) && (y < m_aCurvePoints[j].y())) ||
			 ((m_aCurvePoints[j].y() <= y) && (y < m_aCurvePoints[i].y()))) &&
			  (x < (m_aCurvePoints[j].x() - m_aCurvePoints[i].x()) * (y - m_aCurvePoints[i].y()) / 
				(m_aCurvePoints[j].y() - m_aCurvePoints[i].y()) + m_aCurvePoints[i].x()))
			c = !c;
	}
	return(c==0); 
}



void CqTrimLoopArray::Prepare()
{
	std::vector<CqTrimLoop>::iterator iLoop;
	for(iLoop=m_aLoops.begin(); iLoop!=m_aLoops.end(); iLoop++)
		iLoop->Prepare();
}


TqBool	CqTrimLoopArray::TrimPoint(CqVector2D& v)
{
	TqBool	fTrim=false;

	std::vector<CqTrimLoop>::iterator iLoop;
	for(iLoop=m_aLoops.begin(); iLoop!=m_aLoops.end(); iLoop++)
		fTrim=fTrim || iLoop->TrimPoint(v);
	
	return(fTrim); 
}



//---------------------------------------------------------------------

END_NAMESPACE(Aqsis)