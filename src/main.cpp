#include "Plotter.h"
#include "Normalizer.h"

using namespace std;


Plot fullPlot;
double lumi;
string stylename = "default";

int main(int argc, char* argv[]) {
  if(argc < 2) {
    cerr << "No config file given: Exiting" << endl;
    exit(1);
  }

  string output;
  map<string, Normer*> plots;
  
  for(int i = 1; i < argc; ++i) {
    cout << argv[i] << " " << argv[i][0] << endl;
    if(argv[i][0] == '-') {
      cout << "here" << endl;
      if(strcmp(argv[i],"-help") == 0) {
	cout << "help is on the way" << endl;
	exit(0);
      } else if( strcmp(argv[i], "-sigleft") == 0) bottomType = SigLeft;
      else if( strcmp(argv[i], "-sigright") == 0) bottomType = SigRight;
      else if( strcmp(argv[i],"-sigbin") == 0) bottomType = SigBin;
      else {
	cout << "wrong option, exiting" << endl;
	exit(0);
      }
    } else read_info(argv[i], output, plots);
  }


  vector<string> datan, bgn, sign;
  for(map<string, Normer*>::iterator it = plots.begin(); it != plots.end(); ++it) {
    if(it->second->use == 0) continue;
    string filename = it->second->output;
    while(filename.find("#") != string::npos) {
      filename.erase(filename.find("#"), 1);
    }
    TFile* final = NULL;
    if(it->second->use == 1) {
      it->second->lumi = lumi;
      it->second->print();
      it->second->FileList = new TList();
      for(vector<string>::iterator name = it->second->input.begin(); name != it->second->input.end(); ++name) {
	it->second->FileList->Add(TFile::Open(name->c_str()));
      }
    
      final = new TFile(filename.c_str(), "RECREATE");
      it->second->MergeRootfile(final);
    } else if(it->second->use == 2) {
      final = new TFile(filename.c_str());
    }
    final->SetTitle(it->second->output.c_str());
    fullPlot.addFile(it->second->type, final);

    if(it->second->type == "data") datan.push_back(it->second->output);
    else if(it->second->type == "bg") bgn.push_back(it->second->output);
    else if(it->second->type == "sig") sign.push_back(it->second->output);
  }

  ofstream logfile = setupLogfile(datan, bgn, sign);

  TFile* final = new TFile(output.c_str(), "RECREATE");
  Style stylez("style/" + stylename);

  gStyle = stylez.getStyle();
  CreateStack(final, fullPlot, stylez, logfile);
  final->Close();
  logfile << "\\end{tabular}" << endl;
  logfile.close();
}

ofstream setupLogfile(vector<string> datan, vector<string> bgn, vector<string> sign) {
  ofstream logfile;
  logfile.open("log.txt", ios::out);
  logfile << "\\begin{tabular}{ | l |";
  for(int i = 0; i < datan.size()+bgn.size()+sign.size(); i++) {
    logfile << " c |";
  }
  logfile << " }" << endl << "\\hline" << endl << "Process";
  for(vector<string>::iterator it = datan.begin(); it != datan.end(); it++) logfile << " & " << it->substr(0, it->length()-5);
  for(vector<string>::iterator it = bgn.begin(); it != bgn.end(); it++) logfile << " & " << it->substr(0, it->length()-5);
  for(vector<string>::iterator it = sign.begin(); it != sign.end(); it++) logfile << " & " << it->substr(0, it->length()-5);
  logfile << "\\\\ \\hline" << endl;
  return logfile;
}



void read_info(string filename, string& output, map<string, Normer*>& plots) {
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  ifstream info_file(filename);
  boost::char_separator<char> sep(", \t");

  if(!info_file) {
    std::cout << "could not open file " << filename <<std::endl;
    exit(1);
  }

  vector<string> stemp;
  string group, line;
  while(getline(info_file, line)) {
    tokenizer tokens(line, sep);
    stemp.clear();
    for(tokenizer::iterator iter = tokens.begin();iter != tokens.end(); iter++) {
      if( ((*iter)[0] == '/' && (*iter)[0] == '/') || ((*iter)[0] == '#') ) break;
      stemp.push_back(*iter);

    }

    if(stemp.size() >= 2) {
      if(stemp[0].find("lumi") != string::npos) lumi = stod(stemp[1]);
      else if(stemp[0].find("output") != string::npos) output = stemp[1];
      else if(stemp[0].find("style") != string::npos) stylename = stemp[1];
      else if(plots.find(stemp[1]) == plots.end()) { 
	Normer* tmpplot = new Normer();
	tmpplot->input.push_back(stemp[0]);
	tmpplot->output = stemp[1];
	tmpplot->use = shouldAdd(stemp[0], stemp[1]);
	tmpplot->CumulativeEfficiency.push_back(1.);
	tmpplot->scaleFactor.push_back(1.);
	tmpplot->scaleFactorError.push_back(0.);
	tmpplot->integral.push_back(0);
	if(stemp.size() == 2) {
	  tmpplot->xsec.push_back(1.0);
	  tmpplot->skim.push_back(1.0);
	  tmpplot->isData = true;
	  tmpplot->type = "data";
	}    
	plots[stemp[1]] = tmpplot;
      } else {
	plots[stemp[1]]->input.push_back(stemp[0]);
	plots[stemp[1]]->use = min(shouldAdd(stemp[0], stemp[1]),plots[stemp[1]]->use);
	plots[stemp[1]]->CumulativeEfficiency.push_back(1.);
	plots[stemp[1]]->scaleFactor.push_back(1.);
	plots[stemp[1]]->scaleFactorError.push_back(0.);
	plots[stemp[1]]->integral.push_back(0);
	if(stemp.size() == 2) {
	  plots[stemp[1]]->xsec.push_back(1.0);
	  plots[stemp[1]]->skim.push_back(1.0);
	}    

      }
      if(stemp.size() == 5) {
	plots[stemp[1]]->xsec.push_back(stod(stemp[2]));
	plots[stemp[1]]->skim.push_back(stod(stemp[3]));
	if(plots[stemp[1]]->type == "") plots[stemp[1]]->type = stemp[4];
      }
    } 
  }
  info_file.close();

}

int shouldAdd(string infile, string globalFile) {
  struct stat buffer;
  if(stat(infile.c_str(), &buffer) != 0) return 0;
  ////need to change so doesn't make plot if fatal error (eg file doesn't exist!
  else if(stat(globalFile.c_str(), &buffer) != 0) return 1;
  else if(getModTime(globalFile.c_str()) > getModTime(infile.c_str())) return 2;
  else return 1;

}

int getModTime(const char *path) {
  struct stat attr;
  stat(path, &attr);
  char date[100] = {0};
  strftime(date, 100, "%s", localtime(&attr.st_mtime));
  return atoi(date);

}

