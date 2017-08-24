#pragma once

#include "AIController.h"
#include "FPSSimulatorAIController.generated.h"

UCLASS()
class FPSSIMULATOR_API AFPSSimulatorAIController : public AAIController {
	GENERATED_BODY()

		AFPSSimulatorAIController(const FObjectInitializer& ObjectInitializer);

	UBehaviorTreeComponent* BT;
	UBlackboardComponent* Blackboard;

public:
	virtual void Possess(APawn* InPawn) override;
	virtual void UnPossess() override;
};