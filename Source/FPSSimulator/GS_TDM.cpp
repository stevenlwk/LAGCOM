#include "FPSSimulator.h"
#include "FPSSimulatorGameMode.h"
#include "FPSSimulatorPlayerController.h"
#include "GS_TDM.h"

void AGS_TDM::OnKilled(AFPSSimulatorPlayerState* DamageCauserPlayerState, AFPSSimulatorPlayerState* PlayerState) {
	AFPSSimulatorGameMode* GameMode = GetWorld()->GetAuthGameMode<AFPSSimulatorGameMode>();
	int32 DamageCauserTeamID = DamageCauserPlayerState->GetTeamID();

	if(DamageCauserTeamID != PlayerState->GetTeamID() || GameMode->GetIsDebugMode()) {
		SetScore(DamageCauserTeamID, GetScore(DamageCauserTeamID) + 1);
		if(GetScore(DamageCauserTeamID) == NumKillsToWin) {
			EndRound();
		}
	}
}
