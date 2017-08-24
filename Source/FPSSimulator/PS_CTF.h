#pragma once

#include "FPSSimulatorPlayerState.h"
#include "GS_CTF.h"
#include "PS_CTF.generated.h"

UCLASS()
class FPSSIMULATOR_API APS_CTF : public AFPSSimulatorPlayerState {
	GENERATED_BODY()

public:
	void BeginPlay() override;
	void OnKilled(AFPSSimulatorPlayerState* PlayerState) override;
	void ScoreCarrierKill();
	void ScoreDeliver();
	void ScorePickup();

private:
	AGS_CTF* GameState;
};
