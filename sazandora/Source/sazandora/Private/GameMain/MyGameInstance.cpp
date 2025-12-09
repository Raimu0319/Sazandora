// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMain/MyGameInstance.h"
#include "HAL/PlatformProcess.h"
#include "Misc/Paths.h"

void UMyGameInstance::Init()
{
	Super::Init();

	//LogInWidgetで入力したアドレスをコマンドライン引数から抽出する(Dedicated Server用)
	FString Value;
	if (FParse::Value(FCommandLine::Get(), TEXT("apiip="), Value))
	{
		APIServerIP = Value;
		UE_LOG(LogTemp, Warning, TEXT("API Server IP Passed: %s"), *Value);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("NO API Server IP Passed: %s"), *Value);
	}

	/*if (IsRunningDedicatedServer())
	{
		StartAPIServer();
	}*/

	StartAPIServer();	//APIサーバー開始
	UE_LOG(LogTemp, Warning, TEXT("MyGameInstance:Init"));

	FCoreDelegates::OnPreExit.AddUObject(this, &UMyGameInstance::OnServerPreExit);
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
	
	/*FString BaseDir = FPaths::ConvertRelativePathToFull(FPaths::LaunchDir());*/	//パッケージ化して実行する際のファイルパス
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