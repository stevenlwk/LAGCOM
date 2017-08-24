#include "FPSSimulator.h"
#include "GM_CTF.h"
#include "SpawnArea.h"

ACTF_Flag* AGM_CTF::SpawnFlag(int32 TeamID) {
	if(!CTF_FlagClass->IsValidLowLevel()) return NULL;
	TArray<ASpawnArea*> Areas = TArray<ASpawnArea*>();

	for(TActorIterator<ASpawnArea> Itr(GetWorld()); Itr; ++Itr) {
		ASpawnArea* Area = *Itr;

		if(Area->Channel == ESpawnAreaChannel::CTF_Flag && Area->ID == TeamID) {
			Areas.Add(Area);
		}
	}
	if(Areas.Num() > 0) {
		ASpawnArea* Area = Areas[FMath::RandRange(0, Areas.Num() - 1)];
		FTransform Spawn = Area->GetRandomSpawn(true);
		FActorSpawnParameters SpawnParameters;
		ACTF_Flag* Flag;

		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		Flag = GetWorld()->SpawnActor<ACTF_Flag>(CTF_FlagClass, Spawn, SpawnParameters);
		Flag->TeamID = TeamID;
		return Flag;
	} else {
		return NULL;
	}
}
