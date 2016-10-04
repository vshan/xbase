// xbase.h
// global declarations

#ifndef XBASE_H
#define XBASE_H

#include <iostream>

#define MAXNAME 24          // maximum length of relation
                            // name or attribute name
#define MAXSTRINGLEN 255    // maximum length of a string
                            // type attribute
#define MAXATTRS 40         // maximum attributes for a relation

enum AttrType {
   INT,
   FLOAT,
   STRING
};

typedef int ErrCode;

#define START_SYSPAGE_ERR  (-1)
#define END_SYSPAGE_ERR    (-100)

#define START_SYSPAGE_WARN  1
#define END_SYSPAGE_WARN    100
#define START_RECORD_WARN   101
#define END_RECORD_WARN     200


//
// TRUE, FALSE and BOOLEAN
//
#ifndef BOOLEAN
typedef char Boolean;
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef NULL
#define NULL 0
#endif

#endif
