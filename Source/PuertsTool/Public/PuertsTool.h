// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "../Toolbars/BlueprintToolbar.h"

class FPuertsToolModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	void OnPostEngineInit();

	TSharedRef<FExtender> GetExtender(UObject* InContextObject);

	void BuildToolbar(FToolBarBuilder& ToolbarBuilder, UObject* InContextObject);
	
	void HandleButtonClick(UBlueprint* targetBlueprint, bool bForceOverwrite);

public:
	UBlueprint* Blueprint = nullptr;

private:
	TSharedPtr<class IAssetTypeActions> TsMixinAssetActions;
	
	FString ProcessTemplate(const FString& TemplateContent, FString BlueprintPath, FString BPFileName,FString TSFileName);

	FToolBarExtensionDelegate ToolBarExtensionDelegate;
	
	FExecuteAction InitExecuteAction;
	
	TSharedPtr<FBlueprintToolbar> BlueprintToolbar;
};
