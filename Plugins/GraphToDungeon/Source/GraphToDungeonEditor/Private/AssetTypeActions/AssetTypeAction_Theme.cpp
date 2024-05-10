// Copyright (c) 2024 Richard Pajersky.

#include "AssetTypeAction_Theme.h"
#include "GraphToDungeonTheme.h"

#define LOCTEXT_NAMESPACE "FAssetTypeAction_Theme"

FAssetTypeAction_Theme::FAssetTypeAction_Theme(EAssetTypeCategories::Type InAssetCategory)
	: MyAssetCategory(InAssetCategory)
{
}

FText FAssetTypeAction_Theme::GetName() const
{
	return LOCTEXT("FGraphToDungeonAssetTypeActionsName", "Graph to Dungeon");
}

FColor FAssetTypeAction_Theme::GetTypeColor() const
{
	return FColor(255, 0, 127);
}

UClass* FAssetTypeAction_Theme::GetSupportedClass() const
{
	return UGraphToDungeonTheme::StaticClass();
}

uint32 FAssetTypeAction_Theme::GetCategories()
{
	return MyAssetCategory;
}

#undef LOCTEXT_NAMESPACE