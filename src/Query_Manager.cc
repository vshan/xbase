#include "Query_Manager.h"
#include "Query_Error.h"
#include "parser.h"
#include <cstring>
#include <map>
using namespace std;

namespace ns{
    bool strlt(char* i, char* j)
    {
        return strcmp(i,j) < 0;
    }

    bool streq(char* i,char* j)
    {
        return strcmp(i,j)==0;
    }
}
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
ErrCode Select(int nSelAttrs,const AggRelAttr selAttrs_[],
               int nRelations,const char* const relations_[],
               int nConditions,const Condition conditions_[],
               int order, RelAttr orderAttr,
               bool group, RelAttr groupAttr)
ErrCode Query_Manager::select(int nSelAttrs,const RelAttr selAttrs_[],
                              int nRelations,const char* const relations_[],
                              int nConditions,const Condition conditions_[])
{
    ErrCode invalid = isInvalid(); if(invalid) return invalid;

    //copies for rewrite
    RelAttr* selAttrs = new RelAttr[nSelAttrs];
    for(int i = 0; i < nSelAttrs; i++)
    {
        //selAttrs[i].relName = selAttrs_[i].relName;
        //selAttrs[i].attrName = selAttrs_[i].attrName;
        selAttrs[i] = selAttrs_[i];
    }

    char** relations = new char*[nRelations];
    for(int i=0; i < nRelations; i++)
    {
        relations[i] = strdup(relations_[i]);
    }

    Condition* conditions = new Condition[nConditions];
    for(int i=0; i < nConditions; i++)
    {
        conditions[i] = conditions_[i];
    }

    AggRelAttr* selAggAttrs = new AggRelAttr[nSelAggAttrs];
    for(int i = 0; i < nSelAggAttrs; i++)
    {
        selAggAttrs[i] = selAggAttrs_[i];
    }

    // semantic check
    for(int i=0; i < nRelations; i++)
    {
        ErrCode ec = smm.semCheck(relations[i]);
        if(ec != 0)
            return ec;
    }

    // sort
    sort(relations,relations + nRelations, ns::strlt);

    //check for duplicates
    char** dup = adjacent_find(relations,relations + nRelations, ns::streq);
    if(dup != relations+nRelations)
    {
        return QUERY_DUPREL;
    }

    // rewrite SELECT *
    bool SELECT_STAR = false;
    if(nSelAttrs == 1 && strcmp(selAttrs[0].attrName,"*"))
    {
        SELECT_STAR = TRUE;
        nSelAttrs = 0;
        for(int i = 0; i < nRelations; i++)
        {
            int attrCount;
            DataAttrInfo* attributes;
            ErrCode ec = smm.getFromTable(relations[i], attrCount, attributes);
            if(ec != 0)
                return ec;

            nSelAttrs += attrCount;
            delete[] attributes;
        }

        delete[] selAttrs;
        delete[] selAggAttrs;

        selAttrs = new RelAttr[nSelAttrs];
        selAggAttrs = new AggRelAttr[nSelAttrs];

        int j=0;
        for(int i=0; i < nRelations; i++)
        {
            int attrCount;
            DataAttrInfo* attributes;
            ErrCode ec = smm.getFromTable(relations[i],attrCount,attributes);
            if(ec != 0)
                return ec;
            for(int k=0; k < attrCount; k++)
            {
                selAttrs[j].attrName = selAggAttrs[j].attrName = strdup(attributes[k].attrName);
                selAttrs[j].relName = selAggAttrs[j].relName = relations[i];
                selAggAttrs[j].aggFunc = NO_F;
                j++;
            }
            delete[] attributes;
        }
    } // rewrite select *

    if(order!=0)
    {
        ErrCode ec = smm.findRelForAttr(orderAttr, nRelations, relations);
        if(ec != 0)
            return ec;
        // semantic check
        ec = smm.semCheck(orderAttr);
        if(ec != 0)
            return ec;
    }

    if(group==TRUE)
    {
        ErrCode ec = smm.findRelForAttr(groupAttr, nRelations, relations);
        if(ec != 0)
            return ec;
        ec = smm.semCheck(groupAttr);
        if(ec != 0)
            return ec;
    }
    else
    {
        // make sure no aggregate functions are defined
        for(int i=0; i < nSelAttrs; i++)
        {
            if(selAggAttrs[i].aggFunc != NO_F)
                return SYSMAN_BADAGGFUN;
        }
    }

    // rewrite select COUNT(*)
    for(int i=0; i < nSelAttrs; i++)
    {
        if(strcmp(selAggAttrs[i].attrName, "*") == 0
            && selAggAttrs.aggFunc == COUNT_F)
        {
            selAggAttrs[i].attrName = selAttrs[i].attrName = strdup(groupAttr.attrName);
            selAggAttrs[i].relName = selAttrs[i].relName = strdup(groupAttr.relName);
        }
    }

    for(int i=0; i < nSelAttrs; i++)
    {
        if(selAttrs[i].relName == NULL)
        {
            ErrCode ec = smm.findRelForAttr(selAttrs[i], nRelations, relations);
            if(ec != 0)
                return ec;
        }
        else
        {
            selAttrs[i].relName = strdup(selAttrs[i].relName);
        }
        selAggAttrs[i].relName =strdup(selAttrs[i].relName);
        ErrCode ec = smm.semCheck(selAttrs[i]);
        if(ec != 0)
            return ec;
        ec = smm.semCheck(selAggAttrs[i]);
        if(ec != 0)
            return ec;
    }

    for(int i=0; i < nConditions; i++)
    {
        if(conditions[i].lhsAttr.relName == NULL)
        {
            ErrCode ec = smm.findRelForAttr(conditions[i].lhsAttr, nRelations, relations);
            if(ec != 0)
                return ec;
        }
        else
        {
            conditions[i].lhsAttr.relName = strdup(conditions[i].lhsAttr.relName);
        }
        ErrCode ec = smm.semCheck(conditions[i].lhsAttr);
        if(ec != 0)
            return ec;

        if(conditions[i].ifRhsIsAttr == TRUE)
        {
            if(conditions[i].rhsAttr.relName == NULL)
            {
                ErrCode ec = smm.findRelForAttr(conditions[i].rhsAttr, nRelations, relations);
                if(ec != 0)
                    return ec;
            }
            else
            {
                conditions[i].rhsAttr.relName = strdup(conditions[i].rhsAttr.relName);
            }
            ErrCode ec = smm.semCheck(conditions[i].rhsAttr);
            if(ec != 0)
                return ec;
        }
        ec = smm.semCheck(conditions[i]);
        if(ec != 0)
            return ec;
    }

    // ensure that all relations mentioned in conditions are in the from clause
    for(int i=0; i < nConditions; i++)
    {
        bool lfound = false;
        for(int j=0; j <nRelations; j++)
        {
            if(strcmp(conditions[i].lhsAttr.relName, relations[j])==0)
            {
                lfound = true;
                break;
            }
        }

        if(conditions[i].ifRhsIsAttr == TRUE)
        {
            bool rfound = false;
            for(int j=0; j < nRelations; j++)
            {
                if(strcmp(conditions[i].rhsAttr.relName, relations[j]) == 0)
                {
                    rfound = true;
                    break;
                }
            }
            if(!rfound)
                return QUERY_RELMISSINGFROMFROM;
        }
    }

    if(!lfound)
        return QUERY_RELMISSINGFROMFROM;


} // end of Select

/*
    INSERT Statement
*/
ErrCode Query_Manager::insert()
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
