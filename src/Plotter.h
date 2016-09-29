#ifndef _PLOTTER_H_
#define _PLOTTER_H_

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
#include <sstream>
#include <iomanip>
#include <regex>


#include "Normalizer.h"
#include "Style.h"
#include "Logfile.h"


enum Bottom {SigLeft, SigRight, SigBoth, SigBin, Ratio};



using namespace std;

class Plotter {
 public:
  TList* FileList[3] = {new TList(), new TList(), new TList()};
  Style styler;
  int color[10] = {100, 90, 80, 70, 60, 95, 85, 75, 65, 55};
  bool ssqrtsb = true;
  Bottom bottomType = Ratio;

  map<string, string> latexer = { {"GenTau", "#tau"}, {"GenHadTau", "#tau_h"}, {"GenMuon", "#mu"}, {"TauJet", "#tau"}, {"Muon", "#mu"}, {"DiMuon", "#mu, #mu"}, {"DiTau", "#tau, #tau"}, {"Tau", "#tau"}, {"DiJet", "jj"}, {"Met", "#cancel{E_T}"}};



  void addFile(Normer&);

  TH1D* printBottom(TH1D*, TH1D*);
  TList* signalBottom(TList*, TH1D*);
  TList* signalBottom(TList*, TH1D*, TH1D*);

  void CreateStack( TDirectory*, Logfile&); ///fix plot stuff
  THStack* sortStack(THStack*);
  TLegend* createLeg(TH1*, TList* bgl, TList* sigl);
  TGraphErrors* createError(TH1*, bool);
  void sizePad(double, TVirtualPad*, bool);
  TF1* createLine(TH1*);
  void setXAxisTop(TH1*, TH1*, THStack*);
  void setYAxisTop(TH1*, TH1*, double, THStack*);
  void setXAxisBot(TH1*, double);
  void setYAxisBot(TAxis*, TH1*, double);
  void setYAxisBot(TAxis*, TList*, double);
  vector<double> rebinner(TH1*, double);
  double* rebinner(TH1*, TH1*, double);
  THStack* rebinStack(THStack*, double*, int);
  void divideBin(TH1*, TH1*,THStack*);
  int getSize();
  vector<string> getFilenames(string option="all");
  void setStyle(Style&);
  void setBottomType(Bottom);
  void setSignificanceSSqrtB() {ssqrtsb = false;}

  string newLabel(string);
  string listParticles(string);
};





#endif
