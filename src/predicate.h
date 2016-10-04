#ifndef PRED_H
#define PRED_H

#include <iostream>
#include <cstdio>
#include "xbase.h"

using namespace std;

class Predicate {
public:
	Predicate() {}
	~Predicate() {}

	Predicate(AttrType attrTypeIn,
		      int attrLengthIn,
		      int attrOffsetIn,
		      CompOp compOpIn,
		      void *valueIn,
		      ClientHint pinHintIn)
	{
		attrType = attrTypeIn;
		attrLength = attrLengthIn;
		attrOffset = attrOffsetIn;
		compOp = compOpIn;
		value = valueIn;
		pinHint = pinHintIn;
	}

	CompOp initOp() const { return compOp; }
	bool eval(const char* buf, CompOp c);
	bool eval(const char* lhsbuf, const char* rhsValue, CompOp c);

 private:
 	AttrType attrType;
 	int attrLength;
 	int attrOffset;
 	CompOp compOp;
 	void* value;
 	ClientHint pinHint;	
};

#endif