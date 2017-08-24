#pragma once

#include "FPSSimulatorPlayerState.h"
#include "CapturePoint.h"
#include "GS_Domination.h"
#include "PS_Domination.generated.h"

UCLASS()
class FPSSIMULATOR_API APS_Domination : public AFPSSimulatorPlayerState {
	GENERATED_BODY()

public:
	void BeginPlay() override;
	void OnKilled(AFPSSimulatorPlayerState* PlayerState) override;
	void OnEnterCapturePoint(ACapturePoint* CapturePoint);
	void OnLeaveCapturePoint();
	void ScoreCapture();

protected:
	UFUNCTION()
	void OnCapturePointStateChanged(ECapturePointState OldState, ECapturePointState NewState);
	UFUNCTION(BlueprintImplementableEvent)
	void OnStartCapture();
	UFUNCTION(Client, Unreliable)
	void CallOnStartCapture();
	UFUNCTION(BlueprintImplementableEvent)
	void OnEndCapture();
	UFUNCTION(Client, Unreliable)
	void CallOnEndCapture();
	void ScoreCapturing();

	FTimerHandle ScoreCaptureTimer;

private:
	AGS_Domination* GameState;
	ACapturePoint* CapturePoint;
};
