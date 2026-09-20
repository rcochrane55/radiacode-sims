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
/// \file DetectorConstruction.cc
/// \brief Implementation of the DetectorConstruction class

#include "CADMesh.hh"
#include "DetectorConstruction.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4RunManager.hh"
#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4Cons.hh"
#include "G4Orb.hh"
#include "G4Sphere.hh"
#include "G4Tubs.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "G4SubtractionSolid.hh"
#include "G4OpticalSurface.hh"
#include "G4MaterialPropertiesTable.hh"
#include "G4PhysicalConstants.hh"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>


DetectorConstruction::DetectorConstruction()
: G4VUserDetectorConstruction(),
  fScoringVolume(0)
{ }

DetectorConstruction::~DetectorConstruction()
{ }


G4VPhysicalVolume* DetectorConstruction::Construct()
{  
  // Get nist material manager
  G4NistManager* nist = G4NistManager::Instance();
  
     
  // Option to switch on/off checking of volumes overlaps
  //
  G4bool checkOverlaps = true;

  // World
  G4double world_sizeXY = 30*cm;
  G4double world_sizeZ  = 30*cm;
  G4Material* world_mat = nist->FindOrBuildMaterial("G4_AIR");
  
  G4Box* solidWorld =  new G4Box(
  "World",                       //its name
  0.5*world_sizeXY, 0.5*world_sizeXY, 0.5*world_sizeZ
  );     //its size
      
  G4LogicalVolume* logicWorld =                         
    new G4LogicalVolume(solidWorld,          //its solid
                        world_mat,           //its material
                        "World");            //its name
                                   
  G4VPhysicalVolume* physWorld = 
    new G4PVPlacement(0,                     //no rotation
                      G4ThreeVector(),       //at (0,0,0)
                      logicWorld,            //its logical volume
                      "World",               //its name
                      0,                     //its mother  volume
                      false,                 //no boolean operation
                      0,                     //copy number
                      checkOverlaps);        //overlaps checking

G4ThreeVector marinelliPos = G4ThreeVector(0*cm, 0*cm, 0*cm);

// new G4PVPlacement(0, marinelliPos, marinelliLV, "marinelli", logicWorld, false, 0, checkOverlaps);

// CsI scint assembly

  G4double scintLength = 1.*cm ;
  G4double scintWidth = 1.*cm ;
  G4double scintHeight = 1.*cm ;

  G4Element* Ti = nist->FindOrBuildElement("Ti");
  G4Element* O = nist->FindOrBuildElement("O");
  G4Material* TiO2 = new G4Material("TiO2", 4.23*g/cm3, 2);

  TiO2->AddElement(Ti, 1);
  TiO2->AddElement(O, 2);

  G4double crystalSize = 1.0*cm;
  G4double siPMSide = 0.6*cm;
  G4double siPMThickness = 0.04*cm;
  G4double reflectorThickness = 0.04*cm;
  G4double claddingThickness = 0.16*cm;

// define pos, mat, solids for cladding
  G4double reflectorOuter = 1*cm + 2.0*reflectorThickness;
  G4double claddingOuter = (reflectorOuter + 2.0*claddingThickness);

  auto claddingOuterSolid = new G4Box("CladdingOuter", claddingOuter/2, claddingOuter/2, claddingOuter/2);
  auto claddingInnerSolid = new G4Box("CladdingInner", reflectorOuter/2, reflectorOuter/2, reflectorOuter/2);
  auto claddingSolid = new G4SubtractionSolid("Cladding", claddingOuterSolid, claddingInnerSolid);

  G4Material* claddingMat = nist->FindOrBuildMaterial("G4_POLYETHYLENE");

  auto claddingLV = new G4LogicalVolume(claddingSolid, claddingMat, "CladdingLV");

// define position and solids for reflector
  auto sideReflectorSolid1 = new G4Box("SideReflector1", reflectorThickness/2, crystalSize/2, crystalSize/2);
  auto sideReflectorSolid2 = new G4Box("SideReflector2", reflectorThickness/2, crystalSize/2, crystalSize/2);
  auto sideReflectorSolid3 = new G4Box("SideReflector3", crystalSize/2,reflectorThickness/2, crystalSize/2);
  auto topReflectorSolid = new G4Box("TopReflector", crystalSize/2,crystalSize/2, reflectorThickness/2);
  auto bottomReflectorSolid = new G4Box("BottomReflector", crystalSize/2, crystalSize/2, reflectorThickness/2);;

  auto SideReflectorLV1 = new G4LogicalVolume(sideReflectorSolid1, TiO2, "SideReflectorLogical1");
  auto SideReflectorLV2 = new G4LogicalVolume(sideReflectorSolid2, TiO2, "SideReflectorLogical2");
  auto SideReflectorLV3 = new G4LogicalVolume(sideReflectorSolid3, TiO2, "SideReflectorLogical3");
  auto TopReflectorLV = new G4LogicalVolume(topReflectorSolid, TiO2, "TopReflectorLogical");
  auto BottomReflectorLV = new G4LogicalVolume(bottomReflectorSolid, TiO2, "BottomReflectorLogical");

  G4double REFLECTIVITY = 0.947;
  auto reflectorSurface = new G4OpticalSurface("ReflectorSurface");
  reflectorSurface->SetType(dielectric_metal);
  reflectorSurface->SetFinish(ground);
  reflectorSurface->SetModel(unified);

  G4MaterialPropertiesTable* reflectorMPT = new G4MaterialPropertiesTable();
  reflectorMPT->AddConstProperty("REFLECTIVITY", REFLECTIVITY);
  reflectorSurface->SetMaterialPropertiesTable(reflectorMPT); 

  auto SiPMSolid = new G4Box("SiPM", siPMSide/2, siPMThickness/2,siPMSide/2);
  auto SiPMLV = new G4LogicalVolume(SiPMSolid, nist->FindOrBuildMaterial("G4_Si"), "SiPMLV");

  auto  white = new G4VisAttributes(G4Colour(1.0, 1.0, 1.0)); // white
  auto red = new G4VisAttributes(G4Colour(1.0, 0.0, 0.0)); // red
  auto blue = new G4VisAttributes(G4Colour(0.0, 0.0, 1.0)); // blue
  auto green = new G4VisAttributes(G4Colour(0.0, 1.0, 0.0)); // green
  SideReflectorLV1->SetVisAttributes(white);
  SideReflectorLV2->SetVisAttributes(white);
  SideReflectorLV3->SetVisAttributes(white);
  TopReflectorLV->SetVisAttributes(white);
  BottomReflectorLV->SetVisAttributes(white);
  SiPMLV->SetVisAttributes(red);
  claddingLV->SetVisAttributes(green);

  G4Material* scintMat = nist->FindOrBuildMaterial("G4_CESIUM_IODIDE");

  G4MaterialPropertiesTable* CsI_MPT = new G4MaterialPropertiesTable();

  G4double rindex = 1.79;
  G4double scintillationYield = 54000./MeV;
  G4double decayTime = 1000.*ns;
  std::vector<G4double> photonEnergy;
  std::vector<G4double> emission; 

  std::ifstream file("emission_spectrum_data.csv");

  std::string line;
  while (std::getline(file, line)) {
    std::stringstream ss(line);

    double wavelength_nm;
    double intensity;
    char comma;
    ss >> wavelength_nm >> comma >> intensity;
    G4double energy_eV = (h_Planck * c_light) / (wavelength_nm * nm);
    photonEnergy.push_back(energy_eV);
    emission.push_back(intensity);
  }

  std::vector<G4double> absorptionEnergy;
  std::vector<G4double> absorptionLength;

  std::ifstream absorptionFile("absorption_length_data.csv");

  std::string absorptionLine;
  while (std::getline(absorptionFile, absorptionLine)) {
    std::stringstream ss(absorptionLine);
    
    double energy_eV;
    double length_mm;
    char comma;
    ss >> energy_eV >> comma >> length_mm;

    absorptionEnergy.push_back(energy_eV * eV);
    absorptionLength.push_back(length_mm * mm);
  }

  // CsI_MPT->AddProperty("RINDEX", energies, rindex, nEntries);
  // CsI_MPT->AddProperty("ABSLENGTH", energies, absorption, nEntries);
  // CsI_MPT->AddProperty("SCINTILLATIONCOMPONENT1", photonEnergy, scintSpectrum, nEntries);
  CsI_MPT->AddConstProperty("RINDEX", rindex);
  CsI_MPT->AddConstProperty("SCINTILLATIONYIELD", scintillationYield);
  CsI_MPT->AddConstProperty("SCINTILLATIONTIMECONSTANT1", decayTime);
  CsI_MPT->AddProperty("ABSLENGTH", absorptionEnergy.data(), absorptionLength.data(), absorptionEnergy.size());
  CsI_MPT->AddProperty("SCINTILLATIONCOMPONENT1", photonEnergy.data(), emission.data(), photonEnergy.size());
  CsI_MPT->AddConstProperty("SCINTILLATIONYIELD1", 1.0);
  CsI_MPT->AddConstProperty("RESOLUTIONSCALE", 1.0);

  scintMat->SetMaterialPropertiesTable(CsI_MPT);

  G4ThreeVector scintPos = G4ThreeVector(0, 0, 0*cm);

  G4Box * scintSolid = new G4Box(
      "scint",
      scintWidth/2, // half width in x
      scintHeight/2, // half width in y
      scintLength/2     // half length in z
    );

  
  G4LogicalVolume * scintLV = new G4LogicalVolume(
      scintSolid, // its solid
      scintMat,   // its material
      "scint"     // name
    ) ;

    scintLV->SetVisAttributes(blue);
    
  //auto activeSolid = new G4Box("Active", activeSide/2, activeSide/2, activeSide/2);
  //auto activeLV = new G4LogicalVolume(activeSolid, scintMat, "ActiveLV");

  new G4PVPlacement(0,                       //no rotation
                    scintPos,                    //at position
                    scintLV,             //its logical volume
                    "scint",                //its name
                    logicWorld,                //its mother  volume
                    false,                   //no boolean operation
                    0,                       //copy number
                    checkOverlaps);          //overlaps checking

  G4double offset = crystalSize/2 + reflectorThickness/2;

  new G4PVPlacement(nullptr, scintPos + G4ThreeVector(-offset, 0, 0), SideReflectorLV1, "SideReflector1", logicWorld, false, 0, checkOverlaps);
  new G4PVPlacement(nullptr, scintPos + G4ThreeVector(offset, 0, 0), SideReflectorLV2, "SideReflector2", logicWorld, false, 0, checkOverlaps);
  new G4PVPlacement(nullptr, scintPos + G4ThreeVector(0, -offset, 0), SideReflectorLV3, "SideReflector3", logicWorld, false, 0, checkOverlaps);
  new G4PVPlacement(nullptr, scintPos + G4ThreeVector(0, 0, offset), TopReflectorLV, "TopReflector", logicWorld, false, 0, checkOverlaps);
  new G4PVPlacement(nullptr, scintPos + G4ThreeVector(0, 0, -offset), BottomReflectorLV, "BottomReflector", logicWorld, false, 0, checkOverlaps);
  new G4PVPlacement(nullptr, scintPos + G4ThreeVector(0, offset, 0), SiPMLV, "SiPM", logicWorld, false, 0, checkOverlaps);

  new G4PVPlacement(nullptr, scintPos, claddingLV, "Cladding", logicWorld, false, 0, checkOverlaps);

/* G4LogicalVolume * marinelliLV = new G4LogicalVolume(
  marinelliSolid, // its solid
  KCl,   // its material
  "marinelli"     // name
) ; */

/* G4ThreeVector marinelliPos = G4ThreeVector(0*cm, 0*cm, -0.2*cm);
new G4PVPlacement(0,                       //no rotation
  marinelliPos,                    //at position
  marinelliLV,             //its logical volume
  "marinelli",                //its name
  logicWorld,                //its mother  volume
  false,                   //no boolean operation
  0,                       //copy number
  checkOverlaps);          //overlaps checking */

//G4cout << "Mass = "
//       << marinelliLV->GetMass(true,true)/g
//       << " g" << G4endl;

  // Set scint as scoring volume
  //
  fScoringVolume = scintLV;

  //
  //always return the physical World
  //
  return physWorld;
}
