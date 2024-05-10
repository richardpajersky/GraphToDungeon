// Copyright (c) 2024 Richard Pajersky.

#pragma once

#include "CoreMinimal.h"
#include "GenericGraphNode.h"
#include "GraphToDungeonTheme.h"
#include "LevelGraphNode.generated.h"

UENUM(BlueprintType)
enum class EDialoguerPosition : uint8 {
	Left,
	Right
};

/**
 * @brief Class representing graph node as rooms, providing room properties
 */
UCLASS(Blueprintable)
class GRAPHTODUNGEONEDITOR_API ULevelGraphNode : public UGenericGraphNode
{
	GENERATED_BODY()
public:
    ULevelGraphNode();

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Level Graph Node")
    FText Paragraph;

    UPROPERTY()
    EDialoguerPosition DialoguerPosition;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Level Graph Node")
    int32 Width = 20;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Level Graph Node")
    int32 Height = 30;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Level Graph Node")
    UGraphToDungeonTheme* RoomTheme;

#if WITH_EDITOR
    virtual FText GetNodeTitle() const override;

    virtual void SetNodeTitle(const FText& NewTitle) override;

    virtual FLinearColor GetBackgroundColor() const override;
#endif
};
