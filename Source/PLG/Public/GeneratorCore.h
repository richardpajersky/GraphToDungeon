// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GeneratorCore.generated.h"

UCLASS()
class PLG_API AGeneratorCore : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGeneratorCore();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	TArray<FVector> roomPositions;
	TArray<FVector> roomSizes;
public:
	UPROPERTY(EditAnywhere) int32 mazeSize = 0;
	UPROPERTY(EditAnywhere) int32 maxRoomSize = 0;
	UPROPERTY(EditAnywhere) int32 minRoomSize = 0;
	UPROPERTY(EditAnywhere) int32 maxRoomCount = 0;
	UPROPERTY(EditAnywhere) unsigned roomRetryLimit = 0;
	UPROPERTY(EditAnywhere) float tileSize = 0.f;
	UPROPERTY() UInstancedStaticMeshComponent* floorTile;
private:
	void GenerateRoom(FVector& newPosition, FVector& newSize) const;
	bool TryPlacingRoom(FVector& newRoomPosition, FVector& newRoomSize) const;
	bool OverlappingRoom(const FVector& pos1, const FVector& size1,
						 const FVector& pos2, const FVector& size2) const;
	void GenerateRooms();
	void SpawnRooms();
	void SpawnFloor(const FVector& pos, const FVector& size);
};
