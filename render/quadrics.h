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
		\brief Declares the standard RenderMan quadric primitive classes.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is .h included already?
#ifndef QUADRICS_H_INCLUDED
#define QUADRICS_H_INCLUDED 1

#include	"surface.h"
#include	"micropolygon.h"

#include	"specific.h"	// Needed for namespace macros.

START_NAMESPACE(Aqsis)


#define	ESTIMATEGRIDSIZE	10

//----------------------------------------------------------------------
/** \class CqQuadric
 * Abstract base class from which all quadric primitives are defined.
 */

class CqQuadric : public CqSurface
{
	public:
	virtual				~CqQuadric()	{}

	virtual void		Transform(const CqMatrix& matTx, const CqMatrix& matITTx, const CqMatrix& matRTx);
						/** Get the number of uniform values for this GPrim.
						 */
	virtual	TqInt		cUniform() const					{return(1);}
						/** Get the number of varying values for this GPrim.
						 */
	virtual	TqInt		cVarying() const					{return(4);}
						/** Get the number of vertex values for this GPrim.
						 */
	virtual	TqInt		cVertex() const						{return(4);}


	// Overrides from CqSurface
	virtual	CqMicroPolyGridBase* Dice();
	virtual TqBool		Diceable();

			void		EqtimateGridSize();

						/** Pure virtual, get a surface point.
						 * \param u Surface u coordinate.
						 * \param v Surface v coordinate.
						 * \return 3D vector representing the surface point at the specified u,v coordniates.
						 */
	virtual	CqVector3D	DicePoint(TqInt u, TqInt v)=0;
						/** Pure virtual, get a surface point and normal.
						 * \param u Surface u coordinate.
						 * \param v Surface v coordinate.
						 * \param Normal Storage for the surface normal.
						 * \return 3D vector representing the surface point at the specified u,v coordniates.
						 */
	virtual	CqVector3D	DicePoint(TqInt u, TqInt v, CqVector3D& Normal)=0;

			CqQuadric&	operator=(const CqQuadric& From);

	protected:
			CqMatrix	m_matTx;		///< Transformation matrix from object to camera.
			CqMatrix	m_matITTx;		///< Inverse transpose transformation matrix, for transforming normals.

			TqInt		m_uDiceSize;	///< Calculated dice size in u direction.
			TqInt		m_vDiceSize;	///< Calculated dice size in v direction.
};



//----------------------------------------------------------------------
/** \class CqSphere
 * Sphere quadric GPrim.
 */

class CqSphere : public CqQuadric
{
	public:
				CqSphere(TqFloat radius,TqFloat zmin,TqFloat zmax,TqFloat thetamin,TqFloat thetamax);
				CqSphere(const CqSphere& From)	{*this=From;}
	virtual		~CqSphere()	{}

	virtual	CqBound		Bound() const;
	virtual	TqInt		Split(std::vector<CqBasicSurface*>& aSplits);
	
	virtual	CqVector3D	DicePoint(TqInt u, TqInt v);
	virtual	CqVector3D	DicePoint(TqInt u, TqInt v, CqVector3D& Normal);

			CqSphere&	operator=(const CqSphere& From);

	private:
			TqFloat		m_Radius;		///< Radius.
			TqFloat		m_ZMin;			///< Min value on z axis.
			TqFloat		m_ZMax;			///< Max value on z axis.
			TqFloat		m_ThetaMin;		///< Min angle about z axis.
			TqFloat		m_ThetaMax;		///< Max angle about z axis.
};


//----------------------------------------------------------------------
/** \class CqCone
 * Cone quadric GPrim.
 */

class CqCone : public CqQuadric
{
	public:
				CqCone(TqFloat height,TqFloat radius,TqFloat thetamin,TqFloat thetamax,TqFloat zmin,TqFloat zmax);
				CqCone(const CqCone& From)	{*this=From;}
	virtual		~CqCone()	{}

	virtual	CqBound		Bound() const;
	virtual	TqInt		Split(std::vector<CqBasicSurface*>& aSplits);
	
	virtual	CqVector3D	DicePoint(TqInt u, TqInt v);
	virtual	CqVector3D	DicePoint(TqInt u, TqInt v, CqVector3D& Normal);

			CqCone&	operator=(const CqCone& From);

	private:
			TqFloat		m_Height;		///< Height..
			TqFloat		m_Radius;		///< Radius.
			TqFloat		m_ZMin;			///< Min value on z axis.
			TqFloat		m_ZMax;			///< Max value on z axis.
			TqFloat		m_ThetaMin;		///< Min angle about z axis.
			TqFloat		m_ThetaMax;		///< Max angle about z axis.
};


//----------------------------------------------------------------------
/** \class CqCylinder
 * Cylinder quadric GPrim.
 */

class CqCylinder : public CqQuadric
{
	public:
				CqCylinder(TqFloat radius,TqFloat zmin,TqFloat zmax, TqFloat thetamin,TqFloat thetamax);
				CqCylinder(const CqCylinder& From)	{*this=From;}
	virtual		~CqCylinder()	{}

	virtual	CqBound		Bound() const;
	virtual	TqInt		Split(std::vector<CqBasicSurface*>& aSplits);
	
	virtual	CqVector3D	DicePoint(TqInt u, TqInt v);
	virtual	CqVector3D	DicePoint(TqInt u, TqInt v, CqVector3D& Normal);

			CqCylinder&	operator=(const CqCylinder& From);

	private:
			TqFloat		m_Radius;		///< Radius
			TqFloat		m_ZMin;			///< Min value on zaxis.
			TqFloat		m_ZMax;			///< Max value on z axis.
			TqFloat		m_ThetaMin;		///< Min angle about z axis.
			TqFloat		m_ThetaMax;		///< Max angle about z axis.
};


//----------------------------------------------------------------------
/** \class CqHyperboloid
 * Hyperboloid quadric GPrim.
 */

class CqHyperboloid : public CqQuadric
{
	public:
				CqHyperboloid(CqVector3D& point1,CqVector3D& point2,TqFloat thetamin,TqFloat thetamax);
				CqHyperboloid(const CqHyperboloid& From)	{*this=From;}
	virtual		~CqHyperboloid()	{}

	virtual	CqBound		Bound() const;
	virtual	TqInt		Split(std::vector<CqBasicSurface*>& aSplits);
	
	virtual	CqVector3D	DicePoint(TqInt u, TqInt v);
	virtual	CqVector3D	DicePoint(TqInt u, TqInt v, CqVector3D& Normal);

			CqHyperboloid&	operator=(const CqHyperboloid& From);

	private:
			CqVector3D	m_Point1;		///< Start point of line to revolve.
			CqVector3D	m_Point2;		///< End point of line to revolve.
			TqFloat		m_ThetaMin;		///< Min angle about z axis.
			TqFloat		m_ThetaMax;		///< Max angle about z axis.
};


//----------------------------------------------------------------------
/** \class CqParaboloid
 * Paraboloid quadric GPrim.
 */

class CqParaboloid : public CqQuadric
{
	public:
				CqParaboloid(TqFloat rmax,TqFloat zmin,TqFloat zmax,TqFloat thetamin,TqFloat thetamax);
				CqParaboloid(const CqParaboloid& From)	{*this=From;}
	virtual		~CqParaboloid()	{}

	virtual	CqBound		Bound() const;
	virtual	TqInt		Split(std::vector<CqBasicSurface*>& aSplits);
	
	virtual	CqVector3D	DicePoint(TqInt u, TqInt v);
	virtual	CqVector3D	DicePoint(TqInt u, TqInt v, CqVector3D& Normal);

			CqParaboloid&	operator=(const CqParaboloid& From);

	private:
			TqFloat		m_RMax;			///< Radius at zmax.
			TqFloat 		m_ZMin;			///< Min value on z axis.
			TqFloat 		m_ZMax;			///< Max value on z axis.
			TqFloat		m_ThetaMin;		///< Min angle about z axis.
			TqFloat		m_ThetaMax;		///< Max angle about z axis.
};


//----------------------------------------------------------------------
/** \class CqTorus
 * Torus quadric GPrim.
 */

class CqTorus : public CqQuadric
{
	public:
				CqTorus(TqFloat majorradius,TqFloat minorradius,TqFloat phimin,TqFloat phimax,TqFloat thetamin,TqFloat thetamax);
				CqTorus(const CqTorus& From)	{*this=From;}
	virtual		~CqTorus()	{}

	virtual	CqBound		Bound() const;
	virtual	TqInt		Split(std::vector<CqBasicSurface*>& aSplits);
	
	virtual	CqVector3D	DicePoint(TqInt u, TqInt v);
	virtual	CqVector3D	DicePoint(TqInt u, TqInt v, CqVector3D& Normal);

			CqTorus&	operator=(const CqTorus& From);

	private:
			TqFloat		m_MajorRadius;	///< Major radius.
			TqFloat		m_MinorRadius;	///< Minor radius. 
			TqFloat 		m_PhiMin;		///< Min angle about ring.
			TqFloat 		m_PhiMax;		///< Max angle about ring.
			TqFloat		m_ThetaMin;		///< Min andle about z axis.
			TqFloat		m_ThetaMax;		///< Max angle about z axis.
};


//----------------------------------------------------------------------
/** \class CqDisk
 * Disk quadric primitive.
 */

class CqDisk : public CqQuadric
{
	public:
				CqDisk(TqFloat height,TqFloat minorradius, TqFloat majorradius,TqFloat thetamin,TqFloat thetamax);
				CqDisk(const CqDisk& From)	{*this=From;}
	virtual		~CqDisk()	{}

	virtual	CqBound		Bound() const;
	virtual	TqInt		Split(std::vector<CqBasicSurface*>& aSplits);
	
	virtual	CqVector3D	DicePoint(TqInt u, TqInt v);
	virtual	CqVector3D	DicePoint(TqInt u, TqInt v, CqVector3D& Normal);

			CqDisk&	operator=(const CqDisk& From);

	private:
			TqFloat		m_Height;			///< Position on z axis. 
			TqFloat		m_MajorRadius;		///< Outer radius of disk.
			TqFloat		m_MinorRadius;		///< Inner radius of disk.
			TqFloat		m_ThetaMin;			///< Min angle about z axis.
			TqFloat		m_ThetaMax;			///< Max angle about z axis.
};


//-----------------------------------------------------------------------

END_NAMESPACE(Aqsis)

#endif	// !QUADRICS_H_INCLUDED
