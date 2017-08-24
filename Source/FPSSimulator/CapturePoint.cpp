#include "FPSSimulator.h"
#include "CapturePoint.h"
#include "GS_Domination.h"
#include "PS_Domination.h"
#include "UnrealNetwork.h"

ACapturePoint::ACapturePoint(const FObjectInitializer& ObjectInitializer) {
	Scene = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, "Scene");
	Scene->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	Area = ObjectInitializer.CreateDefaultSubobject<UBoxComponent>(this, "Area");
	Area->AttachToComponent(Scene, FAttachmentTransformRules::KeepRelativeTransform);
}

void ACapturePoint::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	DOREPLIFETIME(ACapturePoint, Control);
	DOREPLIFETIME(ACapturePoint, OwnedBy);
	DOREPLIFETIME(ACapturePoint, TeamWithProgress);
}

void ACapturePoint::BeginPlay() {
	Super::BeginPlay();
	GameState = GetWorld()->GetGameState<AFPSSimulatorGameState>();
	if(!Cast<AGS_Domination>(GameState)) {
		Destroy();
		return;
	}
	if(HasAuthority()) {
		NumPlayersInArea = TArray<int32>();
		NumPlayersInArea.Init(0, GameState->GetNumTeams());
		Area->OnComponentBeginOverlap.AddDynamic(this, &ACapturePoint::OnBeginOverlap);
		Area->OnComponentEndOverlap.AddDynamic(this, &ACapturePoint::OnEndOverlap);
	}
}

void ACapturePoint::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
	AFPSSimulatorCharacter* Character = Cast<AFPSSimulatorCharacter>(OtherActor);

	if(Character) {
		APS_Domination* PlayerState = Cast<APS_Domination>(Character->PlayerState);

		if(PlayerState) PlayerState->OnEnterCapturePoint(this);
		UpdateNumPlayersInArea();
		Character->OnDiedDelegate.AddDynamic(this, &ACapturePoint::OnPlayerDieInArea);
	}
}

void ACapturePoint::OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {
	AFPSSimulatorCharacter* Character = Cast<AFPSSimulatorCharacter>(OtherActor);

	if(Character) {
		APS_Domination* PlayerState = Cast<APS_Domination>(Character->PlayerState);

		if(PlayerState) PlayerState->OnLeaveCapturePoint();
		UpdateNumPlayersInArea();
		Character->OnDiedDelegate.RemoveDynamic(this, &ACapturePoint::OnPlayerDieInArea);
	}
}

void ACapturePoint::OnPlayerDieInArea(AFPSSimulatorCharacter* Victim, AFPSSimulatorCharacter* Killer) {
	APS_Domination* PlayerState = Cast<APS_Domination>(Victim->PlayerState);

	if(PlayerState) PlayerState->OnLeaveCapturePoint();
	UpdateNumPlayersInArea();
	Victim->OnDiedDelegate.RemoveDynamic(this, &ACapturePoint::OnPlayerDieInArea);
}

void ACapturePoint::UpdateNumPlayersInArea() {
	TSet<AActor*> PlayersInArea;

	NumPlayersInArea.Init(0, GameState->GetNumTeams());
	Area->GetOverlappingActors(PlayersInArea, TSubclassOf<AFPSSimulatorCharacter>());
	for(auto& Elem : PlayersInArea) {
		AFPSSimulatorCharacter* Character = Cast<AFPSSimulatorCharacter>(Elem);

		if(Character && Character->GetHealth() > 0 && NumPlayersInArea.IsValidIndex(Character->GetTeamID())) {
			NumPlayersInArea[Character->GetTeamID()]++;
		}
	}
	// Get the number of majority teams
	int32 MaxNumTeamPlayers = 0, NumMajorityTeams = 0;

	for(int32 i = 0; i < NumPlayersInArea.Num(); i++) {
		if(NumPlayersInArea[i] > MaxNumTeamPlayers) {
			MaxNumTeamPlayers = NumPlayersInArea[i];
			NumMajorityTeams = 1;
		} else if(NumPlayersInArea[i] == MaxNumTeamPlayers) {
			NumMajorityTeams++;
		}
	}
	// Update the majority team
	if(NumMajorityTeams != 1) {
		MajorityTeam = -1;
	} else {
		for(int32 i = 0; i < NumPlayersInArea.Num(); i++) {
			if(NumPlayersInArea[i] == MaxNumTeamPlayers) {
				MajorityTeam = i;
				break;
			}
		}
	}
}

int32 ACapturePoint::GetNumPlayersInArea(int32 TeamID) {
	return NumPlayersInArea[TeamID];
}

const TSet<AFPSSimulatorCharacter*> ACapturePoint::GetPlayersInArea(int32 TeamID) {
	TSet<AFPSSimulatorCharacter*> PlayersInArea = TSet<AFPSSimulatorCharacter*>();
	TSet<AActor*> ActorsInArea;

	Area->GetOverlappingActors(ActorsInArea, TSubclassOf<AFPSSimulatorCharacter>());
	for(auto& Elem : ActorsInArea) {
		AFPSSimulatorCharacter* Character = Cast<AFPSSimulatorCharacter>(Elem);

		if(Character && Character->GetHealth() > 0)
			PlayersInArea.Add(Character);
	}
	return PlayersInArea;
}

int32 ACapturePoint::GetTotalNumPlayersInArea() {
	return TotalNumPlayersInArea;
}

int32 ACapturePoint::GetMajorityTeam() {
	return MajorityTeam;
}

void ACapturePoint::SetState(ECapturePointState NewState) {
	if(State != NewState) {
		CallOnStateChanged(State, NewState);
		State = NewState;
	}
}

void ACapturePoint::CallOnStateChanged_Implementation(ECapturePointState OldState, ECapturePointState NewState) {
	OnStateChangedDelegate.Broadcast(OldState, NewState);
}
