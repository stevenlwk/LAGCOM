#pragma once

#include "GameFramework/PlayerState.h"
#include "FPSSimulatorPlayerState.generated.h"

class AFPSSimulatorGameState;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRepScoreDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRepStatsDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRepReadyForNextRoundDelegate);

UENUM(BlueprintType)
enum class EScoreType : uint8 {
	Default,
	Pinned,
	Kill,
	Domination,
	CTF
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnScoreDelegate, EScoreType, Type, const TArray<FString>&, Labels, int32, Score);

UCLASS()
class FPSSIMULATOR_API AFPSSimulatorPlayerState : public APlayerState {
	GENERATED_BODY()

public:
	AFPSSimulatorPlayerState(const FObjectInitializer& ObjectInitializer);
	void BeginPlay() override;
	void Init(int32 TeamID);
	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation, Category = "QOE")
	void SubmitQOE(const TArray<int32>& Scores);
	/* Callback */
	virtual void OnKilled(AFPSSimulatorPlayerState* PlayerState);
	/* Getter */
	bool GetIsAdmin();
	int32 GetTeamID();
	int32 GetDistancesTravelled();
	int32 GetNumKills();
	int32 GetNumDeaths();
	int32 GetNumShotsFired();
	int32 GetNumShotsHit();
	int32 GetNumShotsBehindCovers();
	int32 GetNumShotsBehindCoversReported();
	int32 GetNumShotsDenied();
	int32 GetNumShotsOverruled();
	int32 GetKillStreak();
	/* Setter */
	void AddDistancesTravelled(float Distance);
	void AddNumShotsBehindCovers();
	void AddNumShotsBehindCoversReported();
	void AddNumShotsFired();
	void AddNumShotsDenied();
	void AddNumShotsOverruled();
	void AddNumShotsHit();
	void AddNumShotsHitBy();
	void SetIsAdmin(bool bNewIsAdmin);
	void SetTeamID(int32 TeamID);
	virtual void ScoreKill(AFPSSimulatorPlayerState* VictimPlayerState, bool bIsFriendlyFire = false);
	virtual void ScoreDeath();
	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation, Category = "Game")
	void ToggleReadyForNextRound();
	/* Replication */
	UFUNCTION()
	void OnRep_ReadyForNextRound();
	UFUNCTION()
	void OnRep_Score() override;
	UFUNCTION()
	void OnRepStats();
	/* Scoreboard */
	UFUNCTION(NetMulticast, Reliable)
	void AddToScoreboard();
	UFUNCTION(BlueprintImplementableEvent)
	void AddToScoreboard_BP();
	void RemoveFromScoreboard();
	UFUNCTION(BlueprintImplementableEvent)
	void RemoveFromScoreboard_BP();

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_ReadyForNextRound)
	bool bReadyForNextRound = false;
	UPROPERTY(BlueprintAssignable)
	FOnScoreDelegate OnScoreDelegate;
	int32 TeamPlayerID = -1;
	/* Replication */
	UPROPERTY(BlueprintAssignable)
	FOnRepScoreDelegate OnRepScoreDelegate;
	UPROPERTY(BlueprintAssignable)
	FOnRepReadyForNextRoundDelegate OnRepReadyForNextRoundDelegate;
	UPROPERTY(BlueprintAssignable)
	FOnRepStatsDelegate OnRepStatsDelegate;
	/* QOE */
	TArray<int32> QOE_Scores;

protected:
	void CopyProperties(APlayerState* PlayerState) override;
	UFUNCTION(NetMulticast, Reliable)
	void NotifyScore(EScoreType Type, const TArray<FString>& Labels, int32 InScore);

	UPROPERTY(BlueprintReadOnly, Replicated)
	bool bIsAdmin = false;
	/* Stat */
	UPROPERTY(BlueprintReadOnly, Replicated)
	int32 TeamID = -1;
	float DistancesTravelled = 0;
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRepStats)
	int32 NumKills = 0;
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRepStats)
	int32 NumDeaths = 0;
	UPROPERTY(BlueprintReadOnly, Replicated)
	int32 NumShotsFired = 0;
	UPROPERTY(BlueprintReadOnly, Replicated)
	int32 NumShotsHit = 0;
	int32 NumShotsHitBy = 0;
	int32 NumShotsBehindCovers = 0;
	int32 NumShotsBehindCoversReported = 0;
	int32 NumShotsDenied = 0;
	int32 NumShotsOverruled = 0;
	UPROPERTY(BlueprintReadOnly, Replicated)
	int32 KillStreak = 0;

private:
	AFPSSimulatorGameState* GameState;
};
