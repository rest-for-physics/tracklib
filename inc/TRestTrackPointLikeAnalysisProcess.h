///______________________________________________________________________________
///______________________________________________________________________________
///______________________________________________________________________________
///
///
///             RESTSoft : Software for Rare Event Searches with TPCs
///
///             TRestTrackPointLikeAnalysisProcess.h
///
///_______________________________________________________________________________

#ifndef RestCore_TRestTrackPointLikeAnalysisProcess
#define RestCore_TRestTrackPointLikeAnalysisProcess

#include <TRestTrackEvent.h>

#include "TRestEventProcess.h"

class TRestTrackPointLikeAnalysisProcess : public TRestEventProcess {
   private:
#ifndef __CINT__
    TRestTrackEvent* fTrackEvent;  //!
                                   // TODO We must get here a pointer to TRestDaqMetadata
                                   // In order to convert the parameters to time using the sampling time
#endif

    void InitFromConfigFile();

    void Initialize();

    void LoadDefaultConfig();

   protected:
    // add here the members of your event process

   public:
    inline any GetInputEvent() const { return fTrackEvent; }
    inline any GetOutputEvent() const { return fTrackEvent; }

    void InitProcess();
    TRestEvent* ProcessEvent(TRestEvent* eventInput);
    void EndProcess();

    void LoadConfig(std::string configFilename);

    void PrintMetadata() {
        BeginPrintProcess();

        EndPrintProcess();
    }

    inline TString GetProcessName() const { return (TString) "pointLikeTrackAna"; }
    //       Double_t GetCalibFactor();           ///< Calibration factor is
    //       found.

    // Constructor
    TRestTrackPointLikeAnalysisProcess();
    TRestTrackPointLikeAnalysisProcess(char* configFilename);
    // Destructor
    ~TRestTrackPointLikeAnalysisProcess();

    ClassDef(TRestTrackPointLikeAnalysisProcess,
             1);  // Template for a REST "event process" class inherited from
                  // TRestEventProcess
};
#endif
