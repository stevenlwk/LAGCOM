#pragma once

#include "Engine/GameInstance.h"
#include "FPSSimulatorPlayerState.h"
#include "FPSSimulatorGameInstance.generated.h"

class AFPSSimulatorGameState;

UENUM(BlueprintType)
enum class ELagCompMethod : uint8 {
	None,
	Normal,
	Advanced
};

USTRUCT(BlueprintType)
struct FQOE_Question {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString Question = "";
	UPROPERTY(BlueprintReadOnly)
	FString Label1 = "";
	UPROPERTY(BlueprintReadOnly)
	FString Label2 = "";

	FQOE_Question() {}
};

USTRUCT()
struct FMatchConfig {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FString Scenario = "";
	UPROPERTY(BlueprintReadWrite)
	FString Map = "";
	UPROPERTY(BlueprintReadWrite)
	FString GameMode = "";
	UPROPERTY(BlueprintReadOnly)
	int32 RoundTime = 600;
	UPROPERTY(BlueprintReadOnly)
	int32 RoundIntervalTime = 60;
	UPROPERTY(BlueprintReadWrite)
	ELagCompMethod LagCompMethod = ELagCompMethod::None;
	UPROPERTY(BlueprintReadOnly)
	int32 ALCLowPing = 50;
	UPROPERTY(BlueprintReadOnly)
	int32 ALCHighPing = 250;
	UPROPERTY(BlueprintReadOnly)
	int32 MaxHealth = 100;
	UPROPERTY(BlueprintReadOnly)
	float WalkSpeed = 400.f;
	UPROPERTY(BlueprintReadWrite)
	bool bSkipMOS = false;
	//UPROPERTY(BlueprintReadOnly)
	//TArray<FQOE_Question> QOE_Questions;
	UPROPERTY(BlueprintReadWrite)
	int32 LagInjectionGroup = -1;

	FMatchConfig() {
		//QOE_Questions = TArray<FQOE_Question>();
	}
};

USTRUCT()
struct FPersistentPlayerState{
	GENERATED_BODY()

	UPROPERTY()
	FString PlayerName = "";
	UPROPERTY()
	int32 TeamID = -1;
	UPROPERTY()
	int32 TeamPlayerID = -1;

	FPersistentPlayerState() {}

	FPersistentPlayerState(FString PlayerName, int32 TeamID, int32 TeamPlayerID){
		this->PlayerName = PlayerName;
		this->TeamID = TeamID;
		this->TeamPlayerID = TeamPlayerID;
	}
};

UCLASS()
class FPSSIMULATOR_API UFPSSimulatorGameInstance : public UGameInstance {
	GENERATED_BODY()

public:
	void Init() override;
	void StartNextMatch(bool bRestart = false);
	/* Getter */
	int32 GetCurrentMatch();
	int32 GetNextMatch();
	int32 GetNextRoundTime();
	/* Setter */
	void SetNextMatchConfig(FMatchConfig* NewMatchConfig);

	TArray<FMatchConfig> MatchConfigs;
	// For players to reconnect to the same match
	TArray<AFPSSimulatorPlayerState*> PlayerStates;
	// For carrying over player states to the next match using IP addresses as the key
	TMap<FString, FPersistentPlayerState> PersistentPlayerStates;
	TArray<int32> NextTeamPlayerID;
	FString ClumsyPath = "";
	TArray<FQOE_Question> QOE_Questions;

protected:
	FMatchConfig ParseMatchConfig(TSharedPtr<FJsonObject> Json);
	
	int32 CurrentMatch = 0;
};