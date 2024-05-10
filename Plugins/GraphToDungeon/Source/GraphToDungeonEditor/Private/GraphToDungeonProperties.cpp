// Copyright (c) 2024 Richard Pajersky.


#include "GraphToDungeonProperties.h"

void UGraphToDungeonProperties::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	OnPropertiesEdited.ExecuteIfBound();
}
