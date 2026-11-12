#include "JamUISettings.h"

#include "Engine/Font.h"
#include "JamCore.h"
#include "Styling/CoreStyle.h"

UJamUISettings::UJamUISettings()
{
	CategoryName = TEXT("Game");

	// Neutral dark theme; the art director replaces it from docs/art/STYLE.md.
	Backdrop      = FLinearColor(0.f, 0.f, 0.f, 0.6f);
	Panel         = FLinearColor(0.02f, 0.022f, 0.028f, 0.92f);
	Accent        = FLinearColor(1.f, 0.55f, 0.1f, 1.f);
	ButtonNormal  = FLinearColor(0.06f, 0.065f, 0.08f, 1.f);
	ButtonHovered = FLinearColor(0.14f, 0.15f, 0.18f, 1.f);
	ButtonPressed = FLinearColor(0.04f, 0.04f, 0.05f, 1.f);
	Text          = FLinearColor(0.92f, 0.92f, 0.9f, 1.f);
	TextDim       = FLinearColor(0.55f, 0.56f, 0.58f, 1.f);
	Danger        = FLinearColor(0.85f, 0.15f, 0.12f, 1.f);
}

float UJamUISettings::SizeFor(EJamTextRole Role) const
{
	switch (Role)
	{
	case EJamTextRole::Title:   return TitleSize;
	case EJamTextRole::Heading: return HeadingSize;
	case EJamTextRole::Caption: return CaptionSize;
	case EJamTextRole::Button:  return ButtonSize;
	case EJamTextRole::Body:
	default:                    return BodySize;
	}
}

FSlateFontInfo UJamUISettings::FontFor(EJamTextRole Role) const
{
	const bool bBold = Role == EJamTextRole::Title || Role == EJamTextRole::Heading || Role == EJamTextRole::Button;
	const FName Typeface = bBold ? TitleTypeface : BodyTypeface;
	const float Size = FMath::Max(SizeFor(Role), 1.f);
	if (!FontFamily.IsNull())
	{
		if (const UFont* Font = Cast<UFont>(FontFamily.TryLoad()))
		{
			return FSlateFontInfo(Font, Size, Typeface);
		}
		UE_LOG(LogJam, Error, TEXT("[JamUI] FontFamily %s did not load as a UFont; using the engine font"), *FontFamily.ToString());
	}
	return FCoreStyle::GetDefaultFontStyle(Typeface, Size);
}
