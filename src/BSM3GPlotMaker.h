//////////////////////////////////////////////////////////////////////////////
// Authors:     Alfredo Gurrola, Andres Florez                              //
// contact:                                                                 //
//   Alfredo.Gurrola@cern.ch       (Vanderbilt University)                  //
//   Andres.Florez@cern.ch         (Los Andes University)                   //
//////////////////////////////////////////////////////////////////////////////

#ifndef BSM3GPlotMaker_h
#define BSM3GPlotMaker_h

// system include files
#include <memory>

// user include files
#include <Math/VectorUtil.h>
#include <fstream>
#include <TH1.h>
#include <TH2.h>
#include <TList.h>
#include <TFile.h>
#include <TTree.h>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <TRandom3.h>
#include <TMath.h>
#include <iostream>
#include <iomanip>
#include <utility>
#include <TROOT.h>
#include <TBranch.h>
#include <TApplication.h>
#include <TChain.h>
#include <TDirectory.h>
#include <TLorentzVector.h>
#include <TEnv.h>
#include <TError.h>
#include <TCollection.h>
#include <TKey.h>
#include <TGraphAsymmErrors.h>
#include <TClass.h>
#include <TCanvas.h>
#include <THStack.h>
#include <TStyle.h>
#include <TGraphPolar.h>
#include <TColor.h>
#include <TText.h>
#include <TLine.h>
#include <TLegend.h>
#include <TF1.h>
#include <TPaveText.h>
#include <cassert>

using namespace std;

class BSM3GPlotMaker {
public:
  BSM3GPlotMaker(char*);
  ~BSM3GPlotMaker();

private:

  virtual void beginJob();
  virtual void endJob();
  virtual void getInputs(char*);
  virtual void grabYieldsANDgrabHistos();
  virtual void makePlots();

  // initialize variables
  vector<string> inRootFiles;
  vector<string> inProcess;
  vector<string> inColors;
  vector<string> inFillStyles;
  vector<string> inLegend;
  vector<string> theHistNameStrings;
  vector<string> rebinNumber;
  vector<string> inScaleFactor;
  vector<string> inScaleFactorError;
  float lumi;
  string lumi_string;
  vector<TH1F*> HistList;
  vector<TH1F*> HistList2;
  int nHistList;
  int nHistList2;
  int nHistos;
  string outputLogFile;
  string outputRootFile;
  TFile *theCurrentFile;

};
#endif
