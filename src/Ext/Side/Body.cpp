#include "Body.h"
#include "../../Ares.CRT.h"
#include "../../Utilities/TemplateDef.h"
#include <ScenarioClass.h>
#include <PCX.h>

#include <algorithm>

//Static init
template<> const DWORD Extension<SideClass>::Canary = 0x87654321;
Container<SideExt> SideExt::ExtMap;

template<> SideExt::TT *Container<SideExt>::SavingObject = nullptr;
template<> IStream *Container<SideExt>::SavingStream = nullptr;

int SideExt::CurrentLoadTextColor = -1;

UniqueGamePtr<SHPStruct> SideExt::GraphicalTextImage = nullptr;
UniqueGamePtr<BytePalette> SideExt::GraphicalTextPalette = nullptr;
UniqueGamePtr<ConvertClass> SideExt::GraphicalTextConvert = nullptr;

UniqueGamePtr<SHPStruct> SideExt::DialogBackgroundImage = nullptr;
UniqueGamePtr<BytePalette> SideExt::DialogBackgroundPalette = nullptr;
UniqueGamePtr<ConvertClass> SideExt::DialogBackgroundConvert = nullptr;

void SideExt::ExtData::Initialize(SideClass *pThis)
{
	char* pID = pThis->ID;

	this->ArrayIndex = SideClass::FindIndex(pThis->ID);

	this->ParaDropPlane = AircraftTypeClass::FindIndex("PDPLANE");

	if(!_strcmpi(pID, "Nod")) { //Soviets

		this->EVAIndex = 1;

		this->SidebarMixFileIndex = 2;
		this->SidebarYuriFileNames = false;

		this->ToolTipTextColor = ColorStruct(255, 255, 0);
		this->MessageTextColorIndex = 11;

	} else if(!_strcmpi(pID, "ThirdSide")) { //Yuri

		this->EVAIndex = 2;

		this->SidebarMixFileIndex = 2;
		this->SidebarYuriFileNames = true;

		this->ToolTipTextColor = ColorStruct(255, 255, 0);
		this->MessageTextColorIndex = 25;

	} else { //Allies or any other country

		this->EVAIndex = 0;

		this->SidebarMixFileIndex = 1;
		this->SidebarYuriFileNames = false;

		this->ToolTipTextColor = ColorStruct(164, 210, 255);
		this->MessageTextColorIndex = 21;
	}

	switch(this->ArrayIndex) {
	case 0: // Allied
		this->ScoreCampaignBackground = "ASCRBKMD.SHP";
		this->ScoreCampaignTransition = "ASCRTMD.SHP";
		this->ScoreCampaignAnimation = "ASCRAMD.SHP";
		this->ScoreCampaignPalette = "ASCORE.PAL";
		this->ScoreMultiplayBackground = "MPASCRNL.SHP";
		this->ScoreMultiplayBars = "mpascrnlbar~~.pcx";
		this->ScoreMultiplayPalette = "MPASCRN.PAL";
		break;
	case 1: // Soviet
		this->ScoreCampaignBackground = "SSCRBKMD.SHP";
		this->ScoreCampaignTransition = "SSCRTMD.SHP";
		this->ScoreCampaignAnimation = "SSCRAMD.SHP";
		this->ScoreCampaignPalette = "SSCORE.PAL";
		this->ScoreMultiplayBackground = "MPSSCRNL.SHP";
		this->ScoreMultiplayBars = "mpsscrnlbar~~.pcx";
		this->ScoreMultiplayPalette = "MPSSCRN.PAL";
		break;
	default: // Yuri and others
		this->ScoreCampaignBackground = "SYCRBKMD.SHP";
		this->ScoreCampaignTransition = "SYCRTMD.SHP";
		this->ScoreCampaignAnimation = "SYCRAMD.SHP";
		this->ScoreCampaignPalette = "YSCORE.PAL";
		this->ScoreMultiplayBackground = "MPYSCRNL.SHP";
		this->ScoreMultiplayBars = "mpyscrnlbar~~.pcx";
		this->ScoreMultiplayPalette = "MPYSCRN.PAL";
	}
};

void SideExt::ExtData::LoadFromINIFile(SideClass *pThis, CCINIClass *pINI)
{
	char* section = pThis->get_ID();

	INI_EX exINI(pINI);

	this->BaseDefenseCounts.Read(exINI, section, "AI.BaseDefenseCounts");

	this->BaseDefenses.Read(exINI, section, "AI.BaseDefenses");

	this->Crew.Read(exINI, section, "Crew");

	this->Engineer.Read(exINI, section, "Engineer");

	this->Technician.Read(exINI, section, "Technician");

	this->Disguise.Read(exINI, section, "DefaultDisguise");

	this->EVAIndex.Read(exINI, section, "EVA.Tag");

	this->Parachute_Anim.Read(exINI, section, "Parachute.Anim");

	this->ParaDropPlane.Read(exINI, section, "ParaDrop.Aircraft");

	this->ParaDropTypes.Read(exINI, section, "ParaDrop.Types");

	// remove all types that aren't either infantry or unit types
	this->ParaDropTypes.erase(std::remove_if(this->ParaDropTypes.begin(), this->ParaDropTypes.end(), [section](TechnoTypeClass* pItem) -> bool {
		auto abs = pItem->WhatAmI();
		if(abs == InfantryTypeClass::AbsID || abs == UnitTypeClass::AbsID) {
			return false;
		}

		Debug::INIParseFailed(section, "ParaDrop.Types", pItem->ID, "Only InfantryTypes and UnitTypes are supported.");
		return true;
	}), this->ParaDropTypes.end());

	this->ParaDropNum.Read(exINI, section, "ParaDrop.Num");

	this->SidebarMixFileIndex =  pINI->ReadInteger(section, "Sidebar.MixFileIndex", this->SidebarMixFileIndex);
	this->SidebarYuriFileNames = pINI->ReadBool(section, "Sidebar.YuriFileNames", this->SidebarYuriFileNames);
	this->ToolTipTextColor.Read(exINI, section, "ToolTipColor");
	this->SurvivorDivisor.Read(exINI, section, "SurvivorDivisor");
	this->MessageTextColorIndex.Read(exINI, section, "MessageTextColor");

	this->HunterSeeker.Read(exINI, section, "HunterSeeker");

	// score screens
	this->ScoreCampaignBackground.Read(pINI, section, "CampaignScore.Background");
	this->ScoreCampaignTransition.Read(pINI, section, "CampaignScore.Transition");
	this->ScoreCampaignAnimation.Read(pINI, section, "CampaignScore.Animation");
	this->ScoreCampaignPalette.Read(pINI, section, "CampaignScore.Palette");
	this->ScoreMultiplayBackground.Read(pINI, section, "MultiplayerScore.Background");
	this->ScoreMultiplayPalette.Read(pINI, section, "MultiplayerScore.Palette");
	this->ScoreMultiplayBars.Read(pINI, section, "MultiplayerScore.Bars");

	for(int i = 0; i < 10; ++i) {
		auto pFilename = this->GetMultiplayerScoreBarFilename(i);
		if(!PCX::Instance->GetSurface(pFilename)) {
			PCX::Instance->LoadFile(pFilename);
		}
	}

	this->ScoreCampaignThemeUnderPar.Read(pINI, section, "CampaignScore.UnderParTheme");
	this->ScoreCampaignThemeOverPar.Read(pINI, section, "CampaignScore.OverParTheme");
	this->ScoreMultiplayThemeWin.Read(pINI, section, "MultiplayerScore.WinTheme");
	this->ScoreMultiplayThemeLose.Read(pINI, section, "MultiplayerScore.LoseTheme");

	this->GraphicalTextImage.Read(pINI, section, "GraphicalText.Image");
	this->GraphicalTextPalette.Read(pINI, section, "GraphicalText.Palette");

	this->DialogBackgroundImage.Read(pINI, section, "DialogBackground.Image");
	this->DialogBackgroundPalette.Read(pINI, section, "DialogBackground.Palette");
}

int SideExt::ExtData::GetSurvivorDivisor() const {
	if(this->SurvivorDivisor.isset()) {
		return this->SurvivorDivisor;
	}

	return this->GetDefaultSurvivorDivisor();
}

int SideExt::ExtData::GetDefaultSurvivorDivisor() const {
	switch(this->ArrayIndex) {
	case 0:
		return RulesClass::Instance->AlliedSurvivorDivisor;
	case 1:
		return RulesClass::Instance->SovietSurvivorDivisor;
	case 2:
		return RulesClass::Instance->ThirdSurvivorDivisor;
	default:
		//return 0; would be correct, but Ares < 0.5 does this:
		return RulesClass::Instance->AlliedSurvivorDivisor;
	}
}

InfantryTypeClass* SideExt::ExtData::GetCrew() const {
	if(this->Crew.isset()) {
		return this->Crew;
	}

	return this->GetDefaultCrew();
}

InfantryTypeClass* SideExt::ExtData::GetDefaultCrew() const {
	switch(this->ArrayIndex) {
	case 0:
		return RulesClass::Instance->AlliedCrew;
	case 1:
		return RulesClass::Instance->SovietCrew;
	case 2:
		return RulesClass::Instance->ThirdCrew;
	default:
		//return RulesClass::Instance->Technician; would be correct, but Ares < 0.5 does this:
		return RulesClass::Instance->AlliedCrew;
	}
}

InfantryTypeClass* SideExt::ExtData::GetEngineer() const {
	return this->Engineer.Get(RulesClass::Instance->Engineer);
}

InfantryTypeClass* SideExt::ExtData::GetTechnician() const {
	return this->Technician.Get(RulesClass::Instance->Technician);
}

InfantryTypeClass* SideExt::ExtData::GetDisguise() const {
	if(this->Disguise.isset()) {
		return this->Disguise;
	}

	return this->GetDefaultDisguise();
}

InfantryTypeClass* SideExt::ExtData::GetDefaultDisguise() const {
	switch(this->ArrayIndex) {
	case 0:
		return RulesClass::Instance->AlliedDisguise;
	case 1:
		return RulesClass::Instance->SovietDisguise;
	case 2:
		return RulesClass::Instance->ThirdDisguise;
	default:
		//return RulesClass::Instance->ThirdDisguise; would be correct, but Ares < 0.5 does this:
		return RulesClass::Instance->AlliedDisguise;
	}
}

Iterator<int> SideExt::ExtData::GetBaseDefenseCounts() const {
	if(this->BaseDefenseCounts.HasValue()) {
		return this->BaseDefenseCounts;
	}

	return this->GetDefaultBaseDefenseCounts();
}

Iterator<int> SideExt::ExtData::GetDefaultBaseDefenseCounts() const {
	switch(this->ArrayIndex) {
	case 0:
		return RulesClass::Instance->AlliedBaseDefenseCounts;
	case 1:
		return RulesClass::Instance->SovietBaseDefenseCounts;
	case 2:
		return RulesClass::Instance->ThirdBaseDefenseCounts;
	default:
		//return Iterator<int>(); would be correct, but Ares < 0.5 does this:
		return RulesClass::Instance->AlliedBaseDefenseCounts;
	}
}

Iterator<BuildingTypeClass*> SideExt::ExtData::GetBaseDefenses() const {
	if(this->BaseDefenses.HasValue()) {
		return this->BaseDefenses;
	}

	return this->GetDefaultBaseDefenses();
}

Iterator<BuildingTypeClass*> SideExt::ExtData::GetDefaultBaseDefenses() const {
	switch(this->ArrayIndex) {
	case 0:
		return RulesClass::Instance->AlliedBaseDefenses;
	case 1:
		return RulesClass::Instance->SovietBaseDefenses;
	case 2:
		return RulesClass::Instance->ThirdBaseDefenses;
	default:
		//return Iterator<BuildingTypeClass*>(); would be correct, but Ares < 0.5 does this:
		return RulesClass::Instance->AlliedBaseDefenses;
	}
}

Iterator<TechnoTypeClass*> SideExt::ExtData::GetParaDropTypes() const {
	if(this->ParaDropTypes.HasValue() && this->ParaDropNum.HasValue()) {
		return this->ParaDropTypes;
	}

	return this->GetDefaultParaDropTypes();
}

Iterator<InfantryTypeClass*> SideExt::ExtData::GetDefaultParaDropTypes() const {
	switch(this->ArrayIndex) {
	case 0:
		return RulesClass::Instance->AllyParaDropInf;
	case 1:
		return RulesClass::Instance->SovParaDropInf;
	case 2:
		return RulesClass::Instance->YuriParaDropInf;
	default:
		//return SovParaDropInf would be correct, but Ares < 0.6 does this:
		return RulesClass::Instance->AllyParaDropInf;
	}
}

Iterator<int> SideExt::ExtData::GetParaDropNum() const {
	if(this->ParaDropTypes.HasValue() && this->ParaDropNum.HasValue()) {
		return this->ParaDropNum;
	}

	return this->GetDefaultParaDropNum();
}

Iterator<int> SideExt::ExtData::GetDefaultParaDropNum() const {
	switch(this->ArrayIndex) {
	case 0:
		return RulesClass::Instance->AllyParaDropNum;
	case 1:
		return RulesClass::Instance->SovParaDropNum;
	case 2:
		return RulesClass::Instance->YuriParaDropNum;
	default:
		//return SovParaDropNum would be correct, but Ares < 0.6 does this:
		return RulesClass::Instance->AllyParaDropNum;
	}
}

AnimTypeClass* SideExt::ExtData::GetParachuteAnim() const {
	return this->Parachute_Anim.Get(RulesClass::Instance->Parachute);
}

const char* SideExt::ExtData::GetMultiplayerScoreBarFilename(size_t index) const {
	static char filename[_countof(this->ScoreMultiplayBars.data())];
	auto& data = this->ScoreMultiplayBars.data();
	std::transform(std::begin(data), std::end(data), filename, tolower);

	if(auto pMarker = strstr(filename, "~~")) {
		char number[3];
		sprintf_s(number, "%02u", index + 1);
		pMarker[0] = number[0];
		pMarker[1] = number[1];
	}

	return filename;
}

void SideExt::UpdateGlobalFiles()
{
	// clear old data
	SideExt::GraphicalTextImage = nullptr;
	SideExt::GraphicalTextConvert = nullptr;
	SideExt::GraphicalTextPalette = nullptr;

	SideExt::DialogBackgroundImage = nullptr;
	SideExt::DialogBackgroundConvert = nullptr;
	SideExt::DialogBackgroundPalette = nullptr;

	int idxSide = ScenarioClass::Instance->PlayerSideIndex;
	auto pSide = SideClass::Array->GetItemOrDefault(idxSide);
	auto pExt = SideExt::ExtMap.Find(pSide);

	if(!pExt) {
		return;
	}

	// load graphical text shp
	if(pExt->GraphicalTextImage) {
		auto pShp = FileSystem::AllocateFile<SHPStruct>(pExt->GraphicalTextImage);
		SideExt::GraphicalTextImage.reset(pShp);
	}

	// load graphical text palette and create convert
	if(pExt->GraphicalTextPalette) {
		if(auto pPal = FileSystem::AllocatePalette(pExt->GraphicalTextPalette)) {
			SideExt::GraphicalTextPalette.reset(pPal);

			auto pConvert = GameCreate<ConvertClass>(pPal, FileSystem::TEMPERAT_PAL, DSurface::Primary, 1, false);
			SideExt::GraphicalTextConvert.reset(pConvert);
		}
	}

	// load dialog background shp
	if(pExt->DialogBackgroundImage) {
		auto pShp = FileSystem::AllocateFile<SHPStruct>(pExt->DialogBackgroundImage);
		SideExt::DialogBackgroundImage.reset(pShp);
	}

	// load dialog background palette and create convert
	if(pExt->DialogBackgroundPalette) {
		if(auto pPal = FileSystem::AllocatePalette(pExt->DialogBackgroundPalette)) {
			SideExt::DialogBackgroundPalette.reset(pPal);

			auto pConvert = GameCreate<ConvertClass>(pPal, pPal, DSurface::Alternate, 1, false);
			SideExt::DialogBackgroundConvert.reset(pConvert);
		}
	}
}

DWORD SideExt::LoadTextColor(REGISTERS* R, DWORD dwReturnAddress)
{
	// if there is a cached LoadTextColor, use that.
	int index = SideExt::CurrentLoadTextColor;
	if(auto pCS = ColorScheme::Array->GetItemOrDefault(index)) {
		R->EAX(pCS);
		return dwReturnAddress;
	}

	return 0;
}

DWORD SideExt::MixFileYuriFiles(REGISTERS* R, DWORD dwReturnAddress1, DWORD dwReturnAddress2)
{
	GET(ScenarioClass *, pScen, EAX); //TODO test

	SideClass* pSide = SideClass::Array->GetItem(pScen->PlayerSideIndex);
	if(SideExt::ExtData *pData = SideExt::ExtMap.Find(pSide)) {
		return pData->SidebarYuriFileNames
			? dwReturnAddress1
			: dwReturnAddress2
		;
	} else {
		return 0;
	}
}

SHPStruct* SideExt::GetGraphicalTextImage() {
	if(SideExt::GraphicalTextImage) {
		return SideExt::GraphicalTextImage.get();
	}

	return FileSystem::GRFXTXT_SHP;
}

ConvertClass* SideExt::GetGraphicalTextConvert() {
	if(SideExt::GraphicalTextConvert) {
		return SideExt::GraphicalTextConvert.get();
	}

	return FileSystem::GRFXTXT_Convert;
}

// =============================
// load / save

bool Container<SideExt>::Save(SideClass *pThis, IStream *pStm) {
	SideExt::ExtData* pData = this->SaveKey(pThis, pStm);

	if(pData) {
		//ULONG out;
		//pData->BaseDefenses.Save(pStm);
		//pData->BaseDefenseCounts.Save(pStm);
		//pData->ParaDrop.Save(pStm);
		//pData->ParaDropNum.Save(pStm);
	}

	return pData != nullptr;
}

bool Container<SideExt>::Load(SideClass *pThis, IStream *pStm) {
	SideExt::ExtData* pData = this->LoadKey(pThis, pStm);

	//SWIZZLE(pData->Disguise);
	//SWIZZLE(pData->Crew);
	//pData->BaseDefenses.Load(pStm, 1);
	//pData->BaseDefenseCounts.Load(pStm, 0);
	//pData->ParaDrop.Load(pStm, 1);
	//pData->ParaDropNum.Load(pStm, 0);

	return pData != nullptr;
}

// =============================
// container hooks

DEFINE_HOOK(6A4609, SideClass_CTOR, 7)
{
	GET(SideClass*, pItem, ESI);

	SideExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(6A499F, SideClass_SDDTOR, 6)
{
	GET(SideClass*, pItem, ESI);

	SideExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(6A48A0, SideClass_SaveLoad_Prefix, 5)
DEFINE_HOOK(6A4780, SideClass_SaveLoad_Prefix, 6)
{
	GET_STACK(SideExt::TT*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	Container<SideExt>::PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(6A488B, SideClass_Load_Suffix, 6)
{
	SideExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(6A48FC, SideClass_Save_Suffix, 5)
{
	SideExt::ExtMap.SaveStatic();
	return 0;
}

DEFINE_HOOK(679A10, SideClass_LoadAllFromINI, 5)
{
	GET_STACK(CCINIClass*, pINI, 0x4);
	SideExt::ExtMap.LoadAllFromINI(pINI); // bwahaha

	return 0;
}

/*
FINE_HOOK(6725C4, RulesClass_Addition_Sides, 8)
{
	GET(SideClass *, pItem, EBP);
	GET_STACK(CCINIClass*, pINI, 0x38);

	SideExt::ExtMap.LoadFromINI(pItem, pINI);
	return 0;
}
*/
