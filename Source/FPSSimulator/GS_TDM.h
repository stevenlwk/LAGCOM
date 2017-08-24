#pragma once

#include "FPSSimulatorGameState.h"
#include "GS_TDM.generated.h"

UCLASS()
class FPSSIMULATOR_API AGS_TDM : public AFPSSimulatorGameState {
	GENERATED_BODY()

public:
	void OnKilled(AFPSSimulatorPlayerState* DamageCauserPlayerState, AFPSSimulatorPlayerState* PlayerState) override;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Game")
	int32 NumKillsToWin = 20;
};
