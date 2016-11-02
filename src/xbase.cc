#include "redbase.h"
#include "SysPage_Manager.h"
#include "Record_Manager.h"
#include "Index_Manager.h"
#include "SysMan_Manager.h"
#include "Query_Manager.h"

using namespace std;

static void PrintErrorExit(ErrCode ec) {
  PrintErrorAll(ec);
  exit(ec);
}

// main //

int main(int argc, char *argv[])
{
  ErrCode ec;
  // Look for 2 arguments.  The first is always the name of the program
  // that was executed, and the second should be the name of the
  // database

  if(argc != 2)
  {
    cerr << "Usage: " << argv[0] << "dbName" endl;
    exit(1);
  }
  char *dbName = argv[1];

  SysPage_Manager pfm;
  Record_Manager rmm(pfm);
  Index_Manager ixm(pfm);
  SysMan_Manager smm(ixm, rmm);
  Query_Manager qlm(smm, ixm, rmm);

  //Open the database
  if ((ec = smm.OpenDb(dbname)))
    PrintErrorExit(ec);
  // call the parser
  ErrCode parseEC = RBparse(pfm, smm, qlm);

  // close the database
  if ((ec = smm.closeDb()))
    PrintErrorExit(ec);

  if(parseEC != 0)
    PrintErrorExit(parseEC);

  cout << "Bye.\n";
  return 0;

}
