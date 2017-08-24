#pragma once

#include "GameFramework/Actor.h"
#include "FPSSimulatorCharacter.h"
#include "FPSSimulatorGameState.h"
#include "FPSSimulatorPlayerController.h"
#include "CapturePoint.generated.h"

UENUM(BlueprintType)
enum class ECapturePointState : uint8 {
	Neutral,
	Capturing,
	Captured,
	Neutralizing,
	Reverting
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStateChangedDelegate, ECapturePointState, OldState, ECapturePointState, NewState);

UCLASS()
class FPSSIMULATOR_API ACapturePoint : public AActor {
	GENERATED_BODY()
	
public:	
	ACapturePoint(const FObjectInitializer& ObjectInitializer);
	virtual void BeginPlay() override;
	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	UFUNCTION()
	void OnPlayerDieInArea(AFPSSimulatorCharacter* Victim, AFPSSimulatorCharacter* Killer);
	void UpdateNumPlayersInArea();
	/* Getter */
	int32 GetNumPlayersInArea(int32 TeamID);
	const TSet<AFPSSimulatorCharacter*> GetPlayersInArea(int32 TeamID);
	int32 GetTotalNumPlayersInArea();
	int32 GetMajorityTeam();
	int32 GetOldMajorityTeam();
	/* Setter */
	void SetState(ECapturePointState NewState);
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Capture Point")
	FString letter = "";
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Capture Point")
	float Control = 0;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Capture Point")
	int32 OwnedBy = -1;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Capture Point")
	int32 TeamWithProgress = -1;
	/* Delegate */
	UPROPERTY(BlueprintAssignable)
	FOnStateChangedDelegate OnStateChangedDelegate;

protected:
	UFUNCTION(NetMulticast, Reliable)
	void CallOnStateChanged(ECapturePointState OldState, ECapturePointState NewState);

	USceneComponent* Scene;
	UPROPERTY(EditAnywhere)
	UBoxComponent* Area;
	AFPSSimulatorGameState* GameState;
	ECapturePointState State = ECapturePointState::Neutral;
	TArray<int32> NumPlayersInArea;
	int32 TotalNumPlayersInArea = 0;
	int32 MajorityTeam = -1;
};
