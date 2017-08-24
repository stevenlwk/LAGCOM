#pragma once

#include "GameFramework/Actor.h"
#include "BaseWeapon.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStartReloadingDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMagEmptiedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnReloadingFinishedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCooldownFinishedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFireModeChangedDelegate);

UENUM(BlueprintType)
enum class EWeaponFireMode : uint8 {
	Single,
	Auto
};

UCLASS()
class FPSSIMULATOR_API ABaseWeapon : public AActor {
	GENERATED_BODY()

public:
	ABaseWeapon();
	void BeginPlay() override;
	void Cooldown();
	void CycleFireMode();
	void Fire();
	void Reset();
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void StartReloading();
	/* Getter */
	bool GetCanFire();
	int32 GetDamage();
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Weapon")
	int32 GetRPM();

	UPROPERTY(BlueprintAssignable)
	FOnStartReloadingDelegate OnStartReloading;
	UPROPERTY(BlueprintAssignable)
	FOnMagEmptiedDelegate OnMagEmptied;
	UPROPERTY(BlueprintAssignable)
	FOnReloadingFinishedDelegate OnReloadingFinished;
	UPROPERTY(BlueprintAssignable)
	FOnCooldownFinishedDelegate OnCooldownFinished;
	UPROPERTY(BlueprintAssignable)
	FOnFireModeChangedDelegate OnFireModeChanged;
	UPROPERTY(BlueprintReadOnly)
	bool bIsReloading = false;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	EWeaponFireMode FireMode = EWeaponFireMode::Auto;
	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	int32 MagSize = -1;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	int32 MaxMagSize = -1;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	float ReloadTime = 1.5;
	/* Bullet spread */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float ADSSpreadStand = 0;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float ADSSpreadMove = 0;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float ADSSpreadCrouch = 0;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float HipSpreadStand = 0;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float HipSpreadMove = 0;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float HipSpreadCrouch = 0;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float SpreadIncrease = 0;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float SpreadDecrease = 0;
	/* Recoil */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float RecoilUp = 0;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float RecoilLeft = 0;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float RecoilRight = 0;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float RecoilDecrease = 0;

protected:
	void FinishReloading();

	FTimerHandle CooldownTimerHandle;
	FTimerHandle ReloadTimerHandle;
	bool bIsCoolingDown = false;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	int32 Damage = 10;
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon")
	FText Name;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	int32 RPM = 600;
};
