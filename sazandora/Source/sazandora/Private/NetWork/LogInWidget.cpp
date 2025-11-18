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
	static ConstructorHelpers::FClassFinder<UUserWidget> ServerListWidgetBP(
		TEXT("/Game/Login_UI/ServerList_Widget.ServerList_Widget_C")
	);

	if (ServerListWidgetBP.Succeeded())
	{
		ServerListWidget = ServerListWidgetBP.Class;
		UE_LOG(LogTemp, Warning, TEXT("ServerRowWidgetClass loaded successfully!"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load ServerRowWidgetClass."));
	}
}

void ULogInWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (HostButton)
	{
		HostButton->OnClicked.AddDynamic(this, &ULogInWidget::OnHostButtonClicked);
	}

	if (JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this, &ULogInWidget::OnJoinButtonClicked);
	}

	if (RefreshButton)
	{
		RefreshButton->OnClicked.AddDynamic(this, &ULogInWidget::OnRefreshServerListClicked);
	}

	if (BackButton)
	{
		BackButton->OnClicked.AddDynamic(this, &ULogInWidget::OnBackButtonClicked);
	}

	// 起動時にも1回取得
	OnRefreshServerListClicked();
}


void ULogInWidget::OnHostButtonClicked()
{
	//サーバー起動
	//サーバー起動用の実行ファイルがあるファイルパスを指定
	//FString ServerPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() + TEXT("Binaries/Win64/sazandoraServer.exe"));
	//FString ServerPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() + TEXT("Server/sazandoraServer.exe"));

	//パッケージ化後のサーバーファイルパス
	FString BaseDir = FPaths::ConvertRelativePathToFull(FPaths::LaunchDir());
	FString ServerPath = FPaths::Combine(BaseDir, TEXT("Server/sazandoraServer.exe" /*"Binaries/Win64/sazandoraServer.exe"*/));
	//サーバー起動時に渡すコマンドライン引数
	FString ServerArgs = TEXT("/Game/PolygonCity/Maps/test_map?listen?game=Class'/Script/sazandora.SazandoraGameMode' -log");
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

void ULogInWidget::OnRefreshServerListClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("Requesting server list..."));

	// HTTPリクエストを作成
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(TEXT("http://127.0.0.1:3000/servers")); // Node.js 側のAPI URL
	Request->SetVerb(TEXT("GET"));
	Request->OnProcessRequestComplete().BindUObject(this, &ULogInWidget::OnServerListReceived);
	Request->ProcessRequest();
}

void ULogInWidget::OnBackButtonClicked()
{
	HostButton->SetVisibility(ESlateVisibility::Visible);
	JoinButton->SetVisibility(ESlateVisibility::Visible);
	ServerListScrollBox->SetVisibility(ESlateVisibility::Hidden);
	RefreshButton->SetVisibility(ESlateVisibility::Hidden);
	BackButton->SetVisibility(ESlateVisibility::Hidden);
	UE_LOG(LogTemp, Warning, TEXT("ServerListHidden..."));
}

void ULogInWidget::OnJoinButtonClicked()
{
	HostButton->SetVisibility(ESlateVisibility::Hidden);
	JoinButton->SetVisibility(ESlateVisibility::Hidden);
	ServerListScrollBox->SetVisibility(ESlateVisibility::Visible);
	RefreshButton->SetVisibility(ESlateVisibility::Visible);
	BackButton->SetVisibility(ESlateVisibility::Visible);
	
	OnRefreshServerListClicked();

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

			for (auto ServerValue : *ServerArray)
			{
				TSharedPtr<FJsonObject> ServerData = ServerValue->AsObject();

				FString Name = ServerData->GetStringField(TEXT("name"));
				FString Address = ServerData->GetStringField(TEXT("address"));
				int32 PlayerCount = ServerData->GetNumberField(TEXT("playerCount"));

				if (ServerListWidget == nullptr)
				{
					UE_LOG(LogTemp, Warning, TEXT("ServerListWidget_NULL"));
				}

				// ウィジェット生成
				if (ServerListWidget && ServerListScrollBox)
				{
					UServerListWidget* Row = CreateWidget<UServerListWidget>(this, ServerListWidget);
					if (Row)
					{
						UE_LOG(LogTemp, Warning, TEXT("ServerListWidget_OK"));
						UE_LOG(LogTemp, Warning, TEXT("PlayerCount:%d"), PlayerCount);
						Row->Setup(Name, Address, PlayerCount);
						ServerListScrollBox->AddChild(Row);
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("ServerListWidget_NULL"));
					}
				}
				else
				{
					/*if (ServerListWidget == nullptr)
					{
						UE_LOG(LogTemp, Warning, TEXT("ServerListWidget_NULL"));
					}*/
					if (ServerListScrollBox == nullptr)
					{
						UE_LOG(LogTemp, Warning, TEXT("ServerListScrollBox_NULL"));
					}
			
				}
			}
		}
	}
}



//void ULogInWidget::GetServerListFromMaster()
//{
//	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
//	Request->SetURL("http://127.0.0.1:3000/list");
//	Request->SetVerb("GET");
//	Request->SetHeader("Content-Type", "application/json");
//
//	Request->OnProcessRequestComplete().BindLambda([](FHttpRequestPtr Req, FHttpResponsePtr Response, bool bWasSuccessful)
//		{
//			if (bWasSuccessful)
//			{
//				FString ResponseString = Response->GetContentAsString();
//				UE_LOG(LogTemp, Warning, TEXT("サーバー一覧: %s"), *ResponseString);
//			}
//			else
//			{
//				UE_LOG(LogTemp, Warning, TEXT("サーバー一覧取得失敗"));
//			}
//		});
//
//	Request->ProcessRequest();
//}
//
//void ULogInWidget::OnServerListReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
//{
//	if (!bWasSuccessful) return;
//
//	FString ResponseString = Response->GetContentAsString();
//
//	TArray<TSharedPtr<FJsonValue>> JsonArray;
//	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseString);
//
//	if (FJsonSerializer::Deserialize(Reader, JsonArray))
//	{
//		// 既存のリストをクリア
//		ServerListScrollBox->ClearChildren();
//
//		for (auto& JsonValue : JsonArray)
//		{
//			TSharedPtr<FJsonObject> ServerObj = JsonValue->AsObject();
//			if (!ServerObj.IsValid()) continue;
//
//			FString Name = ServerObj->GetStringField("name");
//			IP = ServerObj->GetStringField("ip");
//			Port = ServerObj->GetIntegerField("port");
//			Map = ServerObj->GetStringField("map");
//
//			// Widgetを生成してScrollBoxに追加
//			ULogInWidget* Row = CreateWidget<ULogInWidget>(this, ServerRowWidgetClass);
//			Row->Setup(Name, IP, Port, Map);
//			ServerListScrollBox->AddChild(Row);
//		}
//	}
//
//}
//
//void ULogInWidget::Setup(const FString& InName, const FString& InIP, int32 InPort, const FString& InMap)
//{
//
//}