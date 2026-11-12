// Builds the stock menu without a viewport and checks the generated tree, buttons and theme.

#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "JamScreen.h"
#include "JamUISettings.h"
#include "Misc/AutomationTest.h"
#include "UObject/Package.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamUIMenuBuildTest, "Project.JamUI.MenuBuildsTree",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FJamUIMenuBuildTest::RunTest(const FString& Parameters)
{
	UJamMenuScreen* Menu = NewObject<UJamMenuScreen>(GetTransientPackage());
	Menu->Title = FText::FromString(TEXT("Title"));
	Menu->Subtitle = FText::FromString(TEXT("Subtitle"));
	Menu->Entries = {
		{FText::FromString(TEXT("Play")), TEXT("Play"), false},
		{FText::FromString(TEXT("Options")), TEXT("Options"), false},
		{FText::FromString(TEXT("Quit")), TEXT("Quit"), true}};
	TestTrue(TEXT("Initialize"), Menu->Initialize());
	Menu->TakeWidget(); // runs RebuildWidget -> BuildScreen

	TestNotNull(TEXT("root widget built"), Menu->WidgetTree ? Menu->WidgetTree->RootWidget.Get() : nullptr);
	TestEqual(TEXT("one button per entry"), Menu->GetButtons().Num(), 3);
	TestNotNull(TEXT("FindButton(Quit)"), Menu->FindButton(TEXT("Quit")));
	TestNull(TEXT("FindButton(missing)"), Menu->FindButton(TEXT("Missing")));
	if (UJamButton* Play = Menu->FindButton(TEXT("Play")))
	{
		TestTrue(TEXT("click is bound"), Play->OnClicked.IsBound());
	}

	int32 Texts = 0;
	Menu->WidgetTree->ForEachWidget([&Texts](UWidget* Widget) { Texts += Widget->IsA<UTextBlock>() ? 1 : 0; });
	TestEqual(TEXT("title + subtitle + 3 labels"), Texts, 5);

	// Rebuilding the Slate widget must not duplicate the tree.
	Menu->ReleaseSlateResources(true);
	Menu->TakeWidget();
	TestEqual(TEXT("no duplicate buttons after rebuild"), Menu->GetButtons().Num(), 3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamUIThemeFontTest, "Project.JamUI.ThemeFonts",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FJamUIThemeFontTest::RunTest(const FString& Parameters)
{
	const UJamUISettings& Theme = UJamUISettings::Get();
	TestEqual(TEXT("title size"), static_cast<float>(Theme.FontFor(EJamTextRole::Title).Size), FMath::Max(Theme.TitleSize, 1.f));
	TestEqual(TEXT("body size"), static_cast<float>(Theme.FontFor(EJamTextRole::Body).Size), FMath::Max(Theme.BodySize, 1.f));
	TestTrue(TEXT("font resolves"), Theme.FontFor(EJamTextRole::Body).HasValidFont());
	return true;
}

#endif
