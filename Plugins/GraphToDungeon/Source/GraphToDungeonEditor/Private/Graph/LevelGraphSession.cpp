// Copyright (c) 2024 Richard Pajersky.


#include "LevelGraphSession.h"
#include "LevelGraphNode.h"
#include "LevelGraphEdge.h"

#define LOCTEXT_NAMESPACE "LevelGraphSession"

ULevelGraphSession::ULevelGraphSession()
{
	NodeType = ULevelGraphNode::StaticClass();
	EdgeType = ULevelGraphEdge::StaticClass();

	LeftDialoguerBgColor = FLinearColor::Black;
	RightDialoguerBgColor = FLinearColor(0.93f, 0.93f, 0.93f, 1.f);

	Name = "LevelGraphSession";
}

#undef LOCTEXT_NAMESPACE
