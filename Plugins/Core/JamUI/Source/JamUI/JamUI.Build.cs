using UnrealBuildTool;

public class JamUI : ModuleRules
{
	public JamUI(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine", "UMG", "SlateCore", "DeveloperSettings", "JamCore" });
		PrivateDependencyModuleNames.AddRange(new[] { "Slate", "InputCore" });
	}
}
