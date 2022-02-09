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

#include "dirent.h"

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
      if(readoutChannel == -1)continue;
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

void DrawOriginEnd(TPad *pad, TGraph *originXZGr, TGraph *originYZGr,TGraph *endXZGr,TGraph *endYZGr){

  pad->cd(1);
  originXZGr->SetMarkerColor(kRed);
  originXZGr->SetMarkerStyle(20);
  originXZGr->Draw("LP");
  endXZGr->SetMarkerColor(kBlack);
  endXZGr->SetMarkerStyle(20);
  endXZGr->Draw("LP");

  pad->cd(2);
  originYZGr->SetMarkerColor(kRed);
  originYZGr->SetMarkerStyle(20);
  originYZGr->Draw("LP");
  endYZGr->SetMarkerColor(kBlack);
  endYZGr->SetMarkerStyle(20);
  endYZGr->Draw("LP");

}

std::vector <string> getFiles(const std::string &fileName){

  std::vector <string> files;
  std::string fName=fileName;
  if(fName.back() == '*')fName.pop_back();

  size_t found = fName.find_last_of("/\\");
  std::string path = "./";
  std::string file = fName;
    if(found != std::string::npos){
      path = fName.substr(0,found+1);
      fName = fName.substr(found+1);
    }

  DIR *directory = opendir(path.c_str());
  struct dirent *direntStruct;

    if (directory != NULL) {
        while ( (direntStruct = readdir(directory)) ) {
          std::string fCheck = direntStruct->d_name;
          if(fCheck.find(fName)==0)files.push_back(path+fCheck);
        }
    }
  closedir(directory);

  std::sort(files.begin(),files.end());

  return files;

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

int REST_Track_PlotAlphaAna(const std::string &fileName, bool interactive = true){

  auto files = getFiles(fileName);

  TH1F *spectraD = nullptr,*spectraU = nullptr, *angle = nullptr;
  TH2F *endH = nullptr, *origH = nullptr,*trackL = nullptr;

  TCanvas *spectraCanvas = new TCanvas("spectraCanvas", "spectraCanvas");
  TCanvas *originCanvas = new TCanvas("originCanvas", "originCanvas");
  TCanvas *endCanvas = new TCanvas("endCanvas", "endCanvas");
  TCanvas *trackLengthCanvas = new TCanvas("trackLengthCanvas", "trackLengthCanvas");
  TCanvas *trackAngleCanvas = new TCanvas("trackAngleCanvas", "trackAngleCanvas");
  TCanvas *hitmapCanvas = new TCanvas("hitmapCanvas", "hitmapCanvas");
  TCanvas *projectionCanvas = new TCanvas("projectionCanvas", "projectionCanvas");
  TCanvas *signalCanvas = new TCanvas("signalCanvas", "signalCanvas");

  double oldTimeEv =0,duration=0,lastTimeStamp=0;
  int currentEventCount=0,oldEventCount=0,rateGraphCounter=0;
  TGraphErrors* rateGraph = new TGraphErrors();


  int c=0;

  for(auto f:files){

    std::cout<<f<<std::endl;

    TRestRun* run = new TRestRun(f);

    if(c==0){
      run->PrintMetadata();
      oldTimeEv = run->GetStartTimestamp();
    }

    TRestTrackAlphaAnalysisProcess *trackAlphaAna = (TRestTrackAlphaAnalysisProcess *)run->GetMetadataClass("TRestTrackAlphaAnalysisProcess");

      if(trackAlphaAna == nullptr){
        std::cout<<"ERROR: No TRestTrackAlphaAnalysisProcess found in "<<f<<" please, provide a valid file"<<std::endl;
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

    spectraCanvas->cd();
    anaTree->Draw("alphaTrackAna_totalEnergy>>sD(200,0,1000000)","alphaTrackAna_downwards");
    TH1F *sD = (TH1F *)gDirectory->Get("sD");
    if(spectraD==nullptr)spectraD=sD;
    else if (sD) spectraD->Add(sD);

    anaTree->Draw("alphaTrackAna_totalEnergy>>sU(200,0,1000000)","!alphaTrackAna_downwards");
    TH1F *sU = (TH1F *)gDirectory->Get("sU");
    if(spectraU==nullptr)spectraU=sU;
    else if (sU) spectraU->Add(sU);

    if(c==0)spectraU->SetLineColor(kRed);
    spectraD->Draw();
    spectraCanvas->cd();
    spectraU->Draw("SAME");
    spectraCanvas->Update();

    originCanvas->cd();
    std::string cmd = "alphaTrackAna_originX:alphaTrackAna_originY>>orig("+planeStr +")";
    anaTree->Draw(cmd.c_str());
    TH2F *orig = (TH2F*)gDirectory->Get("orig");
    if(origH==nullptr)origH=orig;
    else if (orig) origH->Add(orig);
    originCanvas->cd();
    origH->Draw("COLZ");
    originCanvas->Update();


    endCanvas->cd();
    cmd = "alphaTrackAna_endX:alphaTrackAna_endY>>end("+planeStr +")";
    anaTree->Draw(cmd.c_str());
    TH2F *end = (TH2F*)gDirectory->Get("end");
    if(endH==nullptr)endH=end;
    else if (end) endH->Add(end);
    endCanvas->cd();
    endH->Draw("COLZ");
    endCanvas->Update();

    trackLengthCanvas->cd();
    anaTree->Draw("alphaTrackAna_totalEnergy:alphaTrackAna_length>>tckL(100,0,100,100,0,1000000)");
    TH2F *tckL = (TH2F*)gDirectory->Get("tckL");
    if(trackL==nullptr)trackL=tckL;
    else if (tckL) trackL->Add(tckL);
    trackLengthCanvas->cd();
    trackL->Draw("COLZ");
    trackLengthCanvas->Update();

    trackAngleCanvas->cd();
    anaTree->Draw("alphaTrackAna_angle>>ang(100,0,3.1416)");
    TH1F *ang = (TH1F*)gDirectory->Get("ang");
    if(angle==nullptr)angle=ang;
    else if (ang) angle->Add(ang);
    trackAngleCanvas->cd();
    angle->Draw();
    trackAngleCanvas->Update();

    TRestEvent* trackEvent = REST_Reflection::Assembly("TRestTrackEvent");
    TRestEvent* hitsEvent = REST_Reflection::Assembly("TRestDetectorHitsEvent");
    TRestEvent* rawSignalEvent = REST_Reflection::Assembly("TRestRawSignalEvent");

    run->SetInputEvent(trackEvent);
    run->SetInputEvent(hitsEvent);
    run->SetInputEvent(rawSignalEvent);

    TGraph * originXZGr=nullptr,* originYZGr=nullptr,* endXZGr=nullptr,* endYZGr=nullptr;

    for(int i=0;i<run->GetEntries();i++){
      run->GetEntry(i);

      UpdateRate(rawSignalEvent->GetTime(), oldTimeEv, currentEventCount, oldEventCount, rateGraphCounter, rateGraph);
      lastTimeStamp = rawSignalEvent->GetTime();
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
        
        if(originXZGr) delete originXZGr;
        if(originYZGr) delete originYZGr;
        if(endXZGr) delete endXZGr;
        if(endYZGr) delete endYZGr;
        
        originXZGr = new TGraph();
        originXZGr->SetPoint(0,origin.X(),origin.Z());
        originYZGr = new TGraph();
        originYZGr->SetPoint(0,origin.Y(),origin.Z());
        endXZGr = new TGraph();
        endXZGr->SetPoint(0,end.X(),end.Z());
        endYZGr = new TGraph();
        endYZGr->SetPoint(0,end.Y(),end.Z());
        
        hitmapCanvas->cd();
        TPad *hitsPad = static_cast<TRestDetectorHitsEvent *>(hitsEvent)->DrawEvent("hist(COLZ)");
        TVector3 max,min,nBins;
        static_cast<TRestDetectorHitsEvent *>(hitsEvent)->GetBoundaries(max,min,nBins);
        DrawOriginEnd(hitsPad, originXZGr, originYZGr,endXZGr,endYZGr);
        hitmapCanvas->Update();

        projectionCanvas->cd();
        TPad *trackPad =static_cast<TRestTrackEvent *>(trackEvent)->DrawEvent(max,min,nBins);
        DrawOriginEnd(trackPad, originXZGr, originYZGr,endXZGr,endYZGr);
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

    duration += run->GetEndTimestamp() - run->GetStartTimestamp();
    c += run->GetEntries();
  }

  UpdateRate(lastTimeStamp, oldTimeEv, currentEventCount, oldEventCount, rateGraphCounter, rateGraph, true);
  TCanvas *rateCanvas = new TCanvas("rateCanvas", "rateCanvas");
  rateGraph->GetXaxis()->SetTimeDisplay(1);
  rateGraph->GetXaxis()->SetTitle("Date");
  rateGraph->GetYaxis()->SetTitle("Rate (Hz)");
  rateGraph->SetTitle("Rate");
  rateGraph->Draw("ALP");
  rateCanvas->Update();

  std::cout<<"Duration "<<duration/3600.<<" hours"<<std::endl;
  std::cout<<"Entries "<<currentEventCount<<" Downwards "<<spectraD->GetEntries()<<" Upwards "<<spectraU->GetEntries()<<std::endl;
  std::cout<<"Average rate "<<currentEventCount/duration<<" Hz "<<std::endl;

  return 0;

}
