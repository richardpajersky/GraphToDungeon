// Copyright (c) 2024 Richard Pajersky.

#include "AssetTypeAction_Graph.h"
#include "LevelGraphSession.h"

#define LOCTEXT_NAMESPACE "FAssetTypeAction_Graph"

FAssetTypeAction_Graph::FAssetTypeAction_Graph(EAssetTypeCategories::Type InAssetCategory)
	: MyAssetCategory(InAssetCategory)
{
}

FText FAssetTypeAction_Graph::GetName() const
{
	return LOCTEXT("FGraphToDungeonAssetTypeActionsName", "Graph to Dungeon");
}

FColor FAssetTypeAction_Graph::GetTypeColor() const
{
	return FColor(106, 90, 205);
}

UClass* FAssetTypeAction_Graph::GetSupportedClass() const
{
	return ULevelGraphSession::StaticClass();
}

uint32 FAssetTypeAction_Graph::GetCategories()
{
	return MyAssetCategory;
}

#undef LOCTEXT_NAMESPACE