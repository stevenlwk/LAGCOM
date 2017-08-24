#pragma once

#include "FPSSimulatorGameMode.h"
#include "GM_Demo.generated.h"

UCLASS()
class FPSSIMULATOR_API AGM_Demo : public AFPSSimulatorGameMode {
	GENERATED_BODY()

	FTransform GetSpawnTransform(int32 TeamID) override;
};
