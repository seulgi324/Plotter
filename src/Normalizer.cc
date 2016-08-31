#include "Normalizer.h"

using namespace std;

Normer::Normer() {
  
}

Normer::Normer(const Normer& other) {
  input = other.input;
  skim = other.skim;
  xsec = other.xsec;
  output = other.output;
  type = other.type;
  lumi = other.lumi;
  integral = other.integral;
  CumulativeEfficiency = other.CumulativeEfficiency;
  scaleFactor = other.scaleFactor;
  scaleFactorError= other.scaleFactorError;
  use = other.use;
  isData = other.isData;
  FileList = new TList();

  
  TObject* object = other.FileList->First();
  while(object) {
    FileList->Add(new TObject(*object));
    object = FileList->After(object);  
  }
}

Normer& Normer::operator=(const Normer& rhs) {

}

Normer::~Normer() {
  TFile* source = (TFile*)FileList->First();
  while(source) {
    source->Close();
    source = (TFile*)FileList->After(source);  
  }
}




void Normer::print() {
  cout << " =========== " << output << " =========== " << endl;
  for(int i = 0; i < input.size(); ++i) {
    cout << input.at(i) << endl;
  }
  cout << endl;
}




void Normer::MergeRootfile( TDirectory *target) {

  TList* sourcelist = FileList;
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

    integral.at(nplot) = events->GetBinContent(2);
    CumulativeEfficiency.at(nplot) = events->GetBinContent(2)/ events->GetBinContent(1);

    TFile *nextsource = (TFile*)sourcelist->After( first_source );
    while( nextsource) {
      nplot++;
      nextsource->cd(path);
      gDirectory->GetObject("Events", events);
      integral.at(nplot) = events->GetBinContent(2);
      CumulativeEfficiency.at(nplot) = events->GetBinContent(2)/ events->GetBinContent(1);

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
      h1->Sumw2();
      int spot = 0;
      if(!isData) h1->Scale(CumulativeEfficiency.at(spot)/integral.at(spot) * xsec.at(spot)* lumi* skim.at(spot));

      TFile *nextsource = (TFile*)sourcelist->After( first_source );
      
      while ( nextsource ) {
	spot++;
	nextsource->cd( path );
	TKey *key2 = (TKey*)gDirectory->GetListOfKeys()->FindObject(h1->GetName());
	if (key2) {
	  TH1 *h2 = (TH1*)key2->ReadObj();
	  h2->Sumw2();
	  double scale = (isData) ? 1.0 : CumulativeEfficiency.at(spot)/integral.at(spot) * xsec.at(spot)* lumi* skim.at(spot);
	  h1->Add( h2, scale);
	  delete h2;
	  
	}
	nextsource = (TFile*)sourcelist->After( nextsource );
      }
      ////////////////////////////////////////////////////////////
      ////  To gain back Poisson error, uncomment this line /////
      ////////////////////////////////////////////////////////////

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

      //      cout << "Found subdirectory " << obj->GetName() << endl;

      // create a new subdir of same name and title in the target file
      target->cd();
      TDirectory *newdir = target->mkdir( obj->GetName(), obj->GetTitle() );

      // newdir is now the starting point of another round of merging
      // newdir still knows its depth within the target file via
      // GetPath(), so we can still figure out where we are in the recursion
      MergeRootfile( newdir );

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

