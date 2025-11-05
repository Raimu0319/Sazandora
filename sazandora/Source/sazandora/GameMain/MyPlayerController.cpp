// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerController.h"
#include "MyPlayerState.h"
#include "../sazandoraGameMode.h"

AMyPlayerController::AMyPlayerController()
{
	static ConstructorHelpers::FClassFinder<UHUDWidget> widgetclass(TEXT("/Game/ThirdPerson/widget/BP_HUDWidget2"));

	if (widgetclass.Succeeded())
	{
		HUDWidget_class = widgetclass.Class;
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("HUDWidget is not find..."));
	}

}

void AMyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// キャラやUIのロードが完了したタイミングで通知
	NotifyLoaded();

}

void AMyPlayerController::NotifyLoaded()
{
	AMyPlayerState* player_state = GetPlayerState <AMyPlayerState>();

	if (!player_state)
	{

		UE_LOG(LogTemp, Warning, TEXT("[%s] NotifyLoaded skipped: PlayerState not ready"), *GetName());
		return;
	}

	// serverへ自分のロード完了を通知
	player_state->Server_SetLoaded(true);

	// サーバーのみ実行
	// HasAuthorityでサーバーかクライアントかを調べる
	// trueの場合はホストまたはサーバーでの実行
	// falseの場合はクライアントでの実行
	if (HasAuthority())
	{
		AsazandoraGameMode* gamemode = GetWorld()->GetAuthGameMode<AsazandoraGameMode>();
		if (gamemode)
		{
			gamemode->CheckAllPlayersLoaded();
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[%s] NotifyLoaded() called"), *GetName());
}

// HUDWidgetの作成
void AMyPlayerController::Create_HUDWidget()
{
	if (!IsLocalController())
	{
		return;
	}

	if (HUDWidget_class)
	{
		UHUDWidget* HUDWidget = CreateWidget<UHUDWidget>(this, HUDWidget_class);
		HUDWidget->AddToViewport();

		// PlayerStateの紐づけ
		AMyPlayerState* ps = GetPlayerState<AMyPlayerState>();
		HUDWidget->InitializeWidget(ps);
	}
}