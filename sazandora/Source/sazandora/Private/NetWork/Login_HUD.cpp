// Fill out your copyright notice in the Description page of Project Settings.


#include "NetWork/Login_HUD.h"
#include "Blueprint/UserWidget.h"

ALogin_HUD::ALogin_HUD()
{
	static ConstructorHelpers::FClassFinder<UUserWidget> WidgetBP(TEXT("/Game/Login_UI/Login_Widget.Login_Widget_C"));
	if (WidgetBP.Succeeded())
	{
		LoginWidgetClass = WidgetBP.Class;
	}
}

void ALogin_HUD::BeginPlay()
{
	Super::BeginPlay();

	//自分の画面（ローカル）にだけウィジェットを表示
	APlayerController* PlayerController = GetOwningPlayerController();
	//LoginWidgetClass = LoadClass<UUserWidget>(nullptr, TEXT("/Game/Login_UI/Login_Widget.Login_Widget_C"));

	if (PlayerController && PlayerController->IsLocalController())
	{
		if (LoginWidgetClass != nullptr)
		{
			LoginWidget = CreateWidget<UUserWidget>(PlayerController, LoginWidgetClass);
			LoginWidget->AddToViewport();
			UE_LOG(LogTemp, Warning, TEXT("LoginWidgetClassOK"));
			//マウスカーソルを表示
			PlayerController->bShowMouseCursor = true;
			//PlayerController->SetInputMode(FInputModeUIOnly());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("LoginWidgetClassNO"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerControllerNO"));
	}
	
}
