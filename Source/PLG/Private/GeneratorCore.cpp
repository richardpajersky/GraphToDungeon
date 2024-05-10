// Fill out your copyright notice in the Description page of Project Settings.


#include "GeneratorCore.h"

#include "Kismet/KismetMathLibrary.h"
#include "Engine/InstancedStaticMesh.h"

// Sets default values
AGeneratorCore::AGeneratorCore()
{
	UE_LOG(LogTemp, Warning, TEXT("Constructor"));
	mazeSize = 100;
	maxRoomSize = 20;
	minRoomSize = 5;
	maxRoomCount = 50;
	roomRetryLimit = 100;
	tileSize = 100.f;
	floorTile = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("floorTile"));
	SetRootComponent(floorTile);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("StaticMesh'/Engine/BasicShapes/Cube.Cube'"));
	if (CubeMesh.Succeeded())
	{
		floorTile->SetStaticMesh(CubeMesh.Object);
	}
	floorTile->AddInstance(
		FTransform(FVector(0.f, 0.f, 0.f)));
	//GenerateRooms();
	//SpawnRooms();
	/*static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialAsset(TEXT("Material'/Game/testMaterial.testMaterial"));
	if (MaterialAsset.Succeeded())
	{
		UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(MaterialAsset.Object, this);
		// Set the dynamic material instance to the Instanced Static Mesh Component
		floorTile->SetMaterial(0, DynamicMaterial);
	}*/
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void AGeneratorCore::BeginPlay()
{
	UE_LOG(LogTemp, Warning, TEXT("BeginPlay"));
	//floorTile->AddInstance(FTransform(FVector(0, 0, 0)));
	Super::BeginPlay();
	
}

// Called every frame
void AGeneratorCore::Tick(float DeltaTime)
{
	UE_LOG(LogTemp, Warning, TEXT("Tick"));
	Super::Tick(DeltaTime);

}

void AGeneratorCore::GenerateRoom(FVector& newPosition, FVector& newSize) const
{
	//UE_LOG(LogTemp, Warning, TEXT("GenerateRoom"));
	FVector size(FMath::RandRange(minRoomSize, maxRoomSize), FMath::RandRange(minRoomSize, maxRoomSize), 0.f);
	FVector position(FMath::RandRange((int64)0, mazeSize - FMath::TruncToInt(size.X)),
		FMath::RandRange((int64)0, mazeSize - FMath::TruncToInt(size.Y)), 0.f);

	newPosition = position;
	newSize = size;
}

bool AGeneratorCore::TryPlacingRoom(FVector& newRoomPosition, FVector& newRoomSize) const
{
	FVector roomPosition;
	FVector roomSize;
	GenerateRoom(roomPosition, roomSize);
	for (unsigned i = 0; i < static_cast<unsigned>(roomPositions.Num()); i++) {
		if (OverlappingRoom(roomPositions[i], roomSizes[i], roomPosition, roomSize)) {
			return false;
		}
	}
	newRoomPosition = roomPosition;
	newRoomSize = roomSize;
	return true;
}

bool AGeneratorCore::OverlappingRoom(const FVector& pos1, const FVector& size1, const FVector& pos2, const FVector& size2) const
{
	return false;
}

void AGeneratorCore::GenerateRooms()
{
	UE_LOG(LogTemp, Warning, TEXT("GenerateRooms"));
	unsigned tryCount = 0;
	FVector roomPosition;
	FVector roomSize;
	while (roomPositions.Num() != maxRoomCount && tryCount < roomRetryLimit) {
		if (TryPlacingRoom(roomPosition, roomSize)) {
			roomPositions.Add(roomPosition);
			roomSizes.Add(roomSize);
			tryCount = 0;
		}
		else {
			tryCount++;
		}
	}
}

void AGeneratorCore::SpawnRooms()
{
	UE_LOG(LogTemp, Warning, TEXT("SpawnRooms"));
	UE_LOG(LogTemp, Warning, TEXT("%d"), roomPositions.Num());
	UE_LOG(LogTemp, Warning, TEXT("%d"), roomSizes.Num());
	for (unsigned i = 0; i < static_cast<unsigned>(roomPositions.Num()); i++) {
		SpawnFloor(roomPositions[i], roomSizes[i]);
		UE_LOG(LogTemp, Warning, TEXT("%s"), *roomPositions[i].ToString());
	}
}

void AGeneratorCore::SpawnFloor(const FVector& pos, const FVector& size)
{
	FVector subSize = size - FVector(1.f, 10.f, 0.f);
	//UE_LOG(LogTemp, Warning, TEXT("%s"), subSize);
	//UE_LOG(LogTemp, Warning, TEXT("%d"), FMath::TruncToInt(subSize.X));
	//UE_LOG(LogTemp, Warning, TEXT("%d"), FMath::TruncToInt(subSize.Y));
	for (unsigned i = 0; i < FMath::TruncToInt(subSize.X); i++) {
		for (unsigned j = 0; j < FMath::TruncToInt(subSize.Y); j++) {
			UE_LOG(LogTemp, Warning, TEXT("SpwanFloor"));
			floorTile->AddInstance(
				FTransform(FVector(i * tileSize, j * tileSize, 0.f) + FVector(pos * tileSize)));
		}
	}
}



