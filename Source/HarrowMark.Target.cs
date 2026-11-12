using UnrealBuildTool;

public class HarrowMarkTarget : TargetRules
{
	public HarrowMarkTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V7;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
		ExtraModuleNames.Add("HarrowMark");
	}
}
