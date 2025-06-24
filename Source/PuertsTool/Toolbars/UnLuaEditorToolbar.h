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

    void CreateLuaTemplate_Executed();

    void CopyAsRelativePath_Executed() const;

    void BindToLua_Executed() const;

    void UnbindFromLua_Executed() const;

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
