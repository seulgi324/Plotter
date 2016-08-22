//////////////////////////////////////////////////////////////////////////////
// Authors:     Alfredo Gurrola, Andres Florez                              //
// contact:                                                                 //
//   Alfredo.Gurrola@cern.ch       (Vanderbilt University)                  //
//   Andres.Florez@cern.ch         (Los Andes University)                   //
//////////////////////////////////////////////////////////////////////////////

#ifndef BSM3GCutFlowEffTable_h
#define BSM3GCutFlowEffTable_h

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

class BSM3GCutFlowEffTable {
public:
  BSM3GCutFlowEffTable(char*);
  ~BSM3GCutFlowEffTable();

private:

  virtual void beginJob();
  virtual void endJob();
  virtual void getInputs(char*);
  virtual void createLogFile();

  // initialize variables
  vector<string> inRootFiles;
  vector<string> inProcess;
  vector<string> IsData;
  vector<double> finalYield;
  vector<double> finalUncertainty;
  string outputLogFile;
  TFile *theCurrentFile;

};
#endif
