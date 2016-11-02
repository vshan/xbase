#ifndef QUERY_H
#define QUERY_H

class Query_Manager{
public:
    Query_Manager(SysMan_Manager &smm, Index_Manager &ixm, Record_Manager &rm); // Constructor
    ~Query_Manager(); // Destructor

    ErrCode select(int nSelAttrs,        // # attrs in Select clause
                   const RelAttr selAttrs[],       // attrs in Select clause
                   int nRelations,       // # relations in From clause
                   const char * const relations[], // relations in From clause
                   int nConditions,      // # conditions in Where clause
                   const Condition conditions[]);  // conditions in Where clause)

    ErrCode insert(const char* relName, // relation to insert into
                   int nValue, // number of values to insert
                   const Value values[]); // values to insert

    ErrCode update(const char* relName, // relation to Update
                   const RelAttr &updateAttr, // attribute to updateAttr
                   bool ifRhsIsAttr, // TRUE if RHS of = is an Attribute and not a Value
                   const RelAttr &rhsRelAttr, // attribute on the RHS to set LHS equal to
                   const Value &rhsValue, // or value on RHS to set attribute equal to
                   int nConditions; // number of conditions
                   const Condition conditions[]); // conditions

    ErrCode delete(const char* relName, // relation to be deleted
                   int nConditions, // number of conditions
                   const Condition conditions[]); // conditions


private:
    SysMan_Manager &smm;
    Index_Manager &ixm;
    Record_Manager &rm;
};

#endif // QUERY_H
