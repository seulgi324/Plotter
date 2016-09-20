#include "Plotter.h"
#include "Normalizer.h"

using namespace std;


Plot fullPlot;
double lumi;
string stylename = "default";
bool ssqrtsb = true;

int main(int argc, char* argv[]) {
  if(argc < 2) {
    cerr << "No config file given: Exiting" << endl;
    exit(1);
  }

  string output;
  map<string, Normer*> plots;
  vector<string> inputnames;
  for(int i = 1; i < argc; ++i) {
    if(argv[i][0] == '-') {
      if(strcmp(argv[i],"-help") == 0) {
	cout << "Usage: ./Plotter [OPTION] [CONFIG FILE]" << endl;
	cout << "Plotter allows for making stack plots with ratio or significance plots as a\n" << endl;
	cout << "secondary graph. The Default graph is the Ratio plot.  You can change this with\nthe different options" << endl << endl;
	cout << "    -sigleft      Significance plot cumulative from the left. Entry in bin i" << endl;
	cout << "                  represents siginifcance if events with values in bins greater" << endl;
	cout << "                  than i are cut" << endl;
	cout << "    -sigright     Significance plot cumulative from the right. Entry in bin i" << endl;
	cout << "                  represents significance if events with values in bins lower" << endl;
	cout << "                  than i are cut" << endl;
	cout << "    -sigbin       Significance plot with significance of each bin. Not" << endl;
	cout << "                  cumulative, just significance of the respective bin" << endl;
	cout << "    -ssqrtb       Default for Significance is using the formula:" << endl;
        cout << "                  s/sqrt(s + b)" << endl;
	cout << "                  This option changes the significance calculation to be:" << endl;
	cout << "                  s/sqrt(b)" << endl;
	exit(0);
      } else if( strcmp(argv[i], "-sigleft") == 0) bottomType = SigLeft;
      else if( strcmp(argv[i], "-sigright") == 0) bottomType = SigRight;
      else if( strcmp(argv[i],"-sigbin") == 0) bottomType = SigBin;
      else if( strcmp(argv[i],"-ssqrtb") == 0) ssqrtsb = false;
      else {
	cout << "wrong option, exiting" << endl;
	exit(0);
      }
    } else read_info(argv[i], output, plots);
  }

  int totalfiles = 0;
  vector<string> datan, bgn, sign;
  for(map<string, Normer*>::iterator it = plots.begin(); it != plots.end(); ++it) {
    if(it->second->use == 0) continue;
    string filename = it->second->output;
    while(filename.find("#") != string::npos) {
      filename.erase(filename.find("#"), 1);
    }
    TFile* final = NULL;
    if(it->second->use == 1) {
      it->second->lumi = lumi;
      it->second->print();
      it->second->FileList = new TList();
      for(vector<string>::iterator name = it->second->input.begin(); name != it->second->input.end(); ++name) {
	it->second->FileList->Add(TFile::Open(name->c_str()));
      }
    
      final = new TFile(filename.c_str(), "RECREATE");
      it->second->MergeRootfile(final);
    } else if(it->second->use == 2) {
      final = new TFile(filename.c_str());
    }
    final->SetTitle(it->second->output.c_str());
    fullPlot.addFile(it->second->type, final);
    totalfiles++;
    if(it->second->type == "data") datan.push_back(it->second->output);
    else if(it->second->type == "bg") bgn.push_back(it->second->output);
    else if(it->second->type == "sig") sign.push_back(it->second->output);
  }
  TFile* final = new TFile(output.c_str(), "RECREATE");
  ofstream logfile;
  logfile.open("log.txt", ios::out);
  logfile << "\\begin{tabular}{ | l |";
  for(int i = 0; i < totalfiles; i++) {
    logfile << " c |";
  }
  logfile << " }" << endl << "\\hline" << endl << "Process";
  for(vector<string>::iterator it = datan.begin(); it != datan.end(); it++) logfile << " & " << it->substr(0, it->length()-5);
  for(vector<string>::iterator it = bgn.begin(); it != bgn.end(); it++) logfile << " & " << it->substr(0, it->length()-5);
  for(vector<string>::iterator it = sign.begin(); it != sign.end(); it++) logfile << " & " << it->substr(0, it->length()-5);
  logfile << "\\\\ \\hline" << endl;

  Style stylez("style/" + stylename);

  gStyle = stylez.getStyle();
  CreateStack(final, fullPlot, stylez, logfile);
  final->Close();
  logfile << "\\end{tabular}" << endl;
  logfile.close();
}


void read_info(string filename, string& output, map<string, Normer*>& plots) {
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  ifstream info_file(filename);
  boost::char_separator<char> sep(", \t");

  if(!info_file) {
    std::cout << "could not open file " << filename <<std::endl;
    exit(1);
  }

  vector<string> stemp;
  string group, line;
  while(getline(info_file, line)) {
    tokenizer tokens(line, sep);
    stemp.clear();
    for(tokenizer::iterator iter = tokens.begin();iter != tokens.end(); iter++) {
      if( ((*iter)[0] == '/' && (*iter)[0] == '/') || ((*iter)[0] == '#') ) break;
      stemp.push_back(*iter);

    }

    if(stemp.size() >= 2) {
      if(stemp[0].find("lumi") != string::npos) lumi = stod(stemp[1]);
      else if(stemp[0].find("output") != string::npos) output = stemp[1];
      else if(stemp[0].find("style") != string::npos) stylename = stemp[1];
      else if(plots.find(stemp[1]) == plots.end()) { 
	Normer* tmpplot = new Normer();
	tmpplot->input.push_back(stemp[0]);
	tmpplot->output = stemp[1];
	tmpplot->use = shouldAdd(stemp[0], stemp[1]);
	tmpplot->CumulativeEfficiency.push_back(1.);
	tmpplot->scaleFactor.push_back(1.);
	tmpplot->scaleFactorError.push_back(0.);
	tmpplot->integral.push_back(0);
	if(stemp.size() == 2) {
	  tmpplot->xsec.push_back(1.0);
	  tmpplot->skim.push_back(1.0);
	  tmpplot->isData = true;
	  tmpplot->type = "data";
	}    
	plots[stemp[1]] = tmpplot;
      } else {
	plots[stemp[1]]->input.push_back(stemp[0]);
	plots[stemp[1]]->use = min(shouldAdd(stemp[0], stemp[1]),plots[stemp[1]]->use);
	plots[stemp[1]]->CumulativeEfficiency.push_back(1.);
	plots[stemp[1]]->scaleFactor.push_back(1.);
	plots[stemp[1]]->scaleFactorError.push_back(0.);
	plots[stemp[1]]->integral.push_back(0);
	if(stemp.size() == 2) {
	  plots[stemp[1]]->xsec.push_back(1.0);
	  plots[stemp[1]]->skim.push_back(1.0);
	}    

      }
      if(stemp.size() == 5) {
	plots[stemp[1]]->xsec.push_back(stod(stemp[2]));
	plots[stemp[1]]->skim.push_back(stod(stemp[3]));
	if(plots[stemp[1]]->type == "") plots[stemp[1]]->type = stemp[4];
      }
    } 
  }
  info_file.close();

}

int shouldAdd(string infile, string globalFile) {
  struct stat buffer;
  if(stat(infile.c_str(), &buffer) != 0) return 0;
  ////need to change so doesn't make plot if fatal error (eg file doesn't exist!
  else if(stat(globalFile.c_str(), &buffer) != 0) return 1;
  else if(getModTime(globalFile.c_str()) > getModTime(infile.c_str())) return 2;
  else return 1;

}

int getModTime(const char *path) {
  struct stat attr;
  stat(path, &attr);
  char date[100] = {0};
  strftime(date, 100, "%s", localtime(&attr.st_mtime));
  return atoi(date);

}

/*-----------------------------*/


void CreateStack( TDirectory *target, Plot& plot, Style& styler, ofstream& logfile) {

  TList* datalist = plot.FileList[0];
  TList* bglist = plot.FileList[1];
  TList* sglist = plot.FileList[2];

  TString path( (char*)strstr( target->GetPath(), ":" ) );
  path.Remove( 0, 2 );

  TFile *dataStart = (TFile*)datalist->First();
  dataStart->cd( path );
  TDirectory *current_sourcedir = gDirectory;

  Bool_t status = TH1::AddDirectoryStatus();
  TH1::AddDirectory(kFALSE);


  ///try to find events to calculate efficiency
  TH1* events;
  current_sourcedir->GetObject("Events", events);

  if(events) {
    logfile << current_sourcedir->GetName();
    logfile << " & " << fixed << events->GetBinContent(2) << setprecision(1);

    TFile *nextsource = (TFile*)datalist->After( dataStart );
    while( nextsource) {
      nextsource->cd(path);
      gDirectory->GetObject("Events", events);
      logfile << " & " << fixed << events->GetBinContent(2) << setprecision(1);
      nextsource = (TFile*)datalist->After( nextsource );
    }

    nextsource = (TFile*)bglist->First();
    while ( nextsource ) {
      nextsource->cd(path);
      gDirectory->GetObject("Events", events);
      logfile << " & " << fixed << events->GetBinContent(2) << setprecision(1) << " $\\pm$" << fixed << events->GetBinError(2) << setprecision(1);
      nextsource = (TFile*)bglist->After( nextsource );
    }
    nextsource = (TFile*)sglist->First();
    while ( nextsource ) {
      nextsource->cd(path);
      gDirectory->GetObject("Events", events);
      logfile << " & " << fixed << events->GetBinContent(2) << setprecision(1) << " $\\pm$" << fixed << events->GetBinError(2) << setprecision(1);
      nextsource = (TFile*)sglist->After( nextsource );
    }
    logfile << " \\\\ \\hline" << endl;
  }

  delete events;

  // loop over all keys in this directory
  TIter nextkey( current_sourcedir->GetListOfKeys() );
  TKey *key, *oldkey=0;
  while ( (key = (TKey*)nextkey())) {

    //keep only the highest cycle number for each key
    if (oldkey && !strcmp(oldkey->GetName(),key->GetName())) continue;

    dataStart->cd( path );

    TObject *obj = key->ReadObj();
    if ( obj->IsA() ==  TH1D::Class() ) {
      
      TH1 *h1 = (TH1*)obj;
      TH1D* error = new TH1D("error", "error", h1->GetXaxis()->GetNbins(), h1->GetXaxis()->GetXmin(), h1->GetXaxis()->GetXmax());
      TH1D* datahist = new TH1D("data", "data", h1->GetXaxis()->GetNbins(), h1->GetXaxis()->GetXmin(), h1->GetXaxis()->GetXmax());
      TList* sigHists = new TList();
      THStack *hs = new THStack(h1->GetName(),h1->GetName());
      
      /*------------data--------------*/
      datahist->Add(h1);
      TFile *nextfile = (TFile*)datalist->After(dataStart);

      while ( nextfile ) {
	nextfile->cd( path );
	TKey *key2 = (TKey*)gDirectory->GetListOfKeys()->FindObject(h1->GetName());
	if(key2) {
	  TH1* h2 = (TH1*)key2->ReadObj();
	  datahist->Add(h2);
	  delete h2;
	}
	nextfile = (TFile*)datalist->After(nextfile);
      }

      /*------------background--------------*/
      int nfile = 0;
      nextfile = (TFile*)bglist->First();

      while ( nextfile ) {
	nextfile->cd( path );
	TKey *key2 = (TKey*)gDirectory->GetListOfKeys()->FindObject(h1->GetName());
	if(key2) {
	  TH1* h2 = (TH1*)key2->ReadObj();
	  error->Add(h2);
	  for(int i = 1; i < h2->GetXaxis()->GetNbins()+1; i++) {
	    h2->SetBinError(i, 0);
	  }
	  //////style
	  string title = nextfile->GetTitle();
	  title = title.substr(0, title.size()-5);
	  h2->SetTitle(title.c_str());
	  h2->SetLineColor(1);
	  h2->SetFillStyle(1001);
	  h2->SetFillColor(plot.color[nfile]);
	
	  hs->Add(h2);
	}
	nextfile = (TFile*)bglist->After(nextfile);
	nfile++;
      }


      /*------------signal--------------*/

      nextfile = (TFile*)sglist->First();

      while ( nextfile ) {
	nextfile->cd( path );
	TKey *key2 = (TKey*)gDirectory->GetListOfKeys()->FindObject(h1->GetName());
	if(key2) {
	  TH1D* h2 = (TH1D*)key2->ReadObj();
	  for(int i = 1; i < h2->GetXaxis()->GetNbins()+1; i++) {
	    h2->SetBinError(i, 0);
	  }

	  //////style
	  string title = nextfile->GetTitle();
	  title = title.substr(0, title.size()-5);
	  h2->SetTitle(title.c_str());
	  h2->SetLineColor(plot.color[nfile]);
	  h2->SetLineWidth(3);
	  h2->SetLineStyle(2);

	  sigHists->Add(h2);

	}
	nextfile = (TFile*)sglist->After(nextfile);
	nfile++;
      }


      /*--------------write out------------*/

      ///data
      datahist->SetMarkerStyle(20);
      datahist->SetLineColor(1);

      if(hs == NULL || hs->GetNhists() == 0) continue;
      hs = sortStack(hs);

      ///rebin
      ////////////decide how to rebin(data, background, both)
      ////check if binning is valid
      vector<double> bins = rebinner(datahist, styler.getRebinLimit());
      if(bins.size() == 0) continue;
      double* binner = new double[bins.size()];
      bool passed = true;
      binner[0] = bins.back();
      for(int i = 1; i < bins.size(); i++) {
	if(bins.at(bins.size() - i) >= bins.at(bins.size() - i - 1))  {
	  passed = false;
	  break;
	}
	binner[i] = bins.at(bins.size() - i - 1);
      }

      ////rebin histograms
      THStack* hsdraw = hs;
      if(passed && bins.size() > styler.getBinLimit()) {
	datahist = (TH1D*)datahist->Rebin(bins.size()-1, "data_rebin", binner);
	error = (TH1D*)error->Rebin(bins.size()-1, "error_rebin", binner);
	hsdraw = rebinStack(hs, binner, bins.size()-1);	
	TList* tmplist = new TList();
	TH1D* onesig = (TH1D*)sigHists->First();
	while(onesig) {
	  tmplist->Add(onesig->Rebin(bins.size()-1, onesig->GetName(), binner));
	  onesig = (TH1D*)sigHists->After(onesig);
	}
	delete sigHists;
	sigHists = tmplist;
      } 

      ///legend stuff
      TLegend* legend = createLeg(hsdraw->GetHists());
      legend->AddEntry(datahist, "Data", "lep");
      TH1D* tmpsig = (TH1D*)sigHists->First();
      while(tmpsig) {
	legend->AddEntry(tmpsig, tmpsig->GetName(), "lep");
	tmpsig = (TH1D*)sigHists->After(tmpsig);
      }
      
      ////divide by binwidth is option is given
      if(styler.getDivideBins()) divideBin(datahist, error, hsdraw); ///add sig stuff as well
      
      //error
      TGraphErrors* errorstack = createError(error, false);


      ////draw graph
      target->cd();

      TCanvas *c = new TCanvas(h1->GetName(), h1->GetName());//403,50,600,600);
      c->Divide(1,2);
      c->cd(1);
      sizePad(styler.getPadRatio(), gPad, true);

      hsdraw->Draw();
      datahist->Draw("same");
      
      tmpsig = (TH1D*)sigHists->First();
      while(tmpsig) {
	tmpsig->Draw("same");
	tmpsig = (TH1D*)sigHists->After(tmpsig);
      }
      errorstack->Draw("2");
      legend->Draw();
      setYAxisTop(datahist, error, styler.getHeightRatio(), hsdraw);

      ///second pad
      c->cd(2);
      sizePad(styler.getPadRatio(), gPad, false);

      error->Draw("AXIS");
      setXAxisBot(error, styler.getPadRatio());

      TList* signalBot = (bottomType != Ratio) ? signalBottom(sigHists, error) : signalBottom(sigHists, datahist, error);

      TF1* PrevFitTMP = new TF1();
      TGraphErrors* errorratio = createError(error, true);
      if(bottomType == Ratio) {
	tmpsig = (TH1D*)signalBot->Last();
	delete PrevFitTMP;
	PrevFitTMP = createLine(tmpsig);
	setYAxisBot(error->GetYaxis(), tmpsig, styler.getPadRatio());
      } else setYAxisBot(error->GetYaxis(), signalBot, styler.getPadRatio());

      tmpsig = (TH1D*)signalBot->First();
      while(tmpsig) {
	tmpsig->Draw("same");
	tmpsig = (TH1D*)signalBot->After(tmpsig);
      }
      if(bottomType == Ratio) errorratio->Draw("2");

      c->cd();
      c->Write(c->GetName());
      c->Close();
      
      hsdraw->Delete();
      delete datahist;
      delete error;
      delete sigHists;
      delete legend;
      delete errorstack;
      delete errorratio;
      delete PrevFitTMP;
      delete[] binner;

    } else if ( obj->IsA()->InheritsFrom( TDirectory::Class() ) ) {
      target->cd();
      TDirectory *newdir = target->mkdir( obj->GetName(), obj->GetTitle() );

      CreateStack( newdir, plot, styler, logfile );

    } else if ( obj->IsA()->InheritsFrom( TH1::Class() ) ) {
      continue;

    } else {
         cout << "Unknown object type, name: "
	   << obj->GetName() << " title: " << obj->GetTitle() << endl;
    }
  }

  TH1::AddDirectory(status);
}

THStack* sortStack(THStack* old) {
  if(old == NULL || old->GetNhists() == 0) return old;
  string name = old->GetName();

  THStack* newstack = new THStack(name.c_str(),name.c_str());

  TList* list = (TList*)old->GetHists();

  while(list->GetSize() > 0) {
    TIter next(list);
    TH1* smallest = NULL;
    TH1* tmp = NULL;
    while ( (tmp = (TH1*)next()) ) {
      if(smallest == NULL || smallest->Integral() > tmp->Integral()) smallest = tmp;
    }
    newstack->Add(smallest);
    list->Remove(smallest);

  }
  
  delete old;
  return newstack;
} 

TLegend* createLeg(TList* bgl, TList* sigl) {
  TLegend* leg = new TLegend(0.73,0.70,0.93,0.90);
  vector<TList*> vlist;
  if(bgl!=NULL) vlist.push_back(bgl);
  if(sigl!=NULL) vlist.push_back(sigl);
  for(vector<TList*>::iterator it = vlist.begin(); it != vlist.end(); ++it) {
    TIter next(*it);
    TH1* tmp = NULL;
    while( (tmp = (TH1*)next()) ) {
      leg->AddEntry(tmp, tmp->GetTitle(), "f");
    }
  }
  return leg;

}

TGraphErrors* createError(TH1* error, bool ratio) {
  int Nbins =  error->GetXaxis()->GetNbins();
  Double_t* mcX = new Double_t[Nbins];
  Double_t* mcY = new Double_t[Nbins];
  Double_t* mcErrorX = new Double_t[Nbins];
  Double_t* mcErrorY = new Double_t[Nbins];

  for(int bin=0; bin < error->GetXaxis()->GetNbins(); bin++) {
    mcY[bin] = (ratio) ? 1.0 : error->GetBinContent(bin+1);
    mcErrorY[bin] = (ratio) ?  error->GetBinError(bin+1)/error->GetBinContent(bin+1) : error->GetBinError(bin+1);
    mcX[bin] = error->GetBinCenter(bin+1);
    mcErrorX[bin] = error->GetBinWidth(bin+1) * 0.5;
  }
  TGraphErrors *mcError = new TGraphErrors(error->GetXaxis()->GetNbins(),mcX,mcY,mcErrorX,mcErrorY);

  mcError->SetLineWidth(1);
  mcError->SetFillColor(1);
  mcError->SetFillStyle(3002);

  delete[] mcX;
  delete[] mcY;
  delete[] mcErrorX;
  delete[] mcErrorY;
  return mcError;
}

void sizePad(double ratio, TVirtualPad* pad, bool isTop) {
  if(isTop) pad->SetPad("top", "top", 0, 1 / (1.0 + ratio), 1, 1, 0);
  else  {
    pad->SetPad("bottom", "bottom", 0, 0, 1, 1 / (1.0 + ratio), 0);
    pad->SetMargin(pad->GetLeftMargin(),pad->GetRightMargin(),ratio*pad->GetBottomMargin(),0);
    pad->SetTitle("");
  }
}

TF1* createLine(TH1* data_mc) {
  TF1 *PrevFitTMP = new TF1("PrevFitTMP","pol0",-10000,10000);
  PrevFitTMP->SetMarkerStyle(20);
  PrevFitTMP->SetLineColor(2);
  PrevFitTMP->SetLineWidth(1);
  PrevFitTMP->SetParameter(0,1.0);
  PrevFitTMP->SetParError(0,0);
  PrevFitTMP->SetParLimits(0,0,0);
  data_mc->GetListOfFunctions()->Add(PrevFitTMP);
  return PrevFitTMP;
}
 
vector<double> rebinner(TH1* hist, double limit) {
  vector<double> bins;
  double toterror = 0.0, prevbin=0.0;
  double limit2 = pow(limit,2);
  bool foundfirst = false;
  double end;

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


double* rebinner(TH1* hist1, TH1* hist2, double limit) {
  vector<double> bins;
  double toterror1 = 0.0, prevbin1=0.0;
  double toterror2 = 0.0, prevbin2=0.0;
  double limit2 = pow(limit,2);
  bool foundfirst = false;
  double end;

  for(int i = hist1->GetXaxis()->GetNbins(); i > 0; i--) {
    if(hist1->GetBinContent(i) <= 0.0 && hist2->GetBinContent(i) <= 0.0) continue;
    if(!foundfirst) { 
      bins.push_back(hist1->GetXaxis()->GetBinUpEdge(i));
      foundfirst = true;
    } else end = hist1->GetXaxis()->GetBinLowEdge(i);

    if(toterror1* prevbin1 != 0.) toterror1 *= pow(prevbin1,2)/pow(prevbin1+hist1->GetBinContent(i),2);
    if(toterror2* prevbin2 != 0.) toterror2 *= pow(prevbin2,2)/pow(prevbin2+hist2->GetBinContent(i),2);
    prevbin1 += hist1->GetBinContent(i);
    prevbin2 += hist2->GetBinContent(i);
    toterror1 += (2 * pow(hist1->GetBinError(i),2))/pow(prevbin1,2);
    toterror2 += (2 * pow(hist2->GetBinError(i),2))/pow(prevbin2,2);
    if(toterror1 < limit2 && toterror2 < limit2) {
      bins.push_back(hist1->GetXaxis()->GetBinLowEdge(i));
      toterror1 = 0.0;
      prevbin1 = 0.0;
      toterror2 = 0.0;
      prevbin2 = 0.0;
    }
  }

  if(bins.back() != end) {
    bins.push_back(end);
    if(hist1->GetXaxis()->GetXmin() >= 0) bins.push_back(hist1->GetXaxis()->GetXmin());
  }
  
  double* newbins = new double[bins.size()];
  for(int i = 0; i < bins.size(); i++) {
    newbins[i] = bins.at(bins.size() - i - 1);
  }
  
  return newbins;
}

THStack* rebinStack(THStack* hs, double* binner, int total) {
  THStack* newstack = new THStack(hs->GetName(), hs->GetName());
  TList* list = (TList*)hs->GetHists();

  TH1D* tmp = (TH1D*)list->First();
  while ( tmp ) {
    TH1D* forstack = (TH1D*)tmp->Clone();
    forstack = (TH1D*)forstack->Rebin(total, tmp->GetName(), binner);
    newstack->Add(forstack);
    tmp = (TH1D*)list->After(tmp);
  }
  
  hs->Delete();

  return newstack;
}



void setYAxisTop(TH1* datahist, TH1* error, double ratio, THStack* hs) {
  TAxis* yaxis = hs->GetYaxis();
  //  if(dividebins) yaxis->SetTitle("Events/GeV");////get axis title stuff
  yaxis->SetTitle("Events");////Need to sort out units and stuff
  yaxis->SetLabelSize(hs->GetXaxis()->GetLabelSize());
  double max = (error->GetMaximum() > datahist->GetMaximum()) ? error->GetMaximum() : datahist->GetMaximum();
  hs->SetMaximum(max*(1.0/ratio + 1.0));

}

TList* signalBottom(TList* signal, TH1D* background) {
  TList* returnList = new TList();
  
  TH1D* holder = (TH1D*)signal->First();

  while(holder) {
    TH1D* signif = (TH1D*)holder->Clone();
    int Nbins = signif->GetXaxis()->GetNbins();
    for(int i = 0; i < Nbins;i++) {
      if(signif->GetBinContent(i+1) <= 0 && background->GetBinContent(i+1) <= 0) continue;
      int edge1 = i+1, edge2= i+1;
      double sigErr, backErr;
      if(bottomType == SigLeft) edge1 = 0;
      if(bottomType == SigRight) edge2 = Nbins;
      double sigInt = signif->IntegralAndError(edge1, edge2, sigErr);
      double backInt = background->IntegralAndError(edge1, edge2, backErr);
	  
      double total = (ssqrtsb) ? sigInt/sqrt(sigInt+backInt) : sigInt/sqrt(backInt);
      double perErr = (ssqrtsb) ? pow(sigErr/sigInt-sigErr/(2*(sigInt+backInt)),2) + pow(backErr/(2*(sigInt+backInt)),2) : pow(sigErr/sigInt,2) + pow(backErr/(2*backInt),2);

      signif->SetBinContent(i+1, total);
      signif->SetBinError(i+1, total*perErr);
    }
    returnList->Add(signif);
    holder = (TH1D*)signal->After(holder);
  }
  holder = NULL;
  return returnList;
}

TList* signalBottom(TList* signal, TH1D* data, TH1D* background) {
  TList* returnList = new TList();
  
  TH1D* holder = (TH1D*)signal->First();

  while(holder) {
    TH1D* total = (TH1D*)holder->Clone();
    total->Add(background);
    
    total->Divide(data, total);

    returnList->Add(total);
    holder = (TH1D*)signal->After(holder);
  }
  TH1D* data_mc = (TH1D*)data->Clone();
  data_mc->Divide(background);
  returnList->Add(data_mc);

  holder = NULL;
  return returnList;
}

  

void setXAxisBot(TH1* data_mc, double ratio) {
  TAxis* xaxis = data_mc->GetXaxis();
  xaxis->SetLabelSize(xaxis->GetLabelSize()*ratio);
}

void setYAxisBot(TAxis* yaxis, TH1* data_mc, double ratio) {
  double divmin = 0.0, divmax = 2.99;
  double low=2.99, high=0.0, tmpval;
  for(int i = 0; i < data_mc->GetXaxis()->GetNbins(); i++) {
    tmpval = data_mc->GetBinContent(i+1);
    if(tmpval < 2.99 && tmpval > high) {high = tmpval;}
    if(tmpval > 0. && tmpval < low) {low = tmpval;}
  }
  double val = min(abs(1 / (high - 1.)), abs(1 / (1/low -1.)));
  if(high == 0.0) val = 0;
  double factor = 2.0;
  while(val > factor || (factor == 2 && val > 1.)) {
    divmin = 1.0 - 1.0/factor;
    divmax = 1/divmin;
    factor *= 2.0;
  }

  yaxis->SetRangeUser(divmin - 0.00001,divmax - 0.00001);
  yaxis->SetLabelSize(yaxis->GetLabelSize()*ratio);
  yaxis->SetTitleSize(ratio*yaxis->GetTitleSize());
  yaxis->SetTitleOffset(yaxis->GetTitleOffset()/ratio);
  yaxis->SetTitle("#frac{Data}{MC}");
}

void setYAxisBot(TAxis* yaxis, TList* signal, double ratio) {
  double max = 0;
  TH1D* tmphist = (TH1D*)signal->First();
  while(tmphist) {
    max = (max < tmphist->GetMaximum()) ? tmphist->GetMaximum() : max;
    tmphist = (TH1D*)signal->After(tmphist);
  }

  yaxis->SetRangeUser(0,max*11./10 -0.00001);
  yaxis->SetLabelSize(yaxis->GetLabelSize()*ratio);
  yaxis->SetTitleSize(ratio*yaxis->GetTitleSize());
  yaxis->SetTitleOffset(yaxis->GetTitleOffset()/ratio);
  if(ssqrtsb) yaxis->SetTitle("#frac{S}{#sqrt{S+B}}");
  else  yaxis->SetTitle("#frac{S}{#sqrt{B}}");
}


void divideBin(TH1* data, TH1* error, THStack* hs) {
  for(int i = 0; i < data->GetXaxis()->GetNbins(); i++) {
    data->SetBinContent(i+1, data->GetBinContent(i+1)/data->GetBinWidth(i+1));
    data->SetBinError(i+1, data->GetBinError(i+1)/data->GetBinWidth(i+1));
  }
  for(int i = 0; i < error->GetXaxis()->GetNbins(); i++) {
    error->SetBinContent(i+1, error->GetBinContent(i+1)/error->GetBinWidth(i+1));
    error->SetBinError(i+1, error->GetBinError(i+1)/error->GetBinWidth(i+1));
  }

  TList* list = (TList*)hs->GetHists();

  TIter next(list);
  TH1* tmp = NULL;
  while ( (tmp = (TH1*)next()) ) {
    for(int i = 0; i < tmp->GetXaxis()->GetNbins(); i++) {
      tmp->SetBinContent(i+1,tmp->GetBinContent(i+1)/tmp->GetBinWidth(i+1));
      tmp->SetBinError(i+1,tmp->GetBinError(i+1)/tmp->GetBinWidth(i+1));
    }
  }
   
}


