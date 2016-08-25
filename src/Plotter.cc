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
      if(!plotter.isData) h1->Scale(plotter.CumulativeEfficiency.at(spot)/plotter.events.at(spot) * plotter.xsec.at(spot)* lumi* plotter.skim.at(spot));

      TFile *nextsource = (TFile*)sourcelist->After( first_source );
      
      while ( nextsource ) {
	spot++;
	nextsource->cd( path );
	TKey *key2 = (TKey*)gDirectory->GetListOfKeys()->FindObject(h1->GetName());
	if (key2) {
	  TH1 *h2 = (TH1*)key2->ReadObj();
	  h2->Sumw2();
	  double scale = (plotter.isData) ? 1.0 : plotter.CumulativeEfficiency.at(spot)/plotter.events.at(spot) * plotter.xsec.at(spot)* lumi* plotter.skim.at(spot);
	  h1->Add( h2, scale);
	  delete h2;
	  
	}
	nextsource = (TFile*)sourcelist->After( nextsource );
      }
      ////////////////////////////////////////////////////////////
      ////  To gain back Poisson error, uncomment this line /////
      ////////////////////////////////////////////////////////////

      for(int ibin=0; ibin < (h1->GetXaxis()->GetNbins() + 1); ibin++) {
      	h1->SetBinError(ibin, sqrt(pow(h1->GetBinError(ibin),2.0) + h1->GetBinContent(ibin)) );
      }

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

 
// ///Main plot

//       TPad *c1_2 = new TPad("c1_2", "newpad",0.01,0.33,0.99,0.99);
//       c1_2->Draw();
//       c1_2->cd();
//       c1_2->Range(-200,-13.87376,1133.333,1389.866);
//       c1_2->SetFillColor(0);
//       c1_2->SetBorderMode(0);
//       c1_2->SetBorderSize(2);
//       c1_2->SetTickx(1);
//       c1_2->SetTicky(1);
//       c1_2->SetLeftMargin(0.15);
//       c1_2->SetBottomMargin(0.01);
//       c1_2->SetFrameFillStyle(0);
//       c1_2->SetFrameBorderMode(0);
//       c1_2->SetFrameFillStyle(0);
//       c1_2->SetFrameBorderMode(0);

//       data->SetMarkerStyle(20);
//       data->SetLineColor(1);
//       data->GetXaxis()->SetLabelFont(42);
//       data->GetXaxis()->SetLabelOffset(0.007);
//       data->GetXaxis()->SetLabelSize(0.11);
//       data->GetXaxis()->SetTitleSize(0.12);
//       data->GetXaxis()->SetTitleOffset(0.9);
//       data->GetXaxis()->SetTitleFont(42);

//       data->GetYaxis()->SetTitle("Events");
//       data->GetYaxis()->SetLabelFont(42);
//       data->GetYaxis()->SetLabelOffset(0.007);
//       data->GetYaxis()->SetLabelSize(0.05);
//       data->GetYaxis()->SetTitleSize(0.06);
//       data->GetYaxis()->SetTitleOffset(1.1);
//       data->GetYaxis()->SetTitleFont(42);
//       data->GetZaxis()->SetLabelFont(42);
//       data->GetZaxis()->SetLabelOffset(0.007);
//       data->GetZaxis()->SetLabelSize(0.05);
//       data->GetZaxis()->SetTitleSize(0.06);
//       data->GetZaxis()->SetTitleFont(42);
//       data->Draw("ep");
//       hs->Draw("same");

//       for(int bin=0; bin < data->GetXaxis()->GetNbins(); bin++) {
//         mcY[bin] = hError->GetBinContent(bin+1);
//         mcErrorY[bin] = hError->GetBinError(bin+1);
//         mcX[bin] = hError->GetBinCenter(bin+1);
//         mcErrorX[bin] = hError->GetBinWidth(bin+1) * 0.5;
//       }
//       TGraphErrors *mcError = new TGraphErrors(h->GetXaxis()->GetNbins(),mcX,mcY,mcErrorX,mcErrorY);
//       mcError->SetLineWidth(0);
//       mcError->SetFillColor(1);
//       mcError->SetFillStyle(3002);
//       mcError->Draw("sameE2");

//       hhh->SetMarkerStyle(20);
// //      hhh->SetMarkerSize(0.0);
//       hhh->SetLineColor(1);
//       hhh->Draw("samee1p");

//       legend->SetTextFont(42);
//       legend->SetTextSize(0.04195804);
//       legend->SetLineColor(1);
//       legend->SetLineStyle(1);
//       legend->SetLineWidth(1);
//       legend->SetFillColor(0);
//       legend->SetFillStyle(1001);
//       legend->SetBorderSize(0);
//       legend->SetFillColor(kWhite);
//       legend->Draw();

//       TPaveText *pt = new TPaveText(0.2620189,0.9409833,0.662649,0.9913117,"brNDC");
//       pt->SetBorderSize(0);
//       pt->SetFillColor(0);
//       pt->SetLineColor(0);
//       pt->SetTextSize(0.05297732);
//       string TitleString = "CMS Preliminary, L_{int} = " + lumi_string + " fb^{-1}, #sqrt{s} = 13 TeV";
//       TText *text = pt->AddText(TitleString.c_str());
//       text->SetTextFont(42);
//       pt->Draw();

//       c1_2->Modified();
//       c->cd();
//       c->Modified();
//       c->cd();
//       c->SetSelected(c);
//       c->Write();
//       c->Close();

//       delete hhh;
//       delete hError;
//       delete hData;
//       delete hMC;
