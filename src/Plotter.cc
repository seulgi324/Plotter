#include <TTree.h>
#include <TH1.h>
#include <TFile.h>
#include <TDirectory.h>
#include <TClass.h>
#include <TKey.h>
#include <TGraphAsymmErrors.h>
#include <TChain.h>


#include <vector>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <map>
#include "tokenizer.hpp"
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <array>

using namespace std;

struct Plot {
  vector<string> input;
  TList* FileList;
  string output;
  vector<double> skim, xsec;
  vector<double> events, RelativeEffVector, RelativeEffErrorVector, CumulativeEfficiency, scaleFactor, scaleFactorError;
  bool use, isData=false;

  void print() {
    cout <<endl <<  " =========== " << output << " =========== " << endl;
    for(int i = 0; i < input.size(); ++i) {
      cout << input.at(i) ;
      if(isData) cout << endl;
      else cout << " " << skim.at(i) << " " << xsec.at(i) <<  endl;
    }
    cout << endl;
  }
};


vector<string> dirvec;
vector<array<double, 3>> histinfo;
vector<string> histnamevec;
map<string, Plot*> plots;

double lumi;
string lumi_string;
TFile *theCurrentFile;




void setupFiles(const vector<string>&, vector<TFile*>&);
inline int rounder(double val);
void calculateEfficienciesAndErrors(int, double, Plot&);
void read_info(string);
int getModTime(const char*);
bool shouldAdd(string, string);
void createLogFile(string);
void MergeRootfile( TDirectory*, Plot&);


int main(int argc, char* argv[]) {
  if(argc < 2) {
    cerr << "No config file given: Exiting" << endl;
    exit(1);
  }
  for(int i = 1; i < argc; ++i) {
    read_info(argv[i]);
  }


  for(map<string, Plot*>::iterator it = plots.begin(); it != plots.end(); ++it) {
    if(! it->second->use) continue;
    it->second->print();
    it->second->FileList = new TList();
    for(vector<string>::iterator name = it->second->input.begin(); name != it->second->input.end(); ++name) {
      it->second->FileList->Add(TFile::Open(name->c_str()));
    }
    TFile* final = new TFile(it->second->output.c_str(), "RECREATE");
    MergeRootfile(final, *it->second);
    final->Close();
  }

}


void read_info(string filename) {
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
      else if(plots.find(stemp[1]) == plots.end()) { 
	Plot* tmpplot = new Plot;
	tmpplot->input.push_back(stemp[0]);
	tmpplot->output = stemp[1];
	tmpplot->use = shouldAdd(stemp[0], stemp[1]);
	tmpplot->CumulativeEfficiency.push_back(1.);
	tmpplot->RelativeEffVector.push_back(1.);
	tmpplot->RelativeEffErrorVector.push_back(1.);
	tmpplot->scaleFactor.push_back(1.);
	tmpplot->scaleFactorError.push_back(0.);
	tmpplot->events.push_back(0);
	if(stemp.size() == 2) {
	  tmpplot->xsec.push_back(1.0);
	  tmpplot->skim.push_back(1.0);
	  tmpplot->isData = true;
	}    

	plots[stemp[1]] = tmpplot;
      } else {
	plots[stemp[1]]->input.push_back(stemp[0]);
	plots[stemp[1]]->use *= shouldAdd(stemp[0], stemp[1]);
	plots[stemp[1]]->CumulativeEfficiency.push_back(1.);
	plots[stemp[1]]->RelativeEffVector.push_back(1.);
	plots[stemp[1]]->RelativeEffErrorVector.push_back(1.);
	plots[stemp[1]]->scaleFactor.push_back(1.);
	plots[stemp[1]]->scaleFactorError.push_back(0.);
	plots[stemp[1]]->events.push_back(0);
	if(stemp.size() == 2) {
	  plots[stemp[1]]->xsec.push_back(1.0);
	  plots[stemp[1]]->skim.push_back(1.0);
	}    
      }
    } 
    if(stemp.size() == 4) {
      plots[stemp[1]]->xsec.push_back(stod(stemp[2]));
      plots[stemp[1]]->skim.push_back(stod(stemp[3]));
    }
  }
  info_file.close();

}

bool shouldAdd(string infile, string globalFile) {
  struct stat buffer;
  if(stat(infile.c_str(), &buffer) != 0) return false;
  ////need to change so doesn't make plot if fatal error (eg file doesn't exist!
  else if(stat(globalFile.c_str(), &buffer) != 0) return true;
  else if(getModTime(globalFile.c_str()) > getModTime(infile.c_str())) return true;
  else return true;

}

int getModTime(const char *path) {
  struct stat attr;
  stat(path, &attr);
  char date[100] = {0};
  strftime(date, 100, "%s", localtime(&attr.st_mtime));
  return atoi(date);

}


inline int rounder(double val) {
  return (int)(val + 0.5);
}


void MergeRootfile( TDirectory *target, Plot& plotter ) {

  TList* sourcelist = plotter.FileList;

  TString path( (char*)strstr( target->GetPath(), ":" ) );
  path.Remove( 0, 2 );

  TFile *first_source = (TFile*)sourcelist->First();
  first_source->cd( path );
  TDirectory *current_sourcedir = gDirectory;
  //gain time, do not add the objects in the list in memory
  Bool_t status = TH1::AddDirectoryStatus();
  TH1::AddDirectory(kFALSE);


  ///try to find events to calculate efficiency
  TH1F* events;
  current_sourcedir->GetObject("Events", events);

  if(events) {
    int nplot = 0;

    plotter.events.at(nplot) = events->GetBinContent(2);
    plotter.CumulativeEfficiency.at(nplot) = events->GetBinContent(2)/ events->GetBinContent(1);

    //  double cumulEff = events->GetBinContent(2)/ events->GetBinContent(1);
    //    calculateEfficienciesAndErrors(nplot, cumulEff, plotter);

    TFile *nextsource = (TFile*)sourcelist->After( first_source );
    while( nextsource) {
      nplot++;
      nextsource->cd(path);
      gDirectory->GetObject("Events", events);
      plotter.events.at(nplot) = events->GetBinContent(2);
      plotter.CumulativeEfficiency.at(nplot) = events->GetBinContent(2)/ events->GetBinContent(1);
      //      double cumulEff = events->GetBinContent(2)/ events->GetBinContent(1);
      //      calculateEfficienciesAndErrors(nplot, cumulEff, plotter);
      nextsource = (TFile*)sourcelist->After( nextsource );
    }
  }
  delete events;

  // loop over all keys in this directory
  TChain *globChain = 0;
  TIter nextkey( current_sourcedir->GetListOfKeys() );
  TKey *key, *oldkey=0;
  while ( (key = (TKey*)nextkey())) {

    //keep only the highest cycle number for each key
    if (oldkey && !strcmp(oldkey->GetName(),key->GetName())) continue;

    first_source->cd( path );

    TObject *obj = key->ReadObj();
    if ( obj->IsA()->InheritsFrom( TH1::Class() ) ) {
      TH1 *h1 = (TH1*)obj;
      h1->Sumw2();
      int spot = 0;
      h1->Scale(plotter.CumulativeEfficiency.at(spot)/plotter.events.at(spot) * plotter.xsec.at(spot)* lumi* plotter.skim.at(spot));

      TFile *nextsource = (TFile*)sourcelist->After( first_source );
      
      while ( nextsource ) {
	spot++;
	nextsource->cd( path );
	TKey *key2 = (TKey*)gDirectory->GetListOfKeys()->FindObject(h1->GetName());
	if (key2) {
	  TH1 *h2 = (TH1*)key2->ReadObj();
	  h2->Sumw2();
	  h1->Add( h2, plotter.CumulativeEfficiency.at(spot)/plotter.events.at(spot) * plotter.xsec.at(spot)* lumi* plotter.skim.at(spot));
	  delete h2;
	  
	}
	nextsource = (TFile*)sourcelist->After( nextsource );
      }
      ////////////////////////////////////////////////////////////
      ////  To gain back Poisson error, uncomment this line /////
      ////////////////////////////////////////////////////////////

      // for(int ibin=0; ibin < (h1->GetXaxis()->GetNbins() + 1); ibin++) {
      // 	h1->SetBinError(ibin, sqrt(pow(h1->GetBinError(ibin),2.0) + h1->GetBinContent(ibin)) );
      // }

    }
    else if ( obj->IsA()->InheritsFrom( TTree::Class() ) ) {

      // loop over all source files create a chain of Trees "globChain"
      const char* obj_name= obj->GetName();

      globChain = new TChain(obj_name);
      globChain->Add(first_source->GetName());
      TFile *nextsource = (TFile*)sourcelist->After( first_source );
      //      const char* file_name = nextsource->GetName();
      // cout << "file name  " << file_name << endl;
      while ( nextsource ) {
	  
	globChain->Add(nextsource->GetName());
	nextsource = (TFile*)sourcelist->After( nextsource );
      }
	
    } else if ( obj->IsA()->InheritsFrom( TDirectory::Class() ) ) {
      // it's a subdirectory

      //      cout << "Found subdirectory " << obj->GetName() << endl;

      // create a new subdir of same name and title in the target file
      target->cd();
      TDirectory *newdir = target->mkdir( obj->GetName(), obj->GetTitle() );

      // newdir is now the starting point of another round of merging
      // newdir still knows its depth within the target file via
      // GetPath(), so we can still figure out where we are in the recursion
      MergeRootfile( newdir, plotter );

    } else {

      // object is of no type that we know or can handle
      cout << "Unknown object type, name: "
	   << obj->GetName() << " title: " << obj->GetTitle() << endl;
    }

    // now write the merged histogram (which is "in" obj) to the target file
    // note that this will just store obj in the current directory level,
    // which is not persistent until the complete directory itself is stored
    // by "target->Write()" below
    if ( obj ) {
      target->cd();
      if(obj->IsA()->InheritsFrom( TTree::Class() ))
	globChain->Merge(target->GetFile(),0,"keep");
      else
	obj->Write( key->GetName() );
    }

  } // while ( ( TKey *key = (TKey*)nextkey() ) )

  // save modifications to target file
  target->SaveSelf(kTRUE);
  TH1::AddDirectory(status);
}

 

void calculateEfficienciesAndErrors(int i, double cumulEff, Plot& plot) {

  double numerator;
  double denominator;
  double relEff;
  double efferror;


  // Calculate relative and cumulative cut efficiencies
  numerator = rounder(cumulEff);
  denominator = rounder(plot.CumulativeEfficiency.at(i)); ///set cumul to 1 initially

  if(numerator > 0) {
    relEff = numerator / denominator;
    efferror = sqrt(relEff*(1.-relEff)/denominator);
  } else {
    relEff = 0.0;
    efferror = (denominator != 0.0) ? 1.0 / denominator : 0.0;
  }

  /* binomial uncertainties do not work when the efficiency gets close to 1 or 0 (e.g. efficiency cannot
     be 99 +- 2 because the efficiency cannot be e.g. 101) ... in these cases, use bayesian */
  if(((relEff + efferror) > 1.) || ((relEff - efferror) < 0.)){
    TH1F theNumHisto("theNumHisto","theNumHisto",1,0,1);
    theNumHisto.SetBinContent(1,numerator);
    theNumHisto.Sumw2();
    TH1F theDenHisto("theDenHisto","",1,0,1);
    theDenHisto.SetBinContent(1,denominator);
    theDenHisto.Sumw2();
    TGraphAsymmErrors bayesEff;
    bayesEff.BayesDivide(&theNumHisto, &theDenHisto,"b");
    efferror = (bayesEff.GetErrorYhigh(0) > bayesEff.GetErrorYlow(0)) ? bayesEff.GetErrorYhigh(0) : bayesEff.GetErrorYlow(0);
  }
  if(relEff == 1.0) efferror = 0;

  //inscalefactors???

  // recalculate efficiencies and errors incorporating scale factors
  relEff *= plot.scaleFactor.at(i);
  if(numerator > 0) {
    efferror = sqrt(pow(efferror*plot.scaleFactor.at(i),2.0) + pow(plot.scaleFactorError.at(i)*relEff,2.0));
  } else {
    efferror = (denominator != 0) ? plot.scaleFactor.at(i) / denominator : 0.0;
  }

  plot.RelativeEffVector.at(i) = relEff;
  plot.RelativeEffErrorVector.at(i) = efferror;
  plot.CumulativeEfficiency.at(i) = cumulEff;

}
