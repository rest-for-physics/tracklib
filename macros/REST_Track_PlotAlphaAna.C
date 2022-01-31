//
//
//
#include <TRestRun.h>
#include <TRestTrackAlphaAnalysisProcess.h>
#include <TRestDetectorReadout.h>
#include <TRestTrackEvent.h>
#include <TRestAnalysisTree.h>
#include <TRestDetectorHitsEvent.h>
#include <TRestRawSignalEvent.h>

TPad *DrawPulses ( TRestRawSignalEvent *rawSignalEvent, TRestDetectorReadout *fReadout) {

  TRestDetectorReadoutPlane* plane = &(*fReadout)[0];

  std::vector<Int_t> sXIDs;
  std::vector<Int_t> sYIDs;

  Int_t numberOfSignals = rawSignalEvent->GetNumberOfSignals();
  Int_t readoutChannel = -1, readoutModule, planeID=0;
  int maxX=0, maxY=0;
    for (int i = 0; i < numberOfSignals; i++) {
      TRestRawSignal* sgnl = rawSignalEvent->GetSignal(i);
      Int_t signalID = sgnl->GetSignalID();
      fReadout->GetPlaneModuleChannel(signalID, planeID, readoutModule, readoutChannel);
      Double_t x = plane->GetX(readoutModule, readoutChannel);
      Double_t y = plane->GetY(readoutModule, readoutChannel);
        if (TMath::IsNaN(x)) {
          sYIDs.emplace_back(signalID);
        } else if (TMath::IsNaN(y)) {
          sXIDs.emplace_back(signalID);
        }
    }

  TPad *fPad = new TPad("RawSignalEvent", " ", 0, 0, 1, 1);
  fPad->Divide(2);
  fPad->Draw();
  fPad->cd(1);
  rawSignalEvent->DrawSignals(sXIDs);
  fPad->Update();
  fPad->cd(2);
  rawSignalEvent->DrawSignals(sYIDs);
  fPad->Update();

 return fPad;

}

void UpdateRate(const double& currentTimeEv, double& oldTime, const int& currentEventCount, int& oldEventCount, int& rateGraphCounter, TGraphErrors* rateGraph, bool lastPoint = false) {

  if(!lastPoint)
    if ( currentTimeEv - oldTime < 3600 ) return;

  double instantRate = (currentEventCount - oldEventCount) / (currentTimeEv - oldTime);
  double instantRateError = TMath::Sqrt(currentEventCount - oldEventCount) / (currentTimeEv - oldTime);
  double rootTime = 2871763200 + (currentTimeEv + oldTime) / 2.;
  
  rateGraph->SetPoint(rateGraphCounter, rootTime, instantRate);
  
  rateGraph->SetPointError (rateGraphCounter, (currentTimeEv - oldTime) / 2.,instantRateError);

  oldEventCount = currentEventCount;
  oldTime = currentTimeEv;
  rateGraphCounter++;

}

int REST_Track_PlotAlphaAna(const std::string &fileName){

  TRestRun* run = new TRestRun(fileName);

  TRestTrackAlphaAnalysisProcess *trackAlphaAna = (TRestTrackAlphaAnalysisProcess *)run->GetMetadataClass("TRestTrackAlphaAnalysisProcess");

  if(trackAlphaAna == nullptr){
    std::cout<<"ERROR: No TRestTrackAlphaAnalysisProcess found in "<<fileName<<" please, provide a valid file"<<std::endl;
    return -1;
  }

  TRestAnalysisTree* anaTree = run->GetAnalysisTree();
  int n = anaTree->GetNbranches();

  std::cout<<"Entries "<<anaTree->GetEntries()<<std::endl;

  std::string planeStr ="";

  TRestDetectorReadout *fReadout = (TRestDetectorReadout* ) run->GetMetadataClass("TRestDetectorReadout");

  

  double xmin=0,xmax=0,ymin=0,ymax=0;
    if(fReadout) {
      TRestDetectorReadoutPlane* plane = &(*fReadout)[0];
      plane->GetBoundaries(xmin, xmax, ymin, ymax);
      planeStr = std::to_string((int)std::abs(xmax-xmin))+","+std::to_string(xmin)+","+std::to_string(xmax)+","
      +std::to_string((int)std::abs(ymax-ymin))+","+std::to_string(ymin)+","+std::to_string(ymax);
    }

  std::string cmd;
  TCanvas *spectraCanvas = new TCanvas("spectraCanvas", "spectraCanvas");
  anaTree->Draw("alphaTrackAna_totalEnergy>>sD()","alphaTrackAna_downwards");
  TH1F *sD = (TH1F *)gDirectory->Get("sD");

  anaTree->Draw("alphaTrackAna_totalEnergy>>sU","!alphaTrackAna_downwards");
  TH1F *sU = (TH1F *)gDirectory->Get("sU");
  sU->SetLineColor(kRed);
  sD->Draw();
  sU->Draw("SAME");
  int uTracks = sU->GetEntries();
  int dTracks = sD->GetEntries();
  spectraCanvas->Update();

  TCanvas *originCanvas = new TCanvas("originCanvas", "originCanvas");
  cmd = "alphaTrackAna_originX:alphaTrackAna_originY>>orig("+planeStr +")";
  anaTree->Draw(cmd.c_str(),"","COLZ");
  originCanvas->Update();
  //TH2F *orig = (TH2F*)gDirectory->Get("orig");

  TCanvas *endCanvas = new TCanvas("endCanvas", "endCanvas");
  cmd = "alphaTrackAna_endX:alphaTrackAna_endY>>end("+planeStr +")";
  anaTree->Draw(cmd.c_str(),"","COLZ");
  endCanvas->Update();
  //TH2F *end = (TH2F*)gDirectory->Get("end");

  TCanvas *trackLengthCanvas = new TCanvas("trackLengthCanvas", "trackLengthCanvas");
  anaTree->Draw("alphaTrackAna_totalEnergy:alphaTrackAna_length","","COLZ");
  trackLengthCanvas->Update();

  TCanvas *trackAngleCanvas = new TCanvas("trackAngleCanvas", "trackAngleCanvas");
  anaTree->Draw("alphaTrackAna_angle");
  trackAngleCanvas->Update();

  run->PrintMetadata();
  TRestEvent* trackEvent = REST_Reflection::Assembly("TRestTrackEvent");
  TRestEvent* hitsEvent = REST_Reflection::Assembly("TRestDetectorHitsEvent");
  TRestEvent* rawSignalEvent = REST_Reflection::Assembly("TRestRawSignalEvent");

  run->SetInputEvent(trackEvent);
  run->SetInputEvent(hitsEvent);
  run->SetInputEvent(rawSignalEvent);

  TCanvas *hitmapCanvas = new TCanvas("hitmapCanvas", "hitmapCanvas");
  TCanvas *projectionCanvas = new TCanvas("projectionCanvas", "projectionCanvas");
  TCanvas *signalCanvas = new TCanvas("signalCanvas", "signalCanvas");

  double oldTimeEv = run->GetStartTimestamp();
  int currentEventCount=0,oldEventCount=0,rateGraphCounter=0;
  TGraphErrors* rateGraph = new TGraphErrors();

  bool interactive = true;

    for(int i=0;i<run->GetEntries();i++){
      run->GetEntry(i);

      UpdateRate(rawSignalEvent->GetTime(), oldTimeEv, currentEventCount, oldEventCount, rateGraphCounter, rateGraph);
      currentEventCount++;

      if(!interactive)continue;
      std::cout<<"Event ID: "<<anaTree->GetEventID()<<std::endl;
        std::vector <double> orig;
        std::vector<std::string> oName = {"alphaTrackAna_originX","alphaTrackAna_originY","alphaTrackAna_originZ"}; 
          for(auto n :oName){
            any o = anaTree->GetObservable(anaTree->GetObservableID(n));
            double v;
            o >> v;
            orig.push_back(v);
            std::cout<< n <<": "<<v<<"; ";
          }
        std::cout<<std::endl;
        TVector3 origin = orig.data();

        std::vector <double> en;
        std::vector<std::string> eName = {"alphaTrackAna_endX","alphaTrackAna_endY","alphaTrackAna_endZ"}; 
          for(auto n :eName){
            any o = anaTree->GetObservable(anaTree->GetObservableID(n));
            double v;
            o >> v;
            en.push_back(v);
            std::cout<< n <<": "<<v<<"; ";
          }
        std::cout<<std::endl;
        TVector3 end = en.data();

        TVector3 max,min,nBins;

        hitmapCanvas->cd();
        static_cast<TRestDetectorHitsEvent *>(hitsEvent)->GetBoundaries(max,min,nBins);
        static_cast<TRestDetectorHitsEvent *>(hitsEvent)->DrawEvent(origin,end,max,min,nBins);
        hitmapCanvas->Update();

        projectionCanvas->cd();
        static_cast<TRestTrackEvent *>(trackEvent)->DrawEvent(origin,end,max,min,nBins);
        projectionCanvas->Update();

        signalCanvas->cd();
        DrawPulses( (TRestRawSignalEvent *)rawSignalEvent, fReadout);
        signalCanvas->Update();

      char str[200];
      bool ext = false;
      do {
        std::cout<<"Type 'n' for next event or 'q' to quit interactive mode "<<std::endl;
        scanf("%s",str);
        if(strcmp("n",str) == 0)ext=true;
        if(strcmp("q",str) == 0){ext=true;interactive=false;}
      } while (!ext);

    }

  UpdateRate(run->GetEndTimestamp(), oldTimeEv, currentEventCount, oldEventCount, rateGraphCounter, rateGraph, true);
  TCanvas *rateCanvas = new TCanvas("rateCanvas", "rateCanvas");
  rateGraph->GetXaxis()->SetTimeDisplay(1);
  rateGraph->GetXaxis()->SetTitle("Date");
  rateGraph->GetYaxis()->SetTitle("Rate (Hz)");
  rateGraph->SetTitle("Rate");
  rateGraph->Draw("ALP");
  rateCanvas->Update();

  double duration = run->GetEndTimestamp() - run->GetStartTimestamp();

  std::cout<<"Duration "<<duration/3600.<<" hours"<<std::endl;
  std::cout<<"Entries "<<run->GetEntries()<<" Downwards "<<dTracks<<" Upwards "<<uTracks<<std::endl;
  std::cout<<"Average rate "<<currentEventCount/duration<<" Hz "<<std::endl;

  return 0;

}
