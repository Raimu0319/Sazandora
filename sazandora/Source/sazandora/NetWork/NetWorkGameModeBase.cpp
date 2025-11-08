// Fill out your copyright notice in the Description page of Project Settings.


#include "NetWorkGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "../GameMain/Main_Character.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"

int32 ANetWorkGameModeBase::NextSpawnIndex; // 静的変数

ANetWorkGameModeBase::ANetWorkGameModeBase()
{
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Succeeded())
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
		UE_LOG(LogTemp, Warning, TEXT("PlayerPawnBPClassOK"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerPawnBPClassNO"));
	}

	NextSpawnIndex = 0;
}

void ANetWorkGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("Current GameMode: %s"), *GetClass()->GetName());
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

AActor* ANetWorkGameModeBase::ChoosePlayerStart_Implementation(AController* Player)
{
	UE_LOG(LogTemp, Warning, TEXT("ChoosePlayerStart_Implementation called!"));

	UE_LOG(LogTemp, Warning, TEXT("[%s] ChoosePlayerStart called on %s"),
		HasAuthority() ? TEXT("SERVER") : TEXT("CLIENT"), *GetName());

	TArray<AActor*> PlayerStarts;
	UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);

	if (PlayerStarts.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No PlayerStart found!"));
		return Super::ChoosePlayerStart_Implementation(Player);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerStart_Count:%d"), PlayerStarts.Num());
	}

	// スポーン位置を順番に使用
	int32 Index = NextSpawnIndex % PlayerStarts.Num();
	NextSpawnIndex++;

	AActor* Start = PlayerStarts[Index];
	UE_LOG(LogTemp, Warning, TEXT("Spawning Player %d at %s"), Index, *Start->GetName());
	return Start;
}

void ANetWorkGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (!HasAuthority()) return; // サーバーでのみ実行

	if (NewPlayer->GetPawn() == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("PostLogin: Restarting Player"));
		RestartPlayer(NewPlayer);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("PostLogin: Player already has pawn, skipping Restart"));
	}
}

void ANetWorkGameModeBase::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	//ログアウトしたらNextSpawnIndexを減らす
	if (NextSpawnIndex > 0)
	{
		NextSpawnIndex--;
	}
	UE_LOG(LogTemp, Warning, TEXT("NextSpawnIndex = %d"), NextSpawnIndex);
}