// Copyright (c) 2024 Richard Pajersky.


#include "GraphToDungeonGenerator.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/InstancedStaticMesh.h"
#include <Map>
#include <Array>
#include "Containers/Queue.h"

// Sets default values
AGraphToDungeonGenerator::AGraphToDungeonGenerator()
{
	tileSize = FVector(100, 100, 0);

	GeneratorRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Generator Root"));
	SetRootComponent(GeneratorRootComponent);
	RootComponent->SetMobility(EComponentMobility::Movable);

	auto SetupAttachmentToRoot = [&](USceneComponent* Component) -> void
		{
			Component->SetMobility(EComponentMobility::Movable);
			Component->SetRelativeLocation(FVector());
			Component->SetupAttachment(RootComponent);
			Component->RegisterComponent();
		};

 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void AGraphToDungeonGenerator::BeginPlay()
{
	UE_LOG(LogTemp, Warning, TEXT("BeginPlay"));
	Super::BeginPlay();
}

// Called every frame
void AGraphToDungeonGenerator::Tick(float DeltaTime)
{
	UE_LOG(LogTemp, Warning, TEXT("Tick"));
	Super::Tick(DeltaTime);

}

void AGraphToDungeonGenerator::Destroyed()
{
	OnGeneratorDeleted.ExecuteIfBound();
}

void AGraphToDungeonGenerator::GenerateMesh(TArray<FComponentWithProbability>& InputMeshArray, const TMap<FString, TArray<FMeshWithProbability>*>& MeshCategories, const FString& Name)
{
	// If no local theme mesh found, global mesh is used in place
	if (MeshCategories[Name]->Num() == 0)
	{
		GenerateMesh(InputMeshArray, Properties->GlobalLevelTheme->MeshCategories, Name);
		return;
	}

	int32 Index = 0;
	for (const auto& Theme : (*MeshCategories[Name]))
	{
		const int32 ArrayIndex = InputMeshArray.AddDefaulted();
		InputMeshArray[ArrayIndex].Probability = Theme.Mesh ? Theme.Probability : (*Properties->GlobalLevelTheme->MeshCategories[Name])[0].Probability;
		InputMeshArray[ArrayIndex].Component = NewObject<UInstancedStaticMeshComponent>(this,
			FName(Name + FString::FromInt(GeneratedCorridorThemes.Num()) + FString::FromInt(GeneratedRoomThemes.Num()) + FString::FromInt(Index)));
		InputMeshArray[ArrayIndex].Component->SetStaticMesh(Theme.Mesh ? Theme.Mesh : (*Properties->GlobalLevelTheme->MeshCategories[Name])[0].Mesh);
		InputMeshArray[ArrayIndex].Component->SetRelativeLocation(FVector());
		InputMeshArray[ArrayIndex].Component->RegisterComponent();
		InputMeshArray[ArrayIndex].Component->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetIncludingScale);
		Index++;
	}
}

void AGraphToDungeonGenerator::GenerateRoomThemeMeshes(UGraphToDungeonTheme* LevelTheme)
{
	GenerateMesh(
		GeneratedRoomThemes[LevelTheme].RoomFloorTiles,
		LevelTheme->MeshCategories,
		FString("Floors")
	);
	GenerateMesh(
		GeneratedRoomThemes[LevelTheme].RoomWallTiles,
		LevelTheme->MeshCategories,
		FString("Walls")
	);
	GenerateMesh(
		GeneratedRoomThemes[LevelTheme].RoomWallCornerTiles,
		LevelTheme->MeshCategories,
		FString("OutsideWallCorners")
	);
	GenerateMesh(
		GeneratedRoomThemes[LevelTheme].RoomDoorTiles,
		LevelTheme->MeshCategories,
		FString("Doors")
	);
	GenerateMesh(
		GeneratedRoomThemes[LevelTheme].RoomDoorLeftFrameTiles,
		LevelTheme->MeshCategories,
		FString("DoorFrameLeft")
	);
	GenerateMesh(
		GeneratedRoomThemes[LevelTheme].RoomDoorRightFrameTiles,
		LevelTheme->MeshCategories,
		FString("DoorFrameRight")
	);
}

void AGraphToDungeonGenerator::GenerateCorridorThemeMeshes(UGraphToDungeonTheme* LevelTheme)
{
	GenerateMesh(
		GeneratedCorridorThemes[LevelTheme].CorridorFloorTiles,
		LevelTheme->MeshCategories,
		FString("Floors")
	);
	GenerateMesh(
		GeneratedCorridorThemes[LevelTheme].CorridorWallTiles,
		LevelTheme->MeshCategories,
		FString("Walls")
	);
	GenerateMesh(
		GeneratedCorridorThemes[LevelTheme].CorridorWallOutsideCornerTiles,
		LevelTheme->MeshCategories,
		FString("OutsideWallCorners")
	);
	GenerateMesh(
		GeneratedCorridorThemes[LevelTheme].CorridorWallInsideCornerTiles,
		LevelTheme->MeshCategories,
		FString("InsideWallCorners")
	);
}

void AGraphToDungeonGenerator::MeshCleanup()
{
	GeneratedCorridorThemes.Empty();
	GeneratedRoomThemes.Empty();
}

void AGraphToDungeonGenerator::RegenerateTheme()
{
	MeshCleanup();
	SpawnRooms();
}

int32 AGraphToDungeonGenerator::GetRandomThemeIndex(const TArray<FComponentWithProbability>& ComponentArray)
{
	float ProbabilitySum = 0.0f;
	for (const auto& Component : ComponentArray)
	{
		ProbabilitySum += Component.Probability;
	}
	float CompoundProbability = 0.0f;
	int32 Index = 0;
	float RandomNumber = Properties->RandomStream.FRandRange(0, ProbabilitySum);
	for (const auto& Component : ComponentArray)
	{
		CompoundProbability += Component.Probability;
		if (RandomNumber < CompoundProbability) return Index;
		Index++;
	}
	return Index - 1;
}

void AGraphToDungeonGenerator::SpawnRooms()
{
	MeshCleanup();
	UGraphToDungeonTheme* LevelTheme = Properties->GlobalLevelTheme;
	if (LevelTheme)
	{
		GeneratedRoomThemes.Add(LevelTheme);
		GeneratedCorridorThemes.Add(LevelTheme);
		GenerateRoomThemeMeshes(LevelTheme);
		GenerateCorridorThemeMeshes(LevelTheme);
	}
	for (int32 i = 0; i < AllRooms.Num(); i++)
	{
		LevelTheme = Properties->GlobalLevelTheme;
		if (AllRooms[i]->LocalTheme)
		{
			//UE_LOG(LogTemp, Warning, TEXT("Local theme"));
			LevelTheme = AllRooms[i]->LocalTheme;
			if (!GeneratedRoomThemes.Contains(LevelTheme))
			{
				GeneratedRoomThemes.Add(LevelTheme);
				GenerateRoomThemeMeshes(LevelTheme);
			}
		}
		int32 TileIndex(0);
		
		TSet<FIntVector2> DoorPositions;
		// Instanced static mesh for room door
		for (const auto& Door : AllRooms[i]->Doors)
		{
			if (Door.Key.X == Door.Value.X)
			{
				// Left
				if (Door.Key.X == AllRooms[i]->Origin.X)
				{
					const FRotator TileRotation(0, 0 + GlobalTileRotation, 0);
					{
						const FIntVector2 DoorPosition(Door.Key.X, Door.Value.Y);
						DoorPositions.Add(DoorPosition);
						GeneratedRoomThemes[LevelTheme].RoomDoorLeftFrameTiles[GetRandomThemeIndex(GeneratedRoomThemes[LevelTheme].RoomDoorLeftFrameTiles)].Component->AddInstance(FTransform(TileRotation, FVector(DoorPosition.X, DoorPosition.Y, 0) * tileSize));
					}
					{
						const FIntVector2 DoorPosition(Door.Key.X, Door.Key.Y);
						DoorPositions.Add(DoorPosition);
						GeneratedRoomThemes[LevelTheme].RoomDoorRightFrameTiles[GetRandomThemeIndex(GeneratedRoomThemes[LevelTheme].RoomDoorRightFrameTiles)].Component->AddInstance(FTransform(TileRotation, FVector(DoorPosition.X, DoorPosition.Y, 0) * tileSize));
					}
					for (int32 HeightIndex = 1; Door.Key.Y + HeightIndex < Door.Value.Y; HeightIndex++)
					{
						const FIntVector2 DoorPosition(Door.Key.X, Door.Key.Y + HeightIndex);
						DoorPositions.Add(DoorPosition);
						GeneratedRoomThemes[LevelTheme].RoomDoorTiles[GetRandomThemeIndex(GeneratedRoomThemes[LevelTheme].RoomDoorTiles)].Component->AddInstance(FTransform(TileRotation, FVector(DoorPosition.X, DoorPosition.Y, 0) * tileSize));
					}
				}
				else
				{
					const FRotator TileRotation(0, 180 + GlobalTileRotation, 0);
					{
						const FIntVector2 DoorPosition(Door.Key.X, Door.Key.Y);
						DoorPositions.Add(DoorPosition);
						GeneratedRoomThemes[LevelTheme].RoomDoorLeftFrameTiles[GetRandomThemeIndex(GeneratedRoomThemes[LevelTheme].RoomDoorLeftFrameTiles)].Component->AddInstance(FTransform(TileRotation, FVector(DoorPosition.X, DoorPosition.Y, 0) * tileSize));
					}
					{
						const FIntVector2 DoorPosition(Door.Key.X, Door.Value.Y);
						DoorPositions.Add(DoorPosition);
						GeneratedRoomThemes[LevelTheme].RoomDoorRightFrameTiles[GetRandomThemeIndex(GeneratedRoomThemes[LevelTheme].RoomDoorRightFrameTiles)].Component->AddInstance(FTransform(TileRotation, FVector(DoorPosition.X, DoorPosition.Y, 0) * tileSize));
					}
					for (int32 HeightIndex = 1; Door.Key.Y + HeightIndex < Door.Value.Y; HeightIndex++)
					{
						const FIntVector2 DoorPosition(Door.Key.X, Door.Key.Y + HeightIndex);
						DoorPositions.Add(DoorPosition);
						GeneratedRoomThemes[LevelTheme].RoomDoorTiles[GetRandomThemeIndex(GeneratedRoomThemes[LevelTheme].RoomDoorTiles)].Component->AddInstance(FTransform(TileRotation, FVector(DoorPosition.X, DoorPosition.Y, 0) * tileSize));
					}
				}
			}
			else if (Door.Key.Y == Door.Value.Y)
			{
				if (Door.Key.Y == AllRooms[i]->Origin.Y)
				{
					const FRotator TileRotation(0, 90 + GlobalTileRotation, 0);
					{
						const FIntVector2 DoorPosition(Door.Key.X, Door.Key.Y);
						DoorPositions.Add(DoorPosition);
						GeneratedRoomThemes[LevelTheme].RoomDoorLeftFrameTiles[GetRandomThemeIndex(GeneratedRoomThemes[LevelTheme].RoomDoorLeftFrameTiles)].Component->AddInstance(FTransform(TileRotation, FVector(DoorPosition.X, DoorPosition.Y, 0) * tileSize));
					}
					{
						const FIntVector2 DoorPosition(Door.Value.X, Door.Key.Y);
						DoorPositions.Add(DoorPosition);
						GeneratedRoomThemes[LevelTheme].RoomDoorRightFrameTiles[GetRandomThemeIndex(GeneratedRoomThemes[LevelTheme].RoomDoorRightFrameTiles)].Component->AddInstance(FTransform(TileRotation, FVector(DoorPosition.X, DoorPosition.Y, 0) * tileSize));
					}
					for (int32 WidthIndex = 1; Door.Key.X + WidthIndex < Door.Value.X; WidthIndex++)
					{
						const FIntVector2 DoorPosition(Door.Key.X + WidthIndex, Door.Key.Y);
						DoorPositions.Add(DoorPosition);
						GeneratedRoomThemes[LevelTheme].RoomDoorTiles[GetRandomThemeIndex(GeneratedRoomThemes[LevelTheme].RoomDoorTiles)].Component->AddInstance(FTransform(TileRotation, FVector(DoorPosition.X, DoorPosition.Y, 0) * tileSize));
					}
				}
				else
				{
					const FRotator TileRotation(0, -90 + GlobalTileRotation, 0);
					{
						const FIntVector2 DoorPosition(Door.Value.X, Door.Key.Y);
						DoorPositions.Add(DoorPosition);
						GeneratedRoomThemes[LevelTheme].RoomDoorLeftFrameTiles[GetRandomThemeIndex(GeneratedRoomThemes[LevelTheme].RoomDoorLeftFrameTiles)].Component->AddInstance(FTransform(TileRotation, FVector(DoorPosition.X, DoorPosition.Y, 0) * tileSize));
					}
					{
						const FIntVector2 DoorPosition(Door.Key.X, Door.Key.Y);
						DoorPositions.Add(DoorPosition);
						GeneratedRoomThemes[LevelTheme].RoomDoorRightFrameTiles[GetRandomThemeIndex(GeneratedRoomThemes[LevelTheme].RoomDoorRightFrameTiles)].Component->AddInstance(FTransform(TileRotation, FVector(DoorPosition.X, DoorPosition.Y, 0) * tileSize));
					}
					for (int32 WidthIndex = 1; Door.Key.X + WidthIndex < Door.Value.X; WidthIndex++)
					{
						const FIntVector2 DoorPosition(Door.Key.X + WidthIndex, Door.Key.Y);
						DoorPositions.Add(DoorPosition);
						GeneratedRoomThemes[LevelTheme].RoomDoorTiles[GetRandomThemeIndex(GeneratedRoomThemes[LevelTheme].RoomDoorTiles)].Component->AddInstance(FTransform(TileRotation, FVector(DoorPosition.X, DoorPosition.Y, 0) * tileSize));
					}
				}
			}
		}
		const FVector pos(AllRooms[i]->Origin.X, AllRooms[i]->Origin.Y, 0);
		const FVector size(AllRooms[i]->Width, AllRooms[i]->Height, 0);
		// Add room corners
		const FIntVector2 WallCornerPositionBotLeft(pos.X, pos.Y);
		const FIntVector2 WallCornerPositionBotRight(pos.X + AllRooms[i]->Width - 1, pos.Y);
		const FIntVector2 WallCornerPositionTopLeft(pos.X, pos.Y + AllRooms[i]->Height - 1);
		const FIntVector2 WallCornerPositionTopRight(pos.X + AllRooms[i]->Width - 1, pos.Y + AllRooms[i]->Height - 1);
		if (!DoorPositions.Contains(WallCornerPositionBotLeft))
			GeneratedRoomThemes[LevelTheme].RoomWallCornerTiles[GetRandomThemeIndex(GeneratedRoomThemes[LevelTheme].RoomWallCornerTiles)].Component->AddInstance(
				FTransform(FRotator(0, 0 + GlobalTileRotation, 0), FVector(WallCornerPositionBotLeft.X, WallCornerPositionBotLeft.Y, 0) * tileSize));
		if (!DoorPositions.Contains(WallCornerPositionBotRight))
			GeneratedRoomThemes[LevelTheme].RoomWallCornerTiles[GetRandomThemeIndex(GeneratedRoomThemes[LevelTheme].RoomWallCornerTiles)].Component->AddInstance(
				FTransform(FRotator(0, 90 + GlobalTileRotation, 0), FVector(WallCornerPositionBotRight.X, WallCornerPositionBotRight.Y, 0) * tileSize));
		if (!DoorPositions.Contains(WallCornerPositionTopLeft))
			GeneratedRoomThemes[LevelTheme].RoomWallCornerTiles[GetRandomThemeIndex(GeneratedRoomThemes[LevelTheme].RoomWallCornerTiles)].Component->AddInstance(
				FTransform(FRotator(0, -90 + GlobalTileRotation, 0), FVector(WallCornerPositionTopLeft.X, WallCornerPositionTopLeft.Y, 0) * tileSize));
		if (!DoorPositions.Contains(WallCornerPositionTopRight))
			GeneratedRoomThemes[LevelTheme].RoomWallCornerTiles[GetRandomThemeIndex(GeneratedRoomThemes[LevelTheme].RoomWallCornerTiles)].Component->AddInstance(
				FTransform(FRotator(0, 180 + GlobalTileRotation, 0), FVector(WallCornerPositionTopRight.X, WallCornerPositionTopRight.Y, 0) * tileSize));
		for (unsigned ii = 0; ii < FMath::TruncToInt(size.X); ii++) {
			if (ii > 0 && ii < FMath::TruncToInt(size.X) - 1)
			{
				const FIntVector2 WallPositionBot(pos.X + ii, pos.Y);
				const FIntVector2 WallPositionTop(pos.X + ii, pos.Y + AllRooms[i]->Height - 1);
				if (!DoorPositions.Contains(WallPositionBot))
					GeneratedRoomThemes[LevelTheme].RoomWallTiles[GetRandomThemeIndex(GeneratedRoomThemes[LevelTheme].RoomWallTiles)].Component->AddInstance(
						FTransform(FRotator(0, 90 + GlobalTileRotation, 0), FVector(WallPositionBot.X, WallPositionBot.Y, 0) * tileSize));
				if (!DoorPositions.Contains(WallPositionTop))
					GeneratedRoomThemes[LevelTheme].RoomWallTiles[GetRandomThemeIndex(GeneratedRoomThemes[LevelTheme].RoomWallTiles)].Component->AddInstance(
						FTransform(FRotator(0, -90 + GlobalTileRotation, 0), FVector(WallPositionTop.X, WallPositionTop.Y, 0) * tileSize));
			}
			for (unsigned jj = 0; jj < FMath::TruncToInt(size.Y); jj++) {
				if (jj > 0 && jj < FMath::TruncToInt(size.Y) - 1 && ii == 0)
				{
					const FIntVector2 WallPositionLeft(pos.X, pos.Y + jj);
					const FIntVector2 WallPositionRight(pos.X + AllRooms[i]->Width - 1, pos.Y + jj);
					if (!DoorPositions.Contains(WallPositionLeft))
						GeneratedRoomThemes[LevelTheme].RoomWallTiles[GetRandomThemeIndex(GeneratedRoomThemes[LevelTheme].RoomWallTiles)].Component->AddInstance(
							FTransform(FVector(WallPositionLeft.X, WallPositionLeft.Y, 0) * tileSize));
					if (!DoorPositions.Contains(WallPositionRight))
						GeneratedRoomThemes[LevelTheme].RoomWallTiles[GetRandomThemeIndex(GeneratedRoomThemes[LevelTheme].RoomWallTiles)].Component->AddInstance(
							FTransform(FRotator(0, 180 + GlobalTileRotation, 0), FVector(WallPositionRight.X, WallPositionRight.Y, 0) * tileSize));
				}
				GeneratedRoomThemes[LevelTheme].RoomFloorTiles[GetRandomThemeIndex(GeneratedRoomThemes[LevelTheme].RoomFloorTiles)].Component->AddInstance(
					FTransform(FVector(ii * tileSize.X, jj * tileSize.Y, 0.f) + pos * tileSize));
			}
		}
	}
	for (int32 i = 0; i < AllCorridors.Num(); i++)
	{
		const auto& Corridor = AllCorridors[i];
		LevelTheme = Properties->GlobalLevelTheme;
		if (Corridor.LocalTheme)
		{
			//UE_LOG(LogTemp, Warning, TEXT("Local theme"));
			LevelTheme = Corridor.LocalTheme;
			if (!GeneratedCorridorThemes.Contains(LevelTheme))
			{
				GeneratedCorridorThemes.Add(LevelTheme);
				GenerateCorridorThemeMeshes(LevelTheme);
			}
		}
		
		TSet<FIntVector2> Path;
		for (const auto& Square : Corridor.Squares)
		{
			// Generate floor using squares
			for (int32 j = 0; j < Corridor.Width; j++)
			{
				for (int32 k = 0; k < Corridor.Width; k++)
				{
					FIntVector2 PointOnPath(j + Square.X, k + Square.Y);
					Path.Add(PointOnPath);
					GeneratedCorridorThemes[LevelTheme].CorridorFloorTiles[GetRandomThemeIndex(GeneratedCorridorThemes[LevelTheme].CorridorFloorTiles)].Component->AddInstance(FTransform(FVector(PointOnPath.X, PointOnPath.Y, 0) * tileSize));
				}
			}
			// Generate floor using specific points
			for (const auto& Point : Corridor.Points)
			{
				FIntVector2 PointOnPath(Point.X, Point.Y);
				Path.Add(PointOnPath);
				GeneratedCorridorThemes[LevelTheme].CorridorFloorTiles[GetRandomThemeIndex(GeneratedCorridorThemes[LevelTheme].CorridorFloorTiles)].Component->AddInstance(FTransform(FVector(PointOnPath.X, PointOnPath.Y, 0) * tileSize));
			}
		}
		TMap<FIntVector2, EDirection> Border;
		TMap<FIntVector2, TSet<EDirection>> OutsideBorder;
		for (const auto& Point : Path)
		{
			if (Point != Corridor.StartBot && Point != Corridor.StartTop &&
				Point != Corridor.EndBot && Point != Corridor.EndTop)
			{
				if (Corridor.StartBot.X == Corridor.StartTop.X && Point.X == Corridor.StartBot.X &&
					Point.Y > Corridor.StartBot.Y && Point.Y < Corridor.StartTop.Y) continue;
				if (Corridor.StartBot.Y == Corridor.StartTop.Y && Point.Y == Corridor.StartBot.Y &&
					Point.X > Corridor.StartBot.X && Point.X < Corridor.StartTop.X) continue;
				if (Corridor.EndBot.X == Corridor.EndTop.X && Point.X == Corridor.EndBot.X &&
					Point.Y > Corridor.EndBot.Y && Point.Y < Corridor.EndTop.Y) continue;
				if (Corridor.EndBot.Y == Corridor.EndTop.Y && Point.Y == Corridor.EndBot.Y &&
					Point.X > Corridor.EndBot.X && Point.X < Corridor.EndTop.X) continue;
			}
			int32 PathNeighbourCount = 0;
			TSet<EDirection> TileDirections;
			EDirection TileDirection = EDirection::UP;
			if (Path.Contains(Point + FIntVector2(-1, 0))) PathNeighbourCount++;
			else TileDirections.Add(EDirection::LEFT);
			if (Path.Contains(Point + FIntVector2(+1, 0))) PathNeighbourCount++;
			else TileDirections.Add(EDirection::RIGHT);
			if (Path.Contains(Point + FIntVector2(0, -1))) PathNeighbourCount++;
			else TileDirections.Add(EDirection::DOWN);
			if (Path.Contains(Point + FIntVector2(0, +1))) PathNeighbourCount++;
			else TileDirections.Add(EDirection::UP);
			if (TileDirections.Num() == 1) Border.Add(Point, TileDirections.Array()[0]);
			if (TileDirections.Num() == 2) OutsideBorder.Add(Point, TileDirections);
		}
		TMap<FIntVector2, TSet<EDirection>> InsideBorder;
		TSet<FIntVector2> BorderSet;
		Border.GetKeys(BorderSet);
		TSet<FIntVector2> OutsideBorderSet;
		OutsideBorder.GetKeys(OutsideBorderSet);
		for (const auto& Point : Path.Difference(BorderSet.Union(OutsideBorderSet)))
		{
			TSet<EDirection> TileDirections;
			if (BorderSet.Union(OutsideBorderSet).Contains(Point + FIntVector2(-1, 0))) TileDirections.Add(EDirection::LEFT);
			if (BorderSet.Union(OutsideBorderSet).Contains(Point + FIntVector2(+1, 0))) TileDirections.Add(EDirection::RIGHT);
			if (BorderSet.Union(OutsideBorderSet).Contains(Point + FIntVector2(0, -1))) TileDirections.Add(EDirection::DOWN);
			if (BorderSet.Union(OutsideBorderSet).Contains(Point + FIntVector2(0, +1))) TileDirections.Add(EDirection::UP);
			if (TileDirections.Num() == 2 &&
				(TileDirections.Contains(EDirection::LEFT) && TileDirections.Contains(EDirection::UP) ||
				TileDirections.Contains(EDirection::RIGHT) && TileDirections.Contains(EDirection::UP) || 
				TileDirections.Contains(EDirection::LEFT) && TileDirections.Contains(EDirection::DOWN) || 
				TileDirections.Contains(EDirection::RIGHT) && TileDirections.Contains(EDirection::DOWN)))
			{
				if (!Path.Contains(Point + FIntVector2(+1, +1)) ||
					!Path.Contains(Point + FIntVector2(+1, -1)) ||
					!Path.Contains(Point + FIntVector2(-1, +1)) ||
					!Path.Contains(Point + FIntVector2(-1, -1))) InsideBorder.Add(Point, TileDirections);
			}
		}
		for (const auto& Point : Border)
		{
			FRotator WallRotation;
			switch (Point.Value)
			{
			case EDirection::UP:
				WallRotation = FRotator(0, -90 + GlobalTileRotation,0);
				break;
			case EDirection::DOWN:
				WallRotation = FRotator(0, 90 + GlobalTileRotation, 0);
				break;
			case EDirection::LEFT:
				WallRotation = FRotator(0, 0 + GlobalTileRotation, 0);
				break;
			case EDirection::RIGHT:
				WallRotation = FRotator(0, 180 + GlobalTileRotation, 0);
				break;
			}
			GeneratedCorridorThemes[LevelTheme].CorridorWallTiles[GetRandomThemeIndex(GeneratedCorridorThemes[LevelTheme].CorridorWallTiles)].Component->AddInstance(
				FTransform(WallRotation, FVector(Point.Key.X, Point.Key.Y, 0) * tileSize));
		}
		for (const auto& Point : OutsideBorder)
		{
			FRotator WallRotation;
			if (Point.Value.Contains(EDirection::UP) && Point.Value.Contains(EDirection::LEFT)) WallRotation = FRotator(0, -90 + GlobalTileRotation, 0);
			if (Point.Value.Contains(EDirection::UP) && Point.Value.Contains(EDirection::RIGHT)) WallRotation = FRotator(0, 180 + GlobalTileRotation, 0);
			if (Point.Value.Contains(EDirection::DOWN) && Point.Value.Contains(EDirection::LEFT)) WallRotation = FRotator(0, 0 + GlobalTileRotation, 0);
			if (Point.Value.Contains(EDirection::DOWN) && Point.Value.Contains(EDirection::RIGHT)) WallRotation = FRotator(0, 90 + GlobalTileRotation, 0);
			GeneratedCorridorThemes[LevelTheme].CorridorWallOutsideCornerTiles[GetRandomThemeIndex(GeneratedCorridorThemes[LevelTheme].CorridorWallOutsideCornerTiles)].Component->AddInstance(
				FTransform(WallRotation, FVector(Point.Key.X, Point.Key.Y, 0) * tileSize));
		}
		for (const auto& Point : InsideBorder)
		{
			FRotator WallRotation;
			if (Point.Value.Contains(EDirection::UP) && Point.Value.Contains(EDirection::LEFT)) WallRotation = FRotator(0, -90 + GlobalTileRotation, 0);
			if (Point.Value.Contains(EDirection::UP) && Point.Value.Contains(EDirection::RIGHT)) WallRotation = FRotator(0, 180 + GlobalTileRotation, 0);
			if (Point.Value.Contains(EDirection::DOWN) && Point.Value.Contains(EDirection::LEFT)) WallRotation = FRotator(0, 0 + GlobalTileRotation, 0);
			if (Point.Value.Contains(EDirection::DOWN) && Point.Value.Contains(EDirection::RIGHT)) WallRotation = FRotator(0, 90 + GlobalTileRotation, 0);
			GeneratedCorridorThemes[LevelTheme].CorridorWallInsideCornerTiles[GetRandomThemeIndex(GeneratedCorridorThemes[LevelTheme].CorridorWallInsideCornerTiles)].Component->AddInstance(
				FTransform(WallRotation, FVector(Point.Key.X, Point.Key.Y, 0) * tileSize));
		}
	}

}

void AGraphToDungeonGenerator::InsertOccupiedTiles(URoom* Room)
{
	for (int32 i = 0; i < Room->Width; i++)
	{
		OccupiedTiles.Add(FIntVector2(Room->Origin.X + i, Room->Origin.Y));
		OccupiedTiles.Add(FIntVector2(Room->Origin.X + i, Room->Origin.Y + Room->Height - 1));
	}
	for (int32 i = 0; i < Room->Height; i++)
	{
		OccupiedTiles.Add(FIntVector2(Room->Origin.X, Room->Origin.Y + i));
		OccupiedTiles.Add(FIntVector2(Room->Origin.X + Room->Width - 1, Room->Origin.Y + i));
	}
};

bool AGraphToDungeonGenerator::IsOccupied(const FIntVector2 Coords, const int32 Width, const int32 Height)
{
	for (int32 i = 0; i < Width; i++)
	{
		for (int32 j = 0; j < Height; j++)
		{
			if (OccupiedTiles.Contains(Coords + FIntVector2(i, j))) return true;
		}
	}
	return false;
}

int32 AGraphToDungeonGenerator::SegmentLength(const TTuple<FIntVector2, FIntVector2> Segment)
{
	return FMath::Abs(
		Segment.Key.X - Segment.Value.X +
		Segment.Key.Y - Segment.Value.Y);
};

AGraphToDungeonGenerator::URoom* AGraphToDungeonGenerator::ConnectWithNew(URoom* ParentRoom, const ULevelGraphNode* ChildRoomNode, ULevelGraphEdge* Edge)
{
	const int32 EdgeWidth = Edge->Width;
	// Create new room object
	AllRooms.Add(new URoom);
	URoom* NewRoom = AllRooms.Last();
	NewRoom->Width = ChildRoomNode->Width;
	NewRoom->Height = ChildRoomNode->Height;
	NewRoom->LocalTheme = ChildRoomNode->RoomTheme;

	bool bIsCorridorPossible(false);
	int32 GenerationRetries(0);
	// X coordinate of new rooms origin
	int32 RoomXCoord(0);
	// Y coordinate of new rooms origin
	int32 RoomYCoord(0);
	// Number of generation retries
	int32 Retries(0);
	// Minimum distance the new room origin can be from the parent room
	int32 MinDistanceFromParent(0);
	// Maximum distance the new room origin can be from the parent room
	int32 MaxDistanceFromParent(0);
	TTuple<FIntVector2, FIntVector2> ParentRoomDoorSegment(FIntVector2(0, 0), FIntVector2(0, 0));
	TTuple<FIntVector2, FIntVector2> NewRoomDoorSegment(FIntVector2(0, 0), FIntVector2(0, 0));
	int32 ParentRoomDoorSegmentLength(0);
	int32 NewRoomDoorSegmentLength(0);
	int32 ParentRoomDoorStartCoord(0);
	int32 NewRoomDoorStartCoord(0);
	TTuple<FIntVector2, FIntVector2> ParentRoomDoor(FIntVector2(0, 0), FIntVector2(0, 0));
	TTuple<FIntVector2, FIntVector2> NewRoomDoor(FIntVector2(0, 0), FIntVector2(0, 0));
	while (!bIsCorridorPossible && GenerationRetries < 10)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Whole retry %d"), GenerationRetries);
		GenerationRetries++;
		auto ShuffleIndices = [&](TArray<int32>& Array) -> void
			{
				const int32 ArraySize = Array.Num();
				for (int32 i = 0; i < ArraySize; ++i)
				{
					const int32 RandomIndex = Properties->RandomStream.RandRange(i, ArraySize - 1);
					Array.Swap(i, RandomIndex);
				}
			};
		// Create random order of segments
		TArray<int32> SegmentIndices{ 0, 1, 2, 3 };
		ShuffleIndices(SegmentIndices);
		int32 SegmentIndexCounter(0);
		// Find next non used segment
		while (ParentRoom->Segments[SegmentIndices[SegmentIndexCounter]].bIsUsed &&
			SegmentIndexCounter < 3) SegmentIndexCounter++;
		// Parent room segment to use
		URoomSegment& ParentRoomSegment = ParentRoom->Segments[SegmentIndices[SegmentIndexCounter]];
		// Create new room based on segment type
		switch (ParentRoomSegment.Direction)
		{
		case EDirection::DOWN:
			// Generate room coords
			MinDistanceFromParent = NewRoom->Height + EdgeWidth + 1;
			MaxDistanceFromParent = MinDistanceFromParent;
			do
			{
				Properties->RandomStream.GenerateNewSeed();
				RoomYCoord = Properties->RandomStream.RandRange(
					ParentRoom->Origin.Y - MinDistanceFromParent,
					ParentRoom->Origin.Y - MaxDistanceFromParent);
				int32 DistanceFromParent(FMath::Abs(ParentRoom->Origin.Y - RoomYCoord));
				RoomXCoord = Properties->RandomStream.RandRange(
					ParentRoom->Origin.X - DistanceFromParent,
					ParentRoom->Origin.X + ParentRoom->Width - 2 + DistanceFromParent);
				Retries++;
				MaxDistanceFromParent++;
			} while (IsOccupied(FIntVector2(RoomXCoord, RoomYCoord), NewRoom->Width, NewRoom->Height) && Retries < 1000);
			NewRoom->Origin = FIntVector2(RoomXCoord, RoomYCoord);
			ParentRoomDoorSegment = TTuple<FIntVector2, FIntVector2>
				(ParentRoom->Origin + FIntVector2(1, 0), ParentRoom->Origin + FIntVector2(ParentRoom->Width - 2, 0));
			NewRoomDoorSegment = TTuple<FIntVector2, FIntVector2>
				(NewRoom->Origin + FIntVector2(1, NewRoom->Height - 1), NewRoom->Origin + FIntVector2(NewRoom->Width - 2, NewRoom->Height - 1));
			ParentRoomDoorSegmentLength = SegmentLength(ParentRoomDoorSegment);
			NewRoomDoorSegmentLength = SegmentLength(NewRoomDoorSegment);
			ParentRoomDoorStartCoord = Properties->RandomStream.RandRange(0, ParentRoomDoorSegmentLength - EdgeWidth);
			NewRoomDoorStartCoord = Properties->RandomStream.RandRange(0, NewRoomDoorSegmentLength - EdgeWidth);
			ParentRoomDoor = TTuple<FIntVector2, FIntVector2>(
				FIntVector2(ParentRoomDoorSegment.Key.X + ParentRoomDoorStartCoord, ParentRoomDoorSegment.Key.Y),
				FIntVector2(ParentRoomDoorSegment.Key.X + ParentRoomDoorStartCoord + EdgeWidth - 1, ParentRoomDoorSegment.Key.Y));
			NewRoomDoor = TTuple<FIntVector2, FIntVector2>(
				FIntVector2(NewRoomDoorSegment.Key.X + NewRoomDoorStartCoord, NewRoomDoorSegment.Key.Y),
				FIntVector2(NewRoomDoorSegment.Key.X + NewRoomDoorStartCoord + EdgeWidth - 1, NewRoomDoorSegment.Key.Y));
			bIsCorridorPossible = FindAWay(
				std::make_pair(
					std::make_pair(ParentRoomDoor.Key.X, ParentRoomDoor.Key.Y),
					std::make_pair(ParentRoomDoor.Value.X, ParentRoomDoor.Value.Y)),
				std::make_pair(
					std::make_pair(NewRoomDoor.Key.X, NewRoomDoor.Key.Y),
					std::make_pair(NewRoomDoor.Value.X, NewRoomDoor.Value.Y)),
				ParentRoom, NewRoom, Edge, true) > 0;
			if (!bIsCorridorPossible) continue;
			NewRoom->UpSegment.bIsUsed = true;
			ParentRoomSegment.Door = ParentRoomDoor;
			NewRoom->UpSegment.Door = NewRoomDoor;
			break;
		case EDirection::RIGHT:
			MinDistanceFromParent = EdgeWidth + 1;
			MaxDistanceFromParent = MinDistanceFromParent;
			do
			{
				Properties->RandomStream.GenerateNewSeed();
				RoomXCoord = Properties->RandomStream.RandRange(
					ParentRoom->Origin.X + ParentRoom->Width - 1 + MinDistanceFromParent,
					ParentRoom->Origin.X + ParentRoom->Width - 1 + MaxDistanceFromParent);
				int32 DistanceFromParent(FMath::Abs(ParentRoom->Origin.X + ParentRoom->Width - 1 - RoomXCoord));
				RoomYCoord = Properties->RandomStream.RandRange(
					ParentRoom->Origin.Y - DistanceFromParent,
					ParentRoom->Origin.Y + ParentRoom->Height - 2 + DistanceFromParent);
				Retries++;
				MaxDistanceFromParent++;
			} while (IsOccupied(FIntVector2(RoomXCoord, RoomYCoord), NewRoom->Width, NewRoom->Height) && Retries < 1000);
			NewRoom->Origin = FIntVector2(RoomXCoord, RoomYCoord);
			ParentRoomDoorSegment = TTuple<FIntVector2, FIntVector2>
				(ParentRoom->Origin + FIntVector2(ParentRoom->Width - 1, 1), ParentRoom->Origin + FIntVector2(ParentRoom->Width - 1, ParentRoom->Height - 2));
			NewRoomDoorSegment = TTuple<FIntVector2, FIntVector2>
				(NewRoom->Origin + FIntVector2(0, 1), NewRoom->Origin + FIntVector2(0, NewRoom->Height - 2));
			ParentRoomDoorSegmentLength = SegmentLength(ParentRoomDoorSegment);
			NewRoomDoorSegmentLength = SegmentLength(NewRoomDoorSegment);
			ParentRoomDoorStartCoord = Properties->RandomStream.RandRange(0, ParentRoomDoorSegmentLength - EdgeWidth);
			NewRoomDoorStartCoord = Properties->RandomStream.RandRange(0, NewRoomDoorSegmentLength - EdgeWidth);
			ParentRoomDoor = TTuple<FIntVector2, FIntVector2>(
				FIntVector2(ParentRoomDoorSegment.Key.X, ParentRoomDoorSegment.Key.Y + ParentRoomDoorStartCoord),
				FIntVector2(ParentRoomDoorSegment.Key.X, ParentRoomDoorSegment.Key.Y + ParentRoomDoorStartCoord + EdgeWidth - 1));
			NewRoomDoor = TTuple<FIntVector2, FIntVector2>(
				FIntVector2(NewRoomDoorSegment.Key.X, NewRoomDoorSegment.Key.Y + NewRoomDoorStartCoord),
				FIntVector2(NewRoomDoorSegment.Key.X, NewRoomDoorSegment.Key.Y + NewRoomDoorStartCoord + EdgeWidth - 1));
			bIsCorridorPossible = FindAWay(
				std::make_pair(
					std::make_pair(ParentRoomDoor.Key.X, ParentRoomDoor.Key.Y),
					std::make_pair(ParentRoomDoor.Value.X, ParentRoomDoor.Value.Y)),
				std::make_pair(
					std::make_pair(NewRoomDoor.Key.X, NewRoomDoor.Key.Y),
					std::make_pair(NewRoomDoor.Value.X, NewRoomDoor.Value.Y)),
				ParentRoom, NewRoom, Edge, true) > 0;
			if (!bIsCorridorPossible) continue;
			NewRoom->LeftSegment.bIsUsed = true;
			ParentRoomSegment.Door = ParentRoomDoor;
			NewRoom->UpSegment.Door = NewRoomDoor;
			break;
		case EDirection::UP:
			MinDistanceFromParent = EdgeWidth + 1;
			MaxDistanceFromParent = MinDistanceFromParent;
			do
			{
				Properties->RandomStream.GenerateNewSeed();
				RoomYCoord = Properties->RandomStream.RandRange(
					ParentRoom->Origin.Y + ParentRoom->Height - 1 + MinDistanceFromParent,
					ParentRoom->Origin.Y + ParentRoom->Height - 1 + MaxDistanceFromParent);
				int32 DistanceFromParent(FMath::Abs(ParentRoom->Origin.Y + ParentRoom->Height - 1 - RoomYCoord));
				RoomXCoord = Properties->RandomStream.RandRange(
					ParentRoom->Origin.X - DistanceFromParent,
					ParentRoom->Origin.X + ParentRoom->Width - 2 + DistanceFromParent);
				Retries++;
				MaxDistanceFromParent++;
			} while (IsOccupied(FIntVector2(RoomXCoord, RoomYCoord), NewRoom->Width, NewRoom->Height) && Retries < 1000);
			NewRoom->Origin = FIntVector2(RoomXCoord, RoomYCoord);
			ParentRoomDoorSegment = TTuple<FIntVector2, FIntVector2>
				(ParentRoom->Origin + FIntVector2(1, ParentRoom->Height - 1), ParentRoom->Origin + FIntVector2(ParentRoom->Width - 2, ParentRoom->Height - 1));;
			NewRoomDoorSegment = TTuple<FIntVector2, FIntVector2>
				(NewRoom->Origin + FIntVector2(1, 0), NewRoom->Origin + FIntVector2(NewRoom->Width - 2, 0));
			ParentRoomDoorSegmentLength = SegmentLength(ParentRoomDoorSegment);
			NewRoomDoorSegmentLength = SegmentLength(NewRoomDoorSegment);
			ParentRoomDoorStartCoord = Properties->RandomStream.RandRange(0, ParentRoomDoorSegmentLength - EdgeWidth);
			NewRoomDoorStartCoord = Properties->RandomStream.RandRange(0, NewRoomDoorSegmentLength - EdgeWidth);
			ParentRoomDoor = TTuple<FIntVector2, FIntVector2>(
				FIntVector2(ParentRoomDoorSegment.Key.X + ParentRoomDoorStartCoord, ParentRoomDoorSegment.Key.Y),
				FIntVector2(ParentRoomDoorSegment.Key.X + ParentRoomDoorStartCoord + EdgeWidth - 1, ParentRoomDoorSegment.Key.Y));
			NewRoomDoor = TTuple<FIntVector2, FIntVector2>(
				FIntVector2(NewRoomDoorSegment.Key.X + NewRoomDoorStartCoord, NewRoomDoorSegment.Key.Y),
				FIntVector2(NewRoomDoorSegment.Key.X + NewRoomDoorStartCoord + EdgeWidth - 1, NewRoomDoorSegment.Key.Y));
			bIsCorridorPossible = FindAWay(
				std::make_pair(
					std::make_pair(ParentRoomDoor.Key.X, ParentRoomDoor.Key.Y),
					std::make_pair(ParentRoomDoor.Value.X, ParentRoomDoor.Value.Y)),
				std::make_pair(
					std::make_pair(NewRoomDoor.Key.X, NewRoomDoor.Key.Y),
					std::make_pair(NewRoomDoor.Value.X, NewRoomDoor.Value.Y)),
				ParentRoom, NewRoom, Edge, true) > 0;
			if (!bIsCorridorPossible) continue;
			NewRoom->DownSegment.bIsUsed = true;
			ParentRoomSegment.Door = ParentRoomDoor;
			NewRoom->UpSegment.Door = NewRoomDoor;
			break;
		case EDirection::LEFT:
			MinDistanceFromParent = NewRoom->Width + EdgeWidth + 1;
			MaxDistanceFromParent = MinDistanceFromParent;
			do
			{
				Properties->RandomStream.GenerateNewSeed();
				RoomXCoord = Properties->RandomStream.RandRange(
					ParentRoom->Origin.X - MinDistanceFromParent,
					ParentRoom->Origin.X - MaxDistanceFromParent);
				int32 DistanceFromParent(FMath::Abs(ParentRoom->Origin.X - RoomXCoord));
				RoomYCoord = Properties->RandomStream.RandRange(
					ParentRoom->Origin.Y - DistanceFromParent,
					ParentRoom->Origin.Y + ParentRoom->Height - 2 + DistanceFromParent);
				Retries++;
				MaxDistanceFromParent++;
			} while (IsOccupied(FIntVector2(RoomXCoord, RoomYCoord), NewRoom->Width, NewRoom->Height) && Retries < 1000);
			NewRoom->Origin = FIntVector2(RoomXCoord, RoomYCoord);
			ParentRoomDoorSegment = TTuple<FIntVector2, FIntVector2>
				(ParentRoom->Origin + FIntVector2(0, 1), ParentRoom->Origin + FIntVector2(0, ParentRoom->Height - 2));
			NewRoomDoorSegment = TTuple<FIntVector2, FIntVector2>
				(NewRoom->Origin + FIntVector2(NewRoom->Width - 1, 1), NewRoom->Origin + FIntVector2(NewRoom->Width - 1, NewRoom->Height - 2));
			ParentRoomDoorSegmentLength = SegmentLength(ParentRoomDoorSegment);
			NewRoomDoorSegmentLength = SegmentLength(NewRoomDoorSegment);
			ParentRoomDoorStartCoord = Properties->RandomStream.RandRange(0, ParentRoomDoorSegmentLength - EdgeWidth);
			NewRoomDoorStartCoord = Properties->RandomStream.RandRange(0, NewRoomDoorSegmentLength - EdgeWidth);
			ParentRoomDoor = TTuple<FIntVector2, FIntVector2>(
				FIntVector2(ParentRoomDoorSegment.Key.X, ParentRoomDoorSegment.Key.Y + ParentRoomDoorStartCoord),
				FIntVector2(ParentRoomDoorSegment.Key.X, ParentRoomDoorSegment.Key.Y + ParentRoomDoorStartCoord + EdgeWidth - 1));
			NewRoomDoor = TTuple<FIntVector2, FIntVector2>(
				FIntVector2(NewRoomDoorSegment.Key.X, NewRoomDoorSegment.Key.Y + NewRoomDoorStartCoord),
				FIntVector2(NewRoomDoorSegment.Key.X, NewRoomDoorSegment.Key.Y + NewRoomDoorStartCoord + EdgeWidth - 1));
			bIsCorridorPossible = FindAWay(
				std::make_pair(
					std::make_pair(ParentRoomDoor.Key.X, ParentRoomDoor.Key.Y),
					std::make_pair(ParentRoomDoor.Value.X, ParentRoomDoor.Value.Y)),
				std::make_pair(
					std::make_pair(NewRoomDoor.Key.X, NewRoomDoor.Key.Y),
					std::make_pair(NewRoomDoor.Value.X, NewRoomDoor.Value.Y)),
				ParentRoom, NewRoom, Edge, true) > 0;
			if (!bIsCorridorPossible) continue;
			NewRoom->RightSegment.bIsUsed = true;
			ParentRoomSegment.Door = ParentRoomDoor;
			NewRoom->UpSegment.Door = NewRoomDoor;
			break;
		}
		if (bIsCorridorPossible)
		{
			ParentRoom->Doors.Add(ParentRoomSegment.Door);
			NewRoom->Doors.Add(NewRoom->UpSegment.Door);
			ParentRoomSegment.bIsUsed = true;
			InsertOccupiedTiles(NewRoom);
			FindAWay(
				std::make_pair(
					std::make_pair(ParentRoomSegment.Door.Key.X, ParentRoomSegment.Door.Key.Y),
					std::make_pair(ParentRoomSegment.Door.Value.X, ParentRoomSegment.Door.Value.Y)),
				std::make_pair(
					std::make_pair(NewRoom->UpSegment.Door.Key.X, NewRoom->UpSegment.Door.Key.Y),
					std::make_pair(NewRoom->UpSegment.Door.Value.X, NewRoom->UpSegment.Door.Value.Y)),
				ParentRoom, NewRoom, Edge);
		}
	}
	if (!bIsCorridorPossible) InvalidSeed = true;
	return NewRoom;
};

bool AGraphToDungeonGenerator::ConnectWithExisting(URoom* ParentRoom, URoom* ChildRoom, ULevelGraphEdge* Edge)
{
	const int32 EdgeWidth = Edge->Width;
	URoom MirrorParentRoom;
	MirrorParentRoom.Width = ParentRoom->Width;
	MirrorParentRoom.Height = ParentRoom->Height;
	MirrorParentRoom.Origin = ParentRoom->Origin;
	MirrorParentRoom.Segments = ParentRoom->Segments;
	URoom MirrorChildRoom;
	MirrorChildRoom.Width = ChildRoom->Width;
	MirrorChildRoom.Height = ChildRoom->Height;
	MirrorChildRoom.Origin = ChildRoom->Origin;
	MirrorChildRoom.Segments = ChildRoom->Segments;
	// Used segments
	EDirection ParentSegment;
	EDirection ChildSegment;
	// Generate door for mirror parent room segment
	for (auto& ParentRoomSegment : MirrorParentRoom.Segments)
	{
		//if (ParentRoomSegment.bIsUsed) continue;
		//
		TTuple<FIntVector2, FIntVector2> ParentRoomDoorSegment(FIntVector2(0, 0), FIntVector2(0, 0));
		int32 ParentRoomDoorSegmentLength(0);
		int32 ParentRoomDoorStartCoord(0);
		switch (ParentRoomSegment.Direction)
		{
		case EDirection::DOWN:
			ParentSegment = EDirection::DOWN;
			ParentRoomDoorSegment = TTuple<FIntVector2, FIntVector2>
				(ParentRoom->Origin + FIntVector2(1, 0), ParentRoom->Origin + FIntVector2(ParentRoom->Width - 2, 0));
			ParentRoomDoorSegmentLength = SegmentLength(ParentRoomDoorSegment);
			ParentRoomDoorStartCoord = Properties->RandomStream.RandRange(0, ParentRoomDoorSegmentLength - EdgeWidth);
			ParentRoomSegment.Door = TTuple<FIntVector2, FIntVector2>(
				FIntVector2(ParentRoomDoorSegment.Key.X + ParentRoomDoorStartCoord, ParentRoomDoorSegment.Key.Y),
				FIntVector2(ParentRoomDoorSegment.Key.X + ParentRoomDoorStartCoord + EdgeWidth - 1, ParentRoomDoorSegment.Key.Y));
			break;
		case EDirection::RIGHT:
			ParentSegment = EDirection::RIGHT;
			ParentRoomDoorSegment = TTuple<FIntVector2, FIntVector2>
				(ParentRoom->Origin + FIntVector2(ParentRoom->Width - 1, 1), ParentRoom->Origin + FIntVector2(ParentRoom->Width - 1, ParentRoom->Height - 2));
			ParentRoomDoorSegmentLength = SegmentLength(ParentRoomDoorSegment);
			ParentRoomDoorStartCoord = Properties->RandomStream.RandRange(0, ParentRoomDoorSegmentLength - EdgeWidth);
			ParentRoomSegment.Door = TTuple<FIntVector2, FIntVector2>(
				FIntVector2(ParentRoomDoorSegment.Key.X, ParentRoomDoorSegment.Key.Y + ParentRoomDoorStartCoord),
				FIntVector2(ParentRoomDoorSegment.Key.X, ParentRoomDoorSegment.Key.Y + ParentRoomDoorStartCoord + EdgeWidth - 1));
			break;
		case EDirection::UP:
			ParentSegment = EDirection::UP;
			ParentRoomDoorSegment = TTuple<FIntVector2, FIntVector2>
				(ParentRoom->Origin + FIntVector2(1, ParentRoom->Height - 1), ParentRoom->Origin + FIntVector2(ParentRoom->Width - 2, ParentRoom->Height - 1));;
			ParentRoomDoorSegmentLength = SegmentLength(ParentRoomDoorSegment);
			ParentRoomDoorStartCoord = Properties->RandomStream.RandRange(0, ParentRoomDoorSegmentLength - EdgeWidth);
			ParentRoomSegment.Door = TTuple<FIntVector2, FIntVector2>(
				FIntVector2(ParentRoomDoorSegment.Key.X + ParentRoomDoorStartCoord, ParentRoomDoorSegment.Key.Y),
				FIntVector2(ParentRoomDoorSegment.Key.X + ParentRoomDoorStartCoord + EdgeWidth - 1, ParentRoomDoorSegment.Key.Y));
			break;
		case EDirection::LEFT:
			ParentSegment = EDirection::LEFT;
			ParentRoomDoorSegment = TTuple<FIntVector2, FIntVector2>
				(ParentRoom->Origin + FIntVector2(0, 1), ParentRoom->Origin + FIntVector2(0, ParentRoom->Height - 2));
			ParentRoomDoorSegmentLength = SegmentLength(ParentRoomDoorSegment);
			ParentRoomDoorStartCoord = Properties->RandomStream.RandRange(0, ParentRoomDoorSegmentLength - EdgeWidth);
			ParentRoomSegment.Door = TTuple<FIntVector2, FIntVector2>(
				FIntVector2(ParentRoomDoorSegment.Key.X, ParentRoomDoorSegment.Key.Y + ParentRoomDoorStartCoord),
				FIntVector2(ParentRoomDoorSegment.Key.X, ParentRoomDoorSegment.Key.Y + ParentRoomDoorStartCoord + EdgeWidth - 1));
			break;
		}
		MirrorParentRoom.Doors.Add(ParentRoomSegment.Door);
		//
	}
	// Generate door for mirror child room segment
	for (auto& ChildRoomSegment : MirrorChildRoom.Segments)
	{
		if (ChildRoomSegment.bIsUsed) continue;
		//
		TTuple<FIntVector2, FIntVector2> ParentRoomDoorSegment(FIntVector2(0, 0), FIntVector2(0, 0));
		int32 ParentRoomDoorSegmentLength(0);
		int32 ParentRoomDoorStartCoord(0);
		switch (ChildRoomSegment.Direction)
		{
		case EDirection::DOWN:
			ChildSegment = EDirection::DOWN;
			ParentRoomDoorSegment = TTuple<FIntVector2, FIntVector2>
				(ChildRoom->Origin, ChildRoom->Origin + FIntVector2(ChildRoom->Width - 1, 0));
			ParentRoomDoorSegmentLength = SegmentLength(ParentRoomDoorSegment);
			ParentRoomDoorStartCoord = Properties->RandomStream.RandRange(0, ParentRoomDoorSegmentLength - EdgeWidth);
			ChildRoomSegment.Door = TTuple<FIntVector2, FIntVector2>(
				FIntVector2(ParentRoomDoorSegment.Key.X + ParentRoomDoorStartCoord, ParentRoomDoorSegment.Key.Y),
				FIntVector2(ParentRoomDoorSegment.Key.X + ParentRoomDoorStartCoord + EdgeWidth - 1, ParentRoomDoorSegment.Key.Y));
			break;
		case EDirection::RIGHT:
			ChildSegment = EDirection::RIGHT;
			ParentRoomDoorSegment = TTuple<FIntVector2, FIntVector2>
				(ChildRoom->Origin + FIntVector2(ChildRoom->Width - 1, 0), ChildRoom->Origin + FIntVector2(ChildRoom->Width - 1, ChildRoom->Height - 1));
			ParentRoomDoorSegmentLength = SegmentLength(ParentRoomDoorSegment);
			ParentRoomDoorStartCoord = Properties->RandomStream.RandRange(0, ParentRoomDoorSegmentLength - EdgeWidth);
			ChildRoomSegment.Door = TTuple<FIntVector2, FIntVector2>(
				FIntVector2(ParentRoomDoorSegment.Key.X, ParentRoomDoorSegment.Key.Y + ParentRoomDoorStartCoord),
				FIntVector2(ParentRoomDoorSegment.Key.X, ParentRoomDoorSegment.Key.Y + ParentRoomDoorStartCoord + EdgeWidth - 1));
			break;
		case EDirection::UP:
			ChildSegment = EDirection::UP;
			ParentRoomDoorSegment = TTuple<FIntVector2, FIntVector2>
				(ChildRoom->Origin + FIntVector2(0, ChildRoom->Height - 1), ChildRoom->Origin + FIntVector2(ChildRoom->Width - 1, ChildRoom->Height - 1));;
			ParentRoomDoorSegmentLength = SegmentLength(ParentRoomDoorSegment);
			ParentRoomDoorStartCoord = Properties->RandomStream.RandRange(0, ParentRoomDoorSegmentLength - EdgeWidth);
			ChildRoomSegment.Door = TTuple<FIntVector2, FIntVector2>(
				FIntVector2(ParentRoomDoorSegment.Key.X + ParentRoomDoorStartCoord, ParentRoomDoorSegment.Key.Y),
				FIntVector2(ParentRoomDoorSegment.Key.X + ParentRoomDoorStartCoord + EdgeWidth - 1, ParentRoomDoorSegment.Key.Y));
			break;
		case EDirection::LEFT:
			ChildSegment = EDirection::LEFT;
			ParentRoomDoorSegment = TTuple<FIntVector2, FIntVector2>
				(ChildRoom->Origin, ChildRoom->Origin + FIntVector2(0, ChildRoom->Height - 1));
			ParentRoomDoorSegmentLength = SegmentLength(ParentRoomDoorSegment);
			ParentRoomDoorStartCoord = Properties->RandomStream.RandRange(0, ParentRoomDoorSegmentLength - EdgeWidth);
			ChildRoomSegment.Door = TTuple<FIntVector2, FIntVector2>(
				FIntVector2(ParentRoomDoorSegment.Key.X, ParentRoomDoorSegment.Key.Y + ParentRoomDoorStartCoord),
				FIntVector2(ParentRoomDoorSegment.Key.X, ParentRoomDoorSegment.Key.Y + ParentRoomDoorStartCoord + EdgeWidth - 1));
			break;
		}
		MirrorChildRoom.Doors.Add(ChildRoomSegment.Door);
		//
	}
	int32 SmallestDistance(MAX_int32);
	TTuple<FIntVector2, FIntVector2> SourceDoorPos;
	TTuple<FIntVector2, FIntVector2> FinishDoorPos;
	for (const auto& ParentDoors : MirrorParentRoom.Doors)
	{
		for (const auto& ChildDoors : MirrorChildRoom.Doors)
		{
			int32 NewDistance(FindAWay(
				std::make_pair(
					std::make_pair(ParentDoors.Key.X, ParentDoors.Key.Y),
					std::make_pair(ParentDoors.Value.X, ParentDoors.Value.Y)),
				std::make_pair(
					std::make_pair(ChildDoors.Key.X, ChildDoors.Key.Y),
					std::make_pair(ChildDoors.Value.X, ChildDoors.Value.Y)),
				ParentRoom, ChildRoom, Edge, true));
			if (NewDistance > 0 && NewDistance < SmallestDistance)
			{
				SmallestDistance = NewDistance;
				SourceDoorPos = ParentDoors;
				FinishDoorPos = ChildDoors;
			}
		}
	}
	if (SmallestDistance == MAX_int32) return false;
	// Create corridor
	ParentRoom->Doors.Add(SourceDoorPos);
	switch (ParentSegment)
	{
	case EDirection::DOWN:
		ParentRoom->DownSegment.bIsUsed = true;
		ParentRoom->DownSegment.Door = SourceDoorPos;
		break;
	case EDirection::UP:
		ParentRoom->UpSegment.bIsUsed = true;
		ParentRoom->UpSegment.Door = SourceDoorPos;
		break;
	case EDirection::LEFT:
		ParentRoom->LeftSegment.bIsUsed = true;
		ParentRoom->LeftSegment.Door = SourceDoorPos;
		break;
	case EDirection::RIGHT:
		ParentRoom->RightSegment.bIsUsed = true;
		ParentRoom->RightSegment.Door = SourceDoorPos;
		break;
	}
	ChildRoom->Doors.Add(FinishDoorPos);
	switch (ChildSegment)
	{
	case EDirection::DOWN:
		ChildRoom->DownSegment.bIsUsed = true;
		ChildRoom->DownSegment.Door = FinishDoorPos;
		break;
	case EDirection::UP:
		ChildRoom->UpSegment.bIsUsed = true;
		ChildRoom->UpSegment.Door = FinishDoorPos;
		break;
	case EDirection::LEFT:
		ChildRoom->LeftSegment.bIsUsed = true;
		ChildRoom->LeftSegment.Door = FinishDoorPos;
		break;
	case EDirection::RIGHT:
		ChildRoom->RightSegment.bIsUsed = true;
		ChildRoom->RightSegment.Door = FinishDoorPos;
		break;
	}
	FindAWay(
		std::make_pair(
			std::make_pair(SourceDoorPos.Key.X, SourceDoorPos.Key.Y),
			std::make_pair(SourceDoorPos.Value.X, SourceDoorPos.Value.Y)),
		std::make_pair(
			std::make_pair(FinishDoorPos.Key.X, FinishDoorPos.Key.Y),
			std::make_pair(FinishDoorPos.Value.X, FinishDoorPos.Value.Y)),
		ParentRoom, ChildRoom, Edge);
	return true;
}

int32 AGraphToDungeonGenerator::FindAWay(
	const std::pair<std::pair<int, int>, std::pair<int, int>> Source,
	const std::pair<std::pair<int, int>, std::pair<int, int>> Finish,
	const URoom* SourceRoom, const URoom* FinishRoom, ULevelGraphEdge* Edge, const bool bFindPathOnly)
{
	const int Width = std::abs(Source.first.first - Source.second.first) + std::abs(Source.first.second - Source.second.second) + 1;
	AllCorridors.AddDefaulted();
	UCorridor* Corridor = &AllCorridors.Last();
	Corridor->Width = Width; 
	Corridor->LocalTheme = Edge->CorridorTheme;
	struct Element
	{
		Element() = default;
		Element(const std::pair<int, int>& Parent, const unsigned F, const unsigned G)
			: Parent(Parent), F(F), G(G) {}
		std::pair<int, int> Parent;
		unsigned F = 0;
		unsigned G = 0;
	};
	std::map<std::pair<int, int>, Element> OpenList;
	std::map<std::pair<int, int>, Element> ClosedList;

	if (Source.first.first == Source.second.first)
	{
		if (Source.first.first == SourceRoom->Origin.X)
		{
			std::pair<int, int> NewSource(Source.first.first - Width, Source.first.second);
			OpenList.emplace(NewSource, Element(NewSource, 0, 0));
			Corridor->StartBot = FIntVector2(Source.first.first - 1, Source.first.second);
			Corridor->StartTop = FIntVector2(Source.first.first - 1, Source.first.second + Width - 1);
		}
		else
		{
			std::pair<int, int> NewSource(Source.first.first + 1, Source.first.second);
			OpenList.emplace(NewSource, Element(NewSource, 0, 0));
			Corridor->StartBot = FIntVector2(Source.first.first + 1, Source.first.second);
			Corridor->StartTop = FIntVector2(Source.first.first + 1, Source.first.second + Width - 1);
		}
	}
	else
	{
		if (Source.first.second == SourceRoom->Origin.Y)
		{
			std::pair<int, int> NewSource(Source.first.first, Source.first.second - Width);
			OpenList.emplace(NewSource, Element(NewSource, 0, 0));
			Corridor->StartBot = FIntVector2(Source.first.first, Source.first.second - 1);
			Corridor->StartTop = FIntVector2(Source.first.first + Width - 1, Source.first.second - 1);
		}
		else
		{
			std::pair<int, int> NewSource(Source.first.first, Source.first.second + 1);
			OpenList.emplace(NewSource, Element(NewSource, 0, 0));
			Corridor->StartBot = FIntVector2(Source.first.first, Source.first.second + 1);
			Corridor->StartTop = FIntVector2(Source.first.first + Width - 1, Source.first.second + 1);
		}
	}

	std::pair<int, int> NewFinish;
	EDirection FinishDirection;
	if (Finish.first.first == Finish.second.first)
	{
		if (Finish.first.first == FinishRoom->Origin.X)
		{
			std::pair<int, int> NewSource(Finish.first.first - 1, Finish.first.second);
			NewFinish = NewSource;
			FinishDirection = EDirection::LEFT;
			Corridor->EndBot = FIntVector2(Finish.first.first - 1, Finish.first.second);
			Corridor->EndTop = FIntVector2(Finish.first.first - 1, Finish.first.second + Width - 1);
		}
		else
		{
			std::pair<int, int> NewSource(Finish.first.first + 1, Finish.first.second);
			NewFinish = NewSource;
			FinishDirection = EDirection::RIGHT;
			Corridor->EndBot = FIntVector2(Finish.first.first + 1, Finish.first.second);
			Corridor->EndTop = FIntVector2(Finish.first.first + 1, Finish.first.second + Width - 1);
		}
	}
	else
	{
		if (Finish.first.second == FinishRoom->Origin.Y)
		{
			std::pair<int, int> NewSource(Finish.first.first, Finish.first.second - 1);
			NewFinish = NewSource;
			FinishDirection = EDirection::DOWN;
			Corridor->EndBot = FIntVector2(Finish.first.first, Finish.first.second - 1);
			Corridor->EndTop = FIntVector2(Finish.first.first + Width - 1, Finish.first.second - 1);
		}
		else
		{
			std::pair<int, int> NewSource(Finish.first.first, Finish.first.second + 1);
			NewFinish = NewSource;
			FinishDirection = EDirection::UP;
			Corridor->EndBot = FIntVector2(Finish.first.first, Finish.first.second + 1);
			Corridor->EndTop = FIntVector2(Finish.first.first + Width - 1, Finish.first.second + 1);
		}
	}

	int debug_pause = 0;
	bool finished = false;
	std::pair<int, int> Result;

	while (!OpenList.empty() && debug_pause < 1000 && !finished)
	{
		debug_pause++;
		//UE_LOG(LogTemp, Warning, TEXT("OpenList size %d"), OpenList.size());
		std::pair<std::pair<int, int>, Element> Q(std::make_pair(0, 0), Element(Source.first, MAX_int32, MAX_int32));
		for (const auto& A : OpenList)
		{
			if (A.second.F < Q.second.F)
			{
				Q = A;
			}
		}
		if (IsOccupied(FIntVector2(Q.first.first, Q.first.second), Width, Width)) break;
		OpenList.erase(Q.first);
		if (ClosedList.contains(Q.first)) ClosedList[Q.first] = Q.second;
		else ClosedList.emplace(Q);

		std::array<std::pair<int, int>, 4> Successors = { {
				std::make_pair(Q.first.first + Width, Q.first.second),
				std::make_pair(Q.first.first - Width, Q.first.second),
				std::make_pair(Q.first.first, Q.first.second + Width),
				std::make_pair(Q.first.first, Q.first.second - Width)
			} };
		for (const auto& Successor : Successors)
		{
			if (Finish.first.first >= Successor.first &&
				Finish.first.first < Successor.first + Width &&
				Finish.first.second >= Successor.second &&
				Finish.first.second < Successor.second + Width)
			{
				// Skip successor if the doors are on the width size of corridor,
				// accept finishing successor only from normal direction
				if (Finish.first.first == Finish.second.first &&
					Successor.first == Q.first.first &&
					Finish.first.first - FinishRoom->Origin.X < Width) continue;
				if (Finish.first.second == Finish.second.second &&
					Successor.second == Q.first.second &&
					Finish.first.second - FinishRoom->Origin.Y < Width) continue;
				if (Finish.first.first == Finish.second.first &&
					Successor.first == Q.first.first &&
					FinishRoom->Origin.X + FinishRoom->Width - Finish.first.first < Width) continue;
				if (Finish.first.second == Finish.second.second &&
					Successor.second == Q.first.second &&
					FinishRoom->Origin.Y + FinishRoom->Height - Finish.first.second < Width) continue;

				finished = true;
				Result = Q.first;
				std::array<int, 2> MoveDirection = { {0, 0} };
				std::array<int, 2> MoveDirectionWidth = { {0, 0} };
				int FinishDistance = 0;
				switch (FinishDirection)
				{
				case EDirection::LEFT:
					MoveDirection[0] = -1;
					MoveDirectionWidth[1] = 1;
					FinishDistance = NewFinish.first - (Q.first.first + Width - 1);
					break;
				case EDirection::RIGHT:
					MoveDirection[0] = 1;
					MoveDirectionWidth[1] = 1;
					FinishDistance = Q.first.first - NewFinish.first;
					break;
				case EDirection::UP:
					MoveDirection[1] = 1;
					MoveDirectionWidth[0] = 1;
					FinishDistance = Q.first.second - NewFinish.second;
					break;
				case EDirection::DOWN:
					MoveDirection[1] = -1;
					MoveDirectionWidth[0] = 1;
					FinishDistance = NewFinish.second - (Q.first.second + Width - 1);
					break;
				}
				for (int WidthIndex = 0; WidthIndex < Width; WidthIndex++) {
					for (int FinishIndex = 0; FinishIndex < FinishDistance + Width; FinishIndex++)
					{
						// check if not overlaping with the end
						if (FinishDirection == EDirection::UP &&
							NewFinish.second + FinishIndex == Q.first.second &&
							NewFinish.first + WidthIndex <= Q.first.first) break;
						if (FinishDirection == EDirection::DOWN &&
							NewFinish.second - FinishIndex == Q.first.second + Width - 1 &&
							NewFinish.first + WidthIndex <= Q.first.first + Width - 1) break;
						if (FinishDirection == EDirection::LEFT &&
							NewFinish.first - FinishIndex == Q.first.first + Width - 1 &&
							NewFinish.second + WidthIndex <= Q.first.second) break;
						if (FinishDirection == EDirection::RIGHT &&
							NewFinish.first + FinishIndex == Q.first.first &&
							NewFinish.second + WidthIndex <= Q.first.second + Width - 1) break;
						if (!bFindPathOnly)
						{
							OccupiedTiles.Add(
								FIntVector2(FinishIndex* MoveDirection[0], FinishIndex* MoveDirection[1]) +
								FIntVector2(WidthIndex * MoveDirectionWidth[0], WidthIndex * MoveDirectionWidth[1]) +
								FIntVector2(NewFinish.first, NewFinish.second));
							Corridor->Points.Add(
								FIntVector2(FinishIndex* MoveDirection[0], FinishIndex* MoveDirection[1]) +
								FIntVector2(WidthIndex * MoveDirectionWidth[0], WidthIndex * MoveDirectionWidth[1]) +
								FIntVector2(NewFinish.first, NewFinish.second));
						}
					}
				}
			}
			if (finished) break;
			if (IsOccupied(FIntVector2(Successor.first, Successor.second), Width, Width)) continue;
			unsigned G = Q.second.G + Width;
			unsigned H = std::abs(Successor.first - Finish.first.first) + std::abs(Successor.second - Finish.first.second);
			if (OpenList.contains(Successor))
			{
				if (G < OpenList[Successor].G)
				{
					OpenList[Successor].Parent = Q.first;
					OpenList[Successor].G = G;
					OpenList[Successor].F = G + H;
				}
			}
			else if (ClosedList.contains(Successor))
			{
				if (G < ClosedList[Successor].G)
				{
					OpenList.emplace(Successor, Element(Q.first, G + H, G));
				}
			}
			else
			{
				OpenList.emplace(Successor, Element(Q.first, G + H, G));
			}
		}
	}
	int32 ResultLength(0);
	if (finished)
	{
		std::pair<int, int> Actual = Result;
		std::pair<int, int> Next = ClosedList[Actual].Parent;
		bool isSource = false;
		while (!isSource)
		{
			if (!bFindPathOnly)
			{
				Corridor->Squares.Add(FIntVector2(Actual.first, Actual.second));
				for (int i = 0; i < Width; i++)
				{
					for (int j = 0; j < Width; j++)
					{
						OccupiedTiles.Add(FIntVector2(Actual.first, Actual.second) + FIntVector2(i, j));
					}
				}
			}
			isSource = Actual == Next;
			if (!isSource)
			{
				Actual = Next;
				Next = ClosedList[Next].Parent;
			}
			ResultLength++;
		}
	}
	else
	{
		//InvalidSeed = true;
	}
	if (bFindPathOnly) AllCorridors.Pop();
	if (ResultLength > Properties->MaxCorridorLength) ResultLength = 0;
	return ResultLength;
};

bool AGraphToDungeonGenerator::Generate(UGraphToDungeonProperties* LevelProperties)
{
	Properties = LevelProperties;
	GlobalTileRotation = Properties->RotateTiles;
	const ULevelGraphSession* const Graph = Properties->LevelGraph;
	GraphSession = Graph;
	//GeneratorRootComponent->ClearInstances();
	for (auto Room : AllRooms)
	{
		delete Room;
	}
	AllRooms.Empty();
	AllCorridors.Empty();
	OccupiedTiles.Empty();
	tileSize = Properties->TileSize;
	InvalidSeed = false;

	// Prepare all nodes to be processed
	TMap<ULevelGraphNode*, URoom*> NodesToProcess;
	// Node with most neighbours, or first with four (allowed maximum)
	ULevelGraphNode* InitialNode = Cast<ULevelGraphNode>(Graph->AllNodes[0]);
	for (const auto Node : Graph->AllNodes)
	{
		NodesToProcess.Add(Cast<ULevelGraphNode>(Node), nullptr);
		if (Node->ChildrenNodes.Num() + Node->ParentNodes.Num() >
			InitialNode->ChildrenNodes.Num() + InitialNode->ParentNodes.Num())
			InitialNode = Cast<ULevelGraphNode>(Node);
	}
	URoom DefaultRoom;
	URoom* ParentRoom = &DefaultRoom;
	ParentRoom->Width = 0;
	ParentRoom->Height = 0;
	ParentRoom->Origin = FIntVector2(0, 0);

	// Construct initial room
	AllRooms.Add(new URoom());
	URoom* InitialRoom = AllRooms.Last();
	InitialRoom->Width = InitialNode->Width;
	InitialRoom->Height = InitialNode->Height;
	InitialRoom->LocalTheme = InitialNode->RoomTheme;
	InsertOccupiedTiles(InitialRoom);
	NodesToProcess[InitialNode] = InitialRoom;

	TMap<TPair<URoom*, URoom*>, ULevelGraphEdge*> ConnectExistingPool;
	TQueue<TTuple<ULevelGraphNode*, URoom*>> NodeQueue;
	NodeQueue.Enqueue(TTuple<ULevelGraphNode*, URoom*>(InitialNode, InitialRoom));
	while (!NodeQueue.IsEmpty())
	{
		TTuple<ULevelGraphNode*, URoom*> NodeToProcess;
		NodeQueue.Dequeue(NodeToProcess);
		TArray<UGenericGraphNode*> NeighbourNodes;
		NeighbourNodes.Append(NodeToProcess.Key->ChildrenNodes);
		NeighbourNodes.Append(NodeToProcess.Key->ParentNodes);
		for (auto GenericNeighbourNode : NeighbourNodes)
		{
			ULevelGraphEdge* Edge = Cast<ULevelGraphEdge>(NodeToProcess.Key->GetEdge(GenericNeighbourNode));
			if (!Edge) Edge = Cast<ULevelGraphEdge>(GenericNeighbourNode->GetEdge(NodeToProcess.Key));
			int32 EdgeWidth = Edge->Width;
			ULevelGraphNode* NeighbourNode = Cast<ULevelGraphNode>(GenericNeighbourNode);
			URoom* NeighbourRoom = NodesToProcess[NeighbourNode];
			ParentRoom = NodeToProcess.Value;
			if (NeighbourRoom && ParentRoom->Connections.Contains(NeighbourRoom)) continue;
			if (!NeighbourRoom) NodesToProcess[NeighbourNode] = ConnectWithNew(ParentRoom, NeighbourNode, Edge);
			else ConnectExistingPool.Add(TTuple<URoom*, URoom*>(ParentRoom, NeighbourRoom), Edge);
			NeighbourRoom = NodesToProcess[NeighbourNode];
			if (NeighbourNode->RoomTheme) NeighbourRoom->LocalTheme = NeighbourNode->RoomTheme;
			NodeQueue.Enqueue(TTuple<ULevelGraphNode*, URoom*>(NeighbourNode, NeighbourRoom));
			ParentRoom->Connections.Add(NeighbourRoom);
			NeighbourRoom->Connections.Add(ParentRoom);
		}
	}
	for (auto& ConnectionPair : ConnectExistingPool)
	{
		if (!ConnectWithExisting(ConnectionPair.Key.Key, ConnectionPair.Key.Value, ConnectionPair.Value))
		{
			InvalidSeed = true;
		}
	}
	SpawnRooms();
	if (InvalidSeed) UE_LOG(LogTemp, Warning, TEXT("ThemeSeed Invalid"));
	if (InvalidSeed) return false;
	return true;
}
