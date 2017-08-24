#pragma once

#include "FPSSimulatorGameState.h"
#include "CapturePoint.h"
#include "GS_Domination.generated.h"

UCLASS()
class FPSSIMULATOR_API AGS_Domination : public AFPSSimulatorGameState {
	GENERATED_BODY()

public:
	AGS_Domination();
	void BeginPlay() override;
	void Tick(float DeltaSeconds) override;

	/* Scores */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Score")
	int32 CapturingScore = 10;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Score")
	int32 CaptureScore = 25;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Score")
	int32 DefenseScore = 10;

protected:
	void ScoreCapture(ACapturePoint* CapturePoint);

	TArray<ACapturePoint*> CapturePoints;
	UPROPERTY(EditDefaultsOnly, Category = "Game")
	float CaptureRate = 0;
	UPROPERTY(EditDefaultsOnly, Category = "Game")
	float TicketGainRate = 0;
};
