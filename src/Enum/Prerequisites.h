#ifndef GEN_PREREQ_H
#define GEN_PREREQ_H

#include <CCINIClass.h>
#include <HouseClass.h>
#include <UnitTypeClass.h>

#include <TechnoTypeClass.h>

#include "_Enumerator.hpp"
#include "../Ares.CRT.h"
#include "../Utilities/Iterator.h"

#ifdef DEBUGBUILD
#include "../Misc/Debug.h"
#endif

//	Kyouma Hououin 140831EVE
//		for generic generic prerequisite

class HouseClass;

class GenericPrerequisite;

struct PrerequisiteStruct {
    bool isGeneric = false;
    TechnoTypeClass *SpecificType;
    signed int GenericIndex = -1;

    PrerequisiteStruct(bool generic_ = false, TechnoTypeClass *specifictp = nullptr, signed int genericidx = -1) :
        isGeneric(generic_), SpecificType(specifictp), GenericIndex(genericidx) { }
        
    bool operator == (const PrerequisiteStruct& other) const {
    	if (other.isGeneric & this->isGeneric)
    		return this->isGeneric == other.isGeneric;
   		else {
   			return this->SpecificType == other.SpecificType;
   		}
   	}
};

class GenericPrerequisite : public Enumerable<GenericPrerequisite>
{
public:
    GenericPrerequisite(const char *Title) : Enumerable<GenericPrerequisite>(Title) { }

    virtual ~GenericPrerequisite() override = default;

    virtual void LoadFromINI(CCINIClass *pINI) override;

    static void AddDefaults();

    DynamicVectorClass<PrerequisiteStruct> Prereqs;
};

class Prereqs
{
public:
    typedef Iterator<BuildingTypeClass*> BTypeIter;

    static void Parse(CCINIClass *pINI, const char* section, const char *key, DynamicVectorClass<PrerequisiteStruct> *vec);

    static bool HouseOwnsGeneric(HouseClass *pHouse, PrerequisiteStruct &src);
    static bool HouseOwnsSpecific(HouseClass *pHouse, PrerequisiteStruct &src);
    static bool HouseOwnsPrereq(HouseClass *pHouse, PrerequisiteStruct &src);

    static bool HouseOwnsAll(HouseClass *pHouse, DynamicVectorClass<PrerequisiteStruct> *list);
    static bool HouseOwnsAny(HouseClass *pHouse, DynamicVectorClass<PrerequisiteStruct> *list);

    static bool ListContainsGeneric(const BTypeIter &List, PrerequisiteStruct &src);
    static bool ListContainsSpecific(const BTypeIter &List, PrerequisiteStruct &src);
    static bool ListContainsPrereq(const BTypeIter &List, PrerequisiteStruct &src);

    static bool ListContainsAll(const BTypeIter &List, DynamicVectorClass<PrerequisiteStruct> *Requirements);
    static bool ListContainsAny(const BTypeIter &List, DynamicVectorClass<PrerequisiteStruct> *Requirements);
};

#endif
