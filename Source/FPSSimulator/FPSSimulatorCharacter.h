#pragma once

#include "GameFramework/Character.h"
#include "BaseWeapon.h"
#include "FPSSimulatorGameState.h"
#include "FPSSimulatorPlayerState.h"
#include "FPSSimulatorCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDiedDelegate, AFPSSimulatorCharacter*, Victim, AFPSSimulatorCharacter*, Killer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFireDelegate, FHitResult, HitResult);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPullTriggerDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnReleaseTriggerDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnReloadDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRespawnedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRespawnFasterDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStartInvincibilityDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEndInvincibilityDelegate);

class AFPSSimulatorGameMode;
class AFPSSimulatorPlayerController;

USTRUCT()
struct FLagCompFrame {
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FVector ViewLocation;
	UPROPERTY()
	FRotator ViewRotation;
	UPROPERTY()
	TMap<FName, FTransform> BoneTransforms;

	FLagCompFrame() {
		ViewLocation = FVector::ZeroVector;
		ViewRotation = FRotator::ZeroRotator;
		BoneTransforms = TMap<FName, FTransform>();
	}

	FLagCompFrame(ACharacter* Character) {
		TArray<FName> BoneNames;

		ViewLocation = Character->GetPawnViewLocation();
		ViewRotation = Character->GetBaseAimRotation();
		Character->GetMesh()->GetBoneNames(BoneNames);
		BoneTransforms = TMap<FName, FTransform>();
		for(int32 i = 0; i < BoneNames.Num(); i++) {
			BoneTransforms.Add(BoneNames[i], Character->GetMesh()->GetBoneTransform(Character->GetMesh()->GetBoneIndex(BoneNames[i])));
		}
	}
};

UCLASS(Config = Game)
class FPSSIMULATOR_API AFPSSimulatorCharacter : public ACharacter {
	GENERATED_BODY()

public:
	AFPSSimulatorCharacter(const FObjectInitializer& ObjectInitializer);
	void BeginPlay() override;
	void SetupPlayerInputComponent(class UInputComponent* InInputComponent) override;
	void Tick(float DeltaSeconds) override;
	void PossessedBy(AController* NewController) override;

	bool GetIsMovingFoward();
	void MoveForward(float Val);
	void Strafe(float Val);
	/* ADS */
	void StartADS();
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStartADS();
	void StopADS();
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStopADS();
	/* Crouch */
	void OnCrouchPressed();
	void OnCrouchReleased();
	/* Run */
	void StartRunning();
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStartRunning();
	void StopRunning();
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStopRunning();

	UFUNCTION(Server, Reliable, WithValidation)
	void ConfirmHit(AFPSSimulatorCharacter* Target, FVector HitFrom, FName BoneName, FVector HitOffset);
	UFUNCTION(Client, Reliable)
	void OnConfirmHit(FVector HitFrom);
	UFUNCTION(BlueprintImplementableEvent)
	void OnConfirmHit_BP(FVector HitFrom);
	UFUNCTION(NetMulticast, Reliable)
	void OnDied(AFPSSimulatorCharacter* Killer);
	UFUNCTION(BlueprintImplementableEvent)
	void OnDied_BP(AFPSSimulatorCharacter* Killer);
	UFUNCTION(Server, Reliable, WithValidation)
	void ReceiveDamage(float Damage, AFPSSimulatorCharacter* DamageCauser);
	UFUNCTION(Client, Reliable)
	void OnReceiveDamage(AFPSSimulatorCharacter* DamageCauser, FName BoneName, FVector ImpactOffset, bool bShouldRespond);
	UFUNCTION(NetMulticast, Reliable)
	void Respawn();
	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation, Category = "Character")
	void RequestRespawn();
	UFUNCTION(BlueprintImplementableEvent)
	void OnRespawned();
	UFUNCTION(BlueprintImplementableEvent, Category = "Weapon")
	void SetWeapon();
	/* Getter */
	float GetHealth();
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Weapon")
	bool GetIsReloading();
	int32 GetTeamID();
	/* Setter */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void SetHoldToCrouch(bool NewHoldToCrouch);
	void SetTeamID(int32 TeamID);
	/* Animation */
	UFUNCTION(NetMulticast, Unreliable)
	void CallOnReload();
	UFUNCTION(Server, Unreliable, WithValidation)
	void ServerCallOnReload();
	UFUNCTION(BlueprintImplementableEvent)
	void OnADSStarted();
	UFUNCTION(BlueprintImplementableEvent)
	void OnADSEnded();
	UFUNCTION(NetMulticast, Unreliable)
	void OnFired(FHitResult HitResult, bool bIsReplicated = false);
	UFUNCTION(BlueprintImplementableEvent)
	void OnRunningStarted();
	UFUNCTION(BlueprintImplementableEvent)
	void OnRunningEnded();
	/* Appearance */
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateTeamColor(int32 TeamID);
	/* Replication */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Replication")
	void OnRep_TeamID();
	/* Weapon */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void Reload();
	UFUNCTION()
	void SetReloadPending();

	UPROPERTY(EditAnywhere, Category = "AI")
	class UBehaviorTree* BehaviorTree;
	UPROPERTY(BlueprintAssignable)
	FOnDiedDelegate OnDiedDelegate;
	UPROPERTY(BlueprintAssignable)
	FOnRespawnedDelegate OnRespawnedDelegate;
	/* Animation */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	UAnimSequenceBase* CrouchAnimSequence;
	/* Behavior */
	UPROPERTY(EditDefaultsOnly, Category = "Behavior")
	bool bAllowSprint = true;
	UPROPERTY(BlueprintReadWrite, Config, EditDefaultsOnly, Category = "Behavior")
	bool bHoldToCrouch = true;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Behavior")
	bool bIsADS = false;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Behavior")
	bool bIsInvincible = false;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Behavior")
	bool bIsPullingTrigger = false;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Behavior")
	bool bIsRunning = false;
	bool bWantsToRun = false;
	UPROPERTY(BlueprintReadOnly, Replicated)
	float AimPitch = 0.f;
	float AutoMoveForward = 0.f;
	float AutoMoveLeft = 0.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior")
	float WalkSpeed;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior")
	float WalkSpeedCrouched;
	/* Delegate */
	UPROPERTY(BlueprintAssignable)
	FOnStartInvincibilityDelegate OnStartInvincibilityDelegate;
	UPROPERTY(BlueprintAssignable)
	FOnEndInvincibilityDelegate OnEndInvincibilityDelegate;
	/* Networking */
	TArray<FLagCompFrame> FrameHistory;
	/* Weapon */
	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	ABaseWeapon* Weapon;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<ABaseWeapon> WeaponClass;
	UPROPERTY(BlueprintAssignable)
	FOnFireDelegate OnFireDelegate;
	UPROPERTY(BlueprintAssignable)
	FOnPullTriggerDelegate OnPullTrigger;
	UPROPERTY(BlueprintAssignable)
	FOnReleaseTriggerDelegate OnReleaseTrigger;
	UPROPERTY(BlueprintAssignable)
	FOnReloadDelegate OnReload;
	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	bool bIsReloadPending = false;

protected:
	void AddFrame(FLagCompFrame Frame);
	UFUNCTION()
	void OnGameStateChanged(EMatchState NewState);
	/* Behavior */
	void AddPitchInput(float Val);
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerAddPitchInput(float Val);
	void LookYaw(float Val);
	void LookPitch(float Val);
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerLookYaw(float Val);
	void SetWantsToRun();
	void UnsetWantsToRun();
	UFUNCTION(NetMulticast, Reliable)
	void StartInvincibility();
	UFUNCTION(NetMulticast, Reliable)
	void EndInvincibility();
	void ToggleAutoMovingForward();
	void ToggleAutoMovingBackward();
	void ToggleAutoMovingLeft();
	void ToggleAutoMovingRight();
	/* Debug */
	UFUNCTION(Client, Unreliable)
	void ClientDrawDebugLine(FVector const& LineStart, FVector const& LineEnd, FColor const& Color, bool bPersistentLines /*= false*/, float LifeTime /*= -1.f*/, float Thickness /*= 0.f*/);
	UFUNCTION(Client, Unreliable)
	void ClientDrawDebugPoint(FVector const& Position, float Size, FColor const& PointColor, bool bPersistentLines /*= true*/, float LifeTime /*= -1.f*/);
	UFUNCTION(Client, Unreliable)
	void DrawHitbox(const FBox& Box, const FQuat& Rotation, FColor const& Color, bool bPersistentLines /*= true*/, float LifeTime /*= -1.f*/);
	UFUNCTION(Client, Unreliable)
	void DrawHitboxes(const TArray<FBox>& Boxes, const FQuat& Rotation, FColor const& Color, bool bPersistentLines = true, float LifeTime = -1.f);
	/* Respawn */
	void RespawnFaster();
	UFUNCTION(Server, Unreliable, WithValidation)
	void ServerRespawnFaster();
	/* Setter */
	UFUNCTION(Server, Reliable, WithValidation)
	void ReportShotBehindCovers();
	/* UI */
	UFUNCTION(Client, Reliable)
	void ShowHitMarker();
	UFUNCTION(BlueprintImplementableEvent)
	void ShowHitMarker_BP();
	/* Weapon */
	void CycleFireMode();
	void PullTrigger();
	void ReleaseTrigger();
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerReload();
	void FireWeapon();
	void Fire();
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFire(int32 SpreadSeed);
	UFUNCTION(Server, Reliable, WithValidation)
	void RepFire(FHitResult HitResult);
	UFUNCTION(Server, Reliable, WithValidation)
	void DenyHit(AFPSSimulatorCharacter* DamageCauser, FVector HitStart, FName BoneName, FVector HitOffset);
	UFUNCTION()
	void OnWeaponCooldownFinished();
	UFUNCTION()
	void OnWeaponReloadingFinished();
	UFUNCTION(BlueprintImplementableEvent)
	void OnWeaponFireModeChanged();
	// Spread
	float GetBaseSpread();
	void ResetSpread();
	
	AFPSSimulatorGameMode* GameMode;
	AFPSSimulatorPlayerController* PC;
	int32 MaxFrameHistoryTime = 90;
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_TeamID, Category = "Identification")
	int32 TeamID = -1;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Health")
	float Health;
	/* Behavior */
	float WalkSpeedMultiplier;
	float WalkSpeedCrouchedMultiplier;
	/* Debug */
	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	bool bShowImpactPoints = false;
	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	bool bShowTraceLines = false;
	/* Networking */
	USkeletalMeshComponent* LagCompMesh = NULL;
	/* Respawn */
	UPROPERTY(Replicated)
	bool bIsRespawning = false;
	UPROPERTY(BlueprintAssignable)
	FOnRespawnFasterDelegate OnRespawnFasterDelegate;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Respawn")
	float DefaultRespawnTime = 5.f;
	UPROPERTY(BlueprintReadOnly, Category = "Respawn")
	float RemainingRespawnTime;
	FDateTime RespawnStartTime;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Respawn")
	float RespawnTimeReductionPerClick = 0.2f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Respawn")
	float RespawnProtectionDuration = 5.f;
	FTimerHandle InvincibilityTimer;
	/* Weapon */
	FTimerHandle FireTimerHandle;
	bool bIsRecoilResetting = true;
	// Recoil
	float OriginalAimPitch = 0.f;
	float RecoilIncreaseSpeed;
	float RecoilPitch = 0.f;
	float TargetRecoilPitch = 0.f;
	// Spread
	FTimerHandle SpreadResetTimerHandle;
	float Spread = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	float TotalSpread = 0.f;

private:
	AFPSSimulatorGameState* GameState;
	AFPSSimulatorPlayerState* PS;
	FVector LastLocation = FVector::ZeroVector;
};