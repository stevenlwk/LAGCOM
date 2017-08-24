#pragma once

#include "GameFramework/PlayerController.h"
#include "BaseWeapon.h"
#include "DynamicTextureManager.h"
#include "FPSSimulatorCharacter.h"
#include "FPSSimulatorGameInstance.h"
#include "FPSSimulatorGameState.h"
#include "FPSSimulatorHUD.h"
#include "FPSSimulatorPlayerController.generated.h"

class AFPSSimulatorGameMode;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FUIBlurMaskInitedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FUIBlurMaskUpdatedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFOVChangedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnKeyBindedDelegate, FName, ActionName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnKeyUnbindedDelegate, FName, ActionName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMinimapSizeUpdatedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMinimapZoomLevelUpdatedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlanarReflectionLevelUpdatedDelegate);

UENUM(BlueprintType)
enum class EPlanarReflectionLevel: uint8 {
	None,
	SmallOnly,
	LargeOnly,
	All
};

UCLASS(Config = Game)
class FPSSIMULATOR_API AFPSSimulatorPlayerController : public APlayerController {
	GENERATED_BODY()

public:
	void BeginPlay() override;
	void Tick(float DeltaSeconds) override;
	UFUNCTION(BlueprintNativeEvent)
	void OnDied(AFPSSimulatorCharacter* Killer);
	UFUNCTION(Client, Reliable)
	void OnKilled(AFPSSimulatorCharacter* Victim, int32 KillStreak);
	UFUNCTION(BlueprintImplementableEvent)
	void OnKilled_BP(AFPSSimulatorCharacter* Victim, int32 KillStreak);
	UFUNCTION(BlueprintNativeEvent)
	void OnRespawned();
	UFUNCTION(Client, Reliable)
	void OnRestart(bool bRestart);
	UFUNCTION(BlueprintImplementableEvent)
	void OnRestart_BP(bool bRestart);
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "UI")
	void ShowScoreboard();
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "UI")
	void HideScoreboard();
	/* Admin */
	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation, Category = "Admin")
	void SetMatchConfig(FMatchConfig NewMatchConfig);
	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation, Category = "Admin")
	void SetRoundTime(int32 NewRoundTime);
	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation, Category = "Admin")
	void SetRoundIntervalTime(int32 NewRoundIntervalTime);
	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation, Category = "Admin")
	void SetNextMatchConfig(FMatchConfig NewNextMatchConfig);
	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation, Category = "Admin")
	void SetNextRoundTime(int32 NewNextRoundTime);
	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation, Category = "Admin")
	void SetNextRoundIntervalTime(int32 NewNextRoundIntervalTime);
	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation, Category = "Admin")
	void SetNextALCLowPing(int32 NewALCLowPing);
	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation, Category = "Admin")
	void SetNextALCHighPing(int32 NewALCHighPing);
	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation, Category = "Admin")
	void SetNextMaxHealth(int32 NewMaxHealth);
	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation, Category = "Admin")
	void SetNextWalkSpeed(float NewWalkSpeed);
	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation, Category = "Admin")
	void Restart();
	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation, Category = "Admin")
	void StartNextRound();
	/* Debug */
	UFUNCTION(Client, Reliable, Category = "Debug")
	void ClientDebugMessage(const FString& Message);
	UFUNCTION(BlueprintImplementableEvent, Category = "Debug")
	void DebugMessage(const FString& Message);
	UFUNCTION(BlueprintImplementableEvent, Category = "Debug")
	void FlushDebugInfo();
	/* Getter */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Setting")
	float GetFOV();
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Setting")
	float GetMouseSensitivity();
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Setting")
	float GetMinimapSize();
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Setting")
	float GetMinimapZoomLevel();
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Setting")
	FKey GetKeyForAction(FName ActionName);
	/* Networking */
	UFUNCTION(Server, Reliable, WithValidation)
	void ConfirmHit(AFPSSimulatorCharacter* DamageCauser, AFPSSimulatorCharacter* Target, FVector HitFrom, FName BoneName, FVector HitOffset);
	/* Pause menu */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "UI")
	void TogglePauseMenu();
	/* Setter */
	UFUNCTION(BlueprintCallable, Category = "Setting")
	void SetFOV(float NewFOV);
	UFUNCTION(Server, Reliable, WithValidation)
	void RequestSetLagCompMethod(ELagCompMethod NewLagCompMethod);
	UFUNCTION()
	void SwitchLagCompMethod();
	UFUNCTION(BlueprintCallable, Category = "Setting")
	void SetMouseSensitivity(float NewMouseSensitivity);
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Setting")
	void SetMinimapSize(int32 NewMinimapSize);
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Setting")
	void SetMinimapZoomLevel(float NewMinimapZoomLevel);
	UFUNCTION(BlueprintCallable, Category = "Setting")
	void SetPlanarReflectionLevel(EPlanarReflectionLevel NewPlanarReflectionLevel);
	UFUNCTION(BlueprintCallable, Category = "Setting")
	void SetKeyForAction(FName ActionName, FKey Key);
	UFUNCTION(BlueprintCallable, Category = "Setting")
	void UnsetKeyForAction(FName ActionName);

	UPROPERTY(BlueprintReadOnly, Category = "UI")
	ADynamicTextureManager* UIBlurAlphaMask = nullptr;
	UPROPERTY(BlueprintReadOnly, Category = "UI")
	ADynamicTextureManager* UIBlurColor = nullptr;
	bool bIsSpectator = false;
	/* Delegate */
	UPROPERTY(BlueprintAssignable)
	FOnPlanarReflectionLevelUpdatedDelegate OnPlanarReflecitonLevelUpdatedDelegate;

protected:
	void SetupInputComponent() override;
	
	UWorld* World;
	UFPSSimulatorGameInstance* GameInstance;
	AFPSSimulatorGameMode* GameMode;
	AFPSSimulatorGameState* GameState;
	/* Setting */
	UPROPERTY(Config)
	float FieldOfView = 90;
	UPROPERTY(Config)
	float MouseSensitivity = 0.07;
	UPROPERTY(BlueprintReadOnly, Config)
	int32 MinimapSize = 300;
	UPROPERTY(BlueprintReadOnly, Config)
	float MinimapZoomLevel = 0.5;
	UPROPERTY(BlueprintReadOnly)
	EPlanarReflectionLevel PlanarReflectionLevel = EPlanarReflectionLevel::All;
	UPROPERTY(BlueprintReadOnly, Config)
	int32 PlanarReflectionLevelValue = (uint8)EPlanarReflectionLevel::All;
	UPROPERTY(BlueprintAssignable)
	FOnFOVChangedDelegate OnFOVChangedDelegate;
	UPROPERTY(BlueprintAssignable)
	FOnKeyBindedDelegate OnKeyBindedDelegate;
	UPROPERTY(BlueprintAssignable)
	FOnKeyUnbindedDelegate OnKeyUnbindedDelegate;
	/* UI */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class AFPSSimulatorHUD> HUDClass;
	UPROPERTY(BlueprintAssignable)
	FUIBlurMaskInitedDelegate UIBlurMaskInitedDelegate;
	UPROPERTY(BlueprintAssignable)
	FUIBlurMaskUpdatedDelegate UIBlurMaskUpdatedDelegate;
	UPROPERTY(BlueprintAssignable)
	FOnMinimapSizeUpdatedDelegate OnMinimapSizeUpdatedDelegate;
	UPROPERTY(BlueprintAssignable)
	FOnMinimapZoomLevelUpdatedDelegate OnMinimapZoomLevelUpdatedDelegate;
	int32 Height = 0;
	int32 Width = 0;

private:
	AFPSSimulatorPlayerState* PS;
};