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
#include "../Toolbars/UnLuaEditorCommands.h"
#include "../Toolbars/UnLuaEditorToolbar.h"
//#include "Kismet/GameplayStatics.h"

#define LOCTEXT_NAMESPACE "FPuertsToolModule"

void FPuertsToolModule::OnPostEngineInit()
{
	BlueprintToolbar.Get()->Initialize();

	FBlueprintEditorModule& BlueprintEditorModule = FModuleManager::LoadModuleChecked<FBlueprintEditorModule>("Kismet");
	TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());

	MenuExtender->AddToolBarExtension(
		"Asset",
		EExtensionHook::After,
		nullptr,
		FToolBarExtensionDelegate::CreateLambda([this](FToolBarBuilder& ToolbarBuilder)
			{
				ToolbarBuilder.AddToolBarButton(
					FUIAction(
						InitExecuteAction
					),
					NAME_None,
					LOCTEXT("GenerateTemplate", "模版生成蓝图Mixin.ts"),
					LOCTEXT("GenerateTemplateTooltip", "单击生成(文件存在不覆盖),双击强制覆盖+生成ts定义"),
					FSlateIcon()
				);
			}));


	//BlueprintEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
	//auto& BlueprintEditorModule = FModuleManager::LoadModuleChecked<FBlueprintEditorModule>("Kismet");
	auto& ExtenderDelegates = BlueprintEditorModule.GetMenuExtensibilityManager()->GetExtenderDelegates();

	ExtenderDelegates.Add(FAssetEditorExtender::CreateLambda(
		[this, MenuExtender](const TSharedRef<FUICommandList>, const TArray<UObject*> ContextSensitiveObjects)
		{
			this->Blueprint = ContextSensitiveObjects.Num() < 1 ? nullptr : Cast<UBlueprint>(ContextSensitiveObjects[0]);
			//GetExtender(this->Blueprint);
			if (this->Blueprint)
			{
				UE_LOG(LogTemp, Warning, TEXT("Open BlueprintPath=%s"), *this->Blueprint->GetPathName());
				UBlueprint* localBlueprint = this->Blueprint;
				// 用于检测双击的静态变量
				static FDateTime LastClickTime;
				static bool bWaitingForDoubleClick = false;
				static FTimerHandle TimerHandle;

				InitExecuteAction = FExecuteAction::CreateLambda([this, localBlueprint]()
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
							HandleButtonClick(localBlueprint, true);
						}
						else
						{
							// 设置等待双击的计时器
							bWaitingForDoubleClick = true;
							GEditor->GetTimerManager()->SetTimer(
								TimerHandle,
								[this, localBlueprint]()
								{
									if (bWaitingForDoubleClick)
									{
										bWaitingForDoubleClick = false;
										HandleButtonClick(localBlueprint, false); // 执行单击逻辑
									}
								},
								0.3f, // 300毫秒双击检测窗口
								false
							);
						}
					});
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Open the blueprint to obtain the object exception"));
			}

			return MenuExtender.ToSharedRef();
		}
	));
}

void FPuertsToolModule::StartupModule()
{
	FPuertsToolEditorCommands::Register();

	BlueprintToolbar = MakeShareable(new FBlueprintToolbar);
	FCoreDelegates::OnPostEngineInit.AddRaw(this, &FPuertsToolModule::OnPostEngineInit);		
}

void FPuertsToolModule::HandleButtonClick(UBlueprint* targetBlueprint, bool bForceOverwrite)
{
	UE_LOG(LogTemp, Warning, TEXT("HandleButtonClick==%s"), bForceOverwrite ? TEXT("true") : TEXT("false"));

	if (targetBlueprint)
	{
		// 读取模板
		FString TemplatePath = FPaths::Combine(FPaths::ProjectPluginsDir(), TEXT("PuertsTool"), TEXT("Resources"), TEXT("TsMixinTemplate.ts"));
		FString TemplateContent;
		if (FFileHelper::LoadFileToString(TemplateContent, *TemplatePath))
		{
			FString BlueprintPath = targetBlueprint->GetPathName();
			FString LeftS, RightS;
			BlueprintPath.Split(".", &LeftS, &RightS);
			FString BPFileName = RightS;//得到蓝图文件名

			FString SubStr = "/Game/Blueprints/";
			if (LeftS.Contains(SubStr))
			{
				LeftS.Split(SubStr, &LeftS, &RightS);
			}
			else
			{
				SubStr = "/Game/";
				if (LeftS.Contains(SubStr))
				{
					LeftS.Split(SubStr, &LeftS, &RightS);
				}
				else//主要针对插件目录下的文件，基本不动路径
				{
					RightS = LeftS;
				}
			}

			////FString BPFileName换到上面获取了
			//TArray<FString> stringArray;
			//RightS.ParseIntoArray(stringArray, TEXT("/"), false);
			//FString BPFileName = stringArray[stringArray.Num() - 1];

			////去掉BP_的蓝图前缀
			//BPFileName = BPFileName.Replace(TEXT("BP_"), TEXT(""));
			//TCHAR From = TEXT("BP_") + (*BPFileName);
			//RightS = RightS.Replace(&From, *BPFileName);

			FString TSFileName = TEXT("Mixin_") + BPFileName;
			//ts文件增加TS_前缀
			RightS = RightS.Replace(*BPFileName, *TSFileName);


			// 处理模板
			FString ProcessedContent = ProcessTemplate(TemplateContent, BlueprintPath, BPFileName, TSFileName);


			if (bForceOverwrite)
			{
				GEngine->Exec(GWorld, TEXT("Puerts.Gen"));//调用Puerts导出蓝图定义
			}

			// 保存文件
			FString TsFilePath = FPaths::Combine(FPaths::ProjectDir(), TEXT("TypeScript"), TEXT("Mixin"), RightS + FString(".ts"));
			FText Text;
			// 检查文件是否存在且不是强制覆盖
			if (!bForceOverwrite && FPaths::FileExists(TsFilePath))
			{
				// 显示文件已存在通知
				Text = FText::Format(LOCTEXT("TsFileExists", "TS file already exists at: {0}\nDouble-click to overwrite."), FText::FromString(TsFilePath));
			}

			if (FFileHelper::SaveStringToFile(ProcessedContent, *TsFilePath))
			{
				// 显示成功通知
				Text = FText::Format(
					bForceOverwrite ?
					LOCTEXT("TsFileOverwritten", "TS file overwritten at: {0}") :
					LOCTEXT("TsFileGenerated", "TS file generated at: {0}"),
					FText::FromString(TsFilePath));
			}

			FNotificationInfo Info(Text);
			Info.ExpireDuration = 5.0f;
			FSlateNotificationManager::Get().AddNotification(Info);
			UE_LOG(LogTemp, Log, TEXT("%s"), *(Text.ToString()));
		}
	}
	else
		UE_LOG(LogTemp, Warning, TEXT("No active blueprint editor found"));

}

FString FPuertsToolModule::ProcessTemplate(const FString& TemplateContent, FString BlueprintPath, FString BPFileName, FString TSFileName)
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
	Result = Result.Replace(TEXT("%TS_CLASS_NAME%"), *TSFileName);
	Result = Result.Replace(TEXT("%BP_TO_JS%"), *FString::Printf(TEXT("BP_%s"), *BPFileName));

	return Result;
}

void FPuertsToolModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPuertsToolModule, PuertsTool)
