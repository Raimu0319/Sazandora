// Fill out your copyright notice in the Description page of Project Settings.


#include "NetWork/Login_HUD.h"
#include "Blueprint/UserWidget.h"

void ALogin_HUD::BeginPlay()
{
	Super::BeginPlay();

	//自分の画面（ローカル）にだけウィジェットを表示
	APlayerController* PlayerController = GetOwningPlayerController();

	

	if (PlayerController != nullptr)
	{
		LoginWidgetClass = LoadClass<UUserWidget>(nullptr, TEXT("/Game/Login_UI/Login_Widget.Login_Widget_C"));
		
		if (LoginWidgetClass != nullptr)
		{
			LoginWidget = CreateWidget<UUserWidget>(PlayerController, LoginWidgetClass);
			LoginWidget->AddToViewport();
			UE_LOG(LogTemp, Warning, TEXT("PlayerControllerNONE"));
			//マウスカーソルを表示
			PlayerController->bShowMouseCursor = true;
			PlayerController->SetInputMode(FInputModeUIOnly());

		}
		
	}
	
}
