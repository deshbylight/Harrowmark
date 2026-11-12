#include "JamUISubsystem.h"

#include "Blueprint/UserWidget.h"
#include "Engine/LocalPlayer.h"
#include "Framework/Application/SlateApplication.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"
#include "JamCore.h"
#include "JamScreen.h"

namespace
{
	// Above gameplay HUD widgets added with the default z-order 0.
	constexpr int32 BaseZOrder = 100;

	// Theme preview for screenshots: `jf shot --exec "@0:Jam.UI.Demo"`. "Jam.UI.Demo pop" closes it.
	FAutoConsoleCommandWithWorldAndArgs DemoCommand(
		TEXT("Jam.UI.Demo"),
		TEXT("Pushes a sample themed menu for the first local player (arg 'pop' removes the top screen)."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			UJamUISubsystem* UI = World ? UJamUISubsystem::Get(World->GetFirstPlayerController()) : nullptr;
			if (!UI)
			{
				UE_LOG(LogJam, Error, TEXT("[JamUI] Jam.UI.Demo: no local player in this world"));
				return;
			}
			if (Args.Num() > 0 && Args[0] == TEXT("pop"))
			{
				UI->Pop();
				return;
			}
			UI->PushMenu(FText::FromString(TEXT("Jam UI Demo")), FText::FromString(TEXT("Theme preview: title, body text, buttons.")),
				{{FText::FromString(TEXT("Play")), TEXT("Play"), false},
				 {FText::FromString(TEXT("Options")), TEXT("Options"), false},
				 {FText::FromString(TEXT("Quit")), TEXT("Quit"), true}});
		}));

	// Focus check for headless runs: `jf shot --exec "@2:Jam.UI.Demo; @5:Jam.UI.Focus"` then grep the log.
	FAutoConsoleCommandWithWorldAndArgs FocusCommand(
		TEXT("Jam.UI.Focus"),
		TEXT("Logs the widget that has user 0 focus, the top Jam screen and its focused button id."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>&, UWorld* World)
		{
			const bool bSlate = FSlateApplication::IsInitialized();
			const TSharedPtr<SWidget> Focused = bSlate ? FSlateApplication::Get().GetUserFocusedWidget(0) : nullptr;
			UJamUISubsystem* UI = World ? UJamUISubsystem::Get(World->GetFirstPlayerController()) : nullptr;
			const UJamScreen* Top = UI ? UI->Top() : nullptr;
			FName FocusedId = NAME_None;
			if (Top)
			{
				for (const UJamButton* Button : Top->GetButtons())
				{
					if (Button && Button->HasAnyUserFocus())
					{
						FocusedId = Button->GetId();
					}
				}
			}
			UE_LOG(LogJam, Display, TEXT("[JamUI] focus: widget=%s top=%s button=%s app_active=%d"),
				Focused ? *Focused->GetTypeAsString() : TEXT("none"), Top ? *Top->GetName() : TEXT("none"),
				*FocusedId.ToString(), bSlate && FSlateApplication::Get().IsActive() ? 1 : 0);
		}));
}

UJamUISubsystem* UJamUISubsystem::Get(const APlayerController* PlayerController)
{
	return ULocalPlayer::GetSubsystemFromController<UJamUISubsystem>(PlayerController);
}

APlayerController* UJamUISubsystem::GetPlayerController() const
{
	const ULocalPlayer* LocalPlayer = GetLocalPlayer();
	return LocalPlayer ? LocalPlayer->GetPlayerController(LocalPlayer->GetWorld()) : nullptr;
}

UJamScreen* UJamUISubsystem::Push(TSubclassOf<UJamScreen> ScreenClass)
{
	APlayerController* PC = GetPlayerController();
	if (!PC || !ScreenClass || ScreenClass->HasAnyClassFlags(CLASS_Abstract))
	{
		UE_LOG(LogJam, Error, TEXT("[JamUI] Push failed: %s"),
			!PC ? TEXT("no player controller") : !ScreenClass ? TEXT("no screen class") : TEXT("abstract screen class"));
		return nullptr;
	}
	UJamScreen* Screen = CreateWidget<UJamScreen>(PC, ScreenClass);
	if (!Screen)
	{
		UE_LOG(LogJam, Error, TEXT("[JamUI] CreateWidget failed for %s"), *ScreenClass->GetName());
		return nullptr;
	}
	Screen->AddToPlayerScreen(BaseZOrder + Stack.Num());
	Stack.Add(Screen);
	ApplyTop();
	return Screen;
}

UJamMenuScreen* UJamUISubsystem::PushMenu(const FText& Title, const FText& Subtitle, const TArray<FJamMenuEntry>& Entries)
{
	APlayerController* PC = GetPlayerController();
	if (!PC)
	{
		UE_LOG(LogJam, Error, TEXT("[JamUI] PushMenu failed: no player controller"));
		return nullptr;
	}
	// Properties must be set before the widget tree is built (on AddToPlayerScreen), so no Push() here.
	UJamMenuScreen* Menu = CreateWidget<UJamMenuScreen>(PC, UJamMenuScreen::StaticClass());
	if (!Menu)
	{
		UE_LOG(LogJam, Error, TEXT("[JamUI] CreateWidget failed for UJamMenuScreen"));
		return nullptr;
	}
	Menu->Title = Title;
	Menu->Subtitle = Subtitle;
	Menu->Entries = Entries;
	Menu->AddToPlayerScreen(BaseZOrder + Stack.Num());
	Stack.Add(Menu);
	ApplyTop();
	return Menu;
}

void UJamUISubsystem::Pop(UJamScreen* Screen)
{
	UJamScreen* Target = Screen ? Screen : Top();
	const int32 Index = Target ? Stack.Find(Target) : INDEX_NONE;
	if (Index == INDEX_NONE)
	{
		return;
	}
	Stack.RemoveAt(Index);
	Target->RemoveFromParent();
	ApplyTop();
}

void UJamUISubsystem::PopAll()
{
	for (UJamScreen* Screen : Stack)
	{
		if (Screen)
		{
			Screen->RemoveFromParent();
		}
	}
	Stack.Reset();
	ApplyTop();
}

UJamScreen* UJamUISubsystem::Top() const
{
	return Stack.Num() > 0 ? Stack.Last().Get() : nullptr;
}

void UJamUISubsystem::ApplyTop()
{
	Stack.RemoveAll([](const TObjectPtr<UJamScreen>& Screen) { return Screen == nullptr; });

	// Screens under a covering screen are hidden, the rest stay visible.
	bool bCovered = false;
	for (int32 i = Stack.Num() - 1; i >= 0; --i)
	{
		Stack[i]->SetVisibility(bCovered ? ESlateVisibility::Collapsed : ESlateVisibility::SelfHitTestInvisible);
		bCovered |= Stack[i]->bCoversPrevious;
	}
	if (UJamScreen* TopScreen = Top())
	{
		// The top screen must take input (buttons); SelfHitTestInvisible only skips the screen's own root.
		TopScreen->SetVisibility(ESlateVisibility::Visible);
	}

	APlayerController* PC = GetPlayerController();
	if (!PC)
	{
		return;
	}
	UJamScreen* TopScreen = Top();
	const EJamInputMode Mode = TopScreen ? TopScreen->InputMode : EJamInputMode::Game;
	switch (Mode)
	{
	case EJamInputMode::Menu:
	{
		FInputModeUIOnly Input;
		Input.SetWidgetToFocus(TopScreen->TakeWidget());
		Input.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(Input);
		PC->SetShowMouseCursor(true);
		break;
	}
	case EJamInputMode::GameAndMenu:
	{
		FInputModeGameAndUI Input;
		Input.SetWidgetToFocus(TopScreen->TakeWidget());
		Input.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		Input.SetHideCursorDuringCapture(false);
		PC->SetInputMode(Input);
		PC->SetShowMouseCursor(true);
		break;
	}
	case EJamInputMode::Game:
	default:
		PC->SetInputMode(FInputModeGameOnly());
		PC->SetShowMouseCursor(false);
		break;
	}

	const bool bWantPause = TopScreen && TopScreen->bPausesGame;
	if (PC->IsPaused() != bWantPause)
	{
		PC->SetPause(bWantPause);
	}
	if (TopScreen && Mode != EJamInputMode::Game)
	{
		TopScreen->FocusFirstButton();
	}
}

void UJamUISubsystem::Deinitialize()
{
	// Widgets die with the world; only drop references (no input-mode changes during teardown).
	Stack.Reset();
	Super::Deinitialize();
}
