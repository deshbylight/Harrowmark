// Case data model (GDD 9.2, CASE01 section 4). Times are minutes on the case clock (12:00 = 0, so a night
// runs forward through midnight: 23:41 = 701, 02:00 = 840). Parsed from data/<case>.json by UCaseDefinition.

#pragma once

#include "CoreMinimal.h"
#include "CaseTypes.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogInvestigation, Log, All);

UENUM(BlueprintType)
enum class ECaseCardKind : uint8
{
	Anchor,    // a photograph: always true
	Detail,    // a darkroom enlargement: always true
	Statement, // what someone said: may be a lie or an honest mistake
	Person     // a portrait card, granted on first talk; used for WHO
};

/** What a red thread did (GDD 9.3). */
UENUM(BlueprintType)
enum class ECaseRedResult : uint8
{
	NoContradiction, // the thread drops, the night loses 60 minutes
	Lie,             // LIE stamp on the statement
	Wrong,           // WRONG stamp: an honest mistake
	Break,           // the theory breaks, the slot empties; free
	Word,            // statement against a slot: "Word against word."; free
	Invalid          // not held / empty slot / same card: nothing happens
};

UENUM(BlueprintType)
enum class ECaseSlot : uint8
{
	Who,
	With,
	Why
};

UENUM(BlueprintType)
enum class ECaseActionResult : uint8
{
	Ok,
	AlreadyDone,  // topic already heard, subject already on film (sharp)
	Locked,       // behind a gate or a topic condition
	NoFilm,
	Gone,         // expired after its night, or outside its timed window
	Tutorial,     // the forced shot comes first
	NotHeld,
	Elsewhere,    // needs the drive first: the office (hub) is a drive away from the scene
	Invalid
};

/** Three-valued condition for ending and rating rows: unset rows match anything. */
UENUM(BlueprintType)
enum class ECaseTri : uint8
{
	Any,
	Yes,
	No
};

/** AT(entity, place, from, to). */
USTRUCT(BlueprintType)
struct INVESTIGATION_API FCaseAt
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") FName Entity;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") FName Place;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") int32 From = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") int32 To = 0;
};

/** Everything a card, a theory slot or the truth asserts: AT rows and PROP key/values. */
USTRUCT(BlueprintType)
struct INVESTIGATION_API FCaseAssertions
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") TArray<FCaseAt> At;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") TMap<FName, FName> Props;

	bool IsEmpty() const { return At.IsEmpty() && Props.IsEmpty(); }
};

/** Condition on a topic or a shot. */
USTRUCT(BlueprintType)
struct INVESTIGATION_API FCaseRequires
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") TArray<FName> Cards;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") FName Gate;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") FName Refuted;
};

USTRUCT(BlueprintType)
struct INVESTIGATION_API FCaseCard
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") FName Id;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") ECaseCardKind Kind = ECaseCardKind::Anchor;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") FText Text;
	/** Statements: the topic line the player picks in talk. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") FText Topic;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") FName Speaker;
	/** Statements: false means a lie, or an honest mistake when bMistake. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") bool bTrue = true;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") bool bMistake = false;
	/** Statements: minutes the topic costs; -1 = the case default. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") int32 Cost = -1;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") FCaseRequires Requires;
	/** Statements: gate opened when this statement is stamped. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") FName Opens;
	/** Motive cards: whose motive this is (WHY slot). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") FName MotiveOf;
	/** Details: the shot it is enlarged from. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") FName FromShot;
	/** Object cards can go into WITH WHAT. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") bool bObject = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") FCaseAssertions Assertions;

	bool IsHard() const { return Kind == ECaseCardKind::Anchor || Kind == ECaseCardKind::Detail; }
};

USTRUCT(BlueprintType)
struct INVESTIGATION_API FCaseShot
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") FName Id;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") FText Label;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") FName Place;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") TArray<FName> Yields;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") FName Detail;
	/** Last night (1-based) the subject exists; 0 = never expires. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") int32 Expires = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") bool bForced = false;
	/** Exposed before night 1 (a plate already in the camera): it uses up a plate of night 1, cannot be shot,
	 *  and comes out of the first developing (mid-night or dawn) with its cards. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") bool bPreExposed = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") FCaseRequires Requires;
	/** Timed scene: the only night (1-based) it happens; 0 = not timed. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") int32 WindowNight = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") int32 WindowFrom = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") int32 WindowTo = 0;
};

USTRUCT(BlueprintType)
struct INVESTIGATION_API FCasePerson
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") FName Id;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") FText Name;
	/** "tall" / "short"; empty for people who cannot be pinned. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") FName Height;
};

USTRUCT(BlueprintType)
struct INVESTIGATION_API FCaseWeapon
{
	GENERATED_BODY()

	/** What the weapon asserts on a WITH thread ("prop": wound shape, length). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") TMap<FName, FName> Props;
	/** Where the weapon was at the time of death ("at" rows), checked against the hard facts. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") TArray<FCaseAt> At;
};

/** WHO slot template (CASE01 4.5). */
USTRUCT(BlueprintType)
struct INVESTIGATION_API FCaseTheoryWho
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") FName Place;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") int32 From = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") int32 To = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") FName Prop;
	/** Cards that fix the time of death; the WHO window applies once one is held. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") TArray<FName> TimeFrom;
};

/** One row of the dawn monologue table; the first matching row plays. */
USTRUCT(BlueprintType)
struct INVESTIGATION_API FCaseMonologueRow
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") FName Id;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") FText Text;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") FName Who;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") TArray<FName> WithIn;
	/** Matches only while this lie is not yet stamped. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") FName LieOpen;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") TArray<ECaseSlot> Filled;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") TArray<ECaseSlot> Empty;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") TArray<ECaseSlot> EmptyAny;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") ECaseTri WhyWrong = ECaseTri::Any;
};

USTRUCT(BlueprintType)
struct INVESTIGATION_API FCaseEnding
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") FName Id;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") FText Title;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") ECaseTri Proven = ECaseTri::Any;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") FName Attached;
	/** When bHasWho, the pinned WHO must equal Who (NAME_None = empty slot). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") bool bHasWho = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") FName Who;
};

USTRUCT(BlueprintType)
struct INVESTIGATION_API FCaseRating
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") FName Grade;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") ECaseTri Proven = ECaseTri::Any;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") FName Ending;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") int32 MaxNight = 99;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") int32 MaxWrongRed = 99;
};

/** What a proven report needs (CASE01 section 7). */
USTRUCT(BlueprintType)
struct INVESTIGATION_API FCaseProof
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") FName Who;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") FName With;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") TArray<FName> Why;
	/** Optional: a red thread that must have broken this card. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") FName Refuted;
	/** Optional: the player must hold at least one of these (the darkroom enlargements that put him on film). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") TArray<FName> HoldsAny;
};

USTRUCT(BlueprintType)
struct INVESTIGATION_API FCaseNight
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") int32 Start = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") int32 End = 0;
};

USTRUCT(BlueprintType)
struct INVESTIGATION_API FCaseCosts
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") int32 Photo = 10;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") int32 Topic = 15;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") int32 WrongRed = 60;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") int32 Develop = 30;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") int32 FilmPerNight = 12;
	/** One way between the scene and the office (hub). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") int32 Drive = 10;
};

/** One subject in a frame and whether it came out sharp. */
USTRUCT(BlueprintType)
struct INVESTIGATION_API FCaseShotHit
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Case") FName Shot;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Case") bool bSharp = true;

	FCaseShotHit() = default;
	FCaseShotHit(FName InShot, bool bInSharp) : Shot(InShot), bSharp(bInSharp) {}
};

/** A red-thread end: a card id, or a theory slot (Who / With). */
USTRUCT(BlueprintType)
struct INVESTIGATION_API FCaseThreadEnd
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Case") FName Card;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Case", meta = (ScriptName = "IsSlot")) bool bSlot = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Case") ECaseSlot Slot = ECaseSlot::Who;

	static FCaseThreadEnd OfCard(FName InCard) { FCaseThreadEnd E; E.Card = InCard; return E; }
	static FCaseThreadEnd OfSlot(ECaseSlot InSlot) { FCaseThreadEnd E; E.bSlot = true; E.Slot = InSlot; return E; }
};
