// Smoke tests that keep the `Project.` test filter non-empty from day one.
// Feature plugins add their own tests under Project.<Feature>.<Case>.

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamProbeEngineSmokeTest, "Project.JamProbe.EngineAlive",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FJamProbeEngineSmokeTest::RunTest(const FString& Parameters)
{
	TestNotNull(TEXT("GEngine exists"), GEngine);
	return true;
}

#endif
