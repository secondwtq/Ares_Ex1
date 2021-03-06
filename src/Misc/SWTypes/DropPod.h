#ifndef SUPERTYPE_EXT_DROPPOD_H
#define SUPERTYPE_EXT_DROPPOD_H

#include "../SWTypes.h"

class SW_DropPod : public NewSWType
{
public:
	SW_DropPod() : NewSWType()
		{ };

	virtual ~SW_DropPod() override
		{ };

	virtual const char * GetTypeString() override
		{ return "DropPod"; }

	virtual void Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW) override;
	virtual void LoadFromINI(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW, CCINIClass *pINI) override;
	virtual bool Activate(SuperClass* pThis, const CellStruct &Coords, bool IsPlayer) override;
};

#endif
