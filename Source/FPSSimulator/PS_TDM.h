#pragma once

#include "FPSSimulatorPlayerState.h"
#include "PS_TDM.generated.h"

UCLASS()
class FPSSIMULATOR_API APS_TDM : public AFPSSimulatorPlayerState {
	GENERATED_BODY()

public:
	void OnKilled(AFPSSimulatorPlayerState* PlayerState) override;
};
