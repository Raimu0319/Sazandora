// Fill out your copyright notice in the Description page of Project Settings.


#include "NetWorkGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "../GameMain/Main_Character.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Json.h"
#include "JsonUtilities.h"
#include "Sockets.h"
#include "SocketSubsystem.h"

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
	UE_LOG(LogTemp, Warning, TEXT("Address"));
	RegisterServerToAPI();
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

	UpdateServerInfoOnAPI();
	RegisterServerToAPI();
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

	UpdateServerInfoOnAPI();
	RegisterServerToAPI();
}

void ANetWorkGameModeBase::RegisterServerToAPI()
{
	FString ServerName = "SazandoraServer";
	FString ServerAddress = Get_IPAddress(); // 実際は外部IPを取得する方法もあり
	int32 PlayerCount = NextSpawnIndex;
	int32 MaxPlayers = 4;

	UE_LOG(LogTemp, Warning, TEXT("Address:	%s"), *ServerAddress);
	UE_LOG(LogTemp, Warning, TEXT("Address"));

	// JSONデータ作成
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField(TEXT("name"), ServerName);
	JsonObject->SetStringField(TEXT("address"), ServerAddress);
	JsonObject->SetNumberField(TEXT("playerCount"), PlayerCount);
	JsonObject->SetNumberField(TEXT("maxPlayers"), MaxPlayers);

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	// HTTPリクエスト作成
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(TEXT("http://localhost:3000/register")); // Node.js APIサーバーのURL
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetContentAsString(OutputString);

	// コールバック設定
	Request->OnProcessRequestComplete().BindLambda([](FHttpRequestPtr Req, FHttpResponsePtr Res, bool bSuccess)
		{
			if (bSuccess && Res.IsValid())
			{
				UE_LOG(LogTemp, Warning, TEXT("Server Registered: %s"), *Res->GetContentAsString());
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to register server"));
			}
		});

	// 実行
	Request->ProcessRequest();
}

FString ANetWorkGameModeBase::Get_IPAddress()
{
	bool bCanBind = false;
	TSharedRef<FInternetAddr> Addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetLocalHostAddr(*GLog, bCanBind);

	// デフォルトのポート（7777など）を取得
	UWorld* World = GetWorld();
	uint16 Port = 0;
	if (World && World->URL.Port != 0)
	{
		Port = World->URL.Port;
	}
	else
	{
		// Fallback: 一般的なデフォルトポート
		Port = 7777;
	}

	Addr->SetPort(Port);
	return Addr->ToString(true); // falseにするとポート番号を含まない
}

void ANetWorkGameModeBase::UpdateServerInfoOnAPI()
{
	// APIエンドポイント
	FString Url = TEXT("http://127.0.0.1:3000/api/servers/update");

	int32 PlayerCount = NextSpawnIndex;
	int32 MaxPlayers = 4;

	// JSONオブジェクト作成
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField(TEXT("name"), TEXT("SazandoraServer"));
	JsonObject->SetNumberField(TEXT("playerCount"), PlayerCount);
	JsonObject->SetNumberField(TEXT("maxPlayers"), MaxPlayers);

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	// HTTPリクエスト作成
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(Url);
	Request->SetVerb(TEXT("PUT"));  // ここが「更新」
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetContentAsString(OutputString);

	Request->OnProcessRequestComplete().BindLambda([](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
		{
			if (bWasSuccessful && Response->GetResponseCode() == 200)
			{
				UE_LOG(LogTemp, Warning, TEXT("Server info updated successfully."));
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to update server info: %s"), *Response->GetContentAsString());
			}
		});

	Request->ProcessRequest();
}