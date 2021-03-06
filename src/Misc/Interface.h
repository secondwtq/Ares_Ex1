#ifndef INTERFACE_H
#define INTERFACE_H

#include <Windows.h>

class CustomPalette;
struct SHPStruct;

class Interface
{
private:
	Interface(void);
	~Interface(void);

public:
	enum eUIAction {
		uia_Default = 0,
		uia_Message = 1,
		uia_Disable = 2,
		uia_Hide = 3,
		uia_SneakPeek = 13,
		uia_Credits = 15
	};

	struct MenuItem {
		int nIDDlgItem;
		eUIAction uiaAction;
	};

	struct CampaignData {
		char Battle[0x18];
		char Subline[0x1F];
		char ToolTip[0x1F];
		CustomPalette* Palette;
		SHPStruct* Image;
		bool Valid;
	};

	struct ColorData {
		wchar_t* id;
		int colorRGB;
		int selectedIndex;
		char colorSchemeIndex;
		char colorScheme[0x20];
		const wchar_t* sttToolTipSublineText;
	};

	static int lastDialogTemplateID;
	static int nextReturnMenu;
	static int nextAction;
	static const wchar_t* nextMessageText;

	static int slots[4];

	static bool invokeClickAction(eUIAction, char*, int*, int);
	static void updateMenuItems(HWND, MenuItem*, int);
	static void updateMenu(HWND hDlg, int iID);
	static eUIAction parseUIAction(char*, eUIAction);
	static int getSlotIndex(int);

private:
	static void moveItem(HWND, RECT, POINT);
	static void swapItems(HWND, int, int);
};

#endif