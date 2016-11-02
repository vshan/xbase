#include<iostream>
#include<cstring>
#include<string>
#include<cstdlib> // system()
#include<unistd.h> // chdir()

using namespace std;

static void printErrorExit(ErrCode ec)
{
  printErrorAll(ec);
  exit(ec);
}


int main(int argc, char* argv[])
{
    /*
        check for 2 arguements.
        first : command name
        second : <db_name>
    */
    ErrCode ec;

    if(argc!=2)
    {
        cerr << "Usage " << argv[0] << " <db_name> \n";
        exit(0);
    }

    // second arguement is db_name
    string db_name = argv[1];

    // create s subdirectory for the database
    string command="mkdir ";
    command += db_name;
    ec = system(command.str().c_str());

    if(ec != 0)
    {
        cerr << argv[0] <<": mkdir error for " << db_name << "\n";
        exit(ec);
    }

    if(chdir(db_name.c_str()) < 0)
    {
        cerr << argv[0] <<": chdir error to " << db_name << "\n";
        exit(1);
    }

    // Create the system catalogs
    SysPage_Manager pfm;
    Record_Manager rmm(pfm);
    Record_FileHandle relfh, attrfh;

    if(
      (ec = rmm.createFile("relcat", dataRelInfo::size()))
      || (ec = rmm.openFile("relcat", relfh))
    )
    printErrorExit(ec);

    if(
      (ec = rmm.createFile("attrcat", dataAttrInfo::size()))
      || (ec = rmm.openFile("attrcat", attrfh))
    )
    printErrorExit(ec);

    dataRelInfo relcat_rel;
    strcpy(relcat_rel.relName, "relcat");
    relcat_rel.attrCount = dataRelInfo::members();
    relcat_rel.recordSize = dataRelInfo::size();
    relcat_rel.numPages = 1; // initially
    relcat_rel.numRecords = 2; // relcat & attrcat


    dataRelInfo attrcat_rel;
    strcpy(attrcat_rel.relName, "attrcat");
    attrcat_rel.attrCount = dataAttrInfo::members();
    attrcat_rel.recordSize = dataAttrInfo::size();
    attrcat_rel.numPages = 1; // initially
    attrcat_rel.numRecords = dataAttrInfo::members() + dataRelInfo::members();

    Record_RID rid;
    if (
      (ec = relfh.insertRec((char*) &relcat_rel, rid)) < 0
      || (ec = relfh.insertRec((char*) &attrcat_rel, rid)) < 0
      )
      PrintErrorExit(rc);

    // relcat attrs
    dataAttrInfo a;
    strcpy(a.relName, "relcat");
    strcpy(a.attrName, "relName");
    a.offset = offsetof(dataRelInfo, relName);
    a.attrType = STRING;
    a.attrLength = MAXNAME+1;
    a.indexNo = -1;
    if ((ec = attrfh.insertRec((char*) &a, rid)) < 0)
      PrintErrorExit(rc);

    strcpy(a.relName, "relcat");
    strcpy(a.attrName, "recordSize");
    a.offset = offsetof(dataRelInfo, recordSize);
    a.attrType = INT;
    a.attrLength = sizeof(int);
    a.indexNo = -1;
    if ((ec = attrfh.insertRec((char*) &a, rid)) < 0)
      PrintErrorExit(rc);

    strcpy(a.relName, "relcat");
    strcpy(a.attrName, "attrCount");
    a.offset = offsetof(dataRelInfo, attrCount);
    a.attrType = INT;
    a.attrLength = sizeof(int);
    a.indexNo = -1;
    if ((ec = attrfh.insertRec((char*) &a, rid)) < 0)
      PrintErrorExit(rc);

    strcpy(a.relName, "relcat");
    strcpy(a.attrName, "numPages");
    a.offset = offsetof(dataRelInfo, numPages);
    a.attrType = INT;
    a.attrLength = sizeof(int);
    a.indexNo = -1;
    if ((ec = attrfh.insertRec((char*) &a, rid)) < 0)
      PrintErrorExit(rc);

    strcpy(a.relName, "relcat");
    strcpy(a.attrName, "numRecords");
    a.offset = offsetof(dataRelInfo, numRecords);
    a.attrType = INT;
    a.attrLength = sizeof(int);
    a.indexNo = -1;
    if ((ec = attrfh.insertRec((char*) &a, rid)) < 0)
      PrintErrorExit(rc);


    // attrcat attrs
    strcpy(a.relName, "attrcat");
    strcpy(a.attrName, "relName");
    a.offset = offsetof(dataAttrInfo, relName);
    a.attrType = STRING;
    a.attrLength = MAXNAME+1;
    a.indexNo = -1;
    if ((ec = attrfh.insertRec((char*) &a, rid)) < 0)
      PrintErrorExit(rc);

    strcpy(a.relName, "attrcat");
    strcpy(a.attrName, "attrName");
    a.offset = offsetof(dataAttrInfo, relName) + MAXNAME+1;
    a.attrType = STRING;
    a.attrLength = MAXNAME+1;
    a.indexNo = -1;
    if ((ec = attrfh.insertRec((char*) &a, rid)) < 0)
      PrintErrorExit(rc);

    strcpy(a.relName, "attrcat");
    strcpy(a.attrName, "offset");
    a.offset = offsetof(dataAttrInfo, offset);
    a.attrType = INT;
    a.attrLength = sizeof(int);
    a.indexNo = -1;
    if ((ec = attrfh.insertRec((char*) &a, rid)) < 0)
      PrintErrorExit(rc);

    strcpy(a.relName, "attrcat");
    strcpy(a.attrName, "attrType");
    a.offset = offsetof(dataAttrInfo, attrType);
    a.attrType = INT;
    a.attrLength = sizeof(attrType);
    a.indexNo = -1;
    if ((ec = attrfh.insertRec((char*) &a, rid)) < 0)
      PrintErrorExit(rc);

    strcpy(a.relName, "attrcat");
    strcpy(a.attrName, "attrLength");
    a.offset = offsetof(dataAttrInfo, attrLength);
    a.attrType = INT;
    a.attrLength = sizeof(int);
    a.indexNo = -1;
    if ((ec = attrfh.insertRec((char*) &a, rid)) < 0)
      PrintErrorExit(rc);

    strcpy(a.relName, "attrcat");
    strcpy(a.attrName, "indexNo");
    a.offset = offsetof(dataAttrInfo, indexNo);
    a.attrType = INT;
    a.attrLength = sizeof(int);
    a.indexNo = -1;
    if ((ec = attrfh.insertRec((char*) &a, rid)) < 0)
      PrintErrorExit(rc);

    strcpy(a.relName, "attrcat");
    strcpy(a.attrName, "func");
    a.offset = offsetof(dataAttrInfo, func);
    a.attrType = INT;
    a.attrLength = sizeof(AggFun);
    a.indexNo = -1;
         if ((ec = attrfh.insertRec((char*) &a, rid)) < 0)
      PrintErrorExit(rc);


    if ((ec =  rmm.closeFile(attrfh)) < 0
      || (ec =  rmm.closeFile(relfh)) < 0
      )
      PrintErrorExit(rc);


    return 0;
}
