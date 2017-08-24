#include "FPSSimulator.h"
#include "CTF_Base.h"
#include "CTF_Flag.h"
#include "FPSSimulatorCharacter.h"
#include "GS_CTF.h"
#include "PS_CTF.h"
#include "UnrealNetwork.h"

ACTF_Flag::ACTF_Flag(const FObjectInitializer& ObjectInitializer) {
	Scene = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, "Scene");
	Scene->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	Mesh = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, "Area");
	Mesh->AttachToComponent(Scene, FAttachmentTransformRules::KeepRelativeTransform);
	bReplicates = true;
}

void ACTF_Flag::BeginPlay() {
	Super::BeginPlay();
	AGS_CTF* GameState = GetWorld()->GetGameState<AGS_CTF>();
	if(!GameState) {
		Destroy();
		return;
	}
	if(HasAuthority()) {
		SpawnLocation = GetActorLocation();
		this->OnActorBeginOverlap.AddDynamic(this, &ACTF_Flag::OnOverlap);
	}else{
		if(GameState) {
			if(GameState->Flags.IsValidIndex(TeamID)) {
				GameState->Flags[TeamID] = this;
			}
			GameState->CallOnFlagSpawned(this);
		}
	}
}

FVector ACTF_Flag::GetLocation() {
	return PossessedBy ? PossessedBy->GetActorLocation() : GetActorLocation();
}

void ACTF_Flag::CallOnFlagPossessed_Implementation(AFPSSimulatorCharacter* InPossessedBy) {
	OnFlagPossessedDelegate.Broadcast(this, InPossessedBy);
}

void ACTF_Flag::CallOnFlagUnpossessed_Implementation(AFPSSimulatorCharacter* WasPossessedBy) {
	OnFlagUnpossessedDelegate.Broadcast(this, WasPossessedBy);
}

void ACTF_Flag::OnFlagHolderDied(AFPSSimulatorCharacter* Victim, AFPSSimulatorCharacter* Killer) {
	FVector Location = PossessedBy->GetActorLocation();

	SetOwner(nullptr);
	WasPossessedBy = PossessedBy;
	PossessedBy = nullptr;
	WasPossessedBy->OnDiedDelegate.RemoveDynamic(this, &ACTF_Flag::OnFlagHolderDied);
	Mesh->IgnoreActorWhenMoving(WasPossessedBy, true);
	DetachFromActor(FDetachmentTransformRules::KeepRelativeTransform);
	Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Block);
	SetActorLocation(FVector(Location.X, Location.Y, SpawnLocation.Z));
	CallOnFlagUnpossessed(WasPossessedBy);
	// Score
	APS_CTF* KillerPlayerState = Cast<APS_CTF>(Killer->PlayerState);

	if(KillerPlayerState)
		KillerPlayerState->ScoreCarrierKill();
}

void ACTF_Flag::OnFlagHolderRespawned() {
	Mesh->IgnoreActorWhenMoving(WasPossessedBy, false);
	WasPossessedBy->OnRespawnedDelegate.RemoveDynamic(this, &ACTF_Flag::OnFlagHolderRespawned);
}

void ACTF_Flag::OnOverlap(AActor* OverlappedActor, AActor* OtherActor) {
	if(!PossessedBy) {
		AFPSSimulatorCharacter* Character = Cast<AFPSSimulatorCharacter>(OtherActor);

		if(Character && Character->GetTeamID() != TeamID) {
			SetOwner(Character);
			PossessedBy = Character;
			Character->OnDiedDelegate.AddDynamic(this, &ACTF_Flag::OnFlagHolderDied);
			Character->OnRespawnedDelegate.AddDynamic(this, &ACTF_Flag::OnFlagHolderRespawned);
			Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
			Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Ignore);
			AttachToComponent(Character->GetMesh(), FAttachmentTransformRules::FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), "Flag");
			CallOnFlagPossessed(Character);
			// Score
			APS_CTF* PlayerState = Cast<APS_CTF>(Character->PlayerState);

			if(PlayerState)
				PlayerState->ScorePickup();
		}
	} else {
		ACTF_Base* Base = Cast<ACTF_Base>(OtherActor);

		if(Base && Base->GetTeamID() == PossessedBy->GetTeamID()) {
			APS_CTF* PlayerState = Cast<APS_CTF>(PossessedBy->PlayerState);

			OnFlagReturnedDelegate.Broadcast(this);
			if(PlayerState)
				PlayerState->ScoreDeliver();
		}
	}
}

void ACTF_Flag::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACTF_Flag, TeamID);
	DOREPLIFETIME(ACTF_Flag, PossessedBy);
}