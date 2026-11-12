using UnrealBuildTool;

public class Investigation : ModuleRules
{
	public Investigation(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine", "DeveloperSettings", "JamCore" });
		PrivateDependencyModuleNames.AddRange(new[] { "Json" });
	}
}
