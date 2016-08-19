#include <TTree.h>
#include <TH1.h>
#include <TFile.h>
#include <TDirectory.h>
#include <TClass.h>
#include <TKey.h>
#include <TGraphAsymmErrors.h>
#include <TChain.h>


#include <vector>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <map>
#include "tokenizer.hpp"
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <array>

using namespace std;

struct Plot {
  vector<string> input;
  TList* FileList;
  string output;
  vector<double> skim, xsec;
  vector<double> events, RelativeEffVector, RelativeEffErrorVector, CumulativeEfficiency, scaleFactor, scaleFactorError;
  bool use;

  void print() {
    cout << " =========== " << output << " =========== " << endl;
    for(int i = 0; i < input.size(); ++i) {
      cout << input.at(i) << " " << skim.at(i) << " " << xsec.at(i) << ((use) ? " Used" : " Skip" )<<  endl;
    }
    cout << endl;
  }
};


vector<string> dirvec;
vector<array<double, 3>> histinfo;
vector<string> histnamevec;
map<string, Plot*> plots;

double lumi;
string lumi_string;
TFile *theCurrentFile;




void setupFiles(const vector<string>&, vector<TFile*>&);
inline int rounder(double val);
void calculateEfficienciesAndErrors(int, double, Plot&);
void read_info(string);
int getModTime(const char*);
bool shouldAdd(string, string);
void createLogFile(string);
void MergeRootfile( TDirectory*, Plot&);


int main(int argc, char* argv[]) {

  vector<string> files;
  read_info("config.txt");

  for(map<string, Plot*>::iterator it = plots.begin(); it != plots.end(); ++it) {
    //    if(! it->second->use) continue;
    it->second->print();
    it->second->FileList = new TList();
    for(vector<string>::iterator name = it->second->input.begin(); name != it->second->input.end(); ++name) {
      it->second->FileList->Add(TFile::Open(name->c_str()));
    }
    TFile* final = new TFile(it->second->output.c_str(), "RECREATE");
    MergeRootfile(final, *it->second);
    final->Close();
  }

  // vector<TFile*> filevec;
  // setupFiles(files, filevec);
  // for(int i = 0; i < histnamevec.size(); ++i) {
  //   cout << histnamevec.at(i) << " " << histinfo.at(i)[0] << " " << histinfo.at(i)[1] << " " << histinfo.at(i)[2] << endl;

  // }
}


// void setupFiles(const vector<string>& rootfiles, vector<TFile*>& filevec) {
//   bool firstTime = true;
//   bool firstdir = true;
//   for(vector<string>::const_iterator it= rootfiles.begin(); it != rootfiles.end(); ++it) {
//     string name = *it;
//     TFile* tmpfile = new TFile(name.c_str());
//     filevec.push_back(tmpfile);
//     TIter tmpit(tmpfile->GetListOfKeys());

//     while(TObject* obj = tmpit()) {
//       if(firstTime) {
// 	dirvec.push_back(obj->GetName());
// 	if(firstdir) {
// 	  cout << tmpfile->cd(obj->GetName()) << endl;
// 	  TIter tmphist(gDirectory->GetListOfKeys());

// 	  while(TKey* key = (TKey*)tmphist()) {
// 	    TH1F* hist = (TH1F*)key->ReadObj();
// 	    //	    if(! hist->InheritsFrom(TH1F::Class())) continue;
// 	    histnamevec.push_back(hist->GetName());
// 	    cout << (double)((TH1*)hist)->GetXaxis()->GetNbins() << endl;
// 	    histinfo.push_back( {(double)((TH1*)hist)->GetXaxis()->GetNbins(), ((TH1*)hist)->GetXaxis()->GetXmin(), \
// 		  ((TH1*)hist)->GetXaxis()->GetXmax()});
// 	  }
// 	}
// 	firstdir = false;
//       }      
//     }
//     firstTime = false;
//   }
// }

void read_info(string filename) {
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
    if(stemp.size() == 0) continue;
    else if(stemp.size() == 2) {
      lumi = stod(stemp[1]);
    } else if(stemp.size() == 4) {
      if(plots.find(stemp[1]) == plots.end()) { 
	Plot* tmpplot = new Plot;
	tmpplot->input.push_back(stemp[0]);
	tmpplot->output = stemp[1];
	tmpplot->xsec.push_back(stod(stemp[2]));
	tmpplot->skim.push_back(stod(stemp[3]));
	tmpplot->use = shouldAdd(stemp[0], stemp[1]);
	tmpplot->CumulativeEfficiency.push_back(1.);
	tmpplot->RelativeEffVector.push_back(1.);
	tmpplot->RelativeEffErrorVector.push_back(1.);
	tmpplot->scaleFactor.push_back(1.);
	tmpplot->scaleFactorError.push_back(0.);
	tmpplot->events.push_back(0);
	plots[stemp[1]] = tmpplot;
      } else {
	plots[stemp[1]]->input.push_back(stemp[0]);
	plots[stemp[1]]->xsec.push_back(stod(stemp[2]));
	plots[stemp[1]]->skim.push_back(stod(stemp[3]));
	plots[stemp[1]]->use += shouldAdd(stemp[0], stemp[1]);
	plots[stemp[1]]->CumulativeEfficiency.push_back(1.);
	plots[stemp[1]]->RelativeEffVector.push_back(1.);
	plots[stemp[1]]->RelativeEffErrorVector.push_back(1.);
	plots[stemp[1]]->scaleFactor.push_back(1.);
	plots[stemp[1]]->scaleFactorError.push_back(0.);
	plots[stemp[1]]->events.push_back(0);
      }
    }
  }
  info_file.close();

}

bool shouldAdd(string infile, string globalFile) {
  struct stat buffer;
  if(stat(infile.c_str(), &buffer) != 0) return false;
  ////need to change so doesn't make plot if fatal error (eg file doesn't exist!
  else if(stat(globalFile.c_str(), &buffer) != 0) return true;
  else if(getModTime(globalFile.c_str()) > getModTime(infile.c_str())) return false;
  else return true;

}

int getModTime(const char *path) {
  struct stat attr;
  stat(path, &attr);
  char date[100] = {0};
  strftime(date, 100, "%s", localtime(&attr.st_mtime));
  return atoi(date);

}


// void calculateEfficienciesAndErrors() {

//   float numerator = 1.0;
//   float denominator = 1.0;
//   float theRelativeEfficiency = 1.0;
//   float efferror = 0.0;
//   float cumulativeEfficiency = 1.0;
//   float efferror2 = 0.0;

//   // Calculate relative and cumulative cut efficiencies
//   for (int i=0;i<theCumulativeEfficiency.size(); i++) {
//     numerator = rounder(MaxEventsAnalyzed * theCumulativeEfficiency.at(i));
//     denominator = (i > 0) ? rounder(MaxEventsAnalyzed * theCumulativeEfficiency.at(i-1)) : rounder(MaxEventsAnalyzed);

//     if(numerator > 0) {
//       theRelativeEfficiency = numerator / denominator;
//       efferror = sqrt(theRelativeEfficiency*(1.-theRelativeEfficiency)/denominator);
//     } else {
//       theRelativeEfficiency = 0.0;
//       efferror = (denominator != 0.0) ? 1.0 / denominator : 0.0;
//     }

//     /* binomial uncertainties do not work when the efficiency gets close to 1 or 0 (e.g. efficiency cannot
//        be 99 +- 2 because the efficiency cannot be e.g. 101) ... in these cases, use bayesian */
//     if(((theRelativeEfficiency + efferror) > 1.) || ((theRelativeEfficiency - efferror) < 0.)){
//       TH1F theNumHisto("theNumHisto","theNumHisto",1,0,1);
//       theNumHisto.SetBinContent(1,numerator);
//       theNumHisto.Sumw2();
//       TH1F theDenHisto("theDenHisto","",1,0,1);
//       theDenHisto.SetBinContent(1,denominator);
//       theDenHisto.Sumw2();
//       TGraphAsymmErrors bayesEff;
//       bayesEff.BayesDivide(&theNumHisto, &theDenHisto,"b");
//       efferror = (bayesEff.GetErrorYhigh(0) > bayesEff.GetErrorYlow(0)) ? bayesEff.GetErrorYhigh(0) : bayesEff.GetErrorYlow(0);
//     }

//     if(theRelativeEfficiency == 1.0) efferror = 0;

//     // recalculate efficiencies and errors incorporating scale factors
//     theRelativeEfficiency *= inScaleFactor.at(i);
//     if(numerator > 0) {
//       efferror = sqrt(pow(efferror*inScaleFactor.at(i),2.0) + pow(inScaleFactorError.at(i)*theRelativeEfficiency,2.0));
//     } else {
//       efferror = (denominator != 0) ? inScaleFactor.at(i) / denominator : 0.0;
//     }

//     numerator = rounder(MaxEventsAnalyzed * theCumulativeEfficiency.at(i));
//     denominator = rounder(MaxEventsAnalyzed);

//     if(numerator > 0) {
//       cumulativeEfficiency = numerator / denominator;
//       efferror2 = sqrt(cumulativeEfficiency*(1.-cumulativeEfficiency)/denominator);
//     } else {
//       cumulativeEfficiency = 0.0;
//       efferror2 = (denominator != 0.0) ? 1.0 / denominator : 0.0;
//     }

//     /* binomial uncertainties do not work when the efficiency gets close to 1 or 0 (e.g. efficiency cannot
//        be 99 +- 2 because the efficiency cannot be e.g. 101) ... in these cases, use bayesian */
//     if(((cumulativeEfficiency + efferror2) > 1.) || ((cumulativeEfficiency - efferror2) < 0.)){
//       TH1F theNumHisto("theNumHisto","theNumHisto",1,0,1);
//       theNumHisto.SetBinContent(1,numerator);
//       theNumHisto.Sumw2();
//       TH1F theDenHisto("theDenHisto","",1,0,1);
//       theDenHisto.SetBinContent(1,denominator);
//       theDenHisto.Sumw2();
//       TGraphAsymmErrors bayesEff;
//       bayesEff.BayesDivide(&theNumHisto,&theDenHisto,"b");
//       efferror2 = (bayesEff.GetErrorYhigh(0) > bayesEff.GetErrorYlow(0)) ? bayesEff.GetErrorYhigh(0) : bayesEff.GetErrorYlow(0);
//     }

//     if(cumulativeEfficiency == 1.0) efferror2 = 0;
//     // recalculate efficiencies and errors incorporating scale factors
//     for(int numberOfSFs = 0; numberOfSFs <= i; numberOfSFs++) {
//       if(numerator > 0) {
// 	efferror2 = sqrt(pow(efferror2*inScaleFactor.at(numberOfSFs),2.0) + pow(inScaleFactorError.at(numberOfSFs)*cumulativeEfficiency,2.0));
// 	cumulativeEfficiency *= inScaleFactor.at(numberOfSFs);
//       } else {
// 	cumulativeEfficiency *= (denominator != 0.) ? inScaleFactor.at(numberOfSFs) : 0.;
// 	efferror2 = (denominator != 0.) ? inScaleFactor.at(numberOfSFs) / denominator : 0.0;
//       }
//     }
      
//     RelativeEffVector.push_back(theRelativeEfficiency);
//     RelativeEffErrorVector.push_back(efferror);
//     TotalEffVector.push_back(cumulativeEfficiency);
//     TotalEffErrorVector.push_back(efferror2);
//   }
// }

inline int rounder(double val) {
  return (int)(val + 0.5);
}







// //---function called once just before obtaining user inputs. It clears the vectors.
// void NormalizeHistos() {

//   for(int i=0; i<nHistos; i++) {
//     string histname = HistList.at(i)->GetName();
//     string histoEffyName = "hEffy_" + inProcess.at(0);
//     TH1F *h = (TH1F*)HistList.at(i);
//     h->Sumw2();
//     ////add data conditions
//     if(h->Integral() > 0) h->Scale(RelativeEffVector.at(0) / theEventsPassing.at(0) * x_section.at(0) * lumi * effSkim);
//     for(int ibin=0; ibin< (h->GetXaxis()->GetNbins() + 2); ibin++) {
//       h->SetBinError(ibin, sqrt(pow(h->GetBinError(ibin),2.0) + h->GetBinContent(ibin)) ); // propagating MC statistical uncertainty & poisson uncertainty on the r$
//     }
//   }

//   h->SetName(histname.c_str());
//   string YtitleString = "N / " + lumi_string + " pb^{-1}";
//   TFile *hfile = (TFile*)gROOT->FindObject(outputRootFileForNormalizedHistos.c_str());
//   if (hfile) {hfile->Close();}
//   hfile = new TFile(outputRootFileForNormalizedHistos.c_str(),"UPDATE");
//   h->Write();
//   hfile->Close();
//   delete h;


//   for(int j=1;j<inRootFiles.size();j++) {
//     string hist2name = HistList.at(i+(j*nHistos))->GetName();
//     if(theHistNameStrings.at(i) != theHistNameStrings.at(i+(j*nHistos))) continue;

//     string histoEffyName2 = "hhEffy_" + inProcess.at(j);
//     TH1F *hh = (TH1F*)HistList.at(i+(j*nHistos));
//     hh->Sumw2();
//     if(IsData == "0") {
//       if(hh->Integral(0,(hh->GetXaxis()->GetNbins()+1)) > 0) {hh->Scale(1.0 / theEventsPassing.at(j) * x_section.at(j) * lumi * effSkim);}
//       for(int jfile=0;jfile<=j;jfile++) {
// 	hh->Scale(
// 	for(int ibin=0; ibin<=(hhEffy->GetXaxis()->GetNbins() + 1); ibin++) {
// 	  hhEffy->SetBinContent(ibin,RelativeEffVector.at(jfile));
// 	  hhEffy->SetBinError(ibin,0);
// 	}
// 	hh->Multiply(hh,hhEffy,1,1,"");
// 	delete hhEffy;
//       }
//       for(int ibin=0; ibin<=(hh->GetXaxis()->GetNbins() + 1); ibin++) {
// 	hh->SetBinError(ibin, sqrt(pow(hh->GetBinError(ibin),2.0) + hh->GetBinContent(ibin)) );
//       }
//     }
//     hh->SetName(hist2name.c_str());
//     string YtitleString = "N / " + lumi_string + " pb^{-1}";
//     TFile *hfile = (TFile*)gROOT->FindObject(outputRootFileForNormalizedHistos.c_str());
//     if (hfile) {hfile->Close();}
//     hfile = new TFile(outputRootFileForNormalizedHistos.c_str(),"UPDATE");
//     hh->Write();
//     hfile->Close();
//     delete hh;

//   }

// }

// void CreateProbHistos() {

//   if(( nHistList2 % inRootFiles.size() != 0 )) return;

//   for(int i=0; i<nHistos; i++) {
//     string histname = HistList2.at(i)->GetName();
//     TH1F *h = (TH1F*)HistList2.at(i);
//     h->Sumw2();
//     if(h->Integral(0,(h->GetXaxis()->GetNbins()+1)) > 0) {h->Scale(1.0 / h->Integral(0,(h->GetXaxis()->GetNbins()+1)));}
//     h->SetName(histname.c_str());
//     h->GetYaxis()->SetTitle("a.u.");
//     h->GetYaxis()->SetTitleSize(0.06);
//     h->GetYaxis()->SetTitleFont(62);
//     h->GetYaxis()->CenterTitle();
//     h->GetYaxis()->SetLabelSize(0.05);
//     h->GetYaxis()->SetLabelFont(62);
//     h->GetXaxis()->SetTitle(theHistNameStrings.at(i).c_str());
//     h->GetXaxis()->SetTitleSize(0.06);
//     h->GetXaxis()->SetTitleFont(62);
//     h->GetXaxis()->CenterTitle();
//     h->GetXaxis()->SetLabelSize(0.05);
//     h->GetXaxis()->SetLabelFont(62);
//     TFile *hfile = (TFile*)gROOT->FindObject(outputRootFileForProbabilityHistos.c_str());
//     if (hfile) {hfile->Close();}
//     hfile = new TFile(outputRootFileForProbabilityHistos.c_str(),"UPDATE");
//     h->Write();
//     hfile->Close();
//     delete h;
//     if(inRootFiles.size() <= 1) continue;

//     for(int j=1;j<inRootFiles.size();j++) {
//       string hist2name = HistList2.at(i+(j*nHistos))->GetName();
//       if(theHistNameStrings.at(i) != theHistNameStrings.at(i+(j*nHistos))) continue;

//       TH1F *hh = (TH1F*)HistList2.at(i+(j*nHistos));
//       hh->Sumw2();
//       if(hh->Integral(0,(hh->GetXaxis()->GetNbins()+1)) > 0) hh->Scale(1.0 / hh->Integral(0,(hh->GetXaxis()->GetNbins()+1)));
//       hh->SetName(hist2name.c_str());
//       hh->GetYaxis()->SetTitle("a.u.");
//       hh->GetYaxis()->SetTitleSize(0.06);
//       hh->GetYaxis()->SetTitleFont(62);
//       hh->GetYaxis()->CenterTitle();
//       hh->GetYaxis()->SetLabelSize(0.05);
//       hh->GetYaxis()->SetLabelFont(62);
//       hh->GetXaxis()->SetTitle(theHistNameStrings.at(i+(j*nHistos)).c_str());
//       hh->GetXaxis()->SetTitleSize(0.06);
//       hh->GetXaxis()->SetTitleFont(62);
//       hh->GetXaxis()->CenterTitle();
//       hh->GetXaxis()->SetLabelSize(0.05);
//       hh->GetXaxis()->SetLabelFont(62);
//       TFile *hfile = (TFile*)gROOT->FindObject(outputRootFileForProbabilityHistos.c_str());
//       if (hfile) {hfile->Close();}
//       hfile = new TFile(outputRootFileForProbabilityHistos.c_str(),"UPDATE");
//       hh->Write();
//       hfile->Close();
//       delete hh;
//     }
//   }
// }




// void createLogFile(string outputLogFile) {

//   // create an output log file that will contain the cut flow efficiency table
//   ofstream outFile;
//   outFile.open(outputLogFile.c_str(), ios::out);

//   // if output log file cannot be opened, exit the code
//   if (!outFile) {
//     cerr << "Can't open output file " << outputLogFile << endl;
//     exit(1);
//   } else  outFile << endl << "The following input was used: " << endl << endl << endl;


//   // loop over root files
//   for(int j=0;j<inRootFiles.size();j++) {
//     // printout information to log file
//     outFile << "Name of Root File #" << (j+1) << " : " << inRootFiles.at(j) << endl;
//     outFile << "Name of Directory #" << (j+1) << " : " << inDirectories.at(j) << endl;
//     outFile << "Name of Process #" << (j+1) << "   : " << inProcess.at(j) << endl << endl;
//   }

//   //  outFile << "Detailed Efficiencies " << "\n";
//   outFile << "Table of Efficiencies\n\n";
//   outFile << "Efficiencies below are calculated with respect to the number of events after skimming\n\n";
//   outFile << "-------------------------------------------------------------------------------------------------------------------------------\n";
//   outFile << "         Name                         Events               Relative (%)                     Cumulative (%)\n";
//   outFile << "-------------------------------------------------------------------------------------------------------------------------------\n";

//   if(theCumulativeEfficiency.size() <= 0) return;
//   float effy = 1;
//   float deffyOvereffy = 0;
//   for (int i=0;i<theCumulativeEfficiency.size(); i++) {
//     effy = effy * RelativeEffVector.at(i);
//     deffyOvereffy = sqrt(pow(RelativeEffErrorVector.at(i)/RelativeEffVector.at(i),2.0) + pow(deffyOvereffy,2.0));
//     // output for cut flow efficiency table - efficiencies and errors
//     outFile     <<setw(24)<<inProcess.at(i)
// 		<<setw(20)<<round(MaxEventsAnalyzed * theCumulativeEfficiency.at(i))
// 		<<setw(15)<<setprecision(4)<<RelativeEffVector.at(i)*100.0<<setw(4)<<" +- "<<setw(10)<<setprecision(4)<<(RelativeEffErrorVector.at(i)*100.0)
// 		<<setw(20)<<setprecision(4)<<TotalEffVector.at(i)*100.0<<setw(4)<<" +- "<<setw(10)<<setprecision(4)<<(TotalEffErrorVector.at(i) * 100.0)
// 		<<endl;
//   }

//   outFile << "-------------------------------------------------------------------------------------------------------------------------------\n";

//   outFile.close();

// }

// void tester() {
//   TFile* finalfile = new TFile(final.c_str(), "RECREATE");
  

// }



void MergeRootfile( TDirectory *target, Plot& plotter ) {

  TList* sourcelist = plotter.FileList;

  TString path( (char*)strstr( target->GetPath(), ":" ) );
  path.Remove( 0, 2 );

  TFile *first_source = (TFile*)sourcelist->First();
  first_source->cd( path );
  TDirectory *current_sourcedir = gDirectory;
  //gain time, do not add the objects in the list in memory
  Bool_t status = TH1::AddDirectoryStatus();
  TH1::AddDirectory(kFALSE);


  ///try to find events to calculate efficiency
  TH1F* events;
  current_sourcedir->GetObject("Events", events);

  if(events) {
    int nplot = 0;

    plotter.events.at(nplot) = events->GetBinContent(1);
    double cumulEff = events->GetBinContent(2)/ events->GetBinContent(1);
    calculateEfficienciesAndErrors(nplot, cumulEff, plotter);

    TFile *nextsource = (TFile*)sourcelist->After( first_source );
    while( nextsource) {
      nplot++;
      nextsource->cd(path);
      gDirectory->GetObject("Events", events);
      plotter.events.at(nplot) = events->GetBinContent(1);
      double cumulEff = events->GetBinContent(2)/ events->GetBinContent(1);
      calculateEfficienciesAndErrors(nplot, cumulEff, plotter);
      nextsource = (TFile*)sourcelist->After( nextsource );
    }
  }
  delete events;

  // loop over all keys in this directory
  TChain *globChain = 0;
  TIter nextkey( current_sourcedir->GetListOfKeys() );
  TKey *key, *oldkey=0;
  while ( (key = (TKey*)nextkey())) {

    //keep only the highest cycle number for each key
    if (oldkey && !strcmp(oldkey->GetName(),key->GetName())) continue;

    first_source->cd( path );

    TObject *obj = key->ReadObj();
    if ( obj->IsA()->InheritsFrom( TH1::Class() ) ) {
      TH1 *h1 = (TH1*)obj;
      int spot = 0;
      h1->Scale(plotter.RelativeEffVector.at(spot)/plotter.events.at(spot)* plotter.xsec.at(spot)* lumi* plotter.skim.at(spot));

      TFile *nextsource = (TFile*)sourcelist->After( first_source );
      
      while ( nextsource ) {
	spot++;
	nextsource->cd( path );
	TKey *key2 = (TKey*)gDirectory->GetListOfKeys()->FindObject(h1->GetName());
	if (key2) {
	  TH1 *h2 = (TH1*)key2->ReadObj();
	  h1->Add( h2, plotter.RelativeEffVector.at(spot)/plotter.events.at(spot)* plotter.xsec.at(spot)* lumi* plotter.skim.at(spot));
	  delete h2;
	  
	}
	nextsource = (TFile*)sourcelist->After( nextsource );
      }
      for(int ibin=0; ibin < (h1->GetXaxis()->GetNbins() + 1); ibin++) {
	h1->SetBinError(ibin, sqrt(pow(h1->GetBinError(ibin),2.0) + h1->GetBinContent(ibin)) );
      }

    }
    else if ( obj->IsA()->InheritsFrom( TTree::Class() ) ) {

      // loop over all source files create a chain of Trees "globChain"
      const char* obj_name= obj->GetName();

      globChain = new TChain(obj_name);
      globChain->Add(first_source->GetName());
      TFile *nextsource = (TFile*)sourcelist->After( first_source );
      //      const char* file_name = nextsource->GetName();
      // cout << "file name  " << file_name << endl;
      while ( nextsource ) {
	  
	globChain->Add(nextsource->GetName());
	nextsource = (TFile*)sourcelist->After( nextsource );
      }
	
    } else if ( obj->IsA()->InheritsFrom( TDirectory::Class() ) ) {
      // it's a subdirectory

      cout << "Found subdirectory " << obj->GetName() << endl;

      // create a new subdir of same name and title in the target file
      target->cd();
      TDirectory *newdir = target->mkdir( obj->GetName(), obj->GetTitle() );

      // newdir is now the starting point of another round of merging
      // newdir still knows its depth within the target file via
      // GetPath(), so we can still figure out where we are in the recursion
      MergeRootfile( newdir, plotter );

    } else {

      // object is of no type that we know or can handle
      cout << "Unknown object type, name: "
	   << obj->GetName() << " title: " << obj->GetTitle() << endl;
    }

    // now write the merged histogram (which is "in" obj) to the target file
    // note that this will just store obj in the current directory level,
    // which is not persistent until the complete directory itself is stored
    // by "target->Write()" below
    if ( obj ) {
      target->cd();
      if(obj->IsA()->InheritsFrom( TTree::Class() ))
	globChain->Merge(target->GetFile(),0,"keep");
      else
	obj->Write( key->GetName() );
    }

  } // while ( ( TKey *key = (TKey*)nextkey() ) )

  // save modifications to target file
  target->SaveSelf(kTRUE);
  TH1::AddDirectory(status);
}

 

void calculateEfficienciesAndErrors(int i, double cumulEff, Plot& plot) {

  double numerator;
  double denominator;
  double relEff;
  double efferror;


  // Calculate relative and cumulative cut efficiencies
  numerator = rounder(plot.events.at(i) * cumulEff);
  denominator = rounder(plot.events.at(i) * plot.CumulativeEfficiency.at(i)); ///set cumul to 1 initially

  if(numerator > 0) {
    relEff = numerator / denominator;
    efferror = sqrt(relEff*(1.-relEff)/denominator);
  } else {
    relEff = 0.0;
    efferror = (denominator != 0.0) ? 1.0 / denominator : 0.0;
  }

  /* binomial uncertainties do not work when the efficiency gets close to 1 or 0 (e.g. efficiency cannot
     be 99 +- 2 because the efficiency cannot be e.g. 101) ... in these cases, use bayesian */
  if(((relEff + efferror) > 1.) || ((relEff - efferror) < 0.)){
    TH1F theNumHisto("theNumHisto","theNumHisto",1,0,1);
    theNumHisto.SetBinContent(1,numerator);
    theNumHisto.Sumw2();
    TH1F theDenHisto("theDenHisto","",1,0,1);
    theDenHisto.SetBinContent(1,denominator);
    theDenHisto.Sumw2();
    TGraphAsymmErrors bayesEff;
    bayesEff.BayesDivide(&theNumHisto, &theDenHisto,"b");
    efferror = (bayesEff.GetErrorYhigh(0) > bayesEff.GetErrorYlow(0)) ? bayesEff.GetErrorYhigh(0) : bayesEff.GetErrorYlow(0);
  }
  if(relEff == 1.0) efferror = 0;

  //inscalefactors???

  // recalculate efficiencies and errors incorporating scale factors
  relEff *= plot.scaleFactor.at(i);
  if(numerator > 0) {
    efferror = sqrt(pow(efferror*plot.scaleFactor.at(i),2.0) + pow(plot.scaleFactorError.at(i)*relEff,2.0));
  } else {
    efferror = (denominator != 0) ? plot.scaleFactor.at(i) / denominator : 0.0;
  }

  plot.RelativeEffVector.at(i) = relEff;
  plot.RelativeEffErrorVector.at(i) = efferror;
  plot.CumulativeEfficiency.at(i) = cumulEff;

}
