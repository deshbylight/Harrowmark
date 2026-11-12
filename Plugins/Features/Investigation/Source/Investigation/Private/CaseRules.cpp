#include "CaseRules.h"

#include "CaseDefinition.h"

// ---------------------------------------------------------------------------------------------------------
// FCaseRules

int32 FCaseRules::Minutes(const FString& HHMM)
{
	FString H, M;
	if (!HHMM.Split(TEXT(":"), &H, &M) || !H.IsNumeric() || !M.IsNumeric())
	{
		return INDEX_NONE;
	}
	int32 Hours = FCString::Atoi(*H);
	const int32 Mins = FCString::Atoi(*M);
	if (Hours < 0 || Hours > 23 || Mins < 0 || Mins > 59)
	{
		return INDEX_NONE;
	}
	if (Hours < 12)
	{
		Hours += 24;
	}
	return (Hours - 12) * 60 + Mins;
}

FString FCaseRules::Clock(int32 Minutes)
{
	const int32 Total = Minutes + 12 * 60;
	return FString::Printf(TEXT("%02d:%02d"), (Total / 60) % 24, Total % 60);
}

bool FCaseRules::Compatible(const TMap<FName, FName>& Places, FName A, FName B)
{
	auto IsAncestorOrSelf = [&Places](FName Ancestor, FName P)
	{
		for (int32 Guard = 0; !P.IsNone() && Guard < 64; ++Guard)
		{
			if (P == Ancestor)
			{
				return true;
			}
			const FName* Parent = Places.Find(P);
			P = Parent ? *Parent : NAME_None;
		}
		return false;
	};
	return IsAncestorOrSelf(A, B) || IsAncestorOrSelf(B, A);
}

bool FCaseRules::Conflicts(const FCaseAssertions& A, const FCaseAssertions& B, const TMap<FName, FName>& Places, TArray<FString>* OutWhy)
{
	bool bAny = false;
	for (const FCaseAt& X : A.At)
	{
		for (const FCaseAt& Y : B.At)
		{
			const bool bOverlap = X.From <= Y.To && Y.From <= X.To;
			if (X.Entity == Y.Entity && bOverlap && !Compatible(Places, X.Place, Y.Place))
			{
				bAny = true;
				if (OutWhy)
				{
					OutWhy->Add(FString::Printf(TEXT("%s: %s %s-%s vs %s %s-%s"), *X.Entity.ToString(), *X.Place.ToString(), *Clock(X.From),
						*Clock(X.To), *Y.Place.ToString(), *Clock(Y.From), *Clock(Y.To)));
				}
			}
		}
	}
	for (const TPair<FName, FName>& KV : A.Props)
	{
		const FName* Other = B.Props.Find(KV.Key);
		if (Other && *Other != KV.Value)
		{
			bAny = true;
			if (OutWhy)
			{
				OutWhy->Add(FString::Printf(TEXT("%s: %s vs %s"), *KV.Key.ToString(), *KV.Value.ToString(), *Other->ToString()));
			}
		}
	}
	return bAny;
}

// ---------------------------------------------------------------------------------------------------------
// FCaseSim

namespace
{
	bool RequirementsMet(const FCaseRequires& Req, const FCaseState& S)
	{
		for (FName Card : Req.Cards)
		{
			if (!S.Holds(Card))
			{
				return false;
			}
		}
		if (!Req.Gate.IsNone() && !S.Gates.Contains(Req.Gate))
		{
			return false;
		}
		if (!Req.Refuted.IsNone() && !S.Refuted.Contains(Req.Refuted))
		{
			return false;
		}
		return true;
	}

	/** One end of a red thread, resolved. */
	struct FRedItem
	{
		enum class EKind : uint8 { Hard, Statement, Person, Theory };
		EKind Kind = EKind::Person;
		FCaseAssertions Assertions;
		const FCaseCard* Card = nullptr;
		ECaseSlot Slot = ECaseSlot::Who;
	};

	bool Resolve(const UCaseDefinition& Case, const FCaseState& S, const FCaseThreadEnd& End, FRedItem& Out)
	{
		if (End.bSlot)
		{
			if (End.Slot == ECaseSlot::Why || S.Pinned(End.Slot).IsNone())
			{
				return false;
			}
			Out.Kind = FRedItem::EKind::Theory;
			Out.Slot = End.Slot;
			Out.Assertions = FCaseSim::SlotAssertions(Case, S, End.Slot);
			return true;
		}
		if (S.Met.Contains(End.Card))
		{
			Out.Kind = FRedItem::EKind::Person;
			return true;
		}
		const FCaseCard* Card = Case.FindCard(End.Card);
		if (!Card || !S.Holds(End.Card))
		{
			return false;
		}
		Out.Card = Card;
		Out.Kind = Card->IsHard() ? FRedItem::EKind::Hard : FRedItem::EKind::Statement;
		Out.Assertions = Card->Assertions;
		return true;
	}

	void Spend(FCaseState& S, int32 Minutes)
	{
		S.Clock += FMath::Max(0, Minutes);
	}
}

void FCaseSim::Start(const UCaseDefinition& Case, FCaseState& S)
{
	S = FCaseState();
	S.Night = 0;
	S.Clock = Case.Nights.IsEmpty() ? 0 : Case.Nights[0].Start;
	S.Film = Case.Costs.FilmPerNight;
	for (const FCaseShot& Shot : Case.Shots)
	{
		S.Film -= Shot.bPreExposed ? 1 : 0; // night 1 starts with these plates already exposed
	}
}

// The plates exposed before night 1 come out of the first bath, sharp, with their cards.
static void DevelopPreExposed(const UCaseDefinition& Case, FCaseState& S, TArray<FName>* OutNewCards)
{
	for (const FCaseShot& Shot : Case.Shots)
	{
		if (!Shot.bPreExposed || S.Taken.Contains(Shot.Id))
		{
			continue;
		}
		S.Taken.Add(Shot.Id);
		S.Sharp.Add(Shot.Id);
		for (FName Card : Shot.Yields)
		{
			if (!S.Holds(Card))
			{
				S.Hand.Add(Card);
				if (OutNewCards)
				{
					OutNewCards->Add(Card);
				}
			}
		}
	}
}

ECaseActionResult FCaseSim::CanTalk(const UCaseDefinition& Case, const FCaseState& S, FName Topic)
{
	const FCaseCard* Card = Case.FindCard(Topic);
	if (S.bClosed || !Card || Card->Kind != ECaseCardKind::Statement)
	{
		return ECaseActionResult::Invalid;
	}
	if (S.Holds(Topic))
	{
		return ECaseActionResult::AlreadyDone;
	}
	if (!RequirementsMet(Card->Requires, S))
	{
		return ECaseActionResult::Locked;
	}
	return WhereOk(Case, S, false);
}

ECaseActionResult FCaseSim::Talk(const UCaseDefinition& Case, FCaseState& S, FName Topic)
{
	const ECaseActionResult R = CanTalk(Case, S, Topic);
	if (R != ECaseActionResult::Ok)
	{
		return R;
	}
	const FCaseCard& Card = *Case.FindCard(Topic);
	Spend(S, Card.Cost >= 0 ? Card.Cost : Case.Costs.Topic);
	S.Hand.Add(Topic);
	S.Met.AddUnique(Card.Speaker);
	return ECaseActionResult::Ok;
}

bool FCaseSim::ShotPresent(const UCaseDefinition& Case, const FCaseState& S, FName Shot)
{
	const FCaseShot* Def = Case.FindShot(Shot);
	if (!Def)
	{
		return false;
	}
	const int32 NightNumber = S.Night + 1;
	if (Def->Expires > 0 && NightNumber > Def->Expires)
	{
		return false;
	}
	if (Def->WindowNight > 0 && (NightNumber != Def->WindowNight || S.Clock < Def->WindowFrom || S.Clock > Def->WindowTo))
	{
		return false;
	}
	return true;
}

ECaseActionResult FCaseSim::CanShoot(const UCaseDefinition& Case, const FCaseState& S, FName Shot)
{
	const FCaseShot* Def = Case.FindShot(Shot);
	if (S.bClosed || !Def || Def->bPreExposed)
	{
		return ECaseActionResult::Invalid;
	}
	if (!ShotPresent(Case, S, Shot))
	{
		return ECaseActionResult::Gone;
	}
	if (!RequirementsMet(Def->Requires, S))
	{
		return ECaseActionResult::Locked;
	}
	if (S.Sharp.Contains(Shot))
	{
		return ECaseActionResult::AlreadyDone;
	}
	if (WhereOk(Case, S, Case.IsHubPlace(Def->Place)) != ECaseActionResult::Ok)
	{
		return ECaseActionResult::Elsewhere;
	}
	const FName Forced = Case.ForcedShot();
	if (!Forced.IsNone() && !S.Taken.Contains(Forced) && Shot != Forced)
	{
		return ECaseActionResult::Tutorial;
	}
	if (S.Film <= 0)
	{
		return ECaseActionResult::NoFilm;
	}
	return ECaseActionResult::Ok;
}

ECaseActionResult FCaseSim::Shoot(const UCaseDefinition& Case, FCaseState& S, const TArray<FCaseShotHit>& Hits, TArray<FName>* OutNewCards)
{
	TArray<FCaseShotHit> Valid;
	ECaseActionResult FirstFailure = ECaseActionResult::Invalid;
	for (const FCaseShotHit& Hit : Hits)
	{
		const ECaseActionResult R = CanShoot(Case, S, Hit.Shot);
		if (R == ECaseActionResult::Ok)
		{
			if (!Valid.ContainsByPredicate([&Hit](const FCaseShotHit& V) { return V.Shot == Hit.Shot; }))
			{
				Valid.Add(Hit);
			}
		}
		else if (FirstFailure == ECaseActionResult::Invalid)
		{
			FirstFailure = R;
		}
	}
	if (Valid.IsEmpty())
	{
		return FirstFailure;
	}

	S.Film -= 1;
	Spend(S, Case.Costs.Photo);
	for (const FCaseShotHit& Hit : Valid)
	{
		const FCaseShot& Def = *Case.FindShot(Hit.Shot);
		S.Taken.Add(Hit.Shot);
		if (Hit.bSharp || Def.bForced)
		{
			S.Sharp.Add(Hit.Shot);
		}
		for (FName Card : Def.Yields)
		{
			if (!S.Holds(Card))
			{
				S.Hand.Add(Card);
				if (OutNewCards)
				{
					OutNewCards->Add(Card);
				}
			}
		}
	}
	return ECaseActionResult::Ok;
}

ECaseActionResult FCaseSim::WhereOk(const UCaseDefinition& Case, const FCaseState& S, bool bHubPlace)
{
	return Case.Hub.IsNone() || bHubPlace == S.bAtHub ? ECaseActionResult::Ok : ECaseActionResult::Elsewhere;
}

ECaseActionResult FCaseSim::Drive(const UCaseDefinition& Case, FCaseState& S)
{
	if (S.bClosed || Case.Hub.IsNone())
	{
		return ECaseActionResult::Invalid;
	}
	Spend(S, Case.Costs.Drive); // a fade and a short car shot
	S.bAtHub = !S.bAtHub;
	return ECaseActionResult::Ok;
}

ECaseActionResult FCaseSim::CanDevelop(const UCaseDefinition& Case, const FCaseState& S)
{
	return S.bClosed ? ECaseActionResult::Invalid : WhereOk(Case, S, true);
}

ECaseActionResult FCaseSim::Develop(const UCaseDefinition& Case, FCaseState& S, TArray<FName>* OutNewCards)
{
	const ECaseActionResult R = CanDevelop(Case, S);
	if (R != ECaseActionResult::Ok)
	{
		return R;
	}
	Spend(S, Case.Costs.Develop);
	DevelopPreExposed(Case, S, OutNewCards);
	S.Developed.Append(S.Sharp);
	return ECaseActionResult::Ok;
}

TArray<FName> FCaseSim::Enlargeable(const UCaseDefinition& Case, const FCaseState& S)
{
	TArray<FName> Out;
	for (const FCaseShot& Shot : Case.Shots)
	{
		if (!Shot.Detail.IsNone() && S.Developed.Contains(Shot.Id) && !S.Holds(Shot.Detail))
		{
			Out.Add(Shot.Detail);
		}
	}
	return Out;
}

ECaseActionResult FCaseSim::Enlarge(const UCaseDefinition& Case, FCaseState& S, FName Detail)
{
	if (S.Holds(Detail))
	{
		return ECaseActionResult::AlreadyDone;
	}
	if (!Enlargeable(Case, S).Contains(Detail))
	{
		return ECaseActionResult::Locked;
	}
	if (WhereOk(Case, S, true) != ECaseActionResult::Ok)
	{
		return ECaseActionResult::Elsewhere; // the loupe and the prints are in the darkroom
	}
	S.Hand.Add(Detail);
	return ECaseActionResult::Ok;
}

bool FCaseSim::TimeOfDeathKnown(const UCaseDefinition& Case, const FCaseState& S)
{
	for (FName Card : Case.TheoryWho.TimeFrom)
	{
		if (S.Holds(Card))
		{
			return true;
		}
	}
	return false;
}

FCaseAssertions FCaseSim::SlotAssertions(const UCaseDefinition& Case, const FCaseState& S, ECaseSlot Slot)
{
	FCaseAssertions A;
	if (Slot == ECaseSlot::Who && !S.PinWho.IsNone())
	{
		const FCasePerson* Person = Case.FindPerson(S.PinWho);
		A.Props.Add(Case.TheoryWho.Prop, Person ? Person->Height : NAME_None);
		if (TimeOfDeathKnown(Case, S))
		{
			FCaseAt& At = A.At.AddDefaulted_GetRef();
			At.Entity = S.PinWho;
			At.Place = Case.TheoryWho.Place;
			At.From = Case.TheoryWho.From;
			At.To = Case.TheoryWho.To;
		}
	}
	else if (Slot == ECaseSlot::With && !S.PinWith.IsNone())
	{
		if (const FCaseWeapon* W = Case.Weapons.Find(S.PinWith))
		{
			A.Props = W->Props;
			A.At = W->At;
		}
	}
	return A;
}

ECaseRedResult FCaseSim::JudgeRed(const UCaseDefinition& Case, const FCaseState& S, const FCaseThreadEnd& A, const FCaseThreadEnd& B, FName* OutStamped)
{
	if (S.bClosed)
	{
		return ECaseRedResult::Invalid;
	}
	if (A.bSlot == B.bSlot && (A.bSlot ? A.Slot == B.Slot : A.Card == B.Card))
	{
		return ECaseRedResult::Invalid;
	}
	FRedItem X, Y;
	if (!Resolve(Case, S, A, X) || !Resolve(Case, S, B, Y))
	{
		return ECaseRedResult::Invalid;
	}
	using EKind = FRedItem::EKind;
	auto Has = [&X, &Y](EKind K) { return X.Kind == K || Y.Kind == K; };
	const bool bConflict = FCaseRules::Conflicts(X.Assertions, Y.Assertions, Case.Places);

	if (Has(EKind::Hard) && Has(EKind::Theory))
	{
		return bConflict ? ECaseRedResult::Break : ECaseRedResult::NoContradiction;
	}
	if (Has(EKind::Hard) && Has(EKind::Statement))
	{
		if (!bConflict)
		{
			return ECaseRedResult::NoContradiction;
		}
		const FCaseCard* Stmt = X.Kind == EKind::Statement ? X.Card : Y.Card;
		if (OutStamped)
		{
			*OutStamped = Stmt->Id;
		}
		return Stmt->bMistake ? ECaseRedResult::Wrong : ECaseRedResult::Lie;
	}
	if (Has(EKind::Statement) && Has(EKind::Theory))
	{
		return ECaseRedResult::Word;
	}
	return ECaseRedResult::NoContradiction;
}

ECaseRedResult FCaseSim::Red(const UCaseDefinition& Case, FCaseState& S, const FCaseThreadEnd& A, const FCaseThreadEnd& B, FName* OutStamped)
{
	FName Stamped;
	const ECaseRedResult R = JudgeRed(Case, S, A, B, &Stamped);
	switch (R)
	{
	case ECaseRedResult::Lie:
	case ECaseRedResult::Wrong:
		S.Refuted.Add(Stamped);
		if (const FCaseCard* Stmt = Case.FindCard(Stamped); Stmt && !Stmt->Opens.IsNone())
		{
			S.Gates.Add(Stmt->Opens);
		}
		break;
	case ECaseRedResult::Break:
		Unpin(S, A.bSlot ? A.Slot : B.Slot);
		break;
	case ECaseRedResult::NoContradiction:
		S.WrongRed += 1;
		Spend(S, Case.Costs.WrongRed);
		break;
	default:
		break;
	}
	if (OutStamped)
	{
		*OutStamped = Stamped;
	}
	return R;
}

bool FCaseSim::CanPin(const UCaseDefinition& Case, const FCaseState& S, ECaseSlot Slot, FName Value)
{
	if (S.bClosed)
	{
		return false;
	}
	switch (Slot)
	{
	case ECaseSlot::Who:
	{
		// A suspect with an unlock card is pinnable once the player holds it (the detective: his mirror photo).
		const FName* Card = Case.Unlock.Find(Value);
		return Case.Suspects.Contains(Value) && (Card ? S.Holds(*Card) : S.Met.Contains(Value));
	}
	case ECaseSlot::With:
		return Case.Weapons.Contains(Value) && S.Holds(Value);
	case ECaseSlot::Why:
	{
		const FCaseCard* Card = Case.FindCard(Value);
		return Card && !Card->MotiveOf.IsNone() && S.Holds(Value);
	}
	}
	return false;
}

bool FCaseSim::Pin(const UCaseDefinition& Case, FCaseState& S, ECaseSlot Slot, FName Value)
{
	if (!CanPin(Case, S, Slot, Value))
	{
		return false;
	}
	(Slot == ECaseSlot::Who ? S.PinWho : Slot == ECaseSlot::With ? S.PinWith : S.PinWhy) = Value;
	return true;
}

void FCaseSim::Unpin(FCaseState& S, ECaseSlot Slot)
{
	(Slot == ECaseSlot::Who ? S.PinWho : Slot == ECaseSlot::With ? S.PinWith : S.PinWhy) = NAME_None;
}

int32 FCaseSim::NightStart(const UCaseDefinition& Case, const FCaseState& S)
{
	return Case.Nights.IsValidIndex(S.Night) ? Case.Nights[S.Night].Start : 0;
}

int32 FCaseSim::NightEnd(const UCaseDefinition& Case, const FCaseState& S)
{
	return Case.Nights.IsValidIndex(S.Night) ? Case.Nights[S.Night].End : 0;
}

bool FCaseSim::NightOver(const UCaseDefinition& Case, const FCaseState& S)
{
	return S.Clock >= NightEnd(Case, S);
}

bool FCaseSim::IsLastNight(const UCaseDefinition& Case, const FCaseState& S)
{
	return S.Night + 1 >= Case.Nights.Num();
}

FName FCaseSim::DawnLine(const UCaseDefinition& Case, const FCaseState& S)
{
	for (const FCaseMonologueRow& Row : Case.Monologue)
	{
		if (!Row.Who.IsNone() && S.PinWho != Row.Who)
		{
			continue;
		}
		if (!Row.WithIn.IsEmpty() && !Row.WithIn.Contains(S.PinWith))
		{
			continue;
		}
		if (!Row.LieOpen.IsNone() && S.Refuted.Contains(Row.LieOpen))
		{
			continue;
		}
		if (Row.Filled.ContainsByPredicate([&S](ECaseSlot Slot) { return S.Pinned(Slot).IsNone(); }))
		{
			continue;
		}
		if (Row.Empty.ContainsByPredicate([&S](ECaseSlot Slot) { return !S.Pinned(Slot).IsNone(); }))
		{
			continue;
		}
		if (!Row.EmptyAny.IsEmpty() && !Row.EmptyAny.ContainsByPredicate([&S](ECaseSlot Slot) { return S.Pinned(Slot).IsNone(); }))
		{
			continue;
		}
		if (Row.WhyWrong != ECaseTri::Any)
		{
			const FCaseCard* Why = S.PinWhy.IsNone() ? nullptr : Case.FindCard(S.PinWhy);
			const FName Owner = Why ? Why->MotiveOf : NAME_None;
			if ((Owner != S.PinWho) != (Row.WhyWrong == ECaseTri::Yes))
			{
				continue;
			}
		}
		return Row.Id;
	}
	return NAME_None;
}

FName FCaseSim::EndNight(const UCaseDefinition& Case, FCaseState& S, TArray<FName>* OutNewCards)
{
	if (S.bClosed)
	{
		return NAME_None;
	}
	if (IsLastNight(Case, S))
	{
		Close(Case, S); // the ship sails: the report goes with whatever is pinned
		return NAME_None;
	}
	const FName Line = DawnLine(Case, S);
	DevelopPreExposed(Case, S, OutNewCards);
	S.Developed.Append(S.Sharp);
	S.Night += 1;
	S.Clock = NightStart(Case, S);
	S.Film = Case.Costs.FilmPerNight;
	S.bAtHub = !Case.Hub.IsNone(); // dawn is in the darkroom; the next night starts at the office
	return Line;
}

bool FCaseSim::IsProven(const UCaseDefinition& Case, const FCaseState& S)
{
	return S.PinWho == Case.Proof.Who && S.PinWith == Case.Proof.With && Case.Proof.Why.Contains(S.PinWhy)
		&& (Case.Proof.Refuted.IsNone() || S.Refuted.Contains(Case.Proof.Refuted))
		&& (Case.Proof.HoldsAny.IsEmpty() || Case.Proof.HoldsAny.ContainsByPredicate([&S](FName Card) { return S.Holds(Card); }));
}

bool FCaseSim::CanReport(const FCaseState& S)
{
	return !S.bClosed && !S.PinWho.IsNone() && !S.PinWith.IsNone() && !S.PinWhy.IsNone();
}

void FCaseSim::Close(const UCaseDefinition& Case, FCaseState& S, FName Attached)
{
	if (S.bClosed)
	{
		return;
	}
	if (!Attached.IsNone() && !S.Holds(Attached))
	{
		Attached = NAME_None;
	}
	S.bProven = IsProven(Case, S);
	auto TriOk = [](ECaseTri Tri, bool bValue) { return Tri == ECaseTri::Any || (Tri == ECaseTri::Yes) == bValue; };

	S.Ending = NAME_None;
	for (const FCaseEnding& E : Case.Endings)
	{
		if (!TriOk(E.Proven, S.bProven)) continue;
		if (!E.Attached.IsNone() && E.Attached != Attached) continue;
		if (E.bHasWho && E.Who != S.PinWho) continue;
		S.Ending = E.Id;
		break;
	}
	S.Grade = NAME_None;
	for (const FCaseRating& R : Case.Ratings)
	{
		if (!TriOk(R.Proven, S.bProven)) continue;
		if (!R.Ending.IsNone() && R.Ending != S.Ending) continue;
		if (S.Night + 1 > R.MaxNight || S.WrongRed > R.MaxWrongRed) continue;
		S.Grade = R.Grade;
		break;
	}
	S.bClosed = true;
}
