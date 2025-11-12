// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class sazandora : ModuleRules
{
	public sazandora(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "HTTP", "Json", "JsonUtilities", "UMG", "Sockets", "Networking"});
	}
}
