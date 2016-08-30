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

struct Style {
  TStyle* styler = new TStyle("Styler", "Style");
  Style() {
    setStyle();
  }
  void setStyle() {
    styler->SetOptStat(0);
    styler->SetOptTitle(0);
    styler->SetPalette(1,0);
    styler->SetOptStat(0);
    styler->SetOptTitle(0);
    styler->SetOptDate(0);
    styler->SetLabelSize(0.05, "xy");
    styler->SetTitleSize(0.12,"xy");
    styler->SetLabelOffset(0.007, "xy");
    styler->SetTitleOffset(0.9, "xy");
    styler->SetTitleFont(42,"xy");
    styler->SetLabelFont(42, "xy");
    styler->SetCanvasColor(0);
    styler->SetCanvasBorderMode(0);
    styler->SetCanvasBorderSize(3);

    styler->SetPadBottomMargin(0.05);
    styler->SetPadTopMargin(0.05);
    styler->SetPadLeftMargin(0.15);
    styler->SetPadRightMargin(0.05);
    styler->SetPadGridX(0);
    styler->SetPadGridY(0);
    styler->SetPadTickX(1);
    styler->SetPadTickY(1);
  
    styler->SetFrameBorderMode(0);
  }
  TStyle* getStyle() {
    return styler;
  }
};

void read_info(string, string&, map<string, Normer*>&);
int getModTime(const char*);
bool shouldAdd(string, string);

void CreateStack( TDirectory*, Plot&, ofstream&);
THStack* sortStack(THStack*);
TLegend* createLeg(TList* bgl=NULL, TList* sigl=NULL);
TGraphErrors* createError(TH1*, bool);
void sizePad(double, TVirtualPad*, bool);
TF1* createLine(TH1*);
void setXAxisTop(TH1*, TH1*, THStack*);
void setYAxisTop(TH1*, TH1*, double, THStack*);
void setXAxisBot(TH1*, TAxis*, double);
void setYAxisBot(TH1*, double);
vector<double> rebinner(TH1*, double);
double* rebinner(TH1*, TH1*, double);
THStack* rebinStack(THStack*, double*, int);
