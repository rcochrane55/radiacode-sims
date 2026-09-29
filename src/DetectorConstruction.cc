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
#include "G4LogicalBorderSurface.hh"
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

  G4cout << "Defining scint dimensions" << G4endl;
  G4double scintLength = 1.*cm ;
  G4double scintWidth = 1.*cm ;
  G4double scintHeight = 1.*cm ;
  G4cout << "scint dimensions defined" << G4endl;

  G4cout << "defining TiO2" << G4endl;
  G4Element* Ti = nist->FindOrBuildElement("Ti");
  G4Element* O = nist->FindOrBuildElement("O");
  G4Material* TiO2 = new G4Material("TiO2", 4.23*g/cm3, 2);
  G4cout << "TiO2 defined" << G4endl;

  TiO2->AddElement(Ti, 1);
  TiO2->AddElement(O, 2);

  G4double crystalSize = 1.0*cm;
  G4double siPMPackageSide = 0.7 * cm;
  G4double siPMSide = 0.6*cm;
  G4double siPMThickness = 0.03*cm;
  G4double reflectorThickness = 0.04*cm;
  G4double claddingThickness = 0.16*cm;

// ============================================================
// Optical grease - approximate EJ-550
// ============================================================

G4double greaseDensity = 1.06 * g/cm3;

G4Element* elH  = nist->FindOrBuildElement("H");
G4Element* elC  = nist->FindOrBuildElement("C");
G4Element* elO  = nist->FindOrBuildElement("O");
G4Element* elSi = nist->FindOrBuildElement("Si");

G4Material* opticalGrease =
    new G4Material(
        "OpticalGrease",
        greaseDensity,
        4
    );

opticalGrease->AddElement(elC,  2);
opticalGrease->AddElement(elH,  6);
opticalGrease->AddElement(elO,  1);
opticalGrease->AddElement(elSi, 1);

G4MaterialPropertiesTable* greaseMPT =
    new G4MaterialPropertiesTable();

G4double greasePhotonEnergy[] = {
    1.0 * eV,
    4.5 * eV
};

G4double greaseRIndex[] = {
    1.46,
    1.46
};

G4double greaseAbsLength[] = {
    10.0 * m,
    10.0 * m
};

greaseMPT->AddProperty(
    "RINDEX",
    greasePhotonEnergy,
    greaseRIndex,
    2
);

greaseMPT->AddProperty(
    "ABSLENGTH",
    greasePhotonEnergy,
    greaseAbsLength,
    2
);

opticalGrease->SetMaterialPropertiesTable(greaseMPT); 

G4cout << "Defining grease dimensions" << G4endl;
  G4double greaseLength = 0.6*cm ;
  G4double greaseWidth = 0.6*cm ;
  G4double greaseHeight = 0.01*cm ;
  G4cout << "grease defined" << G4endl;

// define pos, mat, solids for cladding
  G4double reflectorOuter = 1*cm + 2.0*reflectorThickness;
  //G4double claddingOuter = (reflectorOuter + 2.0*claddingThickness);

  //auto claddingOuterSolid = new G4Box("CladdingOuter", claddingOuter/2, claddingOuter/2, claddingOuter/2);
  //auto claddingInnerSolid = new G4Box("CladdingInner", reflectorOuter/2, reflectorOuter/2, reflectorOuter/2);
  //auto claddingSolid = new G4SubtractionSolid("Cladding", claddingOuterSolid, claddingInnerSolid);

  //G4Material* claddingMat = nist->FindOrBuildMaterial("G4_POLYETHYLENE");

  //auto claddingLV = new G4LogicalVolume(claddingSolid, claddingMat, "CladdingLV");

// define position and solids for reflector
  G4cout << "defining reflector solids" << G4endl;
  auto sideReflectorSolid1 = new G4Box("SideReflector1", reflectorThickness/2, crystalSize/2, crystalSize/2);
  auto sideReflectorSolid2 = new G4Box("SideReflector2", reflectorThickness/2, crystalSize/2, crystalSize/2);
  auto sideReflectorSolid3 = new G4Box("SideReflector3", crystalSize/2,reflectorThickness/2, crystalSize/2);
  auto topReflectorSolid = new G4Box("TopReflector", crystalSize/2,crystalSize/2, reflectorThickness/2);
  auto bottomReflectorSolid = new G4Box("BottomReflector", crystalSize/2, crystalSize/2, reflectorThickness/2);
  G4cout << "reflector solids defined" << G4endl;

  G4cout << "creating reflector LVs" << G4endl;
  auto SideReflectorLV1 = new G4LogicalVolume(sideReflectorSolid1, TiO2, "SideReflectorLogical1");
  auto SideReflectorLV2 = new G4LogicalVolume(sideReflectorSolid2, TiO2, "SideReflectorLogical2");
  auto SideReflectorLV3 = new G4LogicalVolume(sideReflectorSolid3, TiO2, "SideReflectorLogical3");
  auto TopReflectorLV = new G4LogicalVolume(topReflectorSolid, TiO2, "TopReflectorLogical");
  auto BottomReflectorLV = new G4LogicalVolume(bottomReflectorSolid, TiO2, "BottomReflectorLogical");
  G4cout << "reflector LVs created" << G4endl;

  G4cout << "defining TiO2 reflector properties" << G4endl;
  G4double reflectorEnergy[] = {
    1.0 * eV, 
    4.5 * eV
  };
  G4double reflectivity[] = {
    0.95,
    0.95
  };
  auto reflectorSurface = new G4OpticalSurface("ReflectorSurface");
  reflectorSurface->SetType(dielectric_dielectric);
  reflectorSurface->SetFinish(polished);
  reflectorSurface->SetModel(unified);

  G4MaterialPropertiesTable* reflectorMPT = new G4MaterialPropertiesTable();
  reflectorMPT->AddProperty("REFLECTIVITY", reflectorEnergy, reflectivity, 2);
  reflectorSurface->SetMaterialPropertiesTable(reflectorMPT); 
  G4cout << "TiO2 reflector properties defined" << G4endl;

  G4cout << "defining ESR film reflector properties" << G4endl;
  G4Material* ESRMaterial = nist->FindOrBuildMaterial("G4_MYLAR");
  const G4int nESR = 2;

  G4double ESRPhotonEnergy[nESR] = {
      1.0 * eV,
      4.5 * eV
  };

  G4double ESRReflectivity[nESR] = {
      0.98,
      0.98
  };

  G4double ESREfficiency[nESR] = {
      0.0,
      0.0
  };

  auto ESR_MPT = new G4MaterialPropertiesTable();

  ESR_MPT->AddProperty(
      "REFLECTIVITY",
      ESRPhotonEnergy,
      ESRReflectivity,
      nESR
  );

  ESR_MPT->AddProperty(
      "EFFICIENCY",
      ESRPhotonEnergy,
      ESREfficiency,
      nESR
  );

  auto ESRSurface = new G4OpticalSurface("ESRSurface");

  ESRSurface->SetType(dielectric_metal);
  ESRSurface->SetModel(unified);
  ESRSurface->SetFinish(polished);

  ESRSurface->SetMaterialPropertiesTable(ESR_MPT);
  G4cout << "ESR film reflector properties defined" << G4endl;

  G4cout << "defining ESR film solid and LV" << G4endl;
  G4double esrThickness = 0.065 * mm;

  auto ESRFull = new G4Box("ESRFull", 5.0 * mm, esrThickness/2.0, 5.0 * mm);
  auto SiPMCutout = new G4Box("SiPMCutout", 3.5 * mm, esrThickness, 3.5 * mm);
  G4ThreeVector cutoutOffset(0.0 * mm, 0.0 * mm, 0.5 * mm);
  auto ESRSolid = new G4SubtractionSolid("ESRSolid", ESRFull, SiPMCutout, nullptr, cutoutOffset);
  auto ESRLV = new G4LogicalVolume(ESRSolid, ESRMaterial, "ESRLV");

  // set up window solid and material
  // Approximate SiPM encapsulant/window
G4double windowThickness = 0.3 * mm;

//define window solid
auto windowSolid = new G4Box(
    "SiPMWindow",
    3.5 * mm,
    windowThickness / 2.0,
    3.5 * mm
);

//define window material
auto SiPMWindowMat = new G4Material(
    "SiPMWindowMaterial",
    1.2 * g/cm3,
    3
);

//window material optical properties
SiPMWindowMat->AddElement(
    nist->FindOrBuildElement("C"), 21
);
SiPMWindowMat->AddElement(
    nist->FindOrBuildElement("H"), 25
);
SiPMWindowMat->AddElement(
    nist->FindOrBuildElement("O"), 5
);

const G4int nWindow = 2;

G4double windowEnergy[nWindow] = {
    1.0 * eV,
    4.5 * eV
};

G4double windowRIndex[nWindow] = {
    1.59,
    1.59
};

G4double windowAbsLength[nWindow] = {
    10.0 * m,
    10.0 * m
};

auto windowMPT = new G4MaterialPropertiesTable();

windowMPT->AddProperty(
    "RINDEX",
    windowEnergy,
    windowRIndex,
    nWindow
);

windowMPT->AddProperty(
    "ABSLENGTH",
    windowEnergy,
    windowAbsLength,
    nWindow
);

SiPMWindowMat->SetMaterialPropertiesTable(windowMPT);

//create window LV
auto windowLV = new G4LogicalVolume(
    windowSolid,
    SiPMWindowMat,
    "SiPMWindowLV"
);

  G4cout << "defining SiPM solid and LV" << G4endl;
  auto SiPMSolid = new G4Box("SiPM", siPMSide/2, siPMThickness/2,siPMSide/2);
  auto SiPMLV = new G4LogicalVolume(SiPMSolid, nist->FindOrBuildMaterial("G4_Si"), "SiPMLV");
  G4cout << "SiPM solid + LV defined" << G4endl;

  G4cout << "defining SiPM optical surface" << G4endl;
  auto SiPMSurface = new G4OpticalSurface("SiPMSurface");
  SiPMSurface->SetType(dielectric_metal);
  SiPMSurface->SetFinish(polished);
  SiPMSurface->SetModel(unified);
  G4cout << "SiPM optical surface defined" << G4endl;

  G4cout << "defining SiPM properties" << G4endl;
  auto SiPM_MPT = new G4MaterialPropertiesTable();
  // G4double SiPM_EFFICIENCY = 0.5;
  G4double SiPMEnergy[] = {
    1.0 * eV, 
    4.5 * eV
  };
  G4double SiPMReflectivity[] = {
    0.1,
    0.1
  };
  G4double SiPMEfficiency[] = {
    0.5,
    0.5
  };
  SiPM_MPT->AddProperty("EFFICIENCY", SiPMEnergy, SiPMEfficiency, 2);
  SiPM_MPT->AddProperty("REFLECTIVITY", SiPMEnergy, SiPMReflectivity, 2);
  SiPMSurface->SetMaterialPropertiesTable(SiPM_MPT);
  G4cout << "SiPM properties defined" << G4endl;


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
  //claddingLV->SetVisAttributes(green);

  G4cout << "defining scint material and properties" << G4endl;
  G4Material* scintMat = nist->FindOrBuildMaterial("G4_CESIUM_IODIDE");

  G4MaterialPropertiesTable* CsI_MPT = new G4MaterialPropertiesTable();

  G4cout << "defining refractive index" << G4endl;
  //G4double rindex = 1.79;
  G4double refractivityEnergy[] = {
    1.0 * eV, 
    4.5 * eV
  };
  G4double rindex[] = {
    1.79,
    1.79
  };
  G4cout << "refractive index defined" << G4endl;
  G4cout << "defining light yield" << G4endl;
  G4double scintillationYield = 54000./MeV;
  G4double decayTime = 1000.*ns;

  G4cout << "process emission spectrum data" << G4endl;
  std::vector<G4double> photonEnergy;
  std::vector<G4double> emission; 
  std::vector<std::pair<G4double, G4double>> emissionData;

  std::ifstream file("emission_spectrum_data.csv");

  std::string line;
  while (std::getline(file, line)) {
    std::stringstream ss(line);

    double energy_eV;
    double intensity;
    char comma;
    if (ss >> energy_eV >> comma >> intensity)
    {
      emissionData.emplace_back(
        energy_eV * eV,
        intensity
      );
    }
  }

  std:: sort(
    emissionData.begin(),
    emissionData.end(),
    [](const auto& a, const auto& b)
    {
      return a.first < b.first;
    }
  );

  for (const auto& point: emissionData)
  {
    photonEnergy.push_back(point.first);
    emission.push_back(point.second);
  }
  G4cout << "emission spectrum data processed" << G4endl;

  G4cout << "procesing absorption length data" << G4endl;
  std::vector<G4double> absorptionEnergy;
  std::vector<G4double> absorptionLength;
  std::vector<std::pair<G4double, G4double>> absorptionData;

  std::ifstream absorptionFile("absorption_length_data.csv");

  std::string absorptionLine;
  while (std::getline(absorptionFile, absorptionLine)) {
    std::stringstream ss(absorptionLine);
    
    double energy_eV;
    double length_mm;
    char comma;
    if (ss >> energy_eV >> comma >> length_mm)
    {
      absorptionData.emplace_back(
        energy_eV * eV,
        length_mm * mm
      );
    }
  }

  std::sort(absorptionData.begin(), 
  absorptionData.end(),
  [](const auto& a, const auto& b)
  {
    return a.first < b.first;
  }
);
//std::vector<G4double> absorptionEnergy;
//std::vector<G4double> absorptionLength;

for (const auto& point : absorptionData)
{
  absorptionEnergy.push_back(point.first);
  absorptionLength.push_back(point.second);
}

  G4cout << "absorption length data processed" << G4endl;

  // CsI_MPT->AddProperty("RINDEX", energies, rindex, nEntries);
  // CsI_MPT->AddProperty("ABSLENGTH", energies, absorption, nEntries);
  // CsI_MPT->AddProperty("SCINTILLATIONCOMPONENT1", photonEnergy, scintSpectrum, nEntries);
  G4cout << "adding CsI optical properties" << G4endl;
  G4cout << "Adding RINDEX..." << G4endl;
  CsI_MPT->AddProperty("RINDEX", refractivityEnergy, rindex, 2);
  G4cout << "Adding SCINTILLATIONYIELD..." << G4endl;
  CsI_MPT->AddConstProperty("SCINTILLATIONYIELD", scintillationYield);
  G4cout << "Adding TIMECONSTANT..." << G4endl;
  CsI_MPT->AddConstProperty("SCINTILLATIONTIMECONSTANT1", decayTime);
  G4cout << "Adding ABSLENGTH..." << G4endl;
  for (size_t i = 0; i < absorptionEnergy.size(); ++i)
  {
    G4cout << i
           << "  E = " << absorptionEnergy[i] / eV << " eV"
           << "  L = " << absorptionLength[i] / mm << " mm"
           << G4endl;
  }
  G4cout << "Energy size: " << absorptionEnergy.size() << G4endl;
  G4cout << "Length size: " << absorptionLength.size() << G4endl;
  CsI_MPT->AddProperty("ABSLENGTH", absorptionEnergy.data(), absorptionLength.data(), absorptionEnergy.size());
  G4cout << "Adding SCINTILLATIONCOMPONENT1..." << G4endl;
  CsI_MPT->AddProperty("SCINTILLATIONCOMPONENT1", photonEnergy.data(), emission.data(), photonEnergy.size());
  G4cout << "Adding SCINTILLATIONYIELD1..." << G4endl;
  CsI_MPT->AddConstProperty("SCINTILLATIONYIELD1", 1.0);
  G4cout << "Adding RESOLUTIONSCALE..." << G4endl;
  CsI_MPT->AddConstProperty("RESOLUTIONSCALE", 1.0);

  scintMat->SetMaterialPropertiesTable(CsI_MPT);
  G4cout << "Scint material and properties defined" << G4endl;

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

    G4Box * opticalGreaseSolid = new G4Box(
      "opticalGrease",
      greaseWidth/2, // half width in x
      greaseHeight/2, // half width in y
      greaseLength/2     // half length in z
    ); 

    G4LogicalVolume * opticalGreaseLV = new G4LogicalVolume(
      opticalGreaseSolid, // its solid
      opticalGrease,   // its material
      "opticalGrease"     // name
    ) ;

    scintLV->SetVisAttributes(blue);
    
  //auto activeSolid = new G4Box("Active", activeSide/2, activeSide/2, activeSide/2);
  //auto activeLV = new G4LogicalVolume(activeSolid, scintMat, "ActiveLV");

  G4cout << "placing scint PV" << G4endl;
  auto scintillatorPV = new G4PVPlacement(0,                       //no rotation
                    scintPos,                    //at position
                    scintLV,             //its logical volume
                    "scint",                //its name
                    logicWorld,                //its mother  volume
                    false,                   //no boolean operation
                    0,                       //copy number
                    checkOverlaps);          //overlaps checking
  G4cout << "scint PV placed" << G4endl;
   
  G4double offset = crystalSize/2 + reflectorThickness/2;
  G4double greaseOffset = crystalSize/2 + greaseHeight/2;
  G4double SiPMoffset = crystalSize/2 + greaseHeight + windowThickness + siPMThickness/2;
  G4double ESROffset = crystalSize/2 + esrThickness/2;
  G4double windowOffset = crystalSize/2 + greaseHeight + windowThickness/2;
  G4cout << "placing reflector and SiPM PVs" << G4endl;
  auto sideReflectorPV1 = new G4PVPlacement(nullptr, scintPos + G4ThreeVector(-offset, 0, 0), SideReflectorLV1, "SideReflector1", logicWorld, false, 0, checkOverlaps);
  auto sideReflectorPV2 = new G4PVPlacement(nullptr, scintPos + G4ThreeVector(offset, 0, 0), SideReflectorLV2, "SideReflector2", logicWorld, false, 0, checkOverlaps);
  auto sideReflectorPV3 = new G4PVPlacement(nullptr, scintPos + G4ThreeVector(0, -offset, 0), SideReflectorLV3, "SideReflector3", logicWorld, false, 0, checkOverlaps);
  auto topReflectorPV = new G4PVPlacement(nullptr, scintPos + G4ThreeVector(0, 0, offset), TopReflectorLV, "TopReflector", logicWorld, false, 0, checkOverlaps);
  auto bottomReflectorPV = new G4PVPlacement(nullptr, scintPos + G4ThreeVector(0, 0, -offset), BottomReflectorLV, "BottomReflector", logicWorld, false, 0, checkOverlaps);
  auto siPMPV = new G4PVPlacement(nullptr, scintPos + G4ThreeVector(0, SiPMoffset, 0.5 * mm), SiPMLV, "SiPM", logicWorld, false, 0, checkOverlaps);
  auto opticalGreasePV = new G4PVPlacement(nullptr, scintPos + G4ThreeVector(0, greaseOffset, 0.5 * mm), opticalGreaseLV, "OpticalGrease", logicWorld, false, 0, checkOverlaps);
  auto ESRPV = new G4PVPlacement(nullptr, scintPos + G4ThreeVector(0, ESROffset, 0), ESRLV, "ESRFilm", logicWorld, false, 0, checkOverlaps);
  auto SiPMWindowPV = new G4PVPlacement(
    nullptr,
    G4ThreeVector(
        0,
        windowOffset,
        0.5 * mm
    ),
    SiPMWindowLV,
    "SiPMWindowPV",
    logicWorld,
    false,
    0,
    checkOverlaps
);
  G4cout << "reflector and SiPM PVs placed" << G4endl;

  //auto claddingPV = new G4PVPlacement(nullptr, scintPos, claddingLV, "Cladding", logicWorld, false, 0, checkOverlaps);

  G4cout << "creating reflector and SiPM logical border surfaces" << G4endl;
  new G4LogicalBorderSurface("SideReflectorSurface1", scintillatorPV, sideReflectorPV1, reflectorSurface);
  new G4LogicalBorderSurface("SideReflectorSurface2", scintillatorPV, sideReflectorPV2, reflectorSurface);
  new G4LogicalBorderSurface("SideReflectorSurface3", scintillatorPV, sideReflectorPV3, reflectorSurface);
  new G4LogicalBorderSurface("TopReflectorSurface", scintillatorPV, topReflectorPV, reflectorSurface);
  new G4LogicalBorderSurface("BottomReflectorSurface", scintillatorPV, bottomReflectorPV, reflectorSurface);
  new G4LogicalBorderSurface("SiPMSurface", SiPMWindowPV, siPMPV, SiPMSurface);  
  new G4LogicalBorderSurface("ESRSurface", scintillatorPV, ESRPV, ESRSurface);
  G4cout << "reflector and SiPM logical border surfaces placed" << G4endl;



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
