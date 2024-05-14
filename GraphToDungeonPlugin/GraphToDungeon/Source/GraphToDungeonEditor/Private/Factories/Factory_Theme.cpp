// Copyright (c) 2024 Richard Pajersky.


#include "Factory_Theme.h"
#include "GraphToDungeonTheme.h"

#define LOCTEXT_NAMESPACE "Factory_Theme"

UFactory_Theme::UFactory_Theme()
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UGraphToDungeonTheme::StaticClass();
}

UObject* UFactory_Theme::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UObject>(InParent, Class, Name, Flags | RF_Transactional);
}

FText UFactory_Theme::GetDisplayName() const
{
	return LOCTEXT("Factory Name", "Theme");
}

FString UFactory_Theme::GetDefaultNewAssetName() const
{
	return "Theme";
}

#undef LOCTEXT_NAMESPACE
