#ifndef SUPERTYPE_EXT_H
#define SUPERTYPE_EXT_H

#include <CCINIClass.h>
#include <SuperWeaponTypeClass.h>

#include "../../Ares.CRT.h"
#include "../../Misc/Actions.h"
//#include "../../Misc/SWTypes.h"
#include "../../Utilities/Enums.h"
#include "../../Utilities/Template.h"
#include "../../Utilities/Constructs.h"

#ifdef DEBUGBUILD
#include "../../Misc/Debug.h"
#endif

// the index of the first custom sw type
#define FIRST_SW_TYPE 12

#include "../_Container.hpp"

class AircraftTypeClass;
class AnimClass;
class HouseClass;
class MissionClass;
class NewSWType;
class ParadropPlane;
class RadarEventClass;
class SuperClass;
class VocClass;
class VoxClass;

struct SWRange {
	SWRange(float widthOrRange = -1.0f, int height = -1) : WidthOrRange(widthOrRange), Height(height) {}
	SWRange(int widthOrRange, int height = -1) : WidthOrRange(static_cast<float>(widthOrRange)), Height(height) {}

	float range() const {
		return this->WidthOrRange;
	}

	int width() const {
		return static_cast<int>(this->WidthOrRange);
	}

	int height() const {
		return this->Height;
	}

	bool empty() const {
		return this->WidthOrRange < 0.0
			&& this->Height < 0;
	}

	float WidthOrRange;
	int Height;
};

struct LightingColor {
	int Red, Green, Blue, Ambient;
	bool HasValue;
};

class SWTypeExt
{
public:
	typedef SuperWeaponTypeClass TT;

	class ExtData : public Extension<TT>
	{
	public:
		// SpyPlane
		ValueableIdx<AircraftTypeClass> SpyPlane_TypeIndex;
		Valueable<int> SpyPlane_Count;
		ValueableIdx<MissionClass> SpyPlane_Mission;

		// Lightning Storm
		Nullable<int> Weather_Duration;
		Nullable<int> Weather_HitDelay;
		Nullable<int> Weather_ScatterDelay;
		Valueable<int> Weather_ScatterCount;
		Nullable<int> Weather_Separation;
		Valueable<int> Weather_CloudHeight;
		Nullable<int> Weather_RadarOutage;
		Valueable<int> Weather_DebrisMin;
		Valueable<int> Weather_DebrisMax;
		Nullable<bool> Weather_PrintText;
		Valueable<bool> Weather_IgnoreLightningRod;
		Nullable<AnimTypeClass*> Weather_BoltExplosion;
		NullableVector<AnimTypeClass*> Weather_Clouds;
		NullableVector<AnimTypeClass*> Weather_Bolts;
		NullableVector<AnimTypeClass*> Weather_Debris;
		NullableIdxVector<VocClass> Weather_Sounds;
		ValueableEnum<SuperWeaponAffectedHouse> Weather_RadarOutageAffects;

		// Nuke
		Valueable<WeaponTypeClass*> Nuke_Payload;
		Valueable<AnimTypeClass*> Nuke_PsiWarning;
		Nullable<AnimTypeClass*> Nuke_TakeOff;
		Valueable<bool> Nuke_SiloLaunch;

		// Generic Paradrop
		AresMap<AbstractTypeClass*, std::vector<ParadropPlane*>> ParaDrop;
		std::vector<std::unique_ptr<ParadropPlane>> ParaDropPlanes;

		// Generic Protection
		Nullable<int> Protect_Duration;
		Nullable<int> Protect_PlayFadeSoundTime;
		Nullable<int> Protect_PowerOutageDuration;
		Valueable<bool> Protect_IsForceShield;

		// Chronosphere
		Nullable<AnimTypeClass *> Chronosphere_BlastSrc;
		Nullable<AnimTypeClass *> Chronosphere_BlastDest;
		Valueable<bool> Chronosphere_KillOrganic;
		Valueable<bool> Chronosphere_KillTeleporters;
		Valueable<bool> Chronosphere_AffectUndeployable;
		Valueable<bool> Chronosphere_AffectBuildings;
		Valueable<bool> Chronosphere_AffectUnwarpable;
		Valueable<bool> Chronosphere_AffectIronCurtain;
		Valueable<bool> Chronosphere_BlowUnplaceable;
		Valueable<bool> Chronosphere_ReconsiderBuildings;

		// Genetic Mutator
		Nullable<bool> Mutate_Explosion;
		Valueable<bool> Mutate_IgnoreCyborg;
		Valueable<bool> Mutate_IgnoreNotHuman;
		Valueable<bool> Mutate_KillNatural;

		// Psychic Dominator
		Valueable<bool> Dominator_Capture;
		Nullable<int> Dominator_FireAtPercentage;
		Valueable<int> Dominator_FirstAnimHeight;
		Valueable<int> Dominator_SecondAnimHeight;
		Nullable<AnimTypeClass*> Dominator_FirstAnim;
		Nullable<AnimTypeClass*> Dominator_SecondAnim;
		Nullable<AnimTypeClass*> Dominator_ControlAnim;
		Valueable<bool> Dominator_Ripple;
		Valueable<bool> Dominator_CaptureMindControlled;
		Valueable<bool> Dominator_CapturePermaMindControlled;
		Valueable<bool> Dominator_CaptureImmuneToPsionics;
		Valueable<bool> Dominator_PermanentCapture;
		
		// Sonar
		Valueable<int> Sonar_Delay;

		// Hunter Seeker
		Valueable<UnitTypeClass*> HunterSeeker_Type;
		Valueable<bool> HunterSeeker_RandomOnly;
		ValueableVector<BuildingTypeClass*> HunterSeeker_Buildings;

		// Drop Pod
		Nullable<int> DropPod_Minimum;
		Nullable<int> DropPod_Maximum;
		Valueable<double> DropPod_Veterancy;
		ValueableVector<TechnoTypeClass*> DropPod_Types;

		// Money
		Valueable<int> Money_Amount;
		Valueable<int> Money_DrainAmount;
		Valueable<int> Money_DrainDelay;

		// Generic
		ValueableIdx<VoxClass> EVA_Ready;
		ValueableIdx<VoxClass> EVA_Activated;
		ValueableIdx<VoxClass> EVA_Detected;
		ValueableIdx<VoxClass> EVA_Impatient;
		ValueableIdx<VoxClass> EVA_InsufficientFunds;
		ValueableIdx<VoxClass> EVA_SelectTarget;

		// anim/sound
		NullableIdx<VocClass> SW_Sound;
		NullableIdx<VocClass> SW_ActivationSound;
		Nullable<AnimTypeClass *> SW_Anim;
		Valueable<int> SW_AnimHeight;
		ValueableEnum<SuperWeaponAffectedHouse> SW_AnimVisibility;

		Valueable<bool> SW_TypeCustom;
		Valueable<bool> SW_AutoFire;
		Valueable<bool> SW_ManualFire;
		Valueable<bool> SW_FireToShroud;
		Valueable<bool> SW_RadarEvent;
		Valueable<bool> SW_ShowCameo;
		Valueable<bool> SW_Unstoppable;
		Valueable<MouseCursor> SW_Cursor;
		Valueable<MouseCursor> SW_NoCursor;
		char SW_PostDependent[0x18];
		ValueableEnum<SuperWeaponAITargetingMode> SW_AITargetingType;
		Nullable<double> SW_ChargeToDrainRatio;

		SWRange SW_Range;
		ValueableEnum<SuperWeaponAffectedHouse> SW_AffectsHouse;
		ValueableEnum<SuperWeaponAffectedHouse> SW_RequiresHouse;
		ValueableEnum<SuperWeaponTarget> SW_AffectsTarget;
		ValueableEnum<SuperWeaponTarget> SW_RequiresTarget;
		Nullable<WarheadTypeClass *> SW_Warhead;
		Nullable<int> SW_Damage;
		Nullable<int> SW_Deferment;

		// Lighting
		Valueable<bool> Lighting_Enabled;
		Nullable<int> Lighting_Ambient;
		Nullable<int> Lighting_Green;
		Nullable<int> Lighting_Blue;
		Nullable<int> Lighting_Red;

		// Messages
		Valueable<CSFText> Message_Detected;
		Valueable<CSFText> Message_Ready;
		Valueable<CSFText> Message_Launch;
		Valueable<CSFText> Message_Activate;
		Valueable<CSFText> Message_Abort;
		Valueable<CSFText> Message_InsufficientFunds;
		Valueable<int> Message_ColorScheme;
		Valueable<bool> Message_FirerColor;

		// Texts
		Valueable<CSFText> Text_Preparing;
		Valueable<CSFText> Text_Hold;
		Valueable<CSFText> Text_Ready;
		Valueable<CSFText> Text_Charging;
		Valueable<CSFText> Text_Active;

		CustomPalette CameoPal;

		// Unit Delivery
		ValueableVector<TechnoTypeClass *> SW_Deliverables;
		Valueable<bool> SW_DeliverBuildups;

		AresPCXFile SidebarPCX;

		int HandledByNewSWType;
		int LastAction;

		ExtData(TT* const OwnerObject) : Extension<TT>(OwnerObject),
			SpyPlane_TypeIndex (0),
			SpyPlane_Count (1),
			SpyPlane_Mission (mission_AttackAgain),
			Weather_CloudHeight (-1),
			Weather_ScatterCount (1),
			Nuke_PsiWarning (nullptr),
			Sonar_Delay (0),
			HunterSeeker_Type (nullptr),
			HunterSeeker_RandomOnly (false),
			HunterSeeker_Buildings (),
			DropPod_Minimum (),
			DropPod_Maximum (),
			DropPod_Veterancy (2.0),
			DropPod_Types (),
			Money_Amount (0),
			Money_DrainAmount (0),
			Money_DrainDelay (0),
			EVA_Ready (-1),
			EVA_Activated (-1),
			EVA_Detected (-1),
			EVA_Impatient (-1),
			EVA_InsufficientFunds (-1),
			EVA_SelectTarget (-1),
			Message_Detected (),
			Message_Ready (),
			Message_Launch (),
			Message_Activate (),
			Message_Abort (),
			Message_InsufficientFunds (),
			Message_ColorScheme (-1),
			Message_FirerColor (false),
			Text_Preparing (),
			Text_Ready (),
			Text_Hold (),
			Text_Charging (),
			Text_Active (),
			Lighting_Enabled (true),
			SW_AnimHeight (0),
			SW_AnimVisibility (SuperWeaponAffectedHouse::All),
			SW_TypeCustom (false),
			SW_AutoFire (false),
			SW_ManualFire (true),
			SW_ShowCameo (true),
			SW_Unstoppable (false),
			SW_AffectsHouse (SuperWeaponAffectedHouse::All),
			SW_RequiresHouse (SuperWeaponAffectedHouse::None),
			SW_AffectsTarget (SuperWeaponTarget::All),
			SW_RequiresTarget (SuperWeaponTarget::None),
			SW_AITargetingType (SuperWeaponAITargetingMode::None),
			SW_FireToShroud (true),
			SW_RadarEvent (true),
			SW_Range(),
			HandledByNewSWType (-1),
			CameoPal(),
			SW_DeliverBuildups (false)
			{
				*SW_PostDependent = 0;
			};

		virtual ~ExtData() = default;

		virtual void LoadFromRulesFile(TT *pThis, CCINIClass *pINI);
		virtual void LoadFromINIFile(TT *pThis, CCINIClass *pINI);
		virtual void InitializeConstants(TT *pThis);
		virtual void InitializeRuled(TT *pThis);

		bool UpdateLightingColor(LightingColor& Lighting) const;

		bool IsAnimVisible(HouseClass* pFirer);
		bool CanFireAt(HouseClass* pOwner, const CellStruct &Coords);
		bool IsHouseAffected(HouseClass* pFirer, HouseClass* pHouse);
		bool IsHouseAffected(HouseClass* pFirer, HouseClass* pHouse, SuperWeaponAffectedHouse::Value value);
		bool IsTechnoAffected(TechnoClass* pTechno);
		void PrintMessage(const CSFText& message, HouseClass* pFirer);

		NewSWType* GetNewSWType() const;
		bool IsOriginalType() const;
		bool IsTypeRedirected() const;
		int GetTypeIndexWithRedirect() const;
		int GetNewTypeIndex() const;

		WarheadTypeClass* GetWarhead() const;
		AnimTypeClass* GetAnim() const;
		int GetSound() const;
		int GetDamage() const;
		SWRange GetRange() const;
		double GetChargeToDrainRatio() const;

		virtual void InvalidatePointer(void *ptr, bool bRemoved) {
		}

	private:
		static SuperWeaponAffectedHouse::Value GetRelation(HouseClass* pFirer, HouseClass* pHouse);
		bool IsCellEligible(CellClass* pCell, SuperWeaponTarget::Value allowed);
		bool IsTechnoEligible(TechnoClass* pTechno, SuperWeaponTarget::Value allowed);
	};

	static Container<SWTypeExt> ExtMap;

	static SuperWeaponTypeClass *CurrentSWType;

	bool static Launch(SuperClass* pThis, NewSWType* pData, const CellStruct &Coords, bool IsPlayer);
	void static ClearChronoAnim(SuperClass *pThis);
	void static CreateChronoAnim(SuperClass *pThis, CoordStruct *pCoords, AnimTypeClass *pAnimType);
	static bool ChangeLighting(SuperWeaponTypeClass* pCustom = nullptr);
	static LightingColor GetLightingColor(SuperWeaponTypeClass* pCustom = nullptr);
};

class ParadropPlane {
public:
	Valueable<AircraftTypeClass*> Aircraft;
	ValueableVector<TechnoTypeClass*> Types;
	ValueableVector<int> Num;
};

#endif
