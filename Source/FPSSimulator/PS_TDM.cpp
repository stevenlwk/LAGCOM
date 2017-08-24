#include "FPSSimulator.h"
#include "FPSSimulatorGameState.h"
#include "PS_TDM.h"

void APS_TDM::OnKilled(AFPSSimulatorPlayerState* PlayerState) {
	Super::OnKilled(PlayerState);
	if(GetTeamID() != PlayerState->GetTeamID()) {
		ScoreKill(PlayerState);
		PlayerState->ScoreDeath();
	} else {
		ScoreKill(PlayerState, true);
	}
}