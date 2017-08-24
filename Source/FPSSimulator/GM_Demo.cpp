#include "FPSSimulator.h"
#include "GM_Demo.h"
#include "SpawnArea.h"

FTransform AGM_Demo::GetSpawnTransform(int32 TeamID) {
	TArray<ASpawnArea*> Areas = TArray<ASpawnArea*>();

	for(TActorIterator<ASpawnArea> Itr(GetWorld()); Itr; ++Itr) {
		ASpawnArea* Area = *Itr;

		if(Area->Channel == ESpawnAreaChannel::Demo && Area->ID == TeamID) {
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
