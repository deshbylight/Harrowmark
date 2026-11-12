// Which case the game plays. Project Settings > Game > Investigation, or Config/DefaultGame.ini:
//   [/Script/Investigation.InvestigationSettings]
//   Case=/Game/Case/DA_Case01.DA_Case01

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "InvestigationSettings.generated.h"

class UCaseDefinition;

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Investigation"))
class INVESTIGATION_API UInvestigationSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	static const UInvestigationSettings& Get() { return *GetDefault<UInvestigationSettings>(); }

	/** The case asset (made by Content/Python/import_case.py). */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Case")
	TSoftObjectPtr<UCaseDefinition> Case;

	/** Used when Case is empty (editor runs before the import): path relative to the project directory. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Case")
	FString FallbackJson = TEXT("data/case01.json");
};
