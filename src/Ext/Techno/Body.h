#ifndef TECHNO_EXT_H
#define TECHNO_EXT_H

#include <xcompile.h>
#include <algorithm>

#include "../../Misc/AttachEffect.h"
#include "../../Misc/JammerClass.h"
#include "../../Misc/PoweredUnitClass.h"

#include "../../Utilities/Constructs.h"
#include "../../Utilities/Enums.h"

#include "../_Container.hpp"

class AircraftClass;
class AlphaShapeClass;
class BuildingLightClass;
class EBolt;
class HouseClass;
class HouseTypeClass;
class InfantryClass;
struct SHPStruct;
class TemporalClass;
class WarheadTypeClass;

class TechnoExt
{
public:
	typedef TechnoClass TT;

	class ExtData : public Extension<TT>
	{
	public:
		// weapon slots fsblargh
		BYTE idxSlot_Wave;
		BYTE idxSlot_Beam;
		BYTE idxSlot_Warp;
		BYTE idxSlot_Parasite;

		bool Survivors_Done;

		TimerStruct CloakSkipTimer;
		SHPStruct * Insignia_Image;

		BuildingClass *GarrisonedIn; // when infantry garrisons a building, we need a fast way to find said building when damage forwarding kills it

		AnimClass *EMPSparkleAnim;
		eMission EMPLastMission;

		bool ShadowDrawnManually;

		bool DriverKilled;

		int HijackerHealth;
		HouseClass* HijackerHouse;

		// 305 Radar Jammers
		std::unique_ptr<JammerClass> RadarJam;
		
		// issue #617 powered units
		std::unique_ptr<PoweredUnitClass> PoweredUnit;

		//#1573, #1623, #255 Stat-modifiers/ongoing animations
		std::vector<std::unique_ptr<AttachEffectClass>> AttachedEffects;
		bool AttachEffects_RecreateAnims;

		//stuff for #1623
		bool AttachedTechnoEffect_isset;
		int AttachedTechnoEffect_Delay;

		//crate fields
		double Crate_FirepowerMultiplier;
		double Crate_ArmorMultiplier;
		double Crate_SpeedMultiplier;
		bool Crate_Cloakable;

		TemporalClass * MyOriginalTemporal;

		EBolt * MyBolt;

		HouseTypeClass* OriginalHouseType;

		BuildingLightClass* Spotlight;

		Nullable<bool> AltOccupation; // if the unit marks cell occupation flags, this is set to whether it uses the "high" occupation members

		SuperClass* HunterSeekerSW; // set if a hunter seeker SW created this

		//	Kyouma Hououin 140831PM
		//		for custom speed on ore
		double SpeedMultiplier_OnOre;
		double Ex1_SpeedMultiplier;

		ExtData(TT* const OwnerObject) : Extension<TT>(OwnerObject),
			idxSlot_Wave (0),
			idxSlot_Beam (0),
			idxSlot_Warp (0),
			idxSlot_Parasite(0),
			Survivors_Done (0),
			Insignia_Image (nullptr),
			GarrisonedIn (nullptr),
			HijackerHealth (-1),
			HijackerHouse (nullptr),
			DriverKilled (false),
			EMPSparkleAnim (nullptr),
			EMPLastMission (mission_None),
			ShadowDrawnManually (false),
			RadarJam(nullptr),
			PoweredUnit(nullptr),
			MyOriginalTemporal(nullptr),
			MyBolt(nullptr),
			Spotlight(nullptr),
			AltOccupation(),
			HunterSeekerSW(nullptr),
			OriginalHouseType(nullptr),
			AttachEffects_RecreateAnims(false),
			AttachedTechnoEffect_isset (false),
			AttachedTechnoEffect_Delay (0),
			Crate_FirepowerMultiplier(1.0),
			Crate_ArmorMultiplier(1.0),
			Crate_SpeedMultiplier(1.0),
			Crate_Cloakable(false),
			SpeedMultiplier_OnOre(1.0),
			Ex1_SpeedMultiplier(1.0)
			{
				this->CloakSkipTimer.Stop();
			};

		virtual ~ExtData() {
		};

		// when any pointer in the game expires, this is called - be sure to tell everyone we own to invalidate it
		virtual void InvalidatePointer(void *ptr, bool bRemoved) {
			AnnounceInvalidPointer(this->GarrisonedIn, ptr);
			this->InvalidateAttachEffectPointer(ptr);
			AnnounceInvalidPointer(this->MyOriginalTemporal, ptr);
			AnnounceInvalidPointer(this->Spotlight, ptr);
		}

		bool IsOperated();
		bool IsPowered();

		AresAction::Value GetActionHijack(TechnoClass *pTarget);
		bool PerformActionHijack(TechnoClass* pTarget);

		unsigned int AlphaFrame(SHPStruct * Image);

		bool DrawVisualFX();

		UnitTypeClass * GetUnitType();

		bool IsDeactivated() const;

		eAction GetDeactivatedAction(ObjectClass *Hovered = nullptr) const;

		void InvalidateAttachEffectPointer(void *ptr);

		void RefineTiberium(float amount, int idxType);
		void DepositTiberium(float amount, float bonus, int idxType);

		bool IsCloakable(bool allowPassive) const;
		bool CloakAllowed() const;
		bool CloakDisallowed(bool allowPassive) const;
		bool CanSelfCloakNow() const;

		void SetSpotlight(BuildingLightClass* pSpotlight);

		bool AcquireHunterSeekerTarget() const;
	};

	static Container<TechnoExt> ExtMap;

	static AresMap<ObjectClass*, AlphaShapeClass*> AlphaExt;

	static BuildingLightClass * ActiveBuildingLight;

	static FireError::Value FiringStateCache;

	static bool NeedsRegap;

	static void SpawnSurvivors(FootClass *pThis, TechnoClass *pKiller, bool Select, bool IgnoreDefenses);
	static bool EjectSurvivor(FootClass *Survivor, CoordStruct loc, bool Select);
	static void EjectPassengers(FootClass *, int);
	static CoordStruct GetPutLocation(CoordStruct, int);
	static bool EjectRandomly(FootClass*, CoordStruct const &, int, bool);
	// If available, removes the hijacker from its victim and creates an InfantryClass instance.
	static InfantryClass* RecoverHijacker(FootClass *pThis);

	static void StopDraining(TechnoClass *Drainer, TechnoClass *Drainee);

	static bool CreateWithDroppod(FootClass *Object, const CoordStruct& XYZ);

	static void TransferIvanBomb(TechnoClass *From, TechnoClass *To);
	static void TransferAttachedEffects(TechnoClass *From, TechnoClass *To);

	static void RecalculateStats(TechnoClass *pTechno);

	static void FreeSpecificSlave(TechnoClass *Slave, HouseClass *Affector);
	static void DetachSpecificSpawnee (TechnoClass *Spawnee, HouseClass *NewSpawneeOwner);
	static bool CanICloakByDefault(TechnoClass *pTechno);

	static void Destroy(TechnoClass* pTechno, TechnoClass* pKiller = nullptr, HouseClass* pKillerHouse = nullptr, WarheadTypeClass* pWarhead = nullptr);

	static bool SpawnVisceroid(CoordStruct &crd, ObjectTypeClass* pType, int chance, bool ignoreTibDeathToVisc);
/*
	static int SelectWeaponAgainst(TechnoClass *pThis, TechnoClass *pTarget);
	static bool EvalWeaponAgainst(TechnoClass *pThis, TechnoClass *pTarget, WeaponTypeClass* W);
	static float EvalVersesAgainst(TechnoClass *pThis, TechnoClass *pTarget, WeaponTypeClass* W);
*/
};

#endif
