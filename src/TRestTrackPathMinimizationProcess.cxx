//////////////////////////////////////////////////////////////////////////
///
///             RESTSoft : Software for Rare Event Searches with TPCs
///
///             TRestTrackPathMinimizationProcess.cxx
///
///             Jan 2016:   First concept (Javier Galan)
///
//////////////////////////////////////////////////////////////////////////

#include "TRestTrackPathMinimizationProcess.h"

using namespace std;

ClassImp(TRestTrackPathMinimizationProcess);

TRestTrackPathMinimizationProcess::TRestTrackPathMinimizationProcess() { Initialize(); }

TRestTrackPathMinimizationProcess::~TRestTrackPathMinimizationProcess() { delete fOutputTrackEvent; }

void TRestTrackPathMinimizationProcess::Initialize() {
    SetSectionName(this->ClassName());
    SetLibraryVersion(LIBRARY_VERSION);

    fInputTrackEvent = nullptr;
    fOutputTrackEvent = new TRestTrackEvent();
}

void TRestTrackPathMinimizationProcess::InitProcess() {}

TRestEvent* TRestTrackPathMinimizationProcess::ProcessEvent(TRestEvent* inputEvent) {
    fInputTrackEvent = (TRestTrackEvent*)inputEvent;

    if (this->GetVerboseLevel() >= TRestStringOutput::REST_Verbose_Level::REST_Debug)
        cout << "TRestTrackPathMinimizationProcess. Number of tracks : "
             << fInputTrackEvent->GetNumberOfTracks() << endl;

    // Copying the input tracks to the output track
    for (int tck = 0; tck < fInputTrackEvent->GetNumberOfTracks(); tck++)
        fOutputTrackEvent->AddTrack(fInputTrackEvent->GetTrack(tck));

    for (int tck = 0; tck < fInputTrackEvent->GetNumberOfTracks(); tck++) {
        if (!fInputTrackEvent->isTopLevel(tck)) continue;
        Int_t tckId = fInputTrackEvent->GetTrack(tck)->GetTrackID();

        TRestVolumeHits* hits = fInputTrackEvent->GetTrack(tck)->GetVolumeHits();
        const int nHits = hits->GetNumberOfHits();

        /* {{{ Debug output */
        if (this->GetVerboseLevel() >= TRestStringOutput::REST_Verbose_Level::REST_Debug) {
            Int_t pId = fInputTrackEvent->GetTrack(tck)->GetParentID();
            cout << "Track : " << tck << " TrackID : " << tckId << " ParentID : " << pId << endl;
            cout << "-----------------" << endl;
            hits->PrintHits();
            cout << "-----------------" << endl;
        }
        /* }}} */

        RESTDebug << "hits : " << nHits << RESTendl;

        std::vector<int> bestPath(nHits);
        for (int i = 0; i < nHits; i++) bestPath[i] = i;  // Initialize

        if (fMinMethod == "bruteforce")
            BruteForce(hits, bestPath);
        else if (fMinMethod == "closestN")
            NearestNeighbour(hits, bestPath);
        else
            HeldKarp(hits, bestPath);  // default

        TRestVolumeHits bestHitsOrder;
        for (const auto& v : bestPath) bestHitsOrder.AddHit(*hits, v);

        // TODO We must also copy other track info here
        TRestTrack bestTrack;
        bestTrack.SetTrackID(fOutputTrackEvent->GetNumberOfTracks() + 1);
        bestTrack.SetParentID(tckId);
        bestTrack.SetVolumeHits(bestHitsOrder);
        fOutputTrackEvent->AddTrack(&bestTrack);
    }

    fOutputTrackEvent->SetLevels();
    return fOutputTrackEvent;
}

///////////////////////////////////////////////
/// \brief Return the index with the shortest path solving Travelling Salesman Problem (TSP) using
/// nearest neighbour algorithm, complexity is n2*n, it doesn't guarantee that the minimum
/// path is reached, but the solution should be within 25% of the distance to the minimum path
///
void TRestTrackPathMinimizationProcess::NearestNeighbour(TRestVolumeHits* hits, std::vector<int>& bestPath) {
    const int nHits = hits->GetNumberOfHits();
    vector<vector<double>> dist(nHits,vector<double>(nHits));
    RESTDebug << "Nhits " << nHits << RESTendl;

    if (nHits < 3) return;

    for (int i = 0; i < nHits; i++) {
        for (int j = i; j < nHits; j++) {
            if (i == j) {
                dist[i][j] = 0;  // Distance within the same vertex
            } else {
                dist[i][j] = hits->GetDistance(i, j);  // Distance within two hits vertex
                dist[j][i] = dist[i][j];               // It is symmetric
            }
        }
    }

    double min_path = 1E9;
    std::vector<int> current_path(nHits);

    // Find the shortest path starting from all vertex
    for (int s = 0; s < nHits; s++) {
        std::vector<int> vertex(nHits - 1);

        int j = 0;
        for (int i = 0; i < nHits; i++) {
            if (i == s) continue;
            vertex[j] = i;
            j++;
        }

        // store current Path weight(cost)
        double current_pathweight = 0;
        current_path[0] = s;

        // compute current path weight
        int k = s;
        while (!vertex.empty()) {
            int closestN;
            int index = 0, bestIndex;
            double minDist = 1E9;
            for (const auto& v : vertex) {
                const double d = dist[k][v];
                if (d < minDist) {
                    minDist = d;
                    current_pathweight += d;
                    closestN = v;
                    bestIndex = index;
                }
                index++;
            }
            k = closestN;
            current_path[nHits - vertex.size()] = k;
            vertex.erase(vertex.begin() + bestIndex);
        }
        if (fCyclic) current_pathweight += dist[k][s];

        if (current_pathweight < min_path) {
            min_path = current_pathweight;
            bestPath = current_path;
        }
    }

    if (this->GetVerboseLevel() >= TRestStringOutput::REST_Verbose_Level::REST_Debug) {
        cout << "Min path ";
        for (const auto& v : bestPath) cout << v << " ";
        cout << " " << min_path << endl;
    }
}

///////////////////////////////////////////////
/// \brief This function return the index with the shortest path solving
/// Travelling Salesman Problem (TSP) using brute force,
/// algorithm complexity is n!, the minimum path is guarantee
///
void TRestTrackPathMinimizationProcess::BruteForce(TRestVolumeHits* hits, std::vector<int>& bestPath) {
    const int nHits = hits->GetNumberOfHits();
    vector<vector<double>> dist(nHits, vector<double>(nHits));
    RESTDebug << "Nhits " << nHits << RESTendl;

    if (nHits < 3) return;

    for (int i = 0; i < nHits; i++) {
        for (int j = i; j < nHits; j++) {
            if (i == j) {
                dist[i][j] = 0;  // Distance within the same vertex
            } else {
                dist[i][j] = hits->GetDistance(i, j);  // Distance within two hits vertex
                dist[j][i] = dist[i][j];               // It is symmetric
            }
        }
    }

    if (this->GetVerboseLevel() >= TRestStringOutput::REST_Verbose_Level::REST_Debug) {
        for (int i = 0; i < nHits; i++) {
            for (int j = 0; j < nHits; j++) {
                cout << dist[i][j] << " ";
            }
            cout << "\n";
        }
    }

    double min_path = 1E9;

    // Find the shortest path starting from all vertex
    for (int s = 0; s < nHits; s++) {
        // store all vertex apart from source vertex
        std::vector<int> vertex(nHits - 1);
        int index = 0;
        for (int i = 0; i < nHits; i++) {
            if (i == s) continue;
            vertex[index] = i;
            // debug<<index<<" "<<i<<endl;
            index++;
        }

        // store minimum weight Hamiltonian Cycle.
        do {
            // store current Path weight(cost)
            double current_pathweight = 0;

            // compute current path weight
            int k = s;
            for (const auto& v : vertex) {
                current_pathweight += dist[k][v];
                k = v;
            }
            // In case of cyclic
            if (fCyclic) current_pathweight += dist[k][s];

            // update minimum
            if (current_pathweight < min_path) {
                min_path = current_pathweight;
                bestPath[0] = s;
                for (int i = 0; i < vertex.size(); i++) bestPath[i + 1] = vertex[i];
            }

            // Could be optimized using the shortest neighbour after computing Floyd Warshall Algorithm
        } while (std::next_permutation(vertex.begin(), vertex.end()));
    }

    if (this->GetVerboseLevel() >= TRestStringOutput::REST_Verbose_Level::REST_Debug) {
        cout << "Min path ";
        for (const auto& v : bestPath) cout << v << " ";
        cout << " " << min_path << endl;
    }
}

///////////////////////////////////////////////
/// \brief This function eturn the index with the shortest path
/// Note that this method calls external tsp library and
/// assumes cyclic data
///
void TRestTrackPathMinimizationProcess::HeldKarp(TRestVolumeHits* hits, std::vector<int>& bestPath) {
    const int nHits = hits->GetNumberOfHits();

    if (nHits < 4) return;

    Int_t segment_count = nHits * (nHits - 1) / 2;
    int* elen = (int*)malloc(segment_count * sizeof(int));
    /*
    double *enBetween = (double *) malloc( segment_count * sizeof( double ) );

    Double_t lenghtReduction = 0.25;
    Double_t fTubeRadius = 1.;

    Double_t energyIntegral = 0;
    Int_t integratedSegments = 0;
    */

    int k = 0;
    vector<int> bestP(nHits);
    Int_t rval = 0;
    for (int i = 0; i < nHits; i++) {
        bestP[i] = i;
        for (int j = 0; j < i; j++) {
            TVector3 x0, x1, pos0, pos1;

            x0 = hits->GetPosition(i);
            x1 = hits->GetPosition(j);

            Double_t distance = hits->GetDistance(i, j);

            /* This can be used to weight the segments with the number of hits
            between nodes
             *
            pos0 = lenghtReduction * ( x1-x0 ) + x0;
            pos1 = (1-lenghtReduction) * ( x1-x0 ) + x0;

            Double_t energyBetween = originHits->GetEnergyInCylinder( pos0, pos1,
            fTubeRadius );

            if( GetVerboseLevel() >= REST_Extreme )
            {
                cout << "Distance : " << distance << endl;
                cout << "Segment energy (" << i << " , " << j << " ) : " <<
            energyBetween << endl;
            }

            if( energyBetween > 0 )
            {
                energyIntegral += energyBetween;
                integratedSegments++;
            }

            */

            elen[k] = (int)(100. * distance);

            // enBetween[k] = energyBetween;
            k++;
        }
    }

    /* For the weighting of segments
     *
    Double_t meanHitsConnection = energyIntegral/integratedSegments;
    if( GetVerboseLevel() >= REST_Debug )
        cout << "energyIntegral : " << energyIntegral << " integratedSegments
    : " << integratedSegments << endl;

    for( int n = 0; n < k; n++ )
    {
        if( enBetween[n] > meanHitsConnection )
            elen[n] = elen[n] / 1.5;

        if( enBetween[n] < meanHitsConnection/3 )
            elen[n] = elen[n] * 2.5;
    }

    cout << "Mean hits : " << meanHitsConnection << endl;

    if( GetVerboseLevel() >= REST_Extreme )
        GetChar();
    */

    if (GetVerboseLevel() >= TRestStringOutput::REST_Verbose_Level::REST_Extreme) {
        for (int n = 0; n < segment_count; n++) cout << "n : " << n << " elen : " << elen[n] << endl;
        GetChar();
    }

    rval = TrackMinimization_segment(nHits, elen, &bestP[0]);

    /**** Just Printing
    for( int i = 0; i < hits->GetNumberOfHits()-1; i++ )
    {
        TVector3 x0, x1, pos0, pos1;

        x0 = hits->GetPosition( i );
        x1 = hits->GetPosition( i+1 );

        pos0 = lenghtReduction * ( x1-x0 ) + x0;
        pos1 = (1-lenghtReduction) * ( x1-x0 ) + x0;

        Double_t distance = (x1-x0).Mag();
        Double_t energyBetween = originHits->GetEnergyInCylinder( pos0, pos1,
    fTubeRadius );

        cout << "Hit : " << i << " x : " << hits->GetX(i) <<  " y : " <<
    hits->GetY(i) <<  " z : " << hits->GetZ(i) << endl; cout << "Distance : "
    << distance << " Energy between : " << energyBetween << endl;
    }
    cout << "Hit : " << nHits-1 << " x : " << hits->GetX(nHits-1) <<  " y : "
    << hits->GetY(nHits-1) <<  " z : " << hits->GetZ(nHits-1) << endl;
    ***** */

    free(elen);
    // free( enBetween );

    if (GetVerboseLevel() >= TRestStringOutput::REST_Verbose_Level::REST_Extreme) GetChar();

    if (rval != 0) {
        if (GetVerboseLevel() >= TRestStringOutput::REST_Verbose_Level::REST_Warning) {
            cout << "REST WARNING. TRestTrackPathMinimizationProcess. HELDKARP FAILED." << endl;
        }
        fOutputTrackEvent->SetOK(false);
        return;
    }

    for (int i = 0; i < nHits; i++) bestPath[i] = bestP[i];
}

void TRestTrackPathMinimizationProcess::EndProcess() {}
