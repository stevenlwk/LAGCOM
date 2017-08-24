#pragma once

#include "GameFramework/GameState.h"
#include "Blueprint/UserWidget.h"
#include "LevelSequence.h"
#include "FPSSimulatorGameInstance.h"
#include "FPSSimulatorPlayerState.h"
#include "FPSSimulatorGameState.generated.h"

class AFPSSimulatorPlayerController;

UENUM(BlueprintType)
enum class EMatchState : uint8 {
	InProgress,
	PostMatch,
	PostMatchCountdown
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FScoreChangedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FStateChangedDelegate, EMatchState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllReadyDelegate);

UCLASS()
class FPSSIMULATOR_API AFPSSimulatorGameState : public AGameState {
	GENERATED_BODY()

public:
	virtual void AddPlayerState(class APlayerState* PlayerState);
	void BeginPlay() override;
	FString GetFormattedTime(int32 Time);
	virtual void InitPlayer(AFPSSimulatorPlayerState* PlayerState);
	void StartNextRound(bool bRestart = false);
	void AddScore(int32 TeamID, float Score);
	/* Callback */
	virtual void OnKilled(AFPSSimulatorPlayerState* DamageCauserPlayerState, AFPSSimulatorPlayerState* PlayerState);
	virtual void OnPlayerJoined(AFPSSimulatorPlayerController* PC);
	void OnNumPlayersReadyForNextRoundUpdated();
	/* Getter */
	int32 GetNumPlayers(int32 TeamID = -1);
	int32 GetNumTeams();
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Game")
	float GetScore(int TeamID);
	FString GetTeamName(int32 TeamID);
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Game")
	FString GetFormattedRoundTime();
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Game")
	FString GetFormattedRoundIntervalTime();
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Game")
	FString GetFormattedNextRoundTime();
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Game")
	FString GetFormattedNextRoundIntervalTime();
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Game")
	virtual int32 GetWinningTeam();
	// Networking
	int32 GetALCLowPing();
	int32 GetALCHighPing();
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Networking")
	ELagCompMethod GetLagCompMethod();
	/* Replication */
	UFUNCTION()
	void OnRep_State();
	UFUNCTION()
	void OnRep_TeamScores();
	/* Setter */
	void AddNumQOE_Submitted();
	void SetLagCompMethod(ELagCompMethod NewLagCompMethod);
	void SetMatchConfig(FMatchConfig NewMatchConfig);
	void SetRoundTime(int32 NewRoundTime);
	void SetRoundIntervalTime(int32 NewRoundIntervalTime);
	void SetNextMatchConfig(FMatchConfig NewNextMatchConfig);
	void SetNextRoundTime(int32 NewNextRoundTime);
	void SetNextRoundIntervalTime(int32 NewNextRoundIntervalTime);
	void SetNextALCLowPing(int32 NewALCLowPing);
	void SetNextALCHighPing(int32 NewALCHighPing);
	void SetNextMaxHealth(int32 NewMaxHealth);
	void SetNextWalkSpeed(float NewWalkSpeed);
	void SetScore(int32 TeamID, float NewScore);
	void SetState(EMatchState NewState);
	
	UPROPERTY(BlueprintReadWrite, Replicated)
	FMatchConfig MatchConfig;
	UPROPERTY(BlueprintReadWrite, Replicated)
	FMatchConfig NextMatchConfig;
	UPROPERTY(EditDefaultsOnly, Replicated, Category = "Networking")
	float TraceDistance = 20000.f;
	UPROPERTY(BlueprintReadOnly, Replicated)
	TArray<FQOE_Question> QOE_Questions;
	/* Debug */
	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	bool bShowTraceLines = false;
	/* Delegates */
	UPROPERTY(BlueprintAssignable)
	FScoreChangedDelegate OnScoreChanged;
	UPROPERTY(BlueprintAssignable)
	FStateChangedDelegate OnStateChanged;
	UPROPERTY(BlueprintAssignable)
	FOnAllReadyDelegate OnAllReady;
	/* Game state */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Game")
	TArray<FLinearColor> TeamColors;
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Game")
	TArray<FString> TeamNames;
	/* Scores */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Score")
	int32 ScorePerKill = 100;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Score")
	int32 DoubleKillBonus = 30;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Score")
	int32 TripleKillBonus = 50;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Score")
	int32 MultiKillBonus = 100;
	/* UI */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UUserWidget> HUDClass;
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "UI")
	ULevelSequence* EndRoundSequence;
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "UI")
	UTexture2D* LoadingScreenImage;
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "UI")
	UTexture2D* MapTexture;
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "UI")
	float MapUnitPerPixel = 5.f;
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "UI")
	float MapZoomLevel = 0.5f;

protected:
	UFUNCTION(NetMulticast, Reliable)
	void CallOnAllReady();
	void CountDownRoundTime();
	void CountdownToNextRound();
	void EndRound();
	void StartCountdownToNextRound();

	UFPSSimulatorGameInstance* GameInstance;
	FTimerHandle CountdownTimer;
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_State, Category = "Game")
	EMatchState State;
	int32 NumQOE_Submitted = 0;
	/* Game state */
	UPROPERTY(EditDefaultsOnly, Category = "Game")
	int32 NumTeams = 2;

private:
	UPROPERTY(Transient, ReplicatedUsing = OnRep_TeamScores)
	TArray<float> TeamScores;
};