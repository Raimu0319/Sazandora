// Copyright Epic Games, Inc. All Rights Reserved.

#include "sazandoraGameMode.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "Public/GameMain/GoalActor.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/PlayerController.h"
#include "Public/GameMain/Main_Character.h"
#include "Public/GameMain/MyPlayerState.h"
#include "Public/GameMain/MyPlayerController.h"
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
}

// プレイヤーがログインした際の処理
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

	// GameStateの取得
	AGameStateBase* gs = GameState;
	if (!gs)			// nullptrチェック
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
	UMyGameInstance* GI = GetWorld()->GetGameInstance<UMyGameInstance>();
	GI->Set_PlayerCount(NextPlayerIndex);
	UpdateServerInfoOnAPI();
	//RegisterServerToAPI();
}

// 一度だけ実行される処理
void AsazandoraGameMode::BeginPlay()
{
	Super::BeginPlay();

	TArray<AActor*> starts;
	// ワールドに配置されているスタートクラスのオブジェクトを取得する
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), starts);

	for (int32 i = 0; i < starts.Num(); i++)
	{
		APlayerStart* start_point = Cast<APlayerStart>(starts[i]);

		if (!start_point)
		{
			continue;
		}

		// タグの生成
		FString TagString = FString::Printf(TEXT("StartPoint_%d"), i);
		FName NewTag(*TagString);

		/*start_point->Tags.Add(NewTag);*/

		// タグが付いていなければタグをつける
		if (!start_point->Tags.Contains(NewTag))
		{
			start_point->Tags.Add(NewTag);
		}

		UE_LOG(LogTemp, Warning, TEXT("Assigned Tag %s to %s"), *TagString, *start_point->GetName());

		UE_LOG(LogTemp, Warning, TEXT("---- PlayerStart List ----"));

		for (int32 j = 0; j < starts.Num(); j++)
		{
			UE_LOG(LogTemp, Warning, TEXT("[%d] %s"), j, *starts[j]->GetName());
		}
		//RegisterServerToAPI();
	}

	TArray<AActor*> goal;
	// ワールドに配置されているゴールオブジェクトの取得
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGoalActor::StaticClass(), goal);

	for (int32 j = 0; j < goal.Num(); j++)
	{
		AGoalActor* goal_point = Cast<AGoalActor>(goal[j]);

		if (!goal_point)
		{
			continue;
		}

		// タグの作成
		FString TagString = FString::Printf(TEXT("GoalPoint%d"), j);
		FName NewTag(TagString);

		// タグの追加
		goal_point->Tags.Add(NewTag);

		UE_LOG(LogTemp, Warning, TEXT("Assigned Tag %s to %s"), *TagString, *goal_point->GetName());
	}

	if (IsRunningDedicatedServer())
	{
		//ゲームインスタンスのポインタ取得
		UMyGameInstance* GI = GetWorld()->GetGameInstance<UMyGameInstance>();
		GI->Set_PlayerCount(NextPlayerIndex);
		RegisterServerToAPI();	//APIServerに情報を登録する
	}
}

// クリア判定関数
void AsazandoraGameMode::ClearCheck(AMyPlayerState* p)
{
	// プレイヤーの買い物リストが全て達成済みかどうか
	if (p->Is_Cleared())
	{
		// テキストの表示
		UE_LOG(LogTemp, Warning, TEXT("Player is Goal"));

		// クリアフラグをセット
		p->Set_Is_PlayerClear(true);

		UE_LOG(LogTemp, Log, TEXT("プレイヤーがゴールしました。試合終了！"));

		// プレイヤーコントローラーの取得をプレイヤーの人数分繰り返す
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			// プレイヤーコントローラーからAMyPlayerControllerを取得する
			if (AMyPlayerController* PC = Cast<AMyPlayerController>(It->Get()))
			{
				// PlayerStaterを取得
				AMyPlayerState* Target_player = PC->GetPlayerState<AMyPlayerState>();

				if (!IsValid(Target_player))
				{
					UE_LOG(LogTemp, Error, TEXT("Target_player is INVALID"));
					continue;
				}

				// 勝者判定
				const bool Is_Winner = (Target_player == p);

				// ログ出力
				if (Is_Winner)
				{
					UE_LOG(LogTemp, Warning, TEXT("Is_Winner is true"));
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("Is_Winner is false"));
				}

				// クライアントそれぞれに
				PC->Client_EndGame(Is_Winner);
			}
		}
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

	// タイムハンドルの作成
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
	UE_LOG(LogTemp, Warning, TEXT("StartPoint = %d"), CurrentPlayerIndex);
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

	// ゲーム中フラグがtrueの場合にサーバーの人数が0になった場合
	if (gameplay)
	{
		if (NextPlayerIndex <= 0)
		{
			gameplay = false;
		}
	}

	UMyGameInstance* GI = GetWorld()->GetGameInstance<UMyGameInstance>();
	GI->Set_PlayerCount(NextPlayerIndex);

	UE_LOG(LogTemp, Warning, TEXT("NextSpawnIndex = %d"), NextPlayerIndex);

	UpdateServerInfoOnAPI();
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

	
	//ゲームインスタンスのポインタ取得
	UMyGameInstance* GI = GetGameInstance<UMyGameInstance>();
	FString APIServerIP;

	if (GI)
	{
		//ゲームインスタンスの変数に保持していたIPアドレスを取得する
		if (GI->APIServerIP.IsEmpty())	//インスタンスに保持しているIPアドレスの中身を確認する
		{
			//中身が空だったら、自分のPCを指すループバックアドレスを代入
			APIServerIP = TEXT("127.0.0.1");
		}
		else
		{
			//中身があったら、インスタンスのIPアドレスを代入
			APIServerIP = GI->APIServerIP;
		}
		UE_LOG(LogTemp, Warning, TEXT("GameMode GI OK!!"));
	}
	else
	{
		APIServerIP = TEXT("127.0.0.1");
		UE_LOG(LogTemp, Warning, TEXT("GameMode GI No..."));
	}

	URL = FString::Printf(TEXT("http://%s:3000/register"), *APIServerIP);
	

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

void AsazandoraGameMode::UpdateServerInfoOnAPI()	//APIサーバーに登録している情報の更新処理
{
	UE_LOG(LogTemp, Warning, TEXT("UpdateServerInfoOnAPI"));
	// APIエンドポイント
	int32 Port = GetWorld()->URL.Port;

	UMyGameInstance* GI = GetGameInstance<UMyGameInstance>();
	FString Address;
	if (GI)
	{
		//ゲームインスタンスの変数に保持していたIPアドレスを取得する
		if (GI->APIServerIP.IsEmpty())	//インスタンスに保持しているIPアドレスの中身を確認する
		{
			//中身が空だったら、自分のPCを指すループバックアドレスを代入
			Address = TEXT("127.0.0.1");
		}
		else
		{
			//中身があったら、インスタンスのIPアドレスを代入
			Address = GI->APIServerIP;
		}

		UE_LOG(LogTemp, Warning, TEXT("GIOK_UpdataServer:%s"), *Address);
	}
	else
	{
		Address = TEXT("127.0.0.1");
		UE_LOG(LogTemp, Warning, TEXT("GINO_UpdataServer:%s"), *Address);
	}

	FString Url = FString::Printf(TEXT("http://%s:3000/api/servers/update"), *Address);
	UE_LOG(LogTemp, Warning, TEXT("UpdateServerInfoOnAPI PUT URL = %s"), *Url);
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

void AsazandoraGameMode::Set_Gameplay(bool flag)
{
	gameplay = flag;
}

