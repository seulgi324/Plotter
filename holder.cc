
      int spot = 0;
      
      TLegend *legend = new TLegend(0.6082412,0.4248252,0.8208543,0.7867133,NULL,"brNDC");
      //     legend->AddEntry(h,inProcess.at(0).c_str(), "lep");

      TH1D* error = new TH1D("error", "error", h1->GetXaxis()->GetNbins(), h1->GetXaxis()->GetXmin(), h1->GetXaxis()->GetXmax());
      error->Add(h1);
      for(int i = 1; i < h1->GetXaxis()->GetNbins()+1; i++) {
	h1->SetBinError(i, 0);
      }
      //      h1->SetFillColor((int)plotter.xsec.at(spot));
      h1->SetLineColor(1);
      h1->SetFillStyle(1001);
      string lname = plotter.input.at(spot);
      lname.erase(lname.end()-5, lname.end());
       
      legend->AddEntry(h1,lname.c_str(), "f");
      THStack *hs = new THStack(h1->GetName(),h1->GetName());

      hs->Adpd(h1);

      TFile *nextsource = (TFile*)sourcelist->After( first_source );
      
      while ( nextsource ) {
	spot++;
	nextsource->cd( path );
	TKey *key2 = (TKey*)gDirectory->GetListOfKeys()->FindObject(h1->GetName());
	if (key2) {
	  TH1 *h2 = (TH1*)key2->ReadObj();
	  error->Add(h2);
	  for(int i = 1; i < h2->GetXaxis()->GetNbins()+1; i++) {
	    h2->SetBinError(i, 0);
	  }
	  //  h2->SetFillColor(plotter.xsec.at(spot));
	  h2->SetLineColor(1);
	  h2->SetFillStyle(1001);
	  lname = plotter.input.at(spot);
	  lname.erase(lname.end()-5, lname.end());
	  legend->AddEntry(h2,lname.c_str(), "f");

	  hs->Add(h2);
	  //	  delete h2;
	}
	nextsource = (TFile*)sourcelist->After( nextsource );
      }
      ////////////////////////////////////////////////////////////
      ////  To gain back Poisson error, uncomment this line /////
      ////////////////////////////////////////////////////////////

      // for(int ibin=0; ibin < (h1->GetXaxis()->GetNbins() + 1); ibin++) {
      // 	h1->SetBinError(ibin, sqrt(pow(h1->GetBinError(ibin),2.0) + h1->GetBinContent(ibin)) );
      // }
      target->cd();
      TCanvas *c = new TCanvas(hs->GetName(), hs->GetName(), 500, 500);//403,50,600,600);
      //      c->Range(0,0,1,1);
      // c->SetFillColor(0);
      // c->SetBorderMode(0);
      // c->SetBorderSize(3);
      // c->SetTickx(1);
      // c->SetTicky(1);
      // c->SetLeftMargin(0.15);
      // c->SetRightMargin(0.05);
      // c->SetTopMargin(0.05);
      // c->SetBottomMargin(0.15);
      // c->SetFrameFillStyle(0);
      // c->SetFrameBorderMode(0);
      c->Divide(1,2, 0, 0);
      c->GetPad(1)->SetRightMargin(0.01);
      c->cd(2);
      // TPad *bpad = new TPad("c1_1", "newpad",0.01,0,0.99,0.32);
      // bpad->SetBottomMargin(0);
      // bpad->SetGridx();
      // bpad->Draw();
      // bpad->cd();

      // c1_1->Draw();
      // c1_1->cd();
      // //      c1_1->Range(-200,-2.720435,1133.333,3.047681);
      // c1_1->SetFillColor(0);
      // c1_1->SetFillStyle(4000);
      // c1_1->SetBorderMode(0);
      // c1_1->SetBorderSize(2);
      // c1_1->SetTickx(1);
      // c1_1->SetTicky(1);
      // c1_1->SetLeftMargin(0.15);
      // c1_1->SetTopMargin(0.01);
      // c1_1->SetBottomMargin(0.3);
      // c1_1->SetFrameFillStyle(0);
      // c1_1->SetFrameBorderMode(0);
      // c1_1->SetFrameFillStyle(0);
      // c1_1->SetFrameBorderMode(0);
      
      error->Scale(1/error->Integral());
      error->Draw("ep");
      // error->GetXaxis()->SetLabelFont(42);
      // error->GetXaxis()->SetLabelOffset(0.007);
      // error->GetXaxis()->SetLabelSize(0.11);
      // error->GetXaxis()->SetTitleSize(0.12);
      // error->GetXaxis()->SetTitleOffset(0.9);
      // error->GetXaxis()->SetTitleFont(42);
      // error->GetYaxis()->SetTitle("#frac{Data}{MC}");
      // error->GetYaxis()->SetNdivisions(505);
      // error->GetYaxis()->SetLabelFont(42);
      // error->GetYaxis()->SetLabelOffset(0.012);
      // error->GetYaxis()->SetLabelSize(0.11);
      // error->GetYaxis()->SetTitleSize(0.09);
      // error->GetYaxis()->SetTitleOffset(0.7);
      // error->GetYaxis()->SetTitleFont(42);
//      error->SetMinimum(0.0);
//      error->SetMaximum(2.99);
      // error->SetLineStyle(0);
      // error->SetLineColor(1);
      // error->SetMarkerStyle(20);
//      error->SetMarkerSize(0.8);

      // TF1 *PrevFitTMP = new TF1("PrevFitTMP","pol0",-10000,10000);
      // PrevFitTMP->SetFillColor(19);
      // PrevFitTMP->SetFillStyle(0);
      // PrevFitTMP->SetMarkerStyle(20);
      // PrevFitTMP->SetLineColor(2);
      // PrevFitTMP->SetLineWidth(1);
      // PrevFitTMP->GetXaxis()->SetLabelFont(42);
      // PrevFitTMP->GetXaxis()->SetLabelOffset(0.007);
      // PrevFitTMP->GetXaxis()->SetLabelSize(0.05);
      // PrevFitTMP->GetXaxis()->SetTitleSize(0.06);
      // PrevFitTMP->GetXaxis()->SetTitleOffset(1.1);
      // PrevFitTMP->GetXaxis()->SetTitleFont(42);
      // PrevFitTMP->GetYaxis()->SetLabelFont(42);
      // PrevFitTMP->GetYaxis()->SetLabelOffset(0.007);
      // PrevFitTMP->GetYaxis()->SetLabelSize(0.05);
      // PrevFitTMP->GetYaxis()->SetTitleSize(0.06);
      // PrevFitTMP->GetYaxis()->SetTitleOffset(1.15);
      // PrevFitTMP->GetYaxis()->SetTitleFont(42);
      // PrevFitTMP->SetParameter(0,1.0);
      // PrevFitTMP->SetParError(0,0);
      // PrevFitTMP->SetParLimits(0,0,0);

      // error->GetListOfFunctions()->Add(PrevFitTMP);
      
      //      error->Draw("e1p");


      Double_t mcX2[5000];
      Double_t mcY2[5000];
      Double_t mcErrorX2[5000];
      Double_t mcErrorY2[5000];
      for(int bin=0; bin < error->GetXaxis()->GetNbins(); bin++) {
        mcY2[bin] = 1;
        mcErrorY2[bin] = error->GetBinError(bin+1) / error->GetBinContent(bin+1);
        mcX2[bin] = error->GetBinCenter(bin+1);
        mcErrorX2[bin] = error->GetBinWidth(bin+1) * 0.5;
      }
      TGraphErrors *mcError2 = new TGraphErrors(error->GetXaxis()->GetNbins(),mcX2,mcY2,mcErrorX2,mcErrorY2);
      mcError2->SetLineWidth(0);
      mcError2->SetFillColor(1);
      mcError2->SetFillStyle(3002);
      mcError2->Draw("sameE2");
      
      c->cd(1);
      
      // c1_1->Paint();
      // c1_1->Modified();
      //      c->cd();
      //      c->Update();
      /*-------------------------------------*/

      // TPad *c1_2 = new TPad("c1_2", "newpad",0.01,0.33,0.99,0.99);

      // c1_2->cd();
      // c1_2->Range(-200,-13.87376,1133.333,1389.866);
      // c1_2->SetFillColor(0);
      // c1_2->SetBorderMode(0);
      // c1_2->SetBorderSize(2);
      // c1_2->SetTickx(1);
      // c1_2->SetTicky(1);
      // c1_2->SetLeftMargin(0.15);
      // c1_2->SetBottomMargin(0.01);
      // c1_2->SetFrameFillStyle(0);
      // c1_2->SetFrameBorderMode(0);
      // c1_2->SetFrameFillStyle(0);
      // c1_2->SetFrameBorderMode(0);

      // data->SetMarkerStyle(20);
      // data->SetLineColor(1);
      // data->GetXaxis()->SetLabelFont(42);
      // data->GetXaxis()->SetLabelOffset(0.007);
      // data->GetXaxis()->SetLabelSize(0.11);
      // data->GetXaxis()->SetTitleSize(0.12);
      // data->GetXaxis()->SetTitleOffset(0.9);
      // data->GetXaxis()->SetTitleFont(42);

      // data->GetYaxis()->SetTitle("Events");
      // data->GetYaxis()->SetLabelFont(42);
      // data->GetYaxis()->SetLabelOffset(0.007);
      // data->GetYaxis()->SetLabelSize(0.05);
      // data->GetYaxis()->SetTitleSize(0.06);
      // data->GetYaxis()->SetTitleOffset(1.1);
      // data->GetYaxis()->SetTitleFont(42);
      // data->GetZaxis()->SetLabelFont(42);
      // data->GetZaxis()->SetLabelOffset(0.007);
      // data->GetZaxis()->SetLabelSize(0.05);
      // data->GetZaxis()->SetTitleSize(0.06);
      // data->GetZaxis()->SetTitleFont(42);
      // data->Draw("ep");
      // hs->Draw("");
      // Double_t mcX[5000];
      // Double_t mcY[5000];
      // Double_t mcErrorX[5000];
      // Double_t mcErrorY[5000];

      // for(int bin=0; bin < error->GetXaxis()->GetNbins(); bin++) {
      //   mcY[bin] = error->GetBinContent(bin+1);
      //   mcErrorY[bin] = error->GetBinError(bin+1);
      //   mcX[bin] = error->GetBinCenter(bin+1);
      //   mcErrorX[bin] = error->GetBinWidth(bin+1) * 0.5;
      // }
      // TGraphErrors *mcError = new TGraphErrors(error->GetXaxis()->GetNbins(),mcX,mcY,mcErrorX,mcErrorY);
      // mcError->SetLineWidth(0);
      // mcError->SetFillColor(1);
      // mcError->SetFillStyle(3002);
      // mcError->Draw("sameE2");
      //      delete error;
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

      // // TPaveText *pt = new TPaveText(0.2620189,0.9409833,0.662649,0.9913117,"brNDC");
      // // pt->SetBorderSize(0);
      // // pt->SetFillColor(0);
      // // pt->SetLineColor(0);
      // // pt->SetTextSize(0.05297732);
      // // string TitleString = "CMS Preliminary, L_{int} = " + lumi_string + " fb^{-1}, #sqrt{s} = 13 TeV";
      // // TText *text = pt->AddText(TitleString.c_str());
      // // text->SetTextFont(42);
      // // pt->Draw();

      // //      c1_2->Modified();
      // c1_2->Draw();
      // c->cd();
      // c->Modified();
      // c->cd();
      //      c->SetSelected(c);
      c->Write(c->GetName());
      c->Close();


    }
