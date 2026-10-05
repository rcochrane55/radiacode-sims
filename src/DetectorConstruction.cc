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
#include "G4RotationMatrix.hh"


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

// define KCl, import 3d model of marinelli internal volume

G4Material* KCl = new G4Material("KCl", 1.23*g/cm3, 2);
G4int nAtoms;
G4Element* elK = new G4Element("Potassium", "P", 19, 39.0983*g/mole);
G4Element* elCl = new G4Element("Chloride", "Cl", 17, 35.453*g/mole);
KCl->AddElement(elK, nAtoms=1);
KCl->AddElement(elCl, nAtoms=1);

G4Element *elNb = new G4Element("Niobium", "Nb", 41, 92.906*g/mole);
G4Element *elO = new G4Element("Oxygen", "O", 8, 15.999*g/mole);
G4Material *Nb2O5 = new G4Material("Niobium Pentoxide, Nb2O5", 4.6*g/cm3, 2);
Nb2O5->AddElement(elNb, 2);
Nb2O5->AddElement(elO, 5);

G4Element *elTh = new G4Element("Thorium", "Th", 90, 232.038*g/mole);
G4Material *ThO2 = new G4Material("Thorium Dioxide, ThO2", 10.0*g/cm3, 2);
ThO2->AddElement(elTh, 1);
ThO2->AddElement(elO, 2);

G4Element* elTa = new G4Element("Tantalum", "Ta", 73, 180.94788*g/mole);
G4Material* Ta2O5 = new G4Material("Tantalum Pentoxide, Ta2O5", 8.2*g/cm3, 2);
Ta2O5->AddElement(elTa, 2);
Ta2O5->AddElement(elO, 5);

G4Element* elSi = new G4Element("Silicon", "Si", 14, 28.0855*g/mole);
G4Material* SiO2 = new G4Material("Silicon Dioxide, SiO2", 2.65*g/cm3, 2);
SiO2->AddElement(elSi, 1);
SiO2->AddElement(elO, 2);

G4Element* elCa = new G4Element("Calcium", "Ca", 20, 40.078*g/mole);
G4Material* CaO = new G4Material("Calcium Oxide, CaO", 3.34*g/cm3, 2);
CaO->AddElement(elCa, 1);
CaO->AddElement(elO, 1);

G4Element* elZr = new G4Element("Zirconium", "Zr", 40, 91.224*g/mole);
G4Material* ZrO2 = new G4Material("Zirconium Dioxide, ZrO2", 5.68*g/cm3, 2);
ZrO2->AddElement(elZr, 1);
ZrO2->AddElement(elO, 2);

G4Element* elMn = new G4Element("Manganese", "Mn", 25, 54.938*g/mole);
G4Material* MnO = new G4Material("Manganese Oxide, MnO", 5.03*g/cm3, 2);
MnO->AddElement(elMn, 1);
MnO->AddElement(elO, 1);

G4Element* elPb = new G4Element("Lead", "Pb", 82, 207.2*g/mole);
G4Material* PbO = new G4Material("Lead Oxide, PbO", 9.53*g/cm3, 2);
PbO->AddElement(elPb, 1);
PbO->AddElement(elO, 1);

G4Element* elMg = new G4Element("Magnesium", "Mg", 12, 24.305*g/mole);
G4Material* MgO = new G4Material("Magnesium Oxide, MgO", 3.55*g/cm3, 2);
MgO->AddElement(elMg, 1);
MgO->AddElement(elO, 1);

G4Element* elSn = new G4Element("Tin", "Sn", 50, 118.71*g/mole);
G4Material* SnO2 = new G4Material("Tin Oxide, SnO2", 6.95*g/cm3, 2);
SnO2->AddElement(elSn, 1);
SnO2->AddElement(elO, 2);

G4Element* elAl = new G4Element("Aluminum", "Al", 13, 26.9815*g/mole);
G4Material* Al2O3 = new G4Material("Aluminum Oxide, Al2O3", 3.95*g/cm3, 2);
Al2O3->AddElement(elAl, 2);
Al2O3->AddElement(elO, 3);

G4Element* elCe = new G4Element("Cerium", "Ce", 58, 140.116*g/mole);
G4Material* Ce2O3 = new G4Material("Cerium Oxide, Ce2O3", 7.65*g/cm3, 2);
Ce2O3->AddElement(elCe, 2);
Ce2O3->AddElement(elO, 3);

G4Element* elY = new G4Element("Yttrium", "Y", 39, 88.90584*g/mole);
G4Material* Y2O3 = new G4Material("Yttrium Oxide, Y2O3", 5.01*g/cm3, 2);
Y2O3->AddElement(elY, 2);
Y2O3->AddElement(elO, 3);

G4Element* elFe = new G4Element("Iron", "Fe", 26, 55.845*g/mole);
G4Material* Fe2O3 = new G4Material("Iron(III) Oxide, Fe2O3", 5.24*g/cm3, 2);
Fe2O3->AddElement(elFe, 2);
Fe2O3->AddElement(elO, 3);
G4Material* FeO = new G4Material("Iron(II) Oxide, FeO", 5.745*g/cm3, 2);
FeO->AddElement(elFe, 1);
FeO->AddElement(elO, 1);

G4Element* Ti = nist->FindOrBuildElement("Ti");
G4Material* TiO2 = new G4Material("TiO2", 4.23*g/cm3, 2);
TiO2->AddElement(Ti, 1);
TiO2->AddElement(elO, 2);

G4Material* euxenite = new G4Material("Euxenite", 5.16*g/cm3, 16);
euxenite->AddMaterial(Nb2O5, 0.4386);
euxenite->AddMaterial(ThO2, 0.0495);
euxenite->AddMaterial(Ta2O5, 0.0384);
euxenite->AddMaterial(SiO2, 0.0007);
euxenite->AddMaterial(TiO2, 0.1639);
euxenite->AddMaterial(ZrO2, 0.0004);
euxenite->AddMaterial(SnO2, 0.0012);
euxenite->AddMaterial(Al2O3, 0.0013);
euxenite->AddMaterial(Ce2O3, 0.0434);
euxenite->AddMaterial(Y2O3, 0.1822);
euxenite->AddMaterial(Fe2O3, 0.0132);
euxenite->AddMaterial(FeO, 0.0077);
euxenite->AddMaterial(MnO, 0.0059);
euxenite->AddMaterial(PbO, 0.0037);
euxenite->AddMaterial(MgO, 0.0013);
euxenite->AddMaterial(CaO, 0.0486);

G4double mineralThickness = 5.92 * mm;
G4double mineralDiameter = 31.50 * mm;
auto mineralRotation = new G4RotationMatrix();
mineralRotation->rotateX(90.0 * deg);

auto mineralSolid = new G4Tubs(
    "EuxeniteDisk",
    0.0,
    mineralDiameter / 2,
    mineralThickness / 2,
    0.0 * deg,
    360.0 * deg
);
auto mineralLV = new G4LogicalVolume(mineralSolid, euxenite, "mineralLV");

G4ThreeVector mineralPos = G4ThreeVector(0*cm, -9.5*cm, 0*cm);

new G4PVPlacement(mineralRotation, mineralPos, mineralLV, "mineralSource", logicWorld, false, 0, checkOverlaps);

// auto mesh = CADMesh::TessellatedMesh::FromSTL("D:/Geant4/radiacode-sims/marinelli_volume.stl");
// auto solid = mesh->GetSolid();
// auto marinelliLV = new G4LogicalVolume(solid, KCl, "marinelliLV");

// G4ThreeVector marinelliPos = G4ThreeVector(0*cm, 0*cm, 0*cm);

// new G4PVPlacement(0, marinelliPos, marinelliLV, "marinelli", logicWorld, false, 0, checkOverlaps); 

// CsI scint assembly

  G4double scintLength = 1.*cm ;
  G4double scintWidth = 1.*cm ;
  G4double scintHeight = 1.*cm ;

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
  claddingLV->SetVisAttributes(red);
  SiPMLV->SetVisAttributes(green);

  G4Material* scintMat = nist->FindOrBuildMaterial("G4_CESIUM_IODIDE");
  G4ThreeVector scintPos = G4ThreeVector(0*cm, 0*cm, 0*cm);

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

  fScoringVolume = scintLV;

  //
  //always return the physical World
  //
  return physWorld;
}
