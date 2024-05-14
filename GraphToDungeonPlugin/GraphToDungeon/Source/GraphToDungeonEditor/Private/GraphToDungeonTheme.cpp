// Copyright (c) 2024 Richard Pajersky.


#include "GraphToDungeonTheme.h"

UGraphToDungeonTheme::UGraphToDungeonTheme()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("StaticMesh'/Engine/BasicShapes/Sphere.Sphere'"));
	//UE_LOG(LogTemp, Warning, TEXT("Successor Room Node : %s"), (CubeMesh.Succeeded() ? TEXT("cube mesh succ") : TEXT("sube mesh not succ")));
	//Walls.Add(FMeshWithProbability{ CubeMesh.Object.Get() });
	//Floors.Add(FMeshWithProbability{ CubeMesh.Object.Get() });
	//Doors.Add(FMeshWithProbability{ CubeMesh.Object.Get() });

	MeshCategories.Add("Walls", &Walls);
	MeshCategories.Add("OutsideWallCorners", &OutsideWallCorners);
	MeshCategories.Add("InsideWallCorners", &InsideWallCorners);
	MeshCategories.Add("Floors", &Floors);
	MeshCategories.Add("Doors", &Doors);
	MeshCategories.Add("DoorFrameLeft", &DoorFrameLeft);
	MeshCategories.Add("DoorFrameRight", &DoorFrameRight);
}

UGraphToDungeonTheme::~UGraphToDungeonTheme()
{
}

void UGraphToDungeonTheme::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	for (auto& Category : MeshCategories)
	{
		if (PropertyChangedEvent.GetMemberPropertyName().ToString() == Category.Key)
		{
			ComputeProbability(*Category.Value);
		}
	}
}

void UGraphToDungeonTheme::ComputeProbability(TArray<FMeshWithProbability>& Array)
{
	float ProbabilitySum = 0.0f;
	for (const auto& Component : Array)
	{
		ProbabilitySum += Component.Probability;
	}
	for (auto& Component : Array)
	{
		Component.ProbabilitySum = ProbabilitySum;
		if (ProbabilitySum > 0) {
			const float FinalProbability = Component.Probability / ProbabilitySum;
			Component.RealProbability = FinalProbability * 100;
		}
			
	}
}

bool UGraphToDungeonTheme::AreMeshesDefined()
{
	for (auto& Category : MeshCategories)
	{
		if (!(Category.Value->IsValidIndex(0) && (*Category.Value)[0].Mesh))
		{
			return false;
		}
	}
	return true;
}
