#pragma once

#include "GameFramework/HUD.h"
#include "Blueprint/UserWidget.h"
#include "FPSSimulatorHUD.generated.h"

class AFPSSimulatorPlayerController;

UCLASS()
class FPSSIMULATOR_API AFPSSimulatorHUD : public AHUD {
	GENERATED_BODY()

public:
	void BeginPlay() override;

	UPROPERTY(BlueprintReadOnly, Category = "UI")
	UUserWidget* GameModeHUD;
};
