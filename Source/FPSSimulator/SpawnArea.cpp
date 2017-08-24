#include "FPSSimulator.h"
#include "SpawnArea.h"
#include "FPSSimulatorGameState.h"

ASpawnArea::ASpawnArea(const FObjectInitializer& ObjectInitializer) {
	Scene = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, "Scene");
	Scene->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	Area = ObjectInitializer.CreateDefaultSubobject<UBoxComponent>(this, "Area");
	Area->AttachToComponent(Scene, FAttachmentTransformRules::KeepRelativeTransform);
	Arrow = ObjectInitializer.CreateDefaultSubobject<UArrowComponent>(this, "Arrow");
	Arrow->AttachToComponent(Scene, FAttachmentTransformRules::KeepRelativeTransform);
}

FTransform ASpawnArea::GetRandomSpawn(bool bRandomYaw) {
	FVector Location = FMath::RandPointInBox(Area->Bounds.GetBox());
	FRotator Rotation = GetActorRotation();
	
	if(bRandomYaw) Rotation = FRotator(Rotation.Pitch, FMath::RandRange(0, 360), Rotation.Roll);
	return FTransform(Rotation, FVector(Location.X, Location.Y, GetActorLocation().Z));
}