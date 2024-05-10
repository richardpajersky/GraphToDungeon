// Copyright (c) 2024 Richard Pajersky.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GraphToDungeonTheme.generated.h"

/**
 * @brief Structure representing one entry in theme properties layout, exposing theme settings
 */
USTRUCT()
struct FMeshWithProbability
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly)
	UStaticMesh* Mesh = nullptr;
	UPROPERTY(EditDefaultsOnly, meta = (DisplayName = "Raw Probability", ClampMin = "0.0", ClampMax = "1.0", Delta = 0.01f))
	float Probability = 1.0f;
	UPROPERTY(VisibleAnywhere, meta = (DisplayName = "Final Probability", Units = "%", NoResetToDefault))
	float RealProbability = 100.0f;

	float ProbabilitySum = 1.0f;
};

/**
 * @brief Data asset for defining theme properties
 */
UCLASS()
class UGraphToDungeonTheme : public UDataAsset
{
	GENERATED_BODY()

public:

public:
	UGraphToDungeonTheme();
	~UGraphToDungeonTheme();
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	bool AreMeshesDefined();

public:

	TMap<FString, TArray<FMeshWithProbability>*> MeshCategories;

	UPROPERTY(EditDefaultsOnly, meta = (TitleProperty = "Mesh", FullyExpand))
	TArray<FMeshWithProbability> Walls;
	UPROPERTY(EditDefaultsOnly, meta = (TitleProperty = "Mesh", FullyExpand))
	TArray<FMeshWithProbability> OutsideWallCorners;
	UPROPERTY(EditDefaultsOnly, meta = (TitleProperty = "Mesh", FullyExpand))
	TArray<FMeshWithProbability> InsideWallCorners;
	UPROPERTY(EditDefaultsOnly, meta = (TitleProperty = "Mesh", FullyExpand))
	TArray<FMeshWithProbability> Floors;
	UPROPERTY(EditDefaultsOnly, meta = (TitleProperty = "Mesh", FullyExpand))
	TArray<FMeshWithProbability> Doors;
	UPROPERTY(EditDefaultsOnly, meta = (TitleProperty = "Mesh", FullyExpand))
	TArray<FMeshWithProbability> DoorFrameLeft;
	UPROPERTY(EditDefaultsOnly, meta = (TitleProperty = "Mesh", FullyExpand))
	TArray<FMeshWithProbability> DoorFrameRight;

private:

	/**
	 * @brief Computes probability based on all meshes
	 */
	void ComputeProbability(TArray<FMeshWithProbability>& Array);
};
