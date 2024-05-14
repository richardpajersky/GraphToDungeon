// Copyright (c) 2024 Richard Pajersky.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GraphToDungeonTheme.h"
#include "LevelGraphSession.h"
#include "GraphToDungeonProperties.generated.h"

DECLARE_DELEGATE(FOnPropertiesEdited)

/**
 * @brief Represents overall properties used for the generation
 */
UCLASS()
class GRAPHTODUNGEONEDITOR_API UGraphToDungeonProperties : public UDataAsset
{
	GENERATED_BODY()

public:
	FOnPropertiesEdited OnPropertiesEdited;

public:
	/**
	 * @brief Manages events after changes in editor are made
	 * @param PropertyChangedEvent Property that has been changed
	 */
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	
public:
	FRandomStream RandomStream;

	UPROPERTY(EditAnywhere, Category = "Graph")
	ULevelGraphSession* LevelGraph;

	UPROPERTY(EditAnywhere, Category = "Theme")
	int32 ThemeSeed = 0;

	UPROPERTY(EditAnywhere, Category = "Theme")
	bool bUseRandomThemeSeed = true;

	UPROPERTY(EditAnywhere, Category = "Theme")
	UGraphToDungeonTheme* GlobalLevelTheme;

	UPROPERTY(EditAnywhere, Category = "Settings")
	FVector TileSize = FVector(100.f, 100.f, 0);

	UPROPERTY(EditAnywhere, Category = "Settings")
	int32 RotateTiles = 0;

	UPROPERTY(EditAnywhere, Category = "Settings", meta = (ClampMin = "0"))
	int32 MaxCorridorLength = 1000;

	UPROPERTY(EditAnywhere, Category = "Settings", meta = (ClampMin = "1"))
	int32 MaxGenerationRetries = 100;
};
