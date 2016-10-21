#include "SysPage_Internal.h"

using namespace std;

SysMan_Manager:SysMan_Manager(Index_Manager &ixm_,  Record_Manager &rmm_)
    :rmm(rmm_), ixm(ixm_), isDBOpen(false)
    {
      memset(cwd,0,1024);
    }

SysMan_Manager::~SysMan_Manager()
{

}

ErrCode SysMan_Manager::openDB(const char *dbName)
{
  if(dbName == NULL)
    return SYSMAN_BADOPEN;
  if(isDBOpen)
    return SYSMAN_DBOPEN;

  if(getcwd(cwd,1024)<0)
  {
    return SYSMAN_NOSUCHDB ;
  }

  if(chdir(dbName)<0)
    return SYSMAN_NOSUCHDB;

  ErrCode ec;
  if((ec = rmm.openFile("attrcat", attrfh))
  || (ec = rmm.openFile("relcat", relfh))
  )
  return (ec);

  isDBOpen = true;
  return (0);
}

ErrCode SysMan_Manager:closeDB()
{
  if(!isDBOpen)
    return SYSMAN_NOTOPEN;

  ErrCode ec;
  if(ec = rmm.closeFile(attrfh) || ec = rmm.closeFile(relfh))
    return (ec);

  if(chdir(cwd)<0)
    return SYSMAN_NOSUCHDB;

  isDBOpen = false;
  return (0);
}

ErrCode SysMan_Manager::createTable(const char *relName,
                                    int attrCount,
                                    attrInfo *attributes)
{
  ErrCode invalid = isValid();
  if(invalid)
    return invalid;

  if(relName == NULL || attrCount <=0 || attributes == NULL)
    return SYSMAN_BADTABLE;

  if(strcmp(relName, "relcat") == 0 ||
     strcmp(relName,"attrcat") == 0)
     return SYSMAN_BADTABLE;

  Record_RID rid;
  ErrCode ec;
  set<string> uniq;

  dataAttrInfo *d = new dataAttrInfo[attrCount];
  int size = 0;
  for(int i = 0;i < attrCount; i++)
  {
    d[i] = dataAttrInfo(attributes[i]);
    d[i].offset = size;
    size += attributes[i].attrLength;
    strcpy(d[i].relName, relName);

    if(uniq.find(string(d[i].attrName)) == uniq.end())
      uniq.insert(string(d[i].attrName));
    else
    {
      return SYSMAN_BADATTR;
    }

    if((ec = attrfh.insertRec((char)*&d[i], rid))<0)
      return ec;

  }

  if(ec = rmm.createFile(relName, size))
    return (ec);

  dataRelInfo rel;
  strcpy(rel.relName, relName);
  rel.attrCount = attrCount;
  rel.recordSize = size;
  rel.numPages = 1;
  rel.numRecords = 0;

  if((ec = relfh.insertRec((char)*&rel, rid))<0)
    return (ec);

    delete []d;
    return (0);
}

ErrCode SysMan_Manager::getRelFromCat(const char* relName,
                                      dataRelInfo &rel,
                                      Record_RID &rid) const
{
  ErrCode invalid = isValid();
  if(invalid) return invalid;

  if(relName == NULL)
    return SYSMAN_BADTABLE;

  void *value = const_cast<char*>(relName);
  Record_FileScan rfs;
  ErrCode ec = rfs.openScan(relfh, STRING, MAXNAME+1,offsetof(dataRelInfo, relName),
                            EQ_OP, value, NO_HINT);
  if(ec != 0) return ec;

  Record_Record rec;
  ec = rfs.getNextRec(rec);
  if(ec == RECORD_EOF) return SYSMAN_NOSUCHTABLE;

  rc = rfs.closeScan();
  if(ec != 0) return ec;

  dataRelInfo *prel;
  rec.getData((char *&)prel);
  rel = *prel;
  rec.getRid(rid);

  return 0;
}

ErrCode SysMan_Manager::getAttrFromCat(const char* relname,
                                       const char* attrName,
                                        dataAttrInfo &attr,
                                        Record_RID &rid) const)
{
  ErrCode ec;
  Record_FileScan rfs;
  Record_Record rec;
  dataAttrInfo *data;
  if((ec = rfs.OpenScan(attrfh,
                         STRING,
                         MAXNAME+1,
                         offsetof(DataAttrInfo, relName),
                         EQ_OP,
                         (void*) relName)))
      return ec;

  bool attrFound = false;
  while (ec!=RECORD_EOF) {
    ec = rfs.getNextRec(rec);

    if (ec!=0 && ec!=RECORD_EOF)
      return (ec);

    if (ec!=RECORD_EOF) {
      rec.getData((char*&)data);
      if(strcmp(data->attrName, attrName) == 0) {
        attrFound = true;
        break;
      }
    }
  }

  if ((ec = rfs.closeScan()))
    return (ec);

  if(!attrFound)
    return SYSMAN_BADATTR;

  attr = *data;
  attr.func = NO_F;
  rec.getRid(rid);
  return 0;
}
