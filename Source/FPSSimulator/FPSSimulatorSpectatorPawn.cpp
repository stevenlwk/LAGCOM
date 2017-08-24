#include "FPSSimulator.h"
#include "FPSSimulatorSpectatorPawn.h"

void AFPSSimulatorSpectatorPawn::BeginPlay() {
	Super::BeginPlay();
	PC = Cast<AFPSSimulatorPlayerController>(GetController());
}

void AFPSSimulatorSpectatorPawn::SetupPlayerInputComponent(class UInputComponent* InInputComponent) {
	Super::SetupPlayerInputComponent(InInputComponent);
	InputComponent->BindAxis("LookPitch", this, &AFPSSimulatorSpectatorPawn::LookPitch);
	InputComponent->BindAxis("LookYaw", this, &AFPSSimulatorSpectatorPawn::LookYaw);
	InputComponent->BindAxis("MoveForward", this, &AFPSSimulatorSpectatorPawn::MoveForward);
	InputComponent->BindAxis("MoveUpward", this, &AFPSSimulatorSpectatorPawn::MoveUpward);
	InputComponent->BindAxis("Strafe", this, &AFPSSimulatorSpectatorPawn::Strafe);
}

void AFPSSimulatorSpectatorPawn::LookPitch(float Val) {
	if(PC && Val != 0) PC->AddPitchInput(Val);
}

void AFPSSimulatorSpectatorPawn::LookYaw(float Val) {
	if(PC && Val != 0) PC->AddYawInput(Val);
}

void AFPSSimulatorSpectatorPawn::MoveForward(float Val) {
	if(Val != 0) {
		const FVector Direction = FRotationMatrix(GetControlRotation()).GetUnitAxis(EAxis::X);

		AddMovementInput(FVector(Direction.X, Direction.Y, 0), Val);
	}
}

void AFPSSimulatorSpectatorPawn::MoveUpward(float Val) {
	if(Val != 0) AddMovementInput(FVector(0, 0, 1), Val, true);
}

void AFPSSimulatorSpectatorPawn::Strafe(float Val) {
	if(Val != 0) AddMovementInput(FRotationMatrix(GetControlRotation()).GetScaledAxis(EAxis::Y), Val);
}