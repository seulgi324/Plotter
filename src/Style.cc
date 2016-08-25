gStyle->SetOptStat(0);
gStyle->SetOptTitle(0);



void setCanvas(TCanvas* c) {
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
}

void setPad(TPad* pad) {
  pad->Draw();
  pad->cd();
  pad->Range(-200,-2.720435,1133.333,3.047681);
  pad->SetFillColor(0);
  pad->SetFillStyle(4000);
  pad->SetBorderMode(0);
  pad->SetBorderSize(2);
  pad->SetTickx(1);
  pad->SetTicky(1);
  pad->SetLeftMargin(0.15);
  pad->SetTopMargin(0.01);
  pad->SetBottomMargin(0.3);
  pad->SetFrameFillStyle(0);
  pad->SetFrameBorderMode(0);
  pad->SetFrameFillStyle(0);
  pad->SetFrameBorderMode(0);

}

void setFunc(TF1* PrevFitTMP) {

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
}


// void setLengend(TLegend* leg) {
//   leg->AddEntry(h,inProcess.at(0).c_str(), "lep");
//   if(inRootFiles.size() > 1) {
//     for(int j=1;j<inRootFiles.size();j++) {
//       string hist2name = HistList.at(i+(j*nHistos))->GetName();
//       if(theHistNameStrings.at(i) == theHistNameStrings.at(i+(j*nHistos))) {
// 	TH1F *hh = (TH1F*)HistList.at(i+(j*nHistos));
// 	TH1F *hh2 = (TH1F*)HistList2.at(i+(j*nHistos));
// 	TH1F *hh3 = (TH1F*)HistList2.at(i+(j*nHistos));
// 	hh->SetFillColor((int)atof(inColors.at(j).c_str()));
// 	hh->SetLineColor((int)atof(inColors.at(j).c_str()));
// 	hh->SetFillStyle((int)atof(inFillStyles.at(j).c_str()));
// 	hError->Add(hh2);
// 	hMC->Add(hh3);
// 	hs->Add(hh);
// 	if((int)atof(inLegend.at(j).c_str())) {
// 	  legend->AddEntry(hh,inProcess.at(j).c_str(), "f");
// 	}
//       }
//     }
//   }

// legend->SetTextFont(42);
// legend->SetTextSize(0.04195804);
// legend->SetLineColor(1);
// legend->SetLineStyle(1);
// legend->SetLineWidth(1);
// legend->SetFillColor(0);
// legend->SetFillStyle(1001);
// legend->SetBorderSize(0);
// legend->SetFillColor(kWhite);
// legend->Draw();

// }



THStack *hs = new THStack(theHistNameStrings.at(i).c_str(),theHistNameStrings.at(i).c_str());
string histname = HistList.at(i)->GetName();
TH1F *h = (TH1F*)HistList.at(i);
h->SetName(histname.c_str());


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
l
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
