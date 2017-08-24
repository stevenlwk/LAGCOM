#include "FPSSimulator.h"
#include "CTF_Base.h"
#include "FPSSimulatorCharacter.h"
#include "GS_CTF.h"
#include "GM_CTF.h"

/*void AGS_CTF::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}*/

void AGS_CTF::BeginPlay() {
	Super::BeginPlay();
	Flags = TArray<ACTF_Flag*>();
	Flags.Reserve(NumTeams);
	if(HasAuthority()) {
		AGM_CTF* GameMode = Cast<AGM_CTF>(GetWorld()->GetAuthGameMode());

		if(GameMode) {
			for(int32 i = 0; i < NumTeams; i++) {
				SpawnFlag(i);
			}
		}
	}
}

void AGS_CTF::CallOnFlagReturned_Implementation(ACTF_Flag* Flag) {
	OnFlagReturnedDelegate.Broadcast(Flag);
}

void AGS_CTF::CallOnFlagSpawned_Implementation(ACTF_Flag* Flag) {
	OnFlagSpawnedDelegate.Broadcast(Flag);
}

void AGS_CTF::SpawnFlag(int32 TeamID) {
	AGM_CTF* GameMode = Cast<AGM_CTF>(GetWorld()->GetAuthGameMode());

	if(GameMode) {
		ACTF_Flag* Flag = GameMode->SpawnFlag(TeamID);

		Flag->OnFlagReturnedDelegate.AddDynamic(this, &AGS_CTF::OnFlagReturned);
	}
}

void AGS_CTF::OnFlagReturned(ACTF_Flag* Flag) {
	AddScore(Flag->PossessedBy->GetTeamID(), 1);
	CallOnFlagReturned(Flag);
	GetWorld()->DestroyActor(Flag);
	GetWorldTimerManager().SetTimer(SpawnFlagTimer, FTimerDelegate::CreateUObject(this, &AGS_CTF::SpawnFlag, Flag->TeamID), FlagSpawningDelay, false);
}
