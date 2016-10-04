#include "bitmap.h"

using namespace std;

bitmap::bitmap(int numBits): size(numBits)
{
  buffer = new char[this->numChars()];
  memset((void*)buffer, 0, this->numChars());
  this->reset(); // So the buffer has a clean slate
}

bitmap::bitmap(char * buf, int numBits): size(numBits)
{
  buffer = new char[this->numChars()];
  memcpy(buffer, buf, this->numChars());
}

int bitmap::to_char_buf(char * b, int len) const
{
  assert(b != NULL && len == this->numChars());
  memcpy((void*)b, buffer, len);
  return 0;
}

bitmap::~bitmap()
{
  delete [] buffer;
}

int bitmap::numChars() const
{
  int numChars = (size / 8);
  if((size % 8) != 0)
    numChars++;
  return numChars;
}

void bitmap::reset()
{
  for( unsigned int i = 0; i < size; i++) {
    bitmap::reset(i);
  }
}

void bitmap::reset(unsigned int bitNumber)
{
  assert(bitNumber <= (size - 1));
  int byte = bitNumber/8;
  int offset = bitNumber%8;

  buffer[byte] &= ~(1 << offset);
}

void bitmap::set(unsigned int bitNumber)
{
  assert(bitNumber <= size - 1);
  int byte = bitNumber/8;
  int offset = bitNumber%8;

  buffer[byte] |= (1 << offset);
}

void bitmap::set()
{
  for( unsigned int i = 0; i < size; i++) {
    bitmap::set(i);
  }
}


bool bitmap::test(unsigned int bitNumber) const
{
  assert(bitNumber <= size - 1);
  int byte = bitNumber/8;
  int offset = bitNumber%8;

  return buffer[byte] & (1 << offset);
}
