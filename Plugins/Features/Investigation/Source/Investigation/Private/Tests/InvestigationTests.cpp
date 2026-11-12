// Rules tests. Case01Solvable ports tools/case_check.py and replays every reference path in data/case01.json through FCaseSim, the same code the
// game runs, so the C++ engine and tools/case_check.py cannot drift apart.

#include "CaseDefinition.h"
#include "CaseRules.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "UObject/StrongObjectPtr.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace InvestigationTests
{
	FString CasePath() { return FPaths::Combine(FPaths::ProjectDir(), TEXT("data/case01.json")); }

	UCaseDefinition* LoadCase(FAutomationTestBase& Test)
	{
		FString Error;
		UCaseDefinition* Case = UCaseDefinition::LoadFromFile(CasePath(), Error);
		Test.TestNotNull(FString::Printf(TEXT("case01 loads (%s)"), *Error), Case);
		return Case;
	}

	FCaseAt At(const TCHAR* Entity, const TCHAR* Place, const TCHAR* From, const TCHAR* To)
	{
		FCaseAt A;
		A.Entity = Entity;
		A.Place = Place;
		A.From = FCaseRules::Minutes(From);
		A.To = FCaseRules::Minutes(To);
		return A;
	}

	const TCHAR* RedName(ECaseRedResult R)
	{
		switch (R)
		{
		case ECaseRedResult::NoContradiction: return TEXT("none");
		case ECaseRedResult::Lie: return TEXT("lie");
		case ECaseRedResult::Wrong: return TEXT("wrong");
		case ECaseRedResult::Break: return TEXT("break");
		case ECaseRedResult::Word: return TEXT("word");
		default: return TEXT("invalid");
		}
	}

	FCaseThreadEnd End(const FString& X)
	{
		if (X == TEXT("WHO")) return FCaseThreadEnd::OfSlot(ECaseSlot::Who);
		if (X == TEXT("WITH")) return FCaseThreadEnd::OfSlot(ECaseSlot::With);
		return FCaseThreadEnd::OfCard(FName(*X));
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInvestigationConflictsTest, "Project.Investigation.Conflicts",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FInvestigationConflictsTest::RunTest(const FString& Parameters)
{
	using namespace InvestigationTests;
	TestEqual(TEXT("23:41"), FCaseRules::Minutes(TEXT("23:41")), 701);
	TestEqual(TEXT("02:00 is after midnight"), FCaseRules::Minutes(TEXT("02:00")), 840);
	TestEqual(TEXT("bad time"), FCaseRules::Minutes(TEXT("25:00")), static_cast<int32>(INDEX_NONE));
	TestEqual(TEXT("clock text"), FCaseRules::Clock(701), FString(TEXT("23:41")));

	TMap<FName, FName> Places;
	Places.Add(TEXT("Pier5"), NAME_None);
	Places.Add(TEXT("Berth"), TEXT("Pier5"));
	Places.Add(TEXT("Crane"), TEXT("Berth"));
	Places.Add(TEXT("Office"), NAME_None);
	TestTrue(TEXT("ancestor is compatible"), FCaseRules::Compatible(Places, TEXT("Pier5"), TEXT("Crane")));
	TestFalse(TEXT("siblings are not"), FCaseRules::Compatible(Places, TEXT("Office"), TEXT("Crane")));

	FCaseAssertions A, B;
	A.At.Add(At(TEXT("Varga"), TEXT("Crane"), TEXT("23:30"), TEXT("23:45")));
	B.At.Add(At(TEXT("Varga"), TEXT("Office"), TEXT("23:45"), TEXT("00:10")));
	TestTrue(TEXT("touching windows overlap"), FCaseRules::Conflicts(A, B, Places));
	B.At[0].From = FCaseRules::Minutes(TEXT("23:46"));
	TestFalse(TEXT("disjoint windows do not"), FCaseRules::Conflicts(A, B, Places));
	B.At[0].Place = TEXT("Pier5");
	B.At[0].From = FCaseRules::Minutes(TEXT("23:00"));
	TestFalse(TEXT("inside a containing place is fine"), FCaseRules::Conflicts(A, B, Places));

	FCaseAssertions P, Q;
	P.Props.Add(TEXT("killer.height"), TEXT("tall"));
	Q.Props.Add(TEXT("killer.height"), TEXT("short"));
	TArray<FString> Why;
	TestTrue(TEXT("prop mismatch"), FCaseRules::Conflicts(P, Q, Places, &Why));
	TestEqual(TEXT("one reason"), Why.Num(), 1);
	Q.Props[TEXT("killer.height")] = TEXT("tall");
	TestFalse(TEXT("prop match"), FCaseRules::Conflicts(P, Q, Places));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInvestigationDarkroomTest, "Project.Investigation.Darkroom",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FInvestigationDarkroomTest::RunTest(const FString& Parameters)
{
	using namespace InvestigationTests;
	TStrongObjectPtr<UCaseDefinition> Case(LoadCase(*this));
	if (!Case)
	{
		return false;
	}
	FCaseState S;
	FCaseSim::Start(*Case, S);
	const FName Forced = Case->ForcedShot();
	TestFalse(TEXT("case has a forced shot"), Forced.IsNone());

	// The forced shot comes first.
	for (const FCaseShot& Shot : Case->Shots)
	{
		if (Shot.Id != Forced && !Shot.bPreExposed && !Case->IsHubPlace(Shot.Place) && FCaseSim::ShotPresent(*Case, S, Shot.Id)
			&& Shot.Requires.Cards.IsEmpty() && Shot.Requires.Gate.IsNone() && Shot.Requires.Refuted.IsNone())
		{
			TestEqual(TEXT("tutorial first"), FCaseSim::CanShoot(*Case, S, Shot.Id), ECaseActionResult::Tutorial);
			break;
		}
	}
	const int32 Film = S.Film;
	const int32 Clock = S.Clock;
	TestEqual(TEXT("forced shot"), FCaseSim::Shoot(*Case, S, {{Forced, false}}), ECaseActionResult::Ok);
	TestEqual(TEXT("one plate"), S.Film, Film - 1);
	TestEqual(TEXT("photo minutes"), S.Clock, Clock + Case->Costs.Photo);
	TestTrue(TEXT("forced shot is always sharp"), S.Sharp.Contains(Forced));

	// The office (hub) is a drive away: developing and the loupe happen there, talks and scene shots here.
	const bool bHub = !Case->Hub.IsNone();
	TestFalse(TEXT("night 1 starts at the scene"), S.bAtHub);
	auto GoTo = [&](bool bToHub)
	{
		if (bHub && S.bAtHub != bToHub)
		{
			const int32 C = S.Clock;
			TestEqual(TEXT("drive"), FCaseSim::Drive(*Case, S), ECaseActionResult::Ok);
			TestEqual(TEXT("drive minutes"), S.Clock, C + Case->Costs.Drive);
			TestEqual(TEXT("arrived"), S.bAtHub, bToHub);
		}
	};

	// A grainy frame gives the anchors, not the detail, and can be retaken.
	const FCaseShot* WithDetail = Case->Shots.FindByPredicate([&](const FCaseShot& Sh)
		{ return Sh.Id != Forced && !Sh.Detail.IsNone() && FCaseSim::CanShoot(*Case, S, Sh.Id) == ECaseActionResult::Ok; });
	if (TestNotNull(TEXT("a free shot with a detail"), WithDetail))
	{
		TestEqual(TEXT("grainy"), FCaseSim::Shoot(*Case, S, {{WithDetail->Id, false}}), ECaseActionResult::Ok);
		for (FName Y : WithDetail->Yields)
		{
			TestTrue(TEXT("anchor from a grainy frame"), S.Holds(Y));
		}
		if (bHub)
		{
			const int32 C = S.Clock;
			TestEqual(TEXT("develop needs the office"), FCaseSim::Develop(*Case, S), ECaseActionResult::Elsewhere);
			TestEqual(TEXT("refused develop is free"), S.Clock, C);
		}
		GoTo(true);
		TestEqual(TEXT("develop"), FCaseSim::Develop(*Case, S), ECaseActionResult::Ok);
		TestFalse(TEXT("no detail from grain"), FCaseSim::Enlargeable(*Case, S).Contains(WithDetail->Detail));
		if (bHub)
		{
			TestEqual(TEXT("scene shots need the scene"), FCaseSim::CanShoot(*Case, S, WithDetail->Id), ECaseActionResult::Elsewhere);
		}
		GoTo(false);
		TestEqual(TEXT("retake"), FCaseSim::CanShoot(*Case, S, WithDetail->Id), ECaseActionResult::Ok);
		TestEqual(TEXT("sharp retake"), FCaseSim::Shoot(*Case, S, {{WithDetail->Id, true}}), ECaseActionResult::Ok);
		TestEqual(TEXT("no third time"), FCaseSim::CanShoot(*Case, S, WithDetail->Id), ECaseActionResult::AlreadyDone);
		TestEqual(TEXT("not developed yet"), FCaseSim::Enlarge(*Case, S, WithDetail->Detail), ECaseActionResult::Locked);
		GoTo(true);
		TestEqual(TEXT("develop again"), FCaseSim::Develop(*Case, S), ECaseActionResult::Ok);
		GoTo(false);
		if (bHub)
		{
			TestEqual(TEXT("the loupe is in the darkroom"), FCaseSim::Enlarge(*Case, S, WithDetail->Detail), ECaseActionResult::Elsewhere);
		}
		GoTo(true);
		TestEqual(TEXT("enlarge"), FCaseSim::Enlarge(*Case, S, WithDetail->Detail), ECaseActionResult::Ok);
		GoTo(false);
	}

	// Out of film.
	S.Film = 0;
	const FCaseShot* Any = Case->Shots.FindByPredicate([&](const FCaseShot& Sh) { return !S.Taken.Contains(Sh.Id) && FCaseSim::ShotPresent(*Case, S, Sh.Id)
		&& !Case->IsHubPlace(Sh.Place)
		&& Sh.Requires.Cards.IsEmpty() && Sh.Requires.Gate.IsNone() && Sh.Requires.Refuted.IsNone(); });
	if (Any)
	{
		TestEqual(TEXT("no film"), FCaseSim::CanShoot(*Case, S, Any->Id), ECaseActionResult::NoFilm);
	}

	// A red thread between two true cards costs an hour.
	const int32 Before = S.Clock;
	if (S.Hand.Num() >= 2)
	{
		TestEqual(TEXT("two anchors never contradict"), FCaseSim::Red(*Case, S, FCaseThreadEnd::OfCard(S.Hand[0]), FCaseThreadEnd::OfCard(S.Hand[1])),
			ECaseRedResult::NoContradiction);
		TestEqual(TEXT("wrong red costs"), S.Clock, Before + Case->Costs.WrongRed);
		TestEqual(TEXT("wrong red counted"), S.WrongRed, 1);
	}
	TestEqual(TEXT("empty slot"), FCaseSim::Red(*Case, S, FCaseThreadEnd::OfSlot(ECaseSlot::Who), FCaseThreadEnd::OfCard(S.Hand[0])),
		ECaseRedResult::Invalid);
	TestEqual(TEXT("invalid is free"), S.WrongRed, 1);

	// Dawn: the night's sharp film is developed, plates refill, night-1 subjects are gone.
	const TSet<FName> SharpBefore = S.Sharp;
	FCaseSim::EndNight(*Case, S);
	TestEqual(TEXT("night 2"), S.Night, 1);
	TestEqual(TEXT("film refilled"), S.Film, Case->Costs.FilmPerNight);
	TestEqual(TEXT("clock at night 2 start"), S.Clock, Case->Nights[1].Start);
	TestEqual(TEXT("night 2 starts at the office"), S.bAtHub, bHub);
	TestTrue(TEXT("sharp film developed at dawn"), S.Developed.Includes(SharpBefore));
	for (const FCaseShot& Shot : Case->Shots)
	{
		if (Shot.Expires == 1)
		{
			TestEqual(FString::Printf(TEXT("%s gone"), *Shot.Id.ToString()), FCaseSim::CanShoot(*Case, S, Shot.Id), ECaseActionResult::Gone);
		}
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInvestigationCase01SolvableTest, "Project.Investigation.Case01Solvable",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FInvestigationCase01SolvableTest::RunTest(const FString& Parameters)
{
	using namespace InvestigationTests;
	TStrongObjectPtr<UCaseDefinition> Case(LoadCase(*this));
	FString Text;
	TSharedPtr<FJsonObject> Root;
	if (!Case || !FFileHelper::LoadFileToString(Text, *CasePath()) || !FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Text), Root)
		|| !Root.IsValid())
	{
		AddError(TEXT("cannot read data/case01.json"));
		return false;
	}
	// ---- consistency (case_check.py checks 2-4) ----
	TArray<const FCaseCard*> Hard;
	for (const FCaseCard& Card : Case->Cards)
	{
		if (Card.IsHard()) Hard.Add(&Card);
		if (Card.Assertions.IsEmpty()) continue;
		const bool bMustBeTrue = Card.IsHard() || Card.bTrue;
		TestEqual(FString::Printf(TEXT("truth: %s"), *Card.Id.ToString()), FCaseRules::Conflicts(Card.Assertions, Case->Truth, Case->Places), !bMustBeTrue);
	}
	for (int32 I = 0; I < Hard.Num(); ++I)
	{
		for (int32 J = I + 1; J < Hard.Num(); ++J)
		{
			TestFalse(FString::Printf(TEXT("hard %s vs %s"), *Hard[I]->Id.ToString(), *Hard[J]->Id.ToString()),
				FCaseRules::Conflicts(Hard[I]->Assertions, Hard[J]->Assertions, Case->Places));
		}
	}
	auto GateOf = [&Case](FName CardId) { const FCaseShot* Shot = Case->SourceShot(CardId); return Shot ? Shot->Requires.Gate : NAME_None; };
	auto Refuters = [&](const FCaseAssertions& Item, FName ExcludeGate)
	{
		int32 N = 0;
		for (const FCaseCard* H : Hard)
		{
			N += FCaseRules::Conflicts(Item, H->Assertions, Case->Places) && (ExcludeGate.IsNone() || GateOf(H->Id) != ExcludeGate) ? 1 : 0;
		}
		return N;
	};
	auto Theory = [&](ECaseSlot Slot, FName Value)
	{
		FCaseState T;
		(Slot == ECaseSlot::Who ? T.PinWho : T.PinWith) = Value;
		T.Hand.Append(Case->TheoryWho.TimeFrom);
		return FCaseSim::SlotAssertions(*Case, T, Slot);
	};
	for (FName Suspect : Case->Suspects)
	{
		const int32 N = Refuters(Theory(ECaseSlot::Who, Suspect), NAME_None);
		const bool bTrue = Suspect == Case->TruthKiller;
		TestTrue(FString::Printf(TEXT("theory WHO=%s refuted by %d"), *Suspect.ToString(), N), bTrue ? N == 0 : N >= 2);
	}
	for (const TPair<FName, FCaseWeapon>& Weapon : Case->Weapons)
	{
		const int32 N = Refuters(Theory(ECaseSlot::With, Weapon.Key), NAME_None);
		const bool bTrue = Weapon.Key == Case->TruthWeapon;
		TestTrue(FString::Printf(TEXT("theory WITH=%s refuted by %d"), *Weapon.Key.ToString(), N), bTrue ? N == 0 : N >= 2);
	}
	TSet<FName> Opened;
	for (const FCaseCard& Card : Case->Cards)
	{
		if (!Card.Opens.IsNone()) Opened.Add(Card.Opens);
		if (Card.Kind != ECaseCardKind::Statement || Card.bTrue || Card.Assertions.IsEmpty()) continue;
		const int32 N = Refuters(Card.Assertions, Card.Opens);
		TestTrue(FString::Printf(TEXT("%s %s refuted by %d"), Card.bMistake ? TEXT("mistake") : TEXT("lie"), *Card.Id.ToString(), N), N >= (Card.bMistake ? 1 : 2));
	}
	for (const FCaseShot& Shot : Case->Shots)
	{
		TestTrue(FString::Printf(TEXT("gate of %s opens"), *Shot.Id.ToString()), Shot.Requires.Gate.IsNone() || Opened.Contains(Shot.Requires.Gate));
	}

	// ---- reference paths (check 6) ----
	const TSharedPtr<FJsonObject>* Paths = nullptr;
	if (!Root->TryGetObjectField(TEXT("reference_paths"), Paths) || (*Paths)->Values.Num() < 6)
	{
		AddError(TEXT("reference_paths missing"));
		return false;
	}

	for (const TPair<FString, TSharedPtr<FJsonValue>>& Path : (*Paths)->Values)
	{
		const FString Tag = FString::Printf(TEXT("path %s"), *Path.Key);
		FCaseState S;
		FCaseSim::Start(*Case, S);
		int32 Index = 0;
		bool bClosed = false;
		for (const TSharedPtr<FJsonValue>& StepValue : Path.Value->AsArray())
		{
			const TSharedPtr<FJsonObject> Step = StepValue->AsObject();
			const FString Do = Step->GetStringField(TEXT("do"));
			const FString Where = FString::Printf(TEXT("%s step %d (%s)"), *Tag, Index++, *Do);
			FString A, B, Expect;

			if (Do == TEXT("talk"))
			{
				const FName Topic(*Step->GetStringField(TEXT("card")));
				TestEqual(Where + TEXT(" ") + Topic.ToString(), FCaseSim::Talk(*Case, S, Topic), ECaseActionResult::Ok);
			}
			else if (Do == TEXT("shoot"))
			{
				const FName Shot(*Step->GetStringField(TEXT("shot")));
				TestEqual(Where + TEXT(" ") + Shot.ToString(), FCaseSim::Shoot(*Case, S, {{Shot, true}}), ECaseActionResult::Ok);
			}
			else if (Do == TEXT("develop"))
			{
				TestEqual(Where, FCaseSim::Develop(*Case, S), ECaseActionResult::Ok);
			}
			else if (Do == TEXT("drive"))
			{
				TestEqual(Where, FCaseSim::Drive(*Case, S), ECaseActionResult::Ok);
			}
			else if (Do == TEXT("enlarge"))
			{
				const FName Detail(*Step->GetStringField(TEXT("detail")));
				TestEqual(Where + TEXT(" ") + Detail.ToString(), FCaseSim::Enlarge(*Case, S, Detail), ECaseActionResult::Ok);
			}
			else if (Do == TEXT("red"))
			{
				A = Step->GetStringField(TEXT("a"));
				B = Step->GetStringField(TEXT("b"));
				Expect = Step->GetStringField(TEXT("expect"));
				const ECaseRedResult R = FCaseSim::Red(*Case, S, End(A), End(B));
				TestEqual(FString::Printf(TEXT("%s %s-%s"), *Where, *A, *B), FString(RedName(R)), Expect);
			}
			else if (Do == TEXT("pin"))
			{
				const TPair<const TCHAR*, ECaseSlot> Slots[] = {{TEXT("who"), ECaseSlot::Who}, {TEXT("with"), ECaseSlot::With}, {TEXT("why"), ECaseSlot::Why}};
				for (const TPair<const TCHAR*, ECaseSlot>& Slot : Slots)
				{
					FString Value;
					if (Step->TryGetStringField(Slot.Key, Value))
					{
						TestTrue(FString::Printf(TEXT("%s %s=%s"), *Where, Slot.Key, *Value), FCaseSim::Pin(*Case, S, Slot.Value, FName(*Value)));
					}
				}
			}
			else if (Do == TEXT("end_night") || Do == TEXT("close"))
			{
				if (Do == TEXT("close"))
				{
					TestTrue(Where + TEXT(" report is complete"), FCaseSim::CanReport(S));
					FString Attach;
					Step->TryGetStringField(TEXT("attach"), Attach);
					FCaseSim::Close(*Case, S, Attach.IsEmpty() ? NAME_None : FName(*Attach));
				}
				else
				{
					const FName Line = FCaseSim::EndNight(*Case, S);
					FString ExpectVo;
					if (Step->TryGetStringField(TEXT("expect_vo"), ExpectVo))
					{
						TestEqual(Where + TEXT(" dawn line"), Line.ToString(), ExpectVo);
					}
				}
				if (S.bClosed)
				{
					TestEqual(Where + TEXT(" ending"), S.Ending.ToString(), Step->GetStringField(TEXT("expect")));
					TestEqual(Where + TEXT(" rating"), S.Grade.ToString(), Step->GetStringField(TEXT("rating")));
					bClosed = true;
					break;
				}
			}
			else
			{
				AddError(Where + TEXT(": unknown step"));
			}
			TestTrue(FString::Printf(TEXT("%s: %s within the night"), *Where, *FCaseRules::Clock(S.Clock)), S.Clock <= FCaseSim::NightEnd(*Case, S));
		}
		TestTrue(Tag + TEXT(" closes the case"), bClosed);
		AddInfo(FString::Printf(TEXT("%s: %s %s on night %d, %d wrong red"), *Tag, *S.Ending.ToString(), *S.Grade.ToString(), S.Night + 1, S.WrongRed));
	}
	return true;
}

#endif
