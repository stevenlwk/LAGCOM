#include "FPSSimulator.h"
#include "GS_Demo.h"

void AGS_Demo::BeginPlay() {
	Super::BeginPlay();
	if(HasAuthority()) {
		GetWorldTimerManager().ClearTimer(CountdownTimer);
	}
}
