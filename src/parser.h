
#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include "xbase.h"
#include "SysPage_Manager.h"
//
// Structure declarations and output functions
//
struct attrInfo{
    char     *attrName;   /* attribute name       */
    AttrType attrType;    /* type of attribute    */
    int      attrLength;  /* length of attribute  */
};

struct relAttr{
    char     *relName;    // Relation name (may be NULL)
    char     *attrName;   // Attribute name

    // Print function
    friend std::ostream &operator<<(std::ostream &s, const relAttr &ra);
};

struct aggRelAttr{
    AggFun   func;
    char     *relName;    // Relation name (may be NULL)
    char     *attrName;   // Attribute name

    // Print function
    friend std::ostream &operator<<(std::ostream &s, const AggRelAttr &ra);
};

struct value{
    AttrType type;         /* type of value               */
    void     *data;        /* value                       */
			   /* print function              */
    friend std::ostream &operator<<(std::ostream &s, const value &v);
};

struct condition{
    relAttr  lhsAttr;    /* left-hand side attribute            */
    compOp   op;         /* comparison operator                 */
    int      bRhsIsAttr; /* TRUE if the rhs is an attribute,    */
                         /* in which case rhsAttr below is valid;*/
                         /* otherwise, rhsValue below is valid.  */
    relAttr  rhsAttr;    /* right-hand side attribute            */
    value    rhsValue;   /* right-hand side value                */
			 /* print function                               */
    friend std::ostream &operator<<(std::ostream &s, const Condition &c);

};

//
// Parse function
//
class Query_Manager;
class SysMan_Manager;

ErrCode RBparse(SysPage_Manager &pfm, SysMan_Manager &smm, Query_Manager &qlm);

//
// Error printing function; calls component-specific functions
//
void printError(ErrCode ec);

// bQueryPlans is allocated by parse.y.  When bQueryPlans is 1 then the
// query plan chosen for the SFW query will be displayed.  When
// bQueryPlans is 0 then no query plan is shown.
extern int isQueryPlans;

#endif
