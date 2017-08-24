#include "FPSSimulator.h"
#include "FPSSimulatorGameMode.h"
#include "FPSSimulatorGameState.h"
#include "FPSSimulatorPlayerState.h"
#include "FPSSimulatorStaticLibrary.h"
#include "UnrealNetwork.h"

void AFPSSimulatorGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFPSSimulatorGameState, MatchConfig);
	DOREPLIFETIME(AFPSSimulatorGameState, NextMatchConfig);
	DOREPLIFETIME(AFPSSimulatorGameState, QOE_Questions);
	DOREPLIFETIME(AFPSSimulatorGameState, State);
	DOREPLIFETIME(AFPSSimulatorGameState, TeamScores);
	DOREPLIFETIME(AFPSSimulatorGameState, TraceDistance);
}

void AFPSSimulatorGameState::AddPlayerState(class APlayerState* PlayerState) {
	Super::AddPlayerState(PlayerState);
	if(HasAuthority()) {
		AFPSSimulatorPlayerController* PC = Cast<AFPSSimulatorPlayerController>(PlayerState->GetOwner());

		OnPlayerJoined(PC);
	}
}

void AFPSSimulatorGameState::BeginPlay() {
	Super::BeginPlay();
	if(HasAuthority()) {
		UWorld* World = GetWorld();
		AFPSSimulatorGameMode* GameMode = World->GetAuthGameMode<AFPSSimulatorGameMode>();
		UFPSSimulatorGameInstance* GameInstance = Cast<UFPSSimulatorGameInstance>(GetGameInstance());

		if(GameInstance) {
			this->GameInstance = GameInstance;
			if(GameInstance->MatchConfigs.IsValidIndex(GameInstance->GetCurrentMatch())) {
				MatchConfig = GameInstance->MatchConfigs[GameInstance->GetCurrentMatch()];
				QOE_Questions = TArray<FQOE_Question>(GameInstance->QOE_Questions);
			} else {
				MatchConfig = FMatchConfig();
			}
			GameMode->LogStartEvent(MatchConfig.Scenario, MatchConfig.Map, MatchConfig.GameMode, MatchConfig.LagCompMethod);
			if(GameInstance->MatchConfigs.IsValidIndex(GameInstance->GetNextMatch())) {
				NextMatchConfig = GameInstance->MatchConfigs[GameInstance->GetNextMatch()];
			} else {
				NextMatchConfig = FMatchConfig();
			}
			if(GameInstance->NextTeamPlayerID.Num() == 0)
				GameInstance->NextTeamPlayerID.Init(0, NumTeams);
		}
		TeamScores = TArray<float>();
		for(int32 i = 0; i < NumTeams; i++) {
			TeamScores.Add(0);
		}
		GetWorldTimerManager().SetTimer(CountdownTimer, this, &AFPSSimulatorGameState::CountDownRoundTime, 1.f, true);
	}
}

int32 AFPSSimulatorGameState::GetNumPlayers(int32 TeamID) {
	int32 count = 0;

	for(auto Itr(PlayerArray.CreateIterator()); Itr; Itr++) {
		AFPSSimulatorPlayerState* PlayerState = Cast<AFPSSimulatorPlayerState>(*Itr);

		if(TeamID != -1 && PlayerState->GetTeamID() == TeamID || TeamID == -1 && PlayerState->GetTeamID() != -1) {
			count++;
		}
	}
	return count;
}

int32 AFPSSimulatorGameState::GetNumTeams() {
	return NumTeams;
}

float AFPSSimulatorGameState::GetScore(int TeamID) {
	return TeamScores.IsValidIndex(TeamID) ? TeamScores[TeamID] : -1;
}

FString AFPSSimulatorGameState::GetTeamName(int32 TeamID) {
	return TeamNames.Num() > TeamID ? TeamNames[TeamID] : "";
}

FString AFPSSimulatorGameState::GetFormattedTime(int32 Time) {
	FString Minutes = FString::FromInt(Time / 60);
	FString Seconds = FString::FromInt(Time % 60);
	FString TimeStr = (Minutes.Len() > 1 ? "" : "0") + Minutes;

	TimeStr += ":";
	TimeStr += (Seconds.Len() > 1 ? "" : "0") + Seconds;
	return TimeStr;
}

void AFPSSimulatorGameState::InitPlayer(AFPSSimulatorPlayerState* PlayerState) {
}

void AFPSSimulatorGameState::StartNextRound(bool bRestart /*= false*/) {
	if(GameInstance) {
		AFPSSimulatorGameMode* GameMode = GetWorld()->GetAuthGameMode<AFPSSimulatorGameMode>();

		if(!bRestart) GameInstance->SetNextMatchConfig(&NextMatchConfig);
		if(GameMode) GameMode->StartNextRound(bRestart);
	}
}

void AFPSSimulatorGameState::AddScore(int32 TeamID, float Score) {
	if(TeamScores.IsValidIndex(TeamID)) TeamScores[TeamID] += Score;
}

FString AFPSSimulatorGameState::GetFormattedRoundTime() {
	return GetFormattedTime(MatchConfig.RoundTime);
}

FString AFPSSimulatorGameState::GetFormattedRoundIntervalTime() {
	return GetFormattedTime(MatchConfig.RoundIntervalTime);
}

FString AFPSSimulatorGameState::GetFormattedNextRoundTime() {
	return GetFormattedTime(NextMatchConfig.RoundTime);
}

FString AFPSSimulatorGameState::GetFormattedNextRoundIntervalTime() {
	return GetFormattedTime(NextMatchConfig.RoundIntervalTime);
}

int32 AFPSSimulatorGameState::GetWinningTeam() {
	float MaxScore = -1;
	int32 WinningTeam = -1;

	for(int32 i = 0; i < TeamScores.Num(); i++) {
		if(TeamScores[i] > MaxScore) {
			MaxScore = TeamScores[i];
			WinningTeam = i;
		} else if(TeamScores[i] == MaxScore) {
			WinningTeam = -1;
			break;
		}
	}
	return WinningTeam;
}

int32 AFPSSimulatorGameState::GetALCLowPing() {
	return MatchConfig.ALCLowPing;
}

int32 AFPSSimulatorGameState::GetALCHighPing() {
	return MatchConfig.ALCHighPing;
}

ELagCompMethod AFPSSimulatorGameState::GetLagCompMethod() {
	return MatchConfig.LagCompMethod;
}

void AFPSSimulatorGameState::OnRep_State() {
	OnStateChanged.Broadcast(State);
}

void AFPSSimulatorGameState::OnKilled(AFPSSimulatorPlayerState* DamageCauserPlayerState, AFPSSimulatorPlayerState* PlayerState) {
	// To be overridden
}

void AFPSSimulatorGameState::OnPlayerJoined(AFPSSimulatorPlayerController* PC) {
	// To be overridden
}

void AFPSSimulatorGameState::OnNumPlayersReadyForNextRoundUpdated() {
	for(int32 i = 0; i < PlayerArray.Num(); i++) {
		AFPSSimulatorPlayerState* PS = Cast<AFPSSimulatorPlayerState>(PlayerArray[i]);

		if(PS && !PS->bReadyForNextRound && !PS->GetIsAdmin()) return;
	}
	MatchConfig.RoundIntervalTime = FMath::Min(10, MatchConfig.RoundIntervalTime);
	CallOnAllReady();
}

void AFPSSimulatorGameState::OnRep_TeamScores() {
	OnScoreChanged.Broadcast();
}

void AFPSSimulatorGameState::AddNumQOE_Submitted() {
	if(++NumQOE_Submitted >= GetNumPlayers()) {
		StartCountdownToNextRound();
	}
}

void AFPSSimulatorGameState::SetLagCompMethod(ELagCompMethod NewLagCompMethod) {
	MatchConfig.LagCompMethod = NewLagCompMethod;
}

void AFPSSimulatorGameState::SetMatchConfig(FMatchConfig NewMatchConfig) {
	MatchConfig = NewMatchConfig;
}

void AFPSSimulatorGameState::SetRoundTime(int32 NewRoundTime) {
	if(NewRoundTime > 0 && NewRoundTime < 6000) {
		MatchConfig.RoundTime = NewRoundTime;
	}
}

void AFPSSimulatorGameState::SetRoundIntervalTime(int32 NewRoundIntervalTime) {
	if(NewRoundIntervalTime > 0 && NewRoundIntervalTime < 6000) {
		MatchConfig.RoundIntervalTime = NewRoundIntervalTime;
	}
}

void AFPSSimulatorGameState::SetNextMatchConfig(FMatchConfig NewNextMatchConfig) {
	NextMatchConfig = NewNextMatchConfig;
}

void AFPSSimulatorGameState::SetNextRoundTime(int32 NewNextRoundTime) {
	if(NewNextRoundTime > 0 && NewNextRoundTime < 6000) {
		NextMatchConfig.RoundTime = NewNextRoundTime;
	}
}

void AFPSSimulatorGameState::SetNextRoundIntervalTime(int32 NewNextRoundIntervalTime) {
	if(NewNextRoundIntervalTime > 0 && NewNextRoundIntervalTime < 6000) {
		NextMatchConfig.RoundIntervalTime = NewNextRoundIntervalTime;
	}
}

void AFPSSimulatorGameState::SetNextALCLowPing(int32 NewALCLowPing) {
	if(NewALCLowPing > 0) {
		NextMatchConfig.ALCLowPing = NewALCLowPing;
	}
}

void AFPSSimulatorGameState::SetNextALCHighPing(int32 NewALCHighPing) {
	if(NewALCHighPing > 0) {
		NextMatchConfig.ALCHighPing = NewALCHighPing;
	}
}

void AFPSSimulatorGameState::SetNextMaxHealth(int32 NewMaxHealth) {
	if(NewMaxHealth > 0) {
		NextMatchConfig.MaxHealth = NewMaxHealth;
	}
}

void AFPSSimulatorGameState::SetNextWalkSpeed(float NewWalkSpeed) {
	if(NewWalkSpeed > 0) {
		NextMatchConfig.WalkSpeed = NewWalkSpeed;
	}
}

void AFPSSimulatorGameState::SetScore(int32 TeamID, float NewScore) {
	if(TeamID < TeamScores.Num()) {
		TeamScores[TeamID] = NewScore;
	}
}

void AFPSSimulatorGameState::SetState(EMatchState NewState) {
	if(State != NewState)
		State = NewState;
}

void AFPSSimulatorGameState::CallOnAllReady_Implementation() {
	OnAllReady.Broadcast();
}

void AFPSSimulatorGameState::CountDownRoundTime() {
	MatchConfig.RoundTime--;
	if(MatchConfig.RoundTime <= 0) {
		GetWorldTimerManager().ClearTimer(CountdownTimer);
		EndRound();
	}
}

void AFPSSimulatorGameState::CountdownToNextRound() {
	MatchConfig.RoundIntervalTime--;
	if(MatchConfig.RoundIntervalTime <= 0) {
		GetWorldTimerManager().ClearTimer(CountdownTimer);
		StartNextRound();
	}
}

void AFPSSimulatorGameState::EndRound() {
	AFPSSimulatorGameMode* GameMode = GetWorld()->GetAuthGameMode<AFPSSimulatorGameMode>();
	
	for(int32 i = 0; i < PlayerArray.Num(); i++) {
		AFPSSimulatorPlayerState* PS = Cast<AFPSSimulatorPlayerState>(PlayerArray[i]);

		if(PS && !PS->GetIsAdmin())
			GameMode->LogEndEvent(PS);
	}
	if(!MatchConfig.bSkipMOS) {
		SetState(EMatchState::PostMatch);
	} else {
		StartCountdownToNextRound();
	}
	// Generate command lines for clumsy
	if(!GameInstance->ClumsyPath.IsEmpty() && NextMatchConfig.LagInjectionGroup != -1) {
		FString Command = "start clumsy.exe --filter \"";

		for(auto It = GameInstance->PersistentPlayerStates.CreateConstIterator(); It; ++It) {
			FPersistentPlayerState PlayerState = It.Value();

			if(PlayerState.TeamPlayerID % 2 == NextMatchConfig.LagInjectionGroup) {
				Command.Append("ip.DstAddr==" + It.Key() + " or ip.SrcAddr==" + It.Key() + " or ");
			}
		}
		Command.RemoveFromEnd(" or ");
		Command.Append("\" --lag on --lag-time 125");
		FFileHelper::SaveStringToFile(Command, *(GameInstance->ClumsyPath + "/LAGCOM.bat"));
	}
}

void AFPSSimulatorGameState::StartCountdownToNextRound() {
	AFPSSimulatorGameMode* GameMode = GetWorld()->GetAuthGameMode<AFPSSimulatorGameMode>();

	GetWorldTimerManager().SetTimer(CountdownTimer, this, &AFPSSimulatorGameState::CountdownToNextRound, 1.f, true);
	SetState(EMatchState::PostMatchCountdown);
	GameMode->SaveLogs();
}
