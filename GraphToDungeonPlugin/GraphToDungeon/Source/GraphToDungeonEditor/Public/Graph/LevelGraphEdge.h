// Copyright (c) 2024 Richard Pajersky.

#pragma once

#include "CoreMinimal.h"
#include "GenericGraphEdge.h"
#include "GraphToDungeonTheme.h"
#include "LevelGraphEdge.generated.h"

/**
 * @brief Represents graph edge as corridor, provides corridor properties
 */
UCLASS(Blueprintable)
class ULevelGraphEdge : public UGenericGraphEdge
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Level Graph Edge")
	FText Selection;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Level Graph Edge", meta=(ClampMin = "3"))
	int32 Width = 3;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Level Graph Edge")
	UGraphToDungeonTheme* CorridorTheme;
};
