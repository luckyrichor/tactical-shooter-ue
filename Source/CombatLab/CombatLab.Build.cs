using UnrealBuildTool;
public class CombatLab:ModuleRules {public CombatLab(ReadOnlyTargetRules Target):base(Target){PCHUsage=PCHUsageMode.UseExplicitOrSharedPCHs;PublicDependencyModuleNames.AddRange(new[]{"Core","CoreUObject","Engine","InputCore","GameplayAbilities","GameplayTags","GameplayTasks","AIModule","NavigationSystem"});}}
