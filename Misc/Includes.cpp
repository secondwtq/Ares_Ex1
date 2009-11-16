#include "../Ares.h"
#include "Includes.h"

int Includes::LastReadIndex = -1;
DynamicVectorClass<CCINIClass*> Includes::LoadedINIs;
DynamicVectorClass<char*> Includes::LoadedINIFiles;

// 474200, 6
DEFINE_HOOK(474200, CCINIClass_ReadCCFile1, 6)
{
	Includes::LoadedINIs.AddItem((CCINIClass *)R->get_ECX());
	Includes::LoadedINIFiles.AddItem(_strdup(((CCFileClass*)R->get_EAX())->GetFileName()));
	return 0;
}

// 474314, 6
DEFINE_HOOK(474314, CCINIClass_ReadCCFile2, 6)
{
	char buffer[0x80];
	CCINIClass *xINI = Includes::LoadedINIs[Includes::LoadedINIs.Count - 1];
	const char *key;

	if(!xINI) {
		return 0;
	}

	char section[] = "#include";

	int len = xINI->GetKeyCount(section);
	for(int i = Includes::LastReadIndex; i < len; i = Includes::LastReadIndex) {
		key = xINI->GetKeyName(section, i);
		++Includes::LastReadIndex;
		if(xINI->ReadString(section, key, "", buffer, 0x80)) {
			bool canLoad = 1;
			for(int j = 0; j < Includes::LoadedINIFiles.Count; ++j ) {
				if(!strcmp(Includes::LoadedINIFiles[j], buffer)) {
					canLoad = 0;
					break;
				}
			}

			if(canLoad) {
				CCFileClass *xFile;
				GAME_ALLOC(CCFileClass, xFile, buffer);
				if(xFile->Exists(NULL)) {
					xINI->ReadCCFile(xFile);
				}
				GAME_DEALLOC(xFile);
			}
		}
	}

	if(!len) {
		Includes::LastReadIndex = -1;
	}

	Includes::LoadedINIs.RemoveItem(Includes::LoadedINIs.Count - 1);
	if(!Includes::LoadedINIs.Count) {
		for(int j = Includes::LoadedINIs.Count - 1; j > 0; --j) {
			Includes::LoadedINIs.RemoveItem(j);
		}
		for(int j = Includes::LoadedINIFiles.Count - 1; j > 0; --j) {
			free(Includes::LoadedINIFiles[j]);
			Includes::LoadedINIFiles.RemoveItem(j);
		}
		return 0;
	}
	return 0;
}