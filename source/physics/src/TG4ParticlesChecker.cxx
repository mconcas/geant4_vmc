// $Id$

//------------------------------------------------
// The Geant4 Virtual Monte Carlo package
// Copyright (C) 2007 - 2010 Ivana Hrivnacova
// All rights reserved.
//
// For the licensing terms see geant4_vmc/LICENSE.
// Contact: vmc@pcroot.cern.ch
//-------------------------------------------------

/// \file TG4ParticlesChecker.cxx
/// \brief Implementation of the TG4ParticlesChecker class 
///
/// \author I. Hrivnacova; IPN, Orsay

#include "TG4ParticlesChecker.h"
#include "TG4Globals.h"

#include <G4ParticleTable.hh>
#include <G4ParticleDefinition.hh>

#include <TDatabasePDG.h>
#include <TParticle.h>
#include <TClonesArray.h>
#include <THashList.h>
#include <iomanip>

const G4double TG4ParticlesChecker::fgkDefaultPrecision = 1.e-06;

//
// static methods
//

//_____________________________________________________________________________
G4String TG4ParticlesChecker::GetParticlePropertyName(ParticleProperty property)
{
/// Return the name of property given by ParticleProperty code

  switch (property) {
    case kMass:       return "mass";
    case kCharge:     return "charge";
    case kLifetime:   return "lifetime";
    case kWidth:      return "width";
    case kParity:     return "parity";
    case kSpin:       return "spin";
    case kIsospin:    return "isospin";
    case kIsospin3:   return "isospin3";
    case kNone:
    default:          return "";
  }  
}   

//_____________________________________________________________________________
TG4ParticlesChecker::ParticleProperty 
TG4ParticlesChecker::GetParticleProperty(const G4String& propertyName)
{
/// Return the ParticleProperty code fore the property given by name

  if      ( propertyName == GetParticlePropertyName(kMass) )     return kMass;
  else if ( propertyName == GetParticlePropertyName(kCharge) )   return kCharge;
  else if ( propertyName == GetParticlePropertyName(kLifetime) ) return kLifetime;
  else if ( propertyName == GetParticlePropertyName(kWidth) )    return kWidth;
  else if ( propertyName == GetParticlePropertyName(kParity) )   return kParity;
  else if ( propertyName == GetParticlePropertyName(kSpin) )     return kSpin;
  else if ( propertyName == GetParticlePropertyName(kIsospin) )  return kIsospin;
  else if ( propertyName == GetParticlePropertyName(kIsospin3) ) return kIsospin3;
  else                                                           return kNone;
}    

//
// ctor, dtor
//

//_____________________________________________________________________________
TG4ParticlesChecker::TG4ParticlesChecker()
  : TG4Verbose("particlesChecker"),
    fMessenger(this),
    fAvailableProperties(),
    fCheckedProperties(),
    fPrecision(fgkDefaultPrecision)
{ 
/// Default constructor

  fAvailableProperties.insert(kMass);
  fAvailableProperties.insert(kCharge);
  fAvailableProperties.insert(kLifetime);
  fAvailableProperties.insert(kWidth);
  fAvailableProperties.insert(kParity);
  fAvailableProperties.insert(kSpin);
  fAvailableProperties.insert(kIsospin);
  fAvailableProperties.insert(kIsospin3);

  fCheckedProperties.insert(kMass);
  fCheckedProperties.insert(kCharge);
  fCheckedProperties.insert(kLifetime);
  fCheckedProperties.insert(kWidth);
  
  fMessenger.Init();
}

//_____________________________________________________________________________
TG4ParticlesChecker::~TG4ParticlesChecker() 
{
/// Destructor
}

//
// private methods
//

//_____________________________________________________________________________
G4bool TG4ParticlesChecker::IsEqualRel(G4double dx, G4double dy, 
                                       G4double epsilon) const 
{
/// The function to compare given values dx and dy;
/// the values are found equal if there difference relative to the first value
/// is within the given precision epsilon.
    
  return fabs(dx - dy) <= epsilon * fabs(dx);
}

//_____________________________________________________________________________
G4double TG4ParticlesChecker::GetPropertyValue(ParticleProperty property, 
                                               G4ParticleDefinition* g4Particle) const
{
/// Return the given property value for the given Geant4 particle

  switch (property) {
    case kMass:     return g4Particle->GetPDGMass()/GeV;
    case kCharge:   return g4Particle->GetPDGCharge();
    case kLifetime: return g4Particle->GetPDGLifeTime()/second;
    case kWidth:    return g4Particle->GetPDGWidth()/GeV;
    case kParity:   return g4Particle->GetPDGiParity();
    case kSpin:     return g4Particle->GetPDGSpin();
    case kIsospin:  return g4Particle->GetPDGIsospin();
    case kIsospin3: return g4Particle->GetPDGIsospin3();
    case kNone:
    default:   return 0.;
  }  
}  
                                                                   
//_____________________________________________________________________________
G4double TG4ParticlesChecker::GetPropertyValue(ParticleProperty property, 
                                               TParticlePDG* rtParticle) const
{
/// Return the given property value for the given Root particle

  switch (property) {
    case kMass:     return rtParticle->Mass(); 
    case kCharge:   return rtParticle->Charge()/3.;
    case kLifetime: return rtParticle->Lifetime();
    case kWidth:    return rtParticle->Width();
    case kParity:   return rtParticle->Parity();
    case kSpin:     return rtParticle->Spin();
    case kIsospin:  return rtParticle->Isospin();
    case kIsospin3: return rtParticle->I3();
    case kNone:
    default:        return 0.;
  }
}

//_____________________________________________________________________________
void TG4ParticlesChecker::PrintCheckedProperties() const
{
/// Print the list of properties selected for checking and the selected precision

  std::set<ParticleProperty>::const_iterator it;
  G4cout << "Checking properties: ";
  for ( it = fCheckedProperties.begin(); it != fCheckedProperties.end(); it++ ) {
    G4cout << GetParticlePropertyName(*it) << "  ";
  }  
  G4cout << "  within precision " << fPrecision << G4endl;
}                 

//_____________________________________________________________________________
G4bool TG4ParticlesChecker::CheckProperty(ParticleProperty property,
                                   G4ParticleDefinition* g4Particle, 
                                   TParticlePDG* rtParticle) const
{
/// Check if the given property values in Geant4 and Root for the given particle 
/// are equal within the defined precision. Return true if values match,
/// otherwise print the property name and values and return false.

  G4String propertyName = GetParticlePropertyName(property);
  G4double g4Value = GetPropertyValue(property, g4Particle);
  G4double rtValue = GetPropertyValue(property, rtParticle);

  if ( ! IsEqualRel(g4Value, rtValue, fPrecision) && 
       ! ( property == kLifetime && g4Value < 0 && rtValue == 0 ) ) {   
    G4cout << "  " 
           << std::setw(10) << propertyName 
           << std::setw(8) << "  Root: " << std::setw(12) << rtValue 
           << std::setw(6) << "  G4: " << std::setw(12) << g4Value;
    if ( g4Value != 0 ) {
      G4cout << std::setw(6) << "  eps: " 
             << std::setw(12) << fabs(g4Value - rtValue)/g4Value;
    }         
    G4cout << G4endl;
    return false;
  }
  else {
    if ( VerboseLevel() > 1 )
      G4cout << "  " << propertyName << " equal" << G4endl;
    return true;
  }
}                     

//_____________________________________________________________________________
G4bool TG4ParticlesChecker::CheckParticle(G4ParticleDefinition* g4Particle, 
                                          TParticlePDG* rtParticle) const
{
/// Check all selected properties for the given Geant4 and Root particles.

  G4bool resultAll = true;
  std::set<ParticleProperty>::const_iterator it;
  for ( it = fCheckedProperties.begin(); it != fCheckedProperties.end(); it++ ) {
    G4bool result = CheckProperty(*it, g4Particle, rtParticle);
    resultAll = resultAll && result;
  } 
  
  if ( resultAll )  G4cout << "  all properties match" << G4endl;
  G4cout << G4endl;  
    
  return resultAll;
}  


//
// public methods
//

//_____________________________________________________________________________
G4bool TG4ParticlesChecker::CheckParticles() const
{
/// Loop over Root particles database and check the particles
/// properties wrt Geant4 particles. Return true if all particles selected 
/// properties match.

  G4bool resultAll = true;

  G4ParticleTable* g4ParticleTable 
    = G4ParticleTable::GetParticleTable();                

  const THashList* rootParticles
    = TDatabasePDG::Instance()->ParticleList();
    
  PrintCheckedProperties();   
    
  TIter next(rootParticles);
  TObject* obj;
  while ((obj = next())) {
    TParticlePDG* rootParticle = (TParticlePDG*)obj;
    G4int pdgCode = rootParticle->PdgCode(); 
    G4ParticleDefinition* g4Particle
      = g4ParticleTable->FindParticle(pdgCode);
      
    if ( g4Particle ||  VerboseLevel() > 1 ) {  
      // print particle info
      G4cout << "=====================================================================" 
             << G4endl
             << "Particle: " 
             << std::setw(16) << rootParticle->GetName()
             << "  pdg:  " << std::setw(10) <<  pdgCode
             << G4endl
             << "=====================================================================" 
             << G4endl;
    }         

    if ( ! g4Particle ) {  
      if ( VerboseLevel() > 1 ) 
        G4cout << "  no G4 particle " << G4endl;
      continue;
    }
    
    G4bool result = CheckParticle(g4Particle, rootParticle);
    resultAll = resultAll && result;
  }
  
  return resultAll;  
}
                       
//_____________________________________________________________________________
G4bool TG4ParticlesChecker::CheckParticle(G4int pdgEncoding) const
{
/// Check the properties of the particle with given pdgEncoding.
/// Return true if all particle selected properties match                     

  TParticlePDG* rootParticle 
    = TDatabasePDG::Instance()->GetParticle(pdgEncoding);
  if ( ! rootParticle ) {
    TString text = "Particle with PDG encoding ";
    text += pdgEncoding;
    text += " not found in TDatabasePDG.";
    TG4Globals::Warning("TG4ParticlesChecker", "CheckParticle", text);
    return false;
  }

  G4ParticleDefinition* g4Particle
    = G4ParticleTable::GetParticleTable()->FindParticle(pdgEncoding);                
  if ( ! g4Particle ) {
    TString text = "Particle with PDG encoding ";
    text += pdgEncoding;
    text += " not found in G4ParticleTable.";
    TG4Globals::Warning("TG4ParticlesChecker", "CheckParticle", text);
    return false;
  }

  G4int pdgCode = rootParticle->PdgCode(); 
  G4cout << "Particle: " 
         << std::setw(16) << rootParticle->GetName()
         << "  pdg:  " << std::setw(10) <<  pdgCode
         << G4endl
         << "  ";
  
  PrintCheckedProperties();              

  return CheckParticle(g4Particle, rootParticle);
}  

//_____________________________________________________________________________
void TG4ParticlesChecker::SetChecking(ParticleProperty property, G4bool check)
{
/// Select or deselect the given property for checking.

  std::set<ParticleProperty>::const_iterator it
    = fCheckedProperties.find(property);
    
  if ( (  check && it != fCheckedProperties.end() ) ||
       (! check && it == fCheckedProperties.end() ) ) {
      // no change of checking property
      return;
  }  

  if ( check ) {
    fCheckedProperties.insert(property);
  }
  else {
    fCheckedProperties.erase(it);
  }    
}  
