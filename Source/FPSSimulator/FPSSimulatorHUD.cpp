#include "FPSSimulator.h"
#include "FPSSimulatorHUD.h"
#include "FPSSimulatorGameState.h"
#include "FPSSimulatorPlayerController.h"

void AFPSSimulatorHUD::BeginPlay() {
	Super::BeginPlay();
	AFPSSimulatorGameState* GameState = GetWorld()->GetGameState<AFPSSimulatorGameState>();
	AFPSSimulatorPlayerController* PC = Cast<AFPSSimulatorPlayerController>(GetOwner());

	if(GameState && PC) {
		if(GameState->HUDClass) {
			GameModeHUD = CreateWidget<UUserWidget>(PC, GameState->HUDClass);
			GameModeHUD->AddToViewport();
		}
	}
}
