// Per-player screen stack: Push/Pop handle viewport, z-order, input mode, cursor, pause and focus.
//   UJamUISubsystem::Get(PC)->Push(UMyPauseScreen::StaticClass());

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "JamUISubsystem.generated.h"

class APlayerController;
class UJamMenuScreen;
class UJamScreen;
struct FJamMenuEntry;

UCLASS()
class JAMUI_API UJamUISubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Jam UI", meta = (DefaultToSelf = "PlayerController"))
	static UJamUISubsystem* Get(const APlayerController* PlayerController);

	/** Creates the screen, adds it on top and applies its input mode / pause / focus. */
	UFUNCTION(BlueprintCallable, Category = "Jam UI", meta = (DeterminesOutputType = "ScreenClass"))
	UJamScreen* Push(TSubclassOf<UJamScreen> ScreenClass);

	/** Pushes a UJamMenuScreen filled with these values. */
	UFUNCTION(BlueprintCallable, Category = "Jam UI")
	UJamMenuScreen* PushMenu(const FText& Title, const FText& Subtitle, const TArray<FJamMenuEntry>& Entries);

	/** Removes the given screen (default: the top one); the screen below gets input mode and focus back. */
	UFUNCTION(BlueprintCallable, Category = "Jam UI")
	void Pop(UJamScreen* Screen = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Jam UI") void PopAll();
	UFUNCTION(BlueprintPure, Category = "Jam UI") UJamScreen* Top() const;
	UFUNCTION(BlueprintPure, Category = "Jam UI") int32 Num() const { return Stack.Num(); }

	virtual void Deinitialize() override;

private:
	APlayerController* GetPlayerController() const;
	void ApplyTop();

	UPROPERTY() TArray<TObjectPtr<UJamScreen>> Stack;
};
