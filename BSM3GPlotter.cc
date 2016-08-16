////////////////////////////////////////////////////////////////////////////// 
// Author:	Alfredo Gurrola                                             //
// contact:     Alfredo.Gurrola@cern.ch       (Vanderbilt University)       // 
//////////////////////////////////////////////////////////////////////////////

#include "BSM3GPlotter.h"

//---main function
int main (int argc, char *argv[]) {

  BSM3GPlotter BSM3GPlotter_(argv[1]);

}

BSM3GPlotter::BSM3GPlotter(char* fname) {

  //---obtain the user defined inputs
  std::cout << "" << std::endl;
  std::cout << "BSM3GPlotter Message: Grabbing user defined input parameters (i.e. root files, directories, cross-sections, etc.)" << std::endl;
  std::cout << "" << std::endl;
  beginJob();
  getInputs(fname);
  std::cout << "" << std::endl;
  std::cout << "BSM3GPlotter Message: Finished grabbing configurable inputs. Calculating cut efficiencies and storing histograms." << std::endl;
  std::cout << "" << std::endl;
  if(inRootFiles.size() > 0) {
    grabYieldsANDgrabHistos();
    calculateEfficienciesAndErrors();
    std::cout << "" << std::endl;
    std::cout << "BSM3GPlotter Message: Finished calculating cut efficiencies and storing histograms. Performing normalization of histograms to cross-section times lumi." << std::endl;
    std::cout << "" << std::endl;
    NormalizeHistos();
    std::cout << "" << std::endl;
    std::cout << "BSM3GPlotter Message: Finished normalizing histograms to cross-section times lumi. Performing normalization of histograms to 1." << std::endl;
    std::cout << "" << std::endl;
    CreateProbHistos();
  }
  std::cout << "" << std::endl;
  std::cout << "BSM3GPlotter Message: Finished performing proper normalization of histograms. Creating cut flow eff table (log file)" << std::endl;
  std::cout << "" << std::endl;
  createLogFile();
  std::cout << "" << std::endl;
  std::cout << "BSM3GPlotter Message: Finished creating cut flow eff table. Closing opened root files. " << std::endl;
  std::cout << "" << std::endl;
  endJob();
  std::cout << "" << std::endl;
  std::cout << "BSM3GPlotter Message: Finished closing root files." << std::endl;
  std::cout << "" << std::endl;

}

//---function used to obtain the configurable inputs from the .in files
void BSM3GPlotter::getInputs(char* fname) {

  string inputString;
  string inputType;

  // open input file containing all information: root files, scale factors, cross-sections, etc
  ifstream inFile;
  inFile.open(fname, ios::in);
  // if can't open input file, exit the code
  if (!inFile) {
    std::cerr << "     BSM3GPlotter 'getInputs' function: Error!! --> Can't open the input configuration file " << fname << std::endl;
    exit(1);
  } else {
    std::cout << "     BSM3GPlotter 'getInputs' function: input file '" << fname << "' has been successfully opened." << std::endl;
  }

  // grab all relevant information from the input file
  while (inFile >> inputType >> inputString) {
    if(inputType == "rootfile") {
      inRootFiles.push_back(inputString); // create a vector of root files containing histograms
    } else if(inputType == "directory") {
      inDirectories.push_back(inputString); // directory path within root file that contains the histograms
    } else if(inputType == "process") {
      inProcess.push_back(inputString); // name of the cut ... will be used to rename histograms
    } else if(inputType == "outputPostscriptFile") {
      string psFile1 = inputString; // name of output postscript file
    } else if(inputType == "outputRootFileForNormalizedHistos") {
      outputRootFileForNormalizedHistos = inputString; // name of output root file containing normalized histograms
    } else if(inputType == "outputRootFileForProbabilityHistos") {
      outputRootFileForProbabilityHistos = inputString; // name of output root file containing probability histograms
    } else if(inputType == "outputLogFile") {
      outputLogFile = inputString; // name of output log file (e.g. contains efficiency table)
    } else if(inputType == "scaleFactor") {
      inScaleFactor.push_back(atof(inputString.c_str())); // scale factors used to recalculate efficiencies for a each cut
    } else if(inputType == "scaleFactorError") {
      inScaleFactorError.push_back(atof(inputString.c_str())); // scale factor error used to recalculate efficiency for each cut
    } else if(inputType == "luminosity") {
      lumi = atof(inputString.c_str()); // luminosity
      lumi_string = inputString;
    } else if(inputType == "effectiveXsection") {
      x_section.push_back(atof(inputString.c_str())); // cross-section
    } else if(inputType == "skimmingEff") {
      effSkim = atof(inputString.c_str()); // skimming efficiency
    } else if(inputType == "IsData") {
      IsData = inputString;
    } else {
      std::cerr << "     BSM3GPlotter 'getInputs' function: Error!! --> Incorrect input type " << inputType << std::endl; // exit code if unwanted input is specified
      exit(1);
    }
  }

  //---close the .in file
  inFile.close();

  std::cout << "     BSM3GPlotter 'getInputs' function: input file '" << fname << "' has been closed." << std::endl;

}

//---function called once just before obtaining user inputs. It clears the vectors and initializes parameters.
void BSM3GPlotter::beginJob() {

  inRootFiles.clear();
  inDirectories.clear();
  inProcess.clear();
  inScaleFactor.clear();
  inScaleFactorError.clear();
  x_section.clear();
  theCumulativeEfficiency.clear();
  RelativeEffVector.clear();
  RelativeEffErrorVector.clear();
  TotalEffVector.clear();
  TotalEffErrorVector.clear();
  theEventsAnalyzed.clear();
  theEventsPassing.clear();
  theHistNameStrings.clear();
  HistList.clear();
  HistList2.clear();
  nHistList=0;
  nHistList2=0;
  nHistos=0;
  outputLogFile = "temp.log";
  outputRootFileForNormalizedHistos = "norm.root";
  outputRootFileForProbabilityHistos = "prob.root";
  IsData = "0";
  MaxEventsAnalyzed = 0;
  lumi = 0.0;
  lumi_string = "0.0";

  std::cout << "     BSM3GPlotter 'beginJob' function: Vectors have been cleared and variables initialized." << std::endl;

}

//---function called once just before obtaining user inputs. It clears the vectors.
void BSM3GPlotter::grabYieldsANDgrabHistos() {

  string histoName;
  string histoNameName;

  // loop over root files
  for(int j=0;j<inRootFiles.size();j++) {
    int num=0; // keep track of total number of histograms per root file (later we will require each root file to contain the same histograms)

//    TFile *theCurrentFile = (TFile*) TFile::Open (inRootFiles.at(j).c_str()); // open root file
    theCurrentFile = (TFile*) TFile::Open (inRootFiles.at(j).c_str()); // open root file
    if (!theCurrentFile) {
      std::cerr << "     BSM3GPlotter 'grabYieldsANDgrabHistos' function: Error!! --> Can't open input file " << inRootFiles.at(j).c_str() << " (or file does not exist)" << std::endl;
      exit(1);
    } else {
      std::cout << "     BSM3GPlotter 'grabYieldsANDgrabHistos' function: input file '" << inRootFiles.at(j).c_str() << "' has been successfully opened." << std::endl;
    }
    theCurrentFile->cd(inDirectories.at(j).c_str()); // cd to appropriate directory
    TDirectory *current_sourcedir = gDirectory;
    TIter nextkey( current_sourcedir->GetListOfKeys() );
    TKey *key;

    std::cout << "     BSM3GPlotter 'grabYieldsANDgrabHistos' function: Looping over list of histograms." << std::endl;
    // loop over keys(ROOT objects) within the root file
    while((key = (TKey*)nextkey())) {
      TObject *obj = key->ReadObj();

      // only consider 1D histograms within the root file
      if ( obj->IsA()->InheritsFrom( "TH1" ) ) {
        string histname = obj->GetName();
        TH1 *hobj = (TH1*)obj;

        // use the "Events" histogram to calculate cumulative efficiencies
        if( (hobj->GetYaxis()->GetNbins() == 1) && (histname == "Events") ) {

          // "Events" histogram contains 2 filled bins: bin 1 is the # of events analyzed; bin 2 is the # passing specified cuts
          if(hobj->GetBinContent(1) == 0) {
            std::cerr << "     BSM3GPlotter 'grabYieldsANDgrabHistos' function: Error!! --> 'Events' histogram contains zero entries in bin 1: 0 events were analyzed ..." << std::endl;
            exit(1);
          } else {
            /* calculate cumulative efficiency (at this stage in the code, the calculation is not complete because
               it does not incorporate skimming efficiencies ... skimming eff is implemented later. */
            std::cout << "     BSM3GPlotter 'grabYieldsANDgrabHistos' function: Calculating efficiency for cut #" << j << std::endl;
            if(IsData == "1") {
              theEventsAnalyzed.push_back((double)hobj->GetBinContent(1)); // denominator for calculation of cumulative eff.
              theEventsPassing.push_back((double)hobj->GetBinContent(2)); // numerator for calculation of cumulative eff.
              theCumulativeEfficiency.push_back(((double)hobj->GetBinContent(2)) / ((double)hobj->GetBinContent(1))); // cumulative eff.
            } else {
              theEventsAnalyzed.push_back((double)hobj->GetBinContent(1)); // denominator for calculation of cumulative eff.
              theEventsPassing.push_back((double)hobj->GetBinContent(2)); // numerator for calculation of cumulative eff.
              theCumulativeEfficiency.push_back(((double)hobj->GetBinContent(2)) / ((double)hobj->GetBinContent(1))); // cumulative eff.
            }
          }

        }

        // grab all 1D histograms within the root file and create a vector containing them to be used later
        if( (hobj->GetYaxis()->GetNbins() == 1) && (histname != "Events") ) {

          // rename histogram
          if(j < (inRootFiles.size() - 1)) {histoName = histname + "_" + "After" + inProcess.at(j) + "Before" + inProcess.at(j+1);}
          else {histoName = histname + "_" + "After" + inProcess.at(j);}
          histoNameName = histoName + "_prob";
          //std::cout << "histoName = " << histoName << std::endl;
          //std::cout << "histoNameName = " << histoNameName << std::endl;

          TH1F *h1 = new TH1F(histoName.c_str(),histoName.c_str(),hobj->GetXaxis()->GetNbins(),hobj->GetXaxis()->GetXmin(),hobj->GetXaxis()->GetXmax());
          for(int i=0; i<=(h1->GetXaxis()->GetNbins() + 1); i++) {h1->SetBinContent(i,hobj->GetBinContent(i));}
          TH1F *h2 = new TH1F(histoNameName.c_str(),histoNameName.c_str(),hobj->GetXaxis()->GetNbins(),hobj->GetXaxis()->GetXmin(),hobj->GetXaxis()->GetXmax());
          for(int i=0; i<=(h2->GetXaxis()->GetNbins() + 1); i++) {h2->SetBinContent(i,hobj->GetBinContent(i));}
          //std::cout << "h1 = " << h1->GetName() << std::endl;
          //std::cout << "h2 = " << h2->GetName() << std::endl;

          HistList.push_back(h1);
          //std::cout << "name of histogram = " << (HistList.at(nHistList))->GetName() << std::endl;
          nHistList++;

          HistList2.push_back(h2);
          //std::cout << "name of histogram2 = " << (HistList2.at(nHistList2))->GetName() << std::endl;
          nHistList2++;

          theHistNameStrings.push_back(histname); // store default names - used later in the code
          num++;
        }
      }
    }
    // make sure that each root file contains the same histograms
    if(j==0) {
      nHistos=num;
    } else {
      if(num != nHistos) { 
        std::cerr << "     BSM3GPlotter 'grabYieldsANDgrabHistos' function: Error!! --> Input Root Files DO NOT have the same histograms!!" << std::endl;
        exit(1);
      }
    }
//    theCurrentFile->Close();
  }

  /* it's possible that not all root files were created by running over the same number of events. Therefore, the code below takes
     care of this by renomarlizing everything - renormalized events = max_events_analyzed * cut_efficiency. To do this, the maximum
     number of events analyzed must be determined. */
  MaxEventsAnalyzed = 0;
  for(int i=0; i<theEventsAnalyzed.size(); i++) {
    if(theEventsAnalyzed.at(i) > MaxEventsAnalyzed) {MaxEventsAnalyzed = theEventsAnalyzed.at(i);}
  }

  //std::cout << "here" << std::endl;
  //std::cout << "name of histogram 0th = " << (HistList.at(0))->GetName() << std::endl;

}

//---function called once just before obtaining user inputs. It clears the vectors.
void BSM3GPlotter::calculateEfficienciesAndErrors() {

  float numerator = 1.0;
  float denominator = 1.0;
  float theRelativeEfficiency = 1.0;
  float efferror = 0.0;
  float cumulativeEfficiency = 1.0;
  float efferror2 = 0.0;

  // calculate relative and cumulative cut efficiencies
  for (int i=0;i<theCumulativeEfficiency.size(); i++) {
    numerator = (int)((MaxEventsAnalyzed * theCumulativeEfficiency.at(i))+0.5);
    if(i==0) {denominator = (int)(MaxEventsAnalyzed + 0.5);}
    else {denominator = (int)((MaxEventsAnalyzed * theCumulativeEfficiency.at(i-1))+0.5);}
    if((numerator > 0) && (denominator > 0)) {
      theRelativeEfficiency = numerator / denominator;
      efferror = sqrt(theRelativeEfficiency*(1.-theRelativeEfficiency)/denominator);
    } else if((numerator == 0) && (denominator > 0)) {
      theRelativeEfficiency = 0.0;
      efferror = 1.0 / denominator;
    } else if((numerator == 0) && (denominator == 0)) {
      theRelativeEfficiency = 0.0;
      efferror = 0.0;
    } else {
      theRelativeEfficiency = numerator / denominator;
      efferror = sqrt(theRelativeEfficiency*(1.-theRelativeEfficiency)/denominator);
    }
    /* binomial uncertainties do not work when the efficiency gets close to 1 or 0 (e.g. efficiency cannot
       be 99 +- 2 because the efficiency cannot be e.g. 101) ... in these cases, use bayesian */
    if(((theRelativeEfficiency + efferror) > 1.) || ((theRelativeEfficiency - efferror) < 0.)){
      TH1F* theNumHisto = new TH1F("theNumHisto","theNumHisto",1,0,1);
      theNumHisto->SetBinContent(1,numerator);
      theNumHisto->Sumw2();
      TH1F* theDenHisto = new TH1F("theDenHisto","",1,0,1);
      theDenHisto->SetBinContent(1,denominator);
      theDenHisto->Sumw2();
      TGraphAsymmErrors* bayesEff = new TGraphAsymmErrors();
      bayesEff->BayesDivide(theNumHisto,theDenHisto,"b");
      if(bayesEff->GetErrorYhigh(0) > bayesEff->GetErrorYlow(0)) {efferror = bayesEff->GetErrorYhigh(0);}
      else {efferror = bayesEff->GetErrorYlow(0);}
      delete theNumHisto;
      delete theDenHisto;
      delete bayesEff;
    }
    if(theRelativeEfficiency == 1.0) {efferror = 0;}
    // recalculate efficiencies and errors incorporating scale factors
    if((numerator > 0) && (denominator > 0)) {
      efferror = sqrt(pow(efferror/theRelativeEfficiency,2.0) + pow((inScaleFactorError.at(i)/inScaleFactor.at(i)),2.0));
      theRelativeEfficiency = theRelativeEfficiency * inScaleFactor.at(i);
      efferror = efferror * theRelativeEfficiency;
    } else if((numerator == 0) && (denominator > 0)) {
      efferror = inScaleFactor.at(i) / denominator;
      theRelativeEfficiency = theRelativeEfficiency * inScaleFactor.at(i);
    } else if((numerator == 0) && (denominator == 0)) {
      efferror = 0.0;
      theRelativeEfficiency = theRelativeEfficiency * inScaleFactor.at(i);
    } else {
      efferror = sqrt(pow(efferror/theRelativeEfficiency,2.0) + pow((inScaleFactorError.at(i)/inScaleFactor.at(i)),2.0));
      theRelativeEfficiency = theRelativeEfficiency * inScaleFactor.at(i);
      efferror = efferror * theRelativeEfficiency;
    }
    numerator = (int)((MaxEventsAnalyzed * theCumulativeEfficiency.at(i))+0.5);
    denominator = (int)(MaxEventsAnalyzed + 0.5);
    if((numerator > 0) && (denominator > 0)) {
      cumulativeEfficiency = numerator / denominator;
      efferror2 = sqrt(cumulativeEfficiency*(1.-cumulativeEfficiency)/denominator);
    } else if((numerator == 0) && (denominator > 0)) {
      cumulativeEfficiency = 0.0;
      efferror2 = 1.0 / denominator;
    } else if((numerator == 0) && (denominator == 0)) {
      cumulativeEfficiency = 0.0;
      efferror2 = 0.0;
    } else {
      cumulativeEfficiency = numerator / denominator;
      efferror2 = sqrt(cumulativeEfficiency*(1.-cumulativeEfficiency)/denominator);
    }
    /* binomial uncertainties do not work when the efficiency gets close to 1 or 0 (e.g. efficiency cannot
       be 99 +- 2 because the efficiency cannot be e.g. 101) ... in these cases, use bayesian */
    if(((cumulativeEfficiency + efferror2) > 1.) || ((cumulativeEfficiency - efferror2) < 0.)){
      TH1F* theNumHisto = new TH1F("theNumHisto","theNumHisto",1,0,1);
      theNumHisto->SetBinContent(1,numerator);
      theNumHisto->Sumw2();
      TH1F* theDenHisto = new TH1F("theDenHisto","",1,0,1);
      theDenHisto->SetBinContent(1,denominator);
      theDenHisto->Sumw2();
      TGraphAsymmErrors* bayesEff = new TGraphAsymmErrors();
      bayesEff->BayesDivide(theNumHisto,theDenHisto,"b");
      if(bayesEff->GetErrorYhigh(0) > bayesEff->GetErrorYlow(0)) {efferror2 = bayesEff->GetErrorYhigh(0);}
      else {efferror2 = bayesEff->GetErrorYlow(0);}
      delete theNumHisto;
      delete theDenHisto;
      delete bayesEff;
    }
    if(cumulativeEfficiency == 1.0) {efferror2 = 0;}
    // recalculate efficiencies and errors incorporating scale factors
    if((numerator > 0) && (denominator > 0)) {
      for(int numberOfSFs = 0; numberOfSFs <= i; numberOfSFs++) {
        efferror2 = sqrt(pow(efferror2/cumulativeEfficiency,2.0) + pow((inScaleFactorError.at(numberOfSFs)/inScaleFactor.at(numberOfSFs)),2.0));
        cumulativeEfficiency = cumulativeEfficiency * inScaleFactor.at(numberOfSFs);
        efferror2 = efferror2 * cumulativeEfficiency;
      }
    } else if((numerator == 0) && (denominator > 0)) {
      efferror2 = 1.0;
      for(int numberOfSFs = 0; numberOfSFs <= i; numberOfSFs++) {
        efferror2 = efferror2 * inScaleFactor.at(numberOfSFs);
        cumulativeEfficiency = cumulativeEfficiency * inScaleFactor.at(numberOfSFs);
      }
      efferror2 = efferror2 / denominator;
    } else if((numerator == 0) && (denominator == 0)) {
      efferror2 = 0.0;
      cumulativeEfficiency = 0.0;
    } else {
      for(int numberOfSFs = 0; numberOfSFs <= i; numberOfSFs++) {
        efferror2 = sqrt(pow(efferror2/cumulativeEfficiency,2.0) + pow((inScaleFactorError.at(numberOfSFs)/inScaleFactor.at(numberOfSFs)),2.0));
        cumulativeEfficiency = cumulativeEfficiency * inScaleFactor.at(numberOfSFs);
        efferror2 = efferror2 * cumulativeEfficiency;
      }
    }

    RelativeEffVector.push_back(theRelativeEfficiency);
    RelativeEffErrorVector.push_back(efferror);
    TotalEffVector.push_back(cumulativeEfficiency);
    TotalEffErrorVector.push_back(efferror2);

  }

}

//---function called once just before obtaining user inputs. It clears the vectors.
void BSM3GPlotter::NormalizeHistos() {

  //std::cout << "number of histograms = " << HistList.size() << std::endl;
  std::cout << "     BSM3GPlotter 'NormalizeHistos' function: Extracting histograms from the vectors for proper normalization." << std::endl;

  if(( nHistList % inRootFiles.size() == 0 )) {
    for(int i=0; i<nHistos; i++) {
      std::cout << "     BSM3GPlotter 'NormalizeHistos' function: Histogram with name '" << HistList.at(i)->GetName() << "' is being properly normalized." << std::endl;
      string histname = HistList.at(i)->GetName();
      string histoEffyName = "hEffy_" + inProcess.at(0);
      TH1F *h = (TH1F*)HistList.at(i);
      h->Sumw2();
      if(IsData == "0") {
        if(h->Integral(0,(h->GetXaxis()->GetNbins()+1)) > 0) {h->Scale(1.0 / theEventsPassing.at(0) * x_section.at(0) * lumi * effSkim);}
        TH1F *hEffy = new TH1F(histoEffyName.c_str(),histoEffyName.c_str(),h->GetXaxis()->GetNbins(),h->GetXaxis()->GetXmin(),h->GetXaxis()->GetXmax());
        for(int ibin=0; ibin<=(hEffy->GetXaxis()->GetNbins() + 1); ibin++) {
          hEffy->SetBinContent(ibin,RelativeEffVector.at(0));
          hEffy->SetBinError(ibin,0);
        }
        h->Multiply(h,hEffy,1,1,"");
        for(int ibin=0; ibin<=(h->GetXaxis()->GetNbins() + 1); ibin++) {
          h->SetBinError(ibin, sqrt(pow(h->GetBinError(ibin),2.0) + h->GetBinContent(ibin)) ); // propagating MC statistical uncertainty & poisson uncertainty on the r$
        }
        delete hEffy;
      }
      h->SetName(histname.c_str());
      string YtitleString = "N / " + lumi_string + " pb^{-1}";
      h->GetYaxis()->SetTitle(YtitleString.c_str());
      h->GetYaxis()->SetTitleSize(0.06);
      h->GetYaxis()->SetTitleFont(62);
      h->GetYaxis()->CenterTitle();
      h->GetYaxis()->SetLabelSize(0.05);
      h->GetYaxis()->SetLabelFont(62);
      h->GetXaxis()->SetTitle(theHistNameStrings.at(i).c_str());
      h->GetXaxis()->SetTitleSize(0.06);
      h->GetXaxis()->SetTitleFont(62);
      h->GetXaxis()->CenterTitle();
      h->GetXaxis()->SetLabelSize(0.05);
      h->GetXaxis()->SetLabelFont(62);
      TFile *hfile = (TFile*)gROOT->FindObject(outputRootFileForNormalizedHistos.c_str());
      if (hfile) {hfile->Close();}
      hfile = new TFile(outputRootFileForNormalizedHistos.c_str(),"UPDATE");
      h->Write();
      hfile->Close();
      delete h;
      if(inRootFiles.size() > 1) {
        for(int j=1;j<inRootFiles.size();j++) {
          string hist2name = HistList.at(i+(j*nHistos))->GetName();
          if(theHistNameStrings.at(i) == theHistNameStrings.at(i+(j*nHistos))) {
            string histoEffyName2 = "hhEffy_" + inProcess.at(j);
            TH1F *hh = (TH1F*)HistList.at(i+(j*nHistos));
            hh->Sumw2();
            if(IsData == "0") {
              if(hh->Integral(0,(hh->GetXaxis()->GetNbins()+1)) > 0) {hh->Scale(1.0 / theEventsPassing.at(j) * x_section.at(j) * lumi * effSkim);}
              for(int jfile=0;jfile<=j;jfile++) {
                TH1F *hhEffy = new TH1F(histoEffyName2.c_str(),histoEffyName2.c_str(),
                                  hh->GetXaxis()->GetNbins(),hh->GetXaxis()->GetXmin(),hh->GetXaxis()->GetXmax());
                for(int ibin=0; ibin<=(hhEffy->GetXaxis()->GetNbins() + 1); ibin++) {
                  hhEffy->SetBinContent(ibin,RelativeEffVector.at(jfile));
                  hhEffy->SetBinError(ibin,0);
                }
                hh->Multiply(hh,hhEffy,1,1,"");
                delete hhEffy;
              }
              for(int ibin=0; ibin<=(hh->GetXaxis()->GetNbins() + 1); ibin++) {
                hh->SetBinError(ibin, sqrt(pow(hh->GetBinError(ibin),2.0) + hh->GetBinContent(ibin)) );
              }
            }
            hh->SetName(hist2name.c_str());
            string YtitleString = "N / " + lumi_string + " pb^{-1}";
            hh->GetYaxis()->SetTitle(YtitleString.c_str());
            hh->GetYaxis()->SetTitleSize(0.06);
            hh->GetYaxis()->SetTitleFont(62);
            hh->GetYaxis()->CenterTitle();
            hh->GetYaxis()->SetLabelSize(0.05);
            hh->GetYaxis()->SetLabelFont(62);
            hh->GetXaxis()->SetTitle(theHistNameStrings.at(i+(j*nHistos)).c_str());
            hh->GetXaxis()->SetTitleSize(0.06);
            hh->GetXaxis()->SetTitleFont(62);
            hh->GetXaxis()->CenterTitle();
            hh->GetXaxis()->SetLabelSize(0.05);
            hh->GetXaxis()->SetLabelFont(62);
            TFile *hfile = (TFile*)gROOT->FindObject(outputRootFileForNormalizedHistos.c_str());
            if (hfile) {hfile->Close();}
            hfile = new TFile(outputRootFileForNormalizedHistos.c_str(),"UPDATE");
            hh->Write();
            hfile->Close();
            delete hh;
          }
        }
      }
    }
  }

}

//---function called once just before obtaining user inputs. It clears the vectors.
void BSM3GPlotter::CreateProbHistos() {

  std::cout << "     BSM3GPlotter 'CreateProbHistos' function: Extracting histograms from the vectors in order to normalize to 1." << std::endl;

  if(( nHistList2 % inRootFiles.size() == 0 )) {
    for(int i=0; i<nHistos; i++) {
      std::cout << "     BSM3GPlotter 'CreateProbHistos' function: Histogram with name '" << HistList2.at(i)->GetName() << "' is being normalized to 1." << std::endl;
      string histname = HistList2.at(i)->GetName();
      TH1F *h = (TH1F*)HistList2.at(i);
      h->Sumw2();
      if(h->Integral(0,(h->GetXaxis()->GetNbins()+1)) > 0) {h->Scale(1.0 / h->Integral(0,(h->GetXaxis()->GetNbins()+1)));}
      h->SetName(histname.c_str());
      h->GetYaxis()->SetTitle("a.u.");
      h->GetYaxis()->SetTitleSize(0.06);
      h->GetYaxis()->SetTitleFont(62);
      h->GetYaxis()->CenterTitle();
      h->GetYaxis()->SetLabelSize(0.05);
      h->GetYaxis()->SetLabelFont(62);
      h->GetXaxis()->SetTitle(theHistNameStrings.at(i).c_str());
      h->GetXaxis()->SetTitleSize(0.06);
      h->GetXaxis()->SetTitleFont(62);
      h->GetXaxis()->CenterTitle();
      h->GetXaxis()->SetLabelSize(0.05);
      h->GetXaxis()->SetLabelFont(62);
      TFile *hfile = (TFile*)gROOT->FindObject(outputRootFileForProbabilityHistos.c_str());
      if (hfile) {hfile->Close();}
      hfile = new TFile(outputRootFileForProbabilityHistos.c_str(),"UPDATE");
      h->Write();
      hfile->Close();
      delete h;
      if(inRootFiles.size() > 1) {
        for(int j=1;j<inRootFiles.size();j++) {
          string hist2name = HistList2.at(i+(j*nHistos))->GetName();
          if(theHistNameStrings.at(i) == theHistNameStrings.at(i+(j*nHistos))) {
            TH1F *hh = (TH1F*)HistList2.at(i+(j*nHistos));
            hh->Sumw2();
            if(hh->Integral(0,(hh->GetXaxis()->GetNbins()+1)) > 0) {hh->Scale(1.0 / hh->Integral(0,(hh->GetXaxis()->GetNbins()+1)));}
            hh->SetName(hist2name.c_str());
            hh->GetYaxis()->SetTitle("a.u.");
            hh->GetYaxis()->SetTitleSize(0.06);
            hh->GetYaxis()->SetTitleFont(62);
            hh->GetYaxis()->CenterTitle();
            hh->GetYaxis()->SetLabelSize(0.05);
            hh->GetYaxis()->SetLabelFont(62);
            hh->GetXaxis()->SetTitle(theHistNameStrings.at(i+(j*nHistos)).c_str());
            hh->GetXaxis()->SetTitleSize(0.06);
            hh->GetXaxis()->SetTitleFont(62);
            hh->GetXaxis()->CenterTitle();
            hh->GetXaxis()->SetLabelSize(0.05);
            hh->GetXaxis()->SetLabelFont(62);
            TFile *hfile = (TFile*)gROOT->FindObject(outputRootFileForProbabilityHistos.c_str());
            if (hfile) {hfile->Close();}
            hfile = new TFile(outputRootFileForProbabilityHistos.c_str(),"UPDATE");
            hh->Write();
            hfile->Close();
            delete hh;
          }
        }
      }
    }
  }

}

//---function called once just before obtaining user inputs. It clears the vectors.
void BSM3GPlotter::createLogFile() {

  // create an output log file that will contain the cut flow efficiency table
  ofstream outFile;
  outFile.open(outputLogFile.c_str(), ios::out);
  // if output log file cannot be opened, exit the code
  if (!outFile) {
    cerr << "Can't open output file " << outputLogFile << endl;
    exit(1);
  } else {
    outFile << "" << endl;
    outFile << "The following input was used: " << endl;
    outFile << "" << endl;
    outFile << "" << endl;
  }

  // loop over root files
  for(int j=0;j<inRootFiles.size();j++) {
    // printout information to log file
    outFile << "Name of Root File #" << (j+1) << " : " << inRootFiles.at(j) << endl;
    outFile << "Name of Directory #" << (j+1) << " : " << inDirectories.at(j) << endl;
    outFile << "Name of Process #" << (j+1) << "   : " << inProcess.at(j) << endl;
    outFile << "" << endl;
  }

  //  outFile << "Detailed Efficiencies " << "\n";
  outFile << "Table of Efficiencies " << "\n\n";
  outFile << "Efficiencies below are calculated with respect to the number of events after skimming" << "\n\n";
  outFile << "-------------------------------------------------------------------------------------------------------------------------------\n";
  outFile << "         Name                         Events               Relative (%)                     Cumulative (%)\n";
  outFile << "-------------------------------------------------------------------------------------------------------------------------------\n";

  if(theCumulativeEfficiency.size() > 0) {
    float effy = 1;
    float deffyOvereffy = 0;
    for (int i=0;i<theCumulativeEfficiency.size(); i++) {
      effy = effy * RelativeEffVector.at(i);
      deffyOvereffy = sqrt(pow(RelativeEffErrorVector.at(i)/RelativeEffVector.at(i),2.0) + pow(deffyOvereffy,2.0));
      // output for cut flow efficiency table - efficiencies and errors
      outFile     <<setw(24)<<inProcess.at(i)
                  <<setw(20)<<(int)((MaxEventsAnalyzed * theCumulativeEfficiency.at(i))+0.5)
                  <<setw(15)<<setprecision(4)<<RelativeEffVector.at(i)*100.0<<setw(4)<<" +- "<<setw(10)<<setprecision(4)<<(RelativeEffErrorVector.at(i)*100.0)
                  <<setw(20)<<setprecision(4)<<TotalEffVector.at(i)*100.0<<setw(4)<<" +- "<<setw(10)<<setprecision(4)<<(TotalEffErrorVector.at(i) * 100.0)
                  <<endl;
    }
  }

  outFile << "-------------------------------------------------------------------------------------------------------------------------------\n";

  outFile.close();

}

//---function called once just before obtaining user inputs. It clears the vectors and initializes parameters.
void BSM3GPlotter::endJob() {

  inRootFiles.clear();
  inDirectories.clear();
  inProcess.clear();
  inScaleFactor.clear();
  inScaleFactorError.clear();
  x_section.clear();
  theCumulativeEfficiency.clear();
  RelativeEffVector.clear();
  RelativeEffErrorVector.clear();
  TotalEffVector.clear();
  TotalEffErrorVector.clear();
  theEventsAnalyzed.clear();
  theEventsPassing.clear();
  theHistNameStrings.clear();
  HistList.clear();
  HistList2.clear();
  theCurrentFile->Close();

}

BSM3GPlotter::~BSM3GPlotter() { };
