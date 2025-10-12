// Fill out your copyright notice in the Description page of Project Settings.


#include "NetWork/LogInWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "../../NetWork/NetWorkGameModeBase.h"

void ULogInWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ServerButton)
	{
		ServerButton->OnClicked.AddDynamic(this, &ULogInWidget::OnServerButtonClicked);
	}

	if (ClientButton)
	{
		ClientButton->OnClicked.AddDynamic(this, &ULogInWidget::OnClientButtonClicked);
	}
}

void ULogInWidget::OnServerButtonClicked()
{
	//サーバー起動
	if (ANetWorkGameModeBase* GM = Cast<ANetWorkGameModeBase>(UGameplayStatics::GetGameMode(GetWorld())))
	{
		GM->StartListenServer();
	}
}

void ULogInWidget::OnClientButtonClicked()
{
	//サーバー起動
	if (ANetWorkGameModeBase* GM = Cast<ANetWorkGameModeBase>(UGameplayStatics::GetGameMode(GetWorld())))
	{
		GM->ConnectToServer(TEXT("127.0.0.1"));
	}
}