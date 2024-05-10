// Copyright (c) 2024 Richard Pajersky.

#include "AssetTypeAction_Properties.h"
#include "GraphToDungeonProperties.h"

#define LOCTEXT_NAMESPACE "AssetTypeAction_Properties"

FAssetTypeAction_Properties::FAssetTypeAction_Properties(EAssetTypeCategories::Type InAssetCategory)
	: MyAssetCategory(InAssetCategory)
{
}

FText FAssetTypeAction_Properties::GetName() const
{
	return LOCTEXT("FGraphToDungeonAssetTypeActionsName", "Graph to Dungeon");
}

FColor FAssetTypeAction_Properties::GetTypeColor() const
{
	return FColor(255, 165, 0);
}

UClass* FAssetTypeAction_Properties::GetSupportedClass() const
{
	return UGraphToDungeonProperties::StaticClass();
}

uint32 FAssetTypeAction_Properties::GetCategories()
{
	return MyAssetCategory;
}

#undef LOCTEXT_NAMESPACE