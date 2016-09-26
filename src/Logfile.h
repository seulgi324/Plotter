#ifndef _LOGFILE_H_
#define _LOGFILE_H_

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <vector>

using namespace std;

class Logfile {
 public:
  ofstream logfile;
  
  Logfile() { logfile.open("log.txt", ios::out);  }
  ~Logfile() { logfile.close(); }
  Logfile(string logname) { logfile.open(logname, ios::out); }


  void setHeader(vector<string>);
  void addLine(vector<string>);
  void setTrailer();
};

#endif
