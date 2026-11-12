// The contradiction engine and the case simulation (GDD 9.2-9.8). Pure functions over UCaseDefinition and
// FCaseState: no world, no UI, so the Case01Solvable test replays the reference paths through exactly the
// code the game runs. UInvestigationSubsystem wraps these calls and broadcasts the changes.

#pragma once

#include "CoreMinimal.h"
#include "CaseTypes.h"
#include "CaseRules.generated.h"

class UCaseDefinition;

/** Everything that changes during a case. Saved as is. */
USTRUCT(BlueprintType)
struct INVESTIGATION_API FCaseState
{
	GENERATED_BODY()

	/** 0-based night index. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") int32 Night = 0;
	/** Case clock in minutes (12:00 = 0). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") int32 Clock = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") int32 Film = 0;
	/** Cards held, in the order they were gained. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") TArray<FName> Hand;
	/** Shots on film (sharp or grainy) and the sharp ones. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") TSet<FName> Taken;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") TSet<FName> Sharp;
	/** Sharp shots whose film has been developed (dawn or mid-night); their details can be enlarged. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") TSet<FName> Developed;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") TSet<FName> Refuted;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") TSet<FName> Gates;
	/** People the detective has talked to (their portrait cards). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") TArray<FName> Met;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") FName PinWho;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") FName PinWith;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") FName PinWhy;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") int32 WrongRed = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") bool bClosed = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") FName Ending;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") FName Grade;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") bool bProven = false;
	/** At the office (hub) rather than at the scene. Night 1 starts at the scene, later nights at the office. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") bool bAtHub = false;

	bool Holds(FName Card) const { return Hand.Contains(Card); }
	FName Pinned(ECaseSlot Slot) const { return Slot == ECaseSlot::Who ? PinWho : Slot == ECaseSlot::With ? PinWith : PinWhy; }
};

/** Assertion rules: places hierarchy, time overlap, PROP. */
struct INVESTIGATION_API FCaseRules
{
	/** "23:41" -> 701 (12:00 = 0; hours before noon belong to the next day). INDEX_NONE on bad input. */
	static int32 Minutes(const FString& HHMM);
	/** 701 -> "23:41". */
	static FString Clock(int32 Minutes);
	/** True when one place contains the other (or they are the same). */
	static bool Compatible(const TMap<FName, FName>& Places, FName A, FName B);
	/** True when A and B cannot both be true; reasons appended to OutWhy. */
	static bool Conflicts(const FCaseAssertions& A, const FCaseAssertions& B, const TMap<FName, FName>& Places, TArray<FString>* OutWhy = nullptr);
};

/** The case simulation. Every call validates, applies and returns what happened. */
struct INVESTIGATION_API FCaseSim
{
	static void Start(const UCaseDefinition& Case, FCaseState& S);

	// ---- night actions ----
	static ECaseActionResult CanTalk(const UCaseDefinition& Case, const FCaseState& S, FName Topic);
	static ECaseActionResult Talk(const UCaseDefinition& Case, FCaseState& S, FName Topic);
	/** Whether the subject exists right now (expiry, timed window). Gated subjects exist behind their door. */
	static bool ShotPresent(const UCaseDefinition& Case, const FCaseState& S, FName Shot);
	static ECaseActionResult CanShoot(const UCaseDefinition& Case, const FCaseState& S, FName Shot);
	/** One frame: one plate and one photo cost however many subjects it holds. Invalid subjects are skipped. */
	static ECaseActionResult Shoot(const UCaseDefinition& Case, FCaseState& S, const TArray<FCaseShotHit>& Hits, TArray<FName>* OutNewCards = nullptr);
	/** Ok when an action at a place of this kind (hub or scene) can happen where the detective is now. */
	static ECaseActionResult WhereOk(const UCaseDefinition& Case, const FCaseState& S, bool bHubPlace);
	/** The drive between the scene and the office: 10 minutes each way. Invalid in a case without a hub. */
	static ECaseActionResult Drive(const UCaseDefinition& Case, FCaseState& S);
	/** Mid-night developing at the office. */
	static ECaseActionResult CanDevelop(const UCaseDefinition& Case, const FCaseState& S);
	static ECaseActionResult Develop(const UCaseDefinition& Case, FCaseState& S, TArray<FName>* OutNewCards = nullptr);
	/** Details whose print is developed and that the player does not hold yet. */
	static TArray<FName> Enlargeable(const UCaseDefinition& Case, const FCaseState& S);
	static ECaseActionResult Enlarge(const UCaseDefinition& Case, FCaseState& S, FName Detail);

	// ---- board ----
	static bool TimeOfDeathKnown(const UCaseDefinition& Case, const FCaseState& S);
	/** Assertions of a pinned slot (WHO is timed only once the time of death is known). */
	static FCaseAssertions SlotAssertions(const UCaseDefinition& Case, const FCaseState& S, ECaseSlot Slot);
	/** Pure check, no state change. */
	static ECaseRedResult JudgeRed(const UCaseDefinition& Case, const FCaseState& S, const FCaseThreadEnd& A, const FCaseThreadEnd& B, FName* OutStamped = nullptr);
	/** Judges and applies: stamps, gates, slot breaks, the 60-minute cost. */
	static ECaseRedResult Red(const UCaseDefinition& Case, FCaseState& S, const FCaseThreadEnd& A, const FCaseThreadEnd& B, FName* OutStamped = nullptr);
	static bool CanPin(const UCaseDefinition& Case, const FCaseState& S, ECaseSlot Slot, FName Value);
	static bool Pin(const UCaseDefinition& Case, FCaseState& S, ECaseSlot Slot, FName Value);
	static void Unpin(FCaseState& S, ECaseSlot Slot);

	// ---- dawn and the end ----
	static int32 NightStart(const UCaseDefinition& Case, const FCaseState& S);
	static int32 NightEnd(const UCaseDefinition& Case, const FCaseState& S);
	static bool NightOver(const UCaseDefinition& Case, const FCaseState& S);
	static bool IsLastNight(const UCaseDefinition& Case, const FCaseState& S);
	/** First matching row of the monologue table. */
	static FName DawnLine(const UCaseDefinition& Case, const FCaseState& S);
	/** Develops the night's film; starts the next night, or after the last night forces the report. Returns the dawn line (None when the case closed). */
	static FName EndNight(const UCaseDefinition& Case, FCaseState& S, TArray<FName>* OutNewCards = nullptr);
	static bool IsProven(const UCaseDefinition& Case, const FCaseState& S);
	static bool CanReport(const FCaseState& S);
	/** Sends the report with an optional attached card: picks the ending and the rating. */
	static void Close(const UCaseDefinition& Case, FCaseState& S, FName Attached = NAME_None);
};
