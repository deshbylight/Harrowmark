#include "CaseDefinition.h"

#include "CaseRules.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "UObject/Package.h"

DEFINE_LOG_CATEGORY(LogInvestigation);

namespace
{
	using FJsonObj = TSharedPtr<FJsonObject>;
	using FJsonArr = TArray<TSharedPtr<FJsonValue>>;

	struct FParser
	{
		TArray<FString> Errors;

		void Error(const FString& Msg) { Errors.Add(Msg); }

		int32 Time(const FString& S, const FString& Where)
		{
			const int32 M = FCaseRules::Minutes(S);
			if (M == INDEX_NONE)
			{
				Error(FString::Printf(TEXT("%s: bad time '%s'"), *Where, *S));
				return 0;
			}
			return M;
		}

		FName Name(const FJsonObj& O, const TCHAR* Key)
		{
			FString S;
			return O.IsValid() && O->TryGetStringField(Key, S) ? FName(*S) : NAME_None;
		}

		FText Text(const FJsonObj& O, const TCHAR* Key)
		{
			FString S;
			return O.IsValid() && O->TryGetStringField(Key, S) ? FText::FromString(S) : FText::GetEmpty();
		}

		int32 Int(const FJsonObj& O, const TCHAR* Key, int32 Default)
		{
			double D = 0;
			return O.IsValid() && O->TryGetNumberField(Key, D) ? static_cast<int32>(D) : Default;
		}

		bool Bool(const FJsonObj& O, const TCHAR* Key, bool Default)
		{
			bool B = Default;
			return O.IsValid() && O->TryGetBoolField(Key, B) ? B : Default;
		}

		TArray<FName> Names(const FJsonObj& O, const TCHAR* Key)
		{
			TArray<FName> Out;
			const FJsonArr* Arr = nullptr;
			if (O.IsValid() && O->TryGetArrayField(Key, Arr))
			{
				for (const TSharedPtr<FJsonValue>& V : *Arr)
				{
					Out.Add(FName(*V->AsString()));
				}
			}
			return Out;
		}

		TArray<ECaseSlot> Slots(const FJsonObj& O, const TCHAR* Key)
		{
			TArray<ECaseSlot> Out;
			for (FName N : Names(O, Key))
			{
				if (N == TEXT("who")) Out.Add(ECaseSlot::Who);
				else if (N == TEXT("with")) Out.Add(ECaseSlot::With);
				else if (N == TEXT("why")) Out.Add(ECaseSlot::Why);
				else Error(FString::Printf(TEXT("unknown slot '%s'"), *N.ToString()));
			}
			return Out;
		}

		TMap<FName, FName> Props(const FJsonObj& O, const TCHAR* Key)
		{
			TMap<FName, FName> Out;
			const FJsonObj* P = nullptr;
			if (O.IsValid() && O->TryGetObjectField(Key, P))
			{
				for (const auto& KV : (*P)->Values)
				{
					Out.Add(FName(*KV.Key), FName(*KV.Value->AsString()));
				}
			}
			return Out;
		}

		TArray<FCaseAt> Ats(const FJsonObj& O, const TCHAR* Key, const FString& Where)
		{
			TArray<FCaseAt> Out;
			const FJsonArr* Rows = nullptr;
			if (O.IsValid() && O->TryGetArrayField(Key, Rows))
			{
				for (const TSharedPtr<FJsonValue>& Row : *Rows)
				{
					const FJsonArr& R = Row->AsArray();
					if (R.Num() != 4)
					{
						Error(FString::Printf(TEXT("%s: AT row needs 4 values"), *Where));
						continue;
					}
					FCaseAt& At = Out.AddDefaulted_GetRef();
					At.Entity = FName(*R[0]->AsString());
					At.Place = FName(*R[1]->AsString());
					At.From = Time(R[2]->AsString(), Where);
					At.To = Time(R[3]->AsString(), Where);
				}
			}
			return Out;
		}

		FCaseAssertions Assertions(const FJsonObj& O, const FString& Where)
		{
			FCaseAssertions A;
			A.At = Ats(O, TEXT("at"), Where);
			A.Props = Props(O, TEXT("prop"));
			return A;
		}

		FCaseRequires Requires(const FJsonObj& O)
		{
			FCaseRequires R;
			const FJsonObj* Req = nullptr;
			if (O.IsValid() && O->TryGetObjectField(TEXT("requires"), Req))
			{
				R.Cards = Names(*Req, TEXT("cards"));
				R.Gate = Name(*Req, TEXT("gate"));
				R.Refuted = Name(*Req, TEXT("refuted"));
			}
			return R;
		}

		ECaseTri Tri(const FJsonObj& O, const TCHAR* Key)
		{
			bool B = false;
			return O.IsValid() && O->TryGetBoolField(Key, B) ? (B ? ECaseTri::Yes : ECaseTri::No) : ECaseTri::Any;
		}
	};
}

bool UCaseDefinition::Reload(FString& OutError)
{
	FParser P;
	TSharedPtr<FJsonObject> Root;
	if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(SourceJson), Root) || !Root.IsValid())
	{
		OutError = TEXT("case JSON does not parse");
		return false;
	}

	CaseId = P.Name(Root, TEXT("id"));
	Title = P.Text(Root, TEXT("title"));
	Client = P.Name(Root, TEXT("client"));
	FString Tod;
	TimeOfDeath = Root->TryGetStringField(TEXT("time_of_death"), Tod) ? P.Time(Tod, TEXT("time_of_death")) : 0;

	Nights.Reset();
	const FJsonArr* NightRows = nullptr;
	if (Root->TryGetArrayField(TEXT("nights"), NightRows))
	{
		for (const TSharedPtr<FJsonValue>& Row : *NightRows)
		{
			const FJsonArr& R = Row->AsArray();
			FCaseNight& N = Nights.AddDefaulted_GetRef();
			N.Start = R.Num() > 0 ? P.Time(R[0]->AsString(), TEXT("nights")) : 0;
			N.End = R.Num() > 1 ? P.Time(R[1]->AsString(), TEXT("nights")) : 0;
		}
	}

	const FJsonObj* CostObj = nullptr;
	if (Root->TryGetObjectField(TEXT("costs"), CostObj))
	{
		Costs.Photo = P.Int(*CostObj, TEXT("photo"), 10);
		Costs.Topic = P.Int(*CostObj, TEXT("topic"), 15);
		Costs.WrongRed = P.Int(*CostObj, TEXT("wrong_red"), 60);
		Costs.Develop = P.Int(*CostObj, TEXT("develop"), 30);
		Costs.FilmPerNight = P.Int(*CostObj, TEXT("film_per_night"), 12);
		Costs.Drive = P.Int(*CostObj, TEXT("drive"), 10);
	}

	Places.Reset();
	const FJsonObj* PlaceObj = nullptr;
	if (Root->TryGetObjectField(TEXT("places"), PlaceObj))
	{
		for (const auto& KV : (*PlaceObj)->Values)
		{
			Places.Add(FName(*KV.Key), KV.Value->IsNull() ? NAME_None : FName(*KV.Value->AsString()));
		}
	}

	People.Reset();
	const FJsonObj* PeopleObj = nullptr;
	if (Root->TryGetObjectField(TEXT("people"), PeopleObj))
	{
		for (const auto& KV : (*PeopleObj)->Values)
		{
			const FJsonObj O = KV.Value->AsObject();
			FCasePerson& Person = People.AddDefaulted_GetRef();
			Person.Id = FName(*KV.Key);
			Person.Name = P.Text(O, TEXT("name"));
			Person.Height = P.Name(O, TEXT("height"));
		}
	}
	Suspects = P.Names(Root, TEXT("suspects"));
	Unlock.Reset();
	const FJsonObj* UnlockObj = nullptr;
	if (Root->TryGetObjectField(TEXT("unlock"), UnlockObj))
	{
		for (const TPair<FString, TSharedPtr<FJsonValue>>& KV : (*UnlockObj)->Values)
		{
			Unlock.Add(FName(*KV.Key), FName(*KV.Value->AsString()));
		}
	}

	Weapons.Reset();
	const FJsonObj* WeaponObj = nullptr;
	if (Root->TryGetObjectField(TEXT("weapons"), WeaponObj))
	{
		for (const auto& KV : (*WeaponObj)->Values)
		{
			// {"prop": {...}, "at": [[entity, place, from, to]]}, the same shape as a card's assertions
			const FCaseAssertions A = P.Assertions(KV.Value->AsObject(), FString(KV.Key));
			FCaseWeapon W;
			W.Props = A.Props;
			W.At = A.At;
			Weapons.Add(FName(*KV.Key), W);
		}
	}

	const FJsonObj* Who = nullptr;
	if (Root->TryGetObjectField(TEXT("theory_who"), Who))
	{
		TheoryWho.Place = P.Name(*Who, TEXT("place"));
		TheoryWho.From = P.Time((*Who)->GetStringField(TEXT("from")), TEXT("theory_who"));
		TheoryWho.To = P.Time((*Who)->GetStringField(TEXT("to")), TEXT("theory_who"));
		TheoryWho.Prop = P.Name(*Who, TEXT("prop"));
		TheoryWho.TimeFrom = P.Names(*Who, TEXT("time_from"));
	}

	const FJsonObj* TruthObj = nullptr;
	if (Root->TryGetObjectField(TEXT("truth"), TruthObj))
	{
		TruthKiller = P.Name(*TruthObj, TEXT("killer"));
		TruthWeapon = P.Name(*TruthObj, TEXT("weapon"));
		TruthMotives = P.Names(*TruthObj, TEXT("motives"));
		Truth = P.Assertions(*TruthObj, TEXT("truth"));
	}

	Shots.Reset();
	const FJsonArr* ShotRows = nullptr;
	if (Root->TryGetArrayField(TEXT("shots"), ShotRows))
	{
		for (const TSharedPtr<FJsonValue>& V : *ShotRows)
		{
			const FJsonObj O = V->AsObject();
			FCaseShot& S = Shots.AddDefaulted_GetRef();
			S.Id = P.Name(O, TEXT("id"));
			S.Label = P.Text(O, TEXT("label"));
			S.Place = P.Name(O, TEXT("place"));
			S.Yields = P.Names(O, TEXT("yields"));
			S.Detail = P.Name(O, TEXT("detail"));
			S.Expires = P.Int(O, TEXT("expires"), 0);
			S.bForced = P.Bool(O, TEXT("forced"), false);
			S.bPreExposed = P.Bool(O, TEXT("pre_exposed"), false);
			S.Requires = P.Requires(O);
			const FJsonObj* Window = nullptr;
			if (O->TryGetObjectField(TEXT("window"), Window))
			{
				S.WindowNight = P.Int(*Window, TEXT("night"), 0);
				S.WindowFrom = P.Time((*Window)->GetStringField(TEXT("from")), S.Id.ToString());
				S.WindowTo = P.Time((*Window)->GetStringField(TEXT("to")), S.Id.ToString());
			}
		}
	}

	Cards.Reset();
	const FJsonArr* CardRows = nullptr;
	if (Root->TryGetArrayField(TEXT("cards"), CardRows))
	{
		for (const TSharedPtr<FJsonValue>& V : *CardRows)
		{
			const FJsonObj O = V->AsObject();
			FCaseCard& C = Cards.AddDefaulted_GetRef();
			C.Id = P.Name(O, TEXT("id"));
			const FName Kind = P.Name(O, TEXT("kind"));
			if (Kind == TEXT("anchor")) C.Kind = ECaseCardKind::Anchor;
			else if (Kind == TEXT("detail")) C.Kind = ECaseCardKind::Detail;
			else if (Kind == TEXT("statement")) C.Kind = ECaseCardKind::Statement;
			else P.Error(FString::Printf(TEXT("card %s: unknown kind '%s'"), *C.Id.ToString(), *Kind.ToString()));
			C.Text = P.Text(O, TEXT("text"));
			C.Topic = P.Text(O, TEXT("topic"));
			C.Speaker = P.Name(O, TEXT("speaker"));
			C.bTrue = P.Bool(O, TEXT("true"), true);
			C.bMistake = P.Name(O, TEXT("false_kind")) == TEXT("mistake");
			C.Cost = P.Int(O, TEXT("cost"), -1);
			C.Requires = P.Requires(O);
			C.Opens = P.Name(O, TEXT("opens"));
			C.MotiveOf = P.Name(O, TEXT("motive_of"));
			C.FromShot = P.Name(O, TEXT("from"));
			C.bObject = P.Bool(O, TEXT("object"), false);
			C.Assertions = P.Assertions(O, C.Id.ToString());
		}
	}

	Monologue.Reset();
	const FJsonArr* MonoRows = nullptr;
	if (Root->TryGetArrayField(TEXT("monologue"), MonoRows))
	{
		for (const TSharedPtr<FJsonValue>& V : *MonoRows)
		{
			const FJsonObj O = V->AsObject();
			FCaseMonologueRow& M = Monologue.AddDefaulted_GetRef();
			M.Id = P.Name(O, TEXT("id"));
			M.Text = P.Text(O, TEXT("text"));
			const FJsonObj* When = nullptr;
			if (O->TryGetObjectField(TEXT("when"), When))
			{
				M.Who = P.Name(*When, TEXT("who"));
				M.WithIn = P.Names(*When, TEXT("with_in"));
				M.LieOpen = P.Name(*When, TEXT("lie_open"));
				M.Filled = P.Slots(*When, TEXT("filled"));
				M.Empty = P.Slots(*When, TEXT("empty"));
				M.EmptyAny = P.Slots(*When, TEXT("empty_any"));
				M.WhyWrong = P.Tri(*When, TEXT("why_wrong"));
			}
		}
	}

	Endings.Reset();
	const FJsonObj* EndObj = nullptr;
	if (Root->TryGetObjectField(TEXT("endings"), EndObj))
	{
		const FJsonObj* ProofObj = nullptr;
		if ((*EndObj)->TryGetObjectField(TEXT("proof"), ProofObj))
		{
			Proof.Who = P.Name(*ProofObj, TEXT("who"));
			Proof.With = P.Name(*ProofObj, TEXT("with"));
			Proof.Why = P.Names(*ProofObj, TEXT("why"));
			Proof.Refuted = P.Name(*ProofObj, TEXT("refuted"));
			Proof.HoldsAny = P.Names(*ProofObj, TEXT("holds_any"));
		}
		const FJsonArr* Order = nullptr;
		if ((*EndObj)->TryGetArrayField(TEXT("order"), Order))
		{
			for (const TSharedPtr<FJsonValue>& V : *Order)
			{
				const FJsonObj O = V->AsObject();
				FCaseEnding& E = Endings.AddDefaulted_GetRef();
				E.Id = P.Name(O, TEXT("id"));
				E.Title = P.Text(O, TEXT("title"));
				E.Proven = P.Tri(O, TEXT("proven"));
				E.Attached = P.Name(O, TEXT("attached"));
				if (const TSharedPtr<FJsonValue>* WhoV = O->Values.Find(TEXT("who")))
				{
					E.bHasWho = true;
					E.Who = (*WhoV)->IsNull() ? NAME_None : FName(*(*WhoV)->AsString());
				}
			}
		}
	}

	Ratings.Reset();
	const FJsonArr* RatingRows = nullptr;
	if (Root->TryGetArrayField(TEXT("rating"), RatingRows))
	{
		for (const TSharedPtr<FJsonValue>& V : *RatingRows)
		{
			const FJsonObj O = V->AsObject();
			FCaseRating& R = Ratings.AddDefaulted_GetRef();
			R.Grade = P.Name(O, TEXT("grade"));
			R.Proven = P.Tri(O, TEXT("proven"));
			R.Ending = P.Name(O, TEXT("ending"));
			R.MaxNight = P.Int(O, TEXT("max_night"), 99);
			R.MaxWrongRed = P.Int(O, TEXT("max_wrong_red"), 99);
		}
	}

	Reindex();

	// Referential checks the game relies on (the full logic check is case_check.py / Case01Solvable).
	for (const FCaseShot& S : Shots)
	{
		for (FName A : S.Yields)
		{
			if (!FindCard(A)) P.Error(FString::Printf(TEXT("shot %s yields unknown card %s"), *S.Id.ToString(), *A.ToString()));
		}
		if (!Places.Contains(S.Place)) P.Error(FString::Printf(TEXT("shot %s: unknown place %s"), *S.Id.ToString(), *S.Place.ToString()));
	}
	Hub = P.Name(Root, TEXT("hub"));
	if (!Hub.IsNone() && !Places.Contains(Hub)) P.Error(FString::Printf(TEXT("hub %s is not a place"), *Hub.ToString()));
	if (Nights.IsEmpty()) P.Error(TEXT("no nights"));
	if (ForcedShot().IsNone()) P.Error(TEXT("no forced tutorial shot"));

	if (!P.Errors.IsEmpty())
	{
		OutError = FString::Join(P.Errors, TEXT("; "));
		return false;
	}
	return true;
}

bool UCaseDefinition::LoadFromJson(const FString& Json, FString& OutError)
{
	SourceJson = Json;
	return Reload(OutError);
}

UCaseDefinition* UCaseDefinition::LoadFromFile(const FString& Path, FString& OutError, UObject* Outer)
{
	FString Json;
	if (!FFileHelper::LoadFileToString(Json, *Path))
	{
		OutError = FString::Printf(TEXT("cannot read %s"), *Path);
		return nullptr;
	}
	UCaseDefinition* Case = NewObject<UCaseDefinition>(Outer ? Outer : GetTransientPackage());
	return Case->LoadFromJson(Json, OutError) ? Case : nullptr;
}

void UCaseDefinition::PostLoad()
{
	Super::PostLoad();
	if (!SourceJson.IsEmpty())
	{
		FString Error;
		if (!Reload(Error))
		{
			UE_LOG(LogInvestigation, Error, TEXT("[Investigation] %s: %s"), *GetPathName(), *Error);
		}
	}
}

#if WITH_EDITOR
void UCaseDefinition::PostEditChangeProperty(FPropertyChangedEvent& Event)
{
	Super::PostEditChangeProperty(Event);
	FString Error;
	if (!Reload(Error))
	{
		UE_LOG(LogInvestigation, Error, TEXT("[Investigation] %s: %s"), *GetPathName(), *Error);
	}
}
#endif

void UCaseDefinition::Reindex()
{
	CardIndex.Reset();
	ShotIndex.Reset();
	SourceIndex.Reset();
	for (int32 i = 0; i < Cards.Num(); ++i)
	{
		CardIndex.Add(Cards[i].Id, i);
		if (Cards[i].Kind == ECaseCardKind::Detail)
		{
			SourceIndex.Add(Cards[i].Id, Cards[i].FromShot);
		}
	}
	for (int32 i = 0; i < Shots.Num(); ++i)
	{
		ShotIndex.Add(Shots[i].Id, i);
		for (FName A : Shots[i].Yields)
		{
			SourceIndex.Add(A, Shots[i].Id);
		}
	}
}

const FCaseCard* UCaseDefinition::FindCard(FName Id) const
{
	const int32* I = CardIndex.Find(Id);
	return I ? &Cards[*I] : nullptr;
}

const FCaseShot* UCaseDefinition::FindShot(FName Id) const
{
	const int32* I = ShotIndex.Find(Id);
	return I ? &Shots[*I] : nullptr;
}

const FCasePerson* UCaseDefinition::FindPerson(FName Id) const
{
	return People.FindByPredicate([Id](const FCasePerson& P) { return P.Id == Id; });
}

const FCaseMonologueRow* UCaseDefinition::FindMonologue(FName Id) const
{
	return Monologue.FindByPredicate([Id](const FCaseMonologueRow& R) { return R.Id == Id; });
}

const FCaseEnding* UCaseDefinition::FindEnding(FName Id) const
{
	return Endings.FindByPredicate([Id](const FCaseEnding& E) { return E.Id == Id; });
}

const FCaseShot* UCaseDefinition::SourceShot(FName CardId) const
{
	const FName* Shot = SourceIndex.Find(CardId);
	return Shot ? FindShot(*Shot) : nullptr;
}

bool UCaseDefinition::IsHubPlace(FName Place) const
{
	for (int32 Guard = 0; !Hub.IsNone() && !Place.IsNone() && Guard < 64; ++Guard)
	{
		if (Place == Hub)
		{
			return true;
		}
		const FName* Parent = Places.Find(Place);
		Place = Parent ? *Parent : NAME_None;
	}
	return false;
}

FName UCaseDefinition::ForcedShot() const
{
	const FCaseShot* S = Shots.FindByPredicate([](const FCaseShot& Shot) { return Shot.bForced; });
	return S ? S->Id : NAME_None;
}

TArray<FName> UCaseDefinition::TopicsOf(FName Speaker) const
{
	TArray<FName> Out;
	for (const FCaseCard& C : Cards)
	{
		if (C.Kind == ECaseCardKind::Statement && C.Speaker == Speaker)
		{
			Out.Add(C.Id);
		}
	}
	return Out;
}
