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
#include <unordered_map>
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
  
  bool ssqrtsb = true, onlyTop = false;
  Bottom bottomType = Ratio;

  static unordered_map<string, string> latexer;

  void addFile(Normer&);

  TH1D* printBottom(TH1D*, TH1D*);
  TList* signalBottom(const TList*, const TH1D*);
  TList* signalBottom(const TList*, const TH1D*, const TH1D*);

  void CreateStack( TDirectory*, Logfile&); ///fix plot stuff
  THStack* sortStack(THStack*);
  TLegend* createLeg(const TH1*, const TList*, const TList*);
  TGraphErrors* createError(const TH1*, bool);
  void sizePad(double, TVirtualPad*, bool);
  TF1* createLine(TH1*);
  void setXAxisTop(TH1*, TH1*, THStack*);
  void setYAxisTop(TH1*, TH1*, double, THStack*);
  void setXAxisBot(TH1*, double);
  void setYAxisBot(TAxis*, TH1*, double);
  void setYAxisBot(TAxis*, TList*, double);
  vector<double> rebinner(const TH1*, double);

  THStack* rebinStack(THStack*, const double*, int);
  void divideBin(TH1*, TH1*,THStack*, TList*);
  int getSize();
  vector<string> getFilenames(string option="all");
  void setStyle(Style&);
  void setBottomType(Bottom input) {bottomType = input;}
  void setSignificanceSSqrtB() {ssqrtsb = false;}
  void setNoBottom() {onlyTop = true;}

  string newLabel(string);
  string listParticles(string);
};

#endif
