using UnrealBuildTool;
public class CombatLabTarget:TargetRules {public CombatLabTarget(TargetInfo Target):base(Target){Type=TargetType.Game;DefaultBuildSettings=BuildSettingsVersion.V5;IncludeOrderVersion=EngineIncludeOrderVersion.Unreal5_6;ExtraModuleNames.Add("CombatLab");}}
