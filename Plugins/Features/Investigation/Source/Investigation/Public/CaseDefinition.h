// One asset per case. The source of truth is the JSON in data/<case>.json (checked by tools/case_check.py);
// the asset stores that text in SourceJson and parses it on load, so tuning the case needs no code:
//   Content/Python/import_case.py copies data/case01.json into /Game/Case/DA_Case01.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CaseTypes.h"
#include "CaseDefinition.generated.h"

UCLASS(BlueprintType)
class INVESTIGATION_API UCaseDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** The case JSON (same schema as data/case01.json). */
	UPROPERTY(EditAnywhere, Category = "Case", meta = (MultiLine = true))
	FString SourceJson;

	/** Parses SourceJson into the fields below. Returns false and an error on bad data. */
	UFUNCTION(BlueprintCallable, Category = "Case")
	bool Reload(FString& OutError);

	/** Sets SourceJson and parses it. */
	bool LoadFromJson(const FString& Json, FString& OutError);

	/** Transient case for tests and editor runs: parses a JSON file. Null on error. */
	static UCaseDefinition* LoadFromFile(const FString& Path, FString& OutError, UObject* Outer = nullptr);

	virtual void PostLoad() override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& Event) override;
#endif

	// ---- parsed (transient) ----
	UPROPERTY(VisibleAnywhere, Transient, BlueprintReadOnly, Category = "Parsed") FName CaseId;
	UPROPERTY(VisibleAnywhere, Transient, BlueprintReadOnly, Category = "Parsed") FText Title;
	UPROPERTY(VisibleAnywhere, Transient, BlueprintReadOnly, Category = "Parsed") FName Client;
	UPROPERTY(VisibleAnywhere, Transient, BlueprintReadOnly, Category = "Parsed") int32 TimeOfDeath = 0;
	UPROPERTY(VisibleAnywhere, Transient, BlueprintReadOnly, Category = "Parsed") TArray<FCaseNight> Nights;
	UPROPERTY(VisibleAnywhere, Transient, BlueprintReadOnly, Category = "Parsed") FCaseCosts Costs;
	/** Place -> parent (NAME_None at the root). */
	UPROPERTY(VisibleAnywhere, Transient, BlueprintReadOnly, Category = "Parsed") TMap<FName, FName> Places;
	/** The agency office: darkroom, loupe and its own subjects; a drive away from the scene. None: no drive in this case. */
	UPROPERTY(VisibleAnywhere, Transient, BlueprintReadOnly, Category = "Parsed") FName Hub;
	UPROPERTY(VisibleAnywhere, Transient, BlueprintReadOnly, Category = "Parsed") TArray<FCasePerson> People;
	UPROPERTY(VisibleAnywhere, Transient, BlueprintReadOnly, Category = "Parsed") TArray<FName> Suspects;
	/** Suspect -> the card that makes them pinnable as WHO (instead of having talked to them). */
	UPROPERTY(VisibleAnywhere, Transient, BlueprintReadOnly, Category = "Parsed") TMap<FName, FName> Unlock;
	UPROPERTY(VisibleAnywhere, Transient, BlueprintReadOnly, Category = "Parsed") TMap<FName, FCaseWeapon> Weapons;
	UPROPERTY(VisibleAnywhere, Transient, BlueprintReadOnly, Category = "Parsed") FCaseTheoryWho TheoryWho;
	UPROPERTY(VisibleAnywhere, Transient, BlueprintReadOnly, Category = "Parsed") FName TruthKiller;
	UPROPERTY(VisibleAnywhere, Transient, BlueprintReadOnly, Category = "Parsed") FName TruthWeapon;
	UPROPERTY(VisibleAnywhere, Transient, BlueprintReadOnly, Category = "Parsed") TArray<FName> TruthMotives;
	UPROPERTY(VisibleAnywhere, Transient, BlueprintReadOnly, Category = "Parsed") FCaseAssertions Truth;
	UPROPERTY(VisibleAnywhere, Transient, BlueprintReadOnly, Category = "Parsed") TArray<FCaseShot> Shots;
	UPROPERTY(VisibleAnywhere, Transient, BlueprintReadOnly, Category = "Parsed") TArray<FCaseCard> Cards;
	UPROPERTY(VisibleAnywhere, Transient, BlueprintReadOnly, Category = "Parsed") TArray<FCaseMonologueRow> Monologue;
	UPROPERTY(VisibleAnywhere, Transient, BlueprintReadOnly, Category = "Parsed") FCaseProof Proof;
	UPROPERTY(VisibleAnywhere, Transient, BlueprintReadOnly, Category = "Parsed") TArray<FCaseEnding> Endings;
	UPROPERTY(VisibleAnywhere, Transient, BlueprintReadOnly, Category = "Parsed") TArray<FCaseRating> Ratings;

	bool IsLoaded() const { return !Cards.IsEmpty(); }

	const FCaseCard* FindCard(FName Id) const;
	const FCaseShot* FindShot(FName Id) const;
	const FCasePerson* FindPerson(FName Id) const;
	const FCaseMonologueRow* FindMonologue(FName Id) const;
	const FCaseEnding* FindEnding(FName Id) const;
	/** The shot whose subject yields this anchor, or that this detail is enlarged from. */
	const FCaseShot* SourceShot(FName CardId) const;
	FName ForcedShot() const;
	/** True when the place is the hub or inside it. */
	bool IsHubPlace(FName Place) const;
	/** Statements by one speaker, in case order. */
	TArray<FName> TopicsOf(FName Speaker) const;

private:
	void Reindex();

	TMap<FName, int32> CardIndex;
	TMap<FName, int32> ShotIndex;
	TMap<FName, FName> SourceIndex;
};
