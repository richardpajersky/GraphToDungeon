// Copyright Epic Games, Inc. All Rights Reserved.

#include "GraphToDungeonStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Framework/Application/SlateApplication.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FGraphToDungeonStyle::StyleInstance = nullptr;

void FGraphToDungeonStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FGraphToDungeonStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FGraphToDungeonStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("GraphToDungeonStyle"));
	return StyleSetName;
}

const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);

TSharedRef< FSlateStyleSet > FGraphToDungeonStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("GraphToDungeonStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("GraphToDungeon")->GetBaseDir() / TEXT("Resources"));

	Style->Set("GraphToDungeon.OpenPluginWindow", new IMAGE_BRUSH_SVG(TEXT("GTDIcon"), Icon20x20));

	return Style;
}

void FGraphToDungeonStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FGraphToDungeonStyle::Get()
{
	return *StyleInstance;
}
