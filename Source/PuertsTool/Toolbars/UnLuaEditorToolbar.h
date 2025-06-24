#pragma once

#include "BlueprintEditor.h"
#include <PuertsTool.h>

class FPuertsToolEditorToolbar
{
public:
    virtual ~FPuertsToolEditorToolbar() = default;

    FPuertsToolEditorToolbar();

    TSharedRef<FUICommandList> GetCommandList() const
    {
        return CommandList;
    }

    virtual void Initialize();

    void Puerts_Gen_Executed() const;

    void CreateTemplate_ForceOverwrite_Executed();

    void CreateTemplate_Executed();    

    void RevealInExplorer_Executed();

protected:
    virtual void BindCommands();

    void BuildToolbar(FToolBarBuilder& ToolbarBuilder, UObject* InContextObject);

    void BuildNodeMenu();

    TSharedRef<FExtender> GetExtender(UObject* InContextObject);

    TSharedRef<FUICommandList> CommandList;

    UObject* ContextObject;

    class FPuertsToolModule* PuertsToolModule;
};
