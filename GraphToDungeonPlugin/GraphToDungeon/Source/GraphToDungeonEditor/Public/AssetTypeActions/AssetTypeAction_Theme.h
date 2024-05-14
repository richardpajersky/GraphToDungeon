// Copyright (c) 2024 Richard Pajersky.

#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions_Base.h"

class GRAPHTODUNGEONEDITOR_API FAssetTypeAction_Theme : public FAssetTypeActions_Base
{
public:
	FAssetTypeAction_Theme(EAssetTypeCategories::Type InAssetCategory);

	virtual FText GetName() const override;
	virtual FColor GetTypeColor() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual uint32 GetCategories() override;

private:
	EAssetTypeCategories::Type MyAssetCategory;
};

