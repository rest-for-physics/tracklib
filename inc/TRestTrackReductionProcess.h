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

#include <TRestTrackEvent.h>

#include "TRestEventProcess.h"

class TRestTrackReductionProcess : public TRestEventProcess {
   private:
#ifndef __CINT__
    TRestTrackEvent* fInputTrackEvent;   //!
    TRestTrackEvent* fOutputTrackEvent;  //!
#endif

    void Initialize();

   protected:
    Double_t fStartingDistance = 0.5;
    Double_t fMinimumDistance = 3;
    Double_t fDistanceStepFactor = 1.5;
    Double_t fMaxNodes = 30;
    Int_t fMaxIt = 100;
    Bool_t fKmeans = false;

   public:
    any GetInputEvent() const override { return fInputTrackEvent; }
    any GetOutputEvent() const override { return fOutputTrackEvent; }

    void InitProcess();
    TRestEvent* ProcessEvent(TRestEvent* eventInput);
    void getHitsMerged(TRestVolumeHits& hits);
    void EndProcess();

    void PrintMetadata() {
        BeginPrintProcess();

        metadata << " Starting distance : " << fStartingDistance << endl;
        metadata << " Minimum distance : " << fMinimumDistance << endl;
        metadata << " Distance step factor : " << fDistanceStepFactor << endl;
        metadata << " Maximum number of nodes : " << fMaxNodes << endl;
        metadata << " Perform kMeans clustering : " << fKmeans << endl;
        if (fKmeans) metadata << " Maximum iterations : " << fMaxIt << endl;

        EndPrintProcess();
    }

    inline const char* GetProcessName() const { return "trackReduction"; }

    // Constructor
    TRestTrackReductionProcess();
    // Destructor
    ~TRestTrackReductionProcess();

    ClassDef(TRestTrackReductionProcess, 2);  // Template for a REST "event process" class inherited from
                                              // TRestEventProcess
};
#endif
