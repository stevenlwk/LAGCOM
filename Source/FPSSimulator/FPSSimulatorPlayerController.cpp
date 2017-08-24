#include "FPSSimulator.h"
#include "FPSSimulatorPlayerController.h"
#include "FPSSimulatorCharacter.h"
#include "FPSSimulatorGameMode.h"
#include "FPSSimulatorPlayerState.h"

void AFPSSimulatorPlayerController::BeginPlay() {
	Super::BeginPlay();
	World = GetWorld();
	GameState = World->GetGameState<AFPSSimulatorGameState>();
	if(HasAuthority()) {
		GameInstance = Cast<UFPSSimulatorGameInstance>(GetGameInstance());
		GameMode = World->GetAuthGameMode<AFPSSimulatorGameMode>();
		PS = Cast<AFPSSimulatorPlayerState>(PlayerState);
	} else {
		if(GConfig) {
			GConfig->GetFloat(TEXT("/Game/Blueprints/FPSSimulatorPlayerController_BP.FPSSimulatorPlayerController_BP_C"), TEXT("FieldOfView"), FieldOfView, GGameIni);
			SetFOV(FieldOfView);
			GConfig->GetFloat(TEXT("/Game/Blueprints/FPSSimulatorPlayerController_BP.FPSSimulatorPlayerController_BP_C"), TEXT("MouseSensitivity"), MouseSensitivity, GGameIni);
			SetMouseSensitivity(MouseSensitivity);
			GConfig->GetInt(TEXT("/Game/Blueprints/FPSSimulatorPlayerController_BP.FPSSimulatorPlayerController_BP_C"), TEXT("MinimapSize"), MinimapSize, GGameIni);
			SetMinimapSize(MinimapSize);
			GConfig->GetInt(TEXT("/Game/Blueprints/FPSSimulatorPlayerController_BP.FPSSimulatorPlayerController_BP_C"), TEXT("PlanarReflectionLevelValue"), PlanarReflectionLevelValue, GGameIni);
			PlanarReflectionLevel = (EPlanarReflectionLevel)(uint8)PlanarReflectionLevelValue;
			OnPlanarReflecitonLevelUpdatedDelegate.Broadcast();
		}
		// UI
		UIBlurAlphaMask = NewObject<ADynamicTextureManager>();
		UIBlurColor = NewObject<ADynamicTextureManager>();
		//ClientSetHUD(HUDClass);
	}
}

void AFPSSimulatorPlayerController::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);
	if(Role < ROLE_Authority) {
		if(GEngine && GEngine->GameViewport) {
			FIntPoint ViewportSize = GEngine->GameViewport->Viewport->GetSizeXY();

			if((ViewportSize.X != Width || ViewportSize.Y != Height) && UIBlurAlphaMask->IsValidLowLevel() && UIBlurColor->IsValidLowLevel()) {
				Width = ViewportSize.X;
				Height = ViewportSize.Y;
				UIBlurAlphaMask->CreateTexture(Width, Height, FColor(0, 0, 0, 0));
				UIBlurColor->CreateTexture(Width, Height, FColor(0, 0, 0, 0));
				UIBlurMaskInitedDelegate.Broadcast();
			}
		}
	}
}

void AFPSSimulatorPlayerController::OnKilled_Implementation(AFPSSimulatorCharacter* Victim, int32 KillStreak) {
	OnKilled_BP(Victim, KillStreak);
}

void AFPSSimulatorPlayerController::ClientDebugMessage_Implementation(const FString& Message) {
	DebugMessage(Message);
}

float AFPSSimulatorPlayerController::GetFOV() {
	return PlayerCameraManager->GetFOVAngle();
}

float AFPSSimulatorPlayerController::GetMouseSensitivity() {
	return PlayerInput->GetMouseSensitivity();
}

float AFPSSimulatorPlayerController::GetMinimapSize() {
	return MinimapSize;
}

float AFPSSimulatorPlayerController::GetMinimapZoomLevel() {
	return MinimapZoomLevel;
}

FKey AFPSSimulatorPlayerController::GetKeyForAction(FName ActionName) {
	UInputSettings* Settings = const_cast<UInputSettings*>(GetDefault<UInputSettings>());

	if(Settings) {
		TArray<FInputActionKeyMapping>& ActionMappings = Settings->ActionMappings;

		for(FInputActionKeyMapping& ActionMapping : ActionMappings) {
			if(ActionMapping.ActionName.IsEqual(ActionName)) {
				return ActionMapping.Key;
			}
		}
	}
	return FKey::FKey();
}

void AFPSSimulatorPlayerController::ConfirmHit_Implementation(AFPSSimulatorCharacter* DamageCauser, AFPSSimulatorCharacter* Target, FVector HitFrom, FName BoneName, FVector HitOffset) {
	DamageCauser->ConfirmHit(Target, HitFrom, BoneName, HitOffset);
}

bool AFPSSimulatorPlayerController::ConfirmHit_Validate(AFPSSimulatorCharacter* DamageCauser, AFPSSimulatorCharacter* Target, FVector HitFrom, FName BoneName, FVector HitOffset) {
	return true;
}

void AFPSSimulatorPlayerController::SetFOV(float NewFOV) {
	PlayerCameraManager->SetFOV(NewFOV);
	FieldOfView = NewFOV;
	OnFOVChangedDelegate.Broadcast();
	SaveConfig();
}

void AFPSSimulatorPlayerController::RequestSetLagCompMethod_Implementation(ELagCompMethod NewLagCompMethod) {
	GameState->SetLagCompMethod(NewLagCompMethod);
}

bool AFPSSimulatorPlayerController::RequestSetLagCompMethod_Validate(ELagCompMethod NewLagCompMethod) {
	return true;
}

void AFPSSimulatorPlayerController::SwitchLagCompMethod() {
	switch(GameState->GetLagCompMethod()) {
		case ELagCompMethod::None:
			RequestSetLagCompMethod(ELagCompMethod::Normal);
			break;
		case ELagCompMethod::Normal:
			RequestSetLagCompMethod(ELagCompMethod::Advanced);
			break;
		case ELagCompMethod::Advanced:
			RequestSetLagCompMethod(ELagCompMethod::None);
	}
}

void AFPSSimulatorPlayerController::SetMouseSensitivity(float NewMouseSensitivity) {
	PlayerInput->SetMouseSensitivity(NewMouseSensitivity);
	MouseSensitivity = NewMouseSensitivity;
	SaveConfig();
}

void AFPSSimulatorPlayerController::SetMinimapSize_Implementation(int32 NewMinimapSize) {
	MinimapSize = NewMinimapSize;
	OnMinimapSizeUpdatedDelegate.Broadcast();
	SaveConfig();
}

void AFPSSimulatorPlayerController::SetMinimapZoomLevel_Implementation(float NewMinimapZoomLevel) {
	MinimapZoomLevel = NewMinimapZoomLevel;
	OnMinimapZoomLevelUpdatedDelegate.Broadcast();
	SaveConfig();
}

void AFPSSimulatorPlayerController::SetPlanarReflectionLevel(EPlanarReflectionLevel NewPlanarReflectionLevel) {
	PlanarReflectionLevel = NewPlanarReflectionLevel;
	PlanarReflectionLevelValue = (uint8)NewPlanarReflectionLevel;
	OnPlanarReflecitonLevelUpdatedDelegate.Broadcast();
	SaveConfig();
}

void AFPSSimulatorPlayerController::SetKeyForAction(FName ActionName, FKey Key) {
	UInputSettings* Settings = const_cast<UInputSettings*>(GetDefault<UInputSettings>());

	if(Settings) {
		TArray<FInputActionKeyMapping>& ActionMappings = Settings->ActionMappings;

		for(int32 i = 0; i < ActionMappings.Num(); i++) {
			if(ActionMappings[i].ActionName.IsEqual(ActionName)) {
				Settings->RemoveActionMapping(ActionMappings[i]);
			}
		}
		Settings->AddActionMapping(FInputActionKeyMapping(ActionName, Key));
		Settings->SaveKeyMappings();
		for(TObjectIterator<UPlayerInput> It; It; ++It) {
			It->ForceRebuildingKeyMaps(true);
		}
		OnKeyBindedDelegate.Broadcast(ActionName);
	}
}

void AFPSSimulatorPlayerController::UnsetKeyForAction(FName ActionName) {
	UInputSettings* Settings = const_cast<UInputSettings*>(GetDefault<UInputSettings>());

	if(Settings) {
		TArray<FInputActionKeyMapping>& ActionMappings = Settings->ActionMappings;

		for(int32 i = 0; i < ActionMappings.Num(); i++) {
			if(ActionMappings[i].ActionName.IsEqual(ActionName)) {
				Settings->RemoveActionMapping(ActionMappings[i]);
			}
		}
		Settings->SaveKeyMappings();
		for(TObjectIterator<UPlayerInput> It; It; ++It) {
			It->ForceRebuildingKeyMaps(true);
		}
		OnKeyUnbindedDelegate.Broadcast(ActionName);
	}

}

void AFPSSimulatorPlayerController::SetMatchConfig_Implementation(FMatchConfig NewMatchConfig) { if(GameState) GameState->SetMatchConfig(NewMatchConfig); }

bool AFPSSimulatorPlayerController::SetMatchConfig_Validate(FMatchConfig NewMatchConfig) { return true; }

void AFPSSimulatorPlayerController::SetRoundTime_Implementation(int32 NewRoundTime) { if(GameState) GameState->SetRoundTime(NewRoundTime); }

bool AFPSSimulatorPlayerController::SetRoundTime_Validate(int32 NewRoundTime) { return true; }

void AFPSSimulatorPlayerController::SetRoundIntervalTime_Implementation(int32 NewRoundIntervalTime) { if(GameState) GameState->SetRoundIntervalTime(NewRoundIntervalTime); }

bool AFPSSimulatorPlayerController::SetRoundIntervalTime_Validate(int32 NewRoundIntervalTime) { return true; }

void AFPSSimulatorPlayerController::SetNextMatchConfig_Implementation(FMatchConfig NewNextMatchConfig) { if(GameState) GameState->SetNextMatchConfig(NewNextMatchConfig); }

bool AFPSSimulatorPlayerController::SetNextMatchConfig_Validate(FMatchConfig NewNextMatchConfig) { return true; }

void AFPSSimulatorPlayerController::SetNextRoundTime_Implementation(int32 NewNextRoundTime) { if(GameState) GameState->SetNextRoundTime(NewNextRoundTime); }

bool AFPSSimulatorPlayerController::SetNextRoundTime_Validate(int32 NewNextRoundTime) { return true; }

void AFPSSimulatorPlayerController::SetNextRoundIntervalTime_Implementation(int32 NewNextRoundIntervalTime) { if(GameState) GameState->SetNextRoundIntervalTime(NewNextRoundIntervalTime); }

bool AFPSSimulatorPlayerController::SetNextRoundIntervalTime_Validate(int32 NewNextRoundIntervalTime) { return true; }

void AFPSSimulatorPlayerController::SetNextALCLowPing_Implementation(int32 NewALCLowPing) { if(GameState) GameState->SetNextALCLowPing(NewALCLowPing); }

bool AFPSSimulatorPlayerController::SetNextALCLowPing_Validate(int32 NewALCLowPing) { return true; }

void AFPSSimulatorPlayerController::SetNextALCHighPing_Implementation(int32 NewALCHighPing) { if(GameState) GameState->SetNextALCHighPing(NewALCHighPing); }

bool AFPSSimulatorPlayerController::SetNextALCHighPing_Validate(int32 NewALCHighPing) { return true; }

void AFPSSimulatorPlayerController::SetNextMaxHealth_Implementation(int32 NewMaxHealth) { if(GameState)GameState->SetNextMaxHealth(NewMaxHealth); }

bool AFPSSimulatorPlayerController::SetNextMaxHealth_Validate(int32 NewMaxHealth) { return true; }

void AFPSSimulatorPlayerController::SetNextWalkSpeed_Implementation(float NewWalkSpeed) { if(GameState) GameState->SetNextWalkSpeed(NewWalkSpeed); }

bool AFPSSimulatorPlayerController::SetNextWalkSpeed_Validate(float NewWalkSpeed) { return true; }

void AFPSSimulatorPlayerController::Restart_Implementation() {
	if(GameState) GameState->StartNextRound(true);
}

bool AFPSSimulatorPlayerController::Restart_Validate() {
	return true;
}

void AFPSSimulatorPlayerController::StartNextRound_Implementation() {
	if(GameState) GameState->StartNextRound();
}

bool AFPSSimulatorPlayerController::StartNextRound_Validate() {
	return true;
}

void AFPSSimulatorPlayerController::OnDied_Implementation(AFPSSimulatorCharacter* Killer) {
	SetIgnoreLookInput(true);
	SetIgnoreMoveInput(true);
}

void AFPSSimulatorPlayerController::OnRespawned_Implementation() {
	ResetIgnoreInputFlags();
}

void AFPSSimulatorPlayerController::OnRestart_Implementation(bool bRestart) {
	OnRestart_BP(bRestart);
}

void AFPSSimulatorPlayerController::SetupInputComponent() {
	Super::SetupInputComponent();
	InputComponent->BindAction("Scoreboard", IE_Pressed, this, &AFPSSimulatorPlayerController::ShowScoreboard);
	InputComponent->BindAction("Scoreboard", IE_Released, this, &AFPSSimulatorPlayerController::HideScoreboard);
	InputComponent->BindAction("PauseMenu", IE_Released, this, &AFPSSimulatorPlayerController::TogglePauseMenu);
	InputComponent->BindAction("SwitchLagCompMethod", IE_Released, this, &AFPSSimulatorPlayerController::SwitchLagCompMethod);
	InputComponent->BindAction("FlushDebugInfo", IE_Released, this, &AFPSSimulatorPlayerController::FlushDebugInfo);
}