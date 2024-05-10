// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "GraphToDungeonStyle.h"

class FGraphToDungeonCommands : public TCommands<FGraphToDungeonCommands>
{
public:

	FGraphToDungeonCommands()
		: TCommands<FGraphToDungeonCommands>(TEXT("GraphToDungeon"), NSLOCTEXT("Contexts", "GraphToDungeon", "GraphToDungeon Plugin"), NAME_None, FGraphToDungeonStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};