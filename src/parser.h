/*
    Parser Implementation Structure definitions
*/

#ifndef PARSER_H
#define PARSER_H

struct RelAttr {
    char* relName; // relation name (may be NULL)
    char* attrName; // attribute name
};

struct Value {
    AttrType type; // type - (int/float/string)
    void* data; // this void pointer can be casted to appropriate "type"
};

struct Condition {
    RelAttr lhsAttr; // left-hand side attribute
    CompOp op;  // comparison operator
    bool isRhsIsAttr;   // TRUE if right-hand side is an attribute and not a value
    RelAttr rhsAttr;      // right-hand side attribute if bRhsIsAttr = TRUE
    Value   rhsValue;     // right-hand side value if bRhsIsAttr = FALSE
};

struct AttrInfo {
    char* atrrName; // attribute name
    AttrType attrType ; // attribute type
    int attrlength; // attribute length
};

// used for Aggregate functions (	NO_F, MIN_F, MAX_F, COUNT_F,SUM_F, AVG_F)
struct AffRelAttr {
    AggFun aggFunc; // aggregate function
    char* attrName; // attribute name
    char* relName; // relation name
};

#endif
