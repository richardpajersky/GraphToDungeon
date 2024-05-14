// Copyright (c) 2024 Richard Pajersky.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "Factory_LevelGraph.generated.h"

/**
 * 
 */
UCLASS()
class UFactory_LevelGraph : public UFactory
{
	GENERATED_BODY()
public:
	UFactory_LevelGraph();

	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual FText GetDisplayName() const override;
	virtual FString GetDefaultNewAssetName() const override;
};
