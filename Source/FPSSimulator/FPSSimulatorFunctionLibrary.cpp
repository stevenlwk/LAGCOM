#include "FPSSimulator.h"
#include "Paths.h"
#include "FPSSimulatorFunctionLibrary.h"

bool UFPSSimulatorFunctionLibrary::ProjectWorldToScreen(APlayerController const* Player, const FVector& WorldPosition, FVector2D& ScreenPosition, bool& bTargetBehindCamera, bool bPlayerViewportRelative /*= false*/) {
	FVector Projected;
	bool bSuccess = false;

	ULocalPlayer* const LP = Player ? Player->GetLocalPlayer() : nullptr;
	if(LP && LP->ViewportClient) {
		// get the projection data
		FSceneViewProjectionData ProjectionData;
		if(LP->GetProjectionData(LP->ViewportClient->Viewport, eSSP_FULL, /*out*/ ProjectionData)) {
			const FMatrix ViewProjectionMatrix = ProjectionData.ComputeViewProjectionMatrix();
			const FIntRect ViewRectangle = ProjectionData.GetConstrainedViewRect();

			FPlane Result = ViewProjectionMatrix.TransformFVector4(FVector4(WorldPosition, 1.f));
			if(Result.W < 0.f) {
				bTargetBehindCamera = true;
			} else if(Result.W == 0.f) { // Prevent division by zero
				Result.W = 1.f;
			}

			const float RHW = 1.f / FMath::Abs(Result.W);
			Projected = FVector(Result.X, Result.Y, Result.Z) * RHW;

			// Normalize to 0..1 UI Space
			const float NormX = (Projected.X / 2.f) + 0.5f;
			const float NormY = 1.f - (Projected.Y / 2.f) - 0.5f;

			Projected.X = (float)ViewRectangle.Min.X + (NormX * (float)ViewRectangle.Width());
			Projected.Y = (float)ViewRectangle.Min.Y + (NormY * (float)ViewRectangle.Height());

			bSuccess = true;
			ScreenPosition = FVector2D(Projected.X, Projected.Y);

			if(bPlayerViewportRelative) {
				ScreenPosition -= FVector2D(ProjectionData.GetConstrainedViewRect().Min);
			}
			return bSuccess;
		}
	}

	ScreenPosition = FVector2D::ZeroVector;
	return bSuccess;
}

FString UFPSSimulatorFunctionLibrary::GetGameDir() {
	return FPaths::GameDir();
}

FString UFPSSimulatorFunctionLibrary::GetGameLogDir() {
	return FPaths::GameLogDir();
}

UTexture2D* UFPSSimulatorFunctionLibrary::GetLoadingScreenImage(TSubclassOf<AFPSSimulatorGameState> GameStateClass) {
	return GameStateClass->GetDefaultObject<AFPSSimulatorGameState>()->LoadingScreenImage;
}

bool UFPSSimulatorFunctionLibrary::SaveTextToFile(FString Directory, FString FileName, FString Text, bool AllowOverwriting /*= false*/) {
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	if(PlatformFile.CreateDirectoryTree(*Directory)) {
		FString Path = Directory + "/" + FileName;

		if(AllowOverwriting || !PlatformFile.FileExists(*Path)) {
			return FFileHelper::SaveStringToFile(Text, *Path);
		}
	}
	return false;
}

TSharedPtr<FJsonObject> UFPSSimulatorFunctionLibrary::CreateJson() {
	return MakeShareable(new FJsonObject());
}

FString UFPSSimulatorFunctionLibrary::JsonToString(TSharedPtr<FJsonObject> JsonObject) {
	FString String;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&String);

	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
	return String;
}

TSharedPtr<FJsonObject> UFPSSimulatorFunctionLibrary::StringToJsonObject(FString String) {
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(String);
	FJsonSerializer::Deserialize(JsonReader, JsonObject);

	return JsonObject;
}

TSharedPtr<FJsonValue> UFPSSimulatorFunctionLibrary::StringToJsonValue(FString String) {
	TSharedPtr<FJsonValue> JsonValue;
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(String);
	FJsonSerializer::Deserialize(JsonReader, JsonValue);

	return JsonValue;
}

TArray<TSharedPtr<FJsonValue>> UFPSSimulatorFunctionLibrary::StringToJsonValues(FString String) {
	TArray<TSharedPtr<FJsonValue>> JsonValues = TArray<TSharedPtr<FJsonValue>>();
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(String);
	FJsonSerializer::Deserialize(JsonReader, JsonValues);

	return JsonValues;
}

void UFPSSimulatorFunctionLibrary::SetCurrentCulture(const FString& Name) {
	FInternationalization::Get().SetCurrentCulture(Name);
}
