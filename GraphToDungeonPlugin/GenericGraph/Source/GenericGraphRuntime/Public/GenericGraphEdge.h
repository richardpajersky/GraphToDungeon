#pragma once

#include "CoreMinimal.h"
#include "GenericGraphNode.h"
#include "GenericGraphEdge.generated.h"

class UGenericGraph;

UCLASS(Blueprintable)
class GENERICGRAPHRUNTIME_API UGenericGraphEdge : public UObject
{
	GENERATED_BODY()

public:
	UGenericGraphEdge();
	virtual ~UGenericGraphEdge();

	UPROPERTY()
	UGenericGraph* Graph;

	UPROPERTY(BlueprintReadOnly, Category = "GenericGraphEdge", AdvancedDisplay)
	UGenericGraphNode* StartNode;

	UPROPERTY(BlueprintReadOnly, Category = "GenericGraphEdge", AdvancedDisplay)
	UGenericGraphNode* EndNode;

	UFUNCTION(BlueprintPure, Category = "GenericGraphEdge")
	UGenericGraph* GetGraph() const;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditDefaultsOnly)
	bool bShouldDrawTitle = false;

	UPROPERTY(EditDefaultsOnly)
	FText NodeTitle;

	UPROPERTY()
	FLinearColor EdgeColour = FLinearColor(0.9f, 0.9f, 0.9f, 1.0f);
#endif

#if WITH_EDITOR
	virtual FText GetNodeTitle() const { return NodeTitle; }
	FLinearColor GetEdgeColour() { return EdgeColour; }

	virtual void SetNodeTitle(const FText& NewTitle);
#endif
	
};
