#pragma once

#include "GameFramework/SpectatorPawn.h"
#include "FPSSimulatorPlayerController.h"
#include "FPSSimulatorSpectatorPawn.generated.h"

UCLASS()
class FPSSIMULATOR_API AFPSSimulatorSpectatorPawn : public ASpectatorPawn {
	GENERATED_BODY()

public:
	void BeginPlay() override;
	void SetupPlayerInputComponent(class UInputComponent* InInputComponent) override;

protected:
	/* Behavior */
	void LookPitch(float Val);
	void LookYaw(float Val);
	void MoveForward(float Val);
	void MoveUpward(float Val);
	void Strafe(float Val);

	AFPSSimulatorPlayerController* PC;
};