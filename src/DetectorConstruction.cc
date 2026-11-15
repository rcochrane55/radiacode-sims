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
#include "DetectorConstructionMessenger.hh"
#include "G4Material.hh"
#include "G4RotationMatrix.hh"
#include "G4Exception.hh"


DetectorConstruction::DetectorConstruction()
: G4VUserDetectorConstruction(),
  fScoringVolume(nullptr),
  fMessenger(nullptr)
{
  //default detector config is RC103
  fDetectorConfig.scintWidth = 1.0*cm;
  fDetectorConfig.scintHeight = 1.0*cm;
  fDetectorConfig.scintLength = 1.0*cm;

  // default scint position is 3cm from origin on z axis, identical to existing RC103 marinelli geometry
  fDetectorConfig.scintPosition = G4ThreeVector(0, 0, 3*cm);

  fDetectorConfig.SiPMSide = 0.6*cm;
  fDetectorConfig.SiPMThickness = 0.04*cm;

  fDetectorConfig.reflectorThickness = 0.04*cm;
  fDetectorConfig.claddingThickness = 0.16*cm;

  //default source config is 200ml radiacode marinelli beaker imported using CADMesh
  fSourceConfig.geometry = SourceGeometry::Marinelli;

  fSourceConfig.marinelliMeshFile = "D:/Geant4/radiacode-sims/marinelli_volume.stl";

  fSourceConfig.marinelliPosition = G4ThreeVector(0, 0, 0);

  //default disk source dimensions brought over from RC103 point source geometry
  fSourceConfig.diskDiameter = 31.50*mm;
  fSourceConfig.diskThickness = 5.92*mm;

  fSourceConfig.diskPosition = G4ThreeVector(0, -5.0*cm, 0*cm);
  fSourceConfig.diskRotation = G4ThreeVector(90*deg, 0*deg, 0*deg);

  //create messenger after initializing default values
  fMessenger = new DetectorConstructionMessenger(this);
}

DetectorConstruction::~DetectorConstruction()
{
  delete fMessenger;
}

void DetectorConstruction::SetCrystalDimensions(const G4ThreeVector& dimensions)
{
  if (dimensions.x() <= 0 || 
      dimensions.y() <= 0 ||
      dimensions.z() <= 0)
  {
    G4Exception("DetectorConstruction::SetCrystalDimensions",
                "InvalidCrystalDimensions",
                FatalException,
                "Crystal dimensions must be positive values."
              );
              return;
  }
  fDetectorConfig.scintWidth = dimensions.x();
  fDetectorConfig.scintHeight = dimensions.y();
  fDetectorConfig.scintLength = dimensions.z();
}

void DetectorConstruction::SetCrystalPosition(const G4ThreeVector& position)
{
  fDetectorConfig.scintPosition = position;
}

void DetectorConstruction::SetSiPMSide(G4double side)
{
  if (side <= 0)
  {
    G4Exception("DetectorConstruction::SetSiPMSide",
                "InvalidSiPMSide",
                FatalException,
                "SiPM side length must be a positive value."
              );
              return;
  }
  fDetectorConfig.SiPMSide = side;
}

void DetectorConstruction::SetSiPMThickness(G4double thickness)
{
  if (thickness <= 0)
  {
    G4Exception("DetectorConstruction::SetSiPMThickness",
                "InvalidSiPMThickness",
                FatalException,
                "SiPM thickness must be a positive value."
              );
              return;
  }
  fDetectorConfig.SiPMThickness = thickness;
}

void DetectorConstruction::SetCladdingThickness(G4double thickness)
{
  if (thickness <= 0)
  {
    G4Exception("DetectorConstruction::SetCladdingThickness",
                "InvalidCladdingThickness",
                FatalException,
                "Cladding thickness must be a positive value."
              );
              return;
  }
  fDetectorConfig.claddingThickness = thickness;
}

void DetectorConstruction::SetReflectorThickness(G4double thickness)
{
  if (thickness <= 0)
  {
    G4Exception("DetectorConstruction::SetReflectorThickness",
                "InvalidReflectorThickness",
                FatalException,
                "Reflector thickness must be a positive value."
              );
              return;
  }
  fDetectorConfig.reflectorThickness = thickness;
}

void DetectorConstruction::SetSourceType(SourceGeometry geometry)
{
    fSourceConfig.geometry = geometry;
}

void DetectorConstruction::SetMarinelliMeshFile(const G4String& filename)
{
  fSourceConfig.marinelliMeshFile = filename;
}

void DetectorConstruction::SetMarinelliPosition(const G4ThreeVector& position)
{
  fSourceConfig.marinelliPosition = position;
}

void DetectorConstruction::SetDiskDiameter(G4double diameter)
{
  if (diameter <= 0)
  {
    G4Exception("DetectorConstruction::SetDiskDiameter",
                "InvalidDiskDiameter",
                FatalException,
                "Disk diameter must be a positive value."
              );
              return;
  }
  fSourceConfig.diskDiameter = diameter;
}

void DetectorConstruction::SetDiskThickness(G4double thickness)
{
  if (thickness <= 0)
  {
    G4Exception("DetectorConstruction::SetDiskThickness",
                "InvalidDiskThickness",
                FatalException,
                "Disk thickness must be a positive value."
              );
              return;
  }
  fSourceConfig.diskThickness = thickness;
}

void DetectorConstruction::SetDiskPosition(const G4ThreeVector& position)
{
  fSourceConfig.diskPosition = position;
}

void DetectorConstruction::SetDiskRotation(const G4ThreeVector& rotation)
{
  fSourceConfig.diskRotation = rotation;
}

void DetectorConstruction::DefineMaterials()
{
  G4NistManager* nist = G4NistManager::Instance();

  //define KCl material for marinelli source
  if (!G4Material::GetMaterial("KCl", false))
    {
        G4Element* K = nist->FindOrBuildElement("K");
        G4Element* Cl = nist->FindOrBuildElement("Cl");

        G4Material* KCl =
            new G4Material("KCl", 1.23*g/cm3, 2);

        KCl->AddElement(K, 1);
        KCl->AddElement(Cl, 1);
    }

  //define TiO2 material used by reflector and by euxenite source
  if (!G4Material::GetMaterial("TiO2", false))
    {
        G4Element* Ti = nist->FindOrBuildElement("Ti");
        G4Element* O = nist->FindOrBuildElement("O");

        G4Material* TiO2 =
            new G4Material("TiO2", 4.23*g/cm3, 2);

        TiO2->AddElement(Ti, 1);
        TiO2->AddElement(O, 2);
    }
  
  //define euxenite material
  if (!G4Material::GetMaterial("Euxenite", false))
    {
        G4Element* O = nist->FindOrBuildElement("O");

        // Helper for constructing the individual oxides.
        auto makeOxide =
            [O](const G4String& name,
                G4double density,
                G4Element* metal,
                G4int metalAtoms,
                G4int oxygenAtoms)
        {
            G4Material* oxide =
                new G4Material(name, density, 2);

            oxide->AddElement(metal, metalAtoms);
            oxide->AddElement(O, oxygenAtoms);

            return oxide;
        };

        G4Material* Nb2O5 = makeOxide(
            "Niobium Pentoxide, Nb2O5",
            4.6*g/cm3,
            nist->FindOrBuildElement("Nb"), 2, 5
        );

        G4Material* ThO2 = makeOxide(
            "Thorium Dioxide, ThO2",
            10.0*g/cm3,
            nist->FindOrBuildElement("Th"), 1, 2
        );

        G4Material* Ta2O5 = makeOxide(
            "Tantalum Pentoxide, Ta2O5",
            8.2*g/cm3,
            nist->FindOrBuildElement("Ta"), 2, 5
        );

        G4Material* SiO2 = makeOxide(
            "Silicon Dioxide, SiO2",
            2.65*g/cm3,
            nist->FindOrBuildElement("Si"), 1, 2
        );

        G4Material* ZrO2 = makeOxide(
            "Zirconium Dioxide, ZrO2",
            5.68*g/cm3,
            nist->FindOrBuildElement("Zr"), 1, 2
        );

        G4Material* SnO2 = makeOxide(
            "Tin Oxide, SnO2",
            6.95*g/cm3,
            nist->FindOrBuildElement("Sn"), 1, 2
        );

        G4Material* Al2O3 = makeOxide(
            "Aluminum Oxide, Al2O3",
            3.95*g/cm3,
            nist->FindOrBuildElement("Al"), 2, 3
        );

        G4Material* Ce2O3 = makeOxide(
            "Cerium Oxide, Ce2O3",
            7.65*g/cm3,
            nist->FindOrBuildElement("Ce"), 2, 3
        );

        G4Material* Y2O3 = makeOxide(
            "Yttrium Oxide, Y2O3",
            5.01*g/cm3,
            nist->FindOrBuildElement("Y"), 2, 3
        );

        G4Material* Fe2O3 = makeOxide(
            "Iron(III) Oxide, Fe2O3",
            5.24*g/cm3,
            nist->FindOrBuildElement("Fe"), 2, 3
        );

        G4Material* FeO = makeOxide(
            "Iron(II) Oxide, FeO",
            5.745*g/cm3,
            nist->FindOrBuildElement("Fe"), 1, 1
        );

        G4Material* MnO = makeOxide(
            "Manganese Oxide, MnO",
            5.03*g/cm3,
            nist->FindOrBuildElement("Mn"), 1, 1
        );

        G4Material* PbO = makeOxide(
            "Lead Oxide, PbO",
            9.53*g/cm3,
            nist->FindOrBuildElement("Pb"), 1, 1
        );

        G4Material* MgO = makeOxide(
            "Magnesium Oxide, MgO",
            3.55*g/cm3,
            nist->FindOrBuildElement("Mg"), 1, 1
        );

        G4Material* CaO = makeOxide(
            "Calcium Oxide, CaO",
            3.34*g/cm3,
            nist->FindOrBuildElement("Ca"), 1, 1
        );

        // Reuse the TiO2 material defined above.
        G4Material* TiO2 = G4Material::GetMaterial("TiO2");

        G4Material* euxenite =
            new G4Material("Euxenite", 5.16*g/cm3, 16);

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
    }
}

void DetectorConstruction::ConstructSource(
    G4LogicalVolume* logicWorld,
    G4bool checkOverlaps)
{
    if (fSourceConfig.geometry == SourceGeometry::Marinelli)
    {
        ConstructMarinelliSource(logicWorld, checkOverlaps);
    }
    else if (fSourceConfig.geometry == SourceGeometry::Disk)
    {
        ConstructDiskSource(logicWorld, checkOverlaps);
    }
}

void DetectorConstruction::ConstructMarinelliSource(
    G4LogicalVolume* logicWorld,
    G4bool checkOverlaps)
{
    G4Material* KCl = G4Material::GetMaterial("KCl");

    G4cout << "Loading Marinelli STL: "
          << fSourceConfig.marinelliMeshFile
          << G4endl;

    auto mesh = CADMesh::TessellatedMesh::FromSTL(
        fSourceConfig.marinelliMeshFile
    );

    auto solid = mesh->GetSolid();

    G4cout << "Marinelli solid created: "
       << (solid != nullptr) << G4endl;

    auto marinelliLV = new G4LogicalVolume(
        solid,
        KCl,
        "marinelliLV"
    );

    new G4PVPlacement(
        nullptr,
        fSourceConfig.marinelliPosition,
        marinelliLV,
        "marinelli",
        logicWorld,
        false,
        0,
        true
    );

    G4cout << "Marinelli physical volume placed." << G4endl;
}

void DetectorConstruction::ConstructDiskSource(
    G4LogicalVolume* logicWorld,
    G4bool checkOverlaps)
{
    G4Material* euxenite =
        G4Material::GetMaterial("Euxenite");


    // Construct the disk.
    auto mineralSolid = new G4Tubs(
        "EuxeniteDisk",
        0.0,
        fSourceConfig.diskDiameter/2,
        fSourceConfig.diskThickness/2,
        0.0*deg,
        360.0*deg
    );


    auto mineralLV = new G4LogicalVolume(
        mineralSolid,
        euxenite,
        "mineralLV"
    );


    // Construct the disk rotation.
    auto mineralRotation = new G4RotationMatrix();

    mineralRotation->rotateX(fSourceConfig.diskRotation.x());
    mineralRotation->rotateY(fSourceConfig.diskRotation.y());
    mineralRotation->rotateZ(fSourceConfig.diskRotation.z());


    // Place the disk.
    new G4PVPlacement(
        mineralRotation,
        fSourceConfig.diskPosition,
        mineralLV,
        "mineralSource",
        logicWorld,
        false,
        0,
        checkOverlaps
    );
}

void DetectorConstruction::ConstructDetector(
  G4LogicalVolume* logicWorld,
  G4bool checkOverlaps
)
{
  G4NistManager* nist = G4NistManager::Instance();

  //read detector config params
  G4double scintWidth = fDetectorConfig.scintWidth;
  G4double scintHeight = fDetectorConfig.scintHeight;
  G4double scintLength = fDetectorConfig.scintLength;
  G4ThreeVector scintPos = fDetectorConfig.scintPosition;
  G4double SiPMSide = fDetectorConfig.SiPMSide;
  G4double SiPMThickness = fDetectorConfig.SiPMThickness;
  G4double reflectorThickness = fDetectorConfig.reflectorThickness;
  G4double claddingThickness = fDetectorConfig.claddingThickness;

  //detector materials
  G4Material* TiO2 = G4Material::GetMaterial("TiO2");
  G4Material* claddingMat = nist->FindOrBuildMaterial("G4_POLYETHYLENE");
  G4Material* scintMat = nist->FindOrBuildMaterial("G4_CESIUM_IODIDE");
  G4Material* Si = nist->FindOrBuildMaterial("G4_Si");

  //cladding construction
  //inner dimensions include the crystal and reflector
  G4double reflectorOuterX = scintWidth + 2.0*reflectorThickness;
  G4double reflectorOuterY = scintHeight + 2.0*reflectorThickness;
  G4double reflectorOuterZ = scintLength + 2.0*reflectorThickness;
  //outer dimensions include additional cladding
  G4double claddingOuterX = reflectorOuterX + 2.0*claddingThickness;
  G4double claddingOuterY = reflectorOuterY + 2.0*claddingThickness;
  G4double claddingOuterZ = reflectorOuterZ + 2.0*claddingThickness;

  auto claddingOuterSolid = new G4Box(
      "CladdingOuter",
      claddingOuterX/2,
      claddingOuterY/2,
      claddingOuterZ/2
    );

  auto claddingInnerSolid = new G4Box(
      "CladdingInner",
      reflectorOuterX/2,
      reflectorOuterY/2,
      reflectorOuterZ/2
    );

  auto claddingSolid = new G4SubtractionSolid(
      "Cladding",
      claddingOuterSolid,
      claddingInnerSolid
    );

  auto claddingLV = new G4LogicalVolume(
      claddingSolid,
      claddingMat,
      "CladdingLV"
    );

  //reflector construction
  auto xReflectorSolid = new G4Box(
      "XReflector",
      reflectorThickness/2,
      scintHeight/2,
      scintLength/2
    );

  auto yReflectorSolid = new G4Box(
      "YReflector",
      scintWidth/2,
      reflectorThickness/2,
      scintLength/2
    );

  auto zReflectorSolid = new G4Box(
      "ZReflector",
      scintWidth/2,
      scintHeight/2,
      reflectorThickness/2
    );

  auto xReflectorLV = new G4LogicalVolume(
      xReflectorSolid,
      TiO2,
      "XReflectorLV"
    );

  auto yReflectorLV = new G4LogicalVolume(
      yReflectorSolid,
      TiO2,
      "YReflectorLV"
    );

  auto zReflectorLV = new G4LogicalVolume(
      zReflectorSolid,
      TiO2,
      "ZReflectorLV"
    );

  //SiPM construction
  auto SiPMSolid = new G4Box(
      "SiPM",
      SiPMSide/2,
      SiPMThickness/2,
      SiPMSide/2
    );
  
    auto SiPMLV = new G4LogicalVolume(
      SiPMSolid,
      Si,
      "SiPMLV"
    );

  //visualization params
  auto white = new G4VisAttributes(G4Colour(1.0, 1.0, 1.0)); // white
  auto red = new G4VisAttributes(G4Colour(1.0, 0.0, 0.0)); // red
  auto blue = new G4VisAttributes(G4Colour(0.0, 0.0, 1.0)); // blue
  auto green = new G4VisAttributes(G4Colour(0.0, 1.0, 0.0)); // green
  xReflectorLV->SetVisAttributes(white);
  yReflectorLV->SetVisAttributes(white);
  zReflectorLV->SetVisAttributes(white);
  claddingLV->SetVisAttributes(red);
  SiPMLV->SetVisAttributes(green);

  //scint construction
  auto scintSolid = new G4Box(
      "scint",
      scintWidth/2,
      scintHeight/2,
      scintLength/2
    );

  auto scintLV = new G4LogicalVolume(
      scintSolid,
      scintMat,
      "scint"
    );

  scintLV->SetVisAttributes(blue);

  //place scint
  new G4PVPlacement(
    nullptr,
    scintPos,
    scintLV,
    "scint",
    logicWorld,
    false,
    0,
    checkOverlaps
  );

  //place reflectors
  G4double xOffset = scintWidth/2 + reflectorThickness/2;
  G4double yOffset = scintHeight/2 + reflectorThickness/2;
  G4double zOffset = scintLength/2 + reflectorThickness/2;

  new G4PVPlacement(
    nullptr,
    scintPos + G4ThreeVector(-xOffset, 0, 0),
    xReflectorLV,
    "SideReflector1",
    logicWorld,
    false,
    0,
    checkOverlaps
  );

  new G4PVPlacement(
    nullptr,
    scintPos + G4ThreeVector(xOffset, 0, 0),
    xReflectorLV,
    "SideReflector2",
    logicWorld,
    false,
    0,
    checkOverlaps
  );

  new G4PVPlacement(
    nullptr,
    scintPos + G4ThreeVector(0, -yOffset, 0),
    yReflectorLV,
    "SideReflector3",
    logicWorld,
    false,
    0,
    checkOverlaps
  );

  new G4PVPlacement(
    nullptr,
    scintPos + G4ThreeVector(0, 0, zOffset),
    zReflectorLV,
    "SideReflector4",
    logicWorld,
    false,
    0, 
    checkOverlaps
  );

  new G4PVPlacement(
    nullptr,
    scintPos + G4ThreeVector(0, 0, -zOffset),
    zReflectorLV,
    "SideReflector5",
    logicWorld,
    false,
    0,
    checkOverlaps
  );

  //place SiPM on +Y face
  G4double SiPMYOffset = scintHeight/2 + SiPMThickness/2;
  new G4PVPlacement(
    nullptr,
    scintPos + G4ThreeVector(0, SiPMYOffset, 0),
    SiPMLV,
    "SiPM",
    logicWorld,
    false,
    0,
    checkOverlaps
  );

  //place cladding
  new G4PVPlacement(
    nullptr,
    scintPos,
    claddingLV,
    "Cladding",
    logicWorld,
    false,
    0,
    checkOverlaps
  );

  //set scoring volume to scint
  fScoringVolume = scintLV;
}

G4VPhysicalVolume* DetectorConstruction::Construct()
{
  G4bool checkOverlaps = true;

  G4NistManager* nist = G4NistManager::Instance();

  //define materials
  DefineMaterials();

  //create world volume
  G4double world_sizeXY = 30*cm;
  G4double world_sizeZ  = 30*cm;
  G4Material* world_mat = nist->FindOrBuildMaterial("G4_AIR");
  G4Box* solidWorld =  new G4Box("World", 0.5*world_sizeXY, 0.5*world_sizeXY, 0.5*world_sizeZ);
  G4LogicalVolume* logicWorld = new G4LogicalVolume(solidWorld, world_mat, "World");
  G4VPhysicalVolume* physWorld = new G4PVPlacement(0, G4ThreeVector(), logicWorld, "World", 0, false, 0, checkOverlaps);

  ConstructSource(logicWorld, checkOverlaps);
  ConstructDetector(logicWorld, checkOverlaps);
  return physWorld;
}

/* G4VPhysicalVolume* DetectorConstruction::Construct()
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

auto mesh = CADMesh::TessellatedMesh::FromSTL("D:/Geant4/radiacode-sims/marinelli_volume.stl");
auto solid = mesh->GetSolid();
auto marinelliLV = new G4LogicalVolume(solid, KCl, "marinelliLV");

G4ThreeVector marinelliPos = G4ThreeVector(0*cm, 0*cm, 0*cm);

new G4PVPlacement(0, marinelliPos, marinelliLV, "marinelli", logicWorld, false, 0, checkOverlaps); 

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
  G4ThreeVector scintPos = G4ThreeVector(0, 0, 3*cm);

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
} */
