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

TH2F *trackX;
TH2F *trackY;

TH2F *hitsX;
TH2F *hitsY;

std::vector<TGraph *> pulsesX;
std::vector<TGraph *> pulsesY;

const int offset=10;//bins


TPad *DrawTrack ( TRestTrackEvent *trackEvent, const TVector3 &origin, const TVector3 &end, const TVector3 &max, const TVector3 &min, const TVector3& nBins) {

  if(trackX) delete trackX;
  if(trackY) delete trackY;

  //std::cout<<"MaxX "<<max.X()<<" minX "<<min.X()<<" NbinsX "<<nBins.X()<<std::endl;
  //std::cout<<"MaxY "<<max.Y()<<" minY "<<min.Y()<<" NbinsY "<<nBins.Y()<<std::endl;
  //std::cout<<"MaxZ "<<max.Z()<<" minZ "<<min.Z()<<" NbinsZ "<<nBins.Z()<<std::endl;

  TPad *fPad = new TPad("TrackEvent", " ", 0, 0, 1, 1);
  fPad->Divide(2);
  fPad->Draw();
  trackX = new TH2F("XZ","XZ",(int)nBins.X(),min.X(),max.X(),(int)nBins.Z(), min.Z(), max.Z());
  trackY = new TH2F("YZ","YZ",(int)nBins.Y(),min.Y(),max.Y(),(int)nBins.Z(), min.Z(), max.Z());

  int nTracksX=0;
  int nTracksY=0;

    for (int t = 0; t < trackEvent->GetNumberOfTracks(); t++) {
      TRestTrack* tck = trackEvent->GetTrack(t);
        TRestVolumeHits* hits = tck->GetVolumeHits();
           for(int i=0; i<hits->GetNumberOfHits();i++){
               if(tck->isXZ()){
                 trackX->Fill(hits->GetX(i),hits->GetZ(i),hits->GetEnergy(i));
                 nTracksX++;
               } else if (tck->isYZ()){
                 trackY->Fill(hits->GetY(i),hits->GetZ(i),hits->GetEnergy(i));
                 nTracksY++;
               }
           }
    }

  TGraph *originXZGr = new TGraph();
  originXZGr->SetPoint(0,origin.X(),origin.Z());
  TGraph *originYZGr = new TGraph();
  originYZGr->SetPoint(0,origin.Y(),origin.Z());
  TGraph *endXZGr = new TGraph();
  endXZGr->SetPoint(0,end.X(),end.Z());
  TGraph *endYZGr = new TGraph();
  endYZGr->SetPoint(0,end.Y(),end.Z());

  fPad->cd(1);
  trackX->Draw("COLZ");
  originXZGr->SetMarkerColor(kRed);
  originXZGr->SetMarkerStyle(20);
  originXZGr->Draw("LP");
  endXZGr->SetMarkerColor(kBlack);
  endXZGr->SetMarkerStyle(20);
  endXZGr->Draw("LP");
  
  fPad->cd(2);
  trackY->Draw("COLZ");
  originYZGr->SetMarkerColor(kRed);
  originYZGr->SetMarkerStyle(20);
  originYZGr->Draw("LP");
  endYZGr->SetMarkerColor(kBlack);
  endYZGr->SetMarkerStyle(20);
  endYZGr->Draw("LP");

  return fPad;

}

TPad *DrawHits ( TRestDetectorHitsEvent *hitsEvent, const TVector3 &origin, const TVector3 &end, TVector3 &max, TVector3 &min, TVector3& nBins) {

  std::vector<alphaTrackHit> XZ;
  std::vector<alphaTrackHit> YZ;
  std::vector<alphaTrackHit> allHits;

     for (int h = 0; h < hitsEvent->GetNumberOfHits(); h++) {
       double xVal=hitsEvent->GetX(h);
       double yVal= hitsEvent->GetY(h);
       alphaTrackHit hit;
       hit.z = hitsEvent->GetZ(h);
       hit.en = hitsEvent->GetEnergy(h);
       if(hitsEvent->GetType(h) == REST_HitType::XZ){
         hit.xy= xVal;
         XZ.emplace_back(hit);
       } else if(hitsEvent->GetType(h) == REST_HitType::YZ){
         hit.xy= yVal;
         YZ.emplace_back(hit);
       }
       allHits.emplace_back(hit);
    }

  std::sort(XZ.begin(),XZ.end(),alphaTrackHit::sortByPos);
  double maxX=XZ.front().xy;
  double minX=XZ.back().xy;

  double minDiffX = 1E6;
  double prevVal=1E6;
   for (auto h : XZ){
     double diff = std::abs(h.xy - prevVal);
     if(diff>0 && diff<minDiffX )minDiffX=diff;
     prevVal=h.xy;
   }

  std::sort(YZ.begin(),YZ.end(),alphaTrackHit::sortByPos);
  double maxY=YZ.front().xy;
  double minY=YZ.back().xy;

  double minDiffY = 1E6;
  prevVal=1E6;
   for (auto h : YZ){
     double diff = std::abs(h.xy - prevVal);
     if(diff>0 && diff<minDiffY )minDiffY=diff;
     prevVal=h.xy;
   }


  std::sort(allHits.begin(),allHits.end(),alphaTrackHit::sortByTime);
  double minZ = allHits.back().z;
  double maxZ = allHits.front().z;

  double minDiffZ = 1E6;
  prevVal=1E6;
   for (auto h : allHits){
     double diff = std::abs(h.z - prevVal);
     if(diff>0 && diff<minDiffZ )minDiffZ=diff;
     prevVal=h.z;
   }

  maxX+=offset*minDiffX+minDiffX/2.;
  maxY+=offset*minDiffY+minDiffX/2.;
  maxZ+=offset*minDiffZ+minDiffX/2.;
  minX-=offset*minDiffX-minDiffX/2.;
  minY-=offset*minDiffY-minDiffX/2.;
  minZ-=offset*minDiffZ-minDiffX/2.;

  int nBinsX = std::round((maxX-minX)/minDiffX);
  int nBinsY = std::round((maxY-minY)/minDiffY);
  int nBinsZ = std::round((maxZ-minZ)/minDiffZ);

  max.SetXYZ(maxX,maxY,maxZ);
  min.SetXYZ(minX,minY,minZ);
  nBins.SetXYZ(nBinsX,nBinsY,nBinsZ);

  //std::cout<<"MaxX "<<maxX<<" minX "<<minX<<" minDiffX "<<minDiffX<<" NbinsX "<<nBinsX<<std::endl;
  //std::cout<<"MaxY "<<maxY<<" minY "<<minY<<" minDiffY "<<minDiffY<<" NbinsY "<<nBinsY<<std::endl;
  //std::cout<<"MaxZ "<<maxZ<<" minZ "<<minZ<<" minDiffZ "<<minDiffZ<<" NbinsZ "<<nBinsZ<<std::endl;


  if(hitsX) delete hitsX;
  if(hitsY) delete hitsY;

  TPad *fPad = new TPad("HitEvent", " ", 0, 0, 1, 1);
  fPad->Divide(2);
  fPad->Draw();
  hitsX = new TH2F("HXZ","HXZ",nBinsX,minX, maxX, nBinsZ, minZ, maxZ);
  hitsY = new TH2F("HYZ","HYZ",nBinsY,minY, maxY, nBinsZ, minZ, maxZ);

    for (auto h : XZ){
      hitsX->Fill(h.xy,h.z,h.en);
    }
    for (auto h : YZ){
      hitsY->Fill(h.xy,h.z,h.en);
    }

  //std::cout<<"NHits X: "<<XZ.size()<<" NHits Y: "<<YZ.size()<<std::endl;

  TGraph *originXZGr = new TGraph();
  originXZGr->SetPoint(0,origin.X(),origin.Z());
  TGraph *originYZGr = new TGraph();
  originYZGr->SetPoint(0,origin.Y(),origin.Z());
  TGraph *endXZGr = new TGraph();
  endXZGr->SetPoint(0,end.X(),end.Z());
  TGraph *endYZGr = new TGraph();
  endYZGr->SetPoint(0,end.Y(),end.Z());

  fPad->cd(1);
  hitsX->Draw("COLZ");
  originXZGr->SetMarkerColor(kRed);
  originXZGr->SetMarkerStyle(20);
  originXZGr->Draw("LP");
  endXZGr->SetMarkerColor(kBlack);
  endXZGr->SetMarkerStyle(20);
  endXZGr->Draw("LP");
  
  fPad->cd(2);
  hitsY->Draw("COLZ");
  originYZGr->SetMarkerColor(kRed);
  originYZGr->SetMarkerStyle(20);
  originYZGr->Draw("LP");
  endYZGr->SetMarkerColor(kBlack);
  endYZGr->SetMarkerStyle(20);
  endYZGr->Draw("LP");

  return fPad;

}

TPad *DrawPulses ( TRestRawSignalEvent *rawSignalEvent, TRestDetectorReadout *fReadout) {

  for(auto p :pulsesX)if(p)delete p;
  for(auto p :pulsesY)if(p)delete p;

  pulsesX.clear();
  pulsesY.clear();

  TRestDetectorReadoutPlane* plane = &(*fReadout)[0];

  Int_t numberOfSignals = rawSignalEvent->GetNumberOfSignals();
  Int_t readoutChannel = -1, readoutModule, planeID=0;
  int maxX=0, maxY=0;
    for (int i = 0; i < numberOfSignals; i++) {
      TRestRawSignal* sgnl = rawSignalEvent->GetSignal(i);
      Int_t signalID = sgnl->GetSignalID();
      fReadout->GetPlaneModuleChannel(signalID, planeID, readoutModule, readoutChannel);
      Double_t x = plane->GetX(readoutModule, readoutChannel);
      Double_t y = plane->GetY(readoutModule, readoutChannel);
      TGraph *gr = new TGraph();
      int max=0;
        for(int p =0;p<sgnl->GetNumberOfPoints();p++){
          int val = sgnl->GetData(p);
          if(val>max)max=val;
          gr->SetPoint(p,p,val);
        }

        if (TMath::IsNaN(x)) {
          if(max>maxY)maxY=max;
          pulsesY.emplace_back(gr);
        } else if (TMath::IsNaN(y)) {
          if(max>maxX)maxX=max;
          pulsesX.emplace_back(gr);
        }
    }

  TPad *fPad = new TPad("RawSignalEvent", " ", 0, 0, 1, 1);
  fPad->Divide(2);
  fPad->Draw();
  fPad->cd(1);
  fPad->DrawFrame(0,0,512,maxX+maxX*0.10);
  fPad->cd(2);
  fPad->DrawFrame(0,0,512,maxY+maxY*0.10);

  int color=1;
  for(auto gr : pulsesX){
    fPad->cd(1);
    gr->SetLineWidth(2);
    gr->SetLineColor(color);
    gr->Draw("SAME");
    color++;
  }

  color=1;
  for(auto gr : pulsesY){
    fPad->cd(2);
    gr->SetLineWidth(2);
    gr->SetLineColor(color);
    gr->Draw("SAME");
    color++;
  }

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
        DrawHits( (TRestDetectorHitsEvent *)hitsEvent, origin,end,max,min,nBins);
        hitmapCanvas->Update();

        projectionCanvas->cd();
        DrawTrack( (TRestTrackEvent *)trackEvent, origin,end,max,min,nBins);
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
  std::cout<<"Average rate "<<currentEventCount/duration<<std::endl;

  return 0;

}
