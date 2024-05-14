// Copyright (c) 2024 Richard Pajersky.

#include "LevelGraphNode.h"
#include "LevelGraphSession.h"

#define LOCTEXT_NAMESPACE "LevelGraphSessionNode"

ULevelGraphNode::ULevelGraphNode()
{
#if WITH_EDITORONLY_DATA
    CompatibleGraphType = ULevelGraphSession::StaticClass();

    ContextMenuName = LOCTEXT("ConextMenuName", "Level Graph Node");
#endif
}

#if WITH_EDITOR

FText ULevelGraphNode::GetNodeTitle() const
{
    return Paragraph.IsEmpty() ? LOCTEXT("EmptyParagraph", "(Empty paragraph)") : Paragraph;
}

void ULevelGraphNode::SetNodeTitle(const FText& NewTitle)
{
    Paragraph = NewTitle;
}

FLinearColor ULevelGraphNode::GetBackgroundColor() const
{
    ULevelGraphSession* LevelGraph = Cast<ULevelGraphSession>(GetGraph());

    if (Graph == nullptr)
        return Super::GetBackgroundColor();

    switch (DialoguerPosition)
    {
    case EDialoguerPosition::Left:
        return LevelGraph->LeftDialoguerBgColor;
    case EDialoguerPosition::Right:
        return LevelGraph->RightDialoguerBgColor;
    default:
        return FLinearColor::Black;
    }
}

#endif

#undef LOCTEXT_NAMESPACE