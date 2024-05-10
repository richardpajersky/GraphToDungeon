#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "GenericGraphNode.generated.h"

class UGenericGraph;
class UGenericGraphEdge;

UENUM(BlueprintType)
enum class ENodeLimit : uint8
{
	Unlimited,
    Limited
};


UCLASS(Blueprintable)
class GENERICGRAPHRUNTIME_API UGenericGraphNode : public UObject
{
	GENERATED_BODY()

public:
	UGenericGraphNode();
	virtual ~UGenericGraphNode();

	UPROPERTY(VisibleDefaultsOnly, Category = "GenericGraphNode")
	UGenericGraph* Graph;

	UPROPERTY(BlueprintReadOnly, Category = "GenericGraphNode")
	TArray<UGenericGraphNode*> ParentNodes;

	UPROPERTY(BlueprintReadOnly, Category = "GenericGraphNode")
	TArray<UGenericGraphNode*> ChildrenNodes;

	UPROPERTY(BlueprintReadOnly, Category = "GenericGraphNode")
	TMap<UGenericGraphNode*, UGenericGraphEdge*> Edges;

	UFUNCTION(BlueprintCallable, Category = "GenericGraphNode")
	virtual UGenericGraphEdge* GetEdge(UGenericGraphNode* ChildNode);

	UFUNCTION(BlueprintCallable, Category = "GenericGraphNode")
	bool IsLeafNode() const;

	UFUNCTION(BlueprintCallable, Category = "GenericGraphNode")
	UGenericGraph* GetGraph() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GenericGraphNode")
	FText GetDescription() const;
	virtual FText GetDescription_Implementation() const;

	//////////////////////////////////////////////////////////////////////////
#if WITH_EDITORONLY_DATA
	UPROPERTY(EditDefaultsOnly)
	FText NodeTitle;

	UPROPERTY()
	TSubclassOf<UGenericGraph> CompatibleGraphType;

	UPROPERTY()
	FLinearColor BackgroundColor;

	UPROPERTY()
	FText ContextMenuName;

	UPROPERTY()
	ENodeLimit ParentLimitType;

	UPROPERTY()
	int32 ParentLimit;

	UPROPERTY()
	ENodeLimit ChildrenLimitType;

	UPROPERTY()
	int32 ChildrenLimit;

	UPROPERTY()
	class UEdGraphNode* OriginalNode;
	
#endif

#if WITH_EDITOR
	virtual bool IsNameEditable() const;

	virtual FLinearColor GetBackgroundColor() const;

	virtual FText GetNodeTitle() const;

	virtual void SetNodeTitle(const FText& NewTitle);

	virtual bool CanCreateConnection(UGenericGraphNode* Other, FText& ErrorMessage);

	virtual bool CanCreateConnectionTo(UGenericGraphNode* Other, int32 NumberOfChildrenNodes, FText& ErrorMessage);
	virtual bool CanCreateConnectionFrom(UGenericGraphNode* Other, int32 NumberOfParentNodes, FText& ErrorMessage);
#endif
};
