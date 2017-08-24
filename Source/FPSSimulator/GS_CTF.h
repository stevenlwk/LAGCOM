#pragma once

#include "FPSSimulatorGameState.h"
#include "CTF_Flag.h"
#include "GS_CTF.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFlagSpawnedDelegate, ACTF_Flag*, Flag);

UCLASS()
class FPSSIMULATOR_API AGS_CTF : public AFPSSimulatorGameState {
	GENERATED_BODY()

public:
	void BeginPlay() override;
	UFUNCTION(NetMulticast, Reliable)
	void CallOnFlagReturned(ACTF_Flag* Flag);
	UFUNCTION(Client, Reliable)
	void CallOnFlagSpawned(ACTF_Flag* Flag);

	UPROPERTY(BlueprintReadOnly, Category = "Capture the Flag")
	TArray<ACTF_Flag*> Flags;
	UPROPERTY(BlueprintAssignable)
	FFlagReturnedDelegate OnFlagReturnedDelegate;
	UPROPERTY(BlueprintAssignable)
	FFlagSpawnedDelegate OnFlagSpawnedDelegate;
	/* Scores */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Score")
	int32 PickupScore = 100;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Score")
	int32 DeliverScore = 500;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Score")
	int32 CarrierKillScore = 50;

protected:
	UFUNCTION()
	void SpawnFlag(int32 TeamID);
	UFUNCTION()
	void OnFlagReturned(ACTF_Flag* Flag);

	FTimerHandle SpawnFlagTimer;
	UPROPERTY(EditDefaultsOnly, Category = "Capture the Flag")
	float FlagSpawningDelay = 10.f;
};
