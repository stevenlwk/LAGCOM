#pragma once

#include "GameFramework/Actor.h"
#include "CTF_Base.generated.h"

UCLASS()
class FPSSIMULATOR_API ACTF_Base : public AActor {
	GENERATED_BODY()
	
public:	
	ACTF_Base(const FObjectInitializer& ObjectInitializer);
	void BeginPlay() override;
	int32 GetTeamID();

protected:
	USceneComponent* Scene;
	UPROPERTY(EditAnywhere)
	UBoxComponent* Area;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Capture the Flag")
	int32 TeamID = -1;
};
