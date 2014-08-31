#include <ArrayClasses.h>
#include "../Ares.h"
#include "Prerequisites.h"

Enumerable<GenericPrerequisite>::container_t Enumerable<GenericPrerequisite>::Array;

const char * Enumerable<GenericPrerequisite>::GetMainSection()
{
	return "GenericPrerequisites";
}

//	Kyouma Hououin 140831EVE
//		for generic generic prerequisite

void GenericPrerequisite::LoadFromINI(CCINIClass *pINI)
{
	const char *section = GenericPrerequisite::GetMainSection();

	char generalbuf[0x80];

	char name[0x80];
	strcpy_s(name, this->Name);

	_strlwr_s(name);
	name[0] &= ~0x20; // LOL HACK to uppercase a letter

	DynamicVectorClass<PrerequisiteStruct> *dvc = &this->Prereqs;

	_snprintf_s(generalbuf, _TRUNCATE, "Prerequisite%s", name);
	Prereqs::Parse(pINI, "General", generalbuf, dvc);

	Prereqs::Parse(pINI, section, this->Name, dvc);
}

void GenericPrerequisite::AddDefaults()
{
	FindOrAllocate("POWER");
	FindOrAllocate("FACTORY");
	FindOrAllocate("BARRACKS");
	FindOrAllocate("RADAR");
	FindOrAllocate("TECH");
	FindOrAllocate("PROC");
}

void Prereqs::Parse(CCINIClass *pINI, const char *section, const char *key, DynamicVectorClass<PrerequisiteStruct> *vec)
{
	if(pINI->ReadString(section, key, "", Ares::readBuffer, Ares::readLength)) {
		vec->Clear();

		char* context = nullptr;
		for(char *cur = strtok_s(Ares::readBuffer, ",", &context); cur; cur = strtok_s(nullptr, ",", &context)) {

			TechnoTypeClass *pretp = TechnoTypeClass::Find(cur);
			if(pretp) {
				vec->AddItem(PrerequisiteStruct(false, pretp));
			} else {
				int idx = GenericPrerequisite::FindIndex(cur);
				if(idx > -1) {
					vec->AddItem(PrerequisiteStruct(true, nullptr, -1 - idx));
				} else {
					Debug::INIParseFailed(section, key, cur);
				}
			}
		}
	}
}

	// helper funcs

bool Prereqs::HouseOwnsGeneric(HouseClass *pHouse, PrerequisiteStruct &src)
{
	int Index = -1 - src.GenericIndex; // hack - POWER is -1 , this way converts to 0, and onwards
	if(Index < static_cast<int>(GenericPrerequisite::Array.size())) {
		DynamicVectorClass<PrerequisiteStruct> *dvc = &GenericPrerequisite::Array.at(Index)->Prereqs;
		for(int i = 0; i < dvc->Count; ++i) {
			if(HouseOwnsSpecific(pHouse, dvc->GetItem(i))) {
				return true;
			}
		}
		if(Index == 5) { // PROC alternate, man I hate the special cases
			if(UnitTypeClass *ProcAlt = RulesClass::Instance->PrerequisiteProcAlternate) {
				if(pHouse->OwnedUnitTypes.GetItemCount(ProcAlt->GetArrayIndex())) {
					return true;
				}
			}
		}
		return false;
	}
	return false;
}

bool Prereqs::HouseOwnsSpecific(HouseClass *pHouse, PrerequisiteStruct &src)
{
	TechnoTypeClass *pType = src.SpecificType;
	if (pType->WhatAmI() == abs_BuildingType) {
		char *powerup = static_cast<BuildingTypeClass *>(pType)->PowersUpBuilding;
		if(*powerup) {
			BuildingTypeClass *BCore = BuildingTypeClass::Find(powerup);
			if(pHouse->OwnedBuildingTypes1.GetItemCount(BCore->GetArrayIndex()) < 1) {
				return false;
			}
			for(int i = 0; i < pHouse->Buildings.Count; ++i) {
				BuildingClass *Bld = pHouse->Buildings.GetItem(i);
				if(Bld->Type != BCore) {
					continue;
				}
				for(int j = 0; j < 3; ++j) {
					BuildingTypeClass *Upgrade = Bld->Upgrades[j];
					if(Upgrade == pType) {
						return true;
					}
				}
			}
			return false;
		} else {
			int building_idx = pType->GetArrayIndex();
			return pHouse->OwnedBuildingTypes1.GetItemCount(building_idx) > 0;
		}
	} else {
		return pHouse->CountOwnedNowTotal(pType) > 0;
	}
	return false;
}

bool Prereqs::HouseOwnsPrereq(HouseClass *pHouse, PrerequisiteStruct &src)
{
	return src.isGeneric
		? HouseOwnsGeneric(pHouse, src)
		: HouseOwnsSpecific(pHouse, src)
	;
}

bool Prereqs::HouseOwnsAll(HouseClass *pHouse, DynamicVectorClass<PrerequisiteStruct> *list)
{
	for(int i = 0; i < list->Count; ++i) {
		if(!HouseOwnsPrereq(pHouse, list->GetItem(i))) {
			return false;
		}
	}
	return true;
}

bool Prereqs::HouseOwnsAny(HouseClass *pHouse, DynamicVectorClass<PrerequisiteStruct> *list)
{
	for(int i = 0; i < list->Count; ++i) {
		if(HouseOwnsPrereq(pHouse, list->GetItem(i))) {
			return true;
		}
	}
	return false;
}

bool Prereqs::ListContainsSpecific(const BTypeIter &List, PrerequisiteStruct &src)
{
	if (src.SpecificType->WhatAmI() == abs_BuildingType) {
		auto Target = BuildingTypeClass::Array->GetItem(src.SpecificType->GetArrayIndex());
		return List.contains(Target);
	}
	return false;
}

bool Prereqs::ListContainsGeneric(const BTypeIter &List, PrerequisiteStruct &src)
{
	int Index = -1 - src.GenericIndex; // hack - POWER is -1 , this way converts to 0, and onwards
	if(Index < static_cast<int>(GenericPrerequisite::Array.size())) {
		DynamicVectorClass<PrerequisiteStruct> *dvc = &GenericPrerequisite::Array.at(Index)->Prereqs;
		for(int i = 0; i < dvc->Count; ++i) {
			if(ListContainsSpecific(List, dvc->GetItem(i))) {
				return true;
			}
		}
	}
	return false;
}

bool Prereqs::ListContainsPrereq(const BTypeIter &List, PrerequisiteStruct &src)
{
	return src.isGeneric
		? ListContainsGeneric(List, src)
		: ListContainsSpecific(List, src)
	;
}

bool Prereqs::ListContainsAll(const BTypeIter &List, DynamicVectorClass<PrerequisiteStruct> *Requirements)
{
	for(int i = 0; i < Requirements->Count; ++i) {
		if(!ListContainsPrereq(List, Requirements->GetItem(i))) {
			return false;
		}
	}
	return true;
}

bool Prereqs::ListContainsAny(const BTypeIter &List, DynamicVectorClass<PrerequisiteStruct> *Requirements)
{
	for(int i = 0; i < Requirements->Count; ++i) {
		if(ListContainsPrereq(List, Requirements->GetItem(i))) {
			return true;
		}
	}
	return false;
}

