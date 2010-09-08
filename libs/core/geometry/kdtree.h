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
		\brief Implements a generic templatised K-DTree handling class.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/


#ifndef	KDTREE_H_LOADED
#define	KDTREE_H_LOADED

#include	<aqsis/aqsis.h>
#include	<vector>

namespace Aqsis {


//------------------------------------------------------------------------------
/**
 *	Interface to the data handled by the K-DTree, implementation provided.
 */

template<class T>
struct IqKDTreeData
{
	virtual ~IqKDTreeData()
	{
	};

	/** Function to sort the elements in the given array, in ascending order based on the specified dimension index.
	 */
	virtual void PartitionElements(std::vector<T>& leavesIn, TqInt dimension,
								   std::vector<T>& out1, std::vector<T>& out2) = 0;
	/** Return the number of dimensions in the tree data.
	 */
	virtual TqInt Dimensions() const = 0;
#ifdef _DEBUG

	CqString className() const
	{
		return CqString("IqKDTreeData");
	}
#endif
};


template<class T>
class CqKDTree
{
	public:
		CqKDTree(IqKDTreeData<T>* pDataInterface)	: m_pDataInterface( pDataInterface ), m_Dim( 0 )
		{}
		virtual	~CqKDTree()
		{
		}

		void	Subdivide( CqKDTree<T>& side1, CqKDTree<T>& side2 )
		{
			m_pDataInterface->PartitionElements(m_aLeaves, m_Dim,
					side1.aLeaves(), side2.aLeaves());

			side1.m_Dim = ( m_Dim + 1 ) % m_pDataInterface->Dimensions();
			side2.m_Dim = ( m_Dim + 1 ) % m_pDataInterface->Dimensions();
		}

		/// Accessor for leaves array
		std::vector<T>&	aLeaves()
		{
			return(m_aLeaves);
		}
		const std::vector<T>&	aLeaves() const
		{
			return(m_aLeaves);
		}

	private:
		std::vector<T>		m_aLeaves;
		IqKDTreeData<T>*	m_pDataInterface;
		TqInt				m_Dim;
};

} // namespace Aqsis

#endif	//	KDTREE_H_LOADED
