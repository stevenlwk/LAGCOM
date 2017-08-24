#include "FPSSimulator.h"
#include "PS_Domination.h"

void APS_Domination::BeginPlay() {
	Super::BeginPlay();
	GameState = Cast<AGS_Domination>(GetWorld()->GetGameState());
}

void APS_Domination::OnKilled(AFPSSimulatorPlayerState* PlayerState) {
	Super::OnKilled(PlayerState);
	if(GetTeamID() != PlayerState->GetTeamID()) {
		APS_Domination* PS = Cast<APS_Domination>(PlayerState);

		ScoreKill(PlayerState);
		PlayerState->ScoreDeath();
		if(PS && PS->CapturePoint && PS->CapturePoint->OwnedBy == TeamID) {
			Score += GameState->DefenseScore;
			NotifyScore(EScoreType::Domination, {"Domination defense"}, GameState->DefenseScore);
		}
	} else {
		ScoreKill(PlayerState, true);
	}
}

void APS_Domination::OnEnterCapturePoint(ACapturePoint* CapturePoint) {
	this->CapturePoint = CapturePoint;
	CapturePoint->OnStateChangedDelegate.AddDynamic(this, &APS_Domination::OnCapturePointStateChanged);
}

void APS_Domination::OnLeaveCapturePoint() {
	GetWorldTimerManager().ClearTimer(ScoreCaptureTimer);
	CallOnEndCapture();
	if(CapturePoint)
		CapturePoint->OnStateChangedDelegate.RemoveDynamic(this, &APS_Domination::OnCapturePointStateChanged);
	this->CapturePoint = nullptr;
}

void APS_Domination::ScoreCapture() {
	Score += GameState->CaptureScore;
	NotifyScore(EScoreType::Domination, {"Domination capture"}, GameState->CaptureScore);
}

void APS_Domination::OnCapturePointStateChanged(ECapturePointState OldState, ECapturePointState NewState) {
	if(OldState == ECapturePointState::Capturing || OldState == ECapturePointState::Neutralizing) {
		GetWorldTimerManager().ClearTimer(ScoreCaptureTimer);
		CallOnEndCapture();
	}
	if(NewState == ECapturePointState::Capturing || NewState == ECapturePointState::Neutralizing) {
		GetWorldTimerManager().ClearTimer(ScoreCaptureTimer);
		GetWorldTimerManager().SetTimer(ScoreCaptureTimer, this, &APS_Domination::ScoreCapturing, 1.f, true);
		CallOnStartCapture();
	} else if(NewState == ECapturePointState::Captured) {
		ScoreCapture();
	}
}

void APS_Domination::CallOnStartCapture_Implementation() {
	OnStartCapture();
}

void APS_Domination::CallOnEndCapture_Implementation() {
	OnEndCapture();
}

void APS_Domination::ScoreCapturing() {
	Score += GameState->CapturingScore;
	NotifyScore(EScoreType::Default, {"Capturing"}, GameState->CapturingScore);
}
