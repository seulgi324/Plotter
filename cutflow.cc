////////////////////////////////////////////////////////////////////////////// 
// Author:	Alfredo Gurrola                                             //
// contact:     Alfredo.Gurrola@cern.ch       (Vanderbilt University)       // 
//////////////////////////////////////////////////////////////////////////////



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
    outFile << inProcess.at(j) << " & ";
    theCurrentFile = (TFile*) TFile::Open (inRootFiles.at(j).c_str()); // open root file
    TDirectory *current_sourcedir = gDirectory; // grab the directory path within the root file
    TIter nextkey( current_sourcedir->GetListOfKeys() );
    TKey *key;
    int numNum = 0;

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
              outFile << fixed << setprecision(1) << hobj->Integral(0,10000) << " $\\pm$ " << fixed << setprecision(1) << hobj->GetBinError(1) << " & ";
            }
          } else {
            if(finalYield.at(j) < 1000.0) {
              outFile << fixed << setprecision(1) << finalYield.at(j) << " $\\pm$ " << fixed << setprecision(1) << finalUncertainty.at(j) << " \\" << "\\" << " \\hline" << endl;
            } else {
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
