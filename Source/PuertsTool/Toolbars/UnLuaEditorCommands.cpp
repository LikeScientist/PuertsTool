#include "UnLuaEditorCommands.h"

#define LOCTEXT_NAMESPACE "FPuertsToolEditorCommands"

void FPuertsToolEditorCommands::RegisterCommands()
{
    UI_COMMAND(CreateTemplate, "Create TS Template", "Create ts template file", EUserInterfaceActionType::Button, FInputChord());
    UI_COMMAND(CreateTemplate_ForceOverwrite, "Create TS Template Force Overwrite", "Create ts template file", EUserInterfaceActionType::Button, FInputChord());
    UI_COMMAND(Puerts_Gen, "Generate Puerts Definitions", "Copy module name as relative path.", EUserInterfaceActionType::Button, FInputChord());    
    UI_COMMAND(RevealInExplorer, "Reveal in Explorer", "Reveal ts file in explorer", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
