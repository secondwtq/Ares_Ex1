#ifndef TECHNOTYPE_EXT_H
#define TECHNOTYPE_EXT_H

#include <TechnoTypeClass.h>
#include <ParticleSystemTypeClass.h>

#include "../../Ares.h"
#include "../_Container.hpp"
#include "../../Utilities/Template.h"
#include "../../Utilities/Constructs.h"
#include "../../Misc/AttachEffect.h"
	
#include "../../Enum/Prerequisites.h"

#include <bitset>

class BuildingTypeClass;
class HouseTypeClass;
class VocClass;
class VoxClass;
class WarheadTypeClass;

class TechnoTypeExt
{
public:
	typedef TechnoTypeClass TT;
	enum SpotlightAttachment { sa_Body, sa_Turret, sa_Barrel };

	class ExtData : public Extension<TT>
	{
	public:
		DynamicVectorClass<InfantryTypeClass *> Survivors_Pilots;
		Promotable<int> Survivors_PilotChance;
		Promotable<int> Survivors_PassengerChance;
		// new on 28.09.09 for #631
		int Survivors_PilotCount; //!< Defines the number of pilots inside this vehicle if Crewed=yes; maximum number of pilots who can survive. Defaults to 0 if Crewed=no; defaults to 1 if Crewed=yes. // NOTE: Flag in INI is called Survivor.Pilots
		Nullable<int> Crew_TechnicianChance;
		Nullable<int> Crew_EngineerChance;

		// animated cameos
//		int Cameo_Interval;
//		int Cameo_CurrentFrame;
//		TimerStruct Cameo_Timer;

		//	Kyouma Hououin 140831EVE
		//		for generic generic prerequisite
		std::vector<std::unique_ptr<DynamicVectorClass<PrerequisiteStruct>>> PrerequisiteLists;
		DynamicVectorClass<PrerequisiteStruct> PrerequisiteNegatives;
		DynamicVectorClass<PrerequisiteStruct> PrerequisiteOverride;
		DWORD PrerequisiteTheaters;

		// new secret lab
		DWORD Secret_RequiredHouses;
		DWORD Secret_ForbiddenHouses;

		bool Is_Deso;
		bool Is_Deso_Radiation;
		bool Is_Cow;
		bool Is_Spotlighted;

		// spotlights
		int Spot_Height;
		int Spot_Distance;
		SpotlightAttachment Spot_AttachedTo;
		bool Spot_DisableR;
		bool Spot_DisableG;
		bool Spot_DisableB;
		bool Spot_Reverse;

		bool Is_Bomb;

		// these are not implemented at all yet
		DynamicVectorClass<WeaponStruct> Weapons;
		DynamicVectorClass<WeaponStruct> EliteWeapons;

		Promotable<SHPStruct *> Insignia;
		Nullable<bool> Insignia_ShowEnemy;

		Valueable<AnimTypeClass*> Parachute_Anim;

		// new on 08.11.09 for #342 (Operator=)
		InfantryTypeClass * Operator; //!< Saves a pointer to an InfantryType required to be a passenger of this unit in order for it to work. Defaults to NULL. \sa TechnoClass_Update_CheckOperators, bool IsAPromiscuousWhoreAndLetsAnyoneRideIt
		bool IsAPromiscuousWhoreAndLetsAnyoneRideIt; //!< If this is true, Operator= is not checked, and the object will work with any passenger, provided there is one. \sa InfantryTypeClass * Operator

		CustomPalette CameoPal;

		std::bitset<32> RequiredStolenTech;

		Nullable<bool> ImmuneToEMP;
		bool VeteranAbilityEMPIMMUNE;
		bool EliteAbilityEMPIMMUNE;
		int EMP_Threshold;
		float EMP_Modifier;

		Valueable<double> IronCurtain_Modifier;
		Valueable<double> ForceShield_Modifier;

		Valueable<bool> Chronoshift_Allow;
		Valueable<bool> Chronoshift_IsVehicle;

		// new on 05.04.10 for #733 (KillDriver/"Jarmen Kell")
		bool ProtectedDriver; //!< Whether the driver of this vehicle cannot be killed, i.e. whether this vehicle is immune to KillDriver. Request #733.
		bool CanDrive; //!< Whether this TechnoType can act as the driver of vehicles whose driver has been killed. Request #733.

		bool AlternateTheaterArt;
		
		Valueable<bool> PassengersGainExperience;
		Valueable<bool> ExperienceFromPassengers;
		Valueable<double> PassengerExperienceModifier;
		Valueable<double> MindControlExperienceSelfModifier;
		Valueable<double> MindControlExperienceVictimModifier;
		Valueable<double> SpawnExperienceOwnerModifier;
		Valueable<double> SpawnExperienceSpawnModifier;
		Valueable<bool> ExperienceFromAirstrike;
		Valueable<double> AirstrikeExperienceModifier;

		ValueableIdx<VocClass> VoiceRepair;
		NullableIdx<VocClass> VoiceAirstrikeAttack;
		NullableIdx<VocClass> VoiceAirstrikeAbort;

		ValueableIdx<VocClass> HijackerEnterSound;
		ValueableIdx<VocClass> HijackerLeaveSound;
		Valueable<int> HijackerKillPilots;
		Valueable<bool> HijackerBreakMindControl;
		Valueable<bool> HijackerAllowed;
		Valueable<bool> HijackerOneTime;

		Valueable<UnitTypeClass *> WaterImage;

		NullableIdx<VocClass> CloakSound;
		NullableIdx<VocClass> DecloakSound;
		Valueable<bool> CloakPowered;
		Valueable<bool> CloakDeployed;
		Valueable<bool> CloakAllowed;
		Nullable<int> CloakStages;

		Valueable<bool> SensorArray_Warn;

		AresPCXFile CameoPCX;
		AresPCXFile AltCameoPCX;

		char GroupAs[0x20];

		AresMap<HouseClass*, bool> ReversedByHouses;
		Valueable<bool> CanBeReversed;

		// issue #305
		Valueable<int> RadarJamRadius; //!< Distance in cells to scan for & jam radars

		// issue #1208
		Valueable<bool> PassengerTurret; //!< Whether this unit's turret changes based on the number of people in its passenger hold.

		// issue #617
		ValueableVector<BuildingTypeClass*> PoweredBy;  //!< The buildingtype this unit is powered by or NULL.

		//issue #1623
		AttachEffectTypeClass AttachedTechnoEffect; //The AttachedEffect which should been on the Techno from the start.

		ValueableVector<BuildingTypeClass *> BuiltAt;
		Valueable<bool> Cloneable;
		ValueableVector<BuildingTypeClass *> ClonedAt;

		Nullable<bool> CarryallAllowed;
		Nullable<int> CarryallSizeLimit;

		Valueable<bool> ImmuneToAbduction; //680, 1362

		ValueableVector<HouseTypeClass *> FactoryOwners;
		ValueableVector<HouseTypeClass *> ForbiddenFactoryOwners;
		Valueable<bool> FactoryOwners_HaveAllPlans;

		Valueable<bool> GattlingCyclic;

		Nullable<bool> Crashable;
		Valueable<bool> CrashSpin;
		Valueable<int> AirRate;

		Valueable<bool> CivilianEnemy;

		// custom missiles
		Valueable<bool> IsCustomMissile;
		Valueable<RocketStruct> CustomMissileData;
		Valueable<WarheadTypeClass*> CustomMissileWarhead;
		Valueable<WarheadTypeClass*> CustomMissileEliteWarhead;
		Valueable<AnimTypeClass*> CustomMissileTakeoffAnim;
		Valueable<AnimTypeClass*> CustomMissileTrailerAnim;
		Valueable<int> CustomMissileTrailerSeparation;

		// tiberium related
		Nullable<bool> TiberiumProof;
		Nullable<bool> TiberiumRemains;
		Valueable<bool> TiberiumSpill;
		Nullable<int> TiberiumTransmogrify;

		// refinery and storage related
		Valueable<bool> Refinery_UseStorage;

		ValueableIdx<VoxClass> EVA_UnitLost;

		Valueable<bool> Drain_Local;
		Valueable<int> Drain_Amount;

		// smoke when damaged
		Nullable<int> SmokeChanceRed;
		Nullable<int> SmokeChanceDead;
		Valueable<AnimTypeClass*> SmokeAnim;

		// hunter seeker
		Nullable<int> HunterSeekerDetonateProximity;
		Nullable<int> HunterSeekerDescendProximity;
		Nullable<int> HunterSeekerAscentSpeed;
		Nullable<int> HunterSeekerDescentSpeed;
		Nullable<int> HunterSeekerEmergeSpeed;
		Valueable<bool> HunterSeekerIgnore;

		// particles
		Nullable<bool> DamageSparks;

		NullableVector<ParticleSystemTypeClass*> ParticleSystems_DamageSmoke;
		NullableVector<ParticleSystemTypeClass*> ParticleSystems_DamageSparks;

		//	Kyouma Hououin 140831NOON
		Valueable<int> CrushLevel;
		int Crush_Level;

		Valueable<int> DeployWeaponIndex;
		InfantryTypeClass *CustomDeployWeapon;
		
		Valueable<UnitTypeClass *> YellowImage;
		Valueable<UnitTypeClass *> RedImage;

		Valueable<double> SpeedMultiplierOnTiberium;
		
		Valueable<bool> RecheckTechTreeWhenDeleted;

		ExtData(TT* const OwnerObject) : Extension<TT>(OwnerObject),
			Survivors_PilotChance (),
			Survivors_PassengerChance (),
			Survivors_PilotCount (-1),
			Crew_TechnicianChance (),
			Crew_EngineerChance (),
			PrerequisiteTheaters (0xFFFFFFFF),
			Secret_RequiredHouses (0),
			Secret_ForbiddenHouses (0),
			Is_Deso (false),
			Is_Deso_Radiation (false),
			Is_Cow (false),
			Is_Spotlighted (false),
			Spot_Height (200),
			Spot_Distance (1024),
			Spot_AttachedTo (sa_Body),
			Spot_DisableR (false),
			Spot_DisableG (false),
			Spot_DisableB (false),
			Spot_Reverse (false),
			Drain_Local (false),
			Drain_Amount (0),
			SmokeChanceRed (),
			SmokeChanceDead (),
			SmokeAnim (nullptr),
			HunterSeekerDetonateProximity (),
			HunterSeekerDescendProximity (),
			HunterSeekerAscentSpeed (),
			HunterSeekerDescentSpeed (),
			HunterSeekerEmergeSpeed (),
			HunterSeekerIgnore (false),
			Is_Bomb (false),
			Insignia (),
			Parachute_Anim (nullptr),
			Operator (nullptr),
			IsAPromiscuousWhoreAndLetsAnyoneRideIt (false),
			CameoPal(),
			RequiredStolenTech(0ull),
			Chronoshift_Allow (true),
			Chronoshift_IsVehicle (false),
			IronCurtain_Modifier (1.0),
			ForceShield_Modifier (1.0),
			EMP_Threshold (-1),
			EMP_Modifier (1.0F),
			VeteranAbilityEMPIMMUNE (false),
			EliteAbilityEMPIMMUNE (false),
			ProtectedDriver(false),
			CanDrive (false),
			AlternateTheaterArt (false),
			PassengersGainExperience (false),
			ExperienceFromPassengers (true),
			ExperienceFromAirstrike (false),
			AirstrikeExperienceModifier (1.0),
			PassengerExperienceModifier (1.0),
			MindControlExperienceSelfModifier (0.0),
			MindControlExperienceVictimModifier (1.0),
			SpawnExperienceOwnerModifier (0.0),
			SpawnExperienceSpawnModifier (1.0),
			Insignia_ShowEnemy(),
			GattlingCyclic (false),
			IsCustomMissile (false),
			CustomMissileData (),
			CustomMissileWarhead (nullptr),
			CustomMissileEliteWarhead (nullptr),
			CustomMissileTrailerSeparation (3),
			CustomMissileTrailerAnim (nullptr),
			CustomMissileTakeoffAnim (nullptr),
			VoiceRepair (-1),
			HijackerEnterSound (-1),
			HijackerLeaveSound (-1),
			HijackerKillPilots (0),
			HijackerBreakMindControl (true),
			HijackerAllowed (true),
			HijackerOneTime (false),
			WaterImage (nullptr),
			TiberiumProof (),
			TiberiumRemains(),
			TiberiumSpill (false),
			TiberiumTransmogrify (),
			Refinery_UseStorage (false),
			CloakSound (),
			DecloakSound (),
			CloakPowered (false),
			CloakDeployed (false),
			CloakAllowed (true),
			CloakStages (),
			SensorArray_Warn (true),
			CrashSpin (true),
			CanBeReversed (true),
			RadarJamRadius (0),
			PassengerTurret (false),
			AttachedTechnoEffect(OwnerObject),
			Cloneable (true),
			CarryallAllowed(),
			CarryallSizeLimit (),
			EVA_UnitLost (-1),
			ImmuneToAbduction(false),
			FactoryOwners_HaveAllPlans(false),
			CrushLevel(1), Crush_Level(1),
			CustomDeployWeapon(nullptr),
			DeployWeaponIndex(1),
			YellowImage (nullptr),
			RedImage (nullptr),
			SpeedMultiplierOnTiberium(1.0),
			RecheckTechTreeWhenDeleted(false)
			{
				this->Insignia.SetAll(nullptr);
				*this->GroupAs = 0;
			};

		virtual ~ExtData() {};

		virtual void LoadFromINIFile(TT *pThis, CCINIClass *pINI);
		virtual void Initialize(TT *pThis);

		virtual void InvalidatePointer(void *ptr, bool bRemoved) {
			AnnounceInvalidPointer(Operator, ptr);
		}

		bool CameoIsElite();

		bool CanBeBuiltAt(BuildingTypeClass * FactoryType);

		bool CarryallCanLift(UnitClass * Target);

		const char* GetSelectionGroupID() const;
	};

	static Container<TechnoTypeExt> ExtMap;

	static const char* GetSelectionGroupID(ObjectTypeClass* pType);
	static bool HasSelectionGroupID(ObjectTypeClass* pType, const char* pID);

//	static void ReadWeapon(WeaponStruct *pWeapon, const char *prefix, const char *section, CCINIClass *pINI);
};

#endif
