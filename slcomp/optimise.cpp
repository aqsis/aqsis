////---------------------------------------------------------------------
////    Class definition file:  OPTIMISE.CPP
////    Associated header file: PARSENODE.H
////
////    Author:					Paul C. Gregory
////    Creation date:			25/11/99
////---------------------------------------------------------------------

#include	"aqsis.h"
#include	"parsenode.h"

START_NAMESPACE(Aqsis)

///---------------------------------------------------------------------
/// CqParseNode::Optimise
/// Test output of a parse tree to a specified output stream.

TqBool CqParseNode::Optimise()
{
	// Optimise children nodes
	CqParseNode* pChild=m_pChild;
	while(pChild)
	{
		if(pChild->Optimise())
			pChild=m_pChild;
		else
			pChild=pChild->pNext();
	}

	return(TqFalse);
}


///---------------------------------------------------------------------
/// CqParseNodeFunction::Optimise
/// Optimise a function definition, basically optimise the parameters.

TqBool CqParseNodeFunction::Optimise()
{
	CqParseNode::Optimise();

	return(TqFalse);
}


///---------------------------------------------------------------------
/// CqParseNodeVariable::Optimise
/// Test output of a parse tree to a specified output stream.

TqBool CqParseNodeVariable::Optimise()
{
	CqParseNode::Optimise();

	return(TqFalse);
}


///---------------------------------------------------------------------
/// CqParseNodeAssign::Optimise
/// Test output of a parse tree to a specified output stream.

TqBool CqParseNodeAssign::Optimise()
{
	CqParseNode::Optimise();
	return(TqFalse);
}


///---------------------------------------------------------------------
/// CqParseNodeCast::Optimise
/// Test output of a parse tree to a specified output stream.

TqBool CqParseNodeCast::Optimise()
{
	CqParseNode::Optimise();

	assert(m_pChild!=0);

	// If the child the same as the cast, ignore it.
	if((m_pChild->ResType()&Type_Mask)==
					 (m_tTo&Type_Mask))
	{
		m_pChild->LinkAfter(this);
		m_pChild=0;
		delete(this);
		return(TqTrue);
	}

	// If the child is a point and the cast is to color, ignore it.
/*	if(((m_pChild->ResType()&Type_Mask)==Type_Point) &&
		 			 ((m_tTo&Type_Mask)==Type_Color))
	{
		m_pChild->LinkAfter(this);
		m_pChild=0;
		delete(this);
		return(TqTrue);
	}

	// If the child is a color and the cast is to point, ignore it.
	if(((m_pChild->ResType()&Type_Mask)==Type_Color) &&
		 			 ((m_tTo&Type_Mask)==Type_Point))
	{
		m_pChild->LinkAfter(this);
		m_pChild=0;
		delete(this);
		return(TqTrue);
	}*/

	return(TqFalse);
}

END_NAMESPACE(Aqsis)
//---------------------------------------------------------------------
