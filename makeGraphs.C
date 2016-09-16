vector<double> rebinner(TH1* hist, double limit);

void makeGraphs() {

  TFile* withfile = new TFile("MCwith.root");
  TFile* withoutfile = new TFile("MCwithout.root");

  withfile->cd("METCut");
  TList* with = new TList();
  without->Add((TH1D*)gDirectory->FindObjectAny("NVertices"));
  without->Add((TH1D*)gDirectory->FindObjectAny("Met"));
  without->Add((TH1D*)gDirectory->FindObjectAny("Meff"));
  without->Add((TH1D*)gDirectory->FindObjectAny("BJetPt"));

  withoutfile->cd("METCut");
  TList* without = new TList();
  without->Add((TH1D*)gDirectory->FindObjectAny("NVertices"));
  without->Add((TH1D*)gDirectory->FindObjectAny("Met"));
  without->Add((TH1D*)gDirectory->FindObjectAny("Meff"));
  without->Add((TH1D*)gDirectory->FindObjectAny("BJetPt"));

  TFile* newfile = new TFile("both.root", "RECREATE");
  newfile->cd();

  TH1D* hw = (TH1D*)with->First();
  TH1D* hwo = (TH1D*)without->First();
  while(hw) {
    
    vector<double> bins = rebinner(wMET, 0.3);
    if(bins.size() == 0) return;
    double* binner = new double[bins.size()];
    bool passed = true;
    for(int i = 0; i < bins.size(); i++) {
      if(i == 0) {}
      else if(bins.at(bins.size() - i) >= bins.at(bins.size() - i - 1))  {
	passed = false;
	break;
      }

      binner[i] = bins.at(bins.size() - i - 1);
    }
    wMET = (TH1D*)wMET->Rebin(bins.size()-1, "wMET_rebin", binner);
    woMET = (TH1D*)woMET->Rebin(bins.size()-1, "woMET_rebin", binner);
  
    TH1D* sub = (TH1D*)wMET->Clone();
    sub->Add(woMET, -1);

    TCanvas* c1 = new TCanvas();
    sub->Draw();
    c1->Write("nvertices");

    c1->Close();
    hw = (TH1D*)with->After(hw);
    hwo = (TH1D*)without->After(hwo);
  }


    
}


vector<double> rebinner(TH1* hist, double limit) {
  vector<double> bins;
  double toterror = 0.0, prevbin=0.0;
  double limit2 = pow(limit,2);
  bool foundfirst = false;
  double end;

  //  if(hist) return new double(1);
  //how to tell if ok?

  if(hist->GetEntries() == 0 || hist->Integral() <= 0) return bins;

  for(int i = hist->GetXaxis()->GetNbins(); i > 0; i--) {
    if(hist->GetBinContent(i) <= 0.0) continue;
    if(!foundfirst) { 
      bins.push_back(hist->GetXaxis()->GetBinUpEdge(i));
      foundfirst = true;
    } else end = hist->GetXaxis()->GetBinLowEdge(i);

    if(toterror* prevbin != 0.) toterror *= pow(prevbin,2)/pow(prevbin+hist->GetBinContent(i),2);
    prevbin += hist->GetBinContent(i);
    toterror += (2 * pow(hist->GetBinError(i),2))/pow(prevbin,2);
    if(toterror < limit2) {
      bins.push_back(hist->GetXaxis()->GetBinLowEdge(i));
      toterror = 0.0;
      prevbin = 0.0;
    }
  }

  if(bins.back() != end) {
    bins.push_back(end);
    if(hist->GetXaxis()->GetXmin() >= 0 && end !=hist->GetXaxis()->GetXmin()) bins.push_back(hist->GetXaxis()->GetXmin());
  }

  return bins;
}
