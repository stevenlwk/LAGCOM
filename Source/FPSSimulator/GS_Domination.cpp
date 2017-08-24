#include "FPSSimulator.h"
#include "FPSSimulatorCharacter.h"
#include "GS_Domination.h"
#include "PS_Domination.h"

AGS_Domination::AGS_Domination() {
	if(HasAuthority()) {
		PrimaryActorTick.bCanEverTick = true;
	}
}

void AGS_Domination::BeginPlay() {
	Super::BeginPlay();
	if(HasAuthority()) {
		CapturePoints = TArray<ACapturePoint*>();
		for(TActorIterator<ACapturePoint> Itr(GetWorld()); Itr; ++Itr) {
			CapturePoints.Add(*Itr);
		}
	}
}

void AGS_Domination::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);
	if(HasAuthority()) {
		for(int32 i = 0; i < CapturePoints.Num(); i++) {
			ACapturePoint* CP = CapturePoints[i];
			int32 MajorityTeam = CP->GetMajorityTeam();
			float TargetControl = -1;

			if(CP->OwnedBy != -1) {
				float Score = GetScore(CP->OwnedBy);

				SetScore(CP->OwnedBy, FMath::FInterpConstantTo(Score, Score + 1, DeltaSeconds, TicketGainRate));
			}

			if(CP->Control == 1) {
				if(MajorityTeam != -1) {
					if(CP->OwnedBy == -1) { // Captured
						CP->OwnedBy = MajorityTeam;
						CP->SetState(ECapturePointState::Captured);
					} else if(CP->OwnedBy != MajorityTeam) { // Neutralize
						TargetControl = 0;
						CP->SetState(ECapturePointState::Neutral);
					}
				}
			} else if(CP->Control > 0) {
				if(MajorityTeam != -1) {
					if(MajorityTeam == CP->TeamWithProgress) { // Capturing
						TargetControl = 1;
						CP->SetState(ECapturePointState::Capturing);
					} else { // Neutralizing
						TargetControl = 0;
						CP->SetState(ECapturePointState::Neutralizing);
					}
				} else if(CP->GetTotalNumPlayersInArea() == 0) { // Reverting
					if(CP->OwnedBy != -1) {
						TargetControl = 1;
					} else {
						TargetControl = 0;
					}
					CP->SetState(ECapturePointState::Reverting);
				}
			} else {
				if(MajorityTeam != -1) { // Neutralized
					CP->OwnedBy = -1;
					CP->TeamWithProgress = MajorityTeam;
					TargetControl = 1;
				} else if(CP->GetTotalNumPlayersInArea() == 0) { // Reverted
					CP->TeamWithProgress = -1;
				}
				CP->SetState(ECapturePointState::Neutral);
			}
			if(TargetControl != -1) CP->Control = FMath::FInterpConstantTo(CP->Control, TargetControl, DeltaSeconds, CaptureRate);
		}
	}
}

void AGS_Domination::ScoreCapture(ACapturePoint* CapturePoint) {
	TSet<AFPSSimulatorCharacter*> PlayersInArea = CapturePoint->GetPlayersInArea(CapturePoint->OwnedBy);

	for(auto& Elem : PlayersInArea) {
		AFPSSimulatorCharacter* Character = Cast<AFPSSimulatorCharacter>(Elem);

		if(Character) {
			APS_Domination* PlayerState = Cast<APS_Domination>(Character->PlayerState);

			if(PlayerState)
				PlayerState->ScoreCapture();
		}
	}
}
