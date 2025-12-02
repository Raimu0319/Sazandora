// Fill out your copyright notice in the Description page of Project Settings.


#include "LoginMenuGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "UObject/ConstructorHelpers.h"
#include "../public/NetWork/Login_HUD.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

ALoginMenuGameMode::ALoginMenuGameMode()
{
	UE_LOG(LogTemp, Warning, TEXT("GameMode::LoginMenuGameMode"));

	DefaultPawnClass = nullptr;

	if (!IsRunningDedicatedServer())
	{
		UClass* HUD = ALogin_HUD::StaticClass();

		if (HUD != nullptr)
		{
			HUDClass = HUD;
			UE_LOG(LogTemp, Warning, TEXT("DefaultHUDSettingOK"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("DefaultHUDSettingNO"));
		}
	}
}

void ALoginMenuGameMode::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	if (PlayerController != nullptr)
	{
		//マウスカーソル表示
		PlayerController->bShowMouseCursor = true;
		PlayerController->bEnableClickEvents = true;
		PlayerController->bEnableMouseOverEvents = true;

		// 入力モードをUI専用に設定
		FInputModeUIOnly InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		InputMode.SetWidgetToFocus(nullptr); // フォーカスするWidgetがあれば指定
		//PlayerController->SetInputMode(InputMode);

		UE_LOG(LogTemp, Warning, TEXT("MouseInputSetting"));
	}

}

void ALoginMenuGameMode::GetServerListFromMaster()
{
	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL("http://127.0.0.1:3000/list");
	Request->SetVerb("GET");
	Request->SetHeader("Content-Type", "application/json");

	Request->OnProcessRequestComplete().BindLambda([](FHttpRequestPtr Req, FHttpResponsePtr Response, bool bWasSuccessful)
	{
		if (bWasSuccessful)
		{
			FString ResponseString = Response->GetContentAsString();
			UE_LOG(LogTemp, Log, TEXT("サーバー一覧: %s"), *ResponseString);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("サーバー一覧取得失敗"));
		}
	});

	Request->ProcessRequest();
}