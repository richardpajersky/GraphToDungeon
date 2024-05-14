// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright (c) 2024 Richard Pajersky.

#include "GraphToDungeonEditor.h"
#include "GraphToDungeonStyle.h"
#include "GraphToDungeonCommands.h"
#include "GraphToDungeonProperties.h"
#include "GraphToDungeonGenerator.h"

#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "ToolMenus.h"
#include "PropertyCustomizationHelpers.h"
#include "PropertyHandle.h"
#include "AssetTypeAction_Properties.h"
#include "AssetTypeAction_Theme.h"
#include "AssetTypeAction_Graph.h"

static const FName GraphToDungeonTabName("GraphToDungeon");

#define LOCTEXT_NAMESPACE "FGraphToDungeonModule"

void FGraphToDungeonModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FGraphToDungeonStyle::Initialize();
	FGraphToDungeonStyle::ReloadTextures();

	FGraphToDungeonCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FGraphToDungeonCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FGraphToDungeonModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FGraphToDungeonModule::RegisterMenus));
	
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(GraphToDungeonTabName, FOnSpawnTab::CreateRaw(this, &FGraphToDungeonModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FGraphToDungeonTabTitle", "Graph to Dungeon"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);

	// Register asset actions for assets
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	AssetCategoryBit = AssetTools.RegisterAdvancedAssetCategory(
		FName(TEXT("Graph to Dungeon")), LOCTEXT("GraphToDungeonCategory", "Graph to Dungeon"));
	RegisterAssetTypeAction(AssetTools, MakeShareable(
		new FAssetTypeAction_Properties(AssetCategoryBit)));
	RegisterAssetTypeAction(AssetTools, MakeShareable(
		new FAssetTypeAction_Theme(AssetCategoryBit)));
	//RegisterAssetTypeAction(AssetTools, MakeShareable(
	//	new FAssetTypeAction_Graph(AssetCategoryBit)));

	AssetThumbnailPool = MakeShareable(new FAssetThumbnailPool(24));
}

void FGraphToDungeonModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FGraphToDungeonStyle::Shutdown();

	FGraphToDungeonCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(GraphToDungeonTabName);
}

void FGraphToDungeonModule::GenerateDungeon()
{
	if (!Properties->GlobalLevelTheme->AreMeshesDefined())
	{
		InfoTextBlock->SetText(FText::FromString(TEXT("Not all meshes inside global theme are defined.")));
		return;
	}
	int32 Retries = 0;
	do
	{
		if (Properties->bUseRandomThemeSeed)
		{
			Properties->RandomStream.GenerateNewSeed();
			Properties->ThemeSeed = Properties->RandomStream.GetCurrentSeed();
		}
		else
		{
			Properties->RandomStream = FRandomStream(Properties->ThemeSeed);
		}
		Retries++;
	} while (!Generator->Generate(Properties) && Retries < Properties->MaxGenerationRetries);
	if (Retries == Properties->MaxGenerationRetries)
	{
		InfoTextBlock->SetText(FText::FromString(TEXT("Generation unsuccessful try again or simplify graph.")));
	}
	else 
	{
		InfoTextBlock->SetText(FText::FormatOrdered(FText::FromString(TEXT("Generation successful in {0} retries.")), Retries));
	}
}

FReply FGraphToDungeonModule::OnGenerateNewLevelButtonClicked()
{
	World = GEngine->GetWorldContextFromGameViewport(GEngine->GameViewport)->World();
	if (Generator) Generator->OnGeneratorDeleted.Unbind();
	Generator = (AGraphToDungeonGenerator*)World->SpawnActor(AGraphToDungeonGenerator::StaticClass());
	Generator->OnGeneratorDeleted.BindRaw(this, &FGraphToDungeonModule::HandleGeneratorDeleted);

	UpdateButtonsStatus();

	GenerateDungeon();

	return FReply::Handled();
}

FReply FGraphToDungeonModule::OnDeleteLevelButtonClicked()
{
	if (Generator) Generator->Destroy();
	return FReply::Handled();
}

FReply FGraphToDungeonModule::OnRegenerateLevelButtonClicked()
{
	if (Generator) Generator->Destroy();
	return OnGenerateNewLevelButtonClicked();
}

FReply FGraphToDungeonModule::OnRegenerateThemeButtonClicked()
{
	if (Properties->bUseRandomThemeSeed)
	{
		Properties->RandomStream.GenerateNewSeed();
		Properties->ThemeSeed = Properties->RandomStream.GetCurrentSeed();
	}
	else
	{
		Properties->RandomStream = FRandomStream(Properties->ThemeSeed);
	}
	Generator->RegenerateTheme();
	return FReply::Handled();
}

void FGraphToDungeonModule::HandleGeneratorDeleted()
{
	Generator = nullptr;
	UpdateButtonsStatus();
}

bool FGraphToDungeonModule::IsPropertiesDefined() const
{
	if (!Properties)
	{
		InfoTextBlock->SetText(FText::FromString(TEXT("Properties are not defined.")));
		return false;
	}
	if (!Properties->LevelGraph)
	{
		InfoTextBlock->SetText(FText::FromString(TEXT("Level Graph inside properties is not defined.")));
		return false;
	}
	if (!Properties->GlobalLevelTheme)
	{
		InfoTextBlock->SetText(FText::FromString(TEXT("Global Level Theme inside properties is not defined.")));
		return false;
	}
	InfoTextBlock->SetText(FText::FromString(TEXT("")));
	return true;
}

bool FGraphToDungeonModule::IsGeneratorSpawned() const
{
	return Generator &&	World == GEngine->GetWorldContextFromGameViewport(GEngine->GameViewport)->World();
}

void FGraphToDungeonModule::UpdateButtonsStatus() const
{
	GenerateNewLevelButton->SetEnabled(IsPropertiesDefined());
	RegenerateLevelButton->SetEnabled(IsPropertiesDefined() && IsGeneratorSpawned());
	RegenerateThemeButton->SetEnabled(IsPropertiesDefined() && IsGeneratorSpawned());
	DeleteLevelButton->SetEnabled(IsPropertiesDefined() && IsGeneratorSpawned());
}

TSharedRef<SDockTab> FGraphToDungeonModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	// Main window tab
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("Border"))
				.Padding(20.0f)
				[
					SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 0.0f, 0.0f, 4.0f)
						[
							SNew(STextBlock)
								.Text(FText::FromString("Graph to Dungeon Generation"))
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.VAlign(VAlign_Center)
						.Padding(8.0f, 8.0f, 0.0f, 0.0f)
						[
							SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								.AutoWidth()
								.VAlign(VAlign_Center)
								.Padding(0.0f, 0.0f, 8.0f, 0.0f)
								[
									SNew(STextBlock)
										.Text(FText::FromString("Generation Properties"))
								]
								+ SHorizontalBox::Slot()
								.AutoWidth()
								.VAlign(VAlign_Top)
								[
									SNew(SObjectPropertyEntryBox)
										.AllowedClass(UGraphToDungeonProperties::StaticClass())
										.ObjectPath_Raw(this,
											&FGraphToDungeonModule::GetPropertiesPath)
										.OnObjectChanged_Raw(this,
											&FGraphToDungeonModule::OnPropertiesChanged)
										.ThumbnailPool(AssetThumbnailPool)
								]
								+ SHorizontalBox::Slot()
								.AutoWidth()
								.VAlign(VAlign_Center)
								.Padding(8.0f, 0.0f, 8.0f, 0.0f)
								[
									SAssignNew(InfoTextBlock, STextBlock)
										.Text(FText::FromString(""))
										.ColorAndOpacity(FColor(255, 220, 0))
								]
						]
						// Buttons
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(8.0f, 8.0f, 0.0f, 0.0f)
						[
							SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								.VAlign(VAlign_Top)
								[
									SAssignNew(GenerateNewLevelButton, SButton)
										.Text(FText::FromString("New Dungeon"))
										.HAlign(HAlign_Center)
										.IsEnabled(IsPropertiesDefined())
										.OnClicked_Raw(this,
											&FGraphToDungeonModule
											::OnGenerateNewLevelButtonClicked)
								]
								+ SHorizontalBox::Slot()
								.VAlign(VAlign_Top)
								[
									SAssignNew(RegenerateLevelButton, SButton)
										.Text(FText::FromString("Regenerate Dungeon"))
										.HAlign(HAlign_Center)
										.IsEnabled(IsGeneratorSpawned())
										.OnClicked_Raw(this,
											&FGraphToDungeonModule::OnRegenerateLevelButtonClicked)
								]
								+ SHorizontalBox::Slot()
								.VAlign(VAlign_Top)
								[
									SAssignNew(RegenerateThemeButton, SButton)
										.Text(FText::FromString("Regenerate Theme"))
										.HAlign(HAlign_Center)
										.IsEnabled(IsGeneratorSpawned())
										.OnClicked_Raw(this,
											&FGraphToDungeonModule::OnRegenerateThemeButtonClicked)
								]
								+ SHorizontalBox::Slot()
								.VAlign(VAlign_Top)
								[
									SAssignNew(DeleteLevelButton, SButton)
										.Text(FText::FromString("Delete Dungeon"))
										.HAlign(HAlign_Center)
										.IsEnabled(IsGeneratorSpawned())
										.OnClicked_Raw(this,
											&FGraphToDungeonModule::OnDeleteLevelButtonClicked)
								]
						]
				]
		];
}

void FGraphToDungeonModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(GraphToDungeonTabName);
}

void FGraphToDungeonModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FGraphToDungeonCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FGraphToDungeonCommands::Get().OpenPluginWindow));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

void FGraphToDungeonModule::RegisterAssetTypeAction(IAssetTools& AssetTools, TSharedRef<IAssetTypeActions> Action)
{
	AssetTools.RegisterAssetTypeActions(Action);
	CreatedAssetTypeActions.Add(Action);
}

FString FGraphToDungeonModule::GetPropertiesPath() const
{
	return Properties ? Properties->GetPathName() : FString("");
}

void FGraphToDungeonModule::OnPropertiesChanged(const FAssetData& AssetData)
{
	if (Properties) Properties->OnPropertiesEdited.Unbind();
	Properties = Cast<UGraphToDungeonProperties>(AssetData.GetAsset());
	if (Properties) Properties->OnPropertiesEdited.BindRaw(this, &FGraphToDungeonModule::UpdateButtonsStatus);

	UpdateButtonsStatus();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FGraphToDungeonModule, PLGPlugin)