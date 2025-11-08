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
	//サーバー起動用の実行ファイルがあるファイルパスを指定
	FString ServerPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() + TEXT("Binaries/Win64/sazandoraServer.exe"));
	//サーバー起動時に渡すコマンドライン引数
	FString ServerArgs = TEXT("/Game/PolygonCity/Maps/test_map?listen -log");
	//サーバー起動用ファイルを起動する
	FPlatformProcess::CreateProc(*ServerPath, *ServerArgs, true, false, false, nullptr, 0, nullptr, nullptr);

	//指定した秒数経過後に先ほど起動したサーバーに接続する
	FTimerHandle TimerHandle;

	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
	{
			FString IPAddress = TEXT("127.0.0.1");
			UGameplayStatics::OpenLevel(GetWorld(), FName(*FString::Printf(TEXT("%s:7777"), *IPAddress)));
	}, 2.0f, false);

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

	FString MapPath = FString::Printf(TEXT("/Game/Maps/test_map?listen"));

	// クライアントは IP指定して接続する
	FString ConnectAddress = FString::Printf(TEXT("%s:7777"), *IPAddress);
	UGameplayStatics::OpenLevel(GetWorld(), FName(*ConnectAddress), true);
	UE_LOG(LogTemp, Warning, TEXT("ClientConnection:%s"), *IPAddress);
}