///______________________________________________________________________________
///______________________________________________________________________________
///
///
///             RESTSoft : Software for Rare Event Searches with TPCs
///
///             TRestTrackPointLikeAnalysisProcess.cxx
///
///_______________________________________________________________________________

#include "TRestTrackPointLikeAnalysisProcess.h"

using namespace std;

ClassImp(TRestTrackPointLikeAnalysisProcess);

TRestTrackPointLikeAnalysisProcess::TRestTrackPointLikeAnalysisProcess() { Initialize(); }

TRestTrackPointLikeAnalysisProcess::TRestTrackPointLikeAnalysisProcess(const char* configFilename) {
    Initialize();

    if (LoadConfigFromFile(configFilename)) LoadDefaultConfig();
}

TRestTrackPointLikeAnalysisProcess::~TRestTrackPointLikeAnalysisProcess() {}

void TRestTrackPointLikeAnalysisProcess::LoadDefaultConfig() { SetTitle("Default config"); }

void TRestTrackPointLikeAnalysisProcess::Initialize() {
    SetSectionName(this->ClassName());
    SetLibraryVersion(LIBRARY_VERSION);

    fTrackEvent = nullptr;
}

void TRestTrackPointLikeAnalysisProcess::LoadConfig(string configFilename) {
    if (LoadConfigFromFile(configFilename)) LoadDefaultConfig();

    // fReadout = new TRestDetectorReadout( configFilename.c_str() );
}

void TRestTrackPointLikeAnalysisProcess::InitProcess() { TRestEventProcess::ReadObservables(); }

TRestEvent* TRestTrackPointLikeAnalysisProcess::ProcessEvent(TRestEvent* inputEvent) {
    fTrackEvent = (TRestTrackEvent*)inputEvent;

    Int_t nTracks = fTrackEvent->GetNumberOfTracks();
    Int_t nTotalHits = fTrackEvent->GetTotalHits();

    Double_t totEnergy = 0;
    Double_t maxTrackEnergy = 0;

    Double_t totLength = 0;
    Double_t maxTrackLength = 0;

    Int_t maxNumberHits = 0;

    Int_t maxIndex = 0;

    for (int tck = 0; tck < fTrackEvent->GetNumberOfTracks(); tck++) {
        Double_t trackEnergy = fTrackEvent->GetTrack(tck)->GetEnergy();
        if (trackEnergy > maxTrackEnergy) {
            maxTrackEnergy = trackEnergy;
            maxIndex = tck;
        }
        totEnergy += trackEnergy;

        Double_t trackLength = fTrackEvent->GetTrack(tck)->GetTrackLength();
        if (trackLength > maxTrackLength) maxTrackLength = trackLength;
        totLength += trackLength;

        Int_t numberHits = fTrackEvent->GetTrack(tck)->GetVolumeHits()->GetNumberOfHits();
        if (numberHits > maxNumberHits) maxNumberHits = numberHits;
    }

    Double_t totSize = 0;
    Double_t maxClusterSize = 0;

    for (unsigned int cl = 0; cl < fTrackEvent->GetTrack(maxIndex)->GetVolumeHits()->GetNumberOfHits();
         cl++) {
        Double_t clusterSize = fTrackEvent->GetTrack(maxIndex)->GetVolumeHits()->GetClusterSize(cl);
        if (clusterSize > maxClusterSize) maxClusterSize = clusterSize;
        totSize += clusterSize;
    }

    Double_t clusterEnergyBalance = maxTrackEnergy / totEnergy;
    Double_t clusterLengthBalance = maxTrackLength / totLength;
    Double_t hitsBalance = maxNumberHits / nTotalHits;
    Double_t clusterSizeBalance = maxClusterSize / totSize;

    // A new value for each observable is added
    SetObservableValue("nTracks", nTracks);                            // Total number of tracks of the event
    SetObservableValue("nTotalHits", nTotalHits);                      // Total number of hits of the event
    SetObservableValue("totalEnergy", totEnergy);                      // Total number of hits of the event
    SetObservableValue("clusterEnergyBalance", clusterEnergyBalance);  // cluster energy balance
    SetObservableValue("clusterLengthBalance", clusterLengthBalance);  // cluster length balance
    SetObservableValue("hitsBalance", hitsBalance);                    // cluster number of hits balance
    SetObservableValue("size", totSize);                               // size of the track with more energy
    SetObservableValue("sizeBalance", clusterSizeBalance);             // size balance

    return fTrackEvent;
}

void TRestTrackPointLikeAnalysisProcess::EndProcess() {
    // Function to be executed once at the end of the process
    // (after all events have been processed)

    // Start by calling the EndProcess function of the abstract class.
    // Comment this if you don't want it.
    // TRestEventProcess::EndProcess();
}

void TRestTrackPointLikeAnalysisProcess::InitFromConfigFile() {}
