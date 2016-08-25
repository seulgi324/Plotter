////////////////////////////////////////////////////////////////////////////// 
// Author:	Alfredo Gurrola                                             //
// contact:     Alfredo.Gurrola@cern.ch       (Vanderbilt University)       // 
//////////////////////////////////////////////////////////////////////////////

#include "BSM3GPlotMaker.h"

using namespace std;

//---main function
int main (int argc, char *argv[]) {

  BSM3GPlotMaker BSM3GPlotMaker_(argv[1]);

}

BSM3GPlotMaker::BSM3GPlotMaker(char* fname) {

  //---obtain the user defined inputs
  std::cout << endl << "BSM3GPlotMaker Message: Grabbing user defined input parameters (i.e. root files, directories, cross-sections, etc.)" << std::endl << endl;

  beginJob();
  getInputs(fname);

  std::cout << endl << "BSM3GPlotMaker Message: Finished grabbing configurable inputs." << std::endl << endl;

  if(inRootFiles.size() > 0) {
    grabYieldsANDgrabHistos();
    makePlots();
  }

  std::cout << endl << "BSM3GPlotMaker Message: Finished making plots. Closing opened root files." << std::endl << endl;

  endJob();

  std::cout << endl <<  "BSM3GPlotMaker Message: Finished closing root files." << std::endl << endl;
}

//---function used to obtain the configurable inputs from the .in files
void BSM3GPlotMaker::getInputs(char* fname) {

  string inputString;
  string inputType;

  // open input file containing all information: root files, scale factors, cross-sections, etc
  ifstream inFile;
  inFile.open(fname, ios::in);
  // if can't open input file, exit the code
  if (!inFile) {
    std::cerr << "     BSM3GPlotMaker 'getInputs' function: Error!! --> Can't open the input configuration file " << fname << std::endl;
    exit(1);
  } else {
    std::cout << "     BSM3GPlotMaker 'getInputs' function: input file '" << fname << "' has been successfully opened." << std::endl;
  }

  // grab all relevant information from the input file
  while (inFile >> inputType >> inputString) {
    if(inputType == "rootfile") {
      if(inputString != "BLANK") {inRootFiles.push_back(inputString);}
    } else if(inputType == "process") {
      if(inputString != "BLANK") {inProcess.push_back(inputString);}
    } else if(inputType == "scaleFactor") {
      if(inputString != "BLANK") {inScaleFactor.push_back(inputString);}
      else {inScaleFactor.push_back("1");}
    } else if(inputType == "scaleFactorError") {
      if(inputString != "BLANK") {inScaleFactorError.push_back(inputString);}
      else {inScaleFactorError.push_back("0");}
    } else if(inputType == "AddLegendTitle") {
      if(inputString != "BLANK") {inLegend.push_back(inputString);}
    } else if(inputType == "outputRootFile") {
      if(inputString != "BLANK") {outputRootFile = inputString;}
    } else if(inputType == "outputLogFile") {
      if(inputString != "BLANK") {outputLogFile = inputString;}
    } else if(inputType == "FillColor") {
      if(inputString != "BLANK") {inColors.push_back(inputString);}
    } else if(inputType == "FillStyle") {
      if(inputString != "BLANK") {inFillStyles.push_back(inputString);}
    } else if(inputType == "luminosity") {
      lumi = atof(inputString.c_str()); // luminosity
      lumi_string = inputString;
    } else if(inputType == "Rebin") {
      if(inputString != "BLANK") {rebinNumber.push_back(inputString);}
      else {rebinNumber.push_back("1");}
    } else {
      std::cerr << "     BSM3GPlotMaker 'getInputs' function: Error!! --> Incorrect input type " << inputType << std::endl; // exit code if unwanted input is specified
      exit(1);
    }
  }

  //---close the .in file
  inFile.close();

  std::cout << "     BSM3GPlotMaker 'getInputs' function: input file '" << fname << "' has been closed." << std::endl;

}

//---function called once just before obtaining user inputs. It clears the vectors and initializes parameters.
void BSM3GPlotMaker::beginJob() {

  theHistNameStrings.clear();
  inLegend.clear();
  inFillStyles.clear();
  inColors.clear();
  inProcess.clear();
  inRootFiles.clear();
  rebinNumber.clear();
  inScaleFactor.clear();
  inScaleFactorError.clear();
  lumi = 0.0;
  lumi_string = "0.0";
  HistList.clear();
  HistList2.clear();
  nHistList=0;
  nHistList2=0;
  nHistos=0;
  outputLogFile = "temp.log";
  outputRootFile = "plots.root";

  std::cout << "     BSM3GPlotMaker 'beginJob' function: Vectors have been cleared and variables initialized." << std::endl;

}

//---function called once just before obtaining user inputs. It clears the vectors.
void BSM3GPlotMaker::grabYieldsANDgrabHistos() {

  string histoName1;
  string histoName2;

  // loop over root files
  for(int j=0;j<inRootFiles.size();j++) {
    int num=0; // keep track of total number of histograms per root file (later we will require each root file to contain the same histograms)

    theCurrentFile = (TFile*) TFile::Open (inRootFiles.at(j).c_str()); // open root file
    if (!theCurrentFile) {
      std::cerr << "     BSM3GPlotMaker 'grabYieldsANDgrabHistos' function: Error!! --> Can't open input file " << inRootFiles.at(j).c_str() << " (or file does not exist)" << std::endl;
      exit(1);
    } else {
      std::cout << "     BSM3GPlotMaker 'grabYieldsANDgrabHistos' function: input file '" << inRootFiles.at(j).c_str() << "' has been successfully opened." << std::endl;
    }
    TDirectory *current_sourcedir = gDirectory;
    TIter nextkey( current_sourcedir->GetListOfKeys() );
    TKey *key;

    std::cout << "     BSM3GPlotMaker 'grabYieldsANDgrabHistos' function: Looping over list of histograms." << std::endl;
    // loop over keys(ROOT objects) within the root file
    while((key = (TKey*)nextkey())) {
      TObject *obj = key->ReadObj();

      // only consider 1D histograms within the root file
      if ( obj->IsA()->InheritsFrom( "TH1" ) ) {
        string histname = obj->GetName();
        TH1 *hobj = (TH1*)obj;

        // grab all 1D histograms within the root file and create a vector containing them to be used later
        if( (hobj->GetYaxis()->GetNbins() == 1) && (histname != "Events") ) {

          // rename histogram
          if(j < (inRootFiles.size() - 1)) {
            string histoName1 = histname + "_" + "After" + inProcess.at(j) + "Before" + inProcess.at(j+1) + "_1";
            string histoName2 = histname + "_" + "After" + inProcess.at(j) + "Before" + inProcess.at(j+1) + "_2";
          }
          else {
            string histoName1 = histname + "_" + "After" + inProcess.at(j) + "_1";
            string histoName2 = histname + "_" + "After" + inProcess.at(j) + "_2";
          }

          TH1F *h1 = new TH1F(histoName1.c_str(),histoName1.c_str(),hobj->GetXaxis()->GetNbins(),hobj->GetXaxis()->GetXmin(),hobj->GetXaxis()->GetXmax());
          TH1F *h2 = new TH1F(histoName2.c_str(),histoName2.c_str(),hobj->GetXaxis()->GetNbins(),hobj->GetXaxis()->GetXmin(),hobj->GetXaxis()->GetXmax());

          for(int i=0; i<=(h1->GetXaxis()->GetNbins() + 1); i++) {
            h1->SetBinContent(i,hobj->GetBinContent(i));
            h2->SetBinContent(i,hobj->GetBinContent(i));
            if(j>0) {h2->SetBinError(i,hobj->GetBinError(i));}
            else {h1->SetBinError(i,sqrt(hobj->GetBinContent(i)));}
          }
          h1->Rebin((int)atof(rebinNumber.at(0).c_str()));
          h2->Rebin((int)atof(rebinNumber.at(0).c_str()));

          TH1F *h4 = new TH1F("SFwithError","SFwithError",hobj->GetXaxis()->GetNbins(),hobj->GetXaxis()->GetXmin(),hobj->GetXaxis()->GetXmax());
          h4->Rebin((int)atof(rebinNumber.at(0).c_str()));
          if(inScaleFactor.size() > 0) {
            for(int i=1; i<=(h4->GetXaxis()->GetNbins()); i++) {
              h4->SetBinContent(i,(double)atof(inScaleFactor.at(j).c_str()));
              h4->SetBinError(i,(double)atof(inScaleFactorError.at(j).c_str()));
            }
          } else {
            for(int i=1; i<=(h4->GetXaxis()->GetNbins()); i++) {
              h4->SetBinContent(i,1.0);
              h4->SetBinError(i,0.0);
            }
          }
          if(j>0) {h2->Multiply(h4);h1->Scale(h4->GetBinContent(1));}
          else {h1->Multiply(h4);h2->Scale(h4->GetBinContent(1));}

          HistList.push_back(h1);
          //std::cout << "name of histogram = " << (HistList.at(nHistList))->GetName() << std::endl;
          nHistList++;

          HistList2.push_back(h2);
          //std::cout << "name of histogram2 = " << (HistList2.at(nHistList2))->GetName() << std::endl;
          nHistList2++;

          theHistNameStrings.push_back(histname); // store default names - used later in the code
          num++;

          delete h4;

        }
      }
    }
    // make sure that each root file contains the same histograms
    if(j==0) {
      nHistos=num;
    } else {
      if(num != nHistos) { 
        std::cerr << "     BSM3GPlotMaker 'grabYieldsANDgrabHistos' function: Error!! --> Input Root Files DO NOT have the same histograms!!" << std::endl;
        exit(1);
      }
    }
  }

}

//---function called once just before obtaining user inputs. It clears the vectors.
void BSM3GPlotMaker::makePlots() {

  //std::cout << "number of histograms = " << HistList.size() << std::endl;
  std::cout << "     BSM3GPlotMaker 'makePlots' function: Extracting histograms from the vectors for the creation of plots." << std::endl;

  if(( nHistList % inRootFiles.size() == 0 )) {
    for(int i=0; i<nHistos; i++) {

      TCanvas *c = new TCanvas(theHistNameStrings.at(i).c_str(), theHistNameStrings.at(i).c_str(), 403,50,600,600);
      gStyle->SetOptStat(0);
      gStyle->SetOptTitle(0);
      c->Range(0,0,1,1);
      c->SetFillColor(0);
      c->SetBorderMode(0);
      c->SetBorderSize(3);
      c->SetTickx(1);
      c->SetTicky(1);
      c->SetLeftMargin(0.15);
      c->SetRightMargin(0.05);
      c->SetTopMargin(0.05);
      c->SetBottomMargin(0.15);
      c->SetFrameFillStyle(0);
      c->SetFrameBorderMode(0);

      TPad *c1_1 = new TPad("c1_1", "newpad",0.01,0,0.99,0.32);
      c1_1->Draw();
      c1_1->cd();
      c1_1->Range(-200,-2.720435,1133.333,3.047681);
      c1_1->SetFillColor(0);
      c1_1->SetFillStyle(4000);
      c1_1->SetBorderMode(0);
      c1_1->SetBorderSize(2);
      c1_1->SetTickx(1);
      c1_1->SetTicky(1);
      c1_1->SetLeftMargin(0.15);
      c1_1->SetTopMargin(0.01);
      c1_1->SetBottomMargin(0.3);
      c1_1->SetFrameFillStyle(0);
      c1_1->SetFrameBorderMode(0);
      c1_1->SetFrameFillStyle(0);
      c1_1->SetFrameBorderMode(0);

      THStack *hs = new THStack(theHistNameStrings.at(i).c_str(),theHistNameStrings.at(i).c_str());
      string histname = HistList.at(i)->GetName();
      TH1F *h = (TH1F*)HistList.at(i);
      h->SetName(histname.c_str());
//      if(i > 0) {
//        delete hhh;
//        delete hError;
//        delete hData;
//        delete hMC;
//      }
      TH1F *hhh = new TH1F("hhh","hhh",h->GetXaxis()->GetNbins(),h->GetXaxis()->GetXmin(),h->GetXaxis()->GetXmax());
      TH1F *hError = new TH1F("hError","hError",h->GetXaxis()->GetNbins(),h->GetXaxis()->GetXmin(),h->GetXaxis()->GetXmax());
      TH1F *hData = new TH1F("hData","hData",h->GetXaxis()->GetNbins(),h->GetXaxis()->GetXmin(),h->GetXaxis()->GetXmax());
      TH1F *hMC = new TH1F("hMC","hMC",h->GetXaxis()->GetNbins(),h->GetXaxis()->GetXmin(),h->GetXaxis()->GetXmax());
      Double_t mcX[5000];
      Double_t mcY[5000];
      Double_t mcErrorX[5000];
      Double_t mcErrorY[5000];
      Double_t mcX2[5000];
      Double_t mcY2[5000];
      Double_t mcErrorX2[5000];
      Double_t mcErrorY2[5000];
      for(int ijkk=0; ijkk<=(hhh->GetXaxis()->GetNbins() + 1); ijkk++) {hhh->SetBinContent(ijkk,h->GetBinContent(ijkk));hhh->SetBinError(ijkk,h->GetBinError(ijkk));}
      for(int ijkk=0; ijkk<=(hData->GetXaxis()->GetNbins() + 1); ijkk++) {hData->SetBinContent(ijkk,h->GetBinContent(ijkk));hData->SetBinError(ijkk,h->GetBinError(ijkk));}
      for(int ijkk=0; ijkk<=(hError->GetXaxis()->GetNbins() + 1); ijkk++) {hError->SetBinContent(ijkk,0);}
      for(int ijkk=0; ijkk<=(hMC->GetXaxis()->GetNbins() + 1); ijkk++) {hMC->SetBinContent(ijkk,0);}
//      h->Sumw2();
//      hhh->Sumw2();
//      hData->Sumw2();
      hError->Sumw2();
      hMC->Sumw2();
      TLegend *legend = new TLegend(0.6082412,0.4248252,0.8208543,0.7867133,NULL,"brNDC");
      legend->AddEntry(h,inProcess.at(0).c_str(), "lep");
      if(inRootFiles.size() > 1) {
        for(int j=1;j<inRootFiles.size();j++) {
          string hist2name = HistList.at(i+(j*nHistos))->GetName();
          if(theHistNameStrings.at(i) == theHistNameStrings.at(i+(j*nHistos))) {
            TH1F *hh = (TH1F*)HistList.at(i+(j*nHistos));
            TH1F *hh2 = (TH1F*)HistList2.at(i+(j*nHistos));
            TH1F *hh3 = (TH1F*)HistList2.at(i+(j*nHistos));
            hh->SetFillColor((int)atof(inColors.at(j).c_str()));
            hh->SetLineColor((int)atof(inColors.at(j).c_str()));
            hh->SetFillStyle((int)atof(inFillStyles.at(j).c_str()));
            hError->Add(hh2);
            hMC->Add(hh3);
            hs->Add(hh);
            if((int)atof(inLegend.at(j).c_str())) {
              legend->AddEntry(hh,inProcess.at(j).c_str(), "f");
            }
          }
        }
      }

      hData->Divide(hMC);
      hData->GetXaxis()->SetLabelFont(42);
      hData->GetXaxis()->SetLabelOffset(0.007);
      hData->GetXaxis()->SetLabelSize(0.11);
      hData->GetXaxis()->SetTitleSize(0.12);
      hData->GetXaxis()->SetTitleOffset(0.9);
      hData->GetXaxis()->SetTitleFont(42);
      hData->GetYaxis()->SetTitle("#frac{Data}{MC}");
      hData->GetYaxis()->SetNdivisions(505);
      hData->GetYaxis()->SetLabelFont(42);
      hData->GetYaxis()->SetLabelOffset(0.012);
      hData->GetYaxis()->SetLabelSize(0.11);
      hData->GetYaxis()->SetTitleSize(0.09);
      hData->GetYaxis()->SetTitleOffset(0.7);
      hData->GetYaxis()->SetTitleFont(42);
//      hData->SetMinimum(0.0);
//      hData->SetMaximum(2.99);
      hData->SetLineStyle(0);
      hData->SetLineColor(1);
      hData->SetMarkerStyle(20);
//      hData->SetMarkerSize(0.8);

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

      hData->GetListOfFunctions()->Add(PrevFitTMP);
      hData->Draw("e1p");

      for(int bin=0; bin < h->GetXaxis()->GetNbins(); bin++) {
        mcY2[bin] = 1;
        mcErrorY2[bin] = hError->GetBinError(bin+1) / hError->GetBinContent(bin+1);
        mcX2[bin] = hError->GetBinCenter(bin+1);
        mcErrorX2[bin] = hError->GetBinWidth(bin+1) * 0.5;
      }
      TGraphErrors *mcError2 = new TGraphErrors(h->GetXaxis()->GetNbins(),mcX2,mcY2,mcErrorX2,mcErrorY2);
      mcError2->SetLineWidth(0);
      mcError2->SetFillColor(1);
      mcError2->SetFillStyle(3002);
      mcError2->Draw("sameE2");

      c1_1->Modified();
      c->cd();



      TPad *c1_2 = new TPad("c1_2", "newpad",0.01,0.33,0.99,0.99);
      c1_2->Draw();
      c1_2->cd();
      c1_2->Range(-200,-13.87376,1133.333,1389.866);
      c1_2->SetFillColor(0);
      c1_2->SetBorderMode(0);
      c1_2->SetBorderSize(2);
      c1_2->SetTickx(1);
      c1_2->SetTicky(1);
      c1_2->SetLeftMargin(0.15);
      c1_2->SetBottomMargin(0.01);
      c1_2->SetFrameFillStyle(0);
      c1_2->SetFrameBorderMode(0);
      c1_2->SetFrameFillStyle(0);
      c1_2->SetFrameBorderMode(0);

      h->SetMarkerStyle(20);
//      h->SetMarkerSize(0.0);
      h->SetLineColor(1);
      h->GetXaxis()->SetLabelFont(42);
      h->GetXaxis()->SetLabelOffset(0.007);
      h->GetXaxis()->SetLabelSize(0.11);
      h->GetXaxis()->SetTitleSize(0.12);
      h->GetXaxis()->SetTitleOffset(0.9);
      h->GetXaxis()->SetTitleFont(42);

      h->GetYaxis()->SetTitle("Events");
      h->GetYaxis()->SetLabelFont(42);
      h->GetYaxis()->SetLabelOffset(0.007);
      h->GetYaxis()->SetLabelSize(0.05);
      h->GetYaxis()->SetTitleSize(0.06);
      h->GetYaxis()->SetTitleOffset(1.1);
      h->GetYaxis()->SetTitleFont(42);
      h->GetZaxis()->SetLabelFont(42);
      h->GetZaxis()->SetLabelOffset(0.007);
      h->GetZaxis()->SetLabelSize(0.05);
      h->GetZaxis()->SetTitleSize(0.06);
      h->GetZaxis()->SetTitleFont(42);
      h->Draw("ep");
      hs->Draw("same");

      for(int bin=0; bin < h->GetXaxis()->GetNbins(); bin++) {
        mcY[bin] = hError->GetBinContent(bin+1);
        mcErrorY[bin] = hError->GetBinError(bin+1);
        mcX[bin] = hError->GetBinCenter(bin+1);
        mcErrorX[bin] = hError->GetBinWidth(bin+1) * 0.5;
      }
      TGraphErrors *mcError = new TGraphErrors(h->GetXaxis()->GetNbins(),mcX,mcY,mcErrorX,mcErrorY);
      mcError->SetLineWidth(0);
      mcError->SetFillColor(1);
      mcError->SetFillStyle(3002);
      mcError->Draw("sameE2");

      hhh->SetMarkerStyle(20);
//      hhh->SetMarkerSize(0.0);
      hhh->SetLineColor(1);
      hhh->Draw("samee1p");

      legend->SetTextFont(42);
      legend->SetTextSize(0.04195804);
      legend->SetLineColor(1);
      legend->SetLineStyle(1);
      legend->SetLineWidth(1);
      legend->SetFillColor(0);
      legend->SetFillStyle(1001);
      legend->SetBorderSize(0);
      legend->SetFillColor(kWhite);
      legend->Draw();

      TPaveText *pt = new TPaveText(0.2620189,0.9409833,0.662649,0.9913117,"brNDC");
      pt->SetBorderSize(0);
      pt->SetFillColor(0);
      pt->SetLineColor(0);
      pt->SetTextSize(0.05297732);
      string TitleString = "CMS Preliminary, L_{int} = " + lumi_string + " fb^{-1}, #sqrt{s} = 13 TeV";
      TText *text = pt->AddText(TitleString.c_str());
      text->SetTextFont(42);
      pt->Draw();

      c1_2->Modified();
      c->cd();
      c->Modified();
      c->cd();
      c->SetSelected(c);

      TFile *hfile = (TFile*)gROOT->FindObject(outputRootFile.c_str());
      if (hfile) {hfile->Close();}
      hfile = new TFile(outputRootFile.c_str(),"UPDATE");
      c->Write();
      std::cout << "     BSM3GPlotMaker 'makePlots' function: Writing out plot with name = " << theHistNameStrings.at(i).c_str() << std::endl;
      hfile->Close();
      c->Close();

      delete hhh;
      delete hError;
      delete hData;
      delete hMC;


    }
  }

}

//---function called once just before obtaining user inputs. It clears the vectors and initializes parameters.
void BSM3GPlotMaker::endJob() {

  theHistNameStrings.clear();
  inLegend.clear();
  inFillStyles.clear();
  inColors.clear();
  inProcess.clear();
  inRootFiles.clear();
  rebinNumber.clear();
  inScaleFactor.clear();
  inScaleFactorError.clear();
  HistList.clear();
  HistList2.clear();
  theCurrentFile->Close();

}

BSM3GPlotMaker::~BSM3GPlotMaker() { };
