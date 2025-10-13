// Fill out your copyright notice in the Description page of Project Settings.


#include "LoginMenuGameMode.h"
#include "Kismet/GameplayStatics.h"

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
		PlayerController->SetInputMode(InputMode);
	}

}