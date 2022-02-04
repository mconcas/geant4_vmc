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
// $Id: B4DetectorConstruction.cc 77601 2013-11-26 17:08:44Z gcosmo $
//
/// \file B4DetectorConstruction.cxx
/// \brief Implementation of the B4DetectorConstruction class

#include "B4DetectorConstruction.hh"

// Added for VMC
#include "TG4GeometryManager.h"

#include "G4Material.hh"
#include "G4NistManager.hh"

#include "G4AutoDelete.hh"
#include "G4Box.hh"
#include "G4GlobalMagFieldMessenger.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4PVReplica.hh"

#include "G4GeometryManager.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4SolidStore.hh"

#include "G4Colour.hh"
#include "G4VisAttributes.hh"

#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4ThreadLocal G4GlobalMagFieldMessenger*
  B4DetectorConstruction::fMagFieldMessenger = 0;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

B4DetectorConstruction::B4DetectorConstruction()
  : G4VUserDetectorConstruction(),
    fCheckOverlaps(true)
{}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

B4DetectorConstruction::~B4DetectorConstruction() {}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VPhysicalVolume* B4DetectorConstruction::Construct()
{
  // Define materials
  DefineMaterials();

  // Define volumes
  return DefineVolumes();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void B4DetectorConstruction::DefineMaterials()
{
  // Lead material defined using NIST Manager
  G4NistManager* nistManager = G4NistManager::Instance();
  nistManager->FindOrBuildMaterial("G4_Pb");

  // Liquid argon material
  G4double a; // mass of a mole;
  G4double z; // z=mean number of protons;
  G4double density;
  new G4Material(
    "liquidArgon", z = 18., a = 39.95 * g / mole, density = 1.390 * g / cm3);
  // The argon by NIST Manager is a gas with a different density

  // Vacuum
  new G4Material("Galactic", z = 1., a = 1.01 * g / mole,
    density = universe_mean_density, kStateGas, 2.73 * kelvin, 3.e-18 * pascal);

  // Print materials
  G4cout << *(G4Material::GetMaterialTable()) << G4endl;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VPhysicalVolume* B4DetectorConstruction::DefineVolumes()
{
  // Geometry parameters
  G4int nofLayers = 10;
  G4double absoThickness = 10. * mm;
  G4double gapThickness = 5. * mm;
  G4double calorSizeXY = 10. * cm;

  G4double layerThickness = absoThickness + gapThickness;
  G4double calorThickness = nofLayers * layerThickness;
  G4double worldSizeXY = 1.2 * calorSizeXY;
  G4double worldSizeZ = 1.2 * calorThickness;

  // Get materials
  G4Material* defaultMaterial = G4Material::GetMaterial("Galactic");
  G4Material* absorberMaterial = G4Material::GetMaterial("G4_Pb");
  G4Material* gapMaterial = G4Material::GetMaterial("liquidArgon");

  if (!defaultMaterial || !absorberMaterial || !gapMaterial) {
    G4ExceptionDescription msg;
    msg << "Cannot retrieve materials already defined.";
    G4Exception("B4DetectorConstruction::DefineVolumes()", "MyCode0001",
      FatalException, msg);
  }

  //
  // World
  //
  G4VSolid* worldS = new G4Box("World",                // its name
    worldSizeXY / 2, worldSizeXY / 2, worldSizeZ / 2); // its size

  G4LogicalVolume* worldLV = new G4LogicalVolume(worldS, // its solid
    defaultMaterial,                                     // its material
    "WRLD");                                             // its name

  G4VPhysicalVolume* worldPV = new G4PVPlacement(0, // no rotation
    G4ThreeVector(),                                // at (0,0,0)
    worldLV,                                        // its logical volume
    "WRLD",                                         // its name
    0,                                              // its mother  volume
    false,                                          // no boolean operation
    0,                                              // copy number
    fCheckOverlaps);                                // checking overlaps

  //
  // Calorimeter
  //
  G4VSolid* calorimeterS = new G4Box("Calorimeter",        // its name
    calorSizeXY / 2, calorSizeXY / 2, calorThickness / 2); // its size

  G4LogicalVolume* calorLV = new G4LogicalVolume(calorimeterS, // its solid
    defaultMaterial,                                           // its material
    "CALO");                                                   // its name

  new G4PVPlacement(0, // no rotation
    G4ThreeVector(),   // at (0,0,0)
    calorLV,           // its logical volume
    "CALO",            // its name
    worldLV,           // its mother  volume
    false,             // no boolean operation
    0,                 // copy number
    fCheckOverlaps);   // checking overlaps

  //
  // Layer
  //
  G4VSolid* layerS = new G4Box("Layer",                    // its name
    calorSizeXY / 2, calorSizeXY / 2, layerThickness / 2); // its size

  G4LogicalVolume* layerLV = new G4LogicalVolume(layerS, // its solid
    defaultMaterial,                                     // its material
    "LAYE");                                             // its name

  new G4PVReplica("LAYE", // its name
    layerLV,              // its logical volume
    calorLV,              // its mother
    kZAxis,               // axis of replication
    nofLayers,            // number of replica
    layerThickness);      // witdth of replica

  //
  // Absorber
  //
  auto absoHz = absoThickness / 2;
  // Make the absorber layer composed of two volumes to test setting
  // a sensitive detector to multiple volumes of the same name
  absoHz /= 2.;
  G4VSolid* absorberS = new G4Box("Abso",       // its name
    calorSizeXY / 2, calorSizeXY / 2, absoHz);  // its size

  G4LogicalVolume* absorberLV1 = new G4LogicalVolume(absorberS, // its solid
    absorberMaterial,                                           // its material
    "ABSO");                                                    // its name
  G4LogicalVolume* absorberLV2 = new G4LogicalVolume(absorberS, // its solid
    absorberMaterial,                                           // its material
    "ABSO");                                                    // its name


  auto posz = -gapThickness / 2 - absoHz;
  G4cout << "abso hz " << absoHz << " posz1 " << posz << G4endl;
  new G4PVPlacement(0,             // no rotation
    G4ThreeVector(0., 0., posz),   // its position
    absorberLV1,                   // its logical volume
    "ABSO",                        // its name
    layerLV,                       // its mother  volume
    false,                         // no boolean operation
    0,                             // copy number
    fCheckOverlaps);               // checking overlaps

  posz += 2. * absoHz;
  G4cout << " posz2 " << posz << G4endl;
  new G4PVPlacement(0,             // no rotation
    G4ThreeVector(0., 0., posz),   // its position
    absorberLV2,                   // its logical volume
    "ABSO",                        // its name
    layerLV,                       // its mother  volume
    false,                         // no boolean operation
    1,                             // copy number
    fCheckOverlaps);               // checking overlaps


  //
  // Gap
  //
  auto gapHz = gapThickness / 2;
  // Make the gap layer composed of two volumes to test setting
  // a sensitive detector to multiple volumes of the same name
  gapHz /= 2.;
  G4VSolid* gapS = new G4Box("Gap",            // its name
    calorSizeXY / 2, calorSizeXY / 2, gapHz);  // its size

  G4LogicalVolume* gapLV1 = new G4LogicalVolume(gapS, // its solid
    gapMaterial,                                      // its material
    "GAPX");                                          // its name
  G4LogicalVolume* gapLV2 = new G4LogicalVolume(gapS, // its solid
    gapMaterial,                                      // its material
    "GAPX");                                          // its name

  posz = absoThickness / 2 - gapHz;
  G4cout << "gap hz " << gapHz << " posz1 " << posz << G4endl;
  new G4PVPlacement(0,            // no rotation
    G4ThreeVector(0., 0., posz),  // its position
    gapLV1,                       // its logical volume
    "GAPX",                       // its name
    layerLV,                      // its mother  volume
    false,                        // no boolean operation
    0,                            // copy number
    fCheckOverlaps);              // checking overlaps

  posz += 2. * gapHz;
  G4cout << " posz2 " << posz << G4endl;
  new G4PVPlacement(0,            // no rotation
    G4ThreeVector(0., 0., posz),  // its position
    gapLV2,                       // its logical volume
    "GAPX",                       // its name
    layerLV,                      // its mother  volume
    false,                        // no boolean operation
    1,                            // copy number
    fCheckOverlaps);              // checking overlaps

  //
  // print parameters
  //
  G4cout << "\n------------------------------------------------------------"
         << "\n---> The calorimeter is " << nofLayers << " layers of: [ "
         << absoThickness / mm << "mm of " << absorberMaterial->GetName()
         << " + " << gapThickness / mm << "mm of " << gapMaterial->GetName()
         << " ] "
         << "\n------------------------------------------------------------\n";

  //
  // Visualization attributes
  //
  worldLV->SetVisAttributes(G4VisAttributes::GetInvisible());

  G4VisAttributes* simpleBoxVisAtt =
    new G4VisAttributes(G4Colour(1.0, 1.0, 1.0));
  simpleBoxVisAtt->SetVisibility(true);
  calorLV->SetVisAttributes(simpleBoxVisAtt);

  //
  // Always return the physical World
  //
  return worldPV;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void B4DetectorConstruction::ConstructSDandField()
{
  // Added for VMC
  TG4GeometryManager::Instance()->ConstructSDandField();
  /*
    // Create global magnetic field messenger.
    // Uniform magnetic field is then created automatically if
    // the field value is not zero.
    G4ThreeVector fieldValue = G4ThreeVector();
    fMagFieldMessenger = new G4GlobalMagFieldMessenger(fieldValue);
    fMagFieldMessenger->SetVerboseLevel(1);

    // Register the field messenger for deleting
    G4AutoDelete::Register(fMagFieldMessenger);
  */
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
