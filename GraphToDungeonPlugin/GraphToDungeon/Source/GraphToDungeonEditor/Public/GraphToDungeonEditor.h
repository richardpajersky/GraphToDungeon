// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright (c) 2024 Richard Pajersky.

#pragma once

#include "IAssetTools.h"
#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FToolBarBuilder;
class FMenuBuilder;
class UGraphToDungeonProperties;
class AGraphToDungeonGenerator;

/**
 * @brief Main class controling generation and defining UI
 */
class FGraphToDungeonModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	/** This function will be bound to Command (by default it will bring up plugin window) */
	void PluginButtonClicked();
	
private:

	void RegisterMenus();
	void RegisterAssetTypeAction(IAssetTools& AssetTools, TSharedRef<IAssetTypeActions> Action);

	/** Event handlers */
	TSharedRef<class SDockTab> OnSpawnPluginTab(const class FSpawnTabArgs& SpawnTabArgs);
	void OnPropertiesChanged(const FAssetData& AssetData);
	FReply OnGenerateNewLevelButtonClicked();
	FReply OnDeleteLevelButtonClicked();
	FReply OnRegenerateLevelButtonClicked();
	FReply OnRegenerateThemeButtonClicked();
	void HandleGeneratorDeleted();

	/** Helper functions*/
	FString GetPropertiesPath() const;
	bool IsPropertiesDefined() const;
	bool IsGeneratorSpawned() const;

	/**
	 * @brief Updates activity status of all buttons at once
	 */
	void UpdateButtonsStatus() const;

	/**
	 * @brief Calls Generator to launch generation
	 */
	void GenerateDungeon();

private:
	// Tab buttons
	TSharedPtr<SButton> GenerateNewLevelButton;
	TSharedPtr<SButton> DeleteLevelButton;
	TSharedPtr<SButton> RegenerateLevelButton;
	TSharedPtr<SButton> RegenerateThemeButton;
	TSharedPtr<STextBlock> InfoTextBlock;

	TSharedPtr<FAssetThumbnailPool> AssetThumbnailPool;
	TArray<TSharedPtr<IAssetTypeActions>> CreatedAssetTypeActions;
	EAssetTypeCategories::Type AssetCategoryBit = EAssetTypeCategories::Misc;
	TSharedPtr<class FUICommandList> PluginCommands;

	UWorld* World;

	AGraphToDungeonGenerator* Generator;

	UGraphToDungeonProperties* Properties;
};
