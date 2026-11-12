using UnrealBuildTool;

public class HarrowMarkEditorTarget : TargetRules
{
	public HarrowMarkEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V7;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
		ExtraModuleNames.Add("HarrowMark");
	}
}
