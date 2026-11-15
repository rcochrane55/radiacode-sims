//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
//
/// \file DetectorConstruction.hh
/// \brief Definition of the DetectorConstruction class

#ifndef DetectorConstruction_h
#define DetectorConstruction_h 1

#include "G4VUserDetectorConstruction.hh"
#include "G4ThreeVector.hh"
#include "globals.hh"

class G4VPhysicalVolume;
class G4LogicalVolume;
class DetectorConstructionMessenger;

// two types of source geometries

enum class SourceGeometry
{
  Marinelli,
  Disk
};

/// Detector construction class to define materials and geometry.

class DetectorConstruction : public G4VUserDetectorConstruction
{
  public:
    DetectorConstruction();
    virtual ~DetectorConstruction();

    virtual G4VPhysicalVolume* Construct();
    
    G4LogicalVolume* GetScoringVolume() const 
    { 
        return fScoringVolume; 
    }

    //detector config setters
    void SetCrystalDimensions(const G4ThreeVector& dimensions);
    void SetCrystalPosition(const G4ThreeVector& position);

    void SetSiPMSide(G4double side);
    void SetSiPMThickness(G4double thickness);

    void SetReflectorThickness(G4double thickness);
    void SetCladdingThickness(G4double thickness);

    // source config setters
    void SetMarinelliMeshFile(const G4String& filename);
    void SetMarinelliPosition(const G4ThreeVector& position);
    
    void SetDiskDiameter(G4double diameter);
    void SetDiskThickness(G4double thickness);
    void SetDiskPosition(const G4ThreeVector& position);
    void SetDiskRotation(const G4ThreeVector& rotation);

    void SetSourceType(SourceGeometry geometry);

  private:
    // params describing detector
    struct DetectorConfig
    {
      G4double scintWidth;
      G4double scintHeight;
      G4double scintLength;

      G4ThreeVector scintPosition;

      G4double SiPMSide;
      G4double SiPMThickness;

      G4double reflectorThickness;
      G4double claddingThickness;
    };

    struct SourceConfig
    {
      SourceGeometry geometry;

      G4String marinelliMeshFile;
      G4ThreeVector marinelliPosition;

      G4double diskDiameter;
      G4double diskThickness;

      G4ThreeVector diskPosition;
      G4ThreeVector diskRotation;
    };

    //material construction
    void DefineMaterials();

    //geometry construction
    void ConstructDetector(
        G4LogicalVolume* logicWorld,
        G4bool checkOverlaps
      );

    void ConstructSource(
        G4LogicalVolume* logicWorld,
        G4bool checkOverlaps
      );

    void ConstructMarinelliSource(
        G4LogicalVolume* logicWorld,
        G4bool checkOverlaps
      );

    void ConstructDiskSource(
        G4LogicalVolume* logicWorld,
        G4bool checkOverlaps
      );

    DetectorConfig fDetectorConfig;
    SourceConfig fSourceConfig;
    G4LogicalVolume*  fScoringVolume;
    DetectorConstructionMessenger* fMessenger;
};

#endif

