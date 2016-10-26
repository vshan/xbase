#include <iostream>
#include <string>
#include <cstdlib>

using namespace std;
int main() {
  string s = itoa("50") + string("|") + string("goobly") + string("|") + string("twwonbly");
  cout << s << endl;
  return 0;
}