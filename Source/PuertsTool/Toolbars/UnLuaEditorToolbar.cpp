#include "UnLuaEditorToolbar.h"
#include "Misc/EngineVersionComparison.h"
#include "UnLuaEditorToolbar.h"
#include "UnLuaEditorCommands.h"
#include "Animation/AnimInstance.h"
#include "Blueprint/UserWidget.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Interfaces/IPluginManager.h"
#include "BlueprintEditor.h"
#include "SBlueprintEditorToolbar.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Layout/Children.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "ToolMenus.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"

#define LOCTEXT_NAMESPACE "FPuertsToolEditorModule"

FPuertsToolEditorToolbar::FPuertsToolEditorToolbar()
    : CommandList(new FUICommandList),
      ContextObject(nullptr)
{
}

void FPuertsToolEditorToolbar::Initialize()
{
    CommandList = MakeShared<FUICommandList>();
    ContextObject = nullptr;
    BindCommands();
    this->PuertsToolModule = static_cast<FPuertsToolModule*>(FModuleManager::Get().GetModule("PuertsToolModule"));
}

void FPuertsToolEditorToolbar::BindCommands()
{
    const auto& Commands = FPuertsToolEditorCommands::Get();
    CommandList->MapAction(Commands.Puerts_Gen, FExecuteAction::CreateRaw(this, &FPuertsToolEditorToolbar::Puerts_Gen_Executed));
    CommandList->MapAction(Commands.CreateTemplate, FExecuteAction::CreateRaw(this, &FPuertsToolEditorToolbar::CreateTemplate_Executed));    
    CommandList->MapAction(Commands.CreateTemplate_ForceOverwrite, FExecuteAction::CreateRaw(this, &FPuertsToolEditorToolbar::CreateTemplate_ForceOverwrite_Executed));
    CommandList->MapAction(Commands.RevealInExplorer, FExecuteAction::CreateRaw(this, &FPuertsToolEditorToolbar::RevealInExplorer_Executed));
}

void FPuertsToolEditorToolbar::BuildToolbar(FToolBarBuilder& ToolbarBuilder, UObject* InContextObject)
{
    if (!InContextObject)
        return;

    ToolbarBuilder.BeginSection(NAME_None);

    const auto Blueprint = Cast<UBlueprint>(InContextObject);
    ToolbarBuilder.AddComboButton(
        FUIAction(),
        FOnGetContent::CreateLambda([&, Blueprint, InContextObject]()
        {
            ContextObject = InContextObject;
            const FPuertsToolEditorCommands& Commands = FPuertsToolEditorCommands::Get();
            FMenuBuilder MenuBuilder(true, CommandList);
            MenuBuilder.AddMenuEntry(Commands.Puerts_Gen, NAME_None, LOCTEXT("Puerts_Gen", "Generate Puerts Definitions"));
            MenuBuilder.AddMenuEntry(Commands.RevealInExplorer, NAME_None, LOCTEXT("RevealInExplorer", "Reveal in Explorer"));
            MenuBuilder.AddMenuEntry(Commands.CreateTemplate, NAME_None, LOCTEXT("CreateTemplate", "Create TS Template"));
            MenuBuilder.AddMenuEntry(Commands.CreateTemplate_ForceOverwrite, NAME_None, LOCTEXT("CreateTemplate_ForceOverwrite", "Create TS Template Force Overwrite"));
            return MenuBuilder.MakeWidget();
        }),
        LOCTEXT("PuertsTool_Label", "PuertsTool"),
        LOCTEXT("PuertsTool_ToolTip", "PuertsTool"),
        TAttribute<FSlateIcon>::Create([Blueprint]
        {            
            FString InStyleName;            
            return FSlateIcon("PuertsToolEditorStyle", *InStyleName);
        })
    );

    ToolbarBuilder.EndSection();

    BuildNodeMenu();
}

void FPuertsToolEditorToolbar::BuildNodeMenu()
{
    FToolMenuOwnerScoped OwnerScoped(this);
    UToolMenu* BPMenu = UToolMenus::Get()->ExtendMenu("GraphEditor.GraphNodeContextMenu.K2Node_FunctionResult");
    BPMenu->AddDynamicSection("PuertsTool", FNewToolMenuDelegate::CreateLambda([this](UToolMenu* ToolMenu)
    {
        UGraphNodeContextMenuContext* GraphNodeCtx = ToolMenu->FindContext<UGraphNodeContextMenuContext>();
        if (GraphNodeCtx && GraphNodeCtx->Graph)
        {
            if (GraphNodeCtx->Graph->GetName() == "GetModuleName")
            {
                FToolMenuSection& PuertsToolSection = ToolMenu->AddSection("PuertsTool", FText::FromString("PuertsTool"));
                PuertsToolSection.AddEntry(FToolMenuEntry::InitMenuEntryWithCommandList(FPuertsToolEditorCommands::Get().RevealInExplorer, CommandList, LOCTEXT("RevealInExplorer", "Reveal in Explorer")));
            }
        }
    }), FToolMenuInsert(NAME_None, EToolMenuInsertType::First));
}

TSharedRef<FExtender> FPuertsToolEditorToolbar::GetExtender(UObject* InContextObject)
{
    TSharedRef<FExtender> ToolbarExtender(new FExtender());
    const auto ExtensionDelegate = FToolBarExtensionDelegate::CreateLambda([this, InContextObject](FToolBarBuilder& ToolbarBuilder)
    {
        BuildToolbar(ToolbarBuilder, InContextObject);
    });

    ToolbarExtender->AddToolBarExtension("Debugging", EExtensionHook::After, CommandList, ExtensionDelegate);
    
    return ToolbarExtender;
}

void FPuertsToolEditorToolbar::CreateTemplate_Executed()
{
    const auto Blueprint = Cast<UBlueprint>(ContextObject);
    if (!IsValid(Blueprint))
        return;

    PuertsToolModule->HandleButtonClick(Blueprint, false);
}

void FPuertsToolEditorToolbar::CreateTemplate_ForceOverwrite_Executed()
{
    const auto Blueprint = Cast<UBlueprint>(ContextObject);
    if (!IsValid(Blueprint))
        return;

    PuertsToolModule->HandleButtonClick(Blueprint, true);
}

void FPuertsToolEditorToolbar::RevealInExplorer_Executed()
{
    const auto Blueprint = Cast<UBlueprint>(ContextObject);
    if (!IsValid(Blueprint))
        return;

    const auto TargetClass = Blueprint->GeneratedClass;
    if (!IsValid(TargetClass))
        return;

    const auto FileName = PuertsToolModule->GetCollectionOfFilePaths(Blueprint)[0];

    if (IFileManager::Get().FileExists(*FileName))
    {
        FPlatformProcess::ExploreFolder(*FileName);
    }
    else
    {
        FNotificationInfo NotificationInfo(FText::FromString("PuertsTool Notification"));
        NotificationInfo.Text = LOCTEXT("FileNotExist", "The file does not exist.");
        NotificationInfo.bFireAndForget = true;
        NotificationInfo.ExpireDuration = 100.0f;
        NotificationInfo.bUseThrobber = true;
        FSlateNotificationManager::Get().AddNotification(NotificationInfo);
    }
}

void FPuertsToolEditorToolbar::Puerts_Gen_Executed() const
{
    GEngine->Exec(GWorld, TEXT("Puerts.Gen"));//调用Puerts导出蓝图定义	
}

#undef LOCTEXT_NAMESPACE
