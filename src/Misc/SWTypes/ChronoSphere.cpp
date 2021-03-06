#include "ChronoSphere.h"
#include "../../Ares.h"

#include "../../Utilities/TemplateDef.h"

#include <VocClass.h>
#include <VoxClass.h>

bool SW_ChronoSphere::HandlesType(int type)
{
	return (type == SuperWeaponType::ChronoSphere);
}

SuperWeaponFlags::Value SW_ChronoSphere::Flags()
{
	return SuperWeaponFlags::NoAnim | SuperWeaponFlags::NoEVA | SuperWeaponFlags::NoMoney
		| SuperWeaponFlags::NoEvent | SuperWeaponFlags::NoCleanup | SuperWeaponFlags::NoMessage
		| SuperWeaponFlags::PreClick;
}

AnimTypeClass* SW_ChronoSphere::GetAnim(const SWTypeExt::ExtData* pData) const
{
	return pData->SW_Anim.Get(RulesClass::Instance->ChronoPlacement);
}

SWRange SW_ChronoSphere::GetRange(const SWTypeExt::ExtData* pData) const
{
	if(pData->SW_Range.empty()) {
		return SWRange(3, 3);
	}
	return pData->SW_Range;
}

void SW_ChronoSphere::Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW)
{
	pData->SW_AnimVisibility = SuperWeaponAffectedHouse::Team;
	pData->SW_AnimHeight = 5;

	pData->Chronosphere_KillOrganic = true;
	pData->Chronosphere_KillTeleporters = false;
	pData->Chronosphere_AffectIronCurtain = false;
	pData->Chronosphere_AffectUnwarpable = true;
	pData->Chronosphere_AffectUndeployable = false;
	pData->Chronosphere_AffectBuildings = false;
	pData->Chronosphere_BlowUnplaceable = true;
	pData->Chronosphere_ReconsiderBuildings = true;

	pData->EVA_Ready = VoxClass::FindIndex("EVA_ChronosphereReady");
	pData->EVA_Detected = VoxClass::FindIndex("EVA_ChronosphereDetected");
	pData->EVA_Activated = VoxClass::FindIndex("EVA_ChronosphereActivated");
	
	pData->SW_AffectsTarget = SuperWeaponTarget::Infantry | SuperWeaponTarget::Unit;
	pData->SW_Cursor = MouseCursor::First[MouseCursorType::Chronosphere];
}

void SW_ChronoSphere::LoadFromINI(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW, CCINIClass *pINI)
{
	const char * section = pSW->ID;

	if(!pINI->GetSection(section)) {
		return;
	}

	INI_EX exINI(pINI);

	pData->Chronosphere_KillOrganic.Read(exINI, section, "Chronosphere.KillOrganic");
	pData->Chronosphere_KillTeleporters.Read(exINI, section, "Chronosphere.KillTeleporters");
	pData->Chronosphere_AffectIronCurtain.Read(exINI, section, "Chronosphere.AffectsIronCurtain");
	pData->Chronosphere_AffectUnwarpable.Read(exINI, section, "Chronosphere.AffectsUnwarpable");
	pData->Chronosphere_AffectUndeployable.Read(exINI, section, "Chronosphere.AffectsUndeployable");
	pData->Chronosphere_BlowUnplaceable.Read(exINI, section, "Chronosphere.BlowUnplaceable");
	pData->Chronosphere_ReconsiderBuildings.Read(exINI, section, "Chronosphere.ReconsiderBuildings");

	pData->Chronosphere_BlastSrc.Read(exINI, section, "Chronosphere.BlastSrc");
	pData->Chronosphere_BlastDest.Read(exINI, section, "Chronosphere.BlastDest");

	// reconstruct the original value, then re-read (otherwise buildings will be affected if
	// the SW section is defined in game mode inis or maps without restating SW.AffectsTarget)
	if(!pData->Chronosphere_AffectBuildings) {
		pData->SW_AffectsTarget = (pData->SW_AffectsTarget & ~SuperWeaponTarget::Building);
	}
	pData->SW_AffectsTarget.Read(exINI, section, "SW.AffectsTarget");

	// we handle the distinction between buildings and deployed vehicles ourselves
	pData->Chronosphere_AffectBuildings = ((pData->SW_AffectsTarget & SuperWeaponTarget::Building) != 0);
	pData->SW_AffectsTarget = (pData->SW_AffectsTarget | SuperWeaponTarget::Building);
}

bool SW_ChronoSphere::Activate(SuperClass* pThis, const CellStruct &Coords, bool IsPlayer)
{
	SuperWeaponTypeClass *pSW = pThis->Type;
	SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pSW);

	if(pThis->IsCharged) {
		CellClass *pTarget = MapClass::Instance->GetCellAt(Coords);

		// remember the current source position
		pThis->ChronoMapCoords = Coords;

		// position to play the animation at
		CoordStruct coords = pTarget->GetCoords();
		if(pTarget->Flags & cf_Bridge) {
			coords.Z += pTarget->BridgeHeight();
		}
		coords.Z += pData->SW_AnimHeight;

		// recoded to support customizable anims
		// and visibility for allies, too.
		if(AnimTypeClass* pAnimType = GetAnim(pData)) {
			SWTypeExt::CreateChronoAnim(pThis, &coords, pAnimType);
		}

		if(IsPlayer) {
			// find the corresponding warp SW type.
			int idxWarp = -1;
			for(int i=0; i<SuperWeaponTypeClass::Array->Count; ++i) {
				SuperWeaponTypeClass* pWarp = SuperWeaponTypeClass::Array->GetItem(i);
				if(pWarp->Type == SuperWeaponType::ChronoWarp) {
					if(!_strcmpi(pData->SW_PostDependent, pWarp->ID)) {
						idxWarp = i;
						break;
					} else if(idxWarp == -1) {
						// fallback to use the first warp if there is no specific one
						idxWarp = i;
					}
				}
			}

			if(idxWarp == -1) {
				Debug::Log("[ChronoSphere::Launch] There is no SuperWeaponType of type ChronoWarp. Aborted.\n");
			}
			Unsorted::CurrentSWType = idxWarp;
		}
	}

	return true;
}
