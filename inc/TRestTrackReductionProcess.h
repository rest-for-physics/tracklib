//////////////////////////////////////////////////////////////////////////
///
///
///             RESTSoft : Software for Rare Event Searches with TPCs
///
///             TRestTrackReductionProcess.h
///
///              Jan 2016 : Javier Galan
///
//////////////////////////////////////////////////////////////////////////

#ifndef RestCore_TRestTrackReductionProcess
#define RestCore_TRestTrackReductionProcess

#include <TRestEventProcess.h>

#include "TRestTrackEvent.h"

class TRestTrackReductionProcess : public TRestEventProcess {
   private:
#ifndef __CINT__
    TRestTrackEvent* fInputTrackEvent;   //!
    TRestTrackEvent* fOutputTrackEvent;  //!
#endif

    void Initialize() override;

   protected:
    Double_t fStartingDistance = 0.5;
    Double_t fMinimumDistance = 3;
    Double_t fDistanceStepFactor = 1.5;
    Double_t fMaxNodes = 30;
    Int_t fMaxIt = 100;
    Bool_t fKmeans = false;

   public:
    RESTValue GetInputEvent() const override { return fInputTrackEvent; }
    RESTValue GetOutputEvent() const override { return fOutputTrackEvent; }

    void InitProcess() override;
    TRestEvent* ProcessEvent(TRestEvent* inputEvent) override;
    void getHitsMerged(TRestVolumeHits& hits);
    void EndProcess() override;

    void PrintMetadata() override {
        BeginPrintProcess();

        RESTMetadata << " Starting distance : " << fStartingDistance << RESTendl;
        RESTMetadata << " Minimum distance : " << fMinimumDistance << RESTendl;
        RESTMetadata << " Distance step factor : " << fDistanceStepFactor << RESTendl;
        RESTMetadata << " Maximum number of nodes : " << fMaxNodes << RESTendl;
        RESTMetadata << " Perform kMeans clustering : " << fKmeans << RESTendl;
        if (fKmeans) RESTMetadata << " Maximum iterations : " << fMaxIt << RESTendl;

        EndPrintProcess();
    }

    const char* GetProcessName() const override { return "trackReduction"; }

    // Constructor
    TRestTrackReductionProcess();
    // Destructor
    ~TRestTrackReductionProcess();

    ClassDefOverride(TRestTrackReductionProcess, 2);  // Template for a REST "event process" class inherited
                                                      // from TRestEventProcess
};
#endif
