// The case across two maps: the harbour (L_Pier5) and the agency office uptown (L_Office, the hub of every
// case). Starts the configured case (UInvestigationSettings) on play; the case state lives in a game instance
// subsystem, so it survives the map change. A drive (UInvestigationSubsystem::OnDrive, also fired at dawn)
// fades to black and opens the other map; the player spawns at the PlayerStart tagged with the arrival:
//   Start  - the case start (night 1 at the rope on Pier 5);
//   Car    - the harbour, getting out of the car at the curb outside the Lighthouse;
//   Office - the office, in from the street door;
//   Dawn   - the office darkroom (every night ends there, GDD 9.4).

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HarrowGameMode.generated.h"

UCLASS()
class HARROWMARK_API AHarrowGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AHarrowGameMode();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drive") TSoftObjectPtr<UWorld> HarbourMap;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drive") TSoftObjectPtr<UWorld> HubMap;
	/** Fade to black before the map change, and back in after it. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drive") float FadeSeconds = 0.6f;

	/** PlayerStart tags (see the file comment). */
	static const FName StartTag;
	static const FName CarTag;
	static const FName OfficeTag;
	static const FName DawnTag;

	/** True when the loaded map is the office hub. */
	UFUNCTION(BlueprintPure, Category = "Drive") bool IsHubMap() const;

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void StartPlay() override;
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

protected:
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;

private:
	UFUNCTION() void HandleDrive(bool bAtHub);
	UFUNCTION() void HandleDawn(FName MonologueLine, int32 NewNight);
	void Travel();
	AActor* FindStart(FName Tag) const;
	void Fade(float From, float To) const;

	/** The arrival tag this map was opened with (travel option "Arrive"); none at the case start. */
	FName Arrival;
	bool bDawnPending = false;
	/** Travel() was called because play started on the wrong map, not by a drive. */
	bool bRedirect = false;
	FTimerHandle TravelTimer;
};
