#include "FPSSimulator.h"
#include "FPSSimulatorCharacter.h"
#include "FPSSimulatorGameMode.h"
#include "FPSSimulatorPlayerController.h"
#include "FPSSimulatorPlayerState.h"
#include "UnrealNetwork.h"

void AFPSSimulatorCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFPSSimulatorCharacter, bIsADS);
	DOREPLIFETIME(AFPSSimulatorCharacter, bIsInvincible);
	DOREPLIFETIME(AFPSSimulatorCharacter, bIsPullingTrigger);
	DOREPLIFETIME(AFPSSimulatorCharacter, bIsRunning);
	DOREPLIFETIME_CONDITION(AFPSSimulatorCharacter, AimPitch, COND_SkipOwner);
	DOREPLIFETIME(AFPSSimulatorCharacter, TeamID);
	DOREPLIFETIME(AFPSSimulatorCharacter, Health);
	DOREPLIFETIME(AFPSSimulatorCharacter, bIsRespawning);
}

AFPSSimulatorCharacter::AFPSSimulatorCharacter(const FObjectInitializer& ObjectInitializer) {
}

void AFPSSimulatorCharacter::BeginPlay() {
	Super::BeginPlay();
	UCharacterMovementComponent* CharacterMovement = GetCharacterMovement();

	GameState = GetWorld()->GetGameState<AFPSSimulatorGameState>();
	GameState->OnStateChanged.AddDynamic(this, &AFPSSimulatorCharacter::OnGameStateChanged);
	PC = Cast<AFPSSimulatorPlayerController>(GetController());
	Health = GameState->MatchConfig.MaxHealth;
	WalkSpeed = GameState->MatchConfig.WalkSpeed;
	WalkSpeedMultiplier = WalkSpeed / CharacterMovement->MaxWalkSpeed;
	WalkSpeedCrouchedMultiplier = WalkSpeedCrouched / CharacterMovement->MaxWalkSpeedCrouched;
	// Weapon
	FActorSpawnParameters WeaponSpawnParameters;

	WeaponSpawnParameters.Owner = this;
	Weapon = GetWorld()->SpawnActor<ABaseWeapon>(WeaponClass, WeaponSpawnParameters);

	if(HasAuthority()) {
		TArray<USkeletalMeshComponent*> Components;

		GameMode = GetWorld()->GetAuthGameMode<AFPSSimulatorGameMode>();
		GetMesh()->MeshComponentUpdateFlag = EMeshComponentUpdateFlag::AlwaysTickPoseAndRefreshBones;
		FrameHistory = TArray<FLagCompFrame>();
		FrameHistory.Reserve(MaxFrameHistoryTime);
		GetComponents<USkeletalMeshComponent>(Components);
		for(int32 i = 0; i < Components.Num(); i++) {
			FString Name;

			Components[i]->GetName(Name);
			if(Name == "LagCompMesh") {
				LagCompMesh = Components[i];
				break;
			}
		}
	} else {
		if(IsLocallyControlled()) {
			if(GConfig) {
				GConfig->GetBool(TEXT("/Game/Blueprints/FPSSimulatorCharacter_BP.FPSSimulatorCharacter_BP_C"), TEXT("HoldToCrouch"), bHoldToCrouch, GGameIni);
			}
			Weapon->OnStartReloading.AddDynamic(this, &AFPSSimulatorCharacter::ServerCallOnReload);
			Weapon->OnMagEmptied.AddDynamic(this, &AFPSSimulatorCharacter::SetReloadPending);
			Weapon->OnCooldownFinished.AddDynamic(this, &AFPSSimulatorCharacter::OnWeaponCooldownFinished);
			Weapon->OnReloadingFinished.AddDynamic(this, &AFPSSimulatorCharacter::OnWeaponReloadingFinished);
			Weapon->OnFireModeChanged.AddDynamic(this, &AFPSSimulatorCharacter::OnWeaponFireModeChanged);
		}
		Weapon->SetActorHiddenInGame(true);
		SetWeapon();
	}
}

void AFPSSimulatorCharacter::SetupPlayerInputComponent(class UInputComponent* InInputComponent) {
	Super::SetupPlayerInputComponent(InInputComponent);
	InputComponent->BindAxis("LookYaw", this, &AFPSSimulatorCharacter::LookYaw);
	InputComponent->BindAxis("LookPitch", this, &AFPSSimulatorCharacter::LookPitch);
	InputComponent->BindAxis("MoveForward", this, &AFPSSimulatorCharacter::MoveForward);
	InputComponent->BindAxis("Strafe", this, &AFPSSimulatorCharacter::Strafe);
	InputComponent->BindAction("Crouch", IE_Pressed, this, &AFPSSimulatorCharacter::OnCrouchPressed);
	InputComponent->BindAction("Crouch", IE_Released, this, &AFPSSimulatorCharacter::OnCrouchReleased);
	InputComponent->BindAction("Sprint", IE_Pressed, this, &AFPSSimulatorCharacter::SetWantsToRun);
	InputComponent->BindAction("Sprint", IE_Released, this, &AFPSSimulatorCharacter::UnsetWantsToRun);
	InputComponent->BindAction("Aim", IE_Pressed, this, &AFPSSimulatorCharacter::StartADS);
	InputComponent->BindAction("Aim", IE_Released, this, &AFPSSimulatorCharacter::StopADS);
	InputComponent->BindAction("Fire", IE_Pressed, this, &AFPSSimulatorCharacter::PullTrigger);
	InputComponent->BindAction("Fire", IE_Released, this, &AFPSSimulatorCharacter::ReleaseTrigger);
	InputComponent->BindAction("Reload", IE_Pressed, this, &AFPSSimulatorCharacter::Reload);
	InputComponent->BindAction("CycleFireMode", IE_Pressed, this, &AFPSSimulatorCharacter::CycleFireMode);
	//InputComponent->BindAction("RespawnFaster", IE_Pressed, this, &AFPSSimulatorCharacter::RespawnFaster);
	InputComponent->BindAction("AutoMoveForward", IE_Released, this, &AFPSSimulatorCharacter::ToggleAutoMovingForward);
	InputComponent->BindAction("AutoMoveBackward", IE_Released, this, &AFPSSimulatorCharacter::ToggleAutoMovingBackward);
	InputComponent->BindAction("AutoMoveLeft", IE_Released, this, &AFPSSimulatorCharacter::ToggleAutoMovingLeft);
	InputComponent->BindAction("AutoMoveRight", IE_Released, this, &AFPSSimulatorCharacter::ToggleAutoMovingRight);
	InputComponent->BindAction("ReportSBC", IE_Released, this, &AFPSSimulatorCharacter::ReportShotBehindCovers);
}

void AFPSSimulatorCharacter::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);
	if(bIsRespawning) RemainingRespawnTime -= DeltaSeconds;
	if(HasAuthority()) {
		if(!PS) {
			PS = Cast<AFPSSimulatorPlayerState>(PlayerState);
		} else {
			FVector CurrentLocation = GetActorLocation();
			float Dist = FVector::Dist(LastLocation, CurrentLocation);

			if(Dist <= GetCharacterMovement()->MaxWalkSpeed)
				PS->AddDistancesTravelled(FVector::Dist(LastLocation, CurrentLocation));
			LastLocation = CurrentLocation;
		}
		// Lag compensation
		AddFrame(FLagCompFrame(this));
		// Respawn
		if(bIsRespawning && RemainingRespawnTime <= 0) Respawn();
	} else {
		// Movement
		if(!bIsRunning && bWantsToRun) {
			StartRunning();
		} else if(bIsRunning && GetVelocity().IsZero()) {
			StopRunning();
		}
		if(AutoMoveForward != 0.f)
			MoveForward(AutoMoveForward);
		if(AutoMoveLeft != 0.f)
			Strafe(AutoMoveLeft);
	}
	// Recoil
	if(IsLocallyControlled()) {
		if(Weapon && RecoilPitch != TargetRecoilPitch) {
			RecoilPitch = FMath::FInterpConstantTo(RecoilPitch, TargetRecoilPitch, DeltaSeconds, bIsRecoilResetting ? Weapon->RecoilDecrease : RecoilIncreaseSpeed);
			AddPitchInput(OriginalAimPitch + RecoilPitch - AimPitch);
		}
	}
	// Spread
	if(Weapon && bIsRecoilResetting) Spread = FMath::FInterpConstantTo(Spread, 0, DeltaSeconds, Weapon->SpreadDecrease);
	TotalSpread = GetBaseSpread() + Spread;
}

void AFPSSimulatorCharacter::PossessedBy(AController* NewController) {
	Super::PossessedBy(NewController);
	PC = Cast<AFPSSimulatorPlayerController>(NewController);
}

void AFPSSimulatorCharacter::MoveForward(float Val) {
	if(Val != 0.0f) {
		FRotator Rotation = GetControlRotation();

		if(GetCharacterMovement()->IsMovingOnGround() || GetCharacterMovement()->IsFalling()) {
			Rotation.Pitch = 0.0f;
		}
		const FVector Direction = FRotationMatrix(Rotation).GetScaledAxis(EAxis::X);

		if(bIsRunning && Val < 0) StopRunning();
		AddMovementInput(Direction, bIsRunning ? Val : Val*(bIsCrouched ? WalkSpeedCrouchedMultiplier : WalkSpeedMultiplier));
	}
}

void AFPSSimulatorCharacter::Strafe(float Val) {
	if(Controller != NULL && Val != 0.0f) {
		const FRotator Rotation = Controller->GetControlRotation();
		const FVector Direction = FRotationMatrix(Rotation).GetScaledAxis(EAxis::Y);

		if(bIsRunning) StopRunning();
		AddMovementInput(Direction, Val*(bIsCrouched ? WalkSpeedCrouchedMultiplier : WalkSpeedMultiplier));
	}
}

void AFPSSimulatorCharacter::StartADS() {
	if(Health>0) {
		bIsADS = true;
		if(Role < ROLE_Authority) {
			if(bIsRunning) StopRunning();
			OnADSStarted();
			ServerStartADS();
		}
	}
}

void AFPSSimulatorCharacter::ServerStartADS_Implementation() {
	StartADS();
}

bool AFPSSimulatorCharacter::ServerStartADS_Validate() {
	return true;
}

void AFPSSimulatorCharacter::StopADS() {
	if(bIsADS) {
		bIsADS = false;
		OnADSEnded();
		ServerStopADS();
	}
}

void AFPSSimulatorCharacter::OnCrouchPressed() {
	if(bHoldToCrouch || CanCrouch()) {
		Crouch();
	} else {
		UnCrouch();
	}
}

void AFPSSimulatorCharacter::OnCrouchReleased() {
	if(bHoldToCrouch && !CanCrouch()) {
		UnCrouch();
	}
}

void AFPSSimulatorCharacter::ServerStopADS_Implementation() {
	bIsADS = false;
}

bool AFPSSimulatorCharacter::ServerStopADS_Validate() {
	return true;
}

bool AFPSSimulatorCharacter::GetIsMovingFoward() {
	return GetInputAxisValue("MoveForward") > 0 && GetInputAxisValue("Strafe") == 0;
}

void AFPSSimulatorCharacter::StartRunning() {
	if(!bIsRunning && !bIsADS && !bIsPullingTrigger && (HasAuthority() || GetIsMovingFoward())) {
		bIsRunning = true;
		if(!CanCrouch()) UnCrouch();
		if(Role < ROLE_Authority) {
			OnRunningStarted();
			ServerStartRunning();
		}
	}
}

void AFPSSimulatorCharacter::ServerStartRunning_Implementation() {
	StartRunning();
}

bool AFPSSimulatorCharacter::ServerStartRunning_Validate() {
	return true;
}

void AFPSSimulatorCharacter::StopRunning() {
	if(bIsRunning) {
		bIsRunning = false;
		if(Role < ROLE_Authority) {
			OnRunningEnded();
			ServerStopRunning();
		}
	}
}

void AFPSSimulatorCharacter::ServerStopRunning_Implementation() {
	StopRunning();
}

bool AFPSSimulatorCharacter::ServerStopRunning_Validate() {
	return true;
}

void AFPSSimulatorCharacter::ConfirmHit_Implementation(AFPSSimulatorCharacter* Target, FVector HitFrom, FName BoneName, FVector HitOffset) {
	if(GameMode->GetIsDebugMode())
		PC->ClientDebugMessage("Server confirmed hit");
	AFPSSimulatorPlayerState* PS = Cast<AFPSSimulatorPlayerState>(PlayerState);
	AFPSSimulatorPlayerState* TargetPS = Cast<AFPSSimulatorPlayerState>(Target->PlayerState);

	ShowHitMarker();
	Target->OnConfirmHit(HitFrom);
	Target->ReceiveDamage(Weapon->GetDamage(), this);
	// Stats
	if(PS) PS->AddNumShotsHit();
	if(TargetPS) {
		TargetPS->AddNumShotsHitBy();
	}
}

bool AFPSSimulatorCharacter::ConfirmHit_Validate(AFPSSimulatorCharacter* Target, FVector HitFrom, FName BoneName, FVector HitOffset) {
	return true;
}

void AFPSSimulatorCharacter::OnConfirmHit_Implementation(FVector HitFrom) {
	OnConfirmHit_BP(HitFrom);
}

void AFPSSimulatorCharacter::OnDied_Implementation(AFPSSimulatorCharacter* Killer) {
	bIsRespawning = true;
	RemainingRespawnTime = DefaultRespawnTime;
	if(!HasAuthority()) {
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		GetMesh()->SetSimulatePhysics(true);
		OnDied_BP(Killer);
		if(IsLocallyControlled()) {
			AFPSSimulatorPlayerController* PC = Cast<AFPSSimulatorPlayerController>(GetController());

			if(bIsADS) {
				StopADS();
			}
			if(PC) {
				PC->OnDied(Killer);
			}
		}
	} else {
		RespawnStartTime = FDateTime::UtcNow();
	}
}

void AFPSSimulatorCharacter::Respawn_Implementation() {
	bIsRespawning = false;
	if(Weapon)
		Weapon->Reset();
	if(HasAuthority()) {
		FTimespan RespawnTime = FDateTime::UtcNow() - RespawnStartTime;

		Health = GameState->MatchConfig.MaxHealth;
		StartInvincibility();
		GetWorldTimerManager().SetTimer(InvincibilityTimer, this, &AFPSSimulatorCharacter::EndInvincibility, RespawnProtectionDuration);
		GameMode->Respawn(this);
		OnRespawnedDelegate.Broadcast();
	} else {
		GetMesh()->SetSimulatePhysics(false);
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		GetMesh()->AttachToComponent(this->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		if(IsLocallyControlled()) {
			AFPSSimulatorPlayerController* PC = Cast<AFPSSimulatorPlayerController>(GetController());
			
			OnRespawned();
			if(PC) {
				PC->OnRespawned();
			}
		}
	}
}

void AFPSSimulatorCharacter::RequestRespawn_Implementation() {
	GameMode->Respawn(this);
}

bool AFPSSimulatorCharacter::RequestRespawn_Validate() {
	return true;
}

float AFPSSimulatorCharacter::GetHealth() {
	return Health;
}

bool AFPSSimulatorCharacter::GetIsReloading() {
	return Weapon && Weapon->bIsReloading;
}

int32 AFPSSimulatorCharacter::GetTeamID() {
	return TeamID;
}

void AFPSSimulatorCharacter::SetHoldToCrouch(bool NewHoldToCrouch) {
	bHoldToCrouch = NewHoldToCrouch;
	SaveConfig();
}

void AFPSSimulatorCharacter::SetTeamID(int32 TeamID) {
	this->TeamID = TeamID;
}

void AFPSSimulatorCharacter::CallOnReload_Implementation() {
	OnReload.Broadcast();
}

void AFPSSimulatorCharacter::ServerCallOnReload_Implementation() {
	CallOnReload();
}

bool AFPSSimulatorCharacter::ServerCallOnReload_Validate() {
	return true;
}

void AFPSSimulatorCharacter::OnFired_Implementation(FHitResult HitResult, bool bIsReplicated /*= false*/) {
	if(Role < ROLE_Authority) {
		if(!IsLocallyControlled() || !bIsReplicated) {
			OnFireDelegate.Broadcast(HitResult);
		}
	}
}

void AFPSSimulatorCharacter::OnReceiveDamage_Implementation(AFPSSimulatorCharacter* DamageCauser, FName BoneName, FVector ImpactOffset, bool bShouldRespond) {
	FVector Start = DamageCauser->GetPawnViewLocation();
	FVector End = GetMesh()->GetBoneLocation(BoneName) + ImpactOffset;
	FCollisionQueryParams CollisionQueryParams;
	FHitResult SACTest(ForceInit);

	// Check for "shot around a corner"
	CollisionQueryParams.AddIgnoredActor(this);
	CollisionQueryParams.AddIgnoredActor(DamageCauser);
	if(GameState->bShowTraceLines) DrawDebugLine(GetWorld(), Start, End, FColor::Red, true, -1.f, 0, 1.f);
	if(GetWorld()->LineTraceSingleByChannel(SACTest, Start, End, ECC_Visibility, CollisionQueryParams)) {
		switch(GameState->GetLagCompMethod()) {
			case ELagCompMethod::Normal:
				DenyHit(DamageCauser, Start, BoneName, ImpactOffset);
				break;
			case ELagCompMethod::Advanced:
				if(bShouldRespond) DenyHit(DamageCauser, Start, BoneName, ImpactOffset);
				break;
		}
	} else if(bShouldRespond) {
		PC->ConfirmHit(DamageCauser, this, Start, BoneName, ImpactOffset);
	}
}

void AFPSSimulatorCharacter::AddFrame(FLagCompFrame Frame) {
	if(FrameHistory.Num() == MaxFrameHistoryTime) {
		FrameHistory.RemoveAt(0);
	}
	FrameHistory.Add(Frame);
}

void AFPSSimulatorCharacter::OnGameStateChanged(EMatchState NewState) {
	switch(NewState) {
		case EMatchState::PostMatch:
		case EMatchState::PostMatchCountdown:
			if(PC) PC->SetIgnoreMoveInput(true);
			bIsADS = false;
			bIsPullingTrigger = false;
			bIsRunning = false;
			bWantsToRun = false;
			AutoMoveForward = AutoMoveLeft = 0.f;
	}
}

void AFPSSimulatorCharacter::AddPitchInput(float Val) {
	if(PC) PC->AddPitchInput(Val);
	if(Role < ROLE_Authority) ServerAddPitchInput(Val);
	AimPitch = FMath::Clamp<float>(AimPitch + Val, -90, 90);
}

void AFPSSimulatorCharacter::ServerAddPitchInput_Implementation(float Val) {
	AddPitchInput(Val);
}

bool AFPSSimulatorCharacter::ServerAddPitchInput_Validate(float Val) {
	return true;
}

void AFPSSimulatorCharacter::LookYaw(float Val) {
	if(PC) {
		PC->AddYawInput(Val);
		if(Role < ROLE_Authority) ServerLookYaw(Val);
	}
}

void AFPSSimulatorCharacter::ServerLookYaw_Implementation(float Val) {
	LookYaw(Val);
}

bool AFPSSimulatorCharacter::ServerLookYaw_Validate(float Val) {
	return true;
}

void AFPSSimulatorCharacter::LookPitch(float Val) {
	if(Val != 0) {
		AddPitchInput(Val);
		OriginalAimPitch = AimPitch;
		RecoilPitch = TargetRecoilPitch = 0;
	}
}

void AFPSSimulatorCharacter::SetWantsToRun() {
	if(bAllowSprint) bWantsToRun = true;
}

void AFPSSimulatorCharacter::UnsetWantsToRun() {
	bWantsToRun = false;
	StopRunning();
}

void AFPSSimulatorCharacter::StartInvincibility_Implementation() {
	if(Role == ROLE_Authority) bIsInvincible = true;
	OnStartInvincibilityDelegate.Broadcast();
}

void AFPSSimulatorCharacter::EndInvincibility_Implementation() {
	if(Role == ROLE_Authority) bIsInvincible = false;
	OnEndInvincibilityDelegate.Broadcast();
}

void AFPSSimulatorCharacter::ToggleAutoMovingForward() {
	AutoMoveForward = AutoMoveForward == 1.f ? 0.f : 1.f;
}

void AFPSSimulatorCharacter::ToggleAutoMovingBackward() {
	AutoMoveForward = AutoMoveForward == -1.f ? 0.f : -1.f;
}

void AFPSSimulatorCharacter::ToggleAutoMovingLeft() {
	AutoMoveLeft = AutoMoveLeft == -1.f ? 0.f : -1.f;
}

void AFPSSimulatorCharacter::ToggleAutoMovingRight() {
	AutoMoveLeft = AutoMoveLeft == 1.f ? 0.f : 1.f;
}

void AFPSSimulatorCharacter::ClientDrawDebugPoint_Implementation(FVector const& Position, float Size, FColor const& PointColor, bool bPersistentLines /*= true*/, float LifeTime /*= -1.f*/) {
	DrawDebugPoint(GetWorld(), Position, Size, PointColor, bPersistentLines, LifeTime);
}

void AFPSSimulatorCharacter::ClientDrawDebugLine_Implementation(FVector const& LineStart, FVector const& LineEnd, FColor const& Color, bool bPersistentLines = false, float LifeTime = -1.f, float Thickness = 0.f) {
	DrawDebugLine(GetWorld(), LineStart, LineEnd, Color, bPersistentLines, LifeTime, 0, Thickness);
}

void AFPSSimulatorCharacter::DrawHitbox_Implementation(const FBox& Box, const FQuat& Rotation, FColor const& Color, bool bPersistentLines /*= true*/, float LifeTime /*= -1.f*/) {
	DrawDebugBox(GetWorld(), Box.GetCenter(), Box.GetExtent(), Color, bPersistentLines, LifeTime);
}

void AFPSSimulatorCharacter::DrawHitboxes_Implementation(const TArray<FBox>& Boxes, const FQuat& Rotation, FColor const& Color, bool bPersistentLines /*= true*/, float LifeTime /*= -1.f*/) {
	for(int32 i = 0; i < Boxes.Num(); i++) {
		DrawDebugBox(GetWorld(), Boxes[i].GetCenter(), Boxes[i].GetExtent(), Color, bPersistentLines, LifeTime);
	}
}

void AFPSSimulatorCharacter::ShowHitMarker_Implementation() {
	ShowHitMarker_BP();
}

void AFPSSimulatorCharacter::RespawnFaster() {
	if(bIsRespawning) {
		RemainingRespawnTime -= RespawnTimeReductionPerClick;
		if(!HasAuthority()) {
			ServerRespawnFaster();
			OnRespawnFasterDelegate.Broadcast();
		} else if(RemainingRespawnTime <= 0) {
			Respawn();
		}
	}
}

void AFPSSimulatorCharacter::ReportShotBehindCovers_Implementation() {
	if(PS)
		PS->AddNumShotsBehindCoversReported();
}

bool AFPSSimulatorCharacter::ReportShotBehindCovers_Validate() {
	return true;
}

void AFPSSimulatorCharacter::ServerRespawnFaster_Implementation() {
	RespawnFaster();
}

bool AFPSSimulatorCharacter::ServerRespawnFaster_Validate(){
	return true;
}

void AFPSSimulatorCharacter::CycleFireMode() {
	if(Weapon)
		Weapon->CycleFireMode();
}

void AFPSSimulatorCharacter::PullTrigger() {
	if(Weapon && Weapon->GetCanFire() && Health > 0) {
		if(bIsRunning) StopRunning();
		bIsPullingTrigger = true;
		OriginalAimPitch = AimPitch;
		RecoilPitch = TargetRecoilPitch = 0;
		Fire();
		/*if(Weapon->FireMode == EWeaponFireMode::Auto) {
			GetWorldTimerManager().SetTimer(FireTimerHandle, this, &AFPSSimulatorCharacter::Fire, 60.f / Weapon->GetRPM(), true);
		}*/
		OnPullTrigger.Broadcast();
	}
}

void AFPSSimulatorCharacter::ReleaseTrigger() {
	bIsPullingTrigger = false;
	//GetWorldTimerManager().ClearTimer(FireTimerHandle);
	OnReleaseTrigger.Broadcast();
}

void AFPSSimulatorCharacter::Reload() {
	if(Weapon && Weapon->MagSize < Weapon->MaxMagSize && Weapon->MaxMagSize!=-1 && !Weapon->bIsReloading) {
		Weapon->StartReloading();
		if(Role < ROLE_Authority) {
			ServerReload();
			bIsReloadPending = false;
		}
	}
}

void AFPSSimulatorCharacter::SetReloadPending() {
	bIsReloadPending = true;
}

void AFPSSimulatorCharacter::ServerReload_Implementation() {
	Reload();
}

bool AFPSSimulatorCharacter::ServerReload_Validate() {
	return true;
}

void AFPSSimulatorCharacter::FireWeapon() {
	if(Weapon) {
		Weapon->Fire();
		bIsRecoilResetting = false;
		// Recoil
		if(IsLocallyControlled()) {
			TargetRecoilPitch += Weapon->RecoilUp;;
			RecoilIncreaseSpeed = (TargetRecoilPitch - RecoilPitch) / 0.14;
			LookYaw(FMath::FRandRange(-Weapon->RecoilLeft, Weapon->RecoilRight));
		}
		// Spread
		Spread += Weapon->SpreadIncrease;
		GetWorldTimerManager().SetTimer(SpreadResetTimerHandle, this, &AFPSSimulatorCharacter::ResetSpread, 0.14);
	}
}

void AFPSSimulatorCharacter::Fire() {
	if(Health > 0 && !bIsRunning && Weapon) {
		if(Weapon->MagSize > 0 || Weapon->MagSize == -1) {
			int32 Seed = FMath::Rand();
			FRandomStream RandomStream = FRandomStream(Seed);
			FVector Start = GetPawnViewLocation();
			FVector Spread = RandomStream.VRandCone(GetBaseAimRotation().Vector(), FMath::DegreesToRadians(this->TotalSpread));
			FVector End = Start + Spread * GameState->TraceDistance;
			FCollisionQueryParams TraceParams = FCollisionQueryParams();
			FHitResult HitResult(ForceInit);

			FireWeapon();
			ServerFire(Seed);
			TraceParams.AddIgnoredActor(this);
			GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECollisionChannel::ECC_GameTraceChannel1, TraceParams);
			RepFire(HitResult);
			OnFired(HitResult);
		} else {
			//ReleaseTrigger();
		}
	}
}

void AFPSSimulatorCharacter::ServerFire_Implementation(int32 SpreadSeed) {
	AFPSSimulatorPlayerState* PS = Cast<AFPSSimulatorPlayerState>(PlayerState);
	// Trace
	FRandomStream RandomStream = FRandomStream(SpreadSeed);
	FVector Start = GetPawnViewLocation();
	FVector Spread = RandomStream.VRandCone(GetBaseAimRotation().Vector(), FMath::DegreesToRadians(this->TotalSpread));
	FVector End = Start + Spread * GameState->TraceDistance;
	FCollisionQueryParams TraceParams;
	FHitResult HitResult(ForceInit);
	// Result
	float RolledBackDist = 0;

	FireWeapon();
	if(PS) PS->AddNumShotsFired();
	if(GameState->bShowTraceLines) ClientDrawDebugLine(Start, End, FColor::Red, true, -1.f, 1.f);
	switch(GameState->GetLagCompMethod()) {
		case ELagCompMethod::None:
			TraceParams = FCollisionQueryParams();
			TraceParams.AddIgnoredActor(this);
			if(GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_GameTraceChannel1, TraceParams) &&
				HitResult.GetActor()->GetClass()->IsChildOf(AFPSSimulatorCharacter::StaticClass())) {
				AFPSSimulatorCharacter* Target = Cast<AFPSSimulatorCharacter>(HitResult.GetActor());

				if(Target->GetHealth() > 0) {
					AFPSSimulatorPlayerState* TargetPS = Cast<AFPSSimulatorPlayerState>(Target->PlayerState);

					ConfirmHit(Target, Start, HitResult.BoneName, HitResult.ImpactPoint - Target->GetMesh()->GetBoneLocation(HitResult.BoneName));
					GameMode->LogFireEvent(EFireLogEventType::ServerConfirm, GameState->GetLagCompMethod(), PS, TargetPS, Start, HitResult.ImpactPoint, Target->GetHealth());
				}
				if(GameMode->bShowHitboxes) {
					USkeletalMeshComponent* Mesh = Target->GetMesh();
					TArray<FBox> HitBoxes = TArray<FBox>();

					for(int32 i = 0; i < Mesh->Bodies.Num(); i++) {
						FBodyInstance* BodyInstance = Mesh->Bodies[i];

						if(BodyInstance)
							HitBoxes.Add(BodyInstance->GetBodyBounds());
					}
					DrawHitboxes(HitBoxes, FQuat::Identity, FColor::Green);
				}
			}
			break;
		case ELagCompMethod::Normal:
		case ELagCompMethod::Advanced:
			int32 FrameIndex = FMath::Min(FrameHistory.Num() - 1, MaxFrameHistoryTime - FMath::RoundToInt(PlayerState->ExactPing / (1000.f / GameMode->GetTickRate())) - 1);

			TraceParams = FCollisionQueryParams();
			TraceParams.AddIgnoredActor(this);
			for(TActorIterator<AFPSSimulatorCharacter> ActorItr(GetWorld()); ActorItr; ++ActorItr) {
				AFPSSimulatorCharacter* Target = *ActorItr;

				if(Target == this) continue;
				if(Target->FrameHistory.Num() > FrameIndex) {
					FLagCompFrame Frame = Target->FrameHistory[FrameIndex];

					TraceParams.AddIgnoredComponent(Target->GetMesh());
					if(Target->LagCompMesh) {
						TArray<FBox> HitBoxes = TArray<FBox>();

						for(auto It = Frame.BoneTransforms.CreateConstIterator(); It; ++It) {
							FBodyInstance* BodyInstance = Target->LagCompMesh->GetBodyInstance(It.Key());

							if(BodyInstance) {
								BodyInstance->SetBodyTransform(It.Value(), ETeleportType::TeleportPhysics);
								HitBoxes.Add(BodyInstance->GetBodyBounds());
							}
						}
						if(GameMode->bShowHitboxes)
							DrawHitboxes(HitBoxes, FQuat::Identity, FColor::Green);
					}
				}
			}
			if(GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_GameTraceChannel2, TraceParams) &&
				HitResult.GetActor()->GetClass()->IsChildOf(AFPSSimulatorCharacter::StaticClass())) {
				AFPSSimulatorCharacter* Target = Cast<AFPSSimulatorCharacter>(HitResult.GetActor());
				if(TeamID == Target->GetTeamID() && !GameMode->bFriendlyFire || Target->GetHealth() <= 0 || Target->bIsInvincible) {
					break;
				}
				AFPSSimulatorPlayerState* TargetPS = Cast<AFPSSimulatorPlayerState>(Target->PlayerState);
				FVector ImpactOffset = HitResult.ImpactPoint - Target->LagCompMesh->GetBodyInstance(HitResult.BoneName)->GetBodyBounds().GetCenter();
				bool bShouldRespond = GameState->GetLagCompMethod() == ELagCompMethod::Advanced &&
					PlayerState->ExactPing >= GameState->GetALCHighPing() &&
					TargetPS->ExactPing <= GameState->GetALCLowPing();

				if(GameMode->GetIsDebugMode())
					PC->ClientDebugMessage("Server detected hit");
				RolledBackDist = FVector::Dist(Target->FrameHistory[FrameIndex].ViewLocation, Target->FrameHistory[Target->FrameHistory.Num() - 1].ViewLocation);
				if(!bShouldRespond) {
					ConfirmHit(Target, Start, HitResult.BoneName, ImpactOffset);
					GameMode->LogFireEvent(EFireLogEventType::ServerConfirm, GameState->GetLagCompMethod(), PS, TargetPS, Start, HitResult.ImpactPoint, Target->GetHealth(), RolledBackDist);
				} else {
					GameMode->LogFireEvent(EFireLogEventType::ServerDetect, GameState->GetLagCompMethod(), PS, TargetPS, Start, HitResult.ImpactPoint, Target->GetHealth(), RolledBackDist);
				}
				Target->OnReceiveDamage(this, HitResult.BoneName, ImpactOffset, bShouldRespond);
			}
	}
}

bool AFPSSimulatorCharacter::ServerFire_Validate(int32 SpreadSeed) {
	return true;
}

void AFPSSimulatorCharacter::RepFire_Implementation(FHitResult HitResult) {
	if(Health > 0) {
		OnFired(HitResult, true);
	}
}

bool AFPSSimulatorCharacter::RepFire_Validate(FHitResult HitResult) {
	return true;
}

void AFPSSimulatorCharacter::DenyHit_Implementation(AFPSSimulatorCharacter* DamageCauser, FVector HitStart, FName BoneName, FVector HitOffset) {
	AFPSSimulatorPlayerState* PS = Cast<AFPSSimulatorPlayerState>(PlayerState);
	AFPSSimulatorPlayerState* TargetPS = Cast<AFPSSimulatorPlayerState>(DamageCauser->PlayerState);
	AFPSSimulatorPlayerController* DamageCauserPC = Cast<AFPSSimulatorPlayerController>(DamageCauser->GetController());

	if(GameState->GetLagCompMethod() == ELagCompMethod::Normal) {
		PS->AddNumShotsBehindCovers();
		if(GameMode->GetIsDebugMode())
			DamageCauserPC->ClientDebugMessage("Opponent detected SAC");
		GameMode->LogFireEvent(EFireLogEventType::ClientSAC, GameState->GetLagCompMethod(), TargetPS, PS, HitStart, GetMesh()->GetBoneLocation(BoneName) + HitOffset, Health);
		return;
	}

	FHitResult HitResult(ForceInit);
	FCollisionQueryParams TraceParams = FCollisionQueryParams();
	
	if(GameMode->GetIsDebugMode())
		DamageCauserPC->ClientDebugMessage("Opponent detected SAC and denied the hit");
	GameMode->LogFireEvent(EFireLogEventType::ClientDeny, GameState->GetLagCompMethod(), TargetPS, PS, HitStart, GetMesh()->GetBoneLocation(BoneName) + HitOffset, Health);
	PS->AddNumShotsDenied();
	TraceParams.AddIgnoredActor(this);
	TraceParams.AddIgnoredActor(DamageCauser);
	if(GetWorld()->LineTraceSingleByChannel(HitResult, HitStart, GetMesh()->GetBoneLocation(BoneName) + HitOffset, ECC_Visibility, TraceParams)) {
		GameMode->LogFireEvent(EFireLogEventType::ServerDeny, GameState->GetLagCompMethod(), TargetPS, PS, HitStart, GetMesh()->GetBoneLocation(BoneName) + HitOffset, Health);
		return;
	}
	if(GameMode->GetIsDebugMode())
		DamageCauserPC->ClientDebugMessage("Server overruled opponent's denial");
	DamageCauser->ConfirmHit(this, HitStart, BoneName, HitOffset);
	GameMode->LogFireEvent(EFireLogEventType::ServerOverrule, GameState->GetLagCompMethod(), TargetPS, PS, HitStart, GetMesh()->GetBoneLocation(BoneName) + HitOffset, Health);
	PS->AddNumShotsOverruled();
}

bool AFPSSimulatorCharacter::DenyHit_Validate(AFPSSimulatorCharacter* DamageCauser, FVector HitStart, FName BoneName, FVector HitOffset) {
	return true;
}

void AFPSSimulatorCharacter::OnWeaponCooldownFinished() {
	if(bIsPullingTrigger && Weapon && Weapon->FireMode == EWeaponFireMode::Auto)
		Fire();
}

void AFPSSimulatorCharacter::OnWeaponReloadingFinished() {
	if(bIsPullingTrigger && Weapon && Weapon->FireMode == EWeaponFireMode::Auto)
		Fire();
}

float AFPSSimulatorCharacter::GetBaseSpread() {
	float Spread = 0.f;

	if(Weapon) {
		if(!bIsADS) {
			Spread += bIsCrouched ? Weapon->HipSpreadCrouch : Weapon->HipSpreadStand;
			if(!GetVelocity().IsZero()) Spread += Weapon->HipSpreadMove;
		} else {
			Spread += bIsCrouched ? Weapon->ADSSpreadCrouch : Weapon->ADSSpreadStand;
			if(!GetVelocity().IsZero()) Spread += Weapon->ADSSpreadMove;
		}
	}
	return Spread;
}

void AFPSSimulatorCharacter::ResetSpread() {
	bIsRecoilResetting = true;
	TargetRecoilPitch = 0;
}

void AFPSSimulatorCharacter::ReceiveDamage_Implementation(float Damage, AFPSSimulatorCharacter* DamageCauser) {
	if(Damage >= 0) {
		Health -= Damage;
		if(Health <= 0) {
			Health = 0;
		}
		if(Health <= 0) {
			AFPSSimulatorPlayerState* PlayerState = Cast<AFPSSimulatorPlayerState>(this->PlayerState);
			AFPSSimulatorPlayerState* DamageCauserPlayerState = Cast<AFPSSimulatorPlayerState>(DamageCauser->PlayerState);
			AFPSSimulatorPlayerController* DamageCauserPlayerControntroller = Cast<AFPSSimulatorPlayerController>(DamageCauser->GetController());

			GameState->OnKilled(DamageCauserPlayerState, PlayerState);
			DamageCauserPlayerState->OnKilled(PlayerState);
			OnDied(DamageCauser);
			OnDiedDelegate.Broadcast(this, DamageCauser);
			if(DamageCauserPlayerControntroller) {
				DamageCauserPlayerControntroller->OnKilled(this, DamageCauserPlayerState->GetKillStreak());
			}
		}
	}
}

bool AFPSSimulatorCharacter::ReceiveDamage_Validate(float Val, AFPSSimulatorCharacter* DamageCauser) {
	return true;
}