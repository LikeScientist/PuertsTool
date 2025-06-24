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
	
	void HandleButtonClick(UBlueprint* targetBlueprint, bool bForceOverwrite);

	//获取ts相关文件路径集合
	TArray<FString> GetCollectionOfFilePaths(UBlueprint* targetBlueprint);
public:
	UBlueprint* Blueprint = nullptr;

private:
	void OnPostEngineInit();
	TSharedPtr<class IAssetTypeActions> TsMixinAssetActions;
	
	FString ProcessTemplate(const FString& TemplateContent, FString BlueprintPath, FString BPFileName,FString TSFileName);

	FToolBarExtensionDelegate ToolBarExtensionDelegate;
	
	FExecuteAction InitExecuteAction;
	
	TSharedPtr<FBlueprintToolbar> BlueprintToolbar;
};
