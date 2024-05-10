// Copyright (c) 2024 Richard Pajersky.


#include "Factory_Properties.h"
#include "GraphToDungeonProperties.h"

#define LOCTEXT_NAMESPACE "GraphToDungeon"

UFactory_Properties::UFactory_Properties()
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UGraphToDungeonProperties::StaticClass();
}

UObject* UFactory_Properties::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UObject>(InParent, Class, Name, Flags | RF_Transactional);
}

FText UFactory_Properties::GetDisplayName() const
{
	return LOCTEXT("Factory Name", "Properties");
}

FString UFactory_Properties::GetDefaultNewAssetName() const
{
	return "Properties";
}

#undef LOCTEXT_NAMESPACE