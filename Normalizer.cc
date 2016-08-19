#include "Normalizer.h"


Normalizer::Normalizer(string _name, vector<string> _files, vector<double> _xsec) :
  name(_name), files(_files), xsec(_xsec) {
}

void Normalizer::NormalizeHisto() {
  for(histograms) {
    string histname = HistList.at(i)->GetName();
    string histoEffyName = "hEffy_" + inProcess.at(0);
    TH1F *h = (TH1F*)HistList.at(i);
    h->Sumw2();
    if(IsData != "0") continue;
    if(h->Integral() > 0) h->Scale(1.0 / theEventsPassing.at(0) * x_section.at(0) * lumi * effSkim);
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


    TFile *hfile = (TFile*)gROOT->FindObject(outputRootFileForNormalizedHistos.c_str());
    if (hfile) {hfile->Close();}
    hfile = new TFile(outputRootFileForNormalizedHistos.c_str(),"UPDATE");
    h->Write();
    hfile->Close();
    delete h;
  

    if(inRootFiles.size() > 1) {
      for(int j=1;j<inRootFiles.size();j++) {
	string hist2name = HistList.at(i+(j*nHistos))->GetName();
	if(theHistNameStrings.at(i) != theHistNameStrings.at(i+(j*nHistos))) continue;
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

vector<string> dirvec;

void setupFiles() {
  bool firstTime = true;
  for(name in rootfiles) {
    TFile* tmpfile = new TFile(name.c_str());
    filevec.push_back(tmpfile);
    TIter tmpit(tmpfile->GetListOfKeys());

    while(TObject* obj = tmpit()) {
      if(firstTime)
	dirvec.push_back(obj->GetName());
      //      else if(find(dirvec.begin(), dirvec.end(), obj->GetName()) == dirvec.end())
      // directories???
    }
    firstTime = false;
  }
}

/*
void doStuff() {
  for(vector<string>::iterator it = dirvec.begin(); it != dirvec.end(); ++it) {
    for(int i = 0; i < filevec.size(); ++i) {
      filevec.at(i)->cd(it->c_str());
      get info;
      calculate eff;
    }
    for( histo in listofhistos) {
      does exist;
      TH1* writeout = new TH1(histname.c_str(), histname.c_str(), bins, xmin, xmax);
      for(int i = 0; i < filevec.size(); ++i) {
	filevec.at(i)->cd(it->c_str());
	TH1* tmp = (TH1*)filevec.at(i)->FindObject(histname.c_str());
	writeout.Add(writeout, tmp, 1, eff);
	error stuff;
      }
    }
  }
}
*/
