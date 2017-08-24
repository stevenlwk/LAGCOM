#include "FPSSimulator.h"
#include "BaseWeapon.h"

ABaseWeapon::ABaseWeapon() {
}

void ABaseWeapon::BeginPlay() {
	Super::BeginPlay();
	MagSize = MaxMagSize;
}

void ABaseWeapon::Cooldown() {
	bIsCoolingDown = false;	
	OnCooldownFinished.Broadcast();
}

void ABaseWeapon::CycleFireMode() {
	switch(FireMode) {
		case EWeaponFireMode::Auto:
			FireMode = EWeaponFireMode::Single;
			break;
		case EWeaponFireMode::Single:
			FireMode = EWeaponFireMode::Auto;
	}
	OnFireModeChanged.Broadcast();
}

void ABaseWeapon::Fire() {
	bIsCoolingDown = true;
	GetWorldTimerManager().SetTimer(CooldownTimerHandle, this, &ABaseWeapon::Cooldown, 60.f / RPM);
	if(MaxMagSize != -1 && --MagSize <= 0) OnMagEmptied.Broadcast();
}

void ABaseWeapon::Reset() {
	bIsCoolingDown = false;
	bIsReloading = false;
	MagSize = MaxMagSize;
}

void ABaseWeapon::StartReloading() {
	bIsReloading = true;
	GetWorldTimerManager().SetTimer(ReloadTimerHandle, this, &ABaseWeapon::FinishReloading, ReloadTime);
	OnStartReloading.Broadcast();
}

bool ABaseWeapon::GetCanFire() {
	return !bIsCoolingDown && !bIsReloading;
}

int32 ABaseWeapon::GetDamage() {
	return Damage;
}

int32 ABaseWeapon::GetRPM() {
	return RPM;
}

void ABaseWeapon::FinishReloading() {
	bIsReloading = false;
	if(MagSize != -1) MagSize = MaxMagSize;
	OnReloadingFinished.Broadcast();
}
