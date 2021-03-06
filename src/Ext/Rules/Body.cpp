#include "Body.h"
#include "../Side/Body.h"
#include "../HouseType/Body.h"
#include "../../Enum/Prerequisites.h"
#include "../../Enum/ArmorTypes.h"
#include "../../Enum/RadTypes.h"
#include "../../Utilities/TemplateDef.h"
#include <GameModeOptionsClass.h>

template<> const DWORD Extension<RulesClass>::Canary = 0x12341234;
std::unique_ptr<RulesExt::ExtData> RulesExt::Data = nullptr;

template<> RulesExt::TT *Container<RulesExt>::SavingObject = nullptr;
template<> IStream *Container<RulesExt>::SavingStream = nullptr;

void RulesExt::Allocate(RulesClass *pThis) {
	Data = std::make_unique<RulesExt::ExtData>(pThis);
}

void RulesExt::Remove(RulesClass *pThis) {
	Data = nullptr;
}

void RulesExt::LoadFromINIFile(RulesClass *pThis, CCINIClass *pINI) {
	Data->LoadFromINI(pThis, pINI);
}

void RulesExt::LoadBeforeTypeData(RulesClass *pThis, CCINIClass *pINI) {
	GenericPrerequisite::LoadFromINIList(pINI);
	ArmorType::LoadFromINIList(pINI);

	SideExt::ExtMap.LoadAllFromINI(pINI);
	HouseTypeExt::ExtMap.LoadAllFromINI(pINI);

	Data->LoadBeforeTypeData(pThis, pINI);
}

void RulesExt::LoadAfterTypeData(RulesClass *pThis, CCINIClass *pINI) {
	Data->LoadAfterTypeData(pThis, pINI);
}

void RulesExt::ExtData::InitializeConstants(RulesClass *pThis) {
	GenericPrerequisite::AddDefaults();
	ArmorType::AddDefaults();
}

void RulesExt::ExtData::LoadFromINIFile(RulesClass *pThis, CCINIClass *pINI) {
	// earliest loader - can't really do much because nothing else is initialized yet, so lookups won't work
}

void RulesExt::ExtData::LoadBeforeTypeData(RulesClass *pThis, CCINIClass *pINI) {
	const char* section = "WeaponTypes";
	const char* sectionGeneral = "General";
	const char* sectionCombatDamage = "CombatDamage";
	const char* sectionAV = "AudioVisual";

	int len = pINI->GetKeyCount(section);
	for (int i = 0; i < len; ++i) {
		const char *key = pINI->GetKeyName(section, i);
		if (pINI->ReadString(section, key, "", Ares::readBuffer, Ares::readLength)) {
			WeaponTypeClass::FindOrAllocate(Ares::readBuffer);
		}
	}

	RulesExt::ExtData *pData = RulesExt::Global();

	if (!pData) {
		return;
	}

	INI_EX exINI(pINI);

	pData->CanMakeStuffUp.Read(exINI, sectionGeneral, "CanMakeStuffUp");

	pData->Tiberium_DamageEnabled.Read(exINI, sectionGeneral, "TiberiumDamageEnabled");
	pData->Tiberium_HealEnabled.Read(exINI, sectionGeneral, "TiberiumHealEnabled");
	pData->Tiberium_ExplosiveWarhead.Read(exINI, sectionCombatDamage, "TiberiumExplosiveWarhead");

	pData->OverlayExplodeThreshold.Read(exINI, sectionGeneral, "OverlayExplodeThreshold");

	pData->EnemyInsignia.Read(exINI, sectionGeneral, "EnemyInsignia");

	pData->ReturnStructures.Read(exINI, sectionGeneral, "ReturnStructures");

	pData->TypeSelectUseDeploy.Read(exINI, sectionGeneral, "TypeSelectUseDeploy");

	pData->TeamRetaliate.Read(exINI, sectionGeneral, "TeamRetaliate");

	pData->DeactivateDim_Powered.Read(exINI, sectionAV, "DeactivateDimPowered");
	pData->DeactivateDim_EMP.Read(exINI, sectionAV, "DeactivateDimEMP");
	pData->DeactivateDim_Operator.Read(exINI, sectionAV, "DeactivateDimOperator");

	pData->AutoRepelAI.Read(exINI, sectionCombatDamage, "AutoRepel");
	pData->AutoRepelPlayer.Read(exINI, sectionCombatDamage, "PlayerAutoRepel");
}

// this should load everything that TypeData is not dependant on
// i.e. InfantryElectrocuted= can go here since nothing refers to it
// but [GenericPrerequisites] have to go earlier because they're used in parsing TypeData
void RulesExt::ExtData::LoadAfterTypeData(RulesClass *pThis, CCINIClass *pINI) {
	RulesExt::ExtData *pData = RulesExt::Global();

	if (!pData) {
		return;
	}

	INI_EX exINI(pINI);

	pData->ElectricDeath.Read(exINI, "AudioVisual", "InfantryElectrocuted");

	pData->DecloakSound.Read(exINI, "AudioVisual", "DecloakSound");
	pData->CloakHeight.Read(exINI, "General", "CloakHeight");

	for (int i = 0; i < WeaponTypeClass::Array->Count; ++i) {
		WeaponTypeClass::Array->GetItem(i)->LoadFromINI(pINI);
	}

	pData->EngineerDamage = pINI->ReadDouble("General", "EngineerDamage", pData->EngineerDamage);
	pData->EngineerAlwaysCaptureTech = pINI->ReadBool("General", "EngineerAlwaysCaptureTech", pData->EngineerAlwaysCaptureTech);
	pData->EngineerDamageCursor.Read(exINI, "General", "EngineerDamageCursor");

	pData->HunterSeekerBuildings.Read(exINI, "SpecialWeapons", "HSBuilding");
	pData->HunterSeekerDetonateProximity.Read(exINI, "General", "HunterSeekerDetonateProximity");
	pData->HunterSeekerDescendProximity.Read(exINI, "General", "HunterSeekerDescendProximity");
	pData->HunterSeekerAscentSpeed.Read(exINI, "General", "HunterSeekerAscentSpeed");
	pData->HunterSeekerDescentSpeed.Read(exINI, "General", "HunterSeekerDescentSpeed");
	pData->HunterSeekerEmergeSpeed.Read(exINI, "General", "HunterSeekerEmergeSpeed");

	pData->DropPodTrailer.Read(exINI, "General", "DropPodTrailer");
	pData->DropPodTypes.Read(exINI, "General", "DropPodTypes");
	pData->DropPodMinimum.Read(exINI, "General", "DropPodMinimum");
	pData->DropPodMaximum.Read(exINI, "General", "DropPodMaximum");
}

// =============================
// container hooks

DEFINE_HOOK(667A1D, RulesClass_CTOR, 5) {
	GET(RulesClass*, pItem, ESI);

	RulesExt::Allocate(pItem);
	return 0;
}

DEFINE_HOOK(667A30, RulesClass_DTOR, 5) {
	GET(RulesClass*, pItem, ECX);

	RulesExt::Remove(pItem);
	return 0;
}

/*
A_FINE_HOOK_AGAIN(674730, RulesClass_SaveLoad_Prefix, 6)
A_FINE_HOOK(675210, RulesClass_SaveLoad_Prefix, 5)
{
	//GET(RulesExt::TT*, pItem, ECX);
	//GET_STACK(IStream*, pStm, 0x4);

	return 0;
}

A_FINE_HOOK(678841, RulesClass_Load_Suffix, 7)
{
	return 0;
}

A_FINE_HOOK(675205, RulesClass_Save_Suffix, 8)
{
	return 0;
}
*/
