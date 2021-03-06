#ifndef NEW_SW_TYPE_H
#define NEW_SW_TYPE_H

#include "../Ext/SWType/Body.h"
#include "../Utilities/Enums.h"

#include <SuperClass.h>

#include <vector>

enum class SWStateMachineIdentifier : unsigned int {
	Invalid = 0xFFFFFFFFu,
	UnitDelivery = 0,
	ChronoWarp = 1,
	PsychicDominator = 2
};

class SWTypeExt;

// New SW Type framework. See SWTypes/*.h for examples of implemented ones. Don't touch yet, still WIP.
class NewSWType
{
	static std::vector<std::unique_ptr<NewSWType>> Array;

	static void Register(std::unique_ptr<NewSWType> pType) {
		pType->TypeIndex = static_cast<int>(Array.size());
		Array.push_back(std::move(pType));
	}

	int TypeIndex;

public:
	NewSWType() : TypeIndex(-1) {
	}

	virtual ~NewSWType() {
	};

	virtual bool CanFireAt(SWTypeExt::ExtData *pSWType, HouseClass* pOwner, const CellStruct &Coords) {
		return pSWType->CanFireAt(pOwner, Coords);
	}

	virtual bool AbortFire(SuperClass* pSW, bool IsPlayer) {
		return false;
	}

	virtual bool Activate(SuperClass* pSW, const CellStruct &Coords, bool IsPlayer) = 0;

	virtual void Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW) {
	}

	virtual void LoadFromINI(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW, CCINIClass *pINI) {
	}

	virtual WarheadTypeClass* GetWarhead(const SWTypeExt::ExtData* pData) const {
		return pData->SW_Warhead;
	}

	virtual AnimTypeClass* GetAnim(const SWTypeExt::ExtData* pData) const {
		return pData->SW_Anim;
	}

	virtual int GetSound(const SWTypeExt::ExtData* pData) const {
		return pData->SW_Sound;
	}

	virtual int GetDamage(const SWTypeExt::ExtData* pData) const {
		return pData->SW_Damage;
	}

	virtual SWRange GetRange(const SWTypeExt::ExtData* pData) const {
		return pData->SW_Range;
	}

	virtual const char* GetTypeString() {
		return "";
	}

	int GetTypeIndex() {
		return TypeIndex;
	}

	virtual bool HandlesType(int type) {
		return false;
	}

	virtual SuperWeaponFlags::Value Flags() {
		return SuperWeaponFlags::None;
	}

	// static methods
	static void Init();

	static NewSWType* GetNthItem(int i) {
		return Array.at(i - FIRST_SW_TYPE).get();
	}

	static int FindIndex(const char* pType);

	static int FindHandler(int Type);
};

// state machines - create one to use delayed effects [create a child class per NewSWType, obviously]
// i.e. start anim/sound 1 frame after clicking, fire a damage wave 25 frames later, and play second sound 50 frames after that...
class SWStateMachine {
	static std::vector<std::unique_ptr<SWStateMachine>> Array;

public:
	SWStateMachine()
		: Type(nullptr), Super(nullptr), Coords(), Clock()
	{ }

	SWStateMachine(int Duration, CellStruct XY, SuperClass *pSuper, NewSWType * pSWType)
		: Type(pSWType), Super(pSuper), Coords(XY)
	{
		Clock.Start(Duration);
	}

	virtual ~SWStateMachine() {
	}

	virtual bool Finished() {
		return Clock.IsDone();
	}

	virtual void Update() {
	};

	virtual void InvalidatePointer(void *ptr, bool remove) {
	};

	int TimePassed() {
		return Unsorted::CurrentFrame - Clock.StartTime;
	}

	SWTypeExt::ExtData * FindExtData() const {
		return SWTypeExt::ExtMap.Find(this->Super->Type);
	}

	virtual SWStateMachineIdentifier GetIdentifier() const = 0;

	// static methods
	static void Register(std::unique_ptr<SWStateMachine> Machine) {
		if(Machine) {
			Array.push_back(std::move(Machine));
		}
	}

	static void UpdateAll();

	static void PointerGotInvalid(void *ptr, bool remove);

	static void Clear();

protected:
	TimerStruct Clock;
	SuperClass* Super;
	NewSWType* Type;
	CellStruct Coords;
};

class UnitDeliveryStateMachine : public SWStateMachine {
public:
	UnitDeliveryStateMachine()
		: SWStateMachine()
	{ }

	UnitDeliveryStateMachine(int Duration, CellStruct XY, SuperClass *pSuper, NewSWType * pSWType)
		: SWStateMachine(Duration, XY, pSuper, pSWType)
	{ }

	virtual void Update();

	virtual SWStateMachineIdentifier GetIdentifier() const override {
		return SWStateMachineIdentifier::UnitDelivery;
	}

	void PlaceUnits();
};

class ChronoWarpStateMachine : public SWStateMachine {
public:
	struct ChronoWarpContainer {
	public:
		BuildingClass* pBld;
		CellStruct target;
		CoordStruct origin;
		bool isVehicle;

		ChronoWarpContainer(BuildingClass* pBld, CellStruct target, CoordStruct origin, bool isVehicle) :
			pBld(pBld),
			target(target),
			origin(origin),
			isVehicle(isVehicle)
		{
		}

		ChronoWarpContainer() {
		}

		bool operator == (const ChronoWarpContainer &t) const {
			return (this->pBld == t.pBld);
		}
	};

	ChronoWarpStateMachine()
		: SWStateMachine(), Buildings(), Duration(0)
	{ }

	ChronoWarpStateMachine(int Duration, CellStruct XY, SuperClass *pSuper, NewSWType * pSWType, DynamicVectorClass<ChronoWarpContainer> *Buildings)
		: SWStateMachine(Duration, XY, pSuper, pSWType)
	{
		for(int i=0; i<Buildings->Count; ++i) {
			this->Buildings.AddItem(Buildings->GetItem(i));
		}
		this->Duration = Duration;
	}

	virtual void Update();

	virtual void InvalidatePointer(void *ptr, bool remove);

	virtual SWStateMachineIdentifier GetIdentifier() const override {
		return SWStateMachineIdentifier::ChronoWarp;
	}

protected:
	DynamicVectorClass<ChronoWarpContainer> Buildings;
	int Duration;
};

class PsychicDominatorStateMachine : public SWStateMachine {
public:
	PsychicDominatorStateMachine()
		: SWStateMachine(), Deferment(0)
	{}

	PsychicDominatorStateMachine(CellStruct XY, SuperClass *pSuper, NewSWType * pSWType)
		: SWStateMachine(MAXINT32, XY, pSuper, pSWType), Deferment(0)
	{
		PsyDom::Status = PsychicDominatorStatus::FirstAnim;

		// the initial deferment
		SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pSuper->Type);
		this->Deferment = pData->SW_Deferment.Get(0);

		// make the game happy
		PsyDom::Owner = pSuper->Owner;
		PsyDom::Coords = XY;
		PsyDom::Anim = nullptr;
	};

	virtual void Update();

	virtual SWStateMachineIdentifier GetIdentifier() const override {
		return SWStateMachineIdentifier::PsychicDominator;
	}

protected:
	int Deferment;
};

#endif
