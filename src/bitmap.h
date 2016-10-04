#include <cmath>
#include <cstring>
#include <cassert>

#ifndef Record_BitMap
#define Record_BitMap

class bitmap {
public:
  bitmap(int numBits);
  bitmap(char *buf, int numBits);
  ~bitmap();

  void set(unsigned int bitNumber); // Set particular bit to 1
  void set(); // set all bits to 1
  void reset(unsigned int bitNumber); // Set particular bit to 0
  void reset(); // set all bits to 0
  bool test(unsigned int bitNumber) const; // test particular bit

  int numChars() const;
  int to_char_buf(char *, int len) const; // copy bitmap content to a  char buffer
  int getSize() const { return size; }
private:
  unsigned int size;
  char * buffer;
};

#endif
