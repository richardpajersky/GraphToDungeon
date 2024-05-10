// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class GraphToDungeonEditor : ModuleRules
{
	public GraphToDungeonEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
				"GraphToDungeonEditor/Private",
                "GraphToDungeonEditor/Public",
                "GraphToDungeonEditor/Private/Graph",
                "GraphToDungeonEditor/Public/Graph",
				"GraphToDungeonEditor/Private/AssetTypeActions",
                "GraphToDungeonEditor/Public/AssetTypeActions",
                "GraphToDungeonEditor/Private/Factories",
            }
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
                "PropertyEditor",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Projects",
				"InputCore",
				"EditorFramework",
				"UnrealEd",
				"ToolMenus",
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"GenericGraphRuntime",
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
