#pragma once

#include "FPSSimulatorGameState.h"
#include "GS_Demo.generated.h"

UCLASS()
class FPSSIMULATOR_API AGS_Demo : public AFPSSimulatorGameState {
	GENERATED_BODY()

public:
	void BeginPlay() override;
};
