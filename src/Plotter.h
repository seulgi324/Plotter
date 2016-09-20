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
#include <TStyle.h>
#include <TROOT.h>

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

#include "Normalizer.h"
#include "Style.h"

using namespace std;

struct Plot {
  TList* FileList[3] = {new TList(), new TList(), new TList()};
  string style;

  int color[10] = {100, 90, 80, 70, 60, 95, 85, 75, 65, 55};

  void addFile(string type, TObject* obj) {
    if(type == "data") FileList[0]->Add(obj);
    else if(type == "bg") FileList[1]->Add(obj);
    else if(type == "sig") FileList[2]->Add(obj);
  }
};

void read_info(string, string&, map<string, Normer*>&);
int getModTime(const char*);
int shouldAdd(string, string);

TH1D* printBottom(TH1D*, TH1D*);
TList* signalBottom(TList*, TH1D*);
TList* signalBottom(TList*, TH1D*, TH1D*);

void CreateStack( TDirectory*, Plot&, Style&, ofstream&);
THStack* sortStack(THStack*);
TLegend* createLeg(TList* bgl=NULL, TList* sigl=NULL);
TGraphErrors* createError(TH1*, bool);
void sizePad(double, TVirtualPad*, bool);
TF1* createLine(TH1*);
void setXAxisTop(TH1*, TH1*, THStack*);
void setYAxisTop(TH1*, TH1*, double, THStack*);
void setXAxisBot(TH1*, double);
void setYAxisBot(TH1*, double);
vector<double> rebinner(TH1*, double);
double* rebinner(TH1*, TH1*, double);
THStack* rebinStack(THStack*, double*, int);
void divideBin(TH1*, TH1*,THStack*);

enum Bottom {SigLeft, SigRight, SigBoth, SigBin, Ratio};

Bottom bottomType = Ratio;
