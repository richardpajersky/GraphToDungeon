// Copyright (c) 2024 Richard Pajersky.


#include "Factory_LevelGraph.h"
#include "LevelGraphSession.h"

#define LOCTEXT_NAMESPACE "Factory_LevelGraph"

UFactory_LevelGraph::UFactory_LevelGraph()
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = ULevelGraphSession::StaticClass();
}

UObject* UFactory_LevelGraph::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UObject>(InParent, Class, Name, Flags | RF_Transactional);
}

FText UFactory_LevelGraph::GetDisplayName() const
{
	return LOCTEXT("Factory Name", "Graph");
}

FString UFactory_LevelGraph::GetDefaultNewAssetName() const
{
	return "Graph";
}

#undef LOCTEXT_NAMESPACE
