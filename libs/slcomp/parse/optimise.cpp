////---------------------------------------------------------------------
////    Class definition file:  OPTIMISE.CPP
////    Associated header file: PARSENODE.H
////
////    Author:					Paul C. Gregory
////    Creation date:			25/11/99
////---------------------------------------------------------------------

#include	<aqsis/aqsis.h>
#include	"parsenode.h"

namespace Aqsis {

///---------------------------------------------------------------------
/// CqParseNode::Optimise

bool CqParseNode::Optimise()
{
	// Optimise children nodes
	CqParseNode * pChild = m_pChild;
	while ( pChild )
	{
		if ( pChild->Optimise() )
			pChild = m_pChild;
		else
			pChild = pChild->pNext();
	}

	return ( false );
}


///---------------------------------------------------------------------
/// CqParseNodeFunction:Call:Optimise
/// Optimise a function definition, basically optimise the parameters.

bool CqParseNodeFunctionCall::Optimise()
{
	CqParseNode::Optimise();

	return ( false );
}


///---------------------------------------------------------------------
/// CqParseNodeFunction:Call:Optimise
/// Optimise a function definition, basically optimise the parameters.

bool CqParseNodeUnresolvedCall::Optimise()
{
	CqParseNode::Optimise();

	return ( false );
}


///---------------------------------------------------------------------
/// CqParseNodeVariable::Optimise

bool CqParseNodeVariable::Optimise()
{
	CqParseNode::Optimise();

	return ( false );
}


///---------------------------------------------------------------------
/// CqParseNodeAssign::Optimise

bool CqParseNodeAssign::Optimise()
{
	CqParseNode::Optimise();
	return ( false );
}


///---------------------------------------------------------------------
/// CqParseNodeCast::Optimise

bool CqParseNodeCast::Optimise()
{
	CqParseNode::Optimise();

	assert( m_pChild != 0 );

	// If the child the same as the cast, ignore it.
	if ( ( m_pChild->ResType() & Type_Mask ) ==
	        ( m_tTo & Type_Mask ) )
	{
		m_pChild->LinkAfter( this );
		m_pChild = 0;
		delete( this );
		return ( true );
	}

	// If the child is a point and the cast is to color, ignore it.
	/*	if(((m_pChild->ResType()&Type_Mask)==Type_Point) &&
			 			 ((m_tTo&Type_Mask)==Type_Color))
		{
			m_pChild->LinkAfter(this);
			m_pChild=0;
			delete(this);
			return(true);
		}
	 
		// If the child is a color and the cast is to point, ignore it.
		if(((m_pChild->ResType()&Type_Mask)==Type_Color) &&
			 			 ((m_tTo&Type_Mask)==Type_Point))
		{
			m_pChild->LinkAfter(this);
			m_pChild=0;
			delete(this);
			return(true);
		}*/

	return ( false );
}

} // namespace Aqsis
//---------------------------------------------------------------------
