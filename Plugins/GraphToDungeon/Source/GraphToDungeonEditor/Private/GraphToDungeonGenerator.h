// Copyright (c) 2024 Richard Pajersky.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LevelGraphSession.h"
#include "LevelGraphNode.h"
#include "LevelGraphEdge.h"
#include "GraphToDungeonTheme.h"
#include "GraphToDungeonProperties.h"
#include "GraphToDungeonGenerator.generated.h"

/**
 * @brief Delegate invoked when this actor is deleted
 */
DECLARE_DELEGATE(FOnGeneratorDeleted)

/**
 * @brief Structure representing one mesh component with its probability
 */
USTRUCT()
struct FComponentWithProbability
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly)
	UInstancedStaticMeshComponent* Component = nullptr;
	UPROPERTY(EditDefaultsOnly)
	float Probability = 1.0f;
};

/**
 * @brief Structure encapsulating all room components
 */
USTRUCT()
struct FRoomMeshes
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly)
	TArray<FComponentWithProbability> RoomFloorTiles;
	UPROPERTY(EditDefaultsOnly)
	TArray<FComponentWithProbability> RoomWallTiles;
	UPROPERTY(EditDefaultsOnly)
	TArray<FComponentWithProbability> RoomWallCornerTiles;
	UPROPERTY(EditDefaultsOnly)
	TArray<FComponentWithProbability> RoomDoorTiles;
	UPROPERTY(EditDefaultsOnly)
	TArray<FComponentWithProbability> RoomDoorLeftFrameTiles;
	UPROPERTY(EditDefaultsOnly)
	TArray<FComponentWithProbability> RoomDoorRightFrameTiles;
};

/**
 * @brief Structure encapsulating all corridor components
 */
USTRUCT()
struct FCorridorMeshes
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly)
	TArray<FComponentWithProbability> CorridorFloorTiles;
	UPROPERTY(EditDefaultsOnly)
	TArray<FComponentWithProbability> CorridorWallTiles;
	UPROPERTY(EditDefaultsOnly)
	TArray<FComponentWithProbability> CorridorWallOutsideCornerTiles;
	UPROPERTY(EditDefaultsOnly)
	TArray<FComponentWithProbability> CorridorWallInsideCornerTiles;
};

/**
 * @brief Actor performing dungeon generation and mesh instancing and storage inside a scene
 */
UCLASS()
class AGraphToDungeonGenerator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGraphToDungeonGenerator();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	// Possible directions of the segments
	enum EDirection {
		UP,
		DOWN,
		LEFT,
		RIGHT
	};
	/**
	 * @brief Represents temporary corridor object
	 */
	struct UCorridor
	{
		TArray<FIntVector2> Squares;
		TArray<FIntVector2> Points;
		int32 Width = 0;
		UGraphToDungeonTheme* LocalTheme;
		FIntVector2 StartBot;
		FIntVector2 EndBot;
		FIntVector2 StartTop;
		FIntVector2 EndTop;
	};
	struct URoomSegment
	{
		TTuple<FIntVector2, FIntVector2> Door;
		bool bIsUsed = false;
		const EDirection Direction;

		URoomSegment(const EDirection Direction) : Direction(Direction) {}
	};
	/**
	 * @brief Represents temporary room object
	 */
	struct URoom
	{
		int32 Width;
		int32 Height;
		FIntVector2 Origin;
		TSet<TTuple<FIntVector2, FIntVector2>> Doors;
		TArray<bool> IsUsedSegment = {false, false, false, false};
		UGraphToDungeonTheme* LocalTheme;
		TArray<URoomSegment> Segments = {
			URoomSegment(EDirection::DOWN),
			URoomSegment(EDirection::RIGHT),
			URoomSegment(EDirection::UP),
			URoomSegment(EDirection::LEFT)};
		URoomSegment& DownSegment = Segments[0];
		URoomSegment& RightSegment = Segments[1];
		URoomSegment& UpSegment = Segments[2];
		URoomSegment& LeftSegment = Segments[3];
		TSet<URoom*> Connections;
	};
	bool InvalidSeed = false;
	UGraphToDungeonProperties* Properties;


	TArray<URoom*> AllRooms;
	TArray<UCorridor> AllCorridors;
	TSet<FIntVector2> OccupiedTiles;

	const ULevelGraphSession* GraphSession;
public:
	FOnGeneratorDeleted OnGeneratorDeleted;

	UPROPERTY(EditAnywhere)
	FVector tileSize = FVector(100, 100, 0);

	// Room Components
	UPROPERTY(EditDefaultsOnly, Category = "Generator")
	USceneComponent* RoomRoot;

	UPROPERTY(EditAnywhere, Category = "Generator")
	TMap<UGraphToDungeonTheme*, FRoomMeshes> GeneratedRoomThemes;

	UPROPERTY(EditAnywhere, Category = "Generator")
	TMap<UGraphToDungeonTheme*, FCorridorMeshes> GeneratedCorridorThemes;

	UPROPERTY(EditDefaultsOnly)
	int32 GlobalTileRotation = 0;

	UPROPERTY(EditDefaultsOnly, Category = "Generator")
	USceneComponent* GeneratorRootComponent;
	// Corridor Components
	UPROPERTY(EditDefaultsOnly, Category = "Generator")
	USceneComponent* CorridorRoot;
private:
	virtual void Destroyed() override;

	// Spawns meshes into the world
	void SpawnRooms();

	/**
	 * @brief Creates new room and connects it to its parent room
	 * @param ParentRoom Parent room
	 * @param ChildRoomNode Child room to be created
	 * @param Edge Corresponding graph edge between parent and child rooms
	 * @return Newly created room
	 */
	URoom* ConnectWithNew(URoom* ParentRoom, const ULevelGraphNode* ChildRoomNode, ULevelGraphEdge* Edge);
	/**
	 * @brief Connects two already existing rooms with corridor
	 * @param ParentRoom Parent room to be connected from
	 * @param ChildRoomNode Child room to be connected to
	 * @param Edge Corresponding graph edge between parent and child rooms
	 * @return Newly created room
	 */
	bool ConnectWithExisting(URoom* ParentRoom, URoom* ChildRoom, ULevelGraphEdge* Edge);

	int32 SegmentLength(const TTuple<FIntVector2, FIntVector2> Segment);
	bool IsOccupied(const FIntVector2 Coords, const int32 Width, const int32 Height);
	void InsertOccupiedTiles(URoom* Room);

	/**
	 * @brief A* path finding algorithm with Manhattan distance metrics and ordinary array sorting for priority
	 * @param Source position
	 * @param Finish position
	 * @param SourceRoom object
	 * @param FinishRoom object
	 * @param Edge graph edge
	 * @param bFindPathOnly flag
	 * @return Lenght of the path found
	 */
	int32 FindAWay(
		const std::pair<std::pair<int, int>, std::pair<int, int>> Source,
		const std::pair<std::pair<int, int>, std::pair<int, int>> Finish,
		const URoom* SourceRoom, const URoom* FinishRoom, ULevelGraphEdge* Edge,
		const bool bFindPathOnly = false);

	// Mesh spawning helper functions
	void GenerateMesh(TArray<FComponentWithProbability>& InputMeshArray, const TMap<FString, TArray<FMeshWithProbability>*>& MeshCategories, const FString& Name);
	void GenerateRoomThemeMeshes(UGraphToDungeonTheme* LevelTheme);
	void GenerateCorridorThemeMeshes(UGraphToDungeonTheme* LevelTheme);
	int32 GetRandomThemeIndex(const TArray<FComponentWithProbability>& ComponentArray);
	void MeshCleanup();
public:
	/**
	 * @brief Generates dungeon based on properties provided
	 * @param LevelProperties Properties to be used
	 * @return True - successfull generation, False - otherwise
	 */
	bool Generate(UGraphToDungeonProperties* LevelProperties);

	/**
	 * @brief Regenerates all stored and instanced mesh components
	 */
	void RegenerateTheme();
};
