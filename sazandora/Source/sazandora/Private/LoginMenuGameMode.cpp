// Fill out your copyright notice in the Description page of Project Settings.


#include "LoginMenuGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "UObject/ConstructorHelpers.h"
#include "../public/NetWork/Login_HUD.h"

void ALoginMenuGameMode::BeginPlay()
{
	Super::BeginPlay();

	//APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	//if (PlayerController != nullptr)
	//{
	//	//マウスカーソル表示
	//	PlayerController->bShowMouseCursor = true;
	//	PlayerController->bEnableClickEvents = true;
	//	PlayerController->bEnableMouseOverEvents = true;

	//	// 入力モードをUI専用に設定
	//	FInputModeUIOnly InputMode;
	//	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	//	InputMode.SetWidgetToFocus(nullptr); // フォーカスするWidgetがあれば指定
	//	PlayerController->SetInputMode(InputMode);

	//	UE_LOG(LogTemp, Warning, TEXT("MouseInputSetting"));
	//}

	if (DefaultPawnClass != nullptr)
	{
		DefaultPawnClass = nullptr;
		UE_LOG(LogTemp, Warning, TEXT("DefaultPawnClassSetting_NULL"));
	}

	//UClass* HUD = ALogin_HUD::StaticClass();

	//if (HUD != nullptr)
	//{
	//	HUDClass = HUD;
	//	UE_LOG(LogTemp, Warning, TEXT("DefaultHUDSettingOK"));
	//}
	//else
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("DefaultHUDSettingNO"));
	//}

	/*TSubclassOf<UUserWidget> WidgetClass = LoadClass<UUserWidget>(nullptr, TEXT("/Game/Login_UI/Login_Widget.Login_Widget_C"));
	
	if (WidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("WidgetLoadOK"));
		UUserWidget* Widget = CreateWidget<UUserWidget>(GetWorld(), WidgetClass);

		if (Widget)
		{
			Widget->AddToViewport();
			UE_LOG(LogTemp, Warning, TEXT("WidgetCreateOK"));
		}
	}*/
}