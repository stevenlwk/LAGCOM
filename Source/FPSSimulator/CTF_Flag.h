#pragma once

#include "GameFramework/Actor.h"
#include "FPSSimulatorCharacter.h"
#include "CTF_Flag.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FFlagPossessedDelegate, ACTF_Flag*, Flag, AFPSSimulatorCharacter*, PossessedBy);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FFlagUnpossessedDelegate, ACTF_Flag*, Flag, AFPSSimulatorCharacter*, WasPossessedBy);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFlagReturnedDelegate, ACTF_Flag*, Flag);

UCLASS()
class FPSSIMULATOR_API ACTF_Flag : public AActor {
	GENERATED_BODY()
	
public:	
	ACTF_Flag(const FObjectInitializer& ObjectInitializer);
	void BeginPlay() override;
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "CTF Flag")
	FVector GetLocation();

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_TeamID, Category = "CTF Flag")
	int32 TeamID = -1;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "CTF Flag")
	AFPSSimulatorCharacter* PossessedBy = nullptr;
	/* Delegates */
	UPROPERTY(BlueprintAssignable)
	FFlagPossessedDelegate OnFlagPossessedDelegate;
	UPROPERTY(BlueprintAssignable)
	FFlagUnpossessedDelegate OnFlagUnpossessedDelegate;
	UPROPERTY(BlueprintAssignable)
	FFlagReturnedDelegate OnFlagReturnedDelegate;

protected:
	UFUNCTION(NetMulticast, Reliable)
	void CallOnFlagPossessed(AFPSSimulatorCharacter* InPossessedBy);
	UFUNCTION(NetMulticast, Reliable)
	void CallOnFlagUnpossessed(AFPSSimulatorCharacter* WasPossessedBy);
	UFUNCTION()
	void OnFlagHolderDied(AFPSSimulatorCharacter* Victim, AFPSSimulatorCharacter* Killer);
	UFUNCTION()
	void OnFlagHolderRespawned();
	UFUNCTION()
	void OnOverlap(AActor* OverlappedActor, AActor* OtherActor);
	UFUNCTION(BlueprintImplementableEvent)
	void OnRep_TeamID();

	USceneComponent* Scene;
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Mesh;
	FVector SpawnLocation;
	AFPSSimulatorCharacter* WasPossessedBy = nullptr;
};
