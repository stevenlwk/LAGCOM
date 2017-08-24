#include "FPSSimulator.h"
#include "CTF_Flag.h"
#include "CTF_Base.h"
#include "GS_CTF.h"

ACTF_Base::ACTF_Base(const FObjectInitializer& ObjectInitializer) {
	Scene = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, "Scene");
	Scene->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	Area = ObjectInitializer.CreateDefaultSubobject<UBoxComponent>(this, "Area");
	Area->AttachToComponent(Scene, FAttachmentTransformRules::KeepRelativeTransform);
}

void ACTF_Base::BeginPlay() {
	Super::BeginPlay();
	AGS_CTF* GameState = GetWorld()->GetGameState<AGS_CTF>();

	if(!GameState) Destroy();
}

int32 ACTF_Base::GetTeamID() {
	return TeamID;
}
