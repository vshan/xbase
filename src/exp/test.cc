#include <iostream>
#include <string.h>
#include <stdio.h>

using namespace std;
int main() {
  char itoastr[20];
  char msg[200];
  sprintf(itoastr, "%d", 50);
  string itoas(itoastr);
  string s = itoas + string("|") + string("goobly") + string("|") + string("twwonbly");
  memcpy((void *)msg, (void *)s.c_str(), s.size());
  msg[s.size()] = '\0'; 
  cout << msg << endl;
  return 0;
}