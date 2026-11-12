// Project-wide UI theme. Edited in Project Settings > Game > Jam UI or in Config/DefaultGame.ini:
//   [/Script/JamUI.JamUISettings]
//   Accent=(R=1.0,G=0.55,B=0.1,A=1.0)
// Every UJamScreen reads it when it builds, so a theme change restyles all code-built UI at once.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Fonts/SlateFontInfo.h"
#include "JamUISettings.generated.h"

class USoundBase;

UENUM(BlueprintType)
enum class EJamTextRole : uint8
{
	Title,
	Heading,
	Body,
	Caption,
	Button
};

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Jam UI"))
class JAMUI_API UJamUISettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UJamUISettings();

	static const UJamUISettings& Get() { return *GetDefault<UJamUISettings>(); }

	/** Font for a text role: the configured font object, or the engine default font with this role's typeface and size. */
	FSlateFontInfo FontFor(EJamTextRole Role) const;
	float SizeFor(EJamTextRole Role) const;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Colors") FLinearColor Backdrop;   // full-screen dim behind menus
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Colors") FLinearColor Panel;
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Colors") FLinearColor Accent;
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Colors") FLinearColor ButtonNormal;
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Colors") FLinearColor ButtonHovered;
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Colors") FLinearColor ButtonPressed;
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Colors") FLinearColor Text;
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Colors") FLinearColor TextDim;
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Colors") FLinearColor Danger;

	/** Font family asset (UFont / composite font). Empty = engine default (Roboto). */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Fonts", meta = (AllowedClasses = "/Script/Engine.Font"))
	FSoftObjectPath FontFamily;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Fonts") float TitleSize = 48.f;
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Fonts") float HeadingSize = 30.f;
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Fonts") float BodySize = 20.f;
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Fonts") float CaptionSize = 14.f;
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Fonts") float ButtonSize = 22.f;
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Fonts") FName TitleTypeface = TEXT("Bold");
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Fonts") FName BodyTypeface = TEXT("Regular");

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Layout") float CornerRadius = 8.f;
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Layout") float Padding = 24.f;
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Layout") float Spacing = 12.f;
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Layout") float PanelWidth = 520.f;
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Layout") float ButtonHeight = 56.f;

	/** Played on button click / focus change; empty = silent. Generate with `jf sfx click|hover`. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Sound", meta = (AllowedClasses = "/Script/Engine.SoundBase"))
	FSoftObjectPath ClickSound;
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Sound", meta = (AllowedClasses = "/Script/Engine.SoundBase"))
	FSoftObjectPath HoverSound;
};
