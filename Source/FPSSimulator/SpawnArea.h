#pragma once

#include "GameFramework/Actor.h"
#include "SpawnArea.generated.h"

UENUM(BlueprintType)
enum class ESpawnAreaChannel : uint8 {
	Player,
	CTF_Flag,
	Demo
};

UCLASS()
class FPSSIMULATOR_API ASpawnArea : public AActor {
	GENERATED_BODY()

public:
	ASpawnArea(const FObjectInitializer& ObjectInitializer);
	FTransform GetRandomSpawn(bool bRandomYaw = false);

	UPROPERTY(EditAnywhere)
	UBoxComponent* Area;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ESpawnAreaChannel Channel = ESpawnAreaChannel::Player;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 ID = 0;

protected:
	USceneComponent* Scene;
	UArrowComponent* Arrow;
};