#include "Body.h"
#include <WeaponTypeClass.h>
#include "../../Enum/ArmorTypes.h"

#include <Helpers/Template.h>

const DWORD Extension<WarheadTypeClass>::Canary = 0x22222222;
Container<WarheadTypeExt> WarheadTypeExt::ExtMap;

WarheadTypeExt::TT *Container<WarheadTypeExt>::SavingObject = NULL;
IStream *Container<WarheadTypeExt>::SavingStream = NULL;

hash_ionExt WarheadTypeExt::IonExt;

WarheadTypeClass * WarheadTypeExt::Temporal_WH = NULL;

void WarheadTypeExt::ExtData::LoadFromINIFile(WarheadTypeClass *pThis, CCINIClass *pINI)
{
	const char * section = pThis->get_ID();

	INI_EX exINI(pINI);

	if(!pINI->GetSection(section)) {
		return;
	}

	// writing custom verses parser just because
	char buffer[0x100];
	if(pINI->ReadString(section, "Verses", "", buffer, 0x100)) {
		int idx = 0;
		for(char *cur = strtok(buffer, ","); cur; cur = strtok(NULL, ",")) {
			DWORD specialFX = 0x0;
			this->Verses[idx].Verses = Conversions::Str2Armor(cur, &specialFX);

			this->Verses[idx].ForceFire = ((specialFX & verses_ForceFire) != 0);
			this->Verses[idx].Retaliate = ((specialFX & verses_Retaliate) != 0);
			this->Verses[idx].PassiveAcquire = ((specialFX & verses_PassiveAcquire) != 0);

			++idx;
			if(idx > 10) {
				break;
			}
		}
	}

	ArmorType::LoadForWarhead(pINI, pThis);

	if(pThis->MindControl) {
		this->MindControl_Permanent = pINI->ReadBool(section, "MindControl.Permanent", this->MindControl_Permanent);
		this->Is_Custom |= this->MindControl_Permanent;
	}

	if(pThis->EMEffect) {
		this->EMP_Duration = pINI->ReadInteger(section, "EMP.Duration", this->EMP_Duration);
		this->Is_Custom |= 1;
	}

	this->IC_Duration = pINI->ReadInteger(section, "IronCurtain.Duration", this->IC_Duration);
	this->Is_Custom |= this->IC_Duration != 0;

	if(pThis->Temporal) {
		this->Temporal_WarpAway.Parse(&exINI, section, "Temporal.WarpAway");
	}

	this->DeployedDamage = pINI->ReadDouble(section, "Damage.Deployed", this->DeployedDamage);

	this->Ripple_Radius = pINI->ReadInteger(section, "Ripple.Radius", this->Ripple_Radius);
};

void WarheadTypeExt::PointerGotInvalid(void *ptr) {
	AnnounceInvalidPointerMap(IonExt, ptr);
	AnnounceInvalidPointer(Temporal_WH, ptr);
}

// =============================
// load/save
void Container<WarheadTypeExt>::Save(WarheadTypeClass *pThis, IStream *pStm) {
	WarheadTypeExt::ExtData* pData = this->SaveKey(pThis, pStm);

	if(pData) {
		ULONG out;
		pData->Verses.Save(pStm);
	}
}
/*
		pStm->Write(&IonBlastClass::Array->Count, 4, &out);
		for(int ii = 0; ii < IonBlastClass::Array->Count; ++ii) {
			IonBlastClass *ptr = IonBlastClass::Array->Items[ii];
			pStm->Write(ptr, 4, &out);
			pStm->Write(WarheadTypeExt::IonExt[ptr], 4, &out);
		}
*/
void Container<WarheadTypeExt>::Load(WarheadTypeClass *pThis, IStream *pStm) {
	WarheadTypeExt::ExtData* pData = this->LoadKey(pThis, pStm);

	pData->Verses.Load(pStm, 0);
	
	SWIZZLE(pData->Temporal_WarpAway);
}

// =============================
// container hooks

DEFINE_HOOK(75D1A9, WarheadTypeClass_CTOR, 7)
{
	GET(WarheadTypeClass*, pItem, EBP);

	WarheadTypeExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(75E510, WarheadTypeClass_DTOR, 6)
{
	GET(WarheadTypeClass*, pItem, ECX);

	WarheadTypeExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK(75E0C0, WarheadTypeClass_SaveLoad_Prefix, 8)
DEFINE_HOOK_AGAIN(75E2C0, WarheadTypeClass_SaveLoad_Prefix, 5)
{
	GET_STACK(WarheadTypeExt::TT*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8); 

	Container<WarheadTypeExt>::SavingObject = pItem;
	Container<WarheadTypeExt>::SavingStream = pStm;

	return 0;
}

DEFINE_HOOK(75E2AE, WarheadTypeClass_Load_Suffix, 7)
{
	WarheadTypeExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(75E39C, WarheadTypeClass_Save_Suffix, 5)
{
	WarheadTypeExt::ExtMap.SaveStatic();
	return 0;
}

DEFINE_HOOK(75DEA0, WarheadTypeClass_LoadFromINI, 5)
DEFINE_HOOK_AGAIN(75DEAF, WarheadTypeClass_LoadFromINI, 5)
{
	GET(WarheadTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, 0x150);

	WarheadTypeExt::ExtMap.LoadFromINI(pItem, pINI);
	return 0;
}
