#include "DetectorConstructionMessenger.hh"
#include "DetectorConstruction.hh"

#include "G4UIdirectory.hh"
#include "G4UIcommand.hh"

#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWith3VectorAndUnit.hh"

#include "G4ApplicationState.hh"

DetectorConstructionMessenger::DetectorConstructionMessenger(DetectorConstruction* detector)
: G4UImessenger(),
  fDetector(detector)
  {
    //directories
    fGeometryDir = new G4UIdirectory("/geometry/");
    fGeometryDir->SetGuidance("Geometry configuration.");

    fDetectorDir = new G4UIdirectory("/geometry/detector/");
    fDetectorDir->SetGuidance("Detector geometry configuration.");

    fCrystalDir = new G4UIdirectory("/geometry/detector/crystal/");
    fCrystalDir->SetGuidance("Crystal geometry configuration.");

    fSiPMDir = new G4UIdirectory("/geometry/detector/sipm/");
    fSiPMDir->SetGuidance("SiPM geometry configuration.");

    fReflectorDir = new G4UIdirectory("/geometry/detector/reflector/");
    fReflectorDir->SetGuidance("Reflector geometry configuration.");

    fCladdingDir = new G4UIdirectory("/geometry/detector/cladding/");
    fCladdingDir->SetGuidance("Cladding geometry configuration.");
    
    fSourceDir = new G4UIdirectory("/geometry/source/");
    fSourceDir->SetGuidance("Source geometry configuration.");

    fMarinelliDir = new G4UIdirectory("/geometry/source/marinelli/");
    fMarinelliDir->SetGuidance("Marinelli source geometry configuration.");

    fDiskDir = new G4UIdirectory("/geometry/source/disk/");
    fDiskDir->SetGuidance("Disk source geometry configuration.");

    // crystal commands
    fCrystalDimensionsCmd = new G4UIcmdWith3VectorAndUnit("/geometry/detector/crystal/dimensions", this);

    fCrystalDimensionsCmd->SetGuidance("Set crystal dimensions: width(X), height(Y), length(Z).");

    fCrystalDimensionsCmd->SetParameterName("width", "height", "length", false);

    fCrystalDimensionsCmd->SetDefaultUnit("mm");
    fCrystalDimensionsCmd->AvailableForStates(G4State_PreInit);


    fCrystalPositionCmd = new G4UIcmdWith3VectorAndUnit("/geometry/detector/crystal/position", this);

    fCrystalPositionCmd->SetGuidance("Set crystal center position.");

    fCrystalPositionCmd->SetParameterName("x", "y", "z", false);

    fCrystalPositionCmd->SetDefaultUnit("mm");
    fCrystalPositionCmd->AvailableForStates(G4State_PreInit);

    //SiPM commands
    fSiPMSideCmd = new G4UIcmdWithADoubleAndUnit("/geometry/detector/sipm/side", this);
    fSiPMSideCmd->SetGuidance("Set SiPM side length.");
    fSiPMSideCmd->SetParameterName("side", false);
    fSiPMSideCmd->SetDefaultUnit("mm");
    fSiPMSideCmd->AvailableForStates(G4State_PreInit);

    fSiPMThicknessCmd = new G4UIcmdWithADoubleAndUnit("/geometry/detector/sipm/thickness", this);
    fSiPMThicknessCmd->SetGuidance("Set SiPM thickness.");
    fSiPMThicknessCmd->SetParameterName("thickness", false);
    fSiPMThicknessCmd->SetDefaultUnit("mm");
    fSiPMThicknessCmd->AvailableForStates(G4State_PreInit);

    //reflector commands
    fReflectorThicknessCmd = new G4UIcmdWithADoubleAndUnit("/geometry/detector/reflector/thickness", this);
    fReflectorThicknessCmd->SetGuidance("Set reflector thickness.");
    fReflectorThicknessCmd->SetParameterName("thickness", false);
    fReflectorThicknessCmd->SetDefaultUnit("mm");
    fReflectorThicknessCmd->AvailableForStates(G4State_PreInit);

    //cladding commands
    fCladdingThicknessCmd = new G4UIcmdWithADoubleAndUnit("/geometry/detector/cladding/thickness", this);
    fCladdingThicknessCmd->SetGuidance("Set cladding thickness.");
    fCladdingThicknessCmd->SetParameterName("thickness", false);
    fCladdingThicknessCmd->SetDefaultUnit("mm");
    fCladdingThicknessCmd->AvailableForStates(G4State_PreInit);

    //source type command
    fSourceTypeCmd = new G4UIcmdWithAString("/geometry/source/type", this);
    fSourceTypeCmd->SetGuidance("Set source type: 'marinelli' or 'disk'.");
    fSourceTypeCmd->SetParameterName("type", false);
    fSourceTypeCmd->SetCandidates("marinelli disk");
    fSourceTypeCmd->AvailableForStates(G4State_PreInit);

    //marinelli commands
    fMarinelliMeshFileCmd = new G4UIcmdWithAString("/geometry/source/marinelli/meshfile", this);
    fMarinelliMeshFileCmd->SetGuidance("Set the STL mesh file for the Marinelli source. Must be ASCII STL format.");
    fMarinelliMeshFileCmd->SetParameterName("filename", false);
    fMarinelliMeshFileCmd->AvailableForStates(G4State_PreInit);

    fMarinelliPositionCmd = new G4UIcmdWith3VectorAndUnit("/geometry/source/marinelli/position", this);
    fMarinelliPositionCmd->SetGuidance("Set Marinelli source position.");
    fMarinelliPositionCmd->SetParameterName("x", "y", "z", false);
    fMarinelliPositionCmd->SetDefaultUnit("mm");
    fMarinelliPositionCmd->AvailableForStates(G4State_PreInit);

    //disk commands
    fDiskDiameterCmd = new G4UIcmdWithADoubleAndUnit("/geometry/source/disk/diameter", this);
    fDiskDiameterCmd->SetGuidance("Set disk source diameter.");
    fDiskDiameterCmd->SetParameterName("diameter", false);
    fDiskDiameterCmd->SetDefaultUnit("mm");
    fDiskDiameterCmd->AvailableForStates(G4State_PreInit);

    fDiskThicknessCmd = new G4UIcmdWithADoubleAndUnit("/geometry/source/disk/thickness", this);
    fDiskThicknessCmd->SetGuidance("Set disk source thickness.");
    fDiskThicknessCmd->SetParameterName("thickness", false);
    fDiskThicknessCmd->SetDefaultUnit("mm");
    fDiskThicknessCmd->AvailableForStates(G4State_PreInit);

    fDiskPositionCmd = new G4UIcmdWith3VectorAndUnit("/geometry/source/disk/position", this);
    fDiskPositionCmd->SetGuidance("Set disk source position.");
    fDiskPositionCmd->SetParameterName("x", "y", "z", false);
    fDiskPositionCmd->SetDefaultUnit("mm");
    fDiskPositionCmd->AvailableForStates(G4State_PreInit);

    fDiskRotationCmd = new G4UIcmdWith3VectorAndUnit("/geometry/source/disk/rotation", this);
    fDiskRotationCmd->SetGuidance("Set disk source rotation (Euler angles in degrees).");
    fDiskRotationCmd->SetParameterName("x", "y", "z", false);
    fDiskRotationCmd->SetDefaultUnit("deg");
    fDiskRotationCmd->AvailableForStates(G4State_PreInit);
  }

  DetectorConstructionMessenger::~DetectorConstructionMessenger()
{
    // Commands
    delete fCrystalDimensionsCmd;
    delete fCrystalPositionCmd;

    delete fSiPMSideCmd;
    delete fSiPMThicknessCmd;

    delete fReflectorThicknessCmd;
    delete fCladdingThicknessCmd;

    delete fSourceTypeCmd;

    delete fMarinelliMeshFileCmd;
    delete fMarinelliPositionCmd;

    delete fDiskDiameterCmd;
    delete fDiskThicknessCmd;
    delete fDiskPositionCmd;
    delete fDiskRotationCmd;


    // Directories
    delete fCrystalDir;
    delete fSiPMDir;
    delete fReflectorDir;
    delete fCladdingDir;
    delete fDetectorDir;

    delete fMarinelliDir;
    delete fDiskDir;
    delete fSourceDir;

    delete fGeometryDir;
}

void DetectorConstructionMessenger::SetNewValue(
    G4UIcommand* command,
    G4String newValue)
{
    // ------------------------------------------------
    // Detector commands
    // ------------------------------------------------

    if (command == fCrystalDimensionsCmd)
    {
        fDetector->SetCrystalDimensions(
            fCrystalDimensionsCmd->GetNew3VectorValue(newValue)
        );
    }

    else if (command == fCrystalPositionCmd)
    {
        fDetector->SetCrystalPosition(
            fCrystalPositionCmd->GetNew3VectorValue(newValue)
        );
    }

    else if (command == fSiPMSideCmd)
    {
        fDetector->SetSiPMSide(
            fSiPMSideCmd->GetNewDoubleValue(newValue)
        );
    }

    else if (command == fSiPMThicknessCmd)
    {
        fDetector->SetSiPMThickness(
            fSiPMThicknessCmd->GetNewDoubleValue(newValue)
        );
    }

    else if (command == fReflectorThicknessCmd)
    {
        fDetector->SetReflectorThickness(
            fReflectorThicknessCmd->GetNewDoubleValue(newValue)
        );
    }

    else if (command == fCladdingThicknessCmd)
    {
        fDetector->SetCladdingThickness(
            fCladdingThicknessCmd->GetNewDoubleValue(newValue)
        );
    }


    // ------------------------------------------------
    // Source commands
    // ------------------------------------------------

    else if (command == fSourceTypeCmd)
    {
        if (newValue == "Marinelli")
        {
            fDetector->SetSourceType(SourceGeometry::Marinelli);
        }
        else if (newValue == "Disk")
        {
            fDetector->SetSourceType(SourceGeometry::Disk);
        }
    }

    else if (command == fMarinelliMeshFileCmd)
    {
        fDetector->SetMarinelliMeshFile(newValue);
    }

    else if (command == fMarinelliPositionCmd)
    {
        fDetector->SetMarinelliPosition(
            fMarinelliPositionCmd->GetNew3VectorValue(newValue)
        );
    }

    else if (command == fDiskDiameterCmd)
    {
        fDetector->SetDiskDiameter(
            fDiskDiameterCmd->GetNewDoubleValue(newValue)
        );
    }

    else if (command == fDiskThicknessCmd)
    {
        fDetector->SetDiskThickness(
            fDiskThicknessCmd->GetNewDoubleValue(newValue)
        );
    }

    else if (command == fDiskPositionCmd)
    {
        fDetector->SetDiskPosition(
            fDiskPositionCmd->GetNew3VectorValue(newValue)
        );
    }

    else if (command == fDiskRotationCmd)
    {
        fDetector->SetDiskRotation(
            fDiskRotationCmd->GetNew3VectorValue(newValue)
        );
    }
}