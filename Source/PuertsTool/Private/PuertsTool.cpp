// Copyright Epic Games, Inc. All Rights Reserved.

#include "PuertsTool.h"
#include "AssetToolsModule.h"
#include "ContentBrowserModule.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFilemanager.h"
#include "BlueprintEditor.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "IContentBrowserSingleton.h"
#include <Kismet/GameplayStatics.h>
#include <GameMapsSettings.h>
#include "Kismet2/BlueprintEditorUtils.h"

#define LOCTEXT_NAMESPACE "FPuertsToolModule"

void FPuertsToolModule::StartupModule()
{
	FBlueprintEditorModule& BlueprintEditorModule = FModuleManager::LoadModuleChecked<FBlueprintEditorModule>("Kismet");
	TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());

	// 用于检测双击的静态变量
	static FDateTime LastClickTime;
	static bool bWaitingForDoubleClick = false;
	static FTimerHandle TimerHandle;

	MenuExtender->AddToolBarExtension(
		"Asset",
		EExtensionHook::After,
		nullptr,
		FToolBarExtensionDelegate::CreateLambda([this](FToolBarBuilder& ToolbarBuilder)
			{
				ToolbarBuilder.AddToolBarButton(
					FUIAction(
						FExecuteAction::CreateLambda([this]()
							{
								const FDateTime CurrentTime = FDateTime::Now();
								const FTimespan TimeSinceLastClick = CurrentTime - LastClickTime;
								LastClickTime = CurrentTime;

								// 如果是双击时间范围内
								if (TimeSinceLastClick.GetTotalSeconds() < 0.3f && bWaitingForDoubleClick)
								{
									// 取消等待单击的计时器
									GEditor->GetTimerManager()->ClearTimer(TimerHandle);
									bWaitingForDoubleClick = false;

									// 执行双击逻辑
									HandleButtonClick(true);
								}
								else
								{
									// 设置等待双击的计时器
									bWaitingForDoubleClick = true;
									GEditor->GetTimerManager()->SetTimer(
										TimerHandle,
										[this]()
										{
											if (bWaitingForDoubleClick)
											{
												bWaitingForDoubleClick = false;
												HandleButtonClick(false); // 执行单击逻辑
											}
										},
										0.3f, // 300毫秒双击检测窗口
										false
									);
								}
							})
					),
					NAME_None,
					LOCTEXT("GenerateTemplate", "生成TS模板"),
					LOCTEXT("GenerateTemplateTooltip", "单击生成(检查存在),双击强制覆盖"),
					FSlateIcon()
				);
			})
	);

	BlueprintEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
}

void FPuertsToolModule::HandleButtonClick(bool bForceOverwrite)
{
	UE_LOG(LogTemp, Warning, TEXT("HandleButtonClick==%s"), bForceOverwrite ? TEXT("true") : TEXT("false"));

	// 获取当前编辑的蓝图
	UBlueprint* Blueprint = nullptr;

	// 获取蓝图编辑器模块
	if (FBlueprintEditorModule* BlueprintEditorModule = FModuleManager::GetModulePtr<FBlueprintEditorModule>("Kismet"))
	{
		// 获取所有打开的蓝图编辑器（返回的是TSharedRef）
		const TArray<TSharedRef<IBlueprintEditor>>& BlueprintEditors = BlueprintEditorModule->GetBlueprintEditors();

		// 获取第一个活动的蓝图编辑器
		for (const TSharedRef<IBlueprintEditor>& Editor : BlueprintEditors)
		{
			// 检查编辑器是否有效
			/*if (Editor.IsValid())
			{*/
				// 获取编辑器中的蓝图对象
				if (FBlueprintEditor* BlueprintEditor = static_cast<FBlueprintEditor*>(&Editor.Get()))
				{
					Blueprint = BlueprintEditor->GetBlueprintObj();
					break;
				}
			//}
		}
	}

	if (Blueprint)
	{
		// 读取模板
		FString TemplatePath = FPaths::Combine(FPaths::ProjectPluginsDir(), TEXT("PuertsTool"), TEXT("Resources"), TEXT("TsMixinTemplate.ts"));
		FString TemplateContent;
		if (FFileHelper::LoadFileToString(TemplateContent, *TemplatePath))
		{
			FString BlueprintPath = Blueprint->GetPathName();
			FString LeftS, RightS;
			BlueprintPath.Split(".", &LeftS, &RightS);

			FString SubStr = "/Game/Blueprints/";
			if (LeftS.Contains(SubStr))
			{
				LeftS.Split(SubStr, &LeftS, &RightS);
			}
			else
			{
				LeftS.Split("/Game/", &LeftS, &RightS);
			}
			TArray<FString> stringArray;
			RightS.ParseIntoArray(stringArray, TEXT("/"), false);
			FString FileName = stringArray[stringArray.Num() - 1];

			RightS = RightS.Replace(TEXT("BP_"), TEXT("")).Replace(*FileName, *(TEXT("TS_") + FileName));

			// 处理模板
			FString ProcessedContent = ProcessTemplate(TemplateContent, Blueprint, BlueprintPath, FileName);

			// 保存文件
			FString TsFilePath = FPaths::Combine(FPaths::ProjectDir(), TEXT("TypeScript"), TEXT("Mixin"), RightS + FString(".ts"));

			// 检查文件是否存在且不是强制覆盖
			if (!bForceOverwrite && FPaths::FileExists(TsFilePath))
			{
				// 显示文件已存在通知
				FNotificationInfo Info(FText::Format(LOCTEXT("TsFileExists", "TS file already exists at: {0}\nDouble-click to overwrite."), FText::FromString(TsFilePath)));
				Info.ExpireDuration = 5.0f;
				FSlateNotificationManager::Get().AddNotification(Info);
				return;
			}

			if (FFileHelper::SaveStringToFile(ProcessedContent, *TsFilePath))
			{
				// 显示成功通知
				FNotificationInfo Info(FText::Format(
					bForceOverwrite ?
					LOCTEXT("TsFileOverwritten", "TS file overwritten at: {0}") :
					LOCTEXT("TsFileGenerated", "TS file generated at: {0}"),
					FText::FromString(TsFilePath)));
				Info.ExpireDuration = 5.0f;
				FSlateNotificationManager::Get().AddNotification(Info);
			}
		}
	}
	else
		UE_LOG(LogTemp, Warning, TEXT("No active blueprint editor found"));
		
}

FString FPuertsToolModule::ProcessTemplate(const FString& TemplateContent, UBlueprint* Blueprint, FString BlueprintPath, FString FileName)
{
	FString Result = TemplateContent;

	// 获取蓝图完整类名（包括_C后缀
	BlueprintPath += TEXT("_C");
	FString BlueprintClass = TEXT("UE") + BlueprintPath.Replace(TEXT("/"), TEXT("."));

	// 获取游戏实例类型 - 需要根据项目实际情况实现
	// 获取FSoftClassPath
	FSoftClassPath GameInstanceClassPath = GetMutableDefault<UGameMapsSettings>()->GameInstanceClass;
	// 转换为字符串路径（格式："/Game/Path/Asset.Asset_C"）
	FString GameInstanceType = GameInstanceClassPath.ToString();
	GameInstanceType = TEXT("UE") + GameInstanceType.Replace(TEXT("/"), TEXT("."));

	/*TSubclassOf<UObject>& Class =
	UKismetSystemLibrary::Conv_ClassToSoftClassReference(GameInstanceClass).ToString(); */

	// 替换占位符
	Result = Result.Replace(TEXT("%GAME_INSTANCE_TYPE%"), *GameInstanceType);
	Result = Result.Replace(TEXT("%BLUEPRINT_PATH%"), *BlueprintPath);
	Result = Result.Replace(TEXT("%MIXIN_BLUEPRINT_TYPE%"), *BlueprintClass);

	// 替换模板中的类名
	Result = Result.Replace(TEXT("%TS_CLASS_NAME%"), *FString::Printf(TEXT("TS_%s"), *FileName));
	Result = Result.Replace(TEXT("%BP_TO_JS%"), *FString::Printf(TEXT("BP_%s"), *FileName));

	return Result;
}

void FPuertsToolModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPuertsToolModule, PuertsTool)