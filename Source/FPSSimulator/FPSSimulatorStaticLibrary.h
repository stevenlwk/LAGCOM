#pragma once

#include "Object.h"
#include "Paths.h"
#include "FPSSimulatorStaticLibrary.generated.h"

UCLASS()
class FPSSIMULATOR_API UFPSSimulatorStaticLibrary : public UObject {
	GENERATED_BODY()

public:
	static FORCEINLINE FString GetGameDir() {
		return FPaths::GameDir();
	};
	static bool SaveTextToFile(FString Directory, FString FileName, FString Text, bool AllowOverwriting = false);
};
