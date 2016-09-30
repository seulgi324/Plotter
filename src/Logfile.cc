#include "Logfile.h"

using namespace std;

void Logfile::setHeader(vector<string> plotnames) {

  int totalfiles = plotnames.size();

  logfile << "\\begin{tabular}{ | l |";
  for(int i = 0; i < totalfiles; i++) {
    logfile << " c |";
  }
  logfile << " }" << endl << "\\hline" << endl << "Process";

  for(vector<string>::iterator it = plotnames.begin(); it != plotnames.end(); it++) 
    logfile << " & " << it->substr(0, it->length()-5);

  logfile << "\\\\ \\hline" << endl;
}

void Logfile::addLine(vector<string> values) {
  vector<string>::iterator it = values.begin();
  logfile << *it;
  for(; it != values.end(); it++) {
    logfile << " & " << *it;
  }
  logfile << " \\\\ \\hline" << endl;
}

void Logfile::setTrailer() {
  logfile << "\\end{tabular}" << endl;
}




