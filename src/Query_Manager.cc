#include "Query_Manager.h"
#include "Query_Error.h"
using namespace std;

// Constructor
Query_Manager::Query_Manager(SysMan_Manager &smm_, Index_Manager &ixm_, Record_Manager &rm_)
    : smm(smm_), ixm(ixm_), rm(rm_)
{

}

// Destructor
Query_Manager::~Query_Manager()
{

}

ErrCode Query_Manager::isValid() const
{
    bool ret = (smm.isValid()==0);
    if (ret)
        return 0;
    return QUERY_BADOPEN;
}

/*
    SELECT Statement
*/
ErrCode Select(int nSelAttrs,const RelAttr selAttrs_[],
               int nRelations,const char* const relations_[],
               int nConditions,const Condition conditions_[])
{
    ErrCode invalid = isInvalid(); if(invalid) return invalid;

    //copies for rewrite
    RelAttr* selAttrs = new RelAttr[nSelAttrs];
    for(int i = 0; i < nSelAttrs; i++)
    {
        selAttrs[i].relName = selAttrs_[i].relName;
        selAttrs[i].attrName = selAttrs_[i].attrName;
    }

    
}

/*
    INSERT Statement
*/
ErrCode Insert()
{

}

/*
    UPDATE Statement
*/
ErrCode Update()
{

}

/*
    DELETE Statement
*/
ErrCode Delete()
{

}
