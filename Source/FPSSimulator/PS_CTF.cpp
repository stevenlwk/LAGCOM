#include "FPSSimulator.h"
#include "PS_CTF.h"

void APS_CTF::BeginPlay() {
	Super::BeginPlay();
	GameState = Cast<AGS_CTF>(GetWorld()->GetGameState());
}

void APS_CTF::OnKilled(AFPSSimulatorPlayerState* PlayerState) {
	Super::OnKilled(PlayerState);
	if(GetTeamID() != PlayerState->GetTeamID()) {
		ScoreKill(PlayerState);
		PlayerState->ScoreDeath();
	} else {
		ScoreKill(PlayerState, true);
	}
}

void APS_CTF::ScoreCarrierKill() {
	Score += GameState->CarrierKillScore;
	NotifyScore(EScoreType::Pinned, {"Fire extinguisher carrier killed"}, GameState->CarrierKillScore);
}

void APS_CTF::ScoreDeliver() {
	Score += GameState->DeliverScore;
	NotifyScore(EScoreType::CTF, {"Fire extinguisher deliver"}, GameState->DeliverScore);
}

void APS_CTF::ScorePickup() {
	Score += GameState->PickupScore;
	NotifyScore(EScoreType::CTF, {"Fire extinguisher pickup"}, GameState->PickupScore);
}
