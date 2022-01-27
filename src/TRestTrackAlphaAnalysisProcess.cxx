///______________________________________________________________________________
///______________________________________________________________________________
///
///
///             RESTSoft : Software for Rare Event Searches with TPCs
///
///             TRestTrackAlphaAnalysisProcess.cxx
///
///_______________________________________________________________________________

#include "TRestTrackAlphaAnalysisProcess.h"
using namespace std;

ClassImp(TRestTrackAlphaAnalysisProcess);

//______________________________________________________________________________
TRestTrackAlphaAnalysisProcess::TRestTrackAlphaAnalysisProcess() { Initialize(); }

//______________________________________________________________________________
TRestTrackAlphaAnalysisProcess::TRestTrackAlphaAnalysisProcess(char* cfgFileName) {
    Initialize();

    if (LoadConfigFromFile(cfgFileName)) LoadDefaultConfig();
    PrintMetadata();
}

//______________________________________________________________________________
TRestTrackAlphaAnalysisProcess::~TRestTrackAlphaAnalysisProcess() {
  delete fOutTrackEvent;
}

void TRestTrackAlphaAnalysisProcess::LoadDefaultConfig() { SetTitle("Default config"); }

//______________________________________________________________________________
void TRestTrackAlphaAnalysisProcess::Initialize() {
    SetSectionName(this->ClassName());
    SetLibraryVersion(LIBRARY_VERSION);

    fTrackEvent = nullptr;
    fOutTrackEvent = new TRestTrackEvent();
}

void TRestTrackAlphaAnalysisProcess::LoadConfig(string cfgFilename) {
    if (LoadConfigFromFile(cfgFilename)) LoadDefaultConfig();
}

//______________________________________________________________________________
void TRestTrackAlphaAnalysisProcess::InitProcess() {
  TRestEventProcess::ReadObservables();
}

//______________________________________________________________________________
TRestEvent* TRestTrackAlphaAnalysisProcess::ProcessEvent(TRestEvent* evInput) {

    fTrackEvent = (TRestTrackEvent*)evInput;
    fOutTrackEvent->SetEventInfo(fTrackEvent);

    Int_t nTracks = fTrackEvent->GetNumberOfTracks();
    Int_t nTotalHits = fTrackEvent->GetTotalHits();

    Int_t nTracksX = fTrackEvent->GetNumberOfTracks("X");
    Int_t nTracksY = fTrackEvent->GetNumberOfTracks("Y");

    //No tracks in X or Y nothing to do
    if(nTracksX ==0 || nTracksY==0)return nullptr;

    Double_t totEnergyX = 0, totEnergyY=0;
    Double_t maxTrackEnergyX = 0, maxTrackEnergyY=0;
    Int_t maxIndexX = -1, maxIndexY=-1;

    for (int t = 0; t < fTrackEvent->GetNumberOfTracks(); t++) {
        TRestTrack* track = fTrackEvent->GetTrack(t);
        Double_t trackEn = track->GetEnergy();
        if(track->isXZ()) {
          if(trackEn>maxTrackEnergyX){
            maxTrackEnergyX = trackEn;
            maxIndexX = t;
          }
          totEnergyX += trackEn;
        } else if (track->isYZ()) {
          if(trackEn>maxTrackEnergyY){
            maxTrackEnergyY = trackEn;
            maxIndexY = t;
          }
          totEnergyY += trackEn;
        }
    }

   if(maxTrackEnergyX < totEnergyX*fTrackBalance || maxTrackEnergyY < totEnergyY*fTrackBalance)return nullptr;

   Double_t trackEnergy = maxTrackEnergyX + maxTrackEnergyY;

   TRestTrack* tckX = fTrackEvent->GetTrack(maxIndexX);
   TRestTrack* tckY = fTrackEvent->GetTrack(maxIndexY);

   debug<<" Track Energy "<<trackEnergy<<" "<<endl;

   std::vector<std::pair<double,double> > posZEn;

   TRestVolumeHits* hitsX = tckX->GetVolumeHits();
   std::vector<alphaTrackHit> hitArrayX;
     for(int i=0; i<hitsX->GetNumberOfHits();i++){
       alphaTrackHit hit;
       hit.xy= hitsX->GetX(i);
       hit.z= hitsX->GetZ(i);
       hit.en = hitsX->GetEnergy(i);
       hitArrayX.emplace_back(hit);
       posZEn.emplace_back(std::make_pair(hit.z,hit.en));
     }

   TRestVolumeHits* hitsY = tckY->GetVolumeHits();
   std::vector<alphaTrackHit> hitArrayY;
     for(int i=0; i<hitsY->GetNumberOfHits();i++){
       alphaTrackHit hit;
       hit.xy= hitsY->GetY(i);
       hit.z= hitsY->GetZ(i);
       hit.en = hitsY->GetEnergy(i);
       hitArrayY.emplace_back(hit);
       posZEn.emplace_back(std::make_pair(hit.z,hit.en));
     }

   std::sort(posZEn.begin(),posZEn.end());

   double integ=0;
   double minDiff = 1E10;
   double halfIntZ = 0;
   for(auto p : posZEn){
     integ+= p.second;
     double diff = std::abs (integ - trackEnergy/2.);
     if(diff <minDiff ){halfIntZ=p.first;minDiff =diff; }
   }

  debug<<"Min diff "<<minDiff<<" posZ "<<halfIntZ<<endl;

   double originX, originY, originZ;
   double endX, endY, endZ;
   Bool_t downwards;

     if( std::abs(halfIntZ - posZEn.back().first) > std::abs (halfIntZ - posZEn.front().first) ){ //Direction is downwards
       originZ = posZEn.back().first;
       endZ = posZEn.front().first;
       downwards=true;
     } else {//Direction is upwards
       originZ = posZEn.front().first;
       endZ = posZEn.back().first;
       downwards=false;
     }

   downwards ? debug<<"Downwards "<<endl :debug<<"Upwards "<<endl;

   auto smtTckX = smoothTrack(hitArrayX);
   auto smtTckY = smoothTrack(hitArrayY);

   if(smtTckX.empty() || smtTckY.empty())return nullptr;

   TRestVolumeHits volHit;
     for(auto tck : smtTckX){
       const Double_t x = tck.xy;
       const Double_t y = 0;
       const Double_t z = tck.z;
       const Double_t en = tck.en;
       TVector3 pos(x, y, z);
       TVector3 sigma(0., 0., 0.);
       volHit.AddHit(pos, en, 0, REST_HitType::XZ, sigma);
     }
     TRestTrack *trackXZ = new TRestTrack();
     trackXZ->SetParentID(0);
     trackXZ->SetTrackID(1);
     trackXZ->SetVolumeHits(volHit);
     fOutTrackEvent->AddTrack(trackXZ);
     fOutTrackEvent->SetNumberOfXTracks(1);
     volHit.RemoveHits();
     delete trackXZ;

     for(auto tck : smtTckY){
       const Double_t x = 0;
       const Double_t y = tck.xy;
       const Double_t z = tck.z;
       const Double_t en = tck.en;
       TVector3 pos(x, y, z);
       TVector3 sigma(0., 0., 0.);
       volHit.AddHit(pos, en, 0, REST_HitType::YZ, sigma);
     }
     TRestTrack *trackYZ = new TRestTrack();
     trackYZ->SetParentID(0);
     trackYZ->SetTrackID(2);
     trackYZ->SetVolumeHits(volHit);
     fOutTrackEvent->AddTrack(trackYZ);
     fOutTrackEvent->SetNumberOfYTracks(1);
     volHit.RemoveHits();
     delete trackYZ;


     if ( std::abs(smtTckX.front().z - originZ) >= std::abs(smtTckX.back().z - originZ)  ){
       originX = smtTckX.back().xy;
       endX = smtTckX.front().xy;
     } else {
       endX = smtTckX.back().xy;
       originX = smtTckX.front().xy;
     }

     if ( std::abs(smtTckY.front().z - originZ) >= std::abs(smtTckY.back().z - originZ)  ){
       originY = smtTckY.back().xy;
       endY = smtTckY.front().xy;
     } else {
       endY = smtTckY.back().xy;
       originY = smtTckY.front().xy;
     }

    debug<<"Origin x: "<<originX<<" y: "<<originY<<" z: "<<originZ<<endl;
    debug<<"End x: "<<endX<<" y: "<<endY<<" z: "<<endZ<<endl;

    double dX = (originX - endX);
    double dY = (originY - endY);
    double dZ = (originZ - endZ);
    Double_t length = TMath::Sqrt(dX*dX+dY*dY+dZ*dZ);
    Double_t angle = TMath::ACos(dZ/length);

    debug<<"Track length "<<length<<" angle: "<<angle<<endl;

    // A new value for each observable is added
    SetObservableValue("originX", originX);
    SetObservableValue("originY", originY);
    SetObservableValue("originZ", originZ);
    SetObservableValue("endX", endX);
    SetObservableValue("endY", endY);
    SetObservableValue("endZ", endZ);
    SetObservableValue("length", length);
    SetObservableValue("angle", angle);
    SetObservableValue("downwards", downwards);
    SetObservableValue("totalEnergy", trackEnergy);

    return fOutTrackEvent;
}

std::vector<alphaTrackHit> TRestTrackAlphaAnalysisProcess::smoothTrack(std::vector <alphaTrackHit> &hits){

  std::vector <alphaTrackHit> res;
  std::sort(hits.begin(),hits.end(),alphaTrackHit::sortByTime);
  int i=0;
  const int size = hits.size();
  debug<<"Track size "<<size<<endl;
    while(i<size){
      double posXY=0;
      double en=0;
      alphaTrackHit hit = hits[i];
      double posZ = hits[i].z;
      int nPoints=0;
      while(posZ == hits[i].z && i<size){
        posXY += hits[i].xy*hits[i].en;
        en += hits[i].en;
        i++;
        nPoints++;
      }
        if(en>0 && nPoints >1){
          hit.xy = posXY/en;
          hit.en = en;
          res.emplace_back(hit);
        }

    }

  debug<<"Res size "<<res.size()<<endl;

  return res;
}

//______________________________________________________________________________
void TRestTrackAlphaAnalysisProcess::EndProcess() {
    // Function to be executed once at the end of the process
    // (after all events have been processed)

    // Start by calling the EndProcess function of the abstract class.
    // Comment this if you don't want it.
    // TRestEventProcess::EndProcess();
}

//______________________________________________________________________________
void TRestTrackAlphaAnalysisProcess::InitFromConfigFile() {
  fTrackBalance = StringToDouble(GetParameter("trackBalance", "0.65"));
}
