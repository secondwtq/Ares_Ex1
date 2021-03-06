#include "Body.h"
#include "../../Utilities/TemplateDef.h"

#include <WarheadTypeClass.h>

//Static init
template<> const DWORD Extension<TiberiumClass>::Canary = 0xB16B00B5;
Container<TiberiumExt> TiberiumExt::ExtMap;

template<> TiberiumExt::TT* Container<TiberiumExt>::SavingObject = nullptr;
template<> IStream* Container<TiberiumExt>::SavingStream = nullptr;

void TiberiumExt::ExtData::Initialize(TiberiumClass* pThis)
{
	//const char* pID = pThis->ID;
};

void TiberiumExt::ExtData::LoadFromINIFile(TiberiumClass* pThis, CCINIClass* pINI)
{
	const char* section = pThis->get_ID();

	INI_EX exINI(pINI);

	this->Damage.Read(exINI, section, "Damage");
	this->Warhead.Read(exINI, section, "Warhead");

	this->Heal_Step.Read(exINI, section, "Heal.Step");
	this->Heal_IStep.Read(exINI, section, "Heal.IStep");
	this->Heal_UStep.Read(exINI, section, "Heal.UStep");
	this->Heal_Delay.Read(exINI, section, "Heal.Delay");

	this->ExplosionWarhead.Read(exINI, section, "ExplosionWarhead");
	this->ExplosionDamage.Read(exINI, section, "ExplosionDamage");

	this->DebrisChance.Read(exINI, section, "Debris.Chance");
}

double TiberiumExt::ExtData::GetHealDelay() const
{
	return this->Heal_Delay.Get(RulesClass::Instance->TiberiumHeal);
}

int TiberiumExt::ExtData::GetHealStep(TechnoClass* pTechno) const
{
	auto pType = pTechno->GetTechnoType();
	int step = pType->GetRepairStep();

	switch(pType->WhatAmI()) {
	case InfantryTypeClass::AbsID:
		step = this->Heal_IStep.Get(step);
		break;
	case UnitTypeClass::AbsID:
		step = this->Heal_UStep.Get(step);
		break;
	default:
		step = this->Heal_Step.Get(step);
		break;
	}

	return step;
}

int TiberiumExt::ExtData::GetDamage() const
{
	int damage = this->AttachedToObject->Power / 10;
	if(damage < 1) {
		damage = 1;
	}

	return this->Damage.Get(damage);
}

WarheadTypeClass* TiberiumExt::ExtData::GetWarhead() const
{
	return this->Warhead.Get(RulesClass::Instance->C4Warhead);
}

WarheadTypeClass* TiberiumExt::ExtData::GetExplosionWarhead() const
{
	return this->ExplosionWarhead.Get(RulesClass::Instance->C4Warhead);
}

int TiberiumExt::ExtData::GetExplosionDamage() const
{
	return this->ExplosionDamage.Get(RulesClass::Instance->TiberiumExplosionDamage);
}

int TiberiumExt::ExtData::GetDebrisChance() const
{
	return this->DebrisChance;
}

// =============================
// load / save

bool Container<TiberiumExt>::Save(TiberiumClass* pThis, IStream* pStm) {
	TiberiumExt::ExtData* pData = this->SaveKey(pThis, pStm);

	if(pData) {
		//ULONG out;
	}

	return pData != nullptr;
}

bool Container<TiberiumExt>::Load(TiberiumClass* pThis, IStream* pStm) {
	TiberiumExt::ExtData* pData = this->LoadKey(pThis, pStm);

	UNREFERENCED_PARAMETER(pData);

	return pData != nullptr;
}

// =============================
// container hooks

DEFINE_HOOK(721876, TiberiumClass_CTOR, 5)
{
	GET(TiberiumClass*, pThis, ESI);

	TiberiumExt::ExtMap.FindOrAllocate(pThis);
	return 0;
}

DEFINE_HOOK(72193A, TiberiumClass_DTOR, 6)
{
	GET(TiberiumClass*, pThis, ESI);

	TiberiumExt::ExtMap.Remove(pThis);
	return 0;
}

DEFINE_HOOK_AGAIN(7220D0, TiberiumClass_SaveLoad_Prefix, 5)
DEFINE_HOOK(721E80, TiberiumClass_SaveLoad_Prefix, 7)
{
	GET_STACK(TiberiumExt::TT*, pThis, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	Container<TiberiumExt>::PrepareStream(pThis, pStm);

	return 0;
}

DEFINE_HOOK(72208C, TiberiumClass_Load_Suffix, 7)
{
	TiberiumExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(72212C, TiberiumClass_Save_Suffix, 5)
{
	TiberiumExt::ExtMap.SaveStatic();
	return 0;
}

DEFINE_HOOK_AGAIN(721CDC, TiberiumClass_LoadFromINI, A)
DEFINE_HOOK_AGAIN(721CE9, TiberiumClass_LoadFromINI, A)
DEFINE_HOOK(721C7B, TiberiumClass_LoadFromINI, A)
{
	GET(TiberiumClass*, pThis, ESI);
	GET(CCINIClass*, pINI, EBX);

	TiberiumExt::ExtMap.LoadFromINI(pThis, pINI);
	return 0;
}
