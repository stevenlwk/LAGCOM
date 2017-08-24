#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "FPSSimulatorGameState.h"
#include "FPSSimulatorFunctionLibrary.generated.h"

UCLASS()
class FPSSIMULATOR_API UFPSSimulatorFunctionLibrary : public UBlueprintFunctionLibrary {
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Utilities")
	static bool ProjectWorldToScreen(APlayerController const* Player, const FVector& WorldPosition, FVector2D& ScreenPosition, bool& bTargetBehindCamera, bool bPlayerViewportRelative = false);
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "File")
	static FString GetGameDir();
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "File")
	static FString GetGameLogDir();
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UI")
	static UTexture2D* GetLoadingScreenImage(TSubclassOf<AFPSSimulatorGameState> GameStateClass);
	UFUNCTION(BlueprintCallable, Category = "File")
	static bool SaveTextToFile(FString Directory, FString FileName, FString Text, bool AllowOverwriting = false);
	// JSON
	static TSharedPtr<FJsonObject> CreateJson();
	static FString JsonToString(TSharedPtr<FJsonObject> JsonObject);
	static TSharedPtr<FJsonObject> StringToJsonObject(FString String);
	static TSharedPtr<FJsonValue> StringToJsonValue(FString String);
	static TArray<TSharedPtr<FJsonValue>> StringToJsonValues(FString String);
	// Localization
	UFUNCTION(BlueprintCallable, Category = "Localization")
	static void SetCurrentCulture(const FString& Name);
};
