// Fill out your copyright notice in the Description page of Project Settings.


#include "NetWork/ServerListWidget.h"



UServerListWidget::UServerListWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    UE_LOG(LogTemp, Log, TEXT("ServerListWidgtInit..."));

    static ConstructorHelpers::FClassFinder<UUserWidget>UnableConnectWidgetBP(
        TEXT("/Game/UI/Login_UI/UnableConnect_Widget.UnableConnect_Widget_C"));

    if (UnableConnectWidgetBP.Succeeded())
    {
        UnableConnectWidget = UnableConnectWidgetBP.Class;
        UE_LOG(LogTemp, Log, TEXT("UnableConnectWidgetBP_OK...."));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("UnableConnectWidgetBP_NONE...."));
    }
}

void UServerListWidget::Setup(const FString& ServerName, const FString& IP, int playercount, bool gameplay, TSubclassOf<class UUnableConnectWidget> point)
{
    // 受け取った情報を自分の内部に保存
    ServerIP = IP;
    PlayerCount = playercount;
    GamePlay = gameplay;
    UnableConnectWidget = point;

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
        FString MyString = FString::Printf(TEXT("%d/4"), playercount);
        PlayerCountText->SetText(FText::FromString(MyString));
    }

    //サーバーにプレイヤーが上限(4人)居る、またはサーバーでゲームが既に開始されていた場合は
    //ログインできないことを視覚的にわかるようにフォントの色を赤色にする。
    if (playercount >= 4 || gameplay)
    {
        ServerNameText->SetColorAndOpacity(FSlateColor(FLinearColor::Red));
        ServerIPAddress->SetColorAndOpacity(FSlateColor(FLinearColor::Red));
        PlayerCountText->SetColorAndOpacity(FLinearColor::Red);
    }
    else
    {
        ServerNameText->SetColorAndOpacity(FSlateColor(FLinearColor::Green));
        ServerIPAddress->SetColorAndOpacity(FSlateColor(FLinearColor::Green));
        PlayerCountText->SetColorAndOpacity(FLinearColor::Green);
    }

    // ボタンのクリックイベントを登録
    if (ConnectButton)
    {
        ConnectButton->OnClicked.AddDynamic(this, &UServerListWidget::OnConnectClicked);
    }
}

void UServerListWidget::OnConnectClicked()
{
    //サーバーに居るプレイヤー数が4人以上　または　ゲームが開始されいたら
    //そのサーバーにはログインできないようにする
    if (PlayerCount >= 4 || GamePlay)
    {
        UWorld* World = GetWorld();
        if (World)
        {
            UE_LOG(LogTemp, Warning, TEXT("World type: %s"), *World->GetName());
            UE_LOG(LogTemp, Log, TEXT("World_OK...."));
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("World_None...."));
        }

        //変数の中身がちゃんと入っているかの確認
        if (UnableConnectWidget)
        {
            UE_LOG(LogTemp, Log, TEXT("UnableConnectWidget_OK...."));
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("UnableConnectWidget_None...."));
        }

        UUnableConnectWidget* UnableWidget = CreateWidget<UUnableConnectWidget>(World, UnableConnectWidget);
        if (UnableWidget)
        {
            UE_LOG(LogTemp, Log, TEXT("UnableConnectWidgetBP_OK...."));
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("UnableConnectWidgetBP_NONE...."));
            return;
        }

        UnableWidget->AddToViewport();
        UnableWidget->SetVisibility(ESlateVisibility::Visible);
    }
    else //if条件に当てはまらず、サーバーにログインできる状態ならログインする
    {
        FString ConnectString = FString::Printf(TEXT("%s"), *ServerIP);
        UE_LOG(LogTemp, Log, TEXT("Connecting to server: %s"), *ConnectString);

        APlayerController* PC = GetWorld()->GetFirstPlayerController();
        if (PC)
        {
            PC->ClientTravel(ConnectString, TRAVEL_Absolute);
        }
    }
}