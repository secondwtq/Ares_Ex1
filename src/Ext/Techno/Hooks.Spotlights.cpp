#include "Body.h"
#include "../TechnoType/Body.h"

#include <YRMath.h>
#include <HouseClass.h>
#include <SpotlightClass.h>
#include <TacticalClass.h>

BuildingLightClass * TechnoExt::ActiveBuildingLight = nullptr;

// just in case
DEFINE_HOOK(420F40, Spotlights_UpdateFoo, 6)
{
	return Game::CurrentFrameRate >= Game::GetMinFrameRate()
		? 0
		: 0x421346;
}

// bugfix #182: Spotlights cause an IE
DEFINE_HOOK(5F5155, ObjectClass_Put, 6)
{
	return R->EAX()
		? 0
		: 0x5F5210;
}

DEFINE_HOOK(6F6D0E, TechnoClass_Put_1, 7)
{
	GET(TechnoClass *, T, ESI);
	TechnoTypeExt::ExtData *pTypeData = TechnoTypeExt::ExtMap.Find(T->GetTechnoType());

	if(pTypeData->Is_Spotlighted) {
		auto pExt = TechnoExt::ExtMap.Find(T);
		auto pSpotlight = GameCreate<BuildingLightClass>(T);
		pExt->SetSpotlight(pSpotlight);
	}

	return 0;
}

DEFINE_HOOK(6F6F20, TechnoClass_Put_2, 6)
{
	GET(TechnoClass *, T, ESI);
	TechnoTypeExt::ExtData *pTypeData = TechnoTypeExt::ExtMap.Find(T->GetTechnoType());

	if(pTypeData->Is_Spotlighted) {
		auto pExt = TechnoExt::ExtMap.Find(T);
		auto pSpotlight = GameCreate<BuildingLightClass>(T);
		pExt->SetSpotlight(pSpotlight);
	}

	return 0;
}

DEFINE_HOOK(441163, BuildingClass_Put_DontSpawnSpotlight, 0)
{
	return 0x441196;
}

DEFINE_HOOK(435820, BuildingLightClass_CTOR, 6)
{
	GET_STACK(TechnoClass *, T, 0x4);
	GET(BuildingLightClass *, BL, ECX);

	if(auto pExt = TechnoExt::ExtMap.Find(T)) {
		pExt->Spotlight = BL;
	}
	return 0;
}

DEFINE_HOOK(4370C0, BuildingLightClass_SDDTOR, A)
{
	GET(BuildingLightClass *, BL, ECX);

	TechnoClass *T = BL->OwnerObject;
	if(auto pExt = TechnoExt::ExtMap.Find(T)) {
		pExt->Spotlight = nullptr;
	}
	return 0;
}

DEFINE_HOOK(6F4500, TechnoClass_DTOR_Spotlight, 5)
{
	GET(TechnoClass*, pItem, ECX);
	if(auto pExt = TechnoExt::ExtMap.Find(pItem)) {
		pExt->SetSpotlight(nullptr);
	}
	return 0;
}

DEFINE_HOOK(70FBE3, TechnoClass_Activate, 5)
{
	GET(TechnoClass *, T, ECX);
	TechnoTypeExt::ExtData *pTypeData = TechnoTypeExt::ExtMap.Find(T->GetTechnoType());
	TechnoExt::ExtData *pData = TechnoExt::ExtMap.Find(T);

	/* Check abort conditions:
		- Is the object currently EMP'd?
		- Does the object need an operator, but doesn't have one?
		- Does the object need a powering structure that is offline?
	   If any of the above conditions, bail out and don't activate the object.
	*/
	if(T->IsUnderEMP() || !pData->IsPowered() || !pData->IsOperated()) {
		return 0x70FC82;
	}

	if(pTypeData->Is_Spotlighted) {
		++Unsorted::IKnowWhatImDoing;
		auto pSpotlight = GameCreate<BuildingLightClass>(T);
		pData->SetSpotlight(pSpotlight);
		--Unsorted::IKnowWhatImDoing;
	}
	return 0;
}

DEFINE_HOOK(70FC97, TechnoClass_Deactivate, 6)
{
	GET(TechnoClass *, T, ESI);
	TechnoTypeExt::ExtData *pTypeData = TechnoTypeExt::ExtMap.Find(T->GetTechnoType());

	if(pTypeData->Is_Spotlighted) {
		auto pExt = TechnoExt::ExtMap.Find(T);
		pExt->SetSpotlight(nullptr);
	}
	return 0;
}

DEFINE_HOOK(435C08, BuildingLightClass_Draw_ForceType, 5)
{
	return 0x435C16;
}

DEFINE_HOOK(435C32, BuildingLightClass_Draw_PowerOnline, A)
{
	GET(TechnoClass *, T, EDI);
	return (T->WhatAmI() != abs_Building || (T->IsPowerOnline() && !static_cast<BuildingClass *>(T)->IsFogged))
		? 0x435C52
		: 0x4361BC
	;
}

DEFINE_HOOK(436459, BuildingLightClass_Update, 6)
{
	static const double Facing2Rad = (2 * 3.14) / 0xFFFF;
	GET(BuildingLightClass *, BL, EDI);
	TechnoClass *Owner = BL->OwnerObject;
	if(Owner && Owner->WhatAmI() != abs_Building) {
		TechnoTypeExt::ExtData *pTypeData = TechnoTypeExt::ExtMap.Find(Owner->GetTechnoType());
		CoordStruct Loc = Owner->Location;
		DirStruct Facing;
		switch(pTypeData->Spot_AttachedTo) {
			case TechnoTypeExt::sa_Barrel:
				Owner->BarrelFacing.GetFacing(&Facing);
				break;
			case TechnoTypeExt::sa_Turret:
				Owner->TurretFacing.GetFacing(&Facing);
				break;
			default:
				Owner->Facing.GetFacing(&Facing);
		}

		double Angle = Facing2Rad * Facing.Value;
		Loc.Y -= static_cast<int>(pTypeData->Spot_Distance * Math::cos(Angle));
		Loc.X += static_cast<int>(pTypeData->Spot_Distance * Math::sin(Angle));

		BL->field_B8 = Loc;
		BL->field_C4 = Loc;
//		double zer0 = 0.0;
		__asm { fldz }
	} else {
		double Angle = RulesClass::Instance->SpotlightAngle;
		__asm { fld Angle }
	}

	return R->AL()
		? 0x436461
		: 0x4364C8
	;
}

DEFINE_HOOK(435BE0, BuildingLightClass_Draw_Start, 6)
{
	GET(BuildingLightClass *, BL, ECX);
	TechnoExt::ActiveBuildingLight = BL;
	return 0;
}

DEFINE_HOOK(436072, BuildingLightClass_Draw_430, 6)
{
	TechnoClass *Owner = TechnoExt::ActiveBuildingLight->OwnerObject;
	TechnoTypeExt::ExtData *pTypeData = TechnoTypeExt::ExtMap.Find(Owner->GetTechnoType());
	R->EAX((pTypeData ? pTypeData->Spot_Height : 250) + 180);
	return 0x436078;
}

DEFINE_HOOK(4360D8, BuildingLightClass_Draw_400, 6)
{
	TechnoClass *Owner = TechnoExt::ActiveBuildingLight->OwnerObject;
	TechnoTypeExt::ExtData *pTypeData = TechnoTypeExt::ExtMap.Find(Owner->GetTechnoType());
	R->ECX((pTypeData ? pTypeData->Spot_Height : 250) + 150);
	return 0x4360DE;
}

DEFINE_HOOK(4360FF, BuildingLightClass_Draw_250, 6)
{
	TechnoClass *Owner = TechnoExt::ActiveBuildingLight->OwnerObject;
	TechnoTypeExt::ExtData *pTypeData = TechnoTypeExt::ExtMap.Find(Owner->GetTechnoType());
	R->ECX(pTypeData ? pTypeData->Spot_Height : 250);
	TechnoExt::ActiveBuildingLight = nullptr;
	return 0x436105;
}

DEFINE_HOOK(435CD3, SpotlightClass_CTOR, 6)
{
	GET_STACK(SpotlightClass *, Spot, 0x14);
	GET(BuildingLightClass *, BL, ESI);

	TechnoClass *Owner = BL->OwnerObject;
	TechnoTypeExt::ExtData *pTypeData = TechnoTypeExt::ExtMap.Find(Owner->GetTechnoType());

	eSpotlightFlags Flags = 0;
	if(pTypeData->Spot_Reverse) {
		Flags |= sf_NoColor;
	}
	if(pTypeData->Spot_DisableR) {
		Flags |= sf_NoRed;
	}
	if(pTypeData->Spot_DisableG) {
		Flags |= sf_NoGreen;
	}
	if(pTypeData->Spot_DisableB) {
		Flags |= sf_NoBlue;
	}

	Spot->DisableFlags = Flags;

	return 0;
}
