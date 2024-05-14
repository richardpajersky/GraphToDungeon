#pragma once

#include "CoreMinimal.h"
#include "GenericGraphNode.h"
#include "GenericGraphEdge.h"
#include "GameplayTagContainer.h"
#include "GenericGraph.generated.h"

UCLASS(Blueprintable)
class GENERICGRAPHRUNTIME_API UGenericGraph : public UObject
{
	GENERATED_BODY()

public:
	UGenericGraph();
	virtual ~UGenericGraph();

	UPROPERTY()
	FString Name;

	UPROPERTY()
	TSubclassOf<UGenericGraphNode> NodeType;

	UPROPERTY()
	TSubclassOf<UGenericGraphEdge> EdgeType;

	UPROPERTY()
	FGameplayTagContainer GraphTags;

	UPROPERTY(BlueprintReadOnly, AdvancedDisplay)
	TArray<UGenericGraphNode*> RootNodes;

	UPROPERTY(BlueprintReadOnly, AdvancedDisplay)
	TArray<UGenericGraphNode*> AllNodes;

	UPROPERTY()
	bool bEdgeEnabled;

	UFUNCTION(BlueprintCallable, Category = "GenericGraph")
	void Print(bool ToConsole = true, bool ToScreen = true);

	UFUNCTION(BlueprintCallable, Category = "GenericGraph")
	int GetLevelNum() const;

	UFUNCTION(BlueprintCallable, Category = "GenericGraph")
	void GetNodesByLevel(int Level, TArray<UGenericGraphNode*>& Nodes);

	void ClearGraph();

#if WITH_EDITORONLY_DATA
	UPROPERTY()
	class UEdGraph* EdGraph;

	UPROPERTY()
	bool bCanRenameNode;

	UPROPERTY()
	bool bCanBeCyclical = true;

#endif
};
