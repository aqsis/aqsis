////---------------------------------------------------------------------
////    Class definition file:  VARDEF.CPP
////    Associated header file: VARDEF.H
////
////    Author:					Paul C. Gregory
////    Creation date:			22/07/99
////---------------------------------------------------------------------

#include	"aqsis.h"
#include	"shadervariable.h"
#include	"parsenode.h"
#include	"vardef.h"

START_NAMESPACE(Aqsis)

///---------------------------------------------------------------------
/// Global array of standard variable definitions

CqVarDef	gStandardVars[]={
			CqVarDef(Type_VaryingColor, "Cs"),
			CqVarDef(Type_VaryingColor, "Os"),
			CqVarDef(Type_VaryingPoint, "Ng"),
			CqVarDef(Type_UniformFloat, "du"),
			CqVarDef(Type_UniformFloat, "dv"),
			CqVarDef(Type_VaryingPoint, "L"),
			CqVarDef(Type_VaryingColor, "Cl"),
			CqVarDef(Type_VaryingColor, "Ol"),
			CqVarDef(Type_VaryingPoint, "P"),
			CqVarDef(Type_VaryingPoint, "dPdu"),
			CqVarDef(Type_VaryingPoint, "dPdv"),
			CqVarDef(Type_VaryingPoint, "N"),
			CqVarDef(Type_VaryingFloat, "u"),
			CqVarDef(Type_VaryingFloat, "v"),
			CqVarDef(Type_VaryingFloat, "s"),
			CqVarDef(Type_VaryingFloat, "t"),
			CqVarDef(Type_VaryingPoint, "I"),
			CqVarDef(Type_VaryingColor, "Ci"),
			CqVarDef(Type_VaryingColor, "Oi"),
			CqVarDef(Type_VaryingPoint, "Ps"),
			CqVarDef(Type_UniformPoint, "E"),
			CqVarDef(Type_UniformFloat, "ncomps"),
			CqVarDef(Type_UniformFloat, "time"),
			CqVarDef(Type_UniformFloat, "alpha"),
};
TqInt		gcStandardVars=sizeof(gStandardVars)/sizeof(gStandardVars[0]);

///---------------------------------------------------------------------
/// Global array of standard variable definitions

std::vector<CqVarDef>	gLocalVars;

///---------------------------------------------------------------------
/// Global stack of translation tables

std::vector<std::vector<SqVarRefTranslator>*>	CqVarDef::m_saTransTable;


///---------------------------------------------------------------------
/// operator<<
/// Test output of a parse tree to a specified output stream.

std::ostream& operator<<(std::ostream& Stream, CqVarDef& Var)
{
	if(Var.m_UseCount>0 || (Var.Type()&Type_Param))
	{
		Stream << Var.StorageSpec(Var.m_Type).c_str() << " " 
			   << gVariableTypeNames[Var.m_Type&Type_Mask] << " " 
			   << Var.m_strName.c_str();
		if(Var.Type()&Type_Array)		   
			Stream << "[" << Var.m_ArrayLength << "]";
		
		Stream << std::endl;
	}
	return(Stream);
}


///---------------------------------------------------------------------
/// CqVarDef::CqVarDef
/// Copy constructor.

CqVarDef::CqVarDef(const CqVarDef& from)
{
	*this=from;
}


///---------------------------------------------------------------------
/// CqVarDef::~CqVarDef
/// Destructor.

CqVarDef::~CqVarDef()
{
	delete(m_pDefValue);
}

///---------------------------------------------------------------------
/// CqVarDef::operator=
/// Assignment operator.

CqVarDef& CqVarDef::operator=(const CqVarDef& from)
{
	m_Type=from.m_Type;
	m_strName=from.m_strName;
	m_UseCount=0;
	m_fExtern=from.m_fExtern;
	m_vrExtern=from.m_vrExtern;
	m_ArrayLength=from.m_ArrayLength;
	if(from.m_pDefValue)
		m_pDefValue=from.m_pDefValue->Clone(0);
	else
		m_pDefValue=0;

	return(*this);
}


///---------------------------------------------------------------------
/// CqVarDef::StorageSpec
/// Return a string defining .the storage spec of this variable.

CqString CqVarDef::StorageSpec(EqVariableType Type)
{
	CqString strSpec("");
	
	if(Type&Type_Param)		strSpec+="param ";
	if(Type&Type_Uniform)	strSpec+="uniform ";
	if(Type&Type_Varying)	strSpec+="varying ";

	return(strSpec);
}


///---------------------------------------------------------------------
/// CqVarDef::OutputInit
/// Output the initialisation code for this parameter.

void CqVarDef::OutputInit(std::ostream& Out)
{
	// Simple header nodes use to output the initialisation code.
	CqParseNode Node;

	if(m_Type&Type_Param && m_pDefValue)
	{
		SqVarRef var;
		FindVariable(m_strName.c_str(), var);
		CqParseNodeCast* pCast=new CqParseNodeCast(m_Type);
		Node.AddLastChild(pCast);
		pCast->AddLastChild(m_pDefValue);
		Node.Optimise();
		Out << Node;
		m_pDefValue=0;
	}
}


///---------------------------------------------------------------------
/// CqVarDef::FindFunction
/// Find a variable definition by searching the standard definitions list.

TqBool CqVarDef::FindVariable(const char* strName, SqVarRef& Ref)
{
	// Search the local definitions next.
	TqInt i;
	for(i=0; i<gLocalVars.size(); i++)
	{
		if(gLocalVars[i].m_strName==strName)
		{
			Ref.m_Type=VarTypeLocal;
			Ref.m_Index=i;
			return(TqTrue);
		}
	}

	// Search the standard definitions first.
	for(i=0; i<gcStandardVars; i++)
	{
		if(gStandardVars[i].m_strName==strName)
		{
			Ref.m_Type=VarTypeStandard;
			Ref.m_Index=i;
			return(TqTrue);
		}
	}

	return(TqFalse);
}


///---------------------------------------------------------------------
/// CqVarDef::GetVariablePointer
/// Return a temporary pointer to a variable definition..

CqVarDef* CqVarDef::GetVariablePtr(const SqVarRef& Ref)
{
	SqVarRef RealRef=Ref;
	// Firstly see if the top translation table knows about this variable.
	
	if(!m_saTransTable.empty())
	{
		std::vector<SqVarRefTranslator>* pTransTable=0;
		std::vector<std::vector<SqVarRefTranslator>*>::reverse_iterator iTable=m_saTransTable.rbegin();
		
		int i=0;
		while(iTable!=m_saTransTable.rend())
		{
			pTransTable=*iTable;
			if(pTransTable!=0)
			{
				TqInt i;
				for(i=0; i<pTransTable->size(); i++)
				{
					if((*pTransTable)[i].m_From==RealRef)
					{
						RealRef=(*pTransTable)[i].m_To;
						break;
					}
				}
				// Only continue looking for nested translations if it was found at the current level.
				if(i==pTransTable->size())	break;
			}
			iTable++;
			i++;
		}
	}

	// Look up the variable reference according to its type.
	if(RealRef.m_Type==VarTypeStandard && RealRef.m_Index<gcStandardVars)
		return(&gStandardVars[RealRef.m_Index]);

	if(RealRef.m_Type==VarTypeLocal && RealRef.m_Index<gLocalVars.size())
	{
		if(gLocalVars[RealRef.m_Index].m_fExtern)
			return(GetVariablePtr(gLocalVars[RealRef.m_Index].m_vrExtern));
		else
			return(&gLocalVars[RealRef.m_Index]);
	}

	return(0);
}


///---------------------------------------------------------------------
/// CqVarDef::GetVariablePointer
/// Return a temporary pointer to a variable definition..

TqInt CqVarDef::AddVariable(CqVarDef& Def)
{
	gLocalVars.push_back(Def);
	return(gLocalVars.size()-1);
}


///---------------------------------------------------------------------
/// CqVarDef::PushTransTable
/// Push a new variable reference translation table onto the stack.

void CqVarDef::PushTransTable(std::vector<SqVarRefTranslator>* pTransTable)
{
	m_saTransTable.push_back(pTransTable);
}


///---------------------------------------------------------------------
/// CqVarDef::PopTransTable
/// Pop the previous variable reference translation table from the stack.

std::vector<SqVarRefTranslator>* CqVarDef::PopTransTable()
{
	std::vector<SqVarRefTranslator>* pRes=m_saTransTable.back();
	m_saTransTable.erase(m_saTransTable.end()-1);
	return(pRes);
}



END_NAMESPACE(Aqsis)
//---------------------------------------------------------------------
