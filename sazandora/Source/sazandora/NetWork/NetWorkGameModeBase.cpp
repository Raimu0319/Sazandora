// Fill out your copyright notice in the Description page of Project Settings.


#include "NetWorkGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "../Public/Main_Character.h"

ANetWorkGameModeBase::ANetWorkGameModeBase()
{
	DefaultPawnClass = AMain_Character::StaticClass();
}

void ANetWorkGameModeBase::BeginPlay()
{
	Super::BeginPlay();
}

void ANetWorkGameModeBase::StartListenServer()
{
	UWorld* World = GetWorld();

	if (World)
	{
		FString MapName = "ThirdPersonMap_NetWorkTest";
		FString Options = MapName + "?listen";
		UGameplayStatics::OpenLevel(World, FName(*Options), true);
		UE_LOG(LogTemp, Warning, TEXT("Listenサーバーを開始しました"));
	}
}

void ANetWorkGameModeBase::ConnectToServer(const FString& IPAddress)
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	if (PC)
	{
		FString Command = FString::Printf(TEXT("open %s"), *IPAddress);
		PC->ConsoleCommand(Command);
		UE_LOG(LogTemp, Warning, TEXT("サーバーに接続：%s"), *IPAddress);
	}
}