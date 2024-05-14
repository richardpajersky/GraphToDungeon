// Copyright Epic Games, Inc. All Rights Reserved.

#include "GraphToDungeonCommands.h"

#define LOCTEXT_NAMESPACE "FPLGPluginModule"

void FGraphToDungeonCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "Graph to Dungeon", "Bring up Graph to Dungeon window", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
