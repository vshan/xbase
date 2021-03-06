#include "predicate.h"
#include <iostream>

using namespace std;

bool AlmostEqualRelative(float A, float B, float maxRelativeError=0.000001)
{
  if (A == B)
    return true;
  float relativeError = fabs((A-B)/B);
  if (relativeError <= maxRelativeError)
    return true;
  return false;
}

bool Predicate::eval(const char* buf, CompOp c)
{
  return this->eval(buf, NULL, c);
}

bool Predicate::eval(const char* lhsbuf, const char* rhsValue, CompOp c)
{
  const void * value_ = rhs;
  if (rhs == NULL)
    value_ = value;

  if (c == NO_OP || value_ == NULL) {
    return true;
  }

  const char * attr = buf + attrOffset;

  if (c == LT_OP) {
    if(attrType == INT) {
      return *((int *)attr) < *((int *)value_);
    }
    if (attrType == FLOAT) {
      return *((float *)attr) < *((float *)value_);
    }
    if (attrType == STRING) {
      return strcmp(attr, (char *)value_, attrLength) < 0;
    }
  }

  if (c == GT_OP) {
    if (attrType == INT) {
      return *((int *)attr) > *((int *)value_);
    }
    if (attrType == FLOAT) {
      return *((float *)attr) > *((float *)value_);
    }
    if (attrType == STRING) {
      return strcmp(attr, (char *)value_, attrLength) > 0;
    }
  }

  if (c == EQ_OP) {
    if (attrType == INT) {
      return *((int *)attr) == *((int *)value_);
    }
    if (attrType == FLOAT) {
      return *((float *)attr) == *((float *)value_);
    }
    if (attrType == STRING) {
      return strcmp(attr, (char *)value_, attrLength) == 0;
    }
  }

  if (c == LE_OP) {
    return this->eval(buf, rhs, LT_OP) || this->eval(buf, rhs, EQ_OP);
  }
  if (c == GE_OP) {
    return this->eval(buf, rhs, GT_OP) || this->eval(buf, rhs, EQ_OP);
  }
  if (c == NE_OP) {
    return !this->eval(buf, rhs, EQ_OP);
  }
  return true;
}