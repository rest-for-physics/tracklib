///______________________________________________________________________________
///______________________________________________________________________________
///______________________________________________________________________________
///
///
///             RESTSoft : Software for Rare Event Searches with TPCs
///
///             TRestTrackAlphaAnalysisProcess.h
///
///_______________________________________________________________________________

#ifndef RestCore_TRestTrackAlphaAnalysisProcess
#define RestCore_TRestTrackAlphaAnalysisProcess

#include <TRestTrackEvent.h>

#include "TRestEventProcess.h"

class alphaTrackHit{
  public:
     double xy;
     double z;
     double en;

  static bool sortByTime (const alphaTrackHit &h1, const alphaTrackHit &h2 ){
    return h1.z > h2.z;
  }

  static bool sortByPos (const alphaTrackHit &h1, const alphaTrackHit &h2 ){
    return h1.xy > h2.xy;
  }
};

class TRestTrackAlphaAnalysisProcess : public TRestEventProcess {
   private:
#ifndef __CINT__
    TRestTrackEvent* fTrackEvent;
    TRestTrackEvent* fOutTrackEvent;
#endif

    void InitFromConfigFile();

    void Initialize();

    void LoadDefaultConfig();

   protected:
    // add here the members of your event process

   std::vector<alphaTrackHit> smoothTrack(std::vector <alphaTrackHit> &hits);
   Double_t fTrackBalance = 0.65; //Max track energy is used to define the minimun energy fTrackBalance*fTotEnergy of the total energy to assume a single track event, if the condition is not fulfill the event is rejected

   public:
    any GetInputEvent() { return fTrackEvent; }
    any GetOutputEvent() { return fOutTrackEvent; }

    void InitProcess();
    TRestEvent* ProcessEvent(TRestEvent* eventInput);
    void EndProcess();

    void LoadConfig(std::string cfgFilename);

    void PrintMetadata() {
        BeginPrintProcess();
        metadata<<"Track Balance: "<<fTrackBalance<<endl;
        EndPrintProcess();
    }

    TString GetProcessName() { return (TString) "alphaTrackAna"; }
    //       Double_t GetCalibFactor();           ///< Calibration factor is
    //       found.

    // Constructor
    TRestTrackAlphaAnalysisProcess();
    TRestTrackAlphaAnalysisProcess(char* cfgFileName);
    // Destructor
    ~TRestTrackAlphaAnalysisProcess();

    ClassDef(TRestTrackAlphaAnalysisProcess, 1);
                   // Template for a REST "event process" class inherited from TRestEventProcess
};
#endif
