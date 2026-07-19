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
                     
// CsI scint

  G4double scintLength = 1.*cm ;
  G4double scintWidth = 1.*cm ;
  G4double scintHeight = 1.*cm ;

  G4Element* Ti = nist->FindOrBuildElement("Ti");
  G4Element* O = nist->FindOrBuildElement("O");
  G4Material* TiO2 = new G4Material("TiO2", 4.23*g/cm3, 2);

  TiO2->AddElement(Ti, 1);
  TiO2->AddElement(O, 2);

  G4double crystalSize = 1.0*cm;
  G4double deadLayer = 0.03*cm;
  G4double activeSide = crystalSize - 2*deadLayer;
  G4double reflectorThickness = 0.04*cm;
  G4double claddingThickness = 0.16*cm;

  G4double reflectorOuter = 1*cm + 2.0*reflectorThickness;
  G4double claddingOuter = (reflectorOuter + 2.0*claddingThickness);

  auto claddingOuterSolid = new G4Box("CladdingOuter", claddingOuter/2, claddingOuter/2, claddingOuter/2);
  auto claddingInnerSolid = new G4Box("CladdingInner", reflectorOuter/2, reflectorOuter/2, reflectorOuter/2);
  auto claddingSolid = new G4SubtractionSolid("Cladding", claddingOuterSolid, claddingInnerSolid);

  G4Material* claddingMat = nist->FindOrBuildMaterial("G4_POLYETHYLENE");

  auto claddingLV = new G4LogicalVolume(claddingSolid, claddingMat, "CladdingLV");

  new G4PVPlacement(nullptr, G4ThreeVector(0,0,-0.12*cm), claddingLV, "Cladding", logicWorld, false, 0, checkOverlaps);

  auto sideReflectorSolid1 = new G4Box("SideReflector1", reflectorThickness/2, crystalSize/2, crystalSize/2);
  auto sideReflectorSolid2 = new G4Box("SideReflector2", reflectorThickness/2, crystalSize/2, crystalSize/2);
  auto sideReflectorSolid3 = new G4Box("SideReflector3", crystalSize/2, crystalSize/2, reflectorThickness/2);
  auto topReflectorSolid = new G4Box("TopReflector", crystalSize/2, reflectorThickness/2,crystalSize/2);
  auto bottomReflectorSolid = new G4Box("BottomReflector", crystalSize/2, reflectorThickness/2, crystalSize/2);

  auto SideReflectorLV1 = new G4LogicalVolume(sideReflectorSolid1, TiO2, "SideReflectorLogical1");
  auto SideReflectorLV2 = new G4LogicalVolume(sideReflectorSolid2, TiO2, "SideReflectorLogical2");
  auto SideReflectorLV3 = new G4LogicalVolume(sideReflectorSolid3, TiO2, "SideReflectorLogical3");
  auto TopReflectorLV = new G4LogicalVolume(topReflectorSolid, TiO2, "TopReflectorLogical");
  auto BottomReflectorLV = new G4LogicalVolume(bottomReflectorSolid, TiO2, "BottomReflectorLogical");

  G4double offset = crystalSize/2 + reflectorThickness/2;

  new G4PVPlacement(nullptr, G4ThreeVector(0,0,-0.12*cm) + G4ThreeVector(-offset, 0, 0), SideReflectorLV1, "SideReflector1", logicWorld, false, 0, checkOverlaps);
  new G4PVPlacement(nullptr, G4ThreeVector(0,0,-0.12*cm) + G4ThreeVector(offset, 0, 0), SideReflectorLV2, "SideReflector2", logicWorld, false, 0, checkOverlaps);
  new G4PVPlacement(nullptr, G4ThreeVector(0,0,-0.12*cm) + G4ThreeVector(0, 0, -offset), SideReflectorLV3, "SideReflector3", logicWorld, false, 0, checkOverlaps);
  new G4PVPlacement(nullptr, G4ThreeVector(0,0,-0.12*cm) + G4ThreeVector(0, offset, 0), TopReflectorLV, "TopReflector", logicWorld, false, 0, checkOverlaps);
  new G4PVPlacement(nullptr, G4ThreeVector(0,0,-0.12*cm) + G4ThreeVector(0, -offset, 0), BottomReflectorLV, "BottomReflector", logicWorld, false, 0, checkOverlaps);


  auto  white = new G4VisAttributes(G4Colour(1.0, 1.0, 1.0)); // white
  SideReflectorLV1->SetVisAttributes(white);
  SideReflectorLV2->SetVisAttributes(white);
  SideReflectorLV3->SetVisAttributes(white);
  TopReflectorLV->SetVisAttributes(white);
  BottomReflectorLV->SetVisAttributes(white);

  G4Material* scintMat = nist->FindOrBuildMaterial("G4_CESIUM_IODIDE");
  G4ThreeVector scintPos = G4ThreeVector(0, 0, 0);

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

  auto activeSolid = new G4Box("Active", activeSide/2, activeSide/2, activeSide/2);
  auto activeLV = new G4LogicalVolume(activeSolid, scintMat, "ActiveLV");

  new G4PVPlacement(0,                       //no rotation
                    G4ThreeVector(0,0,-0.12*cm),                    //at position
                    scintLV,             //its logical volume
                    "scint",                //its name
                    logicWorld,                //its mother  volume
                    false,                   //no boolean operation
                    0,                       //copy number
                    checkOverlaps);          //overlaps checking

  new G4PVPlacement(0, G4ThreeVector(), activeLV, "Active", scintLV, false, 0, checkOverlaps);

// Marinelli

G4Cons * marinelliSolidFull = new G4Cons(
"marinelliFull", // name
 0., // inner radius -pDz
 (6.7/2)*cm, // outer radius -pDz
 0., // inner radius +pDz
 (6.7/2)*cm, // outer radius +pDz
 (6.8/2)*cm, // Z half length
 0, // starting Phi
 360*degree); // segment angle

G4Box* marinelliSolidInner = new G4Box(
  "marinelliInner",
  ((3.5/2) + 0.12)*cm, // half width in x
  (1 + 0.12)*cm, // half width in y
  (2.2 + 0.06)*cm   // half length in z
) ;


G4RotationMatrix* rot = new G4RotationMatrix();
G4ThreeVector zTrasl(0, 0, 1.29*cm);

G4SubtractionSolid* marinelliSolid = new G4SubtractionSolid("marinelli", marinelliSolidFull, marinelliSolidInner, rot, zTrasl);

// G4Material* marinelliMat = nist->FindOrBuildMaterial("G4_AIR");

// Elements for building compounds

// G4Element("nome", "nome", z, a)
G4Element* elH = new G4Element("Hydrogen", "H",   1,  1.01*g/mole);
G4Element* elO = new G4Element("Oxygen", "O",     8,  16*g/mole);
G4Element* elCa = new G4Element("Calcium", "Ca",  20, 40.078*g/mole);
G4Element* elP = new G4Element("Phosphorus", "P", 15, 30.973762*g/mole);
G4Element* elSi = new G4Element("Silicon", "Si", 14, 28.0855*g/mole);
G4Element* elFe = new G4Element("Iron", "Fe", 26, 55.845*g/mole);
G4Element* elAl = new G4Element("Aluminum", "Al", 13, 26.981539*g/mole);
G4Element* elC = new G4Element("Carbon", "C", 6, 12.0107*g/mole);
G4Element* elF = new G4Element("Fluorine", "F", 9, 18.998403*g/mole);
G4Element* elK = new G4Element("Potassium", "P", 19, 39.0983*g/mole);
G4Element* elCl = new G4Element("Chloride", "Cl", 17, 35.453*g/mole);
G4Element* elMg = new G4Element("Magnesium", "Mg", 12, 24.305*g/mole);

// superfosfato Ca(H2PO4)2 per marinelli

G4double density = 1.3*g/cm3; // https://en.wikipedia.org/wiki/Physical_properties_of_soil#Density
G4int ncomp = 4;
G4Material* superfosfato = new G4Material("superfosfato", density, ncomp);
G4int nAtoms;
superfosfato->AddElement(elH, nAtoms=4);
superfosfato->AddElement(elO, nAtoms=8);
superfosfato->AddElement(elCa, nAtoms=1);
superfosfato->AddElement(elP, nAtoms=2);

// soil 
G4Material * soil  = new G4Material("soil", 1.3*g/cm3, 5);
soil->AddElement(elSi, nAtoms=1);
soil->AddElement(elO, nAtoms=10);
soil->AddElement(elAl, nAtoms=1);
soil->AddElement(elFe, nAtoms=1);
soil->AddElement(elC, nAtoms=1);

// phoshorite
G4Material * phosphorite  = new G4Material("phosphorite", 1.6*g/cm3, 5);
phosphorite->AddElement(elCa, nAtoms=10);
phosphorite->AddElement(elP, nAtoms=8);
phosphorite->AddElement(elO, nAtoms=30);
phosphorite->AddElement(elF, nAtoms=1);
phosphorite->AddElement(elH, nAtoms=1);

// fertilizer
G4Material * fertilizer  = new G4Material("fertilizer", 1.065*g/cm3, 2);
fertilizer->AddElement(elK, nAtoms=1);
fertilizer->AddElement(elCl, nAtoms=1);

// tuff
G4Material * tuff  = new G4Material("tuff", 1.4*g/cm3, 6);
tuff->AddElement(elK, nAtoms=1);
tuff->AddElement(elAl, nAtoms=1);
tuff->AddElement(elSi, nAtoms=5);
tuff->AddElement(elO, nAtoms=14);
tuff->AddElement(elCa, nAtoms=1);
tuff->AddElement(elMg, nAtoms=1);

//catlitter
G4Material * catlitter  = new G4Material("catlitter", 1.05*g/cm3, 4);
catlitter->AddElement(elAl, nAtoms=2);
catlitter->AddElement(elO, nAtoms=15);
catlitter->AddElement(elSi, nAtoms=4);
catlitter->AddElement(elH, nAtoms=8);

//KCl
G4Material* KCl = new G4Material("KCl", 1.23*g/cm3, 2);
KCl->AddElement(elK, nAtoms=1);
KCl->AddElement(elCl, nAtoms=1);


G4LogicalVolume * marinelliLV = new G4LogicalVolume(
  marinelliSolid, // its solid
  KCl,   // its material
  "marinelli"     // name
) ;

G4ThreeVector marinelliPos = G4ThreeVector(0*cm, 0*cm, -0.2*cm);
new G4PVPlacement(0,                       //no rotation
  marinelliPos,                    //at position
  marinelliLV,             //its logical volume
  "marinelli",                //its name
  logicWorld,                //its mother  volume
  false,                   //no boolean operation
  0,                       //copy number
  checkOverlaps);          //overlaps checking

//G4cout << "Mass = "
//       << marinelliLV->GetMass(true,true)/g
//       << " g" << G4endl;

  // Set scint as scoring volume
  //
  fScoringVolume = activeLV;

  //
  //always return the physical World
  //
  return physWorld;
}
