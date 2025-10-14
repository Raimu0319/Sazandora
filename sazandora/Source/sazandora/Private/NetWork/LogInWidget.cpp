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
	FString MapName = TEXT("ThirdPersonMap_NetWorkTest");	//開くマップ
	FString Options = MapName + TEXT("?listen");	//Listenサーバーとして開く

	UGameplayStatics::OpenLevel(GetWorld(), FName(*Options), true);

	UE_LOG(LogTemp, Warning, TEXT("ListenServer_Start"));	//TEXT()に渡す時は英語で渡す（クラッシュ防止）
}

void ULogInWidget::OnClientButtonClicked()
{
	if (!TextBoxIPAddress) return;

	FString IPAddress = TextBoxIPAddress->GetText().ToString();
	if (IPAddress.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("No_IPAddress"));
		return;
	}

	FString LevelName = IPAddress;
	UGameplayStatics::OpenLevel(GetWorld(), FName(*LevelName), true);
	UE_LOG(LogTemp, Warning, TEXT("ClientConnection:%s"), *IPAddress);
}