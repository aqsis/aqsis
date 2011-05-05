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
		\brief Declares the standard RenderMan quadric primitive classes.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

//? Is .h included already?
#ifndef QUADRICS_H_INCLUDED
#define QUADRICS_H_INCLUDED 1

#include	<aqsis/aqsis.h>

#include	"surface.h"

namespace Aqsis {


#define	ESTIMATEGRIDSIZE    8

//----------------------------------------------------------------------
/** \class CqQuadric
 * Abstract base class from which all quadric primitives are defined.
 */

class CqQuadric : public CqSurface
{
	public:

		CqQuadric();
		virtual	~CqQuadric()
		{}

		CqBound	MotionBound(CqBound&	B) const;
		virtual void	Transform( const CqMatrix& matTx, const CqMatrix& matITTx, const CqMatrix& matRTx, TqInt iTime = 0 );
		/** Get the number of uniform values for this GPrim.
		 */
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
			return ( 4 );
		}
		virtual	TqUint	cFaceVarying() const
		{
			return ( cVarying() );
		}

		// Overrides from CqSurface
		virtual bool	Diceable(const CqMatrix& matCtoR);

		/** Determine whether the passed surface is valid to be used as a
		 *  frame in motion blur for this surface.
		 */
		virtual bool	IsMotionBlurMatch( CqSurface* pSurf )
		{
			return( false );
		}

		TqUlong	EstimateGridSize(const CqMatrix& matCtoR);
		void	Circle( const CqVector3D& O, const CqVector3D& X, const CqVector3D& Y, TqFloat r, TqFloat as, TqFloat ae, std::vector<CqVector3D>& points ) const;
		CqBound	RevolveForBound( const std::vector<CqVector3D>& profile, const CqVector3D& S, const CqVector3D& Tvec, TqFloat theta ) const;

		virtual TqInt DiceAll( CqMicroPolyGrid* pGrid );

		virtual void DicePoints( CqVector3D* pointGrid, CqVector3D* normalGrid ) = 0;

#ifdef _DEBUG

		CqString className() const
		{
			return CqString("CqQuadric");
		}
#endif

	protected:
		void CloneData( CqQuadric* clone ) const;

		CqMatrix	m_matTx;		///< Transformation matrix from object to camera.
		CqMatrix	m_matITTx;		///< Inverse transpose transformation matrix, for transforming normals.

};



//----------------------------------------------------------------------
/** \class CqSphere
 * Sphere quadric GPrim.
 */

class CqSphere : public CqQuadric
{
	public:
		CqSphere( TqFloat radius = 1.0f, TqFloat zmin = -1.0f, TqFloat zmax = 1.0f, TqFloat thetamin = 0.0f, TqFloat thetamax = 360.0f );
		virtual	~CqSphere()
		{}

		virtual	void	Bound(CqBound* bound) const;

		virtual void DicePoints( CqVector3D* pointGrid, CqVector3D* normalGrid );

		virtual	TqInt PreSubdivide( std::vector<boost::shared_ptr<CqSurface> >& aSplits, bool u );


#ifdef _DEBUG

		CqString className() const
		{
			return CqString("CqSphere");
		}
#endif
		virtual CqSurface* Clone() const;

	private:
		TqFloat	m_Radius;		///< Radius.
		TqFloat	m_PhiMin;		///< Min angle about x axis.
		TqFloat	m_PhiMax;		///< Max angle about x axis.
		TqFloat	m_ThetaMin;		///< Min angle about z axis.
		TqFloat	m_ThetaMax;		///< Max angle about z axis.
};


//----------------------------------------------------------------------
/** \class CqCone
 * Cone quadric GPrim.
 */

class CqCone : public CqQuadric
{
	public:
		CqCone( TqFloat height = 1.0f, TqFloat radius = 1.0f, TqFloat thetamin = 0.0f, TqFloat thetamax = 360.0f, TqFloat zmin = 0.0f, TqFloat zmax = 1.0f );
		virtual	~CqCone()
		{}

		virtual	void	Bound(CqBound* bound) const;

		virtual void DicePoints( CqVector3D* pointGrid, CqVector3D* normalGrid );

		virtual	TqInt PreSubdivide( std::vector<boost::shared_ptr<CqSurface> >& aSplits, bool u );


#ifdef _DEBUG

		CqString className() const
		{
			return CqString("CqCone");
		}
#endif
		virtual CqSurface* Clone() const;

	private:
		TqFloat	m_Height;		///< Height..
		TqFloat	m_Radius;		///< Radius.
		TqFloat	m_vMin;			///< Min value on z axis.
		TqFloat	m_vMax;			///< Max value on z axis.
		TqFloat	m_ThetaMin;		///< Min angle about z axis.
		TqFloat	m_ThetaMax;		///< Max angle about z axis.
};


//----------------------------------------------------------------------
/** \class CqCylinder
 * Cylinder quadric GPrim.
 */

class CqCylinder : public CqQuadric
{
	public:
		CqCylinder( TqFloat radius = 1.0f, TqFloat zmin = -1.0f, TqFloat zmax = 1.0f, TqFloat thetamin = 0.0f, TqFloat thetamax = 360.0f );
		virtual	~CqCylinder()
		{}

		virtual	void	Bound(CqBound* bound) const;

		virtual void DicePoints( CqVector3D* pointGrid, CqVector3D* normalGrid );

		virtual	TqInt PreSubdivide( std::vector<boost::shared_ptr<CqSurface> >& aSplits, bool u );


#ifdef _DEBUG

		CqString className() const
		{
			return CqString("CqCylinder");
		}
#endif
		virtual CqSurface* Clone() const;

	private:
		TqFloat	m_Radius;		///< Radius
		TqFloat	m_ZMin;			///< Min value on zaxis.
		TqFloat	m_ZMax;			///< Max value on z axis.
		TqFloat	m_ThetaMin;		///< Min angle about z axis.
		TqFloat	m_ThetaMax;		///< Max angle about z axis.
};


//----------------------------------------------------------------------
/** \class CqHyperboloid
 * Hyperboloid quadric GPrim.
 */

class CqHyperboloid : public CqQuadric
{
	public:
		CqHyperboloid( );
		CqHyperboloid( CqVector3D& point1, CqVector3D& point2, TqFloat thetamin, TqFloat thetamax );
		virtual	~CqHyperboloid()
		{}

		virtual	void	Bound(CqBound* bound) const;

		virtual void DicePoints( CqVector3D* pointGrid, CqVector3D* normalGrid );

		virtual	TqInt PreSubdivide( std::vector<boost::shared_ptr<CqSurface> >& aSplits, bool u );


#ifdef _DEBUG

		CqString className() const
		{
			return CqString("CqHyperboloid");
		}
#endif
		virtual CqSurface* Clone() const;

	private:
		CqVector3D	m_Point1;		///< Start point of line to revolve.
		CqVector3D	m_Point2;		///< End point of line to revolve.
		TqFloat	m_ThetaMin;		///< Min angle about z axis.
		TqFloat	m_ThetaMax;		///< Max angle about z axis.
};


//----------------------------------------------------------------------
/** \class CqParaboloid
 * Paraboloid quadric GPrim.
 */

class CqParaboloid : public CqQuadric
{
	public:
		CqParaboloid( TqFloat rmax = 1.0f, TqFloat zmin = -1.0f, TqFloat zmax = 1.0f, TqFloat thetamin = 0.0f, TqFloat thetamax = 360.0f );
		virtual	~CqParaboloid()
		{}

		virtual	void	Bound(CqBound* bound) const;

		virtual void DicePoints( CqVector3D* pointGrid, CqVector3D* normalGrid );

		virtual	TqInt PreSubdivide( std::vector<boost::shared_ptr<CqSurface> >& aSplits, bool u );


#ifdef _DEBUG

		CqString className() const
		{
			return CqString("CqParaboloid");
		}
#endif
		virtual CqSurface* Clone() const;

	private:
		TqFloat	m_RMax;			///< Radius at zmax.
		TqFloat m_ZMin;			///< Min value on z axis.
		TqFloat m_ZMax;			///< Max value on z axis.
		TqFloat	m_ThetaMin;		///< Min angle about z axis.
		TqFloat	m_ThetaMax;		///< Max angle about z axis.
};


//----------------------------------------------------------------------
/** \class CqTorus
 * Torus quadric GPrim.
 */

class CqTorus : public CqQuadric
{
	public:
		CqTorus( TqFloat majorradius = 1.0f, TqFloat minorradius = 0.2f, TqFloat phimin = 0.0f, TqFloat phimax = 360.0f, TqFloat thetamin = 0.0f, TqFloat thetamax = 360.0f );
		virtual	~CqTorus()
		{}

		virtual	void	Bound(CqBound* bound) const;

		virtual void DicePoints( CqVector3D* pointGrid, CqVector3D* normalGrid );

		virtual	TqInt PreSubdivide( std::vector<boost::shared_ptr<CqSurface> >& aSplits, bool u );


#ifdef _DEBUG

		CqString className() const
		{
			return CqString("CqTorus");
		}
#endif
		virtual CqSurface* Clone() const;

	private:
		TqFloat	m_MajorRadius;	///< Major radius.
		TqFloat	m_MinorRadius;	///< Minor radius.
		TqFloat m_PhiMin;		///< Min angle about ring.
		TqFloat m_PhiMax;		///< Max angle about ring.
		TqFloat	m_ThetaMin;		///< Min andle about z axis.
		TqFloat	m_ThetaMax;		///< Max angle about z axis.
};


//----------------------------------------------------------------------
/** \class CqDisk
 * Disk quadric primitive.
 */

class CqDisk : public CqQuadric
{
	public:
		CqDisk( TqFloat height = 0.0f, TqFloat minorradius = 0.0f, TqFloat majorradius = 1.0f, TqFloat thetamin = 0.0f, TqFloat thetamax = 360.0f );
		virtual	~CqDisk()
		{}

		virtual	void	Bound(CqBound* bound) const;

		virtual void DicePoints( CqVector3D* pointGrid, CqVector3D* normalGrid );

		virtual	TqInt PreSubdivide( std::vector<boost::shared_ptr<CqSurface> >& aSplits, bool u );


#ifdef _DEBUG

		CqString className() const
		{
			return CqString("CqDisk");
		}
#endif
		virtual CqSurface* Clone() const;

	private:
		TqFloat	m_Height;			///< Position on z axis.
		TqFloat	m_MajorRadius;		///< Outer radius of disk.
		TqFloat	m_MinorRadius;		///< Inner radius of disk.
		TqFloat	m_ThetaMin;			///< Min angle about z axis.
		TqFloat	m_ThetaMax;			///< Max angle about z axis.
};


//-----------------------------------------------------------------------

} // namespace Aqsis

#endif	// !QUADRICS_H_INCLUDED
