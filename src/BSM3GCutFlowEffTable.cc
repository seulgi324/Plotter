////////////////////////////////////////////////////////////////////////////// 
// Author:	Alfredo Gurrola                                             //
// contact:     Alfredo.Gurrola@cern.ch       (Vanderbilt University)       // 
//////////////////////////////////////////////////////////////////////////////

#include "BSM3GCutFlowEffTable.h"

//---main function
int main (int argc, char *argv[]) {

  BSM3GCutFlowEffTable BSM3GCutFlowEffTable_(argv[1]);

}

BSM3GCutFlowEffTable::BSM3GCutFlowEffTable(char* fname) {

  //---obtain the user defined inputs
  std::cout << "" << std::endl;
  std::cout << "BSM3GCutFlowEffTable Message: Grabbing user defined inputs (i.e. root files)" << std::endl;
  std::cout << "" << std::endl;
  beginJob();
  getInputs(fname);
  std::cout << "" << std::endl;
  std::cout << "BSM3GCutFlowEffTable Message: Finished grabbing user inputs." << std::endl;
  std::cout << "" << std::endl;
  if(inRootFiles.size() > 0) {
    createLogFile();
  }
  std::cout << "" << std::endl;
  std::cout << "BSM3GCutFlowEffTable Message: Finished making cut flow efficiency table. Closing opened root files." << std::endl;
  std::cout << "" << std::endl;
  endJob();
  std::cout << "" << std::endl;
  std::cout << "BSM3GCutFlowEffTable Message: Finished closing root files." << std::endl;
  std::cout << "" << std::endl;

}

//---function used to obtain the configurable inputs from the .in files
void BSM3GCutFlowEffTable::getInputs(char* fname) {

  string inputString;
  string inputType;

  // open input file containing all information: root files, scale factors, cross-sections, etc
  ifstream inFile;
  inFile.open(fname, ios::in);
  // if can't open input file, exit the code
  if (!inFile) {
    std::cerr << "     BSM3GCutFlowEffTable 'getInputs' function: Error!! --> Can't open the input configuration file " << fname << std::endl;
    exit(1);
  } else {
    std::cout << "     BSM3GCutFlowEffTable 'getInputs' function: input file '" << fname << "' has been successfully opened." << std::endl;
  }

  // grab all relevant information from the input file
  while (inFile >> inputType >> inputString) {
    if(inputType == "rootfile") {
      if(inputString != "BLANK") {inRootFiles.push_back(inputString);}
    } else if(inputType == "process") {
      if(inputString != "BLANK") {inProcess.push_back(inputString);}
    } else if(inputType == "IsData") {
      if(inputString != "BLANK") {IsData.push_back(inputString);}
    } else if(inputType == "outputLogFile") {
      if(inputString != "BLANK") {outputLogFile = inputString;} 
    } else {
      std::cerr << "     BSM3GCutFlowEffTable 'getInputs' function: Error!! --> Incorrect input type " << inputType << std::endl; // exit code if unwanted input is specified
      exit(1);
    }
  }

  //---close the .in file
  inFile.close();

  std::cout << "     BSM3GCutFlowEffTable 'getInputs' function: input file '" << fname << "' has been closed." << std::endl;

}

//---function called once just before obtaining user inputs. It clears the vectors and initializes parameters.
void BSM3GCutFlowEffTable::beginJob() {

  inRootFiles.clear();
  inProcess.clear();
  IsData.clear();
  finalYield.clear();
  finalUncertainty.clear();
  outputLogFile = "temp.log";

  std::cout << "     BSM3GCutFlowEffTable 'beginJob' function: Vectors have been cleared and variables initialized." << std::endl;

}

//---function called once just before obtaining user inputs. It clears the vectors.
void BSM3GCutFlowEffTable::createLogFile() {

  // create an output log file that will contain the cut flow efficiency table
  ofstream outFile;
  outFile.open(outputLogFile.c_str(), ios::out);
  // if output log file cannot be opened, exit the code
  if (!outFile) {
    std::cerr << "     BSM3GCutFlowEffTable 'createLogFile' function: Can't open output file " << outputLogFile << std::endl;
    exit(1);
  } else {
    std::cout << "     BSM3GCutFlowEffTable 'createLogFile' function: output file '" << outputLogFile << "' has been successfully opened/created." << std::endl;
  }

  theCurrentFile = (TFile*) TFile::Open (inRootFiles.at(0).c_str()); // open root file
  if (!theCurrentFile) {
    std::cerr << "     BSM3GCutFlowEffTable 'createLogFile' function: Error!! --> Can't open input file " << inRootFiles.at(0).c_str() << " (or file does not exist)" << std::endl;
    exit(1);
  } else {
    std::cout << "     BSM3GCutFlowEffTable 'createLogFile' function: input file '" << inRootFiles.at(0).c_str() << "' has been successfully opened ... determining number of cuts." << std::endl;
  }
  TDirectory *current_sourcedir = gDirectory; // grab the directory path within the root file
  TIter nextkey( current_sourcedir->GetListOfKeys() );
  TKey *key;
  int num=0;
  // loop over keys(ROOT objects = histograms) within the root file
  while((key = (TKey*)nextkey())) {
    TObject *obj = key->ReadObj();
    string histname = obj->GetName();
    string y = histname.substr(0,4);
    if(y == "Met_") {
      num++;
    }
  }
  theCurrentFile->Close();

  if(num <= 0) {
    std::cerr << "     BSM3GCutFlowEffTable 'createLogFile' function: Error!! Histograms needed to produce the cut flow table do not exist!" << std::endl;
    return;
  }
  outFile << "\\begin{tabular}{ | l |";
  for(int i = 0; i < num; i++) {
    outFile << " c |";
  }
  outFile << " }" << endl;

  outFile << "\\hline" << endl;
  outFile << "Process & ";

  theCurrentFile = (TFile*) TFile::Open (inRootFiles.at(0).c_str()); // open root file
//  TDirectory *current_sourcedir = gDirectory; // grab the directory path within the root file
  current_sourcedir = gDirectory; // grab the directory path within the root file
  TIter nextNextkey( current_sourcedir->GetListOfKeys() );
  TKey *newkey;
  int numNum = 0;
  // loop over keys(ROOT objects) within the root file
  while((newkey = (TKey*)nextNextkey())) {
    TObject *obj = newkey->ReadObj();
    string histname = obj->GetName();
    string y = histname.substr(0,4);
    if(y == "Met_") {
      numNum++;
      if(numNum < num) {
        outFile << "Cut " << numNum << " & ";
      } else {
        outFile << "Cut " << numNum << " \\\\ \\hline" << endl;
      }
    }
//    delete obj;
  }
  theCurrentFile->Close();

  // loop over root files
  for(int j=0;j<inRootFiles.size();j++) {
    theCurrentFile = (TFile*) TFile::Open (inRootFiles.at(j).c_str()); // open root file
    TDirectory *current_sourcedir = gDirectory; // grab the directory path within the root file
    TIter nextkey( current_sourcedir->GetListOfKeys() );
    TKey *key;
    int numNum = 0;
    // loop over keys(ROOT objects = histograms) within the root file
    while((key = (TKey*)nextkey())) {
      TObject *obj = key->ReadObj();
      string histname = obj->GetName();
      string y = histname.substr(0,4);
      if(y == "Met_") {
        numNum++;
        if(numNum == num) {
          TH1 *hobj = (TH1*)obj;
          hobj->Rebin(hobj->GetNbinsX());
          finalYield.push_back(hobj->Integral(0,10000));
          finalUncertainty.push_back(hobj->GetBinError(1));
//          finalUncertainty.push_back(sqrt(pow(hobj->GetBinError(1),2.) - hobj->Integral(0,10000))); // subtracting off poisson error (sqrt[yield])
//          delete hobj;
        }
      } 
//      delete obj;
    }
    theCurrentFile->Close();
  }

  // loop over root files
  for(int j=0;j<inRootFiles.size();j++) {
//    outFile << inProcess.at(j) << " & " << fixed << setprecision(1);
    outFile << inProcess.at(j) << " & ";
    theCurrentFile = (TFile*) TFile::Open (inRootFiles.at(j).c_str()); // open root file
    TDirectory *current_sourcedir = gDirectory; // grab the directory path within the root file
    TIter nextkey( current_sourcedir->GetListOfKeys() );
    TKey *key;
    int numNum = 0;
    // loop over keys(ROOT objects = histograms) within the root file
    while((key = (TKey*)nextkey())) {
      TObject *obj = key->ReadObj();
      string histname = obj->GetName();
      string y = histname.substr(0,4);
      if(y == "Met_") {
        numNum++;
        TH1 *hobj = (TH1*)obj;
        hobj->Rebin(hobj->GetNbinsX());
        if(IsData.at(j) == "1") {
          if(numNum < num) {
            outFile << (int)(hobj->Integral(0,10000)+0.5) << " & ";
          } else {
            outFile << (int)(finalYield.at(j) + 0.5) << " \\" << "\\" << " \\hline" << endl;
          }
        } else {
          if(numNum < num) {
            if(hobj->Integral(0,10000) < 1000.0) {
              outFile << fixed << setprecision(1) << hobj->Integral(0,10000) << " $\\pm$ " << fixed << setprecision(1) << hobj->GetBinError(1) << " & ";
            } else {
//              outFile << setprecision(2) << hobj->Integral(0,10000) << " $\\pm$ " << setprecision(2) << hobj->GetBinError(1) << " & ";
              outFile << fixed << setprecision(1) << hobj->Integral(0,10000) << " $\\pm$ " << fixed << setprecision(1) << hobj->GetBinError(1) << " & ";
            }
          } else {
            if(finalYield.at(j) < 1000.0) {
              outFile << fixed << setprecision(1) << finalYield.at(j) << " $\\pm$ " << fixed << setprecision(1) << finalUncertainty.at(j) << " \\" << "\\" << " \\hline" << endl;
            } else {
//              outFile << setprecision(2) << finalYield.at(j) << " $\\pm$ " << setprecision(2) << finalUncertainty.at(j) << " \\" << "\\" << " \\hline" << endl;
              outFile << fixed << setprecision(2) << finalYield.at(j) << " $\\pm$ " << fixed << setprecision(2) << finalUncertainty.at(j) << " \\" << "\\" << " \\hline" << endl;
            }
          }
        }
//        delete hobj;
      } 
//      delete obj;
    }
    theCurrentFile->Close();
  }

  outFile << "\\end{tabular}";

  outFile.close();

}

//---function called once just before obtaining user inputs. It clears the vectors and initializes parameters.
void BSM3GCutFlowEffTable::endJob() {

  inRootFiles.clear();
  inProcess.clear();
  IsData.clear();
  finalYield.clear();
  finalUncertainty.clear();
  theCurrentFile->Close();

}

BSM3GCutFlowEffTable::~BSM3GCutFlowEffTable() { };
