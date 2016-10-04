// xbase.h
// global declarations

#ifndef XBASE_H
#define XBASE_H

#include <iostream>
#include<cstdio>
#include<cstring>
#include<cerrno>
#include<cmap>
#include<cassert>
#include<fcntl>
#include<unistd.h>

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

enum CompOp {
	NO_OP,
	EQ_OP, NE_OP, GE_OP, GT_OP, LT_OP, LE_OP
};

enum AggFun {
	NO_F,
	MIN_F, MAX_F, COUNT_F,
	SUM_F, AVG_F
};

typedef int ErrCode;

#define START_SYSPAGE_WARN  1
#define END_SYSPAGE_WARN    100
#define START_RECORD_WARN   101
#define END_RECORD_WARN     200

#define START_SYSPAGE_ERR  (-1)
#define END_SYSPAGE_ERR    (-100)
#define START_RECORD_ERR  (-101)
#define END_RECORD_ERR    (-200)
#define START_IX_ERR  (-201)
#define END_IX_ERR    (-300)
#define START_SM_ERR  (-301)
#define END_SM_ERR    (-400)
#define START_QL_ERR  (-401)
#define END_QL_ERR    (-500)

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
