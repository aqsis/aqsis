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
		\brief Implements the CqSurfaceNurbs classes for handling Renderman NURBS primitives.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include	<math.h>

#include	<stdio.h>

#include	"aqsis.h"
#include	"nurbs.h"
#include	"micropolygon.h"
#include	"irenderer.h"
#include	"vector3d.h"
#include	"imagebuffer.h"
#include	"bilinear.h"

START_NAMESPACE(Aqsis)


//---------------------------------------------------------------------
/** Constructor.
 */

CqSurfaceNURBS::CqSurfaceNURBS() : CqSurface()
{
}

//---------------------------------------------------------------------
/** Copy constructor.
 */

CqSurfaceNURBS::CqSurfaceNURBS(const CqSurfaceNURBS& From) :
			CqSurface(From)
{
	*this=From;
}


//---------------------------------------------------------------------
/** Assignment operator.
 */

void CqSurfaceNURBS::operator=(const CqSurfaceNURBS& From)
{
	// Use the CqSurface assignment operator.
	CqSurface::operator=(From);

	// Initialise the NURBS surface.
	Init(From.m_uOrder, From.m_vOrder, From.m_cuVerts, From.m_cvVerts);

	// Copy the knot vectors.
	TqInt i;
	for(i=From.m_auKnots.size()-1; i>=0; i--)
		m_auKnots[i]=From.m_auKnots[i];
	for(	  i=From.m_avKnots.size()-1; i>=0; i--)
		m_avKnots[i]=From.m_avKnots[i];
}


//---------------------------------------------------------------------
/** Comparison operator.
 */

TqInt CqSurfaceNURBS::operator==(const CqSurfaceNURBS& from)
{
	if(from.m_cuVerts!=m_cuVerts || from.m_cvVerts!=m_cvVerts)
		return(0);

	if(from.m_uOrder!=m_uOrder || from.m_vOrder!=m_vOrder)
		return(0);

	TqInt i;
	for(i=P().Size()-1; i>=0; i--)
	{
		if(P()[i]!=from.P()[i])
			return(0);
	}

	for(i=m_auKnots.size()-1; i>=0; i--)
	{
		if(m_auKnots[i]!=from.m_auKnots[i])
			return(0);
	}

	for(i=m_avKnots.size()-1; i>=0; i--)
	{
		if(m_avKnots[i]!=from.m_avKnots[i])
			return(0);
	}
	return(1);
}


//---------------------------------------------------------------------
/** Find the span in the U knot vector containing the specified parameter value.
 */

TqInt CqSurfaceNURBS::FindSpanU(TqFloat u) const
{
	if(u>=m_auKnots[m_cuVerts]) 
		return(m_cuVerts-1);
	if(u<=m_auKnots[uDegree()])
		return(uDegree());
	
	TqInt low=0;
	TqInt high=m_cuVerts+1; 
	TqInt mid=(low+high)/2;

	while(u<m_auKnots[mid] || u>=m_auKnots[mid+1])
	{
		if(u<m_auKnots[mid])
			high=mid;
		else
			low=mid;
		mid=(low+high)/2;
	}
	return(mid);  
}


//---------------------------------------------------------------------
/** Find the span in the V knot vector containing the specified parameter value.
 */

TqInt CqSurfaceNURBS::FindSpanV(TqFloat v) const
{
	if(v>=m_avKnots[m_cvVerts]) 
		return(m_cvVerts-1);
	if(v<=m_avKnots[vDegree()])
		return(vDegree());
	
	TqInt low=0;
	TqInt high=m_cvVerts+1; 
	TqInt mid=(low+high)/2;

	while(v<m_avKnots[mid] || v>=m_avKnots[mid+1])
	{
		if(v<m_avKnots[mid])
			high=mid;
		else
			low=mid;
		mid=(low+high)/2;
	}
	return(mid);  
}


//---------------------------------------------------------------------
/** Return the basis functions for the specified parameter value.
 */

void CqSurfaceNURBS::BasisFunctions(TqFloat u, TqInt span, std::vector<TqFloat>& aKnots, TqInt k, std::vector<TqFloat>& BasisVals)
{
    register TqInt r, s, i;
    register double omega;

	BasisVals[0]=1.0;
	for(r=2; r<=k; r++)
	{
		i=span-r+1;
		BasisVals[r-1]=0.0;
		for(s=r-2; s>=0; s--)
		{
			i++;
			if(i < 0)
				omega=0;
			else
				omega=(u-aKnots[i])/(aKnots[i+r-1]-aKnots[i]);

			BasisVals[s+1]=BasisVals[s+1]+(1-omega)*BasisVals[s];
			BasisVals[s]=omega*BasisVals[s];
		}
	}
}


//---------------------------------------------------------------------
/** Evaluate the nurbs surface at parameter values u,v.
 */

CqVector4D	CqSurfaceNURBS::Evaluate(TqFloat u, TqFloat v)
{
	std::vector<TqFloat> bu(m_uOrder);
	std::vector<TqFloat> bv(m_vOrder);

	CqVector4D r(0,0,0,0);

	/* Evaluate non-uniform basis functions (and derivatives) */

	TqInt uspan=FindSpanU(u);
	TqInt ufirst=uspan-m_uOrder+1;
	BasisFunctions(u, uspan, m_auKnots, m_uOrder, bu);

	TqInt vspan=FindSpanV(v);
	TqInt vfirst=vspan-m_vOrder+1;
	BasisFunctions(v, vspan, m_avKnots, m_vOrder, bv);

	// Weight control points against the basis functions

	TqInt i;
	for(i=0; i<m_vOrder; i++)
	{
		TqInt j;
		for(j=0; j<m_uOrder; j++)
		{
			TqInt ri=m_vOrder-1L-i;
			TqInt rj=m_uOrder-1L-j;

			TqFloat tmp=bu[rj]*bv[ri];
			CqVector4D& cp=CP(j+ufirst,i+vfirst);
			//r+=cp*tmp;
			r.x(r.x()+cp.x()*tmp);
			r.y(r.y()+cp.y()*tmp);
			r.z(r.z()+cp.z()*tmp);
			r.h(r.h()+cp.h()*tmp);
		}
	}
	return(r);
}


//---------------------------------------------------------------------
/** Insert the specified knot into the U knot vector, and refine the control points accordingly.
 * \return The number of new knots created.
 */

TqInt CqSurfaceNURBS::InsertKnotU(TqFloat u, TqInt r)
{
	// Work on a copy.
	CqSurfaceNURBS nS(*this);

	// Compute k and s      u = [ u_k , u_k+1)  with u_k having multiplicity s
	TqInt k=m_auKnots.size()-1,s=0;
	TqInt i,j;
	TqInt p=uDegree();

	if(u<m_auKnots[uDegree()] || u>m_auKnots[m_cuVerts])
	{
		return(0);
	}

	for(i=0; i<m_auKnots.size(); i++)
	{
		if(m_auKnots[i]>u)
		{
			k=i-1;
			break;
		}
	}

	if(u<=m_auKnots[k])
	{
		s=1;
		for(i=k; i>0; i--)
		{
			if(m_auKnots[i]<=m_auKnots[i-1])
				s++;
			else
				break;
		}
	}
	else
		s=0;

	if((r+s)>p+1)
		r=p+1-s;

	if(r<=0)
		return(0); 

	nS.Init(m_uOrder, m_vOrder, m_cuVerts+r, m_cvVerts);

	// Load new knot vector
	for(i=0;i<=k;i++)	nS.m_auKnots[i]=m_auKnots[i];
	for(i=1;i<=r;i++)	nS.m_auKnots[k+i]=u;
	for(i=k+1;i<m_auKnots.size(); i++)
		nS.m_auKnots[i+r]=m_auKnots[i];

	// Save unaltered control points
	std::vector<CqVector4D> R(p+1);

	// Insert control points as required on each row.
	TqInt row;
	for(row=0; row<m_cvVerts; row++)
	{
		for(i=0; i<=k-p; i++)			nS.CP(i,row)=CP(i,row);
		for(i=k-s; i<m_cuVerts; i++)	nS.CP(i+r,row)=CP(i,row);
		for(i=0; i<=p-s; i++)			R[i]=CP(k-p+i,row);

		// Insert the knot r times
		int L=0 ;
		TqFloat alpha;
		for(j=1; j<=r; j++)
		{
			L=k-p+j;
			for(i=0;i<=p-j-s;i++)
			{
				alpha=(u-m_auKnots[L+i])/(m_auKnots[i+k+1]-m_auKnots[L+i]);
				R[i]=CqVector4D(alpha*R[i+1].x()+(1.0-alpha)*R[i].x(),
								alpha*R[i+1].y()+(1.0-alpha)*R[i].y(),
								alpha*R[i+1].z()+(1.0-alpha)*R[i].z(),
								alpha*R[i+1].h()+(1.0-alpha)*R[i].h());
//				R[i]=alpha*R[i+1]+(1.0-alpha)*R[i];
			}
			nS.CP(L,row)=R[0];
			if(p-j-s > 0)
				nS.CP(k+r-j-s,row)=R[p-j-s];
		}

		// Load remaining control points
		for(i=L+1; i<k-s; i++)
		{
			nS.CP(i,row)=R[i-L];
		}
	}
	*this=nS;
	
	return(r); 
}


//---------------------------------------------------------------------
/** Insert the specified knot into the V knot vector, and refine the control points accordingly.
 * \return The number of new knots created.
 */

TqInt CqSurfaceNURBS::InsertKnotV(TqFloat v, TqInt r)
{
	// Work on a copy.
	CqSurfaceNURBS nS(*this);

	// Compute k and s      v = [ v_k , v_k+1)  with v_k having multiplicity s
	TqInt k=m_avKnots.size()-1,s=0;
	TqInt i,j;
	TqInt p=vDegree();

	if(v<m_avKnots[vDegree()] || v>m_avKnots[m_cvVerts])
	{
		return(0);
	}

	for(i=0; i<m_avKnots.size(); i++)
	{
		if(m_avKnots[i]>v)
		{
			k=i-1;
			break;
		}
	}

	if(v<=m_avKnots[k])
	{
		s=1;
		for(i=k; i>0; i--)
		{
			if(m_avKnots[i]<=m_avKnots[i-1])
				s++;
			else
				break;
		}
	}
	else
		s=0;

	if((r+s)>p+1)
		r=p+1-s;

	if(r<=0)
		return(0); 

	nS.Init(m_uOrder, m_vOrder, m_cuVerts, m_cvVerts+r);

	// Load new knot vector
	for(i=0;i<=k;i++)	nS.m_avKnots[i]=m_avKnots[i];
	for(i=1;i<=r;i++)	nS.m_avKnots[k+i]=v;
	for(i=k+1;i<m_avKnots.size(); i++)
		nS.m_avKnots[i+r]=m_avKnots[i];

	// Save unaltered control points
	std::vector<CqVector4D> R(p+1);

	// Insert control points as required on each row.
	TqInt col;
	for(col=0; col<m_cuVerts; col++)
	{
		for(i=0; i<=k-p; i++)			nS.CP(col,i)=CP(col,i);
		for(i=k-s; i<m_cvVerts; i++)	nS.CP(col,i+r)=CP(col,i);
		for(i=0; i<=p-s; i++)			R[i]=CP(col,k-p+i);

		// Insert the knot r times
		int L=0 ;
		TqFloat alpha;
		for(j=1; j<=r; j++)
		{
			L=k-p+j;
			for(i=0;i<=p-j-s;i++)
			{
				alpha=(v-m_avKnots[L+i])/(m_avKnots[i+k+1]-m_avKnots[L+i]);
				R[i]=CqVector4D(alpha*R[i+1].x()+(1.0-alpha)*R[i].x(),
								alpha*R[i+1].y()+(1.0-alpha)*R[i].y(),
								alpha*R[i+1].z()+(1.0-alpha)*R[i].z(),
								alpha*R[i+1].h()+(1.0-alpha)*R[i].h());
//				R[i]=alpha*R[i+1]+(1.0-alpha)*R[i];
			}
			nS.CP(col,L)=R[0];
			if(p-j-s > 0)
				nS.CP(col,k+r-j-s)=R[p-j-s];
		}

		// Load remaining control points
		for(i=L+1; i<k-s; i++)
		{
			nS.CP(col,i)=R[i-L];
		}
	}
	*this=nS;
	
	return(r); 
}



//---------------------------------------------------------------------
/** Insert the specified knots into the U knot vector, and refine the control points accordingly.
 */

void CqSurfaceNURBS::RefineKnotU(const std::vector<TqFloat>& X)
{
	if(X.size()<=0)
		return;

	int n=m_cuVerts-1;
	int p=uDegree();
	int m=n+p+1;
	int a,b;
	int r=X.size()-1;
	
	CqSurfaceNURBS nS(*this);

	nS.Init(m_uOrder, m_vOrder, r+1+n+1, m_cvVerts);

	a=FindSpanU(X[0]);
	b=FindSpanU(X[r]);
	++b;

	int j,row;
	for(row=0; row<m_cvVerts; row++)
	{
		for(j=0; j<=a-p ; j++)
			nS.CP(j,row)=CP(j,row);
		for(j=b-1; j<=n; j++)
			nS.CP(j+r+1,row)=CP(j,row);
	}

	for(j=0; j<=a; j++)
		nS.m_auKnots[j]=m_auKnots[j];
	
	for(j=b+p; j<=m; j++)
		nS.m_auKnots[j+r+1]=m_auKnots[j];
	
	int i=b+p-1; 
	int k=b+p+r;
	
	for(j=r; j>=0; j--)
	{
		while(X[j]<=m_auKnots[i] && i>a)
		{
			for(row=0; row<m_cvVerts; row++)
				nS.CP(k-p-1,row)=CP(i-p-1,row);
			nS.m_auKnots[k]=m_auKnots[i];
			--k;
			--i;
		}
		for(row=0; row<m_cvVerts; row++)
			nS.CP(k-p-1,row)=nS.CP(k-p,row);
		
		int l;
		for(l=1; l<=p ; l++)
		{
			int ind=k-p+l;
			float alpha=nS.m_auKnots[k+l]-X[j];
			if(alpha==0.0)
			{
				for(row=0; row<m_cvVerts; row++)
					nS.CP(ind-1,row)=nS.CP(ind,row);
			}
			else
			{
				alpha/=nS.m_auKnots[k+l]-m_auKnots[i-p+l];
			
				for(row=0; row<m_cvVerts; row++)
				nS.CP(ind-1,row)=CqVector4D(	alpha*nS.CP(ind-1,row).x()+(1.0-alpha)*nS.CP(ind,row).x(),
												alpha*nS.CP(ind-1,row).y()+(1.0-alpha)*nS.CP(ind,row).y(),
												alpha*nS.CP(ind-1,row).z()+(1.0-alpha)*nS.CP(ind,row).z(),
												alpha*nS.CP(ind-1,row).h()+(1.0-alpha)*nS.CP(ind,row).h());
			}
		}
		nS.m_auKnots[k]=X[j];
		--k;
	}
	*this=nS;
}


//---------------------------------------------------------------------
/** Insert the specified knots into the V knot vector, and refine the control points accordingly.
 */

void CqSurfaceNURBS::RefineKnotV(const std::vector<TqFloat>& X)
{
	if(X.size()<=0)
		return;
	
	int n=m_cvVerts-1;
	int p=vDegree();
	int m=n+p+1;
	int a,b;
	int r=X.size()-1;
	CqSurfaceNURBS nS(*this);

	nS.Init(m_uOrder, m_vOrder, m_cuVerts, r+1+n+1);

	a=FindSpanV(X[0]) ;
	b=FindSpanV(X[r]) ;
	++b;

	int j,col;
	for(col=0; col<m_cuVerts; col++)
	{
		for(j=0; j<=a-p; j++)
			nS.CP(col,j)=CP(col,j);
		for(j=b-1; j<=n; j++)
			nS.CP(col,j+r+1)=CP(col,j);
	}
	for(j=0; j<=a; j++)
		nS.m_avKnots[j]=m_avKnots[j];
	
	for(j=b+p; j<=m; j++)
		nS.m_avKnots[j+r+1]=m_avKnots[j];

	int i=b+p-1; 
	int k=b+p+r;
	
	for(j=r; j>=0 ; j--)
	{
		while(X[j]<=m_avKnots[i] && i>a)
		{
			for(col=0; col<m_cuVerts; col++)
				nS.CP(col,k-p-1)=CP(col,i-p-1);
			nS.m_avKnots[k]=m_avKnots[i];
			--k;
			--i;
		}
		for(col=0; col<m_cuVerts; col++)
			nS.CP(col,k-p-1)=nS.CP(col,k-p);
		int l;
		for(l=1; l<=p; l++)
		{
			int ind=k-p+l;
			float alpha=nS.m_avKnots[k+l]-X[j];
			if(alpha==0.0)
			{
				for(col=0; col<m_cuVerts; col++)
					nS.CP(col,ind-1)=nS.CP(col,ind);
			}
			else
			{
				alpha/=nS.m_avKnots[k+l]-m_avKnots[i-p+l];
				for(col=0; col<m_cuVerts; col++)
				nS.CP(col,ind-1)=CqVector4D(	alpha*nS.CP(col,ind-1).x()+(1.0-alpha)*nS.CP(col,ind).x(),
												alpha*nS.CP(col,ind-1).y()+(1.0-alpha)*nS.CP(col,ind).y(),
												alpha*nS.CP(col,ind-1).z()+(1.0-alpha)*nS.CP(col,ind).z(),
												alpha*nS.CP(col,ind-1).h()+(1.0-alpha)*nS.CP(col,ind).h());
			}
		}
		nS.m_avKnots[k]=X[j];
		--k;
	}
	*this=nS;
}


//---------------------------------------------------------------------
/** Ensure a nonperiodic (clamped) knot vector by inserting U[p] and U[m-p] multiple times.
 */

void CqSurfaceNURBS::ClampU()
{
	TqInt n1=InsertKnotU(m_auKnots[uDegree()],uDegree());
	TqInt n2=InsertKnotU(m_auKnots[m_cuVerts],uDegree());

	// Now trim unnecessary knots and control points
	if(n1||n2)
	{
		CqSurfaceNURBS nS(*this);
		m_auKnots.resize(m_auKnots.size()-n1-n2);
		P().SetSize((m_cuVerts-n1-n2)*m_cvVerts);
		m_cuVerts-=n1+n2;
		TqInt i;
		for(i=n1; i<nS.m_auKnots.size()-n2; i++)
			m_auKnots[i-n1]=nS.m_auKnots[i];
		TqInt row;
		for(row=0; row<m_cvVerts; row++)
		{
			TqInt i;
			for(i=n1; i<nS.m_cuVerts-n2; i++)
				CP(i-n1,row)=nS.CP(i,row);
		}
	}
}


//---------------------------------------------------------------------
/** Ensure a nonperiodic (clamped) knot vector by inserting V[p] and V[m-p] multiple times.
 */

void CqSurfaceNURBS::ClampV()
{
	TqInt n1=InsertKnotV(m_avKnots[vDegree()],vDegree());
	TqInt n2=InsertKnotV(m_avKnots[m_cvVerts],vDegree());

	// Now trim unnecessary knots and control points
	if(n1||n2)
	{
		CqSurfaceNURBS nS(*this);
		m_avKnots.resize(m_avKnots.size()-n1-n2);
		P().SetSize((m_cvVerts-n1-n2)*m_cuVerts);
		m_cvVerts-=n1+n2;
		TqInt i;
		for(i=n1; i<nS.m_avKnots.size()-n2; i++)
			m_avKnots[i-n1]=nS.m_avKnots[i];
		TqInt col;
		for(col=0; col<m_cuVerts; col++)
		{
			TqInt i;
			for(i=n1; i<nS.m_cvVerts-n2; i++)
				CP(col,i-n1)=nS.CP(col,i);
		}
	}
}


//---------------------------------------------------------------------
/** Split this NURBS surface into two subsurfaces along u or v depending on dirflag (TRUE=u)
 */

void CqSurfaceNURBS::SplitNURBS(CqSurfaceNURBS& nrbA, CqSurfaceNURBS& nrbB,	TqBool dirflag)
{
	CqSurfaceNURBS tmp(*this);

    std::vector<TqFloat>& aKnots=(dirflag)?m_auKnots:m_avKnots;
	TqInt Order=(dirflag)?m_uOrder:m_vOrder;

	// Add a multiplicty k knot to the knot vector in the direction
	// specified by dirflag, and refine the surface.  This creates two
	// adjacent surfaces with c0 discontinuity at the seam.
	TqInt extra=0L;
    TqInt last=(dirflag)?(m_cuVerts+m_uOrder-1):(m_cvVerts+m_vOrder-1);
//    TqInt middex=last/2;
//    TqFloat midVal=aKnots[middex];
	TqFloat midVal=(aKnots[0]+aKnots[last])/2;
	TqInt middex=(dirflag)?FindSpanU(midVal):FindSpanV(midVal);

	// Search forward and backward to see if multiple knot is already there
    TqInt i=0;
	TqInt same=0L;
	if(aKnots[middex]==midVal)
	{
		i=middex+1L;
		same=1L;
		while((i<last) && (aKnots[i]==midVal))
		{
			i++;
			same++;
		}

		i=middex-1L;
		while((i>0L) && (aKnots[i]==midVal))
		{
			i--;
			middex--;	// middex is start of multiple knot
			same++;
		}
	}

	if(i<=0L)	    // No knot in middle, must create it
	{
		midVal=(aKnots[0L]+aKnots[last])/2.0;
		middex=0;
		while(aKnots[middex+1L]<midVal)
			middex++;
		same=0L;
	}

	extra=Order-same;
    std::vector<TqFloat> anewKnots(extra);

    if(same<Order)	    // Must add knots
	{
		for(i=0; i<extra; i++)
			anewKnots[i]=midVal;
	}

	TqInt SplitPoint=(extra<Order)?middex-1L : middex;
    if(dirflag)	tmp.RefineKnotU(anewKnots);
	else		tmp.RefineKnotV(anewKnots);

	// Build the two child surfaces, and copy the data from the refined
	// version of the parent (tmp) into the two children

	// First half
    nrbA.Init(m_uOrder, m_vOrder,(dirflag)?SplitPoint+1L:m_cuVerts, (dirflag)?m_cvVerts:SplitPoint+1L);
    TqInt j;
	for(i=0L; i<nrbA.m_cvVerts; i++)
		for(j=0L; j<nrbA.m_cuVerts; j++)
			nrbA.CP(j,i)=tmp.CP(j,i);
	
	for(i=0L; i<nrbA.m_uOrder+nrbA.m_cuVerts; i++)
		nrbA.m_auKnots[i]=tmp.m_auKnots[i];
    for (i=0L; i<nrbA.m_vOrder+nrbA.m_cvVerts; i++)
		nrbA.m_avKnots[i]=tmp.m_avKnots[i];

    // Second half
    SplitPoint++;
    nrbB.Init(m_uOrder, m_vOrder,(dirflag)?tmp.m_cuVerts-SplitPoint:m_cuVerts, (dirflag)?m_cvVerts:tmp.m_cvVerts-SplitPoint);
    for(i=0L; i<nrbB.m_cvVerts; i++)
	{
		for(j=0L; j<nrbB.m_cuVerts; j++)
			nrbB.CP(j,i)=tmp.CP((dirflag)?j+SplitPoint:j,(dirflag)?i:(i+SplitPoint));
	}
	for(i=0L; i<nrbB.m_uOrder+nrbB.m_cuVerts; i++)
		nrbB.m_auKnots[i]=tmp.m_auKnots[(dirflag)?(i+SplitPoint):i];
    for(i=0L; i<nrbB.m_vOrder+nrbB.m_cvVerts; i++)
		nrbB.m_avKnots[i]=tmp.m_avKnots[(dirflag)?i:(i+SplitPoint)];
}


//---------------------------------------------------------------------
/** Split this NURBS surface into an array of Bezier segments
 */

void CqSurfaceNURBS::Decompose(std::vector<CqSurfaceNURBS>& S)
{
	int i,m,a,b,nb,mult,j,r,save,s,k,row,col ;
	TqFloat  numer,alpha ;


	//Vector<T> alphas(m_uOrder) ;
	std::vector<TqFloat> alphas(MAX(m_uOrder,m_vOrder));
	// all the surfaces will have the same knot vector in both the U and V
	// direction
	
	std::vector<TqFloat> nU(2*m_uOrder);
	for(i=0;i<nU.size()/2;++i)
		nU[i]=0;
	for(i=nU.size()/2;i<nU.size();++i)
		nU[i]=1;

	std::vector<TqFloat> nV(2*m_vOrder);
	for(i=0;i<nV.size()/2;++i)
		nV[i]=0;
	for(i=nV.size()/2;i<nV.size();++i)
		nV[i]=1;

	std::vector<CqSurfaceNURBS> Su(m_cuVerts-uDegree());
	for(i=0;i<Su.size();i++)
	{
		Su[i].Init(m_uOrder,m_vOrder,m_uOrder,m_cvVerts) ;
		Su[i].m_auKnots=nU;
		Su[i].m_avKnots=m_avKnots;
	}

	m=m_cuVerts+uDegree();
	a=uDegree();
	b=m_uOrder;
	nb=0;

	for(i=0;i<=uDegree();i++)
		for(col=0;col<m_cvVerts;col++)
			Su[nb].CP(i,col)=CP(i,col);
	
	while(b<m)
	{
		i=b;
		while(b<m && m_auKnots[b+1]<=m_auKnots[b]) b++;
		mult=b-i+1;
		if(mult<uDegree())
		{
			numer=m_auKnots[b]-m_auKnots[a];	// the enumerator of the alphas
			for(j=uDegree();j>mult;j--)		// compute and store the alphas
				alphas[j-mult-1]=numer/(m_auKnots[a+j]-m_auKnots[a]);
			r=uDegree()-mult;				// insert knot r times
			for(j=1;j<=r;j++)
			{
				save=r-j;
				s=mult+j;					// this many new points
				for(k=uDegree();k>=s;k--)
				{
					alpha = alphas[k-s];
					for(row=0;row<m_cvVerts;++row)
						Su[nb].CP(k,row)=alpha*Su[nb].CP(k,row)+(1.0-alpha)*Su[nb].CP(k-1,row);
				}
				if(b<m) // control point of next patch
					for(row=0;row<m_cvVerts;++row)
						Su[nb+1].CP(save,row)=Su[nb].CP(uDegree(),row);
			}
		}
		++nb;
		if(b<m)
		{ // initialize for next segment
			for(i=uDegree()-mult; i<=uDegree(); ++i)
				for(row=0;row<m_cvVerts;++row)
					Su[nb].CP(i,row)=CP(b-uDegree()+i,row);
			a=b;
			++b;
		}
	}
	Su.resize(nb);

	S.resize(Su.size()*(m_cvVerts-vDegree())) ;

	for(i=0;i<S.size();i++)
	{
		S[i].Init(m_uOrder,m_vOrder,m_uOrder,m_vOrder);
		S[i].m_auKnots=nU;
		S[i].m_avKnots=nV;
	}

	nb=0;

	int np;
	for(np=0;np<Su.size();++np)
	{
		for(i=0;i<=uDegree();i++)
			for(j=0;j<=vDegree();++j)
				S[nb].CP(i,j)=Su[np].CP(i,j);
		m=m_cvVerts+vDegree();
		a=vDegree() ;
		b=m_vOrder;
		while(b<m)
		{
			i=b;
			while(b<m && m_avKnots[b+1]<=m_avKnots[b]) b++;
			mult=b-i+1;
			if(mult<vDegree())
			{
				numer=m_avKnots[b]-m_avKnots[a];	// the enumerator of the alphas
				for(j=vDegree();j>mult;j--)			// compute and store the alphas
					alphas[j-mult-1]=numer/(m_avKnots[a+j]-m_avKnots[a]);
				r=vDegree()-mult;					// insert knot r times
				for(j=1;j<=r;j++)
				{
					save=r-j;
					s=mult+j; // this many new points
					for(k=vDegree();k>=s;k--)
					{
						alpha=alphas[k-s];
						for(col=0;col<=uDegree();++col)
							S[nb].CP(col,k)=alpha*S[nb].CP(col,k)+(1.0-alpha)*S[nb].CP(col,k-1);
					}				
					if(b<m) // control point of next patch
						for(col=0;col<=uDegree();++col)
							S[nb+1].CP(col,save)=S[nb].CP(col,vDegree());
				}
			}
			++nb;
			if(b<m)
			{ // initialize for next patch
				for(i=vDegree()-mult; i<= vDegree(); ++i)
					for(col=0;col<=uDegree();++col)
						S[nb].CP(col,i)=Su[np].CP(col,b-vDegree()+i);
				a=b;
				++b;
			}
		}
	}

	S.resize(nb);
}


//---------------------------------------------------------------------
/** Find the point at which two infinite lines intersect.
 * The algorithm generates a plane from one of the lines and finds the 
 * intersection point between this plane and the other line.
 * \return TqFalse if they are parallel, TqTrue if they intersect.
 */

TqBool CqSurfaceNURBS::IntersectLine(CqVector3D& P1, CqVector3D& T1, CqVector3D& P2, CqVector3D& T2, CqVector3D& P)
{
	CqVector3D	v,px;

	px=T1%(P1-T2);
	v=px%T1;

	TqFloat	t=(P1-P2)*v;
	TqFloat vw=v*T2;
	if((vw*vw)<1.0e-07)
		return(TqFalse);
	t/=vw;
	P=P2+(((P1-P2)*v)/vw)*T2 ;
	return(TqTrue);
}


//---------------------------------------------------------------------
/** Project a point onto a line, returns the projection point in p.
 */

void CqSurfaceNURBS::ProjectToLine(const CqVector3D& S, const CqVector3D& Trj, const CqVector3D& pnt, CqVector3D& p)
{
	CqVector3D a=pnt-S;  
	TqFloat fraction, denom;
	denom=Trj.Magnitude2(); 
	fraction=(denom==0.0)?0.0:(Trj*a)/denom; 
	p=fraction*Trj; 
	p+=S; 
}


//---------------------------------------------------------------------
/** Generate a circle segment between as and ae of radius r, using X,Y as the x and y axes and O as the origin.
 */

void CqSurfaceNURBS::Circle(const CqVector3D& O, const CqVector3D& X, const CqVector3D& Y, TqFloat r, TqFloat as, TqFloat ae)
{
	TqFloat theta,angle,dtheta;
	TqInt narcs;
	
	while(ae<as)
		ae+=2*RI_PI;
	
	theta=ae-as;
	if(theta<=RI_PIO2)	
		narcs=1;
	else
	{
		if(theta<=RI_PI)
			narcs=2;
		else
		{
			if(theta<=1.5*RI_PI)
				narcs=3;
			else
				narcs=4;
		}
	}
	dtheta=theta/static_cast<TqFloat>(narcs);
	TqInt n=2*narcs+1;				// n control points ;
	TqFloat w1=cos(dtheta/2.0);		// dtheta/2.0 is base angle

	CqVector3D P0,T0,P2,T2,P1;
	P0=O+r*cos(as)*X+r*sin(as)*Y; 
	T0=-sin(as)*X+cos(as)*Y;		// initialize start values
	Init(3,0,n,1);

	P()[0]=P0;
	TqInt index=0;
	angle=as;
	
	TqInt i;
	for(i=1; i<=narcs; i++)
	{
		angle+=dtheta;
		P2=O+r*cos(angle)*X+r*sin(angle)*Y;
		P()[index+2]=P2;
		T2=-sin(angle)*X+cos(angle)*Y;
		IntersectLine(P0,T0,P2,T2,P1);
		P()[index+1]=P1*w1;
		P()[index+1].h(w1);
		index+=2;
		if(i<narcs)
		{
			P0=P2;
			T0=T2;
		}
	}
	
	int j=2*narcs+1;				// load the knot vector
	for(i=0; i<3; i++)
	{
		m_auKnots[i]=0.0;
		m_auKnots[i+j]=1.0;
	}
	
	switch(narcs)
	{
		case 1: 
			break;
		
		case 2: 
			m_auKnots[3]=m_auKnots[4]=0.5;
			break ;
	
		case 3:
			m_auKnots[3]=m_auKnots[4]=1.0/3.0;
			m_auKnots[5]=m_auKnots[6]=2.0/3.0;
			break ;
	
		case 4:
			m_auKnots[3]=m_auKnots[4]=0.25;
			m_auKnots[5]=m_auKnots[6]=0.50;  
			m_auKnots[7]=m_auKnots[8]=0.75;
			break ;    
	}
}


//---------------------------------------------------------------------
/** Generate a linear segment as a nurbs curve for use in surface generation.
 */

void CqSurfaceNURBS::LineSegment(const CqVector3D& P1, CqVector3D& P2)
{
	// Initialise the curve to the required size.
	Init(2,0,2,1);

	// Fill in the knot vector.
	m_auKnots[0]=m_auKnots[1]=0.0;
	m_auKnots[2]=m_auKnots[3]=1.0;

	P()[0]=P1;
	P()[1]=P2;
}


//---------------------------------------------------------------------
/** Generate a surface by revolving a curve around a given axis by a given amount.
 */

void CqSurfaceNURBS::SurfaceOfRevolution(const CqSurfaceNURBS& profile, const CqVector3D& S, const CqVector3D& Tvec, TqFloat theta)
{
	TqFloat angle,dtheta;
	int narcs;
	int i,j;

	if(fabs(theta)>2.0*RI_PI)
	{
		if(theta<0)
			theta=-(2.0*RI_PI);
		else
			theta=2.0*RI_PI;
	}

	if(fabs(theta)<=RI_PIO2)	
		narcs=1;
	else
	{
		if(fabs(theta)<=RI_PI)
			narcs=2;
		else
		{
			if(fabs(theta)<=1.5*RI_PI)
				narcs=3;
			else
				narcs=4;
		}
	}
	dtheta=theta/static_cast<TqFloat>(narcs);

	int n=2*narcs+1;					// n control points ;
	Init(3,profile.uOrder(),n,profile.cuVerts());

	switch(narcs)
	{
		case 1:
			break;
		
		case 2: 
			m_auKnots[3]=m_auKnots[4]=0.5;
			break;
	
		case 3:
			m_auKnots[3]=m_auKnots[4]=1.0/3.0;
			m_auKnots[5]=m_auKnots[6]=2.0/3.0;
			break;
		
		case 4:
			m_auKnots[3]=m_auKnots[4]=0.25;
			m_auKnots[5]=m_auKnots[6]=0.50;  
			m_auKnots[7]=m_auKnots[8]=0.75;
			break;    
	}

	j=3+2*(narcs-1);					// loading the end knots
	for(i=0;i<3;j++,i++)
	{
		m_auKnots[i]=0.0;
		m_auKnots[j]=1.0;
	}

	m_avKnots=profile.m_auKnots;

	TqFloat wm=cos(dtheta/2.0);			// dtheta/2.0 is base angle

	std::vector<TqFloat> cosines(narcs+1);
	std::vector<TqFloat> sines(narcs+1);

	angle=0.0;
	for(i=1; i<=narcs; i++)
	{
		angle=dtheta*static_cast<TqFloat>(i);
		cosines[i]=cos(angle);
		sines[i]=sin(angle);
	}

	CqVector3D P0,T0,P2,T2,P1;

	for(j=0; j<m_cvVerts; j++)
	{
		CqVector3D O;
		CqVector3D pj(profile.CP(j,0));
		TqFloat wj=profile.CP(j,0).h();

		ProjectToLine(S,Tvec,pj,O);
		CqVector3D X,Y;
		X=pj-O;

		TqFloat r=X.Magnitude();

		if(r<1e-7)
		{
			for(i=0;i<m_cuVerts; i+=2)
			{
				CP(i,j)=O*wj;
				CP(i,j).h(wj);
				if(i+1<m_cuVerts)
				{
					CP(i+1,j)=O*cos(RI_PIO2/2);
					CP(i+1,j).h(cos(RI_PIO2/2));
				}
			}
			continue;
		}

		X.Unit();
		Y=Tvec%X;
		Y.Unit();

		P0=profile.CP(j,0);
		CP(0,j)=profile.CP(j,0);

		T0=Y;
		int index=0;

		for(i=1; i<=narcs; ++i)
		{
			angle=dtheta*static_cast<TqFloat>(i);
			P2=O+r*cosines[i]*X+r*sines[i]*Y;  
			CP(index+2,j)=P2*wj;
			CP(index+2,j).h(wj);
			T2=-sines[i]*X+cosines[i]*Y;
			IntersectLine(P0,T0,P2,T2,P1);
			CP(index+1,j)=P1*(wm*wj);
			CP(index+1,j).h(wm*wj);
			index+=2;
			if(i<narcs)
			{
				P0=P2;
				T0=T2;
			}
		}
	}
}


//---------------------------------------------------------------------
/** Subdivide a bicubic patch in the u direction, return the left side.
 */

void CqSurfaceNURBS::uSubdivide(CqSurfaceNURBS*& pnrbA, CqSurfaceNURBS*& pnrbB)
{
	pnrbA=new CqSurfaceNURBS(*this);
	pnrbB=new CqSurfaceNURBS(*this);
	SplitNURBS(*pnrbA,*pnrbB,TqTrue);

	// Subdivide the u/v vectors
	if(USES(Uses(),EnvVars_u))	pnrbA->u().uSubdivide(&pnrbB->u());
	if(USES(Uses(),EnvVars_v))	pnrbA->v().uSubdivide(&pnrbB->v());

	// Subdivide the s/t vectors
	if(USES(Uses(),EnvVars_s))	pnrbA->s().uSubdivide(&pnrbB->s());
	if(USES(Uses(),EnvVars_t))	pnrbA->t().uSubdivide(&pnrbB->t());

	// Subdivide the colors
	if(USES(Uses(),EnvVars_Cs))	pnrbA->Cs().uSubdivide(&pnrbB->Cs());
	if(USES(Uses(),EnvVars_Os))	pnrbA->Os().uSubdivide(&pnrbB->Os());
}


//---------------------------------------------------------------------
/** Subdivide a bicubic patch in the v direction, return the top side.
 */

void CqSurfaceNURBS::vSubdivide(CqSurfaceNURBS*& pnrbA, CqSurfaceNURBS*& pnrbB)
{
	pnrbA=new CqSurfaceNURBS(*this);
	pnrbB=new CqSurfaceNURBS(*this);
	SplitNURBS(*pnrbA,*pnrbB,TqFalse);

	// Subdivide the u/v vectors
	if(USES(Uses(),EnvVars_u))	pnrbA->u().vSubdivide(&pnrbB->u());
	if(USES(Uses(),EnvVars_v))	pnrbA->v().vSubdivide(&pnrbB->v());

	// Subdivide the s/t vectors
	if(USES(Uses(),EnvVars_s))	pnrbA->s().vSubdivide(&pnrbB->s());
	if(USES(Uses(),EnvVars_t))	pnrbA->t().vSubdivide(&pnrbB->t());

	// Subdivide the colors
	if(USES(Uses(),EnvVars_Cs))	pnrbA->Cs().vSubdivide(&pnrbB->Cs());
	if(USES(Uses(),EnvVars_Os))	pnrbA->Os().vSubdivide(&pnrbB->Os());
}


//---------------------------------------------------------------------
/** Return the boundary extents in object space of the surface patch
 */

CqBound CqSurfaceNURBS::Bound() const
{
	// Get the boundary in camera space.
	CqVector3D	vecA(FLT_MAX, FLT_MAX, FLT_MAX);
	CqVector3D	vecB(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	TqInt i;
	for(i=0; i<m_cuVerts*m_cvVerts; i++)
	{
		CqVector3D	vecV=P()[i];
		if(vecV.x()<vecA.x())	vecA.x(vecV.x());
		if(vecV.y()<vecA.y())	vecA.y(vecV.y());
		if(vecV.x()>vecB.x())	vecB.x(vecV.x());
		if(vecV.y()>vecB.y())	vecB.y(vecV.y());
		if(vecV.z()<vecA.z())	vecA.z(vecV.z());
		if(vecV.z()>vecB.z())	vecB.z(vecV.z());
	}
	CqBound	B;
	B.vecMin()=vecA;
	B.vecMax()=vecB;

	return(B);
}


//---------------------------------------------------------------------
/** Dice the patch into a mesh of micropolygons.
 */

CqMicroPolyGridBase* CqSurfaceNURBS::Dice()
{
	// Create a new CqMicorPolyGrid for this patch
	CqMicroPolyGrid* pGrid=new CqMicroPolyGrid(m_uDiceSize, m_vDiceSize, this);

	CqVector4D vec1;
	TqFloat diu=1.0/m_uDiceSize;
	TqFloat div=1.0/m_vDiceSize;

	TqInt lUses=Uses();

	// Dice the primitive variables.
	if(USES(lUses,EnvVars_u))	u().BilinearDice(m_uDiceSize,m_vDiceSize,&pGrid->u());
	if(USES(lUses,EnvVars_v))	v().BilinearDice(m_uDiceSize,m_vDiceSize,&pGrid->v());
	if(USES(lUses,EnvVars_s))	s().BilinearDice(m_uDiceSize,m_vDiceSize,&pGrid->s());
	if(USES(lUses,EnvVars_t))	t().BilinearDice(m_uDiceSize,m_vDiceSize,&pGrid->t());
	if(USES(lUses,EnvVars_Cs))	Cs().BilinearDice(m_uDiceSize,m_vDiceSize,&pGrid->Cs());
	if(USES(lUses,EnvVars_Os))	Os().BilinearDice(m_uDiceSize,m_vDiceSize,&pGrid->Os());
	
	pGrid->Reset();
	TqInt iv;
	for(iv=0; iv<=m_vDiceSize; iv++)
	{
		TqFloat sv=(static_cast<TqFloat>(iv)/static_cast<TqFloat>(m_vDiceSize))
					*(m_avKnots[m_cvVerts]-m_avKnots[m_vOrder-1])
					+m_avKnots[m_vOrder-1];
		TqInt iu;
		for(iu=0; iu<=m_uDiceSize; iu++)
		{
			TqFloat su=(static_cast<TqFloat>(iu)/static_cast<TqFloat>(m_uDiceSize))
						*(m_auKnots[m_cuVerts]-m_auKnots[m_uOrder-1])
						+m_auKnots[m_uOrder-1];

			pGrid->P()=Evaluate(su,sv);
			pGrid->Advance();
		}
	}
	// Only shade if the ImageBuffer mode is at least RGB
	if(pCurrentRenderer()->optCurrent().iDisplayMode()&ModeRGB)
		pGrid->Shade();

	return(pGrid);
}

//---------------------------------------------------------------------
/** Split the patch into smaller patches.
 */

TqInt CqSurfaceNURBS::Split(std::vector<CqBasicSurface*>& aSplits)
{
	// Split the surface in u or v
	CqSurfaceNURBS* pNew1;
	CqSurfaceNURBS* pNew2;
	
	if(m_SplitDir==SplitDir_U)
		uSubdivide(pNew1,pNew2);
	else
		vSubdivide(pNew1,pNew2);
	
	pNew1->SetSurfaceParameters(*this);
	pNew2->SetSurfaceParameters(*this);
	pNew1->m_fDiceable=TqTrue;
	pNew2->m_fDiceable=TqTrue;
	pNew1->m_EyeSplitCount=m_EyeSplitCount;
	pNew2->m_EyeSplitCount=m_EyeSplitCount;

	aSplits.push_back(pNew1);
	aSplits.push_back(pNew2);

	return(2);
}

//---------------------------------------------------------------------
/** Return whether or not the patch is diceable
 */

TqBool	CqSurfaceNURBS::Diceable()
{
	// Convert the control hull to raster space.
	CqVector2D*	avecHull=new CqVector2D[m_cuVerts*m_cvVerts];
	TqInt i;
	for(i=0; i<m_cuVerts*m_cvVerts; i++)
		avecHull[i]=pCurrentRenderer()->matSpaceToSpace("camera","raster",CqMatrix(),pTransform()->matObjectToWorld())*P()[i];
	
	// Now work out the longest continuous line in raster space for u and v.
	TqFloat uLen=0;
	TqFloat vLen=0;
	TqFloat MaxuLen=0;
	TqFloat MaxvLen=0;

	TqInt v;
	for(v=0; v<m_cvVerts; v++)
	{
		TqInt u;
		for(u=0; u<m_cuVerts-1; u++)
			uLen+=CqVector2D(avecHull[(v*m_cuVerts)+u+1]-avecHull[(v*m_cuVerts)+u]).Magnitude();
		if(uLen>MaxuLen)	MaxuLen=uLen;
		uLen=0;
	}

	TqInt u;
	for(u=0; u<m_cuVerts; u++)
	{
		for(v=0; v<m_cvVerts-1; v++)
			vLen+=CqVector2D(avecHull[((v+1)*m_cuVerts)+u]-avecHull[(v*m_cuVerts)+u]).Magnitude();
		if(vLen>MaxvLen)	MaxvLen=vLen;
		vLen=0;
	}

	if(!m_fDiceable)
	{
		m_SplitDir=(MaxuLen>MaxvLen)?SplitDir_U:SplitDir_V;
		delete[](avecHull);
		return(TqFalse);
	}

	if(MaxvLen>255 || MaxuLen>255)
	{
		m_SplitDir=(MaxuLen>MaxvLen)?SplitDir_U:SplitDir_V;
		delete[](avecHull);
		return(TqFalse);
	}	
	
	// TODO: Should ensure powers of half to prevent cracking.
	float ShadingRate=pAttributes()->fEffectiveShadingRate();
//	if(pCurrentRenderer()->Mode()==RenderMode_Shadows)
//	{
//		const TqFloat* pattrShadowShadingRate=m_pAttributes->GetFloatAttribute("render","shadow_shadingrate");
//		if(pattrShadowShadingRate!=0)
//			ShadingRate=pattrShadowShadingRate[0];
//	}	
	ShadingRate=static_cast<float>(sqrt(ShadingRate));
	MaxuLen/=ShadingRate;
	MaxvLen/=ShadingRate;
	m_uDiceSize=static_cast<TqInt>(MAX(MaxuLen,1));
	m_vDiceSize=static_cast<TqInt>(MAX(MaxvLen,1));
	TqFloat Area=m_uDiceSize*m_vDiceSize;

	if(MaxuLen<FLT_EPSILON || MaxvLen<FLT_EPSILON)
	{
		m_fDiscard=TqTrue;
		delete[](avecHull);
		return(TqFalse);
	}


	delete[](avecHull);
	if(fabs(Area)>256)
	{
		m_SplitDir=(MaxuLen>MaxvLen)?SplitDir_U:SplitDir_V;
		return(TqFalse);
	}
	else
		return(TqTrue);
}


//---------------------------------------------------------------------
/** Transform the patch by the specified matrix.
 */

void	CqSurfaceNURBS::Transform(const CqMatrix& matTx, const CqMatrix& matITTx, const CqMatrix& matRTx)
{
	// Tansform the control hull by the specified matrix.
	TqInt i;
	for(i=0; i<P().Size(); i++)
		P()[i]=matTx*P()[i];
}




void CqSurfaceNURBS::OutputMesh()
{
	long Granularity = 30;  // Controls the number of steps in u and v


	std::vector<CqSurfaceNURBS>	S(1);
	S[0]=*this;
//	Decompose(S);

	// Save the grid as a .raw file.
	FILE* fp=fopen("NURBS.RAW", "w");

	TqInt s;
	for(s=0; s<S.size(); s++)
	{
		fprintf(fp, "Surface_%d\n",s);
		std::vector<std::vector<CqVector3D> > aaPoints(Granularity+1);
		TqInt p;
		for(p=0; p<=Granularity; p++)	aaPoints[p].resize(Granularity+1);


		// Compute points on curve

		TqInt i;
		for(i=0; i<=Granularity; i++)
		{
			TqFloat v=(static_cast<TqFloat>(i)/static_cast<TqFloat>(Granularity))
						*(S[s].m_avKnots[S[s].m_cvVerts]-S[s].m_avKnots[S[s].m_vOrder-1])
						+S[s].m_avKnots[S[s].m_vOrder-1];

			TqInt j;
			for(j=0; j<=Granularity; j++)
			{
				TqFloat u=(static_cast<TqFloat>(j)/static_cast<TqFloat>(Granularity))
							*(S[s].m_auKnots[S[s].m_cuVerts]-S[s].m_auKnots[S[s].m_uOrder-1])
							+S[s].m_auKnots[S[s].m_uOrder-1];

				aaPoints[i][j]=S[s].Evaluate(u,v);
			}
		}


		for(i=0; i<Granularity; i++)
		{
			TqInt j;
			for(j=0; j<Granularity; j++)
			{
				fprintf(fp, "%f %f %f %f %f %f %f %f %f\n",
								aaPoints[i][j].x(),aaPoints[i][j].y(),aaPoints[i][j].z(),
								aaPoints[i+1][j+1].x(),aaPoints[i+1][j+1].y(),aaPoints[i+1][j+1].z(),
								aaPoints[i+1][j].x(),aaPoints[i+1][j].y(),aaPoints[i+1][j].z());
				fprintf(fp, "%f %f %f %f %f %f %f %f %f\n",
								aaPoints[i][j].x(),aaPoints[i][j].y(),aaPoints[i][j].z(),
								aaPoints[i][j+1].x(),aaPoints[i][j+1].y(),aaPoints[i][j+1].z(),
								aaPoints[i+1][j+1].x(),aaPoints[i+1][j+1].y(),aaPoints[i+1][j+1].z());
			}
		}
	}
	fclose(fp);
}


void CqSurfaceNURBS::Output(char* name)
{
    // Save the grid as a .out file.
	FILE* fp=fopen(name, "w");
	fprintf(fp, "Renderer V2\n\n");

	fprintf(fp, "uOrder: %d\n", m_uOrder);
	fprintf(fp, "vOrder: %d\n", m_vOrder);
	fprintf(fp, "cuVerts: %d\n", m_cuVerts);
	fprintf(fp, "cvVerts: %d\n", m_cvVerts);

	TqInt i;
	for(i=0; i<m_auKnots.size(); i++)
		fprintf(fp, "%f ", m_auKnots[i]);
	fprintf(fp,"\n");
	for(i=0; i<m_avKnots.size(); i++)
		fprintf(fp, "%f ", m_avKnots[i]);
	fprintf(fp,"\n");

	for(i=0; i<P().Size(); i++)
		fprintf(fp, "%f,%f,%f,%f\n", P()[i].x(),P()[i].y(),P()[i].z(),P()[i].h());

	fclose(fp);
}


END_NAMESPACE(Aqsis)
//---------------------------------------------------------------------
