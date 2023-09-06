//////////////////////////////////////////////////////////////////////////
///
///
///             RESTSoft : Software for Rare Event Searches with TPCs
///
///             TRestTrackReconnectionProcess.h
///
///              Nov 2016 : Javier Galan
///
//////////////////////////////////////////////////////////////////////////

#ifndef RestCore_TRestTrackReconnectionProcess
#define RestCore_TRestTrackReconnectionProcess

#include <TRestTrackEvent.h>

#include "TRestEventProcess.h"

class TRestTrackReconnectionProcess : public TRestEventProcess {
   private:
#ifndef __CINT__
    TRestTrackEvent* fInputTrackEvent;   //!
    TRestTrackEvent* fOutputTrackEvent;  //!

    Double_t fMeanDistance;  //!
    Double_t fSigma;         //!
#endif

    Bool_t fSplitTrack;
    Double_t fNSigmas;

    void InitFromConfigFile() override;

    void Initialize() override;

    void SetDistanceMeanAndSigma(TRestHits* h);

   protected:
   public:
    RESTValue GetInputEvent() const override { return fInputTrackEvent; }
    RESTValue GetOutputEvent() const override { return fOutputTrackEvent; }

    void InitProcess() override;
    TRestEvent* ProcessEvent(TRestEvent* inputEvent) override;
    void EndProcess() override;
    void LoadDefaultConfig();

    void LoadConfig(const std::string& configFilename, const std::string& name = "");

    void PrintMetadata() override {
        BeginPrintProcess();

        RESTMetadata << "Split track : ";
        if (fSplitTrack)
            RESTMetadata << " enabled" << RESTendl;
        else
            RESTMetadata << " disabled" << RESTendl;

        RESTMetadata << "Number of sigmas to defined a branch : " << fNSigmas << RESTendl;

        EndPrintProcess();
    }

    const char* GetProcessName() const override { return "trackReconnection"; }

    void BreakTracks(TRestVolumeHits* hits, std::vector<TRestVolumeHits>& hitSets, Double_t nSigma = 2.);
    void ReconnectTracks(std::vector<TRestVolumeHits>& hitSets);
    Int_t GetTrackBranches(TRestHits& h, Double_t nSigma);

    // Constructor
    TRestTrackReconnectionProcess();
    TRestTrackReconnectionProcess(const char* configFilename);
    // Destructor
    ~TRestTrackReconnectionProcess();

    ClassDefOverride(TRestTrackReconnectionProcess, 1);
};
#endif
