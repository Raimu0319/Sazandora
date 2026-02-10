// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMain/MyGameInstance.h"
#include "HAL/PlatformProcess.h"
#include "Misc/Paths.h"
#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include <windows.h>
#include "Windows/HideWindowsPlatformTypes.h"
#endif

static UMyGameInstance* GMyGI = nullptr;

#if PLATFORM_WINDOWS
BOOL WINAPI ConsoleHandler(DWORD CtrlType)
{
	if (CtrlType == CTRL_CLOSE_EVENT ||
		CtrlType == CTRL_C_EVENT ||
		CtrlType == CTRL_SHUTDOWN_EVENT)
	{
		if (GMyGI)
		{
			UE_LOG(LogTemp, Error, TEXT("Console close detected → stop API server"));
			GMyGI->StopAPIServer();

			Sleep(300); // プロセスが殺される前にAPIを落とす猶予
		}
	}
	return false;
}
#endif

void UMyGameInstance::Init()
{
	Super::Init();

	GMyGI = this;
	//LogInWidgetで入力したアドレスをコマンドライン引数から抽出する(Dedicated Server用)
	FString Value;
	if (FParse::Value(FCommandLine::Get(), TEXT("apiip="), Value))
	{
		APIServerIP = Value;
		UE_LOG(LogTemp, Warning, TEXT("API Server IP Passed: %s"), *Value);
	}
	else
	{
		APIServerIP = TEXT("127.0.0.1");
		UE_LOG(LogTemp, Warning, TEXT("NO API Server IP Passed: %s"), *Value);
	}

	if (IsRunningDedicatedServer())
	{
		GetWorld()->GetTimerManager().SetTimer(
			HeartbeatTimer,
			this,
			&UMyGameInstance::UpdateServerInfoOnAPI,
			10.0f,
			true
		);
	}

	StartAPIServer();	//APIサーバー開始
	UE_LOG(LogTemp, Warning, TEXT("MyGameInstance:Init"));

	FCoreDelegates::OnPreExit.AddUObject(this, &UMyGameInstance::OnServerPreExit);

#if PLATFORM_WINDOWS
	SetConsoleCtrlHandler(ConsoleHandler, true);
#endif
}

void UMyGameInstance::Shutdown()
{
	UE_LOG(LogTemp, Warning, TEXT("MyGameInstance:Shutdown"));
	StopAPIServer();
	Super::Shutdown();
}

//何かしらの理由でShutdownが呼ばれなかった時のための終了処理
void UMyGameInstance::OnServerPreExit()
{
	//二重処理防止
	if (CleanedUp)
	{
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("MyGameInstance:OnServerPreExit"));
	CleanedUp = true;

	StopAPIServer();	//APIサーバーを閉じる
}

void UMyGameInstance::StartAPIServer()	//APIサーバー起動関数
{
	
	//FString BaseDir = FPaths::ConvertRelativePathToFull(FPaths::LaunchDir());	//パッケージ化して実行する際のファイルパス
	FString BaseDir = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());	//UEエディタ上で実行する際のファイルパス
	FString ServerPath = FPaths::Combine(BaseDir, TEXT("APIServer/APIServer.exe"));

	if (!FPaths::FileExists(ServerPath))
	{
		UE_LOG(LogTemp, Error, TEXT("APIServerScriptNO...: %s"), *ServerPath)
		return;
	}

	FString CommandLine = FString::Printf(TEXT("\"%s\""), *ServerPath);

	NodeProcessHandle = FPlatformProcess::CreateProc(
		*ServerPath, *CommandLine,
		true, false, false,
		nullptr, 0, nullptr, nullptr
	);

	if (NodeProcessHandle.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Node.js:APIServerOpen"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Node.js:APIServerCan'tOpen"));
	}
}

void UMyGameInstance::StopAPIServer()	//APIサーバー停止関数
{
	if (NodeProcessHandle.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Node.js:APIServerCloss..."));
		FPlatformProcess::TerminateProc(NodeProcessHandle, true);
		FPlatformProcess::WaitForProc(NodeProcessHandle);
		FPlatformProcess::CloseProc(NodeProcessHandle);
		NodeProcessHandle.Reset();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Node.js:APIServerNOCloss..."));
	}
}

void UMyGameInstance::UpdateServerInfoOnAPI()	//APIサーバーに登録している情報の更新処理
{
	UE_LOG(LogTemp, Warning, TEXT("UpdateServerInfoOnAPI"));
	// APIエンドポイント
	int32 Port = GetWorld()->URL.Port;

	FString Address;
	
		//ゲームインスタンスの変数に保持していたIPアドレスを取得する
		if (APIServerIP.IsEmpty())	//インスタンスに保持しているIPアドレスの中身を確認する
		{
			//中身が空だったら、自分のPCを指すループバックアドレスを代入
			Address = TEXT("127.0.0.1");
		}
		else
		{
			//中身があったら、インスタンスのIPアドレスを代入
			Address = APIServerIP;
		}


	FString Url = FString::Printf(TEXT("http://%s:3000/api/servers/update"), *Address);
	UE_LOG(LogTemp, Warning, TEXT("UpdateServerInfoOnAPI PUT URL = %s"), *Url);
	FString ServerName = FString::Printf(TEXT("Server:%d"), Port);
	int32 MaxPlayers = 4;

	// JSONオブジェクト作成
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField(TEXT("name"), ServerName);
	JsonObject->SetNumberField(TEXT("playerCount"), PlayerCount);
	JsonObject->SetNumberField(TEXT("maxPlayers"), MaxPlayers);
	JsonObject->SetBoolField(TEXT("gameplay"), false);

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	// HTTPリクエスト作成
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(Url);
	Request->SetVerb(TEXT("PUT"));  // ここが「更新」
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetContentAsString(OutputString);

	//OnProcessRequestComplete()：HTTP通信が完了した際に呼ばれるイベント
	//BindLambda()：OnProcessRequestComplete()が呼ばれたらBindLambdaの{}内の処理を行う
	//第一引数：FHttpRequestPtr Request = 送信したリクエスト(URLやPUTなど)
	//第二引数：FHttpResponsePtr Response = 接続したサーバーから返ってきたレスポンス
	//第三引数：bool bWasSuccessful = 通信が成功したか, true = 接続成功, false = 接続不可
	Request->OnProcessRequestComplete().BindLambda([](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
		{
			//通信失敗またはHTTPの応答が無かったら
			if (!bWasSuccessful || !Response.IsValid())
			{
				//エラーログを表示
				UE_LOG(LogTemp, Error, TEXT("HTTP Request failed (no response)"));
				return;
			}

			int32 Code = Response->GetResponseCode();

			//HTTP通信が成功したかどうか
			//Code == 200：成功
			//一部例(HTTPステータスコード参照)：Code == 500：サーバーエラー
			if (Code == 200)
			{
				//通信成功ログ
				UE_LOG(LogTemp, Warning, TEXT("Server info updated successfully."));
			}
			else
			{
				//通信失敗ログ
				UE_LOG(LogTemp, Error, TEXT("Failed to update server info: %s"), *Response->GetContentAsString());
			}
		});

	Request->ProcessRequest();	//リクエスト送信
}

void UMyGameInstance::Set_PlayerCount(int32_t count)
{
	PlayerCount = count;
}