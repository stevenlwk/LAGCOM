#include "FPSSimulator.h"
#include "DynamicTextureManager.h"

ADynamicTextureManager::ADynamicTextureManager() {
	//PrimaryActorTick.bCanEverTick = true;
}

void ADynamicTextureManager::AddCircle(FCircle Circle) {
	FColor Color = Circle.Color.ToFColor(false);

	for(int32 i = Circle.Y - Circle.Radius; i < Circle.Y + Circle.Radius; i++) {
		int32 X = i*SizeX;
		for(int32 j = Circle.X - Circle.Radius; j < Circle.X + Circle.Radius; j++) {
			if(FMath::Sqrt(FMath::Square(j - Circle.X) + FMath::Square(i - Circle.Y)) <= Circle.Radius && X + j < Data.Num()) {
				Data[X + j] = Color;
			}
		}
	}
}

void ADynamicTextureManager::AddRect(FRect Rect) {
	FColor Color = Rect.Color.ToFColor(false);

	for (int32 i = Rect.Y; i < Rect.Y + Rect.Height; i++) {
		int32 X = i*SizeX;
		for (int32 j = Rect.X; j < Rect.X + Rect.Width; j++) {
			if(X + j < Data.Num()) {
				Data[X + j] = Color;
			}
		}
	}
}

void ADynamicTextureManager::CreateTexture(int32 SizeX, int32 SizeY, FColor Color) {
	Texture = UTexture2D::CreateTransient(SizeX, SizeY);
	//Texture->CompressionSettings = TextureCompressionSettings::TC_Default;
	Texture->SRGB = 0;
	Texture->AddToRoot();
	Texture->UpdateResource();
	this->SizeX = SizeX;
	this->SizeY = SizeY;
	Data.Init(Color, SizeX*SizeY);
	UpdateTextureRegion = new FUpdateTextureRegion2D(0, 0, 0, 0, (uint32)SizeX, (uint32)SizeY);
}

void ADynamicTextureManager::UpdateTexture(int32 NewSizeX, int32 NewSizeY) {
	Texture->UpdateTextureRegions(0, 1, UpdateTextureRegion, 4 * SizeX, 4, (uint8*)Data.GetData());
}
