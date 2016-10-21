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
  ErrCode help ();
  ErrCode help (const char *relName);
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
  ErrCode resetIndexFromCatAlone (const char *relName, const char *attrName);

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
