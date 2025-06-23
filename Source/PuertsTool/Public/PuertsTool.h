// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "../Toolbars/BlueprintToolbar.h"

class FPuertsToolModule : public IModuleInterface
{
public:

	void OnPostEngineInit();

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	TSharedRef<FExtender> GetExtender(UObject* InContextObject);

	void BuildToolbar(FToolBarBuilder& ToolbarBuilder, UObject* InContextObject);

	virtual void ShutdownModule() override;
	UBlueprint* Blueprint = nullptr;

public:
	void HandleButtonClick(UBlueprint* targetBlueprint, bool bForceOverwrite);

private:
	TSharedPtr<class IAssetTypeActions> TsMixinAssetActions;
	FString ProcessTemplate(const FString& TemplateContent, FString BlueprintPath, FString BPFileName, FString TSFileName);

	FToolBarExtensionDelegate ToolBarExtensionDelegate;
	FExecuteAction InitExecuteAction;
	TSharedPtr<FBlueprintToolbar> BlueprintToolbar;
	//-----------------------------------------
	TSharedPtr<FUICommandInfo> CreateLuaTemplate;
	TSharedPtr<FUICommandInfo> CopyAsRelativePath;
	TSharedPtr<FUICommandInfo> BindToLua;
	TSharedPtr<FUICommandInfo> UnbindFromLua;
	TSharedPtr<FUICommandInfo> HotReload;
	TSharedPtr<FUICommandInfo> OpenRuntimeSettings;
	TSharedPtr<FUICommandInfo> OpenEditorSettings;
	TSharedPtr<FUICommandInfo> ReportIssue;
	TSharedPtr<FUICommandInfo> About;
	TSharedPtr<FUICommandInfo> GenerateIntelliSense;
	TSharedPtr<FUICommandInfo> RevealInExplorer;
};
