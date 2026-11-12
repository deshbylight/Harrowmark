using UnrealBuildTool;

public class JamProbe : ModuleRules
{
	public JamProbe(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PrivateDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine", "RenderCore", "RHI", "JamCore", "MovieSceneCapture", "AudioMixer" });
	}
}
