// Code-built, themed screens. Subclass UJamScreen, override BuildScreen() and compose the tree with
// the Add* helpers; no widget Blueprint is needed (a WBP_ subclass may still add art on top).
// Buttons report through OnButton(Id) / OnButtonClicked; keyboard and gamepad focus work out of the box.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "JamUISettings.h"
#include "JamScreen.generated.h"

class UBorder;
class UJamScreen;
class UPanelWidget;
class UTextBlock;
class UVerticalBox;

UENUM(BlueprintType)
enum class EJamInputMode : uint8
{
	Game,        // input to the game, no cursor (HUDs)
	Menu,        // input to the UI only, cursor shown
	GameAndMenu  // both (inventories over a running game)
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FJamScreenButtonEvent, UJamScreen*, Screen, FName, ButtonId);

/** Button that remembers its id and forwards clicks/hover to the owning screen. */
UCLASS()
class JAMUI_API UJamButton : public UButton
{
	GENERATED_BODY()

public:
	void Bind(UJamScreen* InOwner, FName InId);
	FName GetId() const { return Id; }

	/** Normal look and the look while it has keyboard/gamepad focus (Slate buttons have no focus state of their own). */
	void SetJamStyles(const FButtonStyle& InNormal, const FButtonStyle& InFocused);
	/** Applies the focused/normal look; returns true when the focus state changed. */
	bool UpdateFocusVisual();

private:
	UFUNCTION() void HandleClicked();
	UFUNCTION() void HandleHovered();

	UPROPERTY() TWeakObjectPtr<UJamScreen> Owner;
	FName Id;
	FButtonStyle NormalStyle;
	FButtonStyle FocusedStyle;
	bool bShowingFocus = false;
};

UCLASS(Abstract, Blueprintable)
class JAMUI_API UJamScreen : public UUserWidget
{
	GENERATED_BODY()

public:
	/** How input behaves while this screen is on top of the stack. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam UI") EJamInputMode InputMode = EJamInputMode::Menu;
	/** Pause the game while on top (pause menus). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam UI") bool bPausesGame = false;
	/** Escape / gamepad B closes the screen. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam UI") bool bCloseOnBack = true;
	/** Hide screens below while this one is on top. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam UI") bool bCoversPrevious = true;

	UPROPERTY(BlueprintAssignable, Category = "Jam UI") FJamScreenButtonEvent OnButtonClicked;

	/** Gives keyboard/gamepad focus to the first button (called by the screen stack on push and reveal). */
	UFUNCTION(BlueprintCallable, Category = "Jam UI") void FocusFirstButton();
	UFUNCTION(BlueprintCallable, Category = "Jam UI") UJamButton* FindButton(FName Id) const;
	const TArray<TObjectPtr<UJamButton>>& GetButtons() const { return Buttons; }

	void NotifyButtonClicked(UJamButton* Button);
	void NotifyButtonHovered(UJamButton* Button);

protected:
	/** Build the widget tree here with the helpers below. Called once, before Slate widgets exist. */
	virtual void BuildScreen() {}
	/** Button reaction in C++; Blueprint/other code can bind OnButtonClicked instead. */
	virtual void OnButton(FName Id) {}
	/** Back action (Escape / gamepad B). Default: pop this screen when bCloseOnBack. */
	virtual void OnBack();

	// ---- layout helpers (theme from UJamUISettings) ----
	/** Full-screen root: optional dim backdrop and a centered panel; returns the panel's vertical box. */
	UVerticalBox* MakeCenteredPanel(bool bDimBackdrop = true);
	/** Root without panel (HUDs): returns an overlay-like vertical box filling the screen. */
	UVerticalBox* MakeFullScreenBox();
	UTextBlock* AddText(UPanelWidget* Parent, const FText& Text, EJamTextRole Role = EJamTextRole::Body, bool bDim = false);
	UJamButton* AddButton(UPanelWidget* Parent, const FText& Label, FName Id, bool bDanger = false);
	UVerticalBox* AddVerticalBox(UPanelWidget* Parent);
	void AddSpacer(UPanelWidget* Parent, float Height);

	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	/** Focus highlight and the focus-move sound for keyboard/gamepad navigation. */
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	void ApplyChildSpacing(UPanelWidget* Parent, UWidget* Child, bool bFill = false) const;
	void FocusWidget(UWidget* Widget);
	static void PlayUISound(const FSoftObjectPath& Sound, const UObject* Context);

	UPROPERTY() TArray<TObjectPtr<UJamButton>> Buttons;
	bool bBuilt = false;
};

USTRUCT(BlueprintType)
struct FJamMenuEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam UI") FText Label;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam UI") FName Id;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam UI") bool bDanger = false;
};

/**
 * Ready-made menu: title, optional subtitle, a column of buttons. Covers main menu, pause, game over
 * and settings stubs without writing a widget: fill the properties before pushing, or subclass.
 */
UCLASS(Blueprintable)
class JAMUI_API UJamMenuScreen : public UJamScreen
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam UI", meta = (ExposeOnSpawn = true)) FText Title;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam UI", meta = (ExposeOnSpawn = true)) FText Subtitle;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam UI", meta = (ExposeOnSpawn = true)) TArray<FJamMenuEntry> Entries;

protected:
	virtual void BuildScreen() override;
};
