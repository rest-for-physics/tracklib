///______________________________________________________________________________
///______________________________________________________________________________
///______________________________________________________________________________
///
///
///             RESTSoft : Software for Rare Event Searches with TPCs
///
///             TRestTrack.h
///
///             Event class to store signals form simulation and acquisition
///             events
///
///             oct 2015:   First concept
///                 Created as part of the conceptualization of existing REST
///                 software.
///                 Javier Gracia
///_______________________________________________________________________________

#ifndef RestCore_TRestTrack
#define RestCore_TRestTrack

#include <TObject.h>
#include <TRestVolumeHits.h>
#include <TVector3.h>

#include <iostream>

class TRestTrack : public TObject {
   protected:
    Int_t fTrackID;         ///< Track ID
    Int_t fParentID;        ///< Parent ID
    Double_t fTrackEnergy;  ///< Total energy of the track
    Double_t fTrackLength;  ///< Total length of the track

    // Hit volumes
    TRestVolumeHits fVolumeHits;  ///< Hit volumes that define a track

   public:
    void Initialize();

    // Setters
    void SetTrackID(Int_t sID) { fTrackID = sID; }
    void SetParentID(Int_t pID) { fParentID = pID; }

    void SetVolumeHits(TRestVolumeHits hits);
    void RemoveVolumeHits();

    Bool_t isXY() { return (&fVolumeHits)->areXY(); }
    Bool_t isXZ() { return (&fVolumeHits)->areXZ(); }
    Bool_t isYZ() { return (&fVolumeHits)->areYZ(); }
    Bool_t isXYZ() { return (&fVolumeHits)->areXYZ(); }

    // Getters
    inline Int_t GetTrackID() const { return fTrackID; }
    inline Int_t GetParentID() const { return fParentID; }
    inline Double_t GetEnergy() const { return fTrackEnergy; }
    inline Double_t GetTrackEnergy() const { return fTrackEnergy; }

    Double_t GetLength(Bool_t update = true) { return GetTrackLength(update); }
    Double_t GetTrackLength(Bool_t update = true) {
        if (update) fTrackLength = fVolumeHits.GetTotalDistance();
        return fTrackLength;
    }

    inline Double_t GetMaximumDistance() const { return fVolumeHits.GetMaximumHitDistance(); }
    inline Double_t GetMaximumDistance2() const { return fVolumeHits.GetMaximumHitDistance2(); }
    inline Double_t GetVolume() const { return fVolumeHits.GetMaximumHitDistance2(); }

    inline TVector3 GetMeanPosition() const { return fVolumeHits.GetMeanPosition(); }

    TRestVolumeHits* GetVolumeHits() { return &fVolumeHits; }
    TRestHits* GetHits() { return (TRestHits*)&fVolumeHits; }
    inline Int_t GetNumberOfHits() { return GetVolumeHits()->GetNumberOfHits(); }

    void PrintTrack(Bool_t fullInfo = true);

    void GetBoundaries(TVector3& orig, TVector3& end);

    // Construtor
    TRestTrack();
    // Destructor
    ~TRestTrack();

    ClassDef(TRestTrack, 2);
};
#endif
