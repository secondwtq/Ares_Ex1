#include <AnimClass.h>
#include <BulletClass.h>
#include <BuildingClass.h>
#include <CellClass.h>

#include "../../Misc/Applicators.h"

#include "Body.h"
#include "../BuildingType/Body.h"
#include "../Building/Body.h"
#include "../House/Body.h"
#include "../BulletType/Body.h"
#include "../TechnoType/Body.h"

DEFINE_HOOK_AGAIN(6FF860, TechnoClass_Fire_FSW, 8)
DEFINE_HOOK(6FF008, TechnoClass_Fire_FSW, 8)
{
	CoordStruct src = *R->lea_Stack<CoordStruct *>(0x44);
	CoordStruct tgt = *R->lea_Stack<CoordStruct *>(0x88);

	BulletClass * Bullet = R->get_Origin() == 0x6FF860
		? R->EDI<BulletClass *>()
		: R->EBX<BulletClass *>()
	;

	BulletTypeExt::ExtData *pBulletData = BulletTypeExt::ExtMap.Find(Bullet->Type);

	if(!pBulletData->SubjectToFirewall || !HouseExt::IsAnyFirestormActive) {
		return 0;
	}

//	check the path of the projectile to see if there are any firestormed cells along the way
//	if so, redirect the proj to the nearest one so it crashes
//	this is technically only necessary for invisible projectiles which don't move to the target
//	- the BulletClass::Update hook above wouldn't work for them

// screw having two code paths

	FirestormFinderApplicator FireFinder(Bullet->Owner->Owner);

	CellSequence Path(&src, &tgt);

	Path.Apply(FireFinder);

	if(FireFinder.found) {
		tgt = CellClass::Cell2Coord(FireFinder.target);
		Bullet->Target = MapClass::Instance->GetCellAt(tgt)->GetContent();
		Bullet->Owner->ShouldLoseTargetNow = 1;
//		Bullet->Owner->SetTarget(nullptr);
//		Bullet->Owner->Scatter(0xB1CFE8, 1, 0);
	}

	return 0;
}

DEFINE_HOOK(4F8440, HouseClass_Update_FSW_Recalc, 5)
{
	GET(HouseClass *, H, ECX);
	HouseExt::ExtData *pHouseData = HouseExt::ExtMap.Find(H);
	if(pHouseData->FirewallRecalc > 0) {
		--pHouseData->FirewallRecalc;
		pHouseData->SetFirestormState(pHouseData->FirewallActive);
	} else if(pHouseData->FirewallRecalc < 0) {
		pHouseData->FirewallRecalc = 0;
	}
	return 0;
}

DEFINE_HOOK(4F8C97, HouseClass_Update_FSW_LowPower, 6)
{
	GET(HouseClass *, H, ESI);
	HouseExt::ExtData *pHouseData = HouseExt::ExtMap.Find(H);
	pHouseData->SetFirestormState(0);

	return 0;
}

