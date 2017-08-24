#include "FPSSimulator.h"
#include "FPSSimulatorStaticLibrary.h"

bool UFPSSimulatorStaticLibrary::SaveTextToFile(FString Directory, FString FileName, FString Text, bool AllowOverwriting /*= false*/) {
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	if(PlatformFile.CreateDirectoryTree(*Directory)) {
		FString Path = Directory + "/" + FileName;

		if(AllowOverwriting || !PlatformFile.FileExists(*Path)) {
			return FFileHelper::SaveStringToFile(Text, *Path);
		}
	}
	return false;
}
