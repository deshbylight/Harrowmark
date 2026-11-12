using UnrealBuildTool;

// Game glue only. Features live in plugins under Plugins/Features/<Name>; shared contracts in JamCore.
public class HarrowMark : ModuleRules
{
	public HarrowMark(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "JamCore", "Investigation" });
	}
}
