#ifndef QUERY_ERROR_H
#define QUERY_ERROR_H

#include "redbase.h"
//
// Print-error function
//
void Query_PrintError(RC rc);

#define QUERY_KEYNOTFOUND    (START_QUERY_WARN + 0)  // cannot find key
#define QUERY_INVALIDSIZE    (START_QUERY_WARN + 1)  // invalid number of attributes
#define QUERY_ENTRYEXISTS    (START_QUERY_WARN + 2)  // key,rid already
                                               // exists in index
#define QUERY_NOSUCHENTRY    (START_QUERY_WARN + 3)  // key,rid combination
                                               // does not exist in index

#define QUERY_LASTWARN QUERY_ENTRYEXISTS


#define QUERY_BADJOINKEY      (START_QUERY_ERR - 0)
#define QUERY_ALREADYOPEN     (START_QUERY_ERR - 1)
#define QUERY_BADATTR         (START_QUERY_ERR - 2)
#define QUERY_DUPREL          (START_QUERY_ERR - 3)
#define QUERY_RELMISSINGFROMFROM (START_QUERY_ERR - 4)
#define QUERY_FNOTOPEN        (START_QUERY_ERR - 5)
#define QUERY_JOINKEYTYPEMISMATCH (START_QUERY_ERR - 6)
#define QUERY_BADOPEN         (START_QUERY_ERR - 7)
#define QUERY_EOF             (START_QUERY_ERR - 8)

#define QUERY_LASTERROR QUERY_EOF

#endif // QUERY_ERROR_H
