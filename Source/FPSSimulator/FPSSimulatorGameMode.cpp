#include "FPSSimulator.h"
#include "FPSSimulatorAIController.h"
#include "FPSSimulatorCharacter.h"
#include "FPSSimulatorFunctionLibrary.h"
#include "FPSSimulatorGameInstance.h"
#include "FPSSimulatorGameMode.h"
#include "FPSSimulatorGameState.h"
#include "FPSSimulatorPlayerController.h"
#include "FPSSimulatorPlayerState.h"
#include "FPSSimulatorSpectatorPawn.h"
#include "SpawnArea.h"

AFPSSimulatorGameMode::AFPSSimulatorGameMode(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer) {
	PrimaryActorTick.bCanEverTick = true;
	DefaultPawnClass = NULL;
	PlayerControllerClass = AFPSSimulatorPlayerController::StaticClass();
	PlayerStateClass = AFPSSimulatorPlayerState::StaticClass();
	GameStateClass = AFPSSimulatorGameState::StaticClass();
	if(GConfig) {
		FString TickRateStr;

		TickRateStr = GConfig->GetStr(TEXT("/Script/OnlineSubsystemUtils.IpNetDriver"), TEXT("NetServerMaxTickRate"), GEngineIni);
		TickRate = FCString::Atoi(*TickRateStr);
	}
}

void AFPSSimulatorGameMode::BeginPlay() {
	Super::BeginPlay();
	GameInstance = Cast<UFPSSimulatorGameInstance>(GetGameInstance());
	GameState = GetGameState<AFPSSimulatorGameState>();
	GameState->SetState(EMatchState::InProgress);
	Logs = TArray<UBaseLogEvent*>();
	LogStartEvent(GameState->MatchConfig.Scenario, GameState->MatchConfig.Map, GameState->MatchConfig.GameMode, GameState->MatchConfig.LagCompMethod);
}

void AFPSSimulatorGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) {
	FString Password = UGameplayStatics::ParseOption(Options, "Password");

	if(bDebugMode) return;
	if(!Password.IsEmpty() && !Password.Equals("704197")) {
		ErrorMessage = "Incorrect password";
	}
}

APlayerController* AFPSSimulatorGameMode::Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) {
	APlayerController* PC = Super::Login(NewPlayer, InRemoteRole, Portal, Options, UniqueId, ErrorMessage);
	AFPSSimulatorPlayerController* CastedPC = Cast<AFPSSimulatorPlayerController>(PC);

	if(bDebugMode && bDebugAsAdmin || UGameplayStatics::ParseOption(Options, "Password").Equals("704197")) {
		if(CastedPC) {
			CastedPC->bIsSpectator = true;
		}
	}
	return PC;
}

void AFPSSimulatorGameMode::PostLogin(APlayerController* NewPlayer) {
	Super::PostLogin(NewPlayer);
	AFPSSimulatorPlayerController* PC = Cast<AFPSSimulatorPlayerController>(NewPlayer);

	if(PC) {
		AFPSSimulatorPlayerState* PlayerState = Cast<AFPSSimulatorPlayerState>(NewPlayer->PlayerState);
		UFPSSimulatorGameInstance* GameInstance = Cast<UFPSSimulatorGameInstance>(GetGameInstance());

		if(!PC->bIsSpectator) {
			bool bIsNewPlayer = true;

			// Restore player states from the last round
			if(GameInstance->PersistentPlayerStates.Contains(PlayerState->SavedNetworkAddress)) {
				FPersistentPlayerState LastRoundPlayerState = GameInstance->PersistentPlayerStates[PlayerState->SavedNetworkAddress];

				PlayerState->SetPlayerName(LastRoundPlayerState.PlayerName);
				PlayerState->SetTeamID(LastRoundPlayerState.TeamID);
				PlayerState->TeamPlayerID = LastRoundPlayerState.TeamPlayerID;
				bIsNewPlayer = false;
			} else {
				// Find and replace reconnecting player's states with saved record
				for(int32 i = 0; i < GameInstance->PlayerStates.Num(); i++) {
					AFPSSimulatorPlayerState* PS = GameInstance->PlayerStates[i];

					if(PS->SavedNetworkAddress == PlayerState->SavedNetworkAddress) {
						PS->DispatchCopyProperties(PlayerState);
						//GameInstance->PlayerStates.RemoveAt(i);
						bIsNewPlayer = false;
						break;
					}
				}
			}
			// For new player
			if(bIsNewPlayer) {
				int32 NumTeams = GameState->GetNumTeams(), TeamID = 0;

				PlayerState->SetPlayerName("Player " + FString::FromInt(PlayerNameSuffix++));
				if(!bDebugMode) {
					if(NumTeams > 0) {
						int32 LeastNumPlayers = GameState->GetNumPlayers(0);

						for(int32 i = 1; i < NumTeams; i++) {
							int32 NumPlayers = GameState->GetNumPlayers(i);

							if(NumPlayers < LeastNumPlayers) {
								LeastNumPlayers = NumPlayers;
								TeamID = i;
							}
						}
						PlayerState->TeamPlayerID = GameInstance->NextTeamPlayerID[TeamID]++;
						GameInstance->PlayerStates.Add(PlayerState);
						GameInstance->PersistentPlayerStates.Add(PlayerState->SavedNetworkAddress, FPersistentPlayerState(PlayerState->PlayerName, TeamID, PlayerState->TeamPlayerID));
					}
				} else {
					TeamID = FMath::RandRange(0, NumTeams - 1);
				}
				PlayerState->SetTeamID(TeamID);
			}

			FTransform SpawnTransform = GetSpawnTransform(PlayerState->GetTeamID());
			FActorSpawnParameters SpawnParameters;
			AFPSSimulatorCharacter* Character;
			
			SpawnParameters.Owner = PC;
			SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			Character = GetWorld()->SpawnActor<AFPSSimulatorCharacter>(CharacterClass, SpawnTransform.GetLocation(), SpawnTransform.Rotator(), SpawnParameters);
			Character->AddActorLocalOffset(FVector(0, 0, Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()));
			Character->SetTeamID(PlayerState->GetTeamID());
			NewPlayer->Possess(Character);
			GameState->InitPlayer(PlayerState);
		} else {
			AFPSSimulatorSpectatorPawn* SpectatorPawn = GetWorld()->SpawnActor<AFPSSimulatorSpectatorPawn>(SpectatorPawnClass, FVector(0, 0, 500), FRotator::ZeroRotator);

			NewPlayer->Possess(SpectatorPawn);
			PlayerState->SetIsAdmin(true);
		}
	}
}

void AFPSSimulatorGameMode::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);
	TickCount++;
}

void AFPSSimulatorGameMode::Respawn(AFPSSimulatorCharacter* Character) {
	AFPSSimulatorPlayerState* PlayerState = Cast<AFPSSimulatorPlayerState>(Character->PlayerState);

	if(PlayerState){
		FTransform SpawnTransform = GetSpawnTransform(PlayerState->GetTeamID());

		Character->TeleportTo(SpawnTransform.GetLocation(), SpawnTransform.Rotator());
	}
}

bool AFPSSimulatorGameMode::GetIsDebugMode() {
	return bDebugMode;
}

void AFPSSimulatorGameMode::StartNextRound(bool bRestart /*= false*/) {
	for(TActorIterator<AFPSSimulatorPlayerController> Itr(GetWorld()); Itr; ++Itr) {
		AFPSSimulatorPlayerController* PC = *Itr;

		PC->OnRestart(bRestart);
	}
	FMatchConfig* NextMatchConfig = &GameInstance->MatchConfigs[!bRestart ? GameInstance->GetNextMatch() : GameInstance->GetCurrentMatch()];
	FURL URL = FURL();

	URL.Map = "/Game/Maps/" + NextMatchConfig->Map;
	URL.AddOption(*FString("Game=/Game/Blueprints/GameMode/GM_" + NextMatchConfig->GameMode + "_BP.GM_" + NextMatchConfig->GameMode + "_BP_C"));
	GameInstance->StartNextMatch(bRestart);
	GetWorld()->ServerTravel(URL.ToString());
}

int32 AFPSSimulatorGameMode::GetTickCount() {
	return TickCount;
}

int32 AFPSSimulatorGameMode::GetTickRate() {
	return TickRate;
}

void AFPSSimulatorGameMode::LogStartEvent(FString Scenario, FString Map, FString GameMode, ELagCompMethod LagCompMethod) {
	UStartLogEvent* Event = NewObject<UStartLogEvent>(this);

	Event->Init(TickCount, Scenario, Map, GameMode, LagCompMethod);
	Logs.Add(Event);
}

void AFPSSimulatorGameMode::LogEndEvent(AFPSSimulatorPlayerState* PlayerState) {
	UEndLogEvent* Event = NewObject<UEndLogEvent>(this);

	Event->Init(TickCount, PlayerState, PlayerState->TeamPlayerID);
	Logs.Add(Event);
}

void AFPSSimulatorGameMode::LogFireEvent(EFireLogEventType Type, ELagCompMethod LagCompMethod, AFPSSimulatorPlayerState* ShooterPS, AFPSSimulatorPlayerState* VictimPS, FVector HitFrom, FVector HitTo, float VictimRemainingHealth, float RollbackDist /*= 0*/) {
	UFireLogEvent* Event = NewObject<UFireLogEvent>(this);

	Event->Init(TickCount, Type, LagCompMethod, ShooterPS, VictimPS, HitFrom, HitTo, VictimRemainingHealth, RollbackDist);
	Logs.Add(Event);
}

void AFPSSimulatorGameMode::LogQOEEvent(AFPSSimulatorPlayerState* PlayerState) {
	UQOELogEvent* Event = NewObject<UQOELogEvent>(this);

	Event->Init(TickCount, PlayerState);
	Logs.Add(Event);
}

FTransform AFPSSimulatorGameMode::GetSpawnTransform(int32 TeamID) {
	TArray<ASpawnArea*> Areas = TArray<ASpawnArea*>();

	for(TActorIterator<ASpawnArea> Itr(GetWorld()); Itr; ++Itr) {
		ASpawnArea* Area = *Itr;

		if(Area->Channel == ESpawnAreaChannel::Player && Area->ID == TeamID) {
			Areas.Add(Area);
		}
	}
	if(Areas.Num() > 0) {
		ASpawnArea* Area = Areas[FMath::RandRange(0, Areas.Num() - 1)];

		return Area->GetRandomSpawn();
	} else {
		return FTransform(FRotator::ZeroRotator, FVector::ZeroVector);
	}
}

void AFPSSimulatorGameMode::SaveLogs() {
	TSharedPtr<FJsonObject> OuterJsonObject = MakeShareable(new FJsonObject());
	TArray<TSharedPtr<FJsonValue>> JsonValues = TArray<TSharedPtr<FJsonValue>>();

	for(int32 i = 0; i < Logs.Num(); i++) {
		if(!Logs[i]->IsValidLowLevel()) continue;
		TSharedPtr<FJsonObject> JsonObject = Logs[i]->ToJsonObject();
		TSharedRef<FJsonValueObject> JsonValueObject = MakeShareable(new FJsonValueObject(JsonObject));

		JsonValues.Add(JsonValueObject);
	}
	OuterJsonObject->SetArrayField("logs", JsonValues);
	FFileHelper::SaveStringToFile(UFPSSimulatorFunctionLibrary::JsonToString(OuterJsonObject),
		*(FPaths::GameLogDir() + GameState->MatchConfig.Scenario + "_" + FString::FromInt(FDateTime::UtcNow().ToUnixTimestamp()) + ".log"));
}
