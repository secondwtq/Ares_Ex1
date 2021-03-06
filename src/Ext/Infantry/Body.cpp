#include <InfantryClass.h>
#include <ScenarioClass.h>

#include <BuildingClass.h>
#include <CellClass.h>
#include <HouseClass.h>
#include <SessionClass.h>

#include "Body.h"

#include "../Rules/Body.h"
#include <GameModeOptionsClass.h>

template<> const DWORD Extension<InfantryClass>::Canary = 0xE1E2E3E4;
Container<InfantryExt> InfantryExt::ExtMap;

template<> InfantryClass *Container<InfantryExt>::SavingObject = nullptr;
template<> IStream *Container<InfantryExt>::SavingStream = nullptr;

bool InfantryExt::ExtData::IsOccupant() {
	InfantryClass* thisTrooper = this->AttachedToObject;

	// under the assumption that occupants' position is correctly updated and not stuck on the pre-occupation one
	if(BuildingClass* buildingBelow = thisTrooper->GetCell()->GetBuilding()) {
		// cases where this could be false would be Nighthawks or Rocketeers above the building
		return (buildingBelow->Occupants.FindItemIndex(thisTrooper) != -1);
	} else {
		return false; // if there is no building, he can't occupy one
	}
}

//! Gets the action an engineer will take when entering an enemy building.
/*!
	This function accounts for the multi-engineer feature.

	\param pBld The Building the engineer enters.

	\author AlexB
	\date 2012-07-22
*/
eAction InfantryExt::GetEngineerEnterEnemyBuildingAction(BuildingClass *pBld) {
	// no other mode than skirmish allows to disable it, so we only check whether
	// multi engineer is disabled there. for all other modes, it's always on.
	// single player campaigns also use special multi engineer behavior.
	bool allowDamage = SessionClass::Instance->GameMode != GameMode::Skirmish || GameModeOptionsClass::Instance->MultiEngineer;

	// single player missions are currently hardcoded to "don't do damage".
	bool campaignSupportsDamage = false; // replace this by a new rules tag.
	if(SessionClass::Instance->GameMode == GameMode::Campaign && !campaignSupportsDamage) {
		allowDamage = false;
	}

	// damage if multi engineer is enabled and target isn't that low on health.
	if(allowDamage) {

		// check to always capture tech structures. a structure counts
		// as tech if its initial owner is a multiplayer-passive country.
		bool isTech = false;
		if(HouseClass * pHouse = pBld->InitialOwner) {
			if(HouseTypeClass * pCountry = pHouse->Type) {
				isTech = pCountry->MultiplayPassive;
			}
		}

		if(!RulesExt::Global()->EngineerAlwaysCaptureTech || !isTech) {
			// no civil structure. apply new logic.
			if(pBld->GetHealthPercentage() > RulesClass::Global()->EngineerCaptureLevel) {
				return (RulesExt::Global()->EngineerDamage > 0) ? act_Damage : act_NoEnter;
			}
		}
	}

	// default.
	return act_Capture;
}

// =============================
// container hooks

#ifdef MAKE_GAME_SLOWER_FOR_NO_REASON
A_FINE_HOOK(517CB0, InfantryClass_CTOR, 5)
{
	GET(InfantryClass*, pItem, ESI);

	InfantryExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

A_FINE_HOOK(517F83, InfantryClass_DTOR, 6)
{
	GET(InfantryClass*, pItem, ESI);

	InfantryExt::ExtMap.Remove(pItem);
	return 0;
}

A_FINE_HOOK_AGAIN(521B00, InfantryClass_SaveLoad_Prefix, 8)
A_FINE_HOOK(521960, InfantryClass_SaveLoad_Prefix, 6)
{
	GET_STACK(InfantryExt::TT*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	Container<InfantryExt>::PrepareStream(pItem, pStm);

	return 0;
}

A_FINE_HOOK(521AEC, InfantryClass_Load_Suffix, 6)
{
	InfantryExt::ExtMap.LoadStatic();
	return 0;
}

A_FINE_HOOK(521B14, InfantryClass_Save_Suffix, 3)
{
	InfantryExt::ExtMap.SaveStatic();
	return 0;
}
#endif
