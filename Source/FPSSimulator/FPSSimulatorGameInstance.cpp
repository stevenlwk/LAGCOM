#include "FPSSimulator.h"
#include "FPSSimulatorFunctionLibrary.h"
#include "FPSSimulatorGameState.h"
#include "FPSSimulatorGameInstance.h"

void UFPSSimulatorGameInstance::Init() {
	Super::Init();
	FString MatchConfigText;

	if(FFileHelper::LoadFileToString(MatchConfigText, *(FPaths::GameSavedDir() + "Config/WindowsServer/MatchSettings.ini"))) {
		TSharedPtr<FJsonValue> JsonValue = UFPSSimulatorFunctionLibrary::StringToJsonValue(MatchConfigText);
		const TArray<TSharedPtr<FJsonValue>> JsonValues = JsonValue->AsArray();
		
		MatchConfigs = TArray<FMatchConfig>();
		QOE_Questions = TArray<FQOE_Question>();
#if !UE_EDITOR
		FMatchConfig InitMatchConfig = FMatchConfig(); // Used to execute the first match config at server startup

		InitMatchConfig.Scenario = "Init";
		InitMatchConfig.RoundTime = 0;
		InitMatchConfig.RoundIntervalTime = 0;
		InitMatchConfig.bSkipMOS = true;
		MatchConfigs.Add(InitMatchConfig);
#endif
		for(int32 i = 0; i < JsonValues.Num(); i++) {
			MatchConfigs.Add(ParseMatchConfig(JsonValues[i]->AsObject()));
		}
	}
	
	PlayerStates = TArray<AFPSSimulatorPlayerState*>();
	PersistentPlayerStates = TMap<FString, FPersistentPlayerState>();
	NextTeamPlayerID = TArray<int32>();
	NextTeamPlayerID.Init(0, 2);
}

void UFPSSimulatorGameInstance::StartNextMatch(bool bRestart /*= false*/) {
	if(MatchConfigs[CurrentMatch].Scenario == "Init" && MatchConfigs[CurrentMatch].RoundTime == 0 && MatchConfigs[CurrentMatch].RoundIntervalTime == 0) {
		MatchConfigs.RemoveAt(CurrentMatch);
	} else {
		PlayerStates.Empty();
		if(!bRestart)
			CurrentMatch = (CurrentMatch + 1) % MatchConfigs.Num();
	}
}

int32 UFPSSimulatorGameInstance::GetCurrentMatch() {
	return CurrentMatch;
}

int32 UFPSSimulatorGameInstance::GetNextMatch() {
	return (CurrentMatch + 1) % MatchConfigs.Num();
}

void UFPSSimulatorGameInstance::SetNextMatchConfig(FMatchConfig* NewMatchConfig) {
	MatchConfigs[GetNextMatch()] = *NewMatchConfig;
}

FMatchConfig UFPSSimulatorGameInstance::ParseMatchConfig(TSharedPtr<FJsonObject> Json) {
	FMatchConfig MatchConfig = FMatchConfig();
	const TArray<TSharedPtr<FJsonValue>>* QOE_Questions;

	if(Json->HasTypedField<EJson::String>("ClumsyPath"))
		ClumsyPath = Json->GetStringField("ClumsyPath");
	if(Json->HasTypedField<EJson::String>("Scenario"))
		MatchConfig.Scenario = Json->GetStringField("Scenario");
	if(Json->HasTypedField<EJson::String>("GameMode"))
		MatchConfig.GameMode = Json->GetStringField("GameMode");
	if(Json->HasTypedField<EJson::String>("Map"))
		MatchConfig.Map = Json->GetStringField("Map");
	if(Json->HasTypedField<EJson::Number>("RoundTime"))
		MatchConfig.RoundTime = Json->GetIntegerField("RoundTime");
	if(Json->HasTypedField<EJson::Number>("RoundIntervalTime"))
		MatchConfig.RoundIntervalTime = Json->GetIntegerField("RoundIntervalTime");
	if(Json->HasTypedField<EJson::String>("LagCompMethod")) {
		const UEnum* Enum = FindObject<UEnum>(ANY_PACKAGE, TEXT("ELagCompMethod"), true);
		FString LagCompMethod = Json->GetStringField("LagCompMethod");
		int32 LagCompMethodIndex = Enum->GetValueByName(FName(*LagCompMethod));

		if(LagCompMethodIndex != INDEX_NONE)
			MatchConfig.LagCompMethod = static_cast<ELagCompMethod>(Enum->GetValueByIndex(LagCompMethodIndex));
	}
	if(Json->HasTypedField<EJson::Number>("ALCLowPing"))
		MatchConfig.ALCLowPing = Json->GetIntegerField("ALCLowPing");
	if(Json->HasTypedField<EJson::Number>("ALCHighPing"))
		MatchConfig.ALCHighPing = Json->GetIntegerField("ALCHighPing");
	if(Json->HasTypedField<EJson::Number>("MaxHealth"))
		MatchConfig.MaxHealth = Json->GetIntegerField("MaxHealth");
	if(Json->HasTypedField<EJson::Number>("WalkSpeed"))
		MatchConfig.WalkSpeed = Json->GetIntegerField("WalkSpeed");
	if(Json->HasTypedField<EJson::Boolean>("bSkipMOS"))
		MatchConfig.bSkipMOS = Json->GetBoolField("bSkipMOS");
	if(Json->TryGetArrayField("QoE", QOE_Questions)) {
		this->QOE_Questions.Empty();
		for(int32 i = 0; i < QOE_Questions->Num(); i++) {
			const TArray<TSharedPtr<FJsonValue>> QOE_QuestionValues = (*QOE_Questions)[i]->AsArray();
			FQOE_Question QOE_Question = FQOE_Question();

			QOE_Question.Question = QOE_QuestionValues[0]->AsString();
			QOE_Question.Label1 = QOE_QuestionValues[1]->AsString();
			QOE_Question.Label2 = QOE_QuestionValues[2]->AsString();
			this->QOE_Questions.Add(QOE_Question);
		}
	}
	if(Json->HasTypedField<EJson::Number>("LagInjectionGroup"))
		MatchConfig.LagInjectionGroup = Json->GetIntegerField("LagInjectionGroup");

	return MatchConfig;
}
