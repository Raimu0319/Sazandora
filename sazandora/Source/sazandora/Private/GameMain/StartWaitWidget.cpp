// Fill out your copyright notice in the Description page of Project Settings.


#include "../Public/GameMain/StartWaitWidget.h"
#include "Components/Button.h"
#include "GameFramework/GameStateBase.h"
#include "../Public/GameMain/MyPlayerController.h"
#include "../Public/GameMain/MyPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "sazandora/sazandoraGameMode.h"

void UStartWaitWidget::Widget_Initialize()
{
	
}

//スタートボタンが押された際の処理
void UStartWaitWidget::OnStartButtonClicked()
{
	AMyPlayerController* pc = Cast<AMyPlayerController>(GetOwningPlayer());

	if(pc)
	{
		UE_LOG(LogTemp, Log, TEXT("push button"));
		pc->Server_RequestStartGame();
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("not push button"));
	}
}

void UStartWaitWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// ボタンが有効ならクリック時のイベントをバインド
	if (start_button)
	{
		start_button->OnClicked.AddDynamic(this, &UStartWaitWidget::OnStartButtonClicked);
	}

	AMyPlayerController* pc = Cast<AMyPlayerController>(GetOwningPlayer());
	if(!pc)
	{
		return;

		/*if ()
		{
			start_button->SetVisibility(ESlateVisibility::Collapsed);
		}*/
	}

	AMyPlayerState* ps = pc->GetPlayerState<AMyPlayerState>();
	if (!ps)
	{
		return;
	}

	// ホストならゲーム開始ボタンの表示
	if (ps->is_host)
	{
		start_button->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		start_button->SetVisibility(ESlateVisibility::Collapsed);
	}
}
