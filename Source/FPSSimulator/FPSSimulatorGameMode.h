#pragma once

#include "GameFramework/GameMode.h"
#include "FPSSimulatorCharacter.h"
#include "FPSSimulatorGameInstance.h"
#include "FPSSimulatorGameState.h"
#include "FPSSimulatorPlayerController.h"
#include "FPSSimulatorGameMode.generated.h"

UENUM(BlueprintType)
enum class EEventType : uint8 {
	None,
	Begin,
	End,
	Fire,
	QOE
};

UENUM()
enum class EFireLogEventType : uint8 {
	None,
	ServerDetect,
	ServerConfirm,
	ClientSAC,
	ClientDeny,
	ServerDeny,
	ServerOverrule
};

UCLASS()
class UBaseLogEvent : public UObject {
	GENERATED_BODY()

public:
	void Init(int32 Tick, EEventType EventType) {
		this->Tick = Tick;
		Event = EventType;
	}

	virtual TSharedPtr<FJsonObject> ToJsonObject() {
		TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
		const UEnum* Enum = FindObject<UEnum>(ANY_PACKAGE, TEXT("EEventType"), true);

		JsonObject->SetStringField("Event", Enum->GetEnumName(Enum->GetValueByIndex((uint8)Event)));
		JsonObject->SetNumberField("Tick", Tick);
		return JsonObject;
	}

	TSharedPtr<FJsonObject> VectorToJsonObject(FVector Vector) {
		TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

		JsonObject->SetNumberField("X", Vector.X);
		JsonObject->SetNumberField("Y", Vector.Y);
		JsonObject->SetNumberField("Z", Vector.Z);
		return JsonObject;
	}

	UPROPERTY()
		int32 Tick = 0;
	UPROPERTY()
		EEventType Event = EEventType::None;
};

UCLASS()
class UStartLogEvent : public UBaseLogEvent {
	GENERATED_BODY()

public:
	void Init(int32 Tick, FString Scenario, FString Map, FString GameMode, ELagCompMethod LagCompMethod) {
		Super::Init(Tick, EEventType::Begin);
		this->Scenario = Scenario;
		this->Map = Map;
		this->GameMode = GameMode;
		this->LagCompMethod = LagCompMethod;
	}

	TSharedPtr<FJsonObject> ToJsonObject() override {
		TSharedPtr<FJsonObject> JsonObject = Super::ToJsonObject();
		const UEnum* Enum = FindObject<UEnum>(ANY_PACKAGE, TEXT("ELagCompMethod"), true);

		JsonObject->SetStringField("Scenario", Scenario);
		JsonObject->SetStringField("Map", Map);
		JsonObject->SetStringField("GameMode", GameMode);
		JsonObject->SetStringField("LagCompMethod", Enum->GetEnumName(Enum->GetValueByIndex((uint8)LagCompMethod)));
		return JsonObject;
	}

	UPROPERTY()
	FString Scenario = "";
	UPROPERTY()
	FString Map = "";
	UPROPERTY()
	FString GameMode = "";
	UPROPERTY()
	ELagCompMethod LagCompMethod = ELagCompMethod::None;
};

UCLASS()
class UEndLogEvent : public UBaseLogEvent {
	GENERATED_BODY()

public:
	void Init(int32 Tick, AFPSSimulatorPlayerState* PlayerState, int32 TeamPlayerID){
		Super::Init(Tick, EEventType::End);
		this->PlayerState = PlayerState;
		this->TeamPlayerID = TeamPlayerID;
	}

	TSharedPtr<FJsonObject> ToJsonObject() override {
		TSharedPtr<FJsonObject> JsonObject = Super::ToJsonObject();

		JsonObject->SetStringField("Player", PlayerState->PlayerName);
		JsonObject->SetNumberField("Team", PlayerState->GetTeamID());
		JsonObject->SetNumberField("TeamPlayerID", TeamPlayerID);
		JsonObject->SetNumberField("DistancesTravelled", PlayerState->GetDistancesTravelled());
		JsonObject->SetNumberField("NumKills", PlayerState->GetNumKills());
		JsonObject->SetNumberField("NumDeaths", PlayerState->GetNumDeaths());
		JsonObject->SetNumberField("NumShotsFired", PlayerState->GetNumShotsFired());
		JsonObject->SetNumberField("NumShotsHit", PlayerState->GetNumShotsHit());
		JsonObject->SetNumberField("NumShotsBehindCovers", PlayerState->GetNumShotsBehindCovers());
		JsonObject->SetNumberField("NumShotsBehindCoversReported", PlayerState->GetNumShotsBehindCoversReported());
		JsonObject->SetNumberField("NumShotsDenied", PlayerState->GetNumShotsDenied());
		JsonObject->SetNumberField("NumShotsOverruled", PlayerState->GetNumShotsOverruled());
		return JsonObject;
	}

	UPROPERTY()
	AFPSSimulatorPlayerState* PlayerState;

	UPROPERTY()
	int32 TeamPlayerID = -1;
};

UCLASS()
class UFireLogEvent : public UBaseLogEvent {
	GENERATED_BODY()

public:
	void Init(int32 Tick, EFireLogEventType Type, ELagCompMethod LagCompMethod, AFPSSimulatorPlayerState* ShooterPS, AFPSSimulatorPlayerState* VictimPS, FVector HitFrom, FVector HitTo, float VictimRemainingHealth, float RollbackDist = 0) {
		Super::Init(Tick, EEventType::Fire);
		this->Type = Type;
		this->LagCompMethod = LagCompMethod;
		this->ShooterName = ShooterPS->PlayerName;
		this->VictimName = VictimPS->PlayerName;
		this->ShooterPing = ShooterPS->ExactPing;
		this->VictimPing = VictimPS->ExactPing;
		this->HitFrom = HitFrom;
		this->HitTo = HitTo;
		this->VictimRemainingHealth = VictimRemainingHealth;
		this->RollbackDist = RollbackDist;
	}

	TSharedPtr<FJsonObject> ToJsonObject() override {
		TSharedPtr<FJsonObject> JsonObject = Super::ToJsonObject();
		const UEnum* Enum = FindObject<UEnum>(ANY_PACKAGE, TEXT("EFireLogEventType"), true);

		JsonObject->SetStringField("Type", Enum->GetEnumName(Enum->GetValueByIndex((uint8)Type)));
		JsonObject->SetStringField("Shooter", ShooterName);
		JsonObject->SetStringField("Victim", VictimName);
		JsonObject->SetNumberField("ShooterPing", ShooterPing);
		JsonObject->SetNumberField("VictimPing", VictimPing);
		JsonObject->SetObjectField("HitFrom", Super::VectorToJsonObject(HitFrom));
		JsonObject->SetObjectField("HitTo", Super::VectorToJsonObject(HitTo));
		JsonObject->SetNumberField("VictimRemainingHealth", VictimRemainingHealth);
		if(Type == EFireLogEventType::ServerConfirm || Type == EFireLogEventType::ServerDetect) {
			JsonObject->SetNumberField("RollbackDist", RollbackDist);
		}
		return JsonObject;
	}

	UPROPERTY()
	EFireLogEventType Type = EFireLogEventType::None;
	UPROPERTY()
	ELagCompMethod LagCompMethod = ELagCompMethod::None;
	UPROPERTY()
	FString ShooterName = "";
	UPROPERTY()
	FString VictimName = "";
	UPROPERTY()
	float ShooterPing = 0;
	UPROPERTY()
	float VictimPing = 0;
	UPROPERTY()
	FVector HitFrom = FVector::ZeroVector;
	UPROPERTY()
	FVector HitTo = FVector::ZeroVector;
	UPROPERTY()
	float VictimRemainingHealth = 0;
	UPROPERTY()
	float RollbackDist = 0;
};

UCLASS()
class UQOELogEvent : public UBaseLogEvent {
	GENERATED_BODY()

public:
	void Init(int32 Tick, AFPSSimulatorPlayerState* PlayerState) {
		Super::Init(Tick, EEventType::QOE);
		Player = PlayerState->PlayerName;
		Scores = PlayerState->QOE_Scores;
	}

	TSharedPtr<FJsonObject> ToJsonObject() override {
		TSharedPtr<FJsonObject> JsonObject = Super::ToJsonObject();

		JsonObject->SetStringField("Player", Player);
		for(int32 i = 0; i < Scores.Num(); i++) {
			JsonObject->SetNumberField(FString::FromInt(i), Scores[i]);
		}
		return JsonObject;
	}

	UPROPERTY()
	FString Player = "";

	UPROPERTY()
	TArray<int32> Scores;
};

UCLASS()
class FPSSIMULATOR_API AFPSSimulatorGameMode : public AGameMode {
	GENERATED_BODY()

public:
	AFPSSimulatorGameMode(const FObjectInitializer& ObjectInitializer);
	void BeginPlay() override;
	void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	APlayerController* Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	void PostLogin(APlayerController* NewPlayer) override;
	void Tick(float DeltaSeconds) override;
	virtual void Respawn(AFPSSimulatorCharacter* Character);
	void StartNextRound(bool bRestart = false);
	/* Getter */
	bool GetIsDebugMode();
	int32 GetNumTeams();
	int32 GetTickCount();
	int32 GetTickRate();
	/* Log */
	void LogStartEvent(FString Scenario, FString Map, FString GameMode, ELagCompMethod LagCompMethod);
	void LogEndEvent(AFPSSimulatorPlayerState* PlayerState);
	void LogFireEvent(EFireLogEventType Type, ELagCompMethod LagCompMethod, AFPSSimulatorPlayerState* ShooterPS, AFPSSimulatorPlayerState* VictimPS, FVector HitFrom, FVector HitTo, float VictimRemainingHealth, float RollbackDist = 0);
	void LogQOEEvent(AFPSSimulatorPlayerState* PlayerState);
	void SaveLogs();

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class AFPSSimulatorCharacter> CharacterClass;
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class ASpectatorPawn> SpectatorPawnClass;
	/* Customization */
	UPROPERTY(EditDefaultsOnly)
	bool bFriendlyFire = false;
	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	bool bShowHitboxes = false;

protected:
	virtual FTransform GetSpawnTransform(int32 TeamID);
	
	UFPSSimulatorGameInstance* GameInstance;
	AFPSSimulatorGameState* GameState;
	int32 TickCount = 0;
	int32 TickRate = 0;
	/* Debug */
	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	bool bDebugMode = false;
	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	bool bDebugAsAdmin = false;
	
private:
	UPROPERTY(Instanced)
	TArray<UBaseLogEvent*> Logs;
	int32 PlayerNameSuffix = 1;
};