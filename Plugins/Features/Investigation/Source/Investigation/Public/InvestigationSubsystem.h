// The running case: one per game instance. Wraps FCaseSim (all rules live there), keeps the prints and the
// board layout, finds subjects in the world and tells the UI what changed. Night ends come from the clock:
// when an action pushes the clock past the night's end, dawn plays right after it (GDD 9.6).

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CaseRules.h"
#include "InvestigationSubsystem.generated.h"

class UCaseDefinition;
class UTexture;
class UPhotoSubjectComponent;
class UTalkComponent;
class UInspectComponent;

/** One developed frame: what it holds and the picture itself. */
USTRUCT(BlueprintType)
struct INVESTIGATION_API FCasePrint
{
	GENERATED_BODY()

	/** 0-based night it was shot on. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") int32 Night = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") int32 Clock = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") TArray<FCaseShotHit> Hits;
	/** Null in tests and before the capture lands. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") TObjectPtr<UTexture> Image;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCaseChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCardsGained, const TArray<FName>&, Cards);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRedJudged, ECaseRedResult, Result, FName, Stamped);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDawn, FName, MonologueLine, int32, NewNight);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCaseClosed, FName, Ending, FName, Grade);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSubtitle, const FText&, Speaker, const FText&, Line);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTalkRequested, UTalkComponent*, Person);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInspectRequested, UInspectComponent*, Thing);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDrive, bool, bAtHub);

UCLASS()
class INVESTIGATION_API UInvestigationSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	static UInvestigationSubsystem* Get(const UObject* WorldContext);

	/** Loads the configured case (or the given one) and starts night 1. False when no case could be loaded. */
	UFUNCTION(BlueprintCallable, Category = "Investigation")
	bool StartCase(UCaseDefinition* InCase = nullptr);

	UFUNCTION(BlueprintPure, Category = "Investigation") UCaseDefinition* GetCase() const { return Case; }
	UFUNCTION(BlueprintPure, Category = "Investigation") const FCaseState& GetState() const { return State; }
	UFUNCTION(BlueprintPure, Category = "Investigation") FString GetClockText() const;
	UFUNCTION(BlueprintPure, Category = "Investigation") const TArray<FCasePrint>& GetPrints() const { return Prints; }

	// ---- actions (each returns what happened; the UI words it) ----
	UFUNCTION(BlueprintCallable, Category = "Investigation") ECaseActionResult Talk(FName Topic);
	/** Topics a speaker offers right now, with their state (Ok / AlreadyDone / Locked). */
	UFUNCTION(BlueprintCallable, Category = "Investigation") TMap<FName, ECaseActionResult> TopicsFor(FName Speaker) const;
	UFUNCTION(BlueprintCallable, Category = "Investigation") ECaseActionResult Shoot(const TArray<FCaseShotHit>& Hits, UTexture* Image);
	UFUNCTION(BlueprintCallable, Category = "Investigation") ECaseActionResult Develop();
	/** The drive between the scene and the office; the game plays the car shot and moves the player on OnDrive. */
	UFUNCTION(BlueprintCallable, Category = "Investigation") ECaseActionResult Drive();
	UFUNCTION(BlueprintCallable, Category = "Investigation") ECaseActionResult Enlarge(FName Detail);
	UFUNCTION(BlueprintCallable, Category = "Investigation") ECaseRedResult Red(const FCaseThreadEnd& A, const FCaseThreadEnd& B);
	UFUNCTION(BlueprintCallable, Category = "Investigation") bool Pin(ECaseSlot Slot, FName Value);
	UFUNCTION(BlueprintCallable, Category = "Investigation") void Unpin(ECaseSlot Slot);
	/** Goes home early: dawn now. */
	UFUNCTION(BlueprintCallable, Category = "Investigation") void EndNight();
	UFUNCTION(BlueprintCallable, Category = "Investigation") bool Report(FName Attached);

	/** Board layout (card -> board position), kept with the case so the board reopens as it was left. */
	UPROPERTY(BlueprintReadWrite, Category = "Investigation") TMap<FName, FVector2D> BoardLayout;
	/** Black threads are the player's own notes: no rules, no cost. */
	UPROPERTY(BlueprintReadWrite, Category = "Investigation") TArray<FCaseThreadEnd> BlackThreadsFrom;
	UPROPERTY(BlueprintReadWrite, Category = "Investigation") TArray<FCaseThreadEnd> BlackThreadsTo;

	// ---- subjects in the world ----
	void RegisterSubject(UPhotoSubjectComponent* Subject);
	void UnregisterSubject(UPhotoSubjectComponent* Subject);
	const TArray<TWeakObjectPtr<UPhotoSubjectComponent>>& GetSubjects() const { return Subjects; }

	UPROPERTY(BlueprintAssignable, Category = "Investigation") FOnCaseChanged OnCaseChanged;
	UPROPERTY(BlueprintAssignable, Category = "Investigation") FOnCardsGained OnCardsGained;
	UPROPERTY(BlueprintAssignable, Category = "Investigation") FOnRedJudged OnRedJudged;
	UPROPERTY(BlueprintAssignable, Category = "Investigation") FOnDawn OnDawn;
	UPROPERTY(BlueprintAssignable, Category = "Investigation") FOnCaseClosed OnCaseClosed;
	UPROPERTY(BlueprintAssignable, Category = "Investigation") FOnSubtitle OnSubtitle;
	UPROPERTY(BlueprintAssignable, Category = "Investigation") FOnTalkRequested OnTalkRequested;
	UPROPERTY(BlueprintAssignable, Category = "Investigation") FOnInspectRequested OnInspectRequested;
	/** After the drive (and after every dawn, which puts the detective in the darkroom): where he is now. */
	UPROPERTY(BlueprintAssignable, Category = "Investigation") FOnDrive OnDrive;

private:
	bool IsReady() const { return Case && !State.bClosed; }
	/** After any action that spends time: dawn if the night ran out. */
	void AfterAction(const TArray<FName>& Gained);
	void DoEndNight();

	UPROPERTY() TObjectPtr<UCaseDefinition> Case;
	UPROPERTY() FCaseState State;
	UPROPERTY() TArray<FCasePrint> Prints;
	TArray<TWeakObjectPtr<UPhotoSubjectComponent>> Subjects;
};
