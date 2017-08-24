#pragma once

#include "FPSSimulatorGameMode.h"
#include "CTF_Flag.h"
#include "GM_CTF.generated.h"

UCLASS()
class FPSSIMULATOR_API AGM_CTF : public AFPSSimulatorGameMode {
	GENERATED_BODY()

public:
	ACTF_Flag* SpawnFlag(int32 TeamID);

private:
	UPROPERTY(EditDefaultsOnly, Category = "Capture the Flag")
	TSubclassOf<class ACTF_Flag> CTF_FlagClass;
};
