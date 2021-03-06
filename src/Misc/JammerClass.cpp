#include "JammerClass.h"
#include <BuildingClass.h>
#include <HouseClass.h>
#include <GeneralStructures.h>

#include "../Ext/Building/Body.h"
#include "../Ext/TechnoType/Body.h"
#include "Debug.h"

void JammerClass::Update() {
	// we don't want to scan & crunch numbers every frame - this limits it to ScanInterval frames
	if((Unsorted::CurrentFrame - this->LastScan) < this->ScanInterval) {
		return;
	}

	// walk through all buildings
	for(int i = 0; i < BuildingClass::Array->Count; ++i) {
		BuildingClass* curBuilding = BuildingClass::Array->GetItem(i);

		// for each jammable building ...
		if(this->IsEligible(curBuilding)) {
			// ...check if it's in range, and jam or unjam based on that
			if(this->InRangeOf(curBuilding)) {
				this->Jam(curBuilding);
			} else {
				this->Unjam(curBuilding);
			}
		} else {
			this->Unjam(curBuilding); // doing this since it could be ineligible after a takeover, for example
			// (e.g. we jammed it before as a hostile building, and then our Engineer took it over - wouldn't want it to be eternally jammed.)
		}
	}

	// lastly, save the current frame for future reference
	this->LastScan = Unsorted::CurrentFrame;
}

//! \param TargetBuilding The building whose eligibility to check.
bool JammerClass::IsEligible(BuildingClass* TargetBuilding) {
	/* Current requirements for being eligible:
		- not an ally (includes ourselves)
		- either a radar or a spysat
	*/
	return (!this->AttachedToObject->Owner->IsAlliedWith(TargetBuilding->Owner)
			&& (TargetBuilding->Type->Radar || TargetBuilding->Type->SpySat));
}

//! \param TargetBuilding The building to check the distance to.
bool JammerClass::InRangeOf(BuildingClass* TargetBuilding) {
	TechnoTypeExt::ExtData* TTExt = TechnoTypeExt::ExtMap.Find(this->AttachedToObject->GetTechnoType());
	CoordStruct JammerLocation = this->AttachedToObject->Location;
	const double JamRadiusInLeptons = 256.0 * TTExt->RadarJamRadius;

	return (TargetBuilding->Location.DistanceFrom(JammerLocation) <= JamRadiusInLeptons);
}

//! \param TargetBuilding The building to jam.
void JammerClass::Jam(BuildingClass* TargetBuilding) {
	BuildingExt::ExtData* TheBuildingExt = BuildingExt::ExtMap.Find(TargetBuilding);
	TheBuildingExt->RegisteredJammers.insert(this->AttachedToObject, true);
	TargetBuilding->Owner->RecheckRadar = true;
}

//! \param TargetBuilding The building to unjam.
void JammerClass::Unjam(BuildingClass* TargetBuilding) {
	BuildingExt::ExtData* TheBuildingExt = BuildingExt::ExtMap.Find(TargetBuilding);
	TheBuildingExt->RegisteredJammers.erase(this->AttachedToObject);
	TargetBuilding->Owner->RecheckRadar = true;
}

void JammerClass::UnjamAll() {
	for(int i = 0; i < BuildingClass::Array->Count; ++i) {
		this->Unjam(BuildingClass::Array->GetItem(i));
	}
}
