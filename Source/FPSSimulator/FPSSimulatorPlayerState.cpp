#include "FPSSimulator.h"
#include "FPSSimulatorCharacter.h"
#include "FPSSimulatorGameState.h"
#include "FPSSimulatorGameMode.h"
#include "FPSSimulatorPlayerState.h"
#include "UnrealNetwork.h"

AFPSSimulatorPlayerState::AFPSSimulatorPlayerState(const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer){
}

void AFPSSimulatorPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFPSSimulatorPlayerState, TeamID);
	DOREPLIFETIME(AFPSSimulatorPlayerState, bReadyForNextRound);
	DOREPLIFETIME(AFPSSimulatorPlayerState, NumKills);
	DOREPLIFETIME(AFPSSimulatorPlayerState, NumDeaths);
	DOREPLIFETIME(AFPSSimulatorPlayerState, NumShotsFired);
	DOREPLIFETIME(AFPSSimulatorPlayerState, NumShotsHit);
}

void AFPSSimulatorPlayerState::BeginPlay() {
	Super::BeginPlay();
	QOE_Scores = TArray<int32>();
	GameState = Cast<AFPSSimulatorGameState>(GetWorld()->GetGameState());
}

void AFPSSimulatorPlayerState::Init(int32 TeamID) {
	this->TeamID = TeamID;
}

void AFPSSimulatorPlayerState::OnKilled(AFPSSimulatorPlayerState* PlayerState) {
	// To be overridden
}

bool AFPSSimulatorPlayerState::GetIsAdmin() {
	return bIsAdmin;
}

void AFPSSimulatorPlayerState::SubmitQOE_Implementation(const TArray<int32>& Scores) {
	AFPSSimulatorGameMode* GameMode = GetWorld()->GetAuthGameMode<AFPSSimulatorGameMode>();
	AFPSSimulatorGameState* GameState = GetWorld()->GetGameState<AFPSSimulatorGameState>();
	
	QOE_Scores = Scores;
	GameMode->LogQOEEvent(this);
	GameState->AddNumQOE_Submitted();
}

bool AFPSSimulatorPlayerState::SubmitQOE_Validate(const TArray<int32>& Scores) {
	return true;
}

int32 AFPSSimulatorPlayerState::GetTeamID() {
	return TeamID;
}

int32 AFPSSimulatorPlayerState::GetNumKills() {
	return NumKills;
}

int32 AFPSSimulatorPlayerState::GetNumDeaths() {
	return NumDeaths;
}

int32 AFPSSimulatorPlayerState::GetNumShotsFired() {
	return NumShotsFired;
}

int32 AFPSSimulatorPlayerState::GetNumShotsHit() {
	return NumShotsHit;
}

int32 AFPSSimulatorPlayerState::GetNumShotsBehindCovers() {
	return NumShotsBehindCovers;
}

int32 AFPSSimulatorPlayerState::GetNumShotsBehindCoversReported() {
	return NumShotsBehindCoversReported;
}

int32 AFPSSimulatorPlayerState::GetNumShotsDenied() {
	return NumShotsDenied;
}

int32 AFPSSimulatorPlayerState::GetNumShotsOverruled() {
	return NumShotsOverruled;
}

int32 AFPSSimulatorPlayerState::GetKillStreak() {
	return KillStreak;
}

void AFPSSimulatorPlayerState::AddDistancesTravelled(float Distance) {
	DistancesTravelled += Distance;
}

int32 AFPSSimulatorPlayerState::GetDistancesTravelled() {
	return DistancesTravelled;
}

void AFPSSimulatorPlayerState::AddNumShotsBehindCovers() {
	NumShotsBehindCovers++;
}

void AFPSSimulatorPlayerState::AddNumShotsBehindCoversReported() {
	NumShotsBehindCoversReported++;
}

void AFPSSimulatorPlayerState::AddNumShotsFired() {
	NumShotsFired++;
}

void AFPSSimulatorPlayerState::AddNumShotsDenied() {
	NumShotsDenied++;
}

void AFPSSimulatorPlayerState::AddNumShotsOverruled() {
	NumShotsOverruled++;
}

void AFPSSimulatorPlayerState::AddNumShotsHit() {
	NumShotsHit++;
}

void AFPSSimulatorPlayerState::AddNumShotsHitBy() {
	NumShotsHitBy++;
}

void AFPSSimulatorPlayerState::SetIsAdmin(bool bNewIsAdmin) {
	bIsAdmin = bNewIsAdmin;
}

void AFPSSimulatorPlayerState::SetTeamID(int32 TeamID) {
	this->TeamID = TeamID;
}

void AFPSSimulatorPlayerState::ScoreKill(AFPSSimulatorPlayerState* VictimPlayerState, bool bIsFriendlyFire /*= false*/) {
	if(!bIsFriendlyFire) {
		NumKills++;
		KillStreak++;
		Score += GameState->ScorePerKill;
		NotifyScore(EScoreType::Kill, {"Killed", VictimPlayerState->PlayerName, FString::FromInt(VictimPlayerState->GetTeamID())}, GameState->ScorePerKill);
	} else {
		NumKills--;
		Score -= GameState->ScorePerKill;
	}
	if(KillStreak == 2) {
		Score += GameState->DoubleKillBonus;
		NotifyScore(EScoreType::Default, {"Double kill"}, GameState->DoubleKillBonus);
	} else if(KillStreak == 3) {
		Score += GameState->TripleKillBonus;
		NotifyScore(EScoreType::Default, {"Triple kill"}, GameState->TripleKillBonus);
	} else if(KillStreak > 3) {
		Score += GameState->MultiKillBonus;
		NotifyScore(EScoreType::Default, {"Multi kill"}, GameState->MultiKillBonus);
	}
}

void AFPSSimulatorPlayerState::ScoreDeath() {
	NumDeaths++;
	KillStreak = 0;
}

void AFPSSimulatorPlayerState::ToggleReadyForNextRound_Implementation() {
	AFPSSimulatorGameState* GameState = Cast<AFPSSimulatorGameState>(GetWorld()->GetGameState());
	
	bReadyForNextRound = !bReadyForNextRound;
	GameState->OnNumPlayersReadyForNextRoundUpdated();
}

bool AFPSSimulatorPlayerState::ToggleReadyForNextRound_Validate() {
	return true;
}

void AFPSSimulatorPlayerState::OnRep_ReadyForNextRound() {
	OnRepReadyForNextRoundDelegate.Broadcast();
}

void AFPSSimulatorPlayerState::OnRep_Score() {
	OnRepScoreDelegate.Broadcast();
}

void AFPSSimulatorPlayerState::OnRepStats() {
	OnRepStatsDelegate.Broadcast();
}

void AFPSSimulatorPlayerState::CopyProperties(APlayerState* PlayerState) {
	Super::CopyProperties(PlayerState);
	if(PlayerState) {
		AFPSSimulatorPlayerState* InPlayerState = Cast<AFPSSimulatorPlayerState>(PlayerState);

		if(InPlayerState) {
			InPlayerState->TeamID = TeamID;
			InPlayerState->TeamPlayerID = TeamPlayerID;
			InPlayerState->DistancesTravelled = DistancesTravelled;
			InPlayerState->NumKills = NumKills;
			InPlayerState->NumDeaths = NumDeaths;
			InPlayerState->NumShotsFired = NumShotsFired;
			InPlayerState->NumShotsHit = NumShotsHit;
			InPlayerState->NumShotsHitBy = NumShotsHitBy;
			InPlayerState->NumShotsBehindCovers = NumShotsBehindCovers;
			InPlayerState->NumShotsBehindCoversReported = NumShotsBehindCoversReported;
			InPlayerState->NumShotsDenied = NumShotsDenied;
			InPlayerState->NumShotsOverruled = NumShotsOverruled;
		}
	}
}

void AFPSSimulatorPlayerState::AddToScoreboard_Implementation() {
	AddToScoreboard_BP();
}

void AFPSSimulatorPlayerState::NotifyScore_Implementation(EScoreType Type, const TArray<FString>& Labels, int32 InScore) {
	OnScoreDelegate.Broadcast(Type, Labels, InScore);
}