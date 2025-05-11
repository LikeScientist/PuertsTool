// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FPuertsToolModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;	
	virtual void ShutdownModule() override;
	UBlueprint* Blueprint = nullptr;

private:
	TSharedPtr<class IAssetTypeActions> TsMixinAssetActions;
	void HandleButtonClick(bool bForceOverwrite);
	FString ProcessTemplate(const FString& TemplateContent, FString BlueprintPath, FString BPFileName,FString TSFileName);
};
