
#pragma once

#include "Framework/Commands/Commands.h"

class FPuertsToolEditorCommands : public TCommands<FPuertsToolEditorCommands>
{
public:
    FPuertsToolEditorCommands()
        : TCommands<FPuertsToolEditorCommands>(TEXT("PuertsToolEditor"), NSLOCTEXT("Contexts", "PuertsToolEditor", "PuertsTool Editor"), NAME_None, "PuertsToolEditorStyle")
    {
    }

    virtual void RegisterCommands() override;

    TSharedPtr<FUICommandInfo> Puerts_Gen;
    TSharedPtr<FUICommandInfo> CreateTemplate;
    TSharedPtr<FUICommandInfo> CreateTemplate_ForceOverwrite;
    TSharedPtr<FUICommandInfo> RevealInExplorer;
};
