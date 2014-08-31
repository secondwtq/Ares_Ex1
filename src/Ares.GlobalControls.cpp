#include "Ares.h"
#include "Utilities\Parser.h"
#include <CCINIClass.h>

//	Hououin 140405 WTF?
//	#include <VersionHelpers.h>

bool Ares::GlobalControls::Initialized = 0;
bool Ares::GlobalControls::AllowParallelAIQueues = 1;

bool Ares::GlobalControls::DebugKeysEnabled = true;

byte Ares::GlobalControls::GFX_DX_Force = 0;

CCINIClass *Ares::GlobalControls::INI = nullptr;

std::bitset<3> Ares::GlobalControls::AllowBypassBuildLimit(0ull);

//	Hououin: dunno know where the code comes from, =_=
bool IsWindowsVistaOrGreater() {
	static bool W7 = false;
	static bool Checked = false;
	if(!Checked) {
		Checked = true;
		OSVERSIONINFO osvi;

		ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx(&osvi);

		W7 = (osvi.dwMajorVersion == 6)/* && (osvi.dwMinorVersion >= 1)*/;
	}
	return W7;
}

void Ares::GlobalControls::Load(CCINIClass *pINI) {
	Initialized = 1;
	AllowParallelAIQueues = pINI->ReadBool("GlobalControls", "AllowParallelAIQueues", AllowParallelAIQueues);

	if(pINI->ReadString("GlobalControls", "AllowBypassBuildLimit", "", Ares::readBuffer, Ares::readLength)) {
		bool temp[3] = {};
		int read = Parser<bool, 3>::Parse(Ares::readBuffer, temp);

		for(int i=0; i<read; ++i) {
			int diffIdx = 2 - i; // remapping so that HouseClass::AIDifficulty can be used as an index
			AllowBypassBuildLimit[diffIdx] = temp[i];
		}
	}

	// used by the keyboard commands
	if(pINI == CCINIClass::INI_Rules) {
		DebugKeysEnabled = true;
	}
	DebugKeysEnabled = pINI->ReadBool("GlobalControls", "DebugKeysEnabled", DebugKeysEnabled);
}

void Ares::GlobalControls::LoadConfig() {
	if(INI->ReadString("Graphics.Advanced", "DirectX.Force", Ares::readDefval, Ares::readBuffer, Ares::readLength)) {
		if(!_strcmpi(Ares::readBuffer, "hardware")) {
			GFX_DX_Force = GFX_DX_HW;
		} else if(!_strcmpi(Ares::readBuffer, "emulation")) {
			GFX_DX_Force = GFX_DX_EM;
		}
	}
	if(IsWindowsVistaOrGreater()) {
		GFX_DX_Force = 0;
	}
}

DEFINE_HOOK(6BC0CD, _LoadRA2MD, 5)
{
	Ares::GlobalControls::INI = Ares::OpenConfig("Ares.ini");
	Ares::GlobalControls::LoadConfig();
	return 0;
}
