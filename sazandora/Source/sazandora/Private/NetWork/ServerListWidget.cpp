// Fill out your copyright notice in the Description page of Project Settings.


#include "NetWork/ServerListWidget.h"
#include "Kismet/GameplayStatics.h"

void UServerListWidget::Setup(const FString& ServerName, const FString& IP, int PlayerCount)
{
    // 受け取った情報を自分の内部に保存
    ServerIP = IP;

    // サーバー名をUI上に表示
    if (ServerNameText)
    {
        ServerNameText->SetText(FText::FromString(ServerName));
    }

    if (ServerIPAddress)
    {
        ServerIPAddress->SetText(FText::FromString(IP));
    }

    if (PlayerCountText)
    {
        PlayerCountText->SetText(FText::AsNumber(PlayerCount));
    }

    // ボタンのクリックイベントを登録
    if (ConnectButton)
    {
        if (PlayerCount < 4)
        {
            ConnectButton->OnClicked.AddDynamic(this, &UServerListWidget::OnConnectClicked);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("PlayerCountOver..."));
        }
    }
}

void UServerListWidget::OnConnectClicked()
{
   /* FString ConnectString = FString::Printf(TEXT("%s"), *ServerIP);
    UE_LOG(LogTemp, Log, TEXT("Connecting to server: %s"), *ConnectString);
    UGameplayStatics::OpenLevel(GetWorld(), FName(*ConnectString));*/

    FString ConnectString = FString::Printf(TEXT("%s"), *ServerIP);
    UE_LOG(LogTemp, Log, TEXT("Connecting to server: %s"), *ConnectString);

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC)
    {
        PC->ClientTravel(ConnectString, TRAVEL_Absolute);
    }
}