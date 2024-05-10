// Copyright (c) 2024 Richard Pajersky.

#pragma once

#include "CoreMinimal.h"
#include "GenericGraph.h"
#include "LevelGraphSession.generated.h"

/**
 * @brief Main graph class, inherits from UGenericGraph
 */
UCLASS(Blueprintable)
class GRAPHTODUNGEONEDITOR_API ULevelGraphSession : public UGenericGraph
{
	GENERATED_BODY()
public:
	ULevelGraphSession();

	UPROPERTY()
	FLinearColor LeftDialoguerBgColor;

	UPROPERTY()
	FLinearColor RightDialoguerBgColor;

};
