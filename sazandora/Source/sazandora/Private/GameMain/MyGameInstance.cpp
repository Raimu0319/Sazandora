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
	FString NodePath = TEXT("C:/PG/Sazandora/sazandora/APIServer/APIServer.exe");
	FString BaseDir = FPaths::ConvertRelativePathToFull(FPaths::LaunchDir());
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