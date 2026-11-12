#include "JamScreen.h"

#include "Blueprint/WidgetTree.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Components/Border.h"
#include "Components/ButtonSlot.h"
#include "Components/SizeBox.h"
#include "Components/Spacer.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Framework/Application/SlateApplication.h"
#include "JamCore.h"
#include "JamUISubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

// ---------------------------------------------------------------- UJamButton

void UJamButton::Bind(UJamScreen* InOwner, FName InId)
{
	Owner = InOwner;
	Id = InId;
	OnClicked.AddUniqueDynamic(this, &UJamButton::HandleClicked);
	OnHovered.AddUniqueDynamic(this, &UJamButton::HandleHovered);
}

void UJamButton::SetJamStyles(const FButtonStyle& InNormal, const FButtonStyle& InFocused)
{
	NormalStyle = InNormal;
	FocusedStyle = InFocused;
	bShowingFocus = false;
	SetStyle(NormalStyle);
}

bool UJamButton::UpdateFocusVisual()
{
	const bool bFocused = HasAnyUserFocus() || HasKeyboardFocus();
	if (bFocused == bShowingFocus)
	{
		return false;
	}
	bShowingFocus = bFocused;
	SetStyle(bFocused ? FocusedStyle : NormalStyle);
	return true;
}

void UJamButton::HandleClicked()
{
	if (UJamScreen* Screen = Owner.Get())
	{
		Screen->NotifyButtonClicked(this);
	}
}

void UJamButton::HandleHovered()
{
	if (UJamScreen* Screen = Owner.Get())
	{
		Screen->NotifyButtonHovered(this);
	}
}

// ---------------------------------------------------------------- UJamScreen

TSharedRef<SWidget> UJamScreen::RebuildWidget()
{
	if (!bBuilt && WidgetTree && !IsDesignTime())
	{
		bBuilt = true;
		SetIsFocusable(true); // receives Escape / gamepad B when no button has focus
		if (!WidgetTree->RootWidget)
		{
			BuildScreen();
		}
	}
	return Super::RebuildWidget();
}

FReply UJamScreen::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const bool bBack = InKeyEvent.GetKey() == EKeys::Escape
		|| (FSlateApplication::IsInitialized()
			&& FSlateApplication::Get().GetNavigationActionFromKey(InKeyEvent) == EUINavigationAction::Back);
	if (bBack && bCloseOnBack)
	{
		OnBack();
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UJamScreen::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	for (UJamButton* Button : Buttons)
	{
		// Focus moved here by keyboard/gamepad: same feedback as a mouse hover (the mouse already got its sound).
		if (Button && Button->UpdateFocusVisual() && Button->HasAnyUserFocus() && !Button->IsHovered())
		{
			PlayUISound(UJamUISettings::Get().HoverSound, this);
		}
	}
}

void UJamScreen::OnBack()
{
	if (UJamUISubsystem* UI = UJamUISubsystem::Get(GetOwningPlayer()))
	{
		UI->Pop(this);
	}
	else
	{
		RemoveFromParent();
	}
}

void UJamScreen::FocusFirstButton()
{
	for (UJamButton* Button : Buttons)
	{
		if (Button && Button->GetIsEnabled() && Button->IsVisible())
		{
			FocusWidget(Button);
			return;
		}
	}
	// No buttons: the screen itself takes focus so Back still works.
	FocusWidget(this);
}

UJamButton* UJamScreen::FindButton(FName Id) const
{
	for (UJamButton* Button : Buttons)
	{
		if (Button && Button->GetId() == Id)
		{
			return Button;
		}
	}
	return nullptr;
}

void UJamScreen::NotifyButtonClicked(UJamButton* Button)
{
	check(Button);
	PlayUISound(UJamUISettings::Get().ClickSound, this);
	const FName Id = Button->GetId();
	OnButton(Id);
	OnButtonClicked.Broadcast(this, Id);
}

void UJamScreen::NotifyButtonHovered(UJamButton* Button)
{
	check(Button);
	// Mouse and keyboard/gamepad share one selection: hovering moves focus, so two buttons never
	// look selected at once and navigation continues from where the mouse was.
	if (!Button->HasAnyUserFocus())
	{
		FocusWidget(Button);
	}
	PlayUISound(UJamUISettings::Get().HoverSound, this);
}

void UJamScreen::FocusWidget(UWidget* Widget)
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		Widget->SetUserFocus(PC);
	}
	else
	{
		Widget->SetKeyboardFocus();
	}
}

void UJamScreen::PlayUISound(const FSoftObjectPath& Sound, const UObject* Context)
{
	if (Sound.IsNull())
	{
		return;
	}
	if (USoundBase* Loaded = Cast<USoundBase>(Sound.TryLoad()))
	{
		UGameplayStatics::PlaySound2D(Context, Loaded);
	}
	else
	{
		UE_LOG(LogJam, Error, TEXT("[JamUI] UI sound %s did not load"), *Sound.ToString());
	}
}

UVerticalBox* UJamScreen::MakeCenteredPanel(bool bDimBackdrop)
{
	const UJamUISettings& Theme = UJamUISettings::Get();

	UBorder* Backdrop = WidgetTree->ConstructWidget<UBorder>();
	Backdrop->SetBrushColor(bDimBackdrop ? Theme.Backdrop : FLinearColor::Transparent);
	Backdrop->SetHorizontalAlignment(HAlign_Center);
	Backdrop->SetVerticalAlignment(VAlign_Center);
	Backdrop->SetPadding(FMargin(0.f));
	WidgetTree->RootWidget = Backdrop;

	USizeBox* Width = WidgetTree->ConstructWidget<USizeBox>();
	Width->SetWidthOverride(Theme.PanelWidth);
	Backdrop->SetContent(Width);

	UBorder* Panel = WidgetTree->ConstructWidget<UBorder>();
	Panel->SetBrush(FSlateRoundedBoxBrush(FLinearColor::White, Theme.CornerRadius));
	Panel->SetBrushColor(Theme.Panel);
	Panel->SetPadding(FMargin(Theme.Padding));
	Width->SetContent(Panel);

	UVerticalBox* Box = WidgetTree->ConstructWidget<UVerticalBox>();
	Panel->SetContent(Box);
	return Box;
}

UVerticalBox* UJamScreen::MakeFullScreenBox()
{
	UVerticalBox* Box = WidgetTree->ConstructWidget<UVerticalBox>();
	WidgetTree->RootWidget = Box;
	return Box;
}

void UJamScreen::ApplyChildSpacing(UPanelWidget* Parent, UWidget* Child, bool bFill) const
{
	if (UVerticalBoxSlot* BoxSlot = Cast<UVerticalBoxSlot>(Child->Slot))
	{
		const bool bFirst = Parent->GetChildIndex(Child) == 0;
		BoxSlot->SetPadding(FMargin(0.f, bFirst ? 0.f : UJamUISettings::Get().Spacing, 0.f, 0.f));
		BoxSlot->SetHorizontalAlignment(HAlign_Fill);
		if (bFill)
		{
			BoxSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		}
	}
}

UTextBlock* UJamScreen::AddText(UPanelWidget* Parent, const FText& Text, EJamTextRole Role, bool bDim)
{
	check(Parent);
	const UJamUISettings& Theme = UJamUISettings::Get();
	UTextBlock* Block = WidgetTree->ConstructWidget<UTextBlock>();
	Block->SetText(Text);
	Block->SetFont(Theme.FontFor(Role));
	Block->SetColorAndOpacity(FSlateColor(bDim ? Theme.TextDim : (Role == EJamTextRole::Title ? Theme.Accent : Theme.Text)));
	Block->SetAutoWrapText(true);
	if (Role == EJamTextRole::Title || Role == EJamTextRole::Heading)
	{
		Block->SetJustification(ETextJustify::Center);
	}
	Parent->AddChild(Block);
	ApplyChildSpacing(Parent, Block);
	return Block;
}

UJamButton* UJamScreen::AddButton(UPanelWidget* Parent, const FText& Label, FName Id, bool bDanger)
{
	check(Parent);
	const UJamUISettings& Theme = UJamUISettings::Get();

	FButtonStyle Style;
	const auto Brush = [&Theme](const FLinearColor& Fill, const FLinearColor& Outline, float OutlineWidth)
	{
		return FSlateRoundedBoxBrush(Fill, Theme.CornerRadius, Outline, OutlineWidth);
	};
	const FLinearColor Edge = bDanger ? Theme.Danger : Theme.Accent;
	Style.SetNormal(Brush(Theme.ButtonNormal, Theme.ButtonNormal, 0.f));
	Style.SetHovered(Brush(Theme.ButtonHovered, Edge, 2.f));
	Style.SetPressed(Brush(Theme.ButtonPressed, Edge, 2.f));
	Style.SetDisabled(Brush(Theme.ButtonNormal * FLinearColor(1.f, 1.f, 1.f, 0.4f), Theme.ButtonNormal, 0.f));
	Style.SetNormalPadding(FMargin(0.f));
	Style.SetPressedPadding(FMargin(0.f));
	// Keyboard/gamepad focus looks like hover, so navigation without a mouse is visible.
	FButtonStyle Focused = Style;
	Focused.SetNormal(Style.Hovered);

	UJamButton* Button = WidgetTree->ConstructWidget<UJamButton>();
	Button->SetJamStyles(Style, Focused);
	Button->Bind(this, Id);

	USizeBox* Height = WidgetTree->ConstructWidget<USizeBox>();
	Height->SetHeightOverride(Theme.ButtonHeight);

	UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>();
	Text->SetText(Label);
	Text->SetFont(Theme.FontFor(EJamTextRole::Button));
	Text->SetColorAndOpacity(FSlateColor(bDanger ? Theme.Danger : Theme.Text));
	Text->SetJustification(ETextJustify::Center);
	Button->SetContent(Text);
	if (UButtonSlot* LabelSlot = Cast<UButtonSlot>(Text->Slot))
	{
		LabelSlot->SetHorizontalAlignment(HAlign_Center);
		LabelSlot->SetVerticalAlignment(VAlign_Center);
	}
	Height->SetContent(Button);

	Parent->AddChild(Height);
	ApplyChildSpacing(Parent, Height);
	Buttons.Add(Button);
	return Button;
}

UVerticalBox* UJamScreen::AddVerticalBox(UPanelWidget* Parent)
{
	check(Parent);
	UVerticalBox* Box = WidgetTree->ConstructWidget<UVerticalBox>();
	Parent->AddChild(Box);
	ApplyChildSpacing(Parent, Box);
	return Box;
}

void UJamScreen::AddSpacer(UPanelWidget* Parent, float Height)
{
	check(Parent);
	USpacer* Spacer = WidgetTree->ConstructWidget<USpacer>();
	Spacer->SetSize(FVector2D(0.f, FMath::Max(Height, 0.f)));
	Parent->AddChild(Spacer);
	ApplyChildSpacing(Parent, Spacer);
}

// ---------------------------------------------------------------- UJamMenuScreen

void UJamMenuScreen::BuildScreen()
{
	UVerticalBox* Box = MakeCenteredPanel(/*bDimBackdrop*/ true);
	if (!Title.IsEmpty())
	{
		AddText(Box, Title, EJamTextRole::Title);
	}
	if (!Subtitle.IsEmpty())
	{
		AddText(Box, Subtitle, EJamTextRole::Body, /*bDim*/ true);
	}
	if (!Title.IsEmpty() || !Subtitle.IsEmpty())
	{
		AddSpacer(Box, UJamUISettings::Get().Spacing);
	}
	for (const FJamMenuEntry& Entry : Entries)
	{
		AddButton(Box, Entry.Label, Entry.Id, Entry.bDanger);
	}
}
