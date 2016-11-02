//
// File:        iterator.h
//

#ifndef ITERATOR_H
#define ITERATOR_H

#include "xbase.h"
#include "data_attr_info.h"
#include "rm_rid.h"
#include <sstream>
#include "predicate.h"

using namespace std;

class DataAttrInfo;

// abstraction to hide details of offsets and type conversions
class Tuple
{
public:
    Tuple(int ct, int length_): count(ct), length(length_), rid(-1, -1)
    {
        data = new char[length];
    }
    Tuple(const Tuple& rhs): count(rhs.count), length(rhs.length), rid(-1, -1)
    {
        data = new char[length];
        memcpy(data, rhs.data, length);
        setAttr(rhs.getAttributes());
    }

    Tuple& operator=(const Tuple& rhs)
    {
        if (this != &rhs)
        {
            memcpy(data, rhs.data, length);
            setAttr(rhs.getAttributes());
        }
        return *this;
    }

  // Destructor
  ~Tuple()
  {
      delete [] data;
  }
  int getLength() const
  {
      return length;
  }
  int getAttrCount() const
  {
      return count;
  }

  DataAttrInfo* getAttributes() const
  {
      return attrs;
  }
  void set(const char * buf)
  {
    assert(buf != NULL);
    memcpy(data, buf, length);
  }

  void getData(const char *& buf) const
  {
      buf = data;
  }

  void getData(char *& buf)
  {
      buf = data;
  }
  void setAttr(DataAttrInfo* pa)
  {
      attrs = pa;
  }
  void get(const char* attrName, void*& p) const
  {
    assert(attrs != NULL);
    for (int i = 0; i < count; i++)
    {
      if(strcmp(attrs[i].attrName, attrName) == 0)
      {
          p = (data+attrs[i].offset);
          return;
      }
    }
  }

  void get(int attrOffset, void*& p) const
  {
      assert(attrs != NULL);
      p = (data+attrOffset);
  }

  void set(int attrOffset, void* p)
  {
      assert(attrs != NULL && p != NULL);
      int attrLength = 0;
      for (int i = 0; i < count; i++)
      {
          if(attrs[i].offset == attrOffset)
          {
              attrLength = attrs[i].attrLength;
          }
      }
      memcpy(data+attrOffset, p, attrLength);
  }

  void get(const char* attrName, int& intAttr) const
  {
      assert(attrs != NULL);
      for (int i = 0; i < count; i++)
      {
          if(strcmp(attrs[i].attrName, attrName) == 0)
          {
              intAttr = *(int*)(data+attrs[i].offset);
              return;
          }
      }
  }
  void get(const char* attrName, float& floatAttr) const
  {
      assert(attrs != NULL);
      for (int i = 0; i < count; i++)
      {
          if(strcmp(attrs[i].attrName, attrName) == 0)
          {
              floatAttr = *(float*)(data+attrs[i].offset);
              return;
          }
      }
  }

  void get(const char* attrName, char strAttr[]) const
  {
      assert(attrs != NULL);
      for (int i = 0; i < count; i++)
      {
          if(strcmp(attrs[i].attrName, attrName) == 0)
          {
              strncpy(strAttr,(char*)(data+attrs[i].offset),attrs[i].attrLength);
              return;
          }
      }
  }
  // only useful for leaf level iterators
  RID getRid() const
  {
      return rid;
  }
  void setRid(RID r)
  {
      rid = r;
  }

 private:
     char * data;
     DataAttrInfo * attrs;
     int count;
     int length;
     RID rid;
};

namespace {
  std::ostream &operator<<(std::ostream &os, const Tuple &t)
  {
    os << "{";
    DataAttrInfo* attrs = t.getAttributes();

    for (int pos = 0; pos < t.getAttrCount(); pos++)
    {
        void * k = NULL;
        AttrType attrType = attrs[pos].attrType;
        t.get(attrs[pos].offset, k);
        if( attrType == INT )
            os << *((int*)k);
        if( attrType == FLOAT )
            os << *((float*)k);
        if( attrType == STRING )
        {
            for(int i=0; i < attrs[pos].attrLength; i++)
            {
                if(((char*)k)[i] == 0) break;
                os << ((char*)k)[i];
            }
        }
        os << ", ";
    }
    os << "\b\b";
    os << "}";
    return os;
  }
};

class Iterator {
 public:
     Iterator():bIterOpen(false), indent(""),bSorted(false), desc(false) {}
     virtual ~Iterator() {}

     virtual RC open() = 0;
     virtual RC getNext(Tuple &t) = 0;
     virtual RC close() = 0;

  // compare the return of GetNext() with Eof() to know when iterator has
  // finished
    virtual RC Eof() const = 0;

  // return must be good enough to use with Tuple::SetAttr()
    virtual DataAttrInfo* getAttr() const
    {
        return attrs;
    }
    virtual int getAttrCount() const
    {
        return attrCount;
    }
    virtual Tuple getTuple() const
    {
        Tuple t(getAttrCount(), TupleLength());
        t.setAttr(this->getAttr());
        return t;
    }
    virtual int TupleLength() const
    {
        int l = 0;
        DataAttrInfo* a = getAttr();
        for(int i = 0; i < getAttrCount(); i++)
            l += a[i].attrLength;
        return l;
    }

    virtual string Explain() = 0;

    virtual void setIndent(const string& indent_)
    {
        indent = indent_;
    }

    virtual bool isSorted() const { return bSorted; }
    virtual bool isDesc() const { return desc; }
    virtual string getSortRel() const { return sortRel; }
    virtual string getSortAttr() const { return sortAttr; }

 protected:
    bool bIterOpen;
    DataAttrInfo* attrs;
    int attrCount;
    stringstream explain;
    string indent;
    // ordering attributes
    bool bSorted;
    bool desc;
    string sortRel;
    string sortAttr;
};  // class Iterator

class TupleCmp {
 public:
  TupleCmp(AttrType     sortKeyType,
           int    sortKeyLength,
           int     sortKeyOffset,
           CompOp c)
    :c(c), p(sortKeyType, sortKeyLength, sortKeyOffset, c, NULL, NO_HINT),
    sortKeyOffset(sortKeyOffset)
    {}
  // default - not the most sensible - here so I can make arrays
  TupleCmp()
    :c(EQ_OP), p(INT, 4, 0, c, NULL, NO_HINT), sortKeyOffset(0)
    {}
  bool operator() (const Tuple& lhs, const Tuple& rhs) const
  {
      void * b = NULL;
      rhs.get(sortKeyOffset, b);
      const char * abuf;
      lhs.getData(abuf);
      return p.eval(abuf, (char*)b, c);
  }
 private:
  CompOp c;
  Predicate p;
  int sortKeyOffset;
}; // class TupleCmp

#endif // ITERATOR_H
