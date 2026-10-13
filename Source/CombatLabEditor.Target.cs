using UnrealBuildTool;
public class CombatLabEditorTarget:TargetRules {public CombatLabEditorTarget(TargetInfo Target):base(Target){Type=TargetType.Editor;DefaultBuildSettings=BuildSettingsVersion.V5;IncludeOrderVersion=EngineIncludeOrderVersion.Unreal5_6;ExtraModuleNames.Add("CombatLab");}}
