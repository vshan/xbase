#include "SysMan_Internal.h"

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

// Get the first matching row for relName
// contents are return in rel and the RID the record is located at is
// returned in rid.
// method returns SM_NOSUCHTABLE if relName was not found
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

// Get the first matching row for relName, attrName
// contents are returned in attr
// location of record is returned in rid
// method returns SM_NOSUCHENTRY if attrName was not found
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

ErrCode SysMan_Manager::dropTable(const char *relName)
{
  ErrCode invalid = isValid();
  if(invalid)
    return invalid;

  if(strcmp(relName, "relcat") == 0||strcmp(relName,"attrcat")==0)
    return SYSMAN_BADTABLE;

  Record_FileScan rfs;
  Record_Record rec;
  dataRelInfo *data;
  ErrCode ec;

  if((ec = rfs.openScan(relfh, STRING, MAXNAME+1, offsetof(dataRelInfo, relName),EQ_OP,(void*)relName)))
    return ec;

  bool attrFound = false;
  while(ec!=RECORD_EOF)
  {
    ec = efs.getNextRec(rec);

    if(ec!=0 && ec!=RECORD_EOF)
      return ec;

    if(ec!=RECORD_EOF)
    {
      rec.getData((char*&)data);
      if(strcmp(data->relName, relName) == 0)
      {
        attrFound = true;
        break;
      }
    }
  }
  if((ec = rfs.closeScan()))
    return ec;

  if(!attrFound)
    return SYSMAN_NOSUCHTABLE;

  Record_RID rid;
  rec.getRid(rid);

  if(ec = rmm.destroyFile(relName))
    return ec;

  if((ec = relfh.deleteRec(rid))!=0)
    return ec;

    {
      Record_Record rec;
      dataAttrInfo * adata;
      if ((ec = rfs.openScan(attrfh,
                             STRING,
                             MAXNAME+1,
                             offsetof(dataAttrInfo, relName),
                             EQ_OP,
                             (void*) relName)))
        return (ec);

      while (ec!=RECORD_EOF) {
        ec = rfs.getNextRec(rec);

        if (ec!=0 && ec!=RECORD_EOF)
        return (ec);

        if (ec!=RECORD_EOF) {
          rec.getData((char*&)adata);
          if(strcmp(adata->relName, relName) == 0) {
            if(adata->indexNo != -1) // drop indexes also
              this->dropIndex(relName, adata->attrName);
            Record_RID rid;
            rec.getRid(rid);
            if ((ec = attrfh.deleteRec(rid)) != 0)
              return ec;
          }
        }
      }

      if ((ec = rfs.closeScan()))
        return (ec);
    }
  return 0;
}

ErrCode SysMan_Manager::dropIndex(const char *relName, const char *attrName)
{
  ErrCode invalid = isValid();
  if(invalid)
    return invalid;

  if(relName == NULL || attrName = NULL)
    return SYSMAN_BADTABLE;

  Record_FileScan rfs;
  Record_Record rec;
  dataAttrInfo *data;
  ErrCode ec;

  if((ec = rfs.openScan(attrfh,
                         STRING,
                         MAXNAME+1,
                         offsetof(DataAttrInfo, relName),
                         EQ_OP,
                         (void*) relName)))
        return ec;

  bool attrFound;
  while(ec!=RECORD_EOF)
  {
    ec = rfs.getNextRec(rec);

    if(ec!=0 && ec!=RECORD_EOF)
      return ec;

    if(ec!=RECORD_EOF)
    {
      rec.getData((char*&)data);
      if(strcmp(data->attrName, attrName) == 0) {
        data->indexNo = -1;
        attrFound = true;
        break;
    }
  }

  }
  if ((ec = rfs.closeScan()))
    return ec

  if(!attrFound)
    return SYSMAN_BADATTR;

  Record_RID rid;
  rec.getRid(rid);

  if((ec = ixm.destroyIndex(relName, data->offset)))
    return ec;

  rec.set((char*)data, dataAttrInfo::size(), rid);
  if((ec = attrfh.updateRec(rec)))
    return ec;
  return 0;

}

ErrCode SysMan_Manager::dropIndexFromAttrCatAlone(const char *relName, const char *attrName)
{
  ErrCode invalid = isValid();
  if(invalid)
    return invalid;

  cerr << "attrName " << attrName << " early in DropIndexFromAttrCatAlone\n";

  if(relName == NULL || attrName = NULL)
    return SYSMAN_BADTABLE;

  Record_FileScan rfs;
  Record_Record rec;
  dataAttrInfo *data;
  ErrCode ec;

  if((ec = rfs.openScan(attrfh,
                         STRING,
                         MAXNAME+1,
                         offsetof(DataAttrInfo, relName),
                         EQ_OP,
                         (void*) relName)))
        return ec;

  bool attrFound;
  while(ec!=RECORD_EOF)
  {
    ec = rfs.getNextRec(rec);

    if(ec!=0 && ec!=RECORD_EOF)
      return ec;

    if(ec!=RECORD_EOF)
    {
      rec.getData((char*&)data);
      cerr << "data->attrName " << data->attrName << " found in DropIndexFromAttrCatAlone\n";

      if(strcmp(data->attrName, attrName) == 0) {
        cerr << "attrName " << attrName << " found in DropIndexFromAttrCatAlone\n";
        data->indexNo = -1;
        attrFound = true;
        break;
    }
  }

  }
  if ((ec = rfs.closeScan()))
    return ec

  if(!attrFound)
    return SYSMAN_BADATTR;

  Record_RID rid;
  rec.getRid(rid);

  // update attrcat
  rec.set((char*)data, dataAttrInfo::size(), rid);
  if((ec = attrfh.updateRec(rec)))
    return ec;
  return 0;

}

ErrCode SysMan_Manager::resetIndexFromAttrCatAlone(const char *relName, const char *attrName)
{
  ErrCode invalid = isValid();
  if(invalid)
    return invalid;

  if(relName == NULL || attrName = NULL)
    return SYSMAN_BADTABLE;

  Record_FileScan rfs;
  Record_Record rec;
  dataAttrInfo *data;
  ErrCode ec;

  if((ec = rfs.openScan(attrfh,
                         STRING,
                         MAXNAME+1,
                         offsetof(DataAttrInfo, relName),
                         EQ_OP,
                         (void*) relName)))
        return ec;

  bool attrFound;
  while(ec!=RECORD_EOF)
  {
    ec = rfs.getNextRec(rec);

    if(ec!=0 && ec!=RECORD_EOF)
      return ec;

    if(ec!=RECORD_EOF)
    {
      rec.getData((char*&)data);
      if(strcmp(data->attrName, attrName) == 0) {
        data->indexNo = data->offset;
        attrFound = true;
        break;
    }
  }

  }
  if ((ec = rfs.closeScan()))
    return ec

  if(!attrFound)
    return SYSMAN_BADATTR;

  Record_RID rid;
  rec.getRid(rid);

  // update attrcat
  rec.set((char*)data, dataAttrInfo::size(), rid);
  if((ec = attrfh.updateRec(rec)))
    return ec;
  return 0;

}

// Load a single record in the buf into the table relName
ErrCode SysMan_Manager::loadRecord(const char *relName,
                          int buflen,
                          const char buf[])
{
  ErrCode invalid = isValid(); if(invalid) return invalid;

  if(relName == NULL || buf == NULL) {
    return SYSMAN_BADTABLE;
  }

  Record_Filehandle rfh;
  ErrCode ec;
  if((ec =	rmm.openFile(relName, rfh))
    )
    return(ec);

  int attrCount = -1;
  dataAttrInfo * attributes;
  ec = getFromTable(relName, attrCount, attributes);
  if(ec != 0) return ec;

  Index_IndexHandle * indexes = new Index_IndexHandle[attrCount];

  int size = 0;
  for (int i = 0; i < attrCount; i++) {
    size += attributes[i].attrLength;
    if(attributes[i].indexNo != -1) {
      ixm.openIndex(relName, attributes[i].indexNo, indexes[i]);
    }
  }

  if(size != buflen)
    return SYSMAN_BADTABLE;

  {
    Record_RID rid;
    if ((ec = rfh.insertRec(buf, rid)) < 0
      )
      return(ec);

    for (int i = 0; i < attrCount; i++) {
      if(attributes[i].indexNo != -1) {
        char * ptr = const_cast<char*>(buf + attributes[i].offset);
        ec = indexes[i].insertEntry(ptr,
                                    rid);
        if (ec != 0) return ec;
      }
    }
  }
  // update numRecords in relcat
  dataRelInfo r;
  Record_RID rid;
  ec = getRelFromCat(relName, r, rid);
  if(ec != 0) return ec;

  r.numRecords += 1;
  r.numPages = rfh.getNumPages();
  Record_Record rec;
  rec.set((char*)&r, dataRelInfo::size(), rid);
  if ((ec = relfh.UpdateRec(rec)) != 0)
    return ec;

  if((ec =	rmm.closeFile(rfh))
    )
    return(ec);

  for (int i = 0; i < attrCount; i++) {
    if(attributes[i].indexNo != -1) {
      ErrCode ec = ixm.closeIndex(indexes[i]);
      if(ec != 0 ) return ec;
    }
  }

  delete [] attributes;
  delete [] indexes;
  return 0;
}


ErrCode SysMan_Manager::load(const char *relName,
                          int buflen,
                          const char buf[])
{
  ErrCode invalid = isValid(); if(invalid) return invalid;

  if(relName == NULL || buf == NULL) {
    return SYSMAN_BADTABLE;
  }

  ifstream ifs(fileName);
  if(ifs.fail())
    return SYSMAN_BADTABLE;

  Record_Filehandle rfh;
  ErrCode ec;
  if((ec =	rmm.openFile(relName, rfh))
    )
    return(ec);

  int attrCount = -1;
  dataAttrInfo * attributes;
  ec = getFromTable(relName, attrCount, attributes);
  if(ec != 0) return ec;

  Index_IndexHandle * indexes = new Index_IndexHandle[attrCount];

  int size = 0;
  for (int i = 0; i < attrCount; i++) {
    size += attributes[i].attrLength;
    if(attributes[i].indexNo != -1) {
      ixm.openIndex(relName, attributes[i].indexNo, indexes[i]);
    }
  }

  char * buf = new char[size];

  int numLines = 0;
  while(!ifs.eof()) {
    memset(buf, 0, size);

    string line;
    getline(ifs, line);
    if(line.length() == 0) continue; // ignore last newline
    numLines++;
    // cerr << "Line " << numLines << ": " << line << endl;
    // tokenize line
    string token;
    std::istringstream iss(line);
    int i = 0;
    while ( getline(iss, token, ',') )
    {
      assert(i < attrCount);
      istringstream ss(token);
      if(attributes[i].attrType == INT) {
        int val;
        ss >> val;
        memcpy(buf + attributes[i].offset , &val, attributes[i].attrLength);
      }
      if(attributes[i].attrType == FLOAT) {
        float val;
        ss >> val;
        memcpy(buf + attributes[i].offset, &val, attributes[i].attrLength);
      }
      if(attributes[i].attrType == STRING) {
        string val = token;
        // cerr << "Line " << numLines << ": token : " << token << endl;
        if(val.length() > attributes[i].attrLength) {
          cerr << "SM_Manager::Load truncating to "
               << attributes[i].attrLength << " - " << val << endl;
        }
        if(val.length() < attributes[i].attrLength)
          memcpy(buf + attributes[i].offset, val.c_str(),
                 val.length());
        else
          memcpy(buf + attributes[i].offset, val.c_str(),
                 attributes[i].attrLength);
      }
      i++;
    }

    Record_RID rid;
    if ((ec = rfh.insertRec(buf, rid)) < 0
      )
      return(ec);

    for (int i = 0; i < attrCount; i++) {
      if(attributes[i].indexNo != -1) {
        char * ptr = const_cast<char*>(buf + attributes[i].offset);
        ec = indexes[i].insertEntry(buf + attributes[i].offset,
                                    rid);
        if (ec != 0) return ec;
      }
    }

  }

  // update numRecords in relcat
  dataRelInfo r;
  Record_RID rid;
  ec = getRelFromCat(relName, r, rid);
  if(ec != 0) return ec;

  r.numRecords += 1;
  r.numPages = rfh.getNumPages();
  Record_Record rec;
  rec.set((char*)&r, dataRelInfo::size(), rid);
  if ((ec = relfh.UpdateRec(rec)) != 0)
    return ec;

  if((ec =	rmm.closeFile(rfh))
    )
    return(ec);

  for (int i = 0; i < attrCount; i++) {
    if(attributes[i].indexNo != -1) {
      ErrCode ec = ixm.closeIndex(indexes[i]);
      if(ec != 0 ) return ec;
    }
  }

  delete [] buf;
  delete [] attributes;
  delete [] indexes;
  ifs.close();
  return 0;
}

ErrCode SysMan_Manager::print(const char *relName)
{
  ErrCode invalid = isValid();
  if(invalid)
    return invalid;

  DataAttrInfo *attributes;
  Record_Filehandle rfh;
  Record_Filehandle *prfh;

  int attrCount;
  Record_Record rec;
  char *data;
  ErrCode ec;

  if(strcmp(relname, "relcat") == 0)
    prfh = &relfh;
  else if(strcmp(relname, "attrcat") == 0)
      prfh = &attrfh;
  else
  {
    ec = rmm.openFile(relName, rfh);
    if(ec!=0)
      return ec;
    prfh  = &rfh;
  }

  ec = getFromTable(relName, attrCount, attributes);
  if(ec != 0)
    return ec;

  Printer p(attributes, attrCount);
  p.PrintHeader(cout);

  Record_FileScan rfs;

  if((ec = rfs.openScan(*prfh, INT, sizeof(int),0,NO_OP, NULL)))
    return ec;

  // print each table
  while(ec!=RECORD_EOF)
  {
    ec = rfs.getNextRec(rec);

    if(ec!=0 && ec!= RECORD_EOF)
      return ec;

    if(ec!=RECORD_EOF)
    {
      rec.getData(data);
      p.print(cout, data);
    }
  }

  //Print Footer information
  p.printFooter(cout);

  if((ec = rfs.closeScan()))
    return ec;

  if((0 == rfh.isValid()))
  {
    if(ec = rmm.closeFile(rfh))
      return ec;

  }

  delete [] attributes;
  return 0;
}

ErrCode SysMan_Manager::set(const char *paramName, const char *value)
{
  ErrCode invalid = isValid(); if(invalid) return invalid;
  if(paramName == NULL || value  == NULL)
    return SYSMAN_BADPARAM;

  params[paramName] = string(value);
  cout << "Set\n"
       << "   paramName=" << paramName << "\n"
       << "   value    =" << value << "\n";
  return 0;
}

ErrCode SysMan_Manager::get(const string& paramName, string& value) const
{
  ErrCode invalid = isValid(); if(invalid) return invalid;

  map<string, string>::const_iterator it;
  it = params.find(paramName);
  if(it == params.end())
    return SM_BADPARAM;
  value = it->second;
  return 0;
}

ErrCode SysMan_Manager::isValid () const
{
  bool ret = true;
  ret =  ret && isDBOpen;
  return ret ? 0 : SYSMAN_BADOPEN;
}

ErrCode SysMan_Manager::getFromTable(const char *relName, int &attrCount, dataAttrInfo *attributes)
{
  ErrCode invalid = isValid();
  if(invalid)
    return invalid;

  if(relName == NULL)
    return SYSMAN_NOSUCHTABLE;

  void *value = const_cast<char*>(relName);
  Record_FileScan rfs;
  ErrCode ec = rfs.openScan(relfh, STRING, MAXNAME+1, offsetof(dataRelInfo, relName),
                            EQ_OP, value, NO_HINT);
  if(ec!=0)
    return ec;

  Record_Record rec;
  ec = rfs.getNextRec(rec);
  if(ec == RECORD_EOF)
    return SYSMAN_NOSUCHTABLE;

  dataRelInfo  *prel;
  rec.getData((char *&)prel);

  ec = rfs.closeScan();
  if(ec != 0)
    return ec;

  attrCount = prel->attrCount;
  attributes = new dataAttrInfo[attrCount];

  Record_FileScan afs;
  ec = afs.OpenScan(attrfh, STRING, MAXNAME+1, offsetof(dataAttrInfo),EQ_OP, value,NO_HINT);

  int numRecs = 0;
  while(1)
  {
    Record_Record rec;
    ec = afs.GetNextRec(rec);
    if(ec == RECORD_EOF || numRecs > attrCount)
      break;
    dataAttrInfo *pattr;
    rec.getData((char *&)pattr);
    attributes[numRecs] = *pattr;
    numRecs++;
  }

  if(numRecs != attrCount)
    return SYSMAN_BADTABLE;

  ec = afs.CloseScan();
  if(ec!=0)
    return ec;
  return 0;
}

bool SysMan_Manager::isAttrIndexed(const char* relName, const char* attrName) const
{
  ErrCode invalid = isValid(); if(invalid) return invalid;
  dataAttrInfo a;
  Record_RID rid;
  ErrCode ec = getAttrFromCat(relName, attrName, a, rid);

  return a.indexNo != -1 ? true : false;
}


ErrCode SysMan_Manager::semCheck(const char* relName) const
{
  ErrCode invalid = isValid(); if(invalid) return invalid;
  dataRelInfo rel;
  Record_RID rid;
  return getRelFromCat(relName, rel, rid);
}

ErrCode SysMan_Manager::semCheck(const relAttr& ra) const
{
  ErrCode invalid = isValid(); if(invalid) return invalid;
  dataAttrInfo a;
  Record_RID rid;
  return getAttrFromCat(ra.relName, ra.attrName, a, rid);
}

ErrCode SysMan_Manager::semCheck(const aggRelAttr& ra) const
{
  ErrCode invalid = isValid(); if(invalid) return invalid;
  dataAttrInfo a;
  Record_RID rid;
  if(ra.func != NO_F &&
     ra.func != MIN_F &&
     ra.func != MAX_F &&
     ra.func != COUNT_F)
    return SYSMAN_BADAGGFUN;

  return getAttrFromCat(ra.relName, ra.attrName, a, rid);
}

// ra.relName must be NULL when we start off and should be free()d by the user
ErrCode SysMan_Manager::findRelForAttr(relAttr& ra, int nRelations,
                              const char * const
                              possibleRelations[]) const {
  ErrCode invalid = isValid(); if(invalid) return invalid;
  if(ra.relName != NULL) return 0;
  dataAttrInfo a;
  Record_RID rid;
  bool found = false;
  for( int i = 0; i < nRelations; i++ ) {
    ErrCode ec = getAttrFromCat(possibleRelations[i], ra.attrName, a, rid);
    if(ec == 0 && !found) {
      found = true;
      ra.relName = strdup(possibleRelations[i]);
    }
    else
      if(ec == 0 && found) {
        // clash
        free(ra.relName);
        ra.relName = NULL;
        return SYSMAN_AMBGATTR;
    }
  }
  if (!found)
    return SYSMAN_NOSUCHENTRY;
  else // ra was populated
    return 0;
}

// should be used only after lhsAttr and rhsAttr have been expanded out of NULL
ErrCode SysMan_Manager::semCheck(const condition& cond) const
{
  if((cond.op < NO_OP) ||
      cond.op > GE_OP)
    return SYSMAN_BADOP;

  if(cond.lhsAttr.relName == NULL || cond.lhsAttr.attrName == NULL)
    return SYSMAN_NOSUCHENTRY;

  if(cond.bRhsIsAttr == TRUE) {
    if(cond.rhsAttr.relName == NULL || cond.rhsAttr.attrName == NULL)
      return SYSMAN_NOSUCHENTRY;
    dataAttrInfo a, b;
    Record_RID rid;
    ErrCode ec = getAttrFromCat(cond.lhsAttr.relName, cond.lhsAttr.attrName, a, rid);
    if (ec !=0) return SYSMAN_NOSUCHENTRY;
    rc = getAttrFromCat(cond.rhsAttr.relName, cond.rhsAttr.attrName, b, rid);
    if (ec !=0) return SYSMAN_NOSUCHENTRY;

    if(b.attrType != a.attrType)
      return SYSMAN_TYPEMISMATCH;

  } else {
    dataAttrInfo a;
    Record_RID rid;
    ErrCode ec = getAttrFromCat(cond.lhsAttr.relName, cond.lhsAttr.attrName, a, rid);
    if (ec !=0) return SYSMAN_NOSUCHENTRY;
    if(cond.rhsValue.type != a.attrType)
      return SYSMAN_TYPEMISMATCH;
  }
  return 0;
}

ErrCode SysMan_Manager::getNumPages(const char* relName) const
{
  dataRelInfo r;
  Record_RID rid;
  ErrCode ec = getRelFromCat(relName, r, rid);
  if(ec != 0) return ec;

  return r.numPages;
}

ErrCode SysMan_Manager::getNumRecords(const char* relName) const
{
  dataRelInfo r;
  Record_RID rid;
  ErrCode ec = getRelFromCat(relName, r, rid);
  if(ec != 0) return ec;

  return r.numRecords;
}
