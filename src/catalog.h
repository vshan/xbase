//
// catalog.h
//
//
// This file defines structures useful for describing the DB catalogs
//

#ifndef CATALOG_H
#define CATALOG_H

#include "printer.h"
#include "parser.h"

// in printer.h
// struct DataAttrInfo {
//     char     relName[MAXNAME+1];    // Relation name
//     char     attrName[MAXNAME+1];   // Attribute name
//     int      offset;                // Offset of attribute
//     AttrType attrType;              // Type of attribute
//     int      attrLength;            // Length of attribute
//     int      indexNo;               // Index number of attribute
// }

struct dataRelInfo
{
  // Default constructor
  dataRelInfo() {
    memset(relName, 0, MAXNAME + 1);
  }

  dataRelInfo( char * buf ) {
    memcpy(this, buf, dataRelInfo::size());
  }

  // Copy constructor
  dataRelInfo( const dataRelInfo &d ) {
    strcpy (relName, d.relName);
    recordSize = d.recordSize;
    attrCount = d.attrCount;
    numPages = d.numPages;
    numRecords = d.numRecords;
  };

  dataRelInfo& operator=(const dataRelInfo &d) {
    if (this != &d) {
      strcpy (relName, d.relName);
      recordSize = d.recordSize;
      attrCount = d.attrCount;
      numPages = d.numPages;
      numRecords = d.numRecords;
    }
    return (*this);
  }

  static unsigned int size() {
    return (MAXNAME+1) + 4*sizeof(int);
  }

  static unsigned int members() {
    return 5;
  }

  int      recordSize;            // Size per row
  int      attrCount;             // # of attributes
  int      numPages;              // # of pages used by relation
  int      numRecords;            // # of records in relation
  char     relName[MAXNAME+1];    // Relation name
};


#endif // CATALOG_H
