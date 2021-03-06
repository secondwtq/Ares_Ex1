#include "Body.h"

DEFINE_HOOK(6DD8D7, TActionClass_Execute, A)
{
	GET(TActionClass*, pAction, ESI);
	GET(ObjectClass*, pObject, ECX);

	GET_STACK(HouseClass*, pHouse, 0x254);
	GET_STACK(TriggerClass*, pTrigger, 0x25C);
	GET_STACK(CellStruct*, pCell, 0x260);

	// check for actions handled in Ares.
	bool ret = false;
	if(TActionExt::Execute(pAction, pHouse, pObject, pTrigger, pCell, &ret)) {
		// returns true or false
		R->AL(ret ? 1 : 0);
		return 0x6DFDDD;
	}

	// replicate the original instructions
	auto value = pAction->ActionKind - 1;
	R->EDX(value);
	return (value > 144) ? 0x6DFDDD : 0x6DD8E7;
}
