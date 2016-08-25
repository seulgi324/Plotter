#include <TTree.h>
#include <TH1.h>
#include <TFile.h>
#include <TDirectory.h>
#include <TClass.h>
#include <TKey.h>
#include <TGraphAsymmErrors.h>
#include <TChain.h>
#include <TCanvas.h>
#include <TText.h>
#include <THStack.h>
#include <TPaveText.h>
#include <TGraphErrors.h>
#include <TLegend.h>
#include <TF1.h>

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
  string style;
  bool use;

  int color[10] = {100, 90, 80, 70, 60, 95, 85, 75, 65, 55};

  void print() {
    for(int i = 0; i < input.size(); ++i) {
      cout << input.at(i) ;

    }
    cout << endl;
  }
};



map<string, Plot*> plots;
string output;
double lumi;
string lumi_string;
TFile *theCurrentFile;




void setupFiles(const vector<string>&, vector<TFile*>&);
inline int rounder(double val);
void calculateEfficienciesAndErrors(int, double, Plot&);
void read_info(string);
int getModTime(const char*);
bool shouldAdd(string);
void createLogFile(string);
void MergeRootfile( TDirectory*, Plot*, Plot*, Plot*);
THStack* sortStack(THStack*);
TLegend* createLeg(TList* bgl=NULL, TList* sigl=NULL);
TGraphErrors* createError(TH1* error, bool ratio);


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
  //   TFile* final = new TFile(it->second->output.c_str(), "RECREATE");
  //   MergeRootfile(final, *it->second);
  //   final->Close();
  // }
  }
  TFile* final = new TFile(output.c_str(), "RECREATE");
  MergeRootfile(final, plots["data"], plots["background"], plots["signal"]);
  final->Close();
    
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
    if(stemp.size() == 1) output = stemp[0];
    else if(stemp.size() >= 2) {
      if(plots.find(stemp[1]) == plots.end()) { 
	Plot* tmpplot = new Plot;
	tmpplot->input.push_back(stemp[0]);
	tmpplot->use = shouldAdd(stemp[0]);
	plots[stemp[1]] = tmpplot;
      } else {
	plots[stemp[1]]->input.push_back(stemp[0]);
	plots[stemp[1]]->use *= shouldAdd(stemp[0]);
      }
      if(stemp.size() == 3) plots[stemp[1]]->style = stemp[2];
      
    } 
  }
  info_file.close();

}

bool shouldAdd(string infile) {
  struct stat buffer;
  if(stat(infile.c_str(), &buffer) != 0) return false;
  ////need to change so doesn't make plot if fatal error (eg file doesn't exist!
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


void MergeRootfile( TDirectory *target, Plot* data, Plot* bg, Plot* sig=NULL) {

  TList* datalist = data->FileList;
  TList* bglist = bg->FileList;
  if(sig != NULL) TList* sglist = sig->FileList;

  TString path( (char*)strstr( target->GetPath(), ":" ) );
  path.Remove( 0, 2 );

  TFile *dataStart = (TFile*)datalist->First();
  dataStart->cd( path );
  TDirectory *current_sourcedir = gDirectory;

  Bool_t status = TH1::AddDirectoryStatus();
  TH1::AddDirectory(kFALSE);


  // loop over all keys in this directory
  TChain *globChain = 0;
  TIter nextkey( current_sourcedir->GetListOfKeys() );
  TKey *key, *oldkey=0;
  while ( (key = (TKey*)nextkey())) {

    //keep only the highest cycle number for each key
    if (oldkey && !strcmp(oldkey->GetName(),key->GetName())) continue;

    dataStart->cd( path );

    TObject *obj = key->ReadObj();
    if ( obj->IsA() ==  TH1F::Class() ) {
      
      TH1 *h1 = (TH1*)obj;
      TH1D* error = new TH1D("error", "error", h1->GetXaxis()->GetNbins(), h1->GetXaxis()->GetXmin(), h1->GetXaxis()->GetXmax());
      TH1D* datahist = new TH1D("data", "data", h1->GetXaxis()->GetNbins(), h1->GetXaxis()->GetXmin(), h1->GetXaxis()->GetXmax());
      TH1D* sighist = new TH1D("signal", "signal", h1->GetXaxis()->GetNbins(), h1->GetXaxis()->GetXmin(), h1->GetXaxis()->GetXmax());
      THStack *hs = new THStack(h1->GetName(),h1->GetName());
      
      /*------------data--------------*/
      datahist->Add(h1);
      TFile *nextfile = (TFile*)datalist->After(dataStart);

      while ( nextfile ) {
	nextfile->cd( path );
	TKey *key2 = (TKey*)gDirectory->GetListOfKeys()->FindObject(h1->GetName());
	if(key2) {
	  TH1* h2 = (TH1*)key2->ReadObj();
	  datahist->Add(h2);
	  delete h2;
	}
	nextfile = (TFile*)datalist->After(nextfile);
      }

      /*------------background--------------*/
      int nfile = 0;
      nextfile = (TFile*)bglist->First();

      while ( nextfile ) {

	nextfile->cd( path );
	TKey *key2 = (TKey*)gDirectory->GetListOfKeys()->FindObject(h1->GetName());
	if(key2) {
	  TH1* h2 = (TH1*)key2->ReadObj();
	  error->Add(h2);
	  for(int i = 1; i < h2->GetXaxis()->GetNbins()+1; i++) {
	    h2->SetBinError(i, 0);
	  }
	  //////style
	  h2->SetLineColor(1);
	  h2->SetFillStyle(1001);
	  h2->SetFillColor(bg->color[nfile]);
	  //////endstyle
	  hs->Add(h2);
	  //	  delete h2;
	}
	nextfile = (TFile*)bglist->After(nextfile);
	nfile++;
      }


      /*------------signal--------------*/

      /*--------------write out------------*/
      datahist->SetMarkerStyle(20);
      datahist->SetLineColor(1);

      hs = sortStack(hs);
      TLegend* legend = createLeg(hs->GetHists());
      legend->AddEntry(datahist, "Data", "f");
      TGraphErrors* errorstack = createError(error, false);
      TGraphErrors* errorratio = createError(error, true);
      TH1D* data_mc = (TH1D*)datahist->Clone("data_mc");
      data_mc->Divide(error);
      data_mc->SetMinimum(0.0);
      data_mc->SetMaximum(2.99);
      //      TF1 *PrevFitTMP = new TF1("PrevFitTMP","pol0",-10000,10000);
      TF1 *PrevFitTMP = new TF1("PrevFitTMP","pol0",-10000,10000);
      PrevFitTMP->SetFillColor(19);
      PrevFitTMP->SetFillStyle(0);
      PrevFitTMP->SetMarkerStyle(20);
      PrevFitTMP->SetLineColor(2);
      PrevFitTMP->SetLineWidth(1);
      PrevFitTMP->GetXaxis()->SetLabelFont(42);
      PrevFitTMP->GetXaxis()->SetLabelOffset(0.007);
      PrevFitTMP->GetXaxis()->SetLabelSize(0.05);
      PrevFitTMP->GetXaxis()->SetTitleSize(0.06);
      PrevFitTMP->GetXaxis()->SetTitleOffset(1.1);
      PrevFitTMP->GetXaxis()->SetTitleFont(42);
      PrevFitTMP->GetYaxis()->SetLabelFont(42);
      PrevFitTMP->GetYaxis()->SetLabelOffset(0.007);
      PrevFitTMP->GetYaxis()->SetLabelSize(0.05);
      PrevFitTMP->GetYaxis()->SetTitleSize(0.06);
      PrevFitTMP->GetYaxis()->SetTitleOffset(1.15);
      PrevFitTMP->GetYaxis()->SetTitleFont(42);
      PrevFitTMP->SetParameter(0,1.0);
      PrevFitTMP->SetParError(0,0);
      PrevFitTMP->SetParLimits(0,0,0);

      data_mc->GetListOfFunctions()->Add(PrevFitTMP);
      

      target->cd();

      TCanvas *c = new TCanvas(h1->GetName(), h1->GetName());//403,50,600,600);
      c->Divide(1,2);
      c->cd(1);
      hs->Draw("");
      datahist->Draw("sameep");
      errorstack->Draw("2");
      legend->Draw();

      c->cd(2);
      data_mc->Draw("ep1");
      errorratio->Draw("2");

      c->cd();
      c->Write(c->GetName());
      c->Close();

      delete error;
      delete datahist;
      delete sighist;
      delete hs;
      delete h1;
      delete legend;
      delete errorstack;

    } else if ( obj->IsA()->InheritsFrom( TDirectory::Class() ) ) {
      // it's a subdirectory

      //      cout << "Found subdirectory " << obj->GetName() << endl;

      // create a new subdir of same name and title in the target file
      target->cd();
      TDirectory *newdir = target->mkdir( obj->GetName(), obj->GetTitle() );

      // newdir is now the starting point of another round of merging
      // newdir still knows its depth within the target file via
      // GetPath(), so we can still figure out where we are in the recursion
      MergeRootfile( newdir, data, bg, sig );

    } else if ( obj->IsA()->InheritsFrom( TH1::Class() ) ) {
      continue;

    } else {
         cout << "Unknown object type, name: "
	   << obj->GetName() << " title: " << obj->GetTitle() << endl;
    }


  }
  // save modifications to target file
  //  target->SaveSelf(kTRUE);
  TH1::AddDirectory(status);
}

THStack* sortStack(THStack* old) {
  if(old == NULL || old->GetNhists() == 0) return old;
  string name = old->GetName();

  THStack* newstack = new THStack(name.c_str(),name.c_str());

  TList* list = (TList*)old->GetHists();

  while(list->GetSize() > 0) {
    TIter next(list);
    TH1* smallest = NULL;
    TH1* tmp = NULL;
    while ( (tmp = (TH1*)next()) ) {
      if(smallest == NULL || smallest->Integral() > tmp->Integral()) smallest = tmp;
    }
    newstack->Add(smallest);
    list->Remove(smallest);

  }
  
  delete old;
  return newstack;
} 

TLegend* createLeg(TList* bgl, TList* sigl) {
  TLegend* leg = new TLegend(0.6082412,0.4248252,0.8208543,0.7867133);
  vector<TList*> vlist;
  if(bgl!=NULL) vlist.push_back(bgl);
  if(sigl!=NULL) vlist.push_back(sigl);
  for(vector<TList*>::iterator it = vlist.begin(); it != vlist.end(); ++it) {
    TIter next(*it);
    TH1* tmp = NULL;
    while( (tmp = (TH1*)next()) ) {
      leg->AddEntry(tmp, "", "f");
    }
  }
  return leg;

}

TGraphErrors* createError(TH1* error, bool ratio) {
  Double_t mcX[5000];
  Double_t mcY[5000];
  Double_t mcErrorX[5000];
  Double_t mcErrorY[5000];

  for(int bin=0; bin < error->GetXaxis()->GetNbins(); bin++) {
    mcY[bin] = (ratio) ? 1.0 : error->GetBinContent(bin+1);
    mcErrorY[bin] = (ratio) ?  error->GetBinError(bin+1)/error->GetBinContent(bin+1) : error->GetBinError(bin+1);
    mcX[bin] = error->GetBinCenter(bin+1);
    mcErrorX[bin] = error->GetBinWidth(bin+1) * 0.5;
  }
  TGraphErrors *mcError = new TGraphErrors(error->GetXaxis()->GetNbins(),mcX,mcY,mcErrorX,mcErrorY);

  mcError->SetLineWidth(1);
  mcError->SetFillColor(1);
  mcError->SetFillStyle(3002);
  return mcError;
}
