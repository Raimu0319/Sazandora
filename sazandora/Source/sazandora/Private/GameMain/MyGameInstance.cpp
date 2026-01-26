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

	/*if (IsRunningDedicatedServer())
	{
		StartAPIServer();
	}*/

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