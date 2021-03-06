#ifndef SYSMAN
#define SYSMAN


#include "xbase.h"
#include "parser.h"
#include "SysPage_Internal.h"
#include "Index_Internal.h"
#include "catalog.h"

class SysMan_Manager{

public:
  SysMan_Manager (Index_Manager &ixm_, Record_Manager &rmm_);
  ~SysMan_Manager ();

  ErrCode openDB (const char *dbName);
  ErrCode closeDB ();

  ErrCode createTable (const char *relName, int attrCount, attrInfo *attributes);
  ErrCode createIndex (const char *relName, const char *attrName);
  ErrCode dropTable (const char *relName);
  ErrCode dropIndex (const char *relName, const char *attrName);
  ErrCode load (const char *relName);
  // ErrCode help ();
  // ErrCode help (const char *relName);
  ErrCode print (const char *relName);
  ErrCode set (const char *paramName);
  ErrCode get (const string& paramName, string& value) const;

  ErrCode isValid () const;
  ErrCode getFromTable (const char *relName, int& attrCount, dataAttrInfo *&attributes);
  ErrCode getRelFromCat (const char *relName, dataRelInfo &rel, Record_RID &rid) const;
  ErrCode getAttrFromCat (const char *relName, dataRelInfo &rel, Record_RID &rid) const;
  ErrCode getNumPages(const char* relName) const;
  ErrCode getNumRecords(const char* relName) const;

  ErrCode semCheck (const char *relName) const;
  ErrCode semCheck (const relAttr  &ra) const;
  ErrCode semCheck (const aggRelAttr &ra) const;
  ErrCode semCheck (const condition &cond) const;

  ErrCode findrelForAttr (relAttr &ra, int nRelations, const char * const possibleRelations[]) const;
  ErrCode loadRecord (const char *relName, int buflen, const char buff[]);
  bool isAttrIndexed (const char *relname, const char *attrName) const;
  ErrCode dropIndexFromAttrCatAlone (const char *relName, const char *attrName);
  ErrCode resetIndexFromAttrCatAlone (const char *relName, const char *attrName);

private:
  Record_Manager &rmm;
  Index_Manager &ixm;
  bool isDBOpen;
  Record_Filehandle relfh;
  Record_Filehandle attrfh;
  char cwd[1024];
  map<string, string> params;

};
#endif

//
// sm_error.h
//


#ifndef SYSMAN_ERROR_H
#define SYSMAN_ERROR_H

// #include "redbase.h"
//
// Print-error function
//
void SYSMAN_PrintError(RC rc);

#define SYSMAN_KEYNOTFOUND    (START_SYSMAN_WARN + 0)  // cannot find key
#define SYSMAN_INVALIDSIZE    (START_SYSMAN_WARN + 1)  // invalid entry size
#define SYSMAN_ENTRYEXISTS    (START_SYSMAN_WARN + 2)  // key,rid already
                                               // exists in index
#define SYSMAN_NOSUCHENTRY    (START_SYSMAN_WARN + 3)

#define SYSMAN_LASTWARN SYSMAN_NOSUCHENTRY


#define SYSMAN_DBOPEN          (START_SYSMAN_ERR - 0)
#define SYSMAN_NOSUCHDB        (START_SYSMAN_ERR - 1)
#define SYSMAN_NOTOPEN         (START_SYSMAN_ERR - 2)
#define SYSMAN_NOSUCHTABLE     (START_SYSMAN_ERR - 3)
#define SYSMAN_BADOPEN         (START_SYSMAN_ERR - 4)
#define SYSMAN_FNOTOPEN        (START_SYSMAN_ERR - 5)
#define SYSMAN_BADATTR         (START_SYSMAN_ERR - 6)
#define SYSMAN_BADTABLE        (START_SYSMAN_ERR - 7)
#define SYSMAN_INDEXEXISTS     (START_SYSMAN_ERR - 8)
#define SYSMAN_TYPEMISYSMANATCH    (START_SYSMAN_ERR - 9)
#define SYSMAN_BADOP           (START_SYSMAN_ERR - 10)
#define SYSMAN_AMBGATTR        (START_SYSMAN_ERR - 11)
#define SYSMAN_BADPARAM        (START_SYSMAN_ERR - 12)
#define SYSMAN_BADAGGFUN       (START_SYSMAN_ERR - 13)

#define SYSMAN_LASTERROR SYSMAN_BADAGGFUN

#endif // SYSMAN_ERROR_H
