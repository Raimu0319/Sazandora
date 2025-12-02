// Copyright Epic Games, Inc. All Rights Reserved.

#include "sazandoraGameMode.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "GameMain/GoalActor.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/PlayerController.h"
#include "GameMain/Main_Character.h"
#include "GameMain/GoalActor.h"
#include "GameMain/MyPlayerState.h"
#include "GameMain/MyPlayerController.h"
#include "UObject/ConstructorHelpers.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Json.h"
#include "JsonUtilities.h"
#include "Sockets.h"
#include "SocketSubsystem.h"

AsazandoraGameMode::AsazandoraGameMode()
{
	// カスタムMainCharacterの設定
	DefaultPawnClass = AMain_Character::StaticClass();

	// カスタムのPlayerStateクラスの指定
	PlayerStateClass = AMyPlayerState::StaticClass();

	// カスタムPlayerControllerの設定
	PlayerControllerClass = AMyPlayerController::StaticClass();

	//Tickを有効にする
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void AsazandoraGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// サーバー自身の場合
	if (NewPlayer->IsLocalController())
	{
		if (AMyPlayerState* player_state = NewPlayer->GetPlayerState<AMyPlayerState>())
		{
			player_state->Server_SetLoaded(true);
		}
	}

	AGameStateBase* gs = GameState;
	if (!gs)
	{
		return;
	}

	// 最初のプレイヤーにホストフラグを渡す
	if (gs->PlayerArray.Num() == 1)
	{
		AMyPlayerState* ps = NewPlayer->GetPlayerState<AMyPlayerState>();

		if (ps)
		{
			ps->is_host = true;
		}
	}

	if (AMyPlayerState* ps = NewPlayer->GetPlayerState<AMyPlayerState>())
	{
		// psに中身がちゃんとあるか
		if (ps)
		{
			// player番号を格納
			ps->Set_PlayerNumber(NextPlayerIndex);

			UE_LOG(LogTemp, Log, TEXT("now team_number : %d"), ps->player_number);
		}
	}

	NextPlayerIndex++;

	UpdateServerInfoOnAPI();
	RegisterServerToAPI();
}

void AsazandoraGameMode::BeginPlay()
{
	Super::BeginPlay();

	TArray<AActor*> starts;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), starts);

	for (int32 i = 0; i < starts.Num(); i++)
	{
		APlayerStart* start_point = Cast<APlayerStart>(starts[i]);

		if (!start_point)
		{
			continue;
		}

		FString TagString = FString::Printf(TEXT("StartPoint_%d"), i);
		FName NewTag(*TagString);

		start_point->Tags.Add(NewTag);

		UE_LOG(LogTemp, Warning, TEXT("Assigned Tag %s to %s"), *TagString, *start_point->GetName());
	}

	TArray<AActor*> goal;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGoalActor::StaticClass(), goal);

	for (int32 j = 0; j < goal.Num(); j++)
	{
		AGoalActor* goal_point = Cast<AGoalActor>(goal[j]);

		if (!goal_point)
		{
			continue;
		}

		FString TagString = FString::Printf(TEXT("GoalPoint%d"), j);
		FName NewTag(TagString);

		goal_point->Tags.Add(NewTag);

		UE_LOG(LogTemp, Warning, TEXT("Assigned Tag %s to %s"), TagString, *goal_point->GetName());
	}

	RegisterServerToAPI();	//APIServerに情報を登録する
}

void AsazandoraGameMode::Tick(float DeltaTime)
{
	//サーバーが終了していないことをAPIサーバーに通知する
	Time_count += DeltaTime;
	if (Time_count >= 5.0f)
	{
		SendHeartbeatToAPI();
		Time_count = 0.0f;
		UE_LOG(LogTemp, Warning, TEXT("Tick"));
	}
}

void AsazandoraGameMode::ClearCheck(AMyPlayerState* p)
{
	// プレイヤーの買い物リストが全て達成済みかどうか
	if (p->Is_Cleared())
	{
		// テキストの表示
		UE_LOG(LogTemp, Warning, TEXT("Player is Goal"));
	}
	else
	{
		// テキストの表示
		UE_LOG(LogTemp, Warning, TEXT("Player is not Goal"));
	}
}

// 全てのプレイヤーのロードが完了したかどうか
void AsazandoraGameMode::CheckAllPlayersLoaded()
{
	UE_LOG(LogTemp, Log, TEXT("[GM CheckAllPlayersLoaded] Checking players..."));

	// 1フレーム後にチェック（レプリケーションが追いつく）
	FTimerHandle TimerHandle;

	// 
	GetWorldTimerManager().SetTimer(
		TimerHandle,
		[this]()
		{
			bool bAllReady = true;
			int32 ready_player = 0;

			// worldに存在しているプレイヤーを取得
			for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
			{
				// 対象のプレイヤーコントローラーの取得
				if (APlayerController* PC = It->Get())
				{
					// プレイヤーコントローラーからAMPlayerStateを取得
					if (AMyPlayerState* PS = PC->GetPlayerState<AMyPlayerState>())
					{
						UE_LOG(LogTemp, Log, TEXT(" - PC=%s PS=%s is_loaded=%d"), *PC->GetName(), PS ? *PS->GetName() : TEXT("null"), PS ? PS->is_loaded : 0);

						// player_stateに存在するis_loadedがtrueじゃなければ中に入る
						if (!PS->is_loaded)
						{
							bAllReady = false;
							break;
						}

						ready_player++;

					}
				}
			}

			if (bAllReady)
			{
				start_flg = true;
			}

			/*if (bAllReady && ready_player >= 2)
			{
				UE_LOG(LogTemp, Log, TEXT("全プレイヤーのロード完了。試合開始！"));
				for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
				{
					if (AMyPlayerController* PC = Cast<AMyPlayerController>(It->Get()))
					{
							PC->Client_StartGame();
					}
				}
			}*/
		},
		0.1f,  // 0.1秒遅らせる
		false
	);
}

void AsazandoraGameMode::Start_Game()
{
	if (!start_flg)
	{
		UE_LOG(LogTemp, Log, TEXT("start_flg = false!!"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("全プレイヤーのロード完了。試合開始！"));
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (AMyPlayerController* PC = Cast<AMyPlayerController>(It->Get()))
		{
			PC->Client_StartGame();
		}
	}

	//Multicast_StartGame();
}

// 全クライアントでゲームを開始する関数
//void AsazandoraGameMode::Multicast_StartGame_Implementation()
//{
//	for (FConstPlayerControllerIterator it = GetWorld()->GetPlayerControllerIterator(); it; ++it)
//	{
//		AMyPlayerController* my_controller = Cast<AMyPlayerController>(it->Get());
//		if (!my_controller)
//		{
//			continue;
//		}
//
//		AMyPlayerState* player_state = my_controller->GetPlayerState<AMyPlayerState>();
//		if (!player_state)
//		{
//			continue;
//		}
//
//		player_state->My_State_Initialize();
//
//		if (HasAuthority())
//		{
//			player_state->My_State_Initialize();
//		}
//
//		if (my_controller->IsLocalController())
//		{
//			// ホスト（ListenServer）含め、実際に画面を持つ人だけ
//			my_controller->Create_HUDWidget();
//		}
//	}
//}

AActor* AsazandoraGameMode::FindPlayerStart_Implementation(AController* player, const FString& IncomingName)
{
	// 参照したプレイヤーがサーバーではなければ
	//if (player->HasAuthority())
	//{
	//	NextPlayerIndex++;			// 次のプレイヤーのためにインクリメント
	//	UE_LOG(LogTemp, Warning, TEXT("NextPlayerIndex : %d") ,NextPlayerIndex);
	//}

	// 次にスポーンするプレイヤーのインデックスを格納
	const int32 CurrentPlayerIndex = NextPlayerIndex;

	// 探したいPlayerStartのタグ作成	例）CurrentPlayerIndexが0ならStartPoint_0を作成、1ならStartPoint_1になる
	const FName TargetTag = FName(*FString::Printf(TEXT("StartPoint_%d"), CurrentPlayerIndex));
	/*const FName TargetTag = FName(*FString::Printf(TEXT("%d"), CurrentPlayerIndex));*/

	// ワールドに存在する全てのPlayerStartを探す	Itはイテレーター（要素にアクセスできるポインタ。++Itをすると次の要素に移動出来たりする）
	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		// StartPointに現在のポインタを格納する
		APlayerStart* StartPoint = *It;

		// TargetTagと比較しているPlayerStartが持つ据えてのタグを出力
		FString AllTags = TEXT("");
		for (const FName& Tag : StartPoint->Tags)
		{
			AllTags += Tag.ToString() + TEXT(" ");

			UE_LOG(LogTemp, Warning, TEXT("StartPoint->Tags.Num() = %d"), StartPoint->Tags.Num());

			// ターゲットタグとの直接比較結果を出力
			if (Tag == TargetTag)
			{
				// 論理的にはここに入るはずなのに、ActorHasTagで通過しない場合は問題あり
				UE_LOG(LogTemp, Error, TEXT("TAG MATCH FOUND via direct FName comparison: %s"), *Tag.ToString());
				return StartPoint;
			}
		}

		UE_LOG(LogTemp, Error, TEXT("StartPoint Name = %s"), *StartPoint->GetName());

		// どのPlayerStartを探しているか、どのPlayerStartをチェックしているかを出力
		UE_LOG(LogTemp, Log, TEXT("Checking PlayerStart at %s. TargetTag: %s. Actor Tags: %s"),
			*StartPoint->GetActorLocation().ToString(),
			*TargetTag.ToString(),
			*AllTags);

		// StartPointが探しているタグ(TargetTag)と同じなら
		//if (StartPoint->GetActorTags().HasTag(TargetTag))
		//{
		//	// StartPointを返り値として渡す
		//	return StartPoint;
		//}
	}

	// なかった場合はログを出力
	UE_LOG(LogTemp, Warning, TEXT("PlayerStart with tag %s not found. Usingdefault PlayerStart selection."), *TargetTag.ToString());

	// デフォルトのスポーン地点を返り値として渡す
	return Super::FindPlayerStart_Implementation(player, IncomingName);
}

void AsazandoraGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	//ログアウトしたらNextSpawnIndexを減らす
	if (NextPlayerIndex > 0)
	{
		NextPlayerIndex--;
	}
	UE_LOG(LogTemp, Warning, TEXT("NextSpawnIndex = %d"), NextPlayerIndex);

	UpdateServerInfoOnAPI();
	RegisterServerToAPI();
}

void AsazandoraGameMode::RegisterServerToAPI()
{
	int32 Port = GetWorld()->URL.Port;
	FString ServerName = FString::Printf(TEXT("Server:%d"), Port);
	FString ServerAddress = Get_IPAddress(); // 実際は外部IPを取得する方法もあり
	int32 PlayerCount = NextPlayerIndex;
	int32 MaxPlayers = 4;

	UE_LOG(LogTemp, Warning, TEXT("Address:	%s"), *ServerAddress);
	UE_LOG(LogTemp, Warning, TEXT("Address"));

	// JSONデータ作成
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField(TEXT("name"), ServerName);
	JsonObject->SetStringField(TEXT("address"), ServerAddress);
	JsonObject->SetNumberField(TEXT("playerCount"), PlayerCount);
	JsonObject->SetNumberField(TEXT("maxPlayers"), MaxPlayers);
	JsonObject->SetBoolField(TEXT("gameplay"), gameplay);

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	// HTTPリクエスト作成
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();

	if (!IsRunningDedicatedServer())
	{
		UMyGameInstance* GI = GetGameInstance<UMyGameInstance>();
		FString APIServerIP;

		if (GI)
		{
			APIServerIP = GI->APIServerIP;
			if (APIServerIP.IsEmpty())
			{
				APIServerIP = TEXT("127.0.0.1");
				UE_LOG(LogTemp, Warning, TEXT("APIServer:127.0.0.1"));
			}
			UE_LOG(LogTemp, Warning, TEXT("GameMode GI OK!!"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("GameMode GI No..."));
		}
		URL = FString::Printf(TEXT("http://%s:3000/register"), *APIServerIP);
	}
	else
	{
		URL = FString::Printf(TEXT("http://127.0.0.1:3000/register"));
	}

	Request->SetURL(URL); // Node.js APIサーバーのURL
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

FString AsazandoraGameMode::Get_IPAddress()
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

void AsazandoraGameMode::UpdateServerInfoOnAPI()
{
	// APIエンドポイント
	//FString Url = TEXT("http://127.0.0.1:3000/api/servers/update");
	int32 Port = GetWorld()->URL.Port;
	FString ServerName = FString::Printf(TEXT("Server:%d"), Port);
	int32 PlayerCount = NextPlayerIndex;
	int32 MaxPlayers = 4;

	// JSONオブジェクト作成
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField(TEXT("name"), ServerName);
	JsonObject->SetNumberField(TEXT("playerCount"), PlayerCount);
	JsonObject->SetNumberField(TEXT("maxPlayers"), MaxPlayers);
	JsonObject->SetBoolField(TEXT("gameplay"), gameplay);

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	// HTTPリクエスト作成
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(URL);
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

void AsazandoraGameMode::Set_Gameplay(bool flag)
{
	gameplay = flag;
}

//APIサーバーにサーバー起動中の通知を送る
void AsazandoraGameMode::SendHeartbeatToAPI()
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request =
		FHttpModule::Get().CreateRequest();

	Request->SetURL(URL);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

	// JSONデータ作成（ServerIdなど）
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField("serverId", Get_IPAddress());

	FString Body;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Body);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	Request->SetContentAsString(Body);

	Request->ProcessRequest();
}