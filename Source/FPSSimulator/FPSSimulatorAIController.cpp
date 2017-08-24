#include "FPSSimulator.h"
#include "FPSSimulatorAIController.h"
#include "FPSSimulatorCharacter.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

AFPSSimulatorAIController::AFPSSimulatorAIController(const FObjectInitializer& ObjectInitializer) {
	BT = ObjectInitializer.CreateDefaultSubobject<UBehaviorTreeComponent>(this, TEXT("BT"));
	Blackboard = ObjectInitializer.CreateDefaultSubobject<UBlackboardComponent>(this, TEXT("Blackboard"));
}

void AFPSSimulatorAIController::Possess(APawn* InPawn) {
	Super::Possess(InPawn);
	AFPSSimulatorCharacter* Character = Cast<AFPSSimulatorCharacter>(InPawn);

	if (Character) {
		if (Character->BehaviorTree->BlackboardAsset) {
			Blackboard->InitializeBlackboard(*Character->BehaviorTree->BlackboardAsset);
		}
		BT->StartTree(*Character->BehaviorTree);
	}
}

void AFPSSimulatorAIController::UnPossess() {
	Super::UnPossess();
	BT->StopTree();
}
