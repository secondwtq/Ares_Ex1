#include "Body.h"
#include "../../Misc/SWTypes.h"
#include "../House/Body.h"

#include <StringTable.h>
#include <VoxClass.h>

DEFINE_HOOK(6CEF84, SuperWeaponTypeClass_GetCursorOverObject, 7)
{
	GET(SuperWeaponTypeClass*, pThis, ECX);

	SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pThis);
	auto pType = pData->GetNewSWType();

	if((pThis->Action == SW_YES_CURSOR) || pType) {
		GET_STACK(CellStruct *, pMapCoords, 0x0C);

		int Action = SW_YES_CURSOR;

		// prevent firing into shroud
		if(!pData->SW_FireToShroud) {
			CellClass* pCell = MapClass::Instance->GetCellAt(*pMapCoords);
			CoordStruct Crd = pCell->GetCoords();

			if(MapClass::Instance->IsLocationShrouded(Crd)) {
				Action = SW_NO_CURSOR;
			}
		}

		// new SW types have to check whether the coordinates are valid.
		if(Action == SW_YES_CURSOR) {
			if(pType && !pType->CanFireAt(pData, HouseClass::Player, *pMapCoords)) {
				Action = SW_NO_CURSOR;
			}
		}

		R->EAX(Action);

		if(Action == SW_YES_CURSOR) {
			SWTypeExt::CurrentSWType = pThis;
			Actions::Set(&pData->SW_Cursor, pData->SW_FireToShroud);
		} else {
			SWTypeExt::CurrentSWType = nullptr;
			Actions::Set(&pData->SW_NoCursor, pData->SW_FireToShroud);
		}
		return 0x6CEFD9;
	}
	return 0;
}


DEFINE_HOOK(653B3A, RadarClass_GetMouseAction_CustomSWAction, 5)
{
	int idxSWType = Unsorted::CurrentSWType;
	if(idxSWType > -1) {
		GET_STACK(byte, EventFlags, 0x58);

		MouseEvent::Value E(EventFlags);
		if(E & (MouseEvent::RightDown | MouseEvent::RightUp)) {
			return 0x653D6F;
		}

		SuperWeaponTypeClass *pThis = SuperWeaponTypeClass::Array->GetItem(idxSWType);
		SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pThis);
		auto pType = pData->GetNewSWType();

		if((pThis->Action == SW_YES_CURSOR) || pType) {
			GET_STACK(CellStruct, MapCoords, STACK_OFFS(0x54, 0x3C));

			int Action = SW_YES_CURSOR;

			// prevent firing into shroud
			if(!pData->SW_FireToShroud) {
				CellClass* pCell = MapClass::Instance->GetCellAt(MapCoords);
				CoordStruct Crd = pCell->GetCoords();

				if(MapClass::Instance->IsLocationShrouded(Crd)) {
					Action = SW_NO_CURSOR;
				}
			}

			// new SW types have to check whether the coordinates are valid.
			if(Action == SW_YES_CURSOR) {
				if(pType && !pType->CanFireAt(pData, HouseClass::Player, MapCoords)) {
					Action = SW_NO_CURSOR;
				}
			}

			R->ESI(Action);

			if(Action == SW_YES_CURSOR) {
				SWTypeExt::CurrentSWType = pThis;
				Actions::Set(&pData->SW_Cursor, pData->SW_FireToShroud);
			} else {
				SWTypeExt::CurrentSWType = nullptr;
				Actions::Set(&pData->SW_NoCursor, pData->SW_FireToShroud);
			}
			return 0x653CA3;
		}
	}
	return 0;
}

DEFINE_HOOK(6AAEDF, SidebarClass_ProcessCameoClick_SuperWeapons, 6) {
	GET(int, idxSW, ESI);
	SuperClass* pSuper = HouseClass::Player->Supers.GetItem(idxSW);

	if(SWTypeExt::ExtData* pData = SWTypeExt::ExtMap.Find(pSuper->Type)) {
		// if this SW is only auto-firable, discard any clicks.
		// if AutoFire is off, the sw would not be firable at all,
		// thus we ignore the setting in that case.
		bool manual = !pData->SW_ManualFire && pData->SW_AutoFire;
		bool unstoppable = pSuper->Type->UseChargeDrain && pSuper->ChargeDrainState == ChargeDrainState::Draining
			&& pData->SW_Unstoppable;

		// play impatient voice, if this isn't charged yet
		if(!pSuper->CanFire() && !manual) {
			VoxClass::PlayIndex(pData->EVA_Impatient);
			return 0x6AAFB1;
		}

		// prevent firing the SW if the player doesn't have sufficient
		// funds. play an EVA message in that case.
		if(pData->Money_Amount < 0) {
			if(HouseClass::Player->Available_Money() < -pData->Money_Amount) {
				VoxClass::PlayIndex(pData->EVA_InsufficientFunds);
				pData->PrintMessage(pData->Message_InsufficientFunds, HouseClass::Player);
				return 0x6AAFB1;
			}
		}
		
		// disallow manuals and active unstoppables
		if(manual || unstoppable) {
			return 0x6AAFB1;
		}

		return 0x6AAEF7;
	}

	return 0;
}

// play a customizable target selection EVA message
DEFINE_HOOK(6AAF9D, SidebarClass_ProcessCameoClick_SelectTarget, 5)
{
	GET(int, index, ESI);
	if(SuperClass* pSW = HouseClass::Player->Supers.GetItem(index)) {
		if(SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pSW->Type)) {
			VoxClass::PlayIndex(pData->EVA_SelectTarget);
		}
	}

	return 0x6AB95A;
}

DEFINE_HOOK(6A932B, CameoClass_GetTip_MoneySW, 6) {
	GET(SuperWeaponTypeClass*, pSW, EAX);

	if(SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pSW)) {
		if(pData->Money_Amount < 0) {
			wchar_t* pTip = SidebarClass::TooltipBuffer;
			int length = SidebarClass::TooltipLength;

			// account for no-name SWs
			if(*(byte*)0x884B8C || !wcslen(pSW->UIName)) {
				const wchar_t* pFormat = StringTable::LoadStringA("TXT_MONEY_FORMAT_1");
				swprintf(pTip, length, pFormat, -pData->Money_Amount);
			} else {
				// then, this must be brand SWs
				const wchar_t* pFormat = StringTable::LoadStringA("TXT_MONEY_FORMAT_2");
				swprintf(pTip, length, pFormat, pSW->UIName, -pData->Money_Amount);
			}
			pTip[length - 1] = 0;

			// replace space by new line
			for(int i=wcslen(pTip); i>=0; --i) {
				if(pTip[i] == 0x20) {
					pTip[i] = 0xA;
					break;
				}
			}

			// put it there
			R->EAX(pTip);
			return 0x6A93E5;
		}
	}

	return 0;
}

// 6CEE96, 5
DEFINE_HOOK(6CEE96, SuperWeaponTypeClass_GetTypeIndex, 5)
{
	GET(const char *, TypeStr, EDI);
	int customType = NewSWType::FindIndex(TypeStr);
	if(customType > -1) {
		R->ESI(customType);
		return 0x6CEE9C;
	}
	return 0;
}

// 4AC20C, 7
// translates SW click to type
DEFINE_HOOK(4AC20C, DisplayClass_LMBUp, 7)
{
	int Action = R->Stack32(0x9C);
	if(Action < SW_NO_CURSOR) {
		// get the actual firing SW type instead of just the first type of the
		// requested action. this allows clones to work for legacy SWs (the new
		// ones use SW_*_CURSORs). we have to check that the action matches the
		// action of the found type as the no-cursor represents a different
		// action and we don't want to start a force shield even tough the UI
		// says no.
		auto pSW = SuperWeaponTypeClass::Array->GetItemOrDefault(Unsorted::CurrentSWType);
		if(pSW && (pSW->Action != Action)) {
			pSW = nullptr;
		}

		R->EAX(pSW);
		return pSW ? 0x4AC21C : 0x4AC294;
	}
	else if(Action == SW_NO_CURSOR) {
		R->EAX(0);
		return 0x4AC294;
	}

	R->EAX(SWTypeExt::CurrentSWType);
	return 0x4AC21C;
}

// decoupling sw anims from types
// 446418, 6
DEFINE_HOOK(446418, BuildingClass_Place1, 6)
{
	GET(BuildingClass *, pBuild, EBP);
	GET(HouseClass *, pHouse, EAX);
	int swTIdx = pBuild->Type->SuperWeapon;
	if(swTIdx == -1) {
		swTIdx = pBuild->Type->SuperWeapon2;
		if(swTIdx == -1) {
			return 0x446580;
		}
	}

	R->EAX(pHouse->Supers.GetItem(swTIdx));
	return 0x44643E;
}

// 44656D, 6
DEFINE_HOOK(44656D, BuildingClass_Place2, 6)
{
	return 0x446580;
}

// 45100A, 6
DEFINE_HOOK(45100A, BuildingClass_ProcessAnims1, 6)
{
	GET(BuildingClass *, pBuild, ESI);
	GET(HouseClass *, pHouse, EAX);
	int swTIdx = pBuild->Type->SuperWeapon;
	if(swTIdx == -1) {
		swTIdx = pBuild->Type->SuperWeapon2;
		if(swTIdx == -1) {
			return 0x451145;
		}
	}

	R->EDI(pBuild->Type);
	R->EAX(pHouse->Supers.GetItem(swTIdx));
	return 0x451030;
}

// 451132, 6
DEFINE_HOOK(451132, BuildingClass_ProcessAnims2, 6)
{
	return 0x451145;
}

// EVA_Detected
// 446937, 6
DEFINE_HOOK(446937, BuildingClass_AnnounceSW, 6)
{
	GET(BuildingClass *, pBuild, EBP);
	int swTIdx = pBuild->Type->SuperWeapon;
	if(swTIdx == -1) {
		swTIdx = pBuild->Type->SuperWeapon2;
		if(swTIdx == -1) {
			return 0x44699A;
		}
	}

	SuperWeaponTypeClass *pSW = SuperWeaponTypeClass::Array->GetItem(swTIdx);
	SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pSW);

	pData->PrintMessage(pData->Message_Detected, pBuild->Owner);

	if(pData->EVA_Detected != -1 || pData->IsTypeRedirected()) {
		if(pData->EVA_Detected != -1) {
			VoxClass::PlayIndex(pData->EVA_Detected);
		}
		return 0x44699A;
	}
	return 0;
}

// EVA_Ready
// 6CBDD7, 6
DEFINE_HOOK(6CBDD7, SuperClass_AnnounceReady, 6)
{
	GET(SuperWeaponTypeClass *, pThis, EAX);
	SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pThis);

	pData->PrintMessage(pData->Message_Ready, HouseClass::Player);

	if(pData->EVA_Ready != -1 || pData->IsTypeRedirected()) {
		if(pData->EVA_Ready != -1) {
			VoxClass::PlayIndex(pData->EVA_Ready);
		}
		return 0x6CBE68;
	}
	return 0;
}

// 6CC0EA, 9
DEFINE_HOOK(6CC0EA, SuperClass_AnnounceQuantity, 9)
{
	GET(SuperClass *, pThis, ESI);
	SuperWeaponTypeClass *pSW = pThis->Type;
	SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pSW);

	pData->PrintMessage(pData->Message_Ready, HouseClass::Player);

	if(pData->EVA_Ready != -1 || pData->IsTypeRedirected()) {
		if(pData->EVA_Ready != -1) {
			VoxClass::PlayIndex(pData->EVA_Ready);
		}
		return 0x6CC17E;
	}
	return 0;
}

// AI SW targeting submarines
DEFINE_HOOK(50CFAA, HouseClass_PickOffensiveSWTarget, 0)
{
	// reset weight
	R->ESI(0);

	// mark as ineligible
	R->Stack8(0x13, 0);

	return 0x50CFC9;
}

// psydom and lightning storm premature exit route
DEFINE_HOOK(6CBA9E, SuperClass_ClickFire_Abort, 7)
{
	GET(SuperClass *, pSuper, ESI);
	SuperWeaponTypeClass* pType = pSuper->Type;
	SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pType);

	GET_STACK(bool, IsPlayer, 0x20);

	// auto-abort if no money
	if(pData->Money_Amount < 0) {
		if(pSuper->Owner->Available_Money() < -pData->Money_Amount) {
			if(pSuper->Owner == HouseClass::Player) {
				VoxClass::PlayIndex(pData->EVA_InsufficientFunds);
				pData->PrintMessage(pData->Message_InsufficientFunds, pSuper->Owner);
			}
			return 0x6CBABF;
		}
	}

	// can this super weapon fire now?
	if(NewSWType* pNSW = pData->GetNewSWType()) {
		if(pNSW->AbortFire(pSuper, IsPlayer)) {
			return 0x6CBABF;
		}
	}

	return 0;
}

DEFINE_HOOK(6CBB0D, SuperClass_ClickFire_ResetAfterLaunch, 6)
{
	GET(SuperClass*, pSW, ESI);
	GET(SuperWeaponTypeClass*, pType, EAX);

	// do as the original game set it, but do not reset
	// the ready state for PreClick SWs neither. they will
	// be reset after the PostClick SW fired.
	if(pType && !pType->PostClick && !pType->PreClick) {
		pSW->IsCharged = false;
	}

	return 0x6CBB18;
}

// ARGH!
DEFINE_HOOK(6CC390, SuperClass_Launch, 6)
{
	GET(SuperClass *, pSuper, ECX);
	GET_STACK(CellStruct*, pCoords, 0x4);
	GET_STACK(bool, IsPlayer, 0x8);

	SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pSuper->Type);

	Debug::Log("[LAUNCH] %s\n", pSuper->Type->ID);

	bool handled = false;
	if(NewSWType* pNSW = pData->GetNewSWType()) {
		handled = SWTypeExt::Launch(pSuper, pNSW, *pCoords, IsPlayer);
	}

	return handled ? 0x6CDE40 : 0;
}

DEFINE_HOOK(44691B, BuildingClass_4DC_SWAvailable, 6)
{
	GET(BuildingClass *, Structure, EBP);
	GET(BuildingTypeClass *, AuxBuilding, EAX);
	return Structure->Owner->CountOwnedAndPresent(AuxBuilding) > 0
		? 0x446937
		: 0x44699A
	;
}

DEFINE_HOOK(45765A, BuildingClass_SWAvailable, 6)
{
	GET(BuildingClass *, Structure, ESI);
	GET(BuildingTypeClass *, AuxBuilding, EAX);
	return Structure->Owner->CountOwnedAndPresent(AuxBuilding) > 0
		? 0x45767B
		: 0x457676
	;
}

DEFINE_HOOK(4576BA, BuildingClass_SW2Available, 6)
{
	GET(BuildingClass *, Structure, ESI);
	GET(BuildingTypeClass *, AuxBuilding, EAX);
	return Structure->Owner->CountOwnedAndPresent(AuxBuilding) > 0
		? 0x4576DB
		: 0x4576D6
	;
}

DEFINE_HOOK(4FAE72, HouseClass_SWFire_PreDependent, 6)
{
	GET(HouseClass*, pThis, EBX);

	// find the predependent SW. decouple this from the chronosphere.
	// don't use a fixed SW type but the very one acutually fired last.
	SuperClass* pSource = nullptr;
	if(HouseExt::ExtData *pExt = HouseExt::ExtMap.Find(pThis)) {
		pSource = pThis->Supers.GetItemOrDefault(pExt->SWLastIndex);
	}

	R->ESI(pSource);

	return 0x4FAE7B;
}

DEFINE_HOOK(6CC2B0, SuperClass_NameReadiness, 5) {
	GET(SuperClass*, pThis, ECX);
	SuperWeaponTypeClass *pSW = pThis->Type;
	SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pSW);

	// complete rewrite of this method.

	CSFText* text = &pData->Text_Preparing;
	if(pThis->IsOnHold) {
		// on hold
		text = &pData->Text_Hold;
	} else {
		if(pThis->Type->UseChargeDrain) {
			switch(pThis->ChargeDrainState) {
			case ChargeDrainState::Charging:
				// still charging
				text = &pData->Text_Charging;
				break;
			case ChargeDrainState::Ready:
				// ready
				text = &pData->Text_Ready;
				break;
			case ChargeDrainState::Draining:
				// currently active
				text = &pData->Text_Active;
				break;
			}

		} else {
			// ready
			if(pThis->IsCharged) {
				text = &pData->Text_Ready;
			}
		}
	}

	R->EAX(text->empty() ? nullptr : text->Text);
	return 0x6CC352;
}

// #896002: darken SW cameo if player can't afford it
DEFINE_HOOK(6A99B7, TabCameoListClass_Draw_SuperDarken, 5)
{
	GET(int, idxSW, EDI);

	auto pSW = HouseClass::Player->Supers.GetItem(idxSW);
	auto pExt = SWTypeExt::ExtMap.Find(pSW->Type);

	bool darken = false;
	if(pSW->IsCharged && pExt->Money_Amount < 0) {
		if(pSW->Owner->Available_Money() < -pExt->Money_Amount) {
			darken = true;
		}
	}

	R->BL(darken);
	return 0;
}

DEFINE_HOOK(5098F0, HouseClass_Update_AI_TryFireSW, 5) {
	GET(HouseClass*, pThis, ECX);

	// this method iterates over every available SW and checks
	// whether it should be fired automatically. the original
	// method would abort if this house is human-controlled.
	bool AIFire = !pThis->ControlledByHuman();

	for(int i=0; i<pThis->Supers.Count; ++i) {
		if(SuperClass* pSuper = pThis->Supers.GetItem(i)) {
			SWTypeExt::ExtData* pExt = SWTypeExt::ExtMap.Find(pSuper->Type);

			// fire if this is AI owned or the SW has auto fire set.
			if(AIFire || pExt->SW_AutoFire) {

				if(pSuper->IsCharged) {
					CellStruct Cell = CellStruct::Empty;
					auto LaunchSW = [&](const CellStruct &Cell) {
						int idxSW = pThis->Supers.FindItemIndex(pSuper);
						pThis->Fire_SW(idxSW, Cell);
					};

					// don't try to fire if we obviously haven't enough money
					if(pExt->Money_Amount < 0) {
						if(pThis->Available_Money() < -pExt->Money_Amount) {
							continue;
						}
					}

					// all the different AI targeting modes
					switch(pExt->SW_AITargetingType) {
					case SuperWeaponAITargetingMode::Nuke:
						{
							if(pThis->EnemyHouseIndex != -1) {
								if(pThis->PreferredTargetCell == CellStruct::Empty) {
									Cell = *((pThis->PreferredTargetType == TargetType::Anything)
										? pThis->PickIonCannonTarget(Cell)
										: pThis->PickTargetByType(&Cell, pThis->PreferredTargetType));
								} else {
									Cell = pThis->PreferredTargetCell;
								}

								if(Cell != CellStruct::Empty) {
									LaunchSW(Cell);
								}
							}
							break;
						}

					case SuperWeaponAITargetingMode::LightningStorm:
						{
							pThis->Fire_LightningStorm(pSuper);
							break;
						}

					case SuperWeaponAITargetingMode::PsychicDominator:
						{
							pThis->Fire_PsyDom(pSuper);
							break;
						}

					case SuperWeaponAITargetingMode::ParaDrop:
						{
							pThis->Fire_ParaDrop(pSuper);
							break;
						}

					case SuperWeaponAITargetingMode::GeneticMutator:
						{
							pThis->Fire_GenMutator(pSuper);
							break;
						}

					case SuperWeaponAITargetingMode::ForceShield:
						{
							if(pThis->PreferredDefensiveCell2 == CellStruct::Empty) {
								if(pThis->PreferredDefensiveCell != CellStruct::Empty
									&& RulesClass::Instance->AISuperDefenseFrames + pThis->PreferredDefensiveCellStartTime > Unsorted::CurrentFrame) {
									Cell = pThis->PreferredDefensiveCell;
								}
							} else {
								Cell = pThis->PreferredDefensiveCell2;
							}

							if(Cell != CellStruct::Empty) {
								LaunchSW(Cell);
								pThis->PreferredDefensiveCell = CellStruct::Empty;
							}
							break;
						}

					case SuperWeaponAITargetingMode::Offensive:
						{
							if(pThis->EnemyHouseIndex != -1 && pExt->IsHouseAffected(pThis, HouseClass::Array->GetItem(pThis->EnemyHouseIndex))) {
								pThis->PickIonCannonTarget(Cell);
								if(Cell != CellStruct::Empty) {
									LaunchSW(Cell);
								}
							}
							break;
						}

					case SuperWeaponAITargetingMode::NoTarget:
						{
							LaunchSW(Cell);
							break;
						}

					case SuperWeaponAITargetingMode::Stealth:
						{
							// find one of the cloaked enemy technos, posing the largest threat.
							DynamicVectorClass<TechnoClass*> list;
							int currentValue = 0;
							for(int j=0; j<TechnoClass::Array->Count; ++j) {
								if(TechnoClass* pTechno = TechnoClass::Array->GetItem(j)) {
									if(pTechno->CloakState) {
										if(pExt->IsHouseAffected(pThis, pTechno->Owner)) {
											if(pExt->IsTechnoAffected(pTechno)) {
												int thisValue = pTechno->GetTechnoType()->ThreatPosed;
												if(currentValue < thisValue) {
													list.Clear();
													currentValue = thisValue;
												}
												if(currentValue == thisValue) {
													list.AddItem(pTechno);
												}
											}
										}
									}
								}
							}
							if(list.Count) {
								int rnd = ScenarioClass::Instance->Random.RandomRanged(0, list.Count - 1);
								Cell = list.GetItem(rnd)->GetCell()->MapCoords;
								LaunchSW(Cell);
							}
							break;
						}

					case SuperWeaponAITargetingMode::Base:
						{
							// fire at the SW's owner's base cell
							Cell = pThis->Base_Center();
							LaunchSW(Cell);
							break;
						}

					case SuperWeaponAITargetingMode::Self:
						{
							// find the first building providing pSuper
							SuperWeaponTypeClass *pType = pSuper->Type;
							BuildingClass *pBld = nullptr;
							for(int j=0; j<BuildingTypeClass::Array->Count; ++j) {
								BuildingTypeClass *pTBld = BuildingTypeClass::Array->GetItem(j);
								if((pTBld->SuperWeapon == pType->ArrayIndex) || (pTBld->SuperWeapon2 == pType->ArrayIndex)) {
									if((pBld = pThis->FindBuildingOfType(pTBld->ArrayIndex, -1)) != nullptr) {
										break;
									}
								}
							}
							if(pBld) {
								Cell = pBld->GetCell()->MapCoords;
								LaunchSW(Cell);
							}

							break;
						}
					}
				}
			}
		}
	}

	return 0x509AE7;
}

DEFINE_HOOK(4F9004, HouseClass_Update_TrySWFire, 7) {
	GET(HouseClass*, pThis, ESI);
	bool isHuman = R->AL() != 0;

	if(isHuman) {
		// update the SWs for human players to support auto firing.
		pThis->AI_TryFireSW();
	} else if(!pThis->Type->MultiplayPassive) {
		return 0x4F9015;
	}

	return 0x4F9038;
}

DEFINE_HOOK(6CBF5B, SuperClass_GetCameoChargeState_ChargeDrainRatio, 9) {
	GET_STACK(int, rechargeTime1, 0x10);
	GET_STACK(int, rechargeTime2, 0x14);
	GET_STACK(int, timeLeft, 0xC);
	
	GET(SuperWeaponTypeClass*, pType, EBX);
	if(SWTypeExt::ExtData* pData = SWTypeExt::ExtMap.Find(pType)) {

		// use per-SW charge-to-drain ratio.
		double percentage = 0.0;
		double ratio = pData->GetChargeToDrainRatio();
		if(std::abs(rechargeTime2 * ratio) > 0.001) {
			percentage = 1.0 - (rechargeTime1 * ratio - timeLeft) / (rechargeTime2 * ratio);
		}

		// up to 55 steps
		int charge = Game::F2I(percentage * 54.0);
		R->EAX(charge);
		return 0x6CC053;
	}

	return 0;
}

DEFINE_HOOK(6CC053, SuperClass_GetCameoChargeState_FixFullyCharged, 5) {
	GET(int, charge, EAX);

	// some smartass capped this at 53, causing the last
	// wedge of darken.shp never to disappear.
	R->EAX(std::min(charge, 54));
	return 0x6CC066;
}

DEFINE_HOOK(6CB995, SuperClass_ClickFire_ChargeDrainRatioA, 8) {
	GET_STACK(int, rechargeTime, 0x24);
	GET_STACK(int, timeLeft, 0x20);

	// recreate the SW from a pointer to its CreationTimer
	GET(SuperClass*, pSuper, ESI);
	pSuper = (SuperClass*)((char*)pSuper - 30);

	if(SWTypeExt::ExtData* pData = SWTypeExt::ExtMap.Find(pSuper->Type)) {
		double ratio = pData->GetChargeToDrainRatio();
		double remaining = rechargeTime - timeLeft / ratio;
		int frames = Game::F2I(remaining);
	
		R->EAX(frames);
		return 0x6CB9B0;
	}

	return 0;
}

DEFINE_HOOK(6CBA19, SuperClass_ClickFire_ChargeDrainRatioB, A) {
	GET(int, length, EDI);
	GET(SuperClass*, pSuper, ESI);

	if(SWTypeExt::ExtData* pData = SWTypeExt::ExtMap.Find(pSuper->Type)) {
		double ratio = pData->GetChargeToDrainRatio();
		double remaining = length * ratio;
		int frames = Game::F2I(remaining);
	
		R->EAX(frames);
		return 0x6CBA28;
	}

	return 0;
}

DEFINE_HOOK(6CBD6B, SuperClass_Update_DrainMoney, 8) {
	// draining weapon active. take or give money. stop, 
	// if player has insufficient funds.
	GET(SuperClass*, pSuper, ESI);
	GET(int, timeLeft, EAX);

	if(timeLeft > 0 && pSuper->Type->UseChargeDrain && pSuper->ChargeDrainState == ChargeDrainState::Draining) {
		if(SWTypeExt::ExtData* pData = SWTypeExt::ExtMap.Find(pSuper->Type)) {
			int money = pData->Money_DrainAmount;
			if(money != 0 && pData->Money_DrainDelay > 0) {
				if(!(timeLeft % pData->Money_DrainDelay)) {

					// only abort if SW drains money and there is none
					if(pData->Money_DrainAmount < 0) {
						if(pSuper->Owner->Available_Money() < -money) {
							if(pSuper->Owner->ControlledByHuman()) {
								VoxClass::PlayIndex(pData->EVA_InsufficientFunds);
								pData->PrintMessage(pData->Message_InsufficientFunds, HouseClass::Player);
							}
							return 0x6CBD73;
						}
					}

					// apply drain money
					if(money > 0) {
						pSuper->Owner->GiveMoney(money);
					} else {
						pSuper->Owner->TakeMoney(-money);
					}
				}
			}
		}
	}

	return (timeLeft ? 0x6CBE7C : 0x6CBD73);
}

// used only to find the nuke for ICBM crates. only supports nukes fully.
DEFINE_HOOK(6CEEB0, SuperWeaponTypeClass_FindFirstOfAction, 8) {
	GET(int, action, ECX);

	R->EAX(0);

	// this implementation is as stupid as short sighted, but it should work
	// for the moment. as there are no actions any more, this has to be
	// reworked if powerups are expanded. for now, it only has to find a nuke.
	for(int i=0; i<SuperWeaponTypeClass::Array->Count; ++i) {
		if(SuperWeaponTypeClass* pType = SuperWeaponTypeClass::Array->GetItem(i)) {
			if(pType->Action == action) {
				R->EAX(pType);
				break;
			} else {
				if(SWTypeExt::ExtData* pExt = SWTypeExt::ExtMap.Find(pType)) {
					if(NewSWType *pNewSWType = pExt->GetNewSWType()) {
						if(pNewSWType->HandlesType(SuperWeaponType::Nuke)) {
							R->EAX(pType);
							break;
						}
					}
				}
			}
		}
	}

	// put a hint into the debug log to explain why we will crash now.
	if(!R->EAX()) {
		Debug::FatalErrorAndExit("Failed finding an Action=Nuke or Type=MultiMissile super weapon to be granted by ICBM crate.");
	}

	return 0x6CEEE5;
}
