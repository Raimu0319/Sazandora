// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMain/MyGameInstance.h"
#include "HAL/PlatformProcess.h"
#include "Misc/Paths.h"

void UMyGameInstance::Init()
{
	Super::Init();
	StartAPIServer();
	UE_LOG(LogTemp, Warning, TEXT("MyGameInstance:Init"));
}

void UMyGameInstance::Shutdown()
{
	StopAPIServer();
	Super::Shutdown();
	UE_LOG(LogTemp, Warning, TEXT("MyGameInstance:Shutdown"));
}

void UMyGameInstance::StartAPIServer()
{
	FString NodePath = TEXT("C:/Program Files/nodejs/node.exe");
	FString ServerScript = FPaths::ProjectDir() / TEXT("API/server.js");

	if (!FPaths::FileExists(ServerScript))
	{
		UE_LOG(LogTemp, Error, TEXT("APIServerScriptNO...: %s"), *ServerScript)
		return;
	}

	FString CommandLine = FString::Printf(TEXT("\"%s\""), *ServerScript);

	NodeProcessHandle = FPlatformProcess::CreateProc(
		*NodePath, *CommandLine,
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

void UMyGameInstance::StopAPIServer()
{
	if (NodeProcessHandle.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Node.js:APIServerCloss..."));
		FPlatformProcess::TerminateProc(NodeProcessHandle, true);
		FPlatformProcess::CloseProc(NodeProcessHandle);
		NodeProcessHandle.Reset();
	}
}