// Fill out your copyright notice in the Description page of Project Settings.


#include "NetWork/LogInWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "../../NetWork/NetWorkGameModeBase.h"
#include "../Public/NetWork/ServerListWidget.h"


ULogInWidget::ULogInWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	if (!IsRunningDedicatedServer())
	{

		static ConstructorHelpers::FClassFinder<UUserWidget> ServerListWidgetBP(
			TEXT("/Game/UI/Login_UI/ServerList_Widget.ServerList_Widget_C"));

		static ConstructorHelpers::FClassFinder<UUnableConnectWidget> UnableConnectWidgetBP(
			TEXT("/Game/UI/Login_UI/UnableConnect_Widget.UnableConnect_Widget_C"));

		if (ServerListWidgetBP.Succeeded())
		{
			ServerListWidget = ServerListWidgetBP.Class;
			UE_LOG(LogTemp, Warning, TEXT("ServerRowWidgetClass loaded successfully!"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to load ServerRowWidgetClass."));
		}

		if (UnableConnectWidgetBP.Succeeded())
		{
			UE_LOG(LogTemp, Warning, TEXT("UnableConnectWidgetClass loaded successfully!"));
			UnableConnectWidget = UnableConnectWidgetBP.Class;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to load UnableConnectWidgetClass"));
		}
	}
}

void ULogInWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (HostButton)	//ホストボタンが押されたら
	{
		HostButton->OnClicked.AddDynamic(this, &ULogInWidget::OnHostButtonClicked);
	}

	if (JoinButton)	//ジョインボタンが押されたら
	{
		JoinButton->OnClicked.AddDynamic(this, &ULogInWidget::OnJoinButtonClicked);
	}

	if (RefreshButton)	//リフレッシュボタンが押されたら
	{
		RefreshButton->OnClicked.AddDynamic(this, &ULogInWidget::OnRefreshServerListClicked);
	}

	if (BackButton)	//バックボタンが押されたら
	{
		BackButton->OnClicked.AddDynamic(this, &ULogInWidget::OnBackButtonClicked);
	}

	// 起動時にAPIサーバーからサーバーリストを取得する
	OnRefreshServerListClicked();
}


void ULogInWidget::OnHostButtonClicked()
{
	//ホストサーバー起動フラグ
	HostServerStart = true;

	//今現在、稼働しているサーバーリスト更新＆ホストサーバー起動フラグがtrueならサーバーを起動する
	OnRefreshServerListClicked();
}

void ULogInWidget::OnRefreshServerListClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("Requesting server list..."));

	APIServerIP = IPAddressTextBox->GetText().ToString();
	if (APIServerIP.IsEmpty())
	{
		//APIServerIP = TEXT("192.168.0.5");
		APIServerIP = TEXT("127.0.0.1");
		UE_LOG(LogTemp, Warning, TEXT("UserIP_None..."));
	}
	else
	{
		UMyGameInstance* GI = GetWorld()->GetGameInstance<UMyGameInstance>();
		if (GI)
		{
			UE_LOG(LogTemp, Warning, TEXT("LoginWidget GI OK!!"));
			GI->APIServerIP = APIServerIP;
			UE_LOG(LogTemp, Warning, TEXT("%s"), *APIServerIP);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("LoginWidget GI NO..."));
		}
		UE_LOG(LogTemp, Warning, TEXT("UserIP_Yes!!"));
	}
	FString URL = FString::Printf(TEXT("http://%s:3000/servers"), *APIServerIP);
	
	// HTTPリクエストを作成
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(URL); // Node.js 側のAPI URL
	Request->SetVerb(TEXT("GET"));
	Request->OnProcessRequestComplete().BindUObject(this, &ULogInWidget::OnServerListReceived);
	Request->ProcessRequest();
}

void ULogInWidget::OnBackButtonClicked()
{
	//画面表示(Visible,Hidden)を切り替える
	HostButton->SetVisibility(ESlateVisibility::Visible);
	JoinButton->SetVisibility(ESlateVisibility::Visible);
	ServerListScrollBox->SetVisibility(ESlateVisibility::Hidden);
	RefreshButton->SetVisibility(ESlateVisibility::Hidden);
	BackButton->SetVisibility(ESlateVisibility::Hidden);
	UE_LOG(LogTemp, Warning, TEXT("ServerListHidden..."));

	if (ServerListScrollBox)
	{
		ServerListScrollBox->ClearChildren();
	}
}

void ULogInWidget::OnJoinButtonClicked()
{
	//画面表示(Visible,Hidden)を切り替える
	HostButton->SetVisibility(ESlateVisibility::Hidden);
	JoinButton->SetVisibility(ESlateVisibility::Hidden);
	ServerListScrollBox->SetVisibility(ESlateVisibility::Visible);
	RefreshButton->SetVisibility(ESlateVisibility::Visible);
	BackButton->SetVisibility(ESlateVisibility::Visible);
	
	//APIサーバーからサーバーリストの情報を受け取る
	OnRefreshServerListClicked();

}

void ULogInWidget::Server_Start()
{
	int32  Port = CheckforfreePorts();	//使用できるポート番号検索

	if (Port < 0)
	{
		UE_LOG(LogTemp, Error, TEXT("No available port found!"));
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("Using Port: %d"), Port);

	//パッケージ化後のサーバーファイルパス
	FString BaseDir = FPaths::ConvertRelativePathToFull(FPaths::LaunchDir());
	FString ServerPath = FPaths::Combine(BaseDir, TEXT(/*"Server/sazandoraServer.exe"*/"Binaries/Win64/sazandoraServer.exe"));
	//サーバー起動時に渡すコマンドライン引数
	FString ServerArgs = FString::Printf(TEXT("/Game/PolygonCity/Maps/test_map?listen?port=%d?apiip=%s?game=Class'/Script/sazandora.SazandoraGameMode' -log"), Port, *APIServerIP);
	//サーバー起動用ファイルを起動する
	FPlatformProcess::CreateProc(*ServerPath, *ServerArgs, true, false, false, nullptr, 0, nullptr, nullptr);

	//指定した秒数経過後に先ほど起動したサーバーに接続する
	FTimerHandle TimerHandle;

	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, Port]()
		{
			FString IPAddress = TEXT("127.0.0.1");
			UGameplayStatics::OpenLevel(GetWorld(), FName(*FString::Printf(TEXT("%s:%d"), *IPAddress, Port)));
			ReleaseReservedPorts();
		}, 2.0f, false);

	UE_LOG(LogTemp, Warning, TEXT("ListenServer_Start"));	//TEXT()に渡す時は英語で渡す（クラッシュ防止）

}


void ULogInWidget::OnServerListReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful || !Response.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to get server list"));
		return;
	}

	FString ResponseString = Response->GetContentAsString();
	UE_LOG(LogTemp, Warning, TEXT("Server list: %s"), *ResponseString);

	// JSON解析
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseString);

	if (FJsonSerializer::Deserialize(Reader, JsonObject))
	{
		const TArray<TSharedPtr<FJsonValue>>* ServerArray;
		if (JsonObject->TryGetArrayField(TEXT("servers"), ServerArray))
		{
			if (ServerListScrollBox)
			{
				ServerListScrollBox->ClearChildren();
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("ServerListScrollBox_NULL"));
			}

			OccupiedPorts.Empty();	//配列の中身を初期化

			for (auto ServerValue : *ServerArray)
			{
				TSharedPtr<FJsonObject> ServerData = ServerValue->AsObject();

				FString Name = ServerData->GetStringField(TEXT("name"));
				FString Address = ServerData->GetStringField(TEXT("address"));
				int32 PlayerCount = ServerData->GetNumberField(TEXT("playerCount"));
				bool gameplay = ServerData->GetBoolField(TEXT("gameplay"));
				if (ServerListWidget == nullptr)
				{
					UE_LOG(LogTemp, Warning, TEXT("ServerListWidget_NULL"));
				}

				//受取ったIPアドレスからポート番号を抽出する
				FString IPPart, PortPart;
				if (Address.Split(TEXT(":"), &IPPart, &PortPart))
				{
					int32 Port = FCString::Atoi(*PortPart);	//文字列を整数に変換する
					OccupiedPorts.Add(Port);				//使用しているポート番号を格納する
					UE_LOG(LogTemp, Warning, TEXT("PushPort:%d"), Port);
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("NOPushPort"));
				}
				
				// サーバーリストウィジェット生成
				if (ServerListWidget && ServerListScrollBox)
				{
					UServerListWidget* Row = CreateWidget<UServerListWidget>(this, ServerListWidget);
					if (Row)
					{
						UE_LOG(LogTemp, Warning, TEXT("ServerListWidget_OK"));
						UE_LOG(LogTemp, Warning, TEXT("PlayerCount:%d"), PlayerCount);
						if (UnableConnectWidget)
						{
							UE_LOG(LogTemp, Warning, TEXT("UnableConnectWidget_OK"));
						}
						else
						{
							UE_LOG(LogTemp, Warning, TEXT("UnableConnectWidget_None...."));
						}
						Row->Setup(Name, Address, PlayerCount, gameplay, UnableConnectWidget);	//APIサーバーから取得した情報をセットする
						ServerListScrollBox->AddChild(Row);
					}
				}
			}

			//ホストサーバー起動フラグがtrueならサーバーを起動する
			if (HostServerStart)
			{
				HostServerStart = false;
				Server_Start();		//サーバー起動用関数
			}
		}
	}
}

int32 ULogInWidget::CheckforfreePorts()
{
	ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);

	//使用されていないポート番号を調べる
    for (int32 Port = 7777; Port <= 7799; Port++)
    {
		// 既に使用済みならスキップ
		if (OccupiedPorts.Contains(Port))
		{
			UE_LOG(LogTemp, Warning, TEXT("ContinuePort:%d"), Port);
			continue;
		}

        TSharedRef<FInternetAddr> Addr = SocketSubsystem->CreateInternetAddr();
        bool bIsValid = false;
        Addr->SetIp(TEXT("127.0.0.1"), bIsValid);
        Addr->SetPort(Port);

        // UDP ソケット作成（UE Dedicated Serverと競合する）
        FSocket* Socket = SocketSubsystem->CreateSocket(NAME_DGram, TEXT("PortCheckSocket"), false);
        if (!Socket)
            continue;

        Socket->SetReuseAddr(true);
        Socket->SetNonBlocking(true);


        if (Socket->Bind(*Addr))
        {
            // ソケットを解放せず、保持する
			OccupiedPortSockets.Add(Socket);
			OccupiedPorts.Add(Port);         // ポート番号も保持
			UE_LOG(LogTemp, Warning, TEXT("returnPort:%d"),Port);
            return Port;
        }

        // bind NO = ソケットを破棄
        Socket->Close();
        SocketSubsystem->DestroySocket(Socket);
    }

    return -1;

}

void ULogInWidget::ReleaseReservedPorts()
{
	ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);

	for (FSocket* Socket : OccupiedPortSockets)
	{
		if (Socket)
		{
			Socket->Close();
			SocketSubsystem->DestroySocket(Socket);
		}
	}

	OccupiedPortSockets.Empty();
	//OccupiedPorts.Empty(); // ポート番号もクリア
}

void ULogInWidget::UnableConnectView(bool flag)
{
	bool Widget_flag = flag;
	UUnableConnectWidget* Row = CreateWidget<UUnableConnectWidget>(this, UnableConnectWidget);
	if (flag)
	{
		Row->AddToViewport();
		Row->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		Row->SetVisibility(ESlateVisibility::Hidden);
	}
}