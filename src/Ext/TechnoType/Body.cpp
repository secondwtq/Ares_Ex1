#include "Body.h"
#include "../BuildingType/Body.h"
#include "../HouseType/Body.h"
#include "../Side/Body.h"
#include "../../Enum/Prerequisites.h"
#include "../../Misc/Debug.h"
#include "../../Utilities/TemplateDef.h"

#include <AbstractClass.h>
#include <PCX.h>
#include <Theater.h>
#include <VocClass.h>
#include <WarheadTypeClass.h>

template<> const DWORD Extension<TechnoTypeClass>::Canary = 0x44444444;
Container<TechnoTypeExt> TechnoTypeExt::ExtMap;

template<> TechnoTypeExt::TT *Container<TechnoTypeExt>::SavingObject = nullptr;
template<> IStream *Container<TechnoTypeExt>::SavingStream = nullptr;

// =============================
// member funcs

void TechnoTypeExt::ExtData::Initialize(TechnoTypeClass *pThis) {
	this->Survivors_PilotChance.SetAll(-1); // was int(RulesClass::Instance->CrewEscape * 100), now negative values indicate "use CrewEscape"
	this->Survivors_PassengerChance.SetAll(-1); // was (int)RulesClass::Global()->CrewEscape * 100); - changed to -1 to indicate "100% if this is a land transport"

	this->Survivors_PilotCount = -1; // defaults to (crew ? 1 : 0)

	this->PrerequisiteLists.clear();
	this->PrerequisiteLists.push_back(std::make_unique<DynamicVectorClass<PrerequisiteStruct>>());

	this->PrerequisiteTheaters = 0xFFFFFFFF;

	this->Secret_RequiredHouses = 0xFFFFFFFF;
	this->Secret_ForbiddenHouses = 0;

	this->Is_Deso = this->Is_Deso_Radiation = !strcmp(pThis->ID, "DESO");
	this->Is_Cow = !strcmp(pThis->ID, "COW");

	if(pThis->WhatAmI() == AircraftTypeClass::AbsID) {
		this->CustomMissileTrailerAnim = AnimTypeClass::Find("V3TRAIL");
		this->CustomMissileTakeoffAnim = AnimTypeClass::Find("V3TAKOFF");

		this->SmokeAnim = AnimTypeClass::Find("SGRYSMK1");
	}

	this->EVA_UnitLost = VoxClass::FindIndex("EVA_UnitLost");
}

/*
EXT_LOAD(TechnoTypeClass)
{
	if(CONTAINS(Ext_p, pThis))
	{
		Create(pThis);

		ULONG out;
		pStm->Read(&Ext_p[pThis], sizeof(ExtData), &out);

		Ext_p[pThis]->Survivors_Pilots.Load(pStm);
		Ext_p[pThis]->Weapons.Load(pStm);
		Ext_p[pThis]->EliteWeapons.Load(pStm);
		for ( int ii = 0; ii < Ext_p[pThis]->Weapons.get_Count(); ++ii )
			SWIZZLE(Ext_p[pThis]->Weapons[ii].WeaponType);
		for ( int ii = 0; ii < Ext_p[pThis]->EliteWeapons.get_Count(); ++ii )
			SWIZZLE(Ext_p[pThis]->EliteWeapons[ii].WeaponType);
		SWIZZLE(Ext_p[pThis]->Insignia_R);
		SWIZZLE(Ext_p[pThis]->Insignia_V);
		SWIZZLE(Ext_p[pThis]->Insignia_E);
	}
}

EXT_SAVE(TechnoTypeClass)
{
	if(CONTAINS(Ext_p, pThis))
	{
		ULONG out;
		pStm->Write(&Ext_p[pThis], sizeof(ExtData), &out);

		Ext_p[pThis]->Survivors_Pilots.Save(pStm);
		Ext_p[pThis]->Weapons.Save(pStm);
		Ext_p[pThis]->EliteWeapons.Save(pStm);
	}
}
*/

void TechnoTypeExt::ExtData::LoadFromINIFile(TechnoTypeClass *pThis, CCINIClass *pINI)
{
	const char * section = pThis->ID;

	if(!pINI->GetSection(section)) {
		return;
	}

	// survivors
	this->Survivors_Pilots.Reserve(SideClass::Array->Count);
	for(int i=this->Survivors_Pilots.Count; i<SideClass::Array->Count; ++i) {
		this->Survivors_Pilots[i] = nullptr;
	}
	this->Survivors_Pilots.Count = SideClass::Array->Count;

	this->Survivors_PilotCount = pINI->ReadInteger(section, "Survivor.Pilots", this->Survivors_PilotCount);

	this->Survivors_PilotChance.Read(pINI, section, "Survivor.%sPilotChance");
	this->Survivors_PassengerChance.Read(pINI, section, "Survivor.%sPassengerChance");

	char flag[256];
	for(int i = 0; i < SideClass::Array->Count; ++i) {
		_snprintf_s(flag, 255, "Survivor.Side%d", i);
		if(pINI->ReadString(section, flag, "", Ares::readBuffer, Ares::readLength)) {
			if((this->Survivors_Pilots[i] = InfantryTypeClass::Find(Ares::readBuffer)) == nullptr) {
				if(!INIClass::IsBlank(Ares::readBuffer)) {
					Debug::INIParseFailed(section, flag, Ares::readBuffer);
				}
			}
		}
	}

	// prereqs
	int PrereqListLen = pINI->ReadInteger(section, "Prerequisite.Lists", this->PrerequisiteLists.size() - 1);

	if(PrereqListLen < 1) {
		PrereqListLen = 0;
	}
	++PrereqListLen;
	while(PrereqListLen > static_cast<int>(this->PrerequisiteLists.size())) {
		this->PrerequisiteLists.push_back(std::make_unique<DynamicVectorClass<PrerequisiteStruct>>());
	}
	this->PrerequisiteLists.erase(this->PrerequisiteLists.begin() + PrereqListLen, this->PrerequisiteLists.end());

	DynamicVectorClass<PrerequisiteStruct> *dvc = this->PrerequisiteLists.at(0).get();
	Prereqs::Parse(pINI, section, "Prerequisite", dvc);

	dvc = &this->PrerequisiteOverride;
	Prereqs::Parse(pINI, section, "PrerequisiteOverride", dvc);

	for(size_t i = 0; i < this->PrerequisiteLists.size(); ++i) {
		_snprintf_s(flag, 255, "Prerequisite.List%u", i);
		dvc = this->PrerequisiteLists.at(i).get();
		Prereqs::Parse(pINI, section, flag, dvc);
	}

	dvc = &this->PrerequisiteNegatives;
	Prereqs::Parse(pINI, section, "Prerequisite.Negative", dvc);

	if(pINI->ReadString(section, "Prerequisite.RequiredTheaters", "", Ares::readBuffer, Ares::readLength)) {
		this->PrerequisiteTheaters = 0;

		char* context = nullptr;
		for(char *cur = strtok_s(Ares::readBuffer, ",", &context); cur; cur = strtok_s(nullptr, ",", &context)) {
			signed int idx = Theater::FindIndex(cur);
			if(idx != -1) {
				this->PrerequisiteTheaters |= (1 << idx);
			} else {
				Debug::INIParseFailed(section, "Prerequisite.RequiredTheaters", cur);
			}
		}
	}

	// new secret lab
	this->Secret_RequiredHouses
		= pINI->ReadHouseTypesList(section, "SecretLab.RequiredHouses", this->Secret_RequiredHouses);

	this->Secret_ForbiddenHouses
		= pINI->ReadHouseTypesList(section, "SecretLab.ForbiddenHouses", this->Secret_ForbiddenHouses);

	this->Is_Deso = pINI->ReadBool(section, "IsDesolator", this->Is_Deso);
	this->Is_Deso_Radiation = pINI->ReadBool(section, "IsDesolator.RadDependant", this->Is_Deso_Radiation);
	this->Is_Cow  = pINI->ReadBool(section, "IsCow", this->Is_Cow);

	this->Is_Spotlighted = pINI->ReadBool(section, "HasSpotlight", this->Is_Spotlighted);
	this->Spot_Height = pINI->ReadInteger(section, "Spotlight.StartHeight", this->Spot_Height);
	this->Spot_Distance = pINI->ReadInteger(section, "Spotlight.Distance", this->Spot_Distance);
	if(pINI->ReadString(section, "Spotlight.AttachedTo", "", Ares::readBuffer, Ares::readLength)) {
		if(!_strcmpi(Ares::readBuffer, "body")) {
			this->Spot_AttachedTo = sa_Body;
		} else if(!_strcmpi(Ares::readBuffer, "turret")) {
			this->Spot_AttachedTo = sa_Turret;
		} else if(!_strcmpi(Ares::readBuffer, "barrel")) {
			this->Spot_AttachedTo = sa_Barrel;
		} else {
			Debug::INIParseFailed(section, "Spotlight.AttachedTo", Ares::readBuffer);
		}
	}
	this->Spot_DisableR = pINI->ReadBool(section, "Spotlight.DisableRed", this->Spot_DisableR);
	this->Spot_DisableG = pINI->ReadBool(section, "Spotlight.DisableGreen", this->Spot_DisableG);
	this->Spot_DisableB = pINI->ReadBool(section, "Spotlight.DisableBlue", this->Spot_DisableB);
	this->Spot_Reverse = pINI->ReadBool(section, "Spotlight.IsInverted", this->Spot_Reverse);

	this->Is_Bomb = pINI->ReadBool(section, "IsBomb", this->Is_Bomb);

/*
	this is too late - Art files are loaded before this hook fires... brilliant
	if(pINI->ReadString(section, "WaterVoxel", "", buffer, 256)) {
		this->WaterAlt = 1;
	}
*/

	INI_EX exINI(pINI);
	this->Insignia.Read(pINI, section, "Insignia.%s");
	this->Parachute_Anim.Read(exINI, section, "Parachute.Anim");

	// new on 08.11.09 for #342 (Operator=)
	if(pINI->ReadString(section, "Operator", "", Ares::readBuffer, Ares::readLength)) { // try to read the flag
		this->IsAPromiscuousWhoreAndLetsAnyoneRideIt = (strcmp(Ares::readBuffer, "_ANY_") == 0); // set whether this type accepts all operators
		if(!this->IsAPromiscuousWhoreAndLetsAnyoneRideIt) { // if not, find the specific operator it allows
			if(auto Operator = InfantryTypeClass::Find(Ares::readBuffer)) {
				this->Operator = Operator;
			} else if(!INIClass::IsBlank(Ares::readBuffer)) {
				Debug::INIParseFailed(section, "Operator", Ares::readBuffer);
			}
		}
	}

	this->CameoPal.LoadFromINI(CCINIClass::INI_Art, pThis->ImageFile, "CameoPalette");

	if(pINI->ReadString(section, "Prerequisite.StolenTechs", "", Ares::readBuffer, Ares::readLength)) {
		this->RequiredStolenTech.reset();

		char* context = nullptr;
		for(char *cur = strtok_s(Ares::readBuffer, ",", &context); cur; cur = strtok_s(nullptr, ",", &context)) {
			signed int idx = atoi(cur);
			if(idx > -1 && idx < 32) {
				this->RequiredStolenTech.set(idx);
			} else if(idx != -1) {
				Debug::INIParseFailed(section, "Prerequisite.StolenTechs", cur, "Expected a number between 0 and 31 inclusive");
			}
		}
	}

	this->ImmuneToEMP.Read(exINI, section, "ImmuneToEMP");
	this->EMP_Modifier = (float)pINI->ReadDouble(section, "EMP.Modifier", this->EMP_Modifier);

	if(pINI->ReadString(section, "EMP.Threshold", "inair", Ares::readBuffer, Ares::readLength)) {
		if(_strcmpi(Ares::readBuffer, "inair") == 0) {
			this->EMP_Threshold = -1;
		} else if((_strcmpi(Ares::readBuffer, "yes") == 0) || (_strcmpi(Ares::readBuffer, "true") == 0)) {
			this->EMP_Threshold = 1;
		} else if((_strcmpi(Ares::readBuffer, "no") == 0) || (_strcmpi(Ares::readBuffer, "false") == 0)) {
			this->EMP_Threshold = 0;
		} else {
			this->EMP_Threshold = pINI->ReadInteger(section, "EMP.Threshold", this->EMP_Threshold);
		}
	}

	if(pINI->ReadString(section, "VeteranAbilities", "", Ares::readBuffer, Ares::readLength)) {
		char* context = nullptr;
		for(char *cur = strtok_s(Ares::readBuffer, ",", &context); cur; cur = strtok_s(nullptr, ",", &context)) {
			if(!_strcmpi(cur, "empimmune")) {
				this->VeteranAbilityEMPIMMUNE = true;
				this->EliteAbilityEMPIMMUNE = true;
			}
		}
	}

	if(pINI->ReadString(section, "EliteAbilities", "", Ares::readBuffer, Ares::readLength)) {
		char* context = nullptr;
		for(char *cur = strtok_s(Ares::readBuffer, ",", &context); cur; cur = strtok_s(nullptr, ",", &context)) {
			if(!_strcmpi(cur, "empimmune")) {
				this->EliteAbilityEMPIMMUNE = true;
			}
		}
	}

	// #733
	this->ProtectedDriver = pINI->ReadBool(section, "ProtectedDriver", this->ProtectedDriver);
	this->CanDrive = pINI->ReadBool(section, "CanDrive", this->CanDrive);

	// #346, #464, #970, #1014
	this->PassengersGainExperience.Read(exINI, section, "Experience.PromotePassengers");
	this->ExperienceFromPassengers.Read(exINI, section, "Experience.FromPassengers");
	this->PassengerExperienceModifier.Read(exINI, section, "Experience.PassengerModifier");
	this->MindControlExperienceSelfModifier.Read(exINI, section, "Experience.MindControlSelfModifier");
	this->MindControlExperienceVictimModifier.Read(exINI, section, "Experience.MindControlVictimModifier");
	this->SpawnExperienceOwnerModifier.Read(exINI, section, "Experience.SpawnOwnerModifier");
	this->SpawnExperienceSpawnModifier.Read(exINI, section, "Experience.SpawnModifier");
	this->ExperienceFromAirstrike.Read(exINI, section, "Experience.FromAirstrike");
	this->AirstrikeExperienceModifier.Read(exINI, section, "Experience.AirstrikeModifier");
	this->Insignia_ShowEnemy.Read(exINI, section, "Insignia.ShowEnemy");

	this->VoiceRepair.Read(exINI, section, "VoiceIFVRepair");

	this->VoiceAirstrikeAttack.Read(exINI, section, "VoiceAirstrikeAttack");
	this->VoiceAirstrikeAbort.Read(exINI, section, "VoiceAirstrikeAbort");

	this->HijackerEnterSound.Read(exINI, section, "VehicleThief.EnterSound");
	this->HijackerLeaveSound.Read(exINI, section, "VehicleThief.LeaveSound");
	this->HijackerKillPilots.Read(exINI, section, "VehicleThief.KillPilots");
	this->HijackerBreakMindControl.Read(exINI, section, "VehicleThief.BreakMindControl");
	this->HijackerAllowed.Read(exINI, section, "VehicleThief.Allowed");
	this->HijackerOneTime.Read(exINI, section, "VehicleThief.OneTime");

	this->IronCurtain_Modifier.Read(exINI, section, "IronCurtain.Modifier");

	this->ForceShield_Modifier.Read(exINI, section, "ForceShield.Modifier");

	this->Chronoshift_Allow.Read(exINI, section, "Chronoshift.Allow");
	this->Chronoshift_IsVehicle.Read(exINI, section, "Chronoshift.IsVehicle");

	this->CameoPCX.Read(CCINIClass::INI_Art, pThis->ImageFile, "CameoPCX");
	this->AltCameoPCX.Read(CCINIClass::INI_Art, pThis->ImageFile, "AltCameoPCX");

	this->CanBeReversed.Read(exINI, section, "CanBeReversed");

	// #305
	this->RadarJamRadius.Read(exINI, section, "RadarJamRadius");

	// #1208
	this->PassengerTurret.Read(exINI, section, "PassengerTurret");
	
	// #617 powered units
	this->PoweredBy.Read(exINI, section, "PoweredBy");

	//#1623 - AttachEffect on unit-creation
	this->AttachedTechnoEffect.Read(exINI);

	this->BuiltAt.Read(exINI, section, "BuiltAt");

	this->Cloneable.Read(exINI, section, "Cloneable");

	this->ClonedAt.Read(exINI, section, "ClonedAt");

	this->CarryallAllowed.Read(exINI, section, "Carryall.Allowed");
	this->CarryallSizeLimit.Read(exINI, section, "Carryall.SizeLimit");

	// #680, 1362
	this->ImmuneToAbduction.Read(exINI, section, "ImmuneToAbduction");

	this->FactoryOwners.Read(exINI, section, "FactoryOwners");
	this->ForbiddenFactoryOwners.Read(exINI, section, "FactoryOwners.Forbidden");
	this->FactoryOwners_HaveAllPlans.Read(exINI, section, "FactoryOwners.HaveAllPlans");

	// issue #896235: cyclic gattling
	this->GattlingCyclic.Read(exINI, section, "Gattling.Cycle");

	// #245 custom missiles
	if(auto pAircraftType = specific_cast<AircraftTypeClass*>(pThis)) {
		this->IsCustomMissile.Read(exINI, section, "Missile.Custom");
		this->CustomMissileData.Read(exINI, section, "Missile");
		this->CustomMissileData.GetEx()->Type = pAircraftType;
		this->CustomMissileWarhead.Read(exINI, section, "Missile.Warhead");
		this->CustomMissileEliteWarhead.Read(exINI, section, "Missile.EliteWarhead");
		this->CustomMissileTakeoffAnim.Read(exINI, section, "Missile.TakeOffAnim");
		this->CustomMissileTrailerAnim.Read(exINI, section, "Missile.TrailerAnim");
		this->CustomMissileTrailerSeparation.Read(exINI, section, "Missile.TrailerSeparation");
	}

	// non-crashable aircraft
	this->Crashable.Read(exINI, section, "Crashable");

	this->CrashSpin.Read(exINI, section, "CrashSpin");

	this->AirRate.Read(exINI, section, "AirRate");

	// tiberium
	this->TiberiumProof.Read(exINI, section, "TiberiumProof");
	this->TiberiumRemains.Read(exINI, section, "TiberiumRemains");
	this->TiberiumSpill.Read(exINI, section, "TiberiumSpill");
	this->TiberiumTransmogrify.Read(exINI, section, "TiberiumTransmogrify");

	// refinery and storage
	this->Refinery_UseStorage.Read(exINI, section, "Refinery.UseStorage");

	// cloak
	this->CloakSound.Read(exINI, section, "CloakSound");
	this->DecloakSound.Read(exINI, section, "DecloakSound");
	this->CloakPowered.Read(exINI, section, "Cloakable.Powered");
	this->CloakDeployed.Read(exINI, section, "Cloakable.Deployed");
	this->CloakAllowed.Read(exINI, section, "Cloakable.Allowed");
	this->CloakStages.Read(exINI, section, "Cloakable.Stages");

	// sensors
	this->SensorArray_Warn.Read(exINI, section, "SensorArray.Warn");

	this->EVA_UnitLost.Read(exINI, section, "EVA.Lost");

	// linking units for type selection
	if(pINI->ReadString(section, "GroupAs", "", Ares::readBuffer, Ares::readLength)) {
		if(!INIClass::IsBlank(Ares::readBuffer)) {
			AresCRT::strCopy(this->GroupAs, Ares::readBuffer);
		} else {
			*this->GroupAs = 0;
		}
	}

	// crew settings
	this->Crew_TechnicianChance.Read(exINI, section, "Crew.TechnicianChance");
	this->Crew_EngineerChance.Read(exINI, section, "Crew.EngineerChance");

	// drain settings
	this->Drain_Local.Read(exINI, section, "Drain.Local");
	this->Drain_Amount.Read(exINI, section, "Drain.Amount");

	// smoke when damaged
	this->SmokeAnim.Read(exINI, section, "Smoke.Anim");
	this->SmokeChanceRed.Read(exINI, section, "Smoke.ChanceRed");
	this->SmokeChanceDead.Read(exINI, section, "Smoke.ChanceDead");

	// hunter seeker
	this->HunterSeekerDetonateProximity.Read(exINI, section, "HunterSeeker.DetonateProximity");
	this->HunterSeekerDescendProximity.Read(exINI, section, "HunterSeeker.DescendProximity");
	this->HunterSeekerAscentSpeed.Read(exINI, section, "HunterSeeker.AscentSpeed");
	this->HunterSeekerDescentSpeed.Read(exINI, section, "HunterSeeker.DescentSpeed");
	this->HunterSeekerEmergeSpeed.Read(exINI, section, "HunterSeeker.EmergeSpeed");
	this->HunterSeekerIgnore.Read(exINI, section, "HunterSeeker.Ignore");

	this->CivilianEnemy.Read(exINI, section, "CivilianEnemy");

	// particles
	this->DamageSparks.Read(exINI, section, "DamageSparks");

	this->ParticleSystems_DamageSmoke.Read(exINI, section, "DamageSmokeParticleSystems");
	this->ParticleSystems_DamageSparks.Read(exINI, section, "DamageSparksParticleSystems");

	//	Kyouma Hououin 140831NOON
	this->CrushLevel.Read(exINI, section, "CrushLevel");
	this->DeployWeaponIndex.Read(exINI, section, "DeployWeaponIndex");

	this->Crush_Level = (this->AttachedToObject->OmniCrusher && this->CrushLevel == 1) ? this->CrushLevel+1 : this->CrushLevel;

	if (this->DeployWeaponIndex != 1) {
		this->AttachedToObject->DeployFireWeapon = this->DeployWeaponIndex;
		if (pINI->ReadString(section, "CustomDeployWeapon", "", Ares::readBuffer, Ares::readLength)) {
			if (auto CustomDeployWeapon = InfantryTypeClass::Find(Ares::readBuffer))
				this->CustomDeployWeapon = CustomDeployWeapon;
			else if (!INIClass::IsBlank(Ares::readBuffer))
				Debug::INIParseFailed(section, "CustomDeployWeapon", Ares::readBuffer);
		}
	}

	this->SpeedMultiplierOnTiberium.Read(exINI, section, "SpeedMultiplierOnTiberium");
	
	this->RecheckTechTreeWhenDeleted.Read(exINI, section, "RecheckTechTreeWhenDeleted");

	// quick fix - remove after the rest of weapon selector code is done
	return;
}

/*
	// weapons
	int WeaponCount = pINI->ReadInteger(section, "WeaponCount", pData->Weapons.get_Count());

	if(WeaponCount < 2)
	{
		WeaponCount = 2;
	}

	while(WeaponCount < pData->Weapons.get_Count())
	{
		pData->Weapons.RemoveItem(pData->Weapons.get_Count() - 1);
	}
	if(WeaponCount > pData->Weapons.get_Count())
	{
		pData->Weapons.SetCapacity(WeaponCount, nullptr);
		pData->Weapons.set_Count(WeaponCount);
	}

	while(WeaponCount < pData->EliteWeapons.get_Count())
	{
		pData->EliteWeapons.RemoveItem(pData->EliteWeapons.get_Count() - 1);
	}
	if(WeaponCount > pData->EliteWeapons.get_Count())
	{
		pData->EliteWeapons.SetCapacity(WeaponCount, nullptr);
		pData->EliteWeapons.set_Count(WeaponCount);
	}

	WeaponStruct *W = &pData->Weapons[0];
	ReadWeapon(W, "Primary", section, pINI);

	W = &pData->EliteWeapons[0];
	ReadWeapon(W, "ElitePrimary", section, pINI);

	W = &pData->Weapons[1];
	ReadWeapon(W, "Secondary", section, pINI);

	W = &pData->EliteWeapons[1];
	ReadWeapon(W, "EliteSecondary", section, pINI);

	for(int i = 0; i < WeaponCount; ++i)
	{
		W = &pData->Weapons[i];
		_snprintf(flag, 256, "Weapon%d", i);
		ReadWeapon(W, flag, section, pINI);

		W = &pData->EliteWeapons[i];
		_snprintf(flag, 256, "EliteWeapon%d", i);
		ReadWeapon(W, flag, section, pINI);
	}

void TechnoTypeClassExt::ReadWeapon(WeaponStruct *pWeapon, const char *prefix, const char *section, CCINIClass *pINI)
{
	char buffer[256];
	char flag[64];

	pINI->ReadString(section, prefix, "", buffer, 0x100);

	if(strlen(buffer))
	{
		pWeapon->WeaponType = WeaponTypeClass::FindOrAllocate(buffer);
	}

	CCINIClass *pArtINI = CCINIClass::INI_Art;

	CoordStruct FLH;
	// (Elite?)(Primary|Secondary)FireFLH - FIRE suffix
	// (Elite?)(Weapon%d)FLH - no suffix
	if(prefix[0] == 'W' || prefix[5] == 'W') // W EliteW
	{
		_snprintf(flag, 64, "%sFLH", prefix);
	}
	else
	{
		_snprintf(flag, 64, "%sFireFLH", prefix);
	}
	pArtINI->Read3Integers((int *)&FLH, section, flag, (int *)&pWeapon->FLH);
	pWeapon->FLH = FLH;

	_snprintf(flag, 64, "%sBarrelLength", prefix);
	pWeapon->BarrelLength = pArtINI->ReadInteger(section, flag, pWeapon->BarrelLength);
	_snprintf(flag, 64, "%sBarrelThickness", prefix);
	pWeapon->BarrelThickness = pArtINI->ReadInteger(section, flag, pWeapon->BarrelThickness);
	_snprintf(flag, 64, "%sTurretLocked", prefix);
	pWeapon->TurretLocked = pArtINI->ReadBool(section, flag, pWeapon->TurretLocked);
}
*/

void Container<TechnoTypeExt>::InvalidatePointer(void *ptr, bool bRemoved) {
}

const char* TechnoTypeExt::ExtData::GetSelectionGroupID() const
{
	return *this->GroupAs ? this->GroupAs : this->AttachedToObject->ID;
}

const char* TechnoTypeExt::GetSelectionGroupID(ObjectTypeClass* pType)
{
	if(auto pExt = TechnoTypeExt::ExtMap.Find(static_cast<TechnoTypeClass*>(pType))) {
		return pExt->GetSelectionGroupID();
	}

	return pType->ID;
}

bool TechnoTypeExt::HasSelectionGroupID(ObjectTypeClass* pType, const char* pID)
{
	auto id = TechnoTypeExt::GetSelectionGroupID(pType);
	return (_strcmpi(id, pID) == 0);
}

bool TechnoTypeExt::ExtData::CameoIsElite()
{
	HouseClass * House = HouseClass::Player;
	HouseTypeClass *Country = House->Type;

	TechnoTypeClass * const T = this->AttachedToObject;
	TechnoTypeExt::ExtData* pExt = TechnoTypeExt::ExtMap.Find(T);

	if(!T->AltCameo && !pExt->AltCameoPCX.Exists()) {
		return false;
	}

	switch(T->WhatAmI()) {
		case abs_InfantryType:
			if(House->BarracksInfiltrated && !T->Naval && T->Trainable) {
				return true;
			} else {
				return Country->VeteranInfantry.FindItemIndex((InfantryTypeClass *)T) != -1;
			}
		case abs_UnitType:
			if(House->WarFactoryInfiltrated && !T->Naval && T->Trainable) {
				return true;
			} else {
				return Country->VeteranUnits.FindItemIndex((UnitTypeClass *)T) != -1;
			}
		case abs_AircraftType:
			return Country->VeteranAircraft.FindItemIndex((AircraftTypeClass *)T) != -1;
		case abs_BuildingType:
			if(TechnoTypeClass *Item = T->UndeploysInto) {
				return Country->VeteranUnits.FindItemIndex((UnitTypeClass *)Item) != -1;
			} else {
				auto pData = HouseTypeExt::ExtMap.Find(Country);
				return pData->VeteranBuildings.Contains((BuildingTypeClass*)T);
			}
	}

	return false;
}

bool TechnoTypeExt::ExtData::CanBeBuiltAt(BuildingTypeClass * FactoryType) {
	auto pBExt = BuildingTypeExt::ExtMap.Find(FactoryType);
	return (!this->BuiltAt.size() && !pBExt->Factory_ExplicitOnly) || this->BuiltAt.Contains(FactoryType);
}

bool TechnoTypeExt::ExtData::CarryallCanLift(UnitClass * Target) {
	if(Target->ParasiteEatingMe) {
		return false;
	}
	auto TargetData = TechnoTypeExt::ExtMap.Find(Target->Type);
	UnitTypeClass *TargetType = Target->Type;
	bool canCarry = !TargetType->Organic && !TargetType->NonVehicle;
	if(TargetData->CarryallAllowed.isset()) {
		canCarry = !!TargetData->CarryallAllowed;
	}
	if(!canCarry) {
		return false;
	}
	if(this->CarryallSizeLimit.isset()) {
		int maxSize = this->CarryallSizeLimit;
		if(maxSize != -1) {
			return maxSize >= static_cast<TechnoTypeClass *>(Target->Type)->Size;
		}
	}
	return true;

}

// =============================
// load / save

bool Container<TechnoTypeExt>::Save(TechnoTypeClass *pThis, IStream *pStm) {
	TechnoTypeExt::ExtData* pData = this->SaveKey(pThis, pStm);

	if(pData) {
		//ULONG out;
		//pData->Survivors_Pilots.Save(pStm);

		//pData->PrerequisiteNegatives.Save(pStm);
		//pData->Weapons.Save(pStm);
		//pData->EliteWeapons.Save(pStm);
	}

	return pData != nullptr;
}

bool Container<TechnoTypeExt>::Load(TechnoTypeClass *pThis, IStream *pStm) {
	TechnoTypeExt::ExtData* pData = this->LoadKey(pThis, pStm);

	//ULONG out;

	//pData->Survivors_Pilots.Load(pStm, 1);

	//pData->PrerequisiteNegatives.Load(pStm, 0);
	//pData->Weapons.Load(pStm, 1);
	//pData->EliteWeapons.Load(pStm, 1);

/*
	SWIZZLE(pData->Parachute_Anim);
	SWIZZLE(pData->Insignia_R);
	SWIZZLE(pData->Insignia_V);
	SWIZZLE(pData->Insignia_E);
*/

	//for(int ii = 0; ii < pData->Weapons.Count; ++ii) {
	//	SWIZZLE(pData->Weapons.Items[ii].WeaponType);
	//}

	//for(int ii = 0; ii < pData->EliteWeapons.Count; ++ii) {
	//	SWIZZLE(pData->EliteWeapons.Items[ii].WeaponType);
	//}

	return pData != nullptr;
}

// =============================
// container hooks

DEFINE_HOOK(711835, TechnoTypeClass_CTOR, 5)
{
	GET(TechnoTypeClass*, pItem, ESI);

	TechnoTypeExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(711AE0, TechnoTypeClass_DTOR, 5)
{
	GET(TechnoTypeClass*, pItem, ECX);

	TechnoTypeExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(716DC0, TechnoTypeClass_SaveLoad_Prefix, 5)
DEFINE_HOOK(7162F0, TechnoTypeClass_SaveLoad_Prefix, 6)
{
	GET_STACK(TechnoTypeExt::TT*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	Container<TechnoTypeExt>::PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(716DAC, TechnoTypeClass_Load_Suffix, A)
{
	TechnoTypeExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(717094, TechnoTypeClass_Save_Suffix, 5)
{
	TechnoTypeExt::ExtMap.SaveStatic();
	return 0;
}

DEFINE_HOOK_AGAIN(716132, TechnoTypeClass_LoadFromINI, 5)
DEFINE_HOOK(716123, TechnoTypeClass_LoadFromINI, 5)
{
	GET(TechnoTypeClass*, pItem, EBP);
	GET_STACK(CCINIClass*, pINI, 0x380);

	TechnoTypeExt::ExtMap.LoadFromINI(pItem, pINI);
	return 0;
}

DEFINE_HOOK(679CAF, RulesClass_LoadAfterTypeData_CheckRubbleFoundation, 5) {
	//GET(CCINIClass*, pINI, ESI);

	for(int i=0; i<BuildingTypeClass::Array->Count; ++i) {
		BuildingTypeClass* pTBld = BuildingTypeClass::Array->GetItem(i);
		if(BuildingTypeExt::ExtData *pData = BuildingTypeExt::ExtMap.Find(pTBld)) {
			pData->CompleteInitialization(pTBld);
		}
	}

	return 0;
}
