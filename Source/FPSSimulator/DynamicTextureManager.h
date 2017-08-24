#pragma once

#include "GameFramework/Actor.h"
#include "DynamicTextureManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPostInitDelegate);

USTRUCT()
struct FRect {
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 X = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Y = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Width = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Height = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor Color = FLinearColor(0, 0, 0, 0);

	FRect() {}
};

USTRUCT()
struct FCircle {
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 X = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Y = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Radius = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor Color = FLinearColor(0, 0, 0, 0);

	FCircle() {}
};

UCLASS()
class FPSSIMULATOR_API ADynamicTextureManager : public AActor {
	GENERATED_BODY()

public:
	ADynamicTextureManager();
	UFUNCTION(BlueprintCallable, Category = "Texture")
	void AddCircle(FCircle Circle);
	UFUNCTION(BlueprintCallable, Category="Texture")
	void AddRect(FRect Rect);
	UFUNCTION(BlueprintCallable, Category = "Texture")
	void CreateTexture(int32 SizeX, int32 SizeY, FColor Color);
	UFUNCTION(BlueprintCallable, Category = "Texture")
	void UpdateTexture(int32 NewSizeX = -1, int32 NewSizeY = -1);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UTexture2D* Texture;

protected:
	int32 SizeX;
	int32 SizeY;
	TArray<FCircle> Circles;
	TArray<FRect> Rects;

private:
	UPROPERTY()
	TArray<FColor> Data;
	FUpdateTextureRegion2D* UpdateTextureRegion;
};
