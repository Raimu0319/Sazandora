// Copyright Epic Games, Inc. All Rights Reserved.

#include "sazandoraGameMode.h"
#include "GameFramework/GameStateBase.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/PlayerController.h"
#include "GameMain/Main_Character.h"
#include "GameMain/MyPlayerState.h"
#include "GameMain/MyPlayerController.h"
#include "UObject/ConstructorHelpers.h"

AsazandoraGameMode::AsazandoraGameMode()
{
	// カスタムMainCharacterの設定
	DefaultPawnClass = AMain_Character::StaticClass();

	// カスタムのPlayerStateクラスの指定
	PlayerStateClass = AMyPlayerState::StaticClass();

	// カスタムPlayerControllerの設定
	PlayerControllerClass = AMyPlayerController::StaticClass();
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

	if (gs->PlayerArray.Num() == 1)
	{
		AMyPlayerState* ps = NewPlayer->GetPlayerState<AMyPlayerState>();

		if (ps)
		{
			ps->is_host = true;
		}
	}
}

void AsazandoraGameMode::ClearCheck(AMyPlayerState* p)
{
	// プレイヤーの買い物リストが全て達成済みかどうか
	if (p->Is_Cleared())
	{
		// テキストの表示
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, TEXT("Player is Goal"));
	}
	else
	{
		// テキストの表示
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, TEXT("Player is not Goal"));
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
void AsazandoraGameMode::Multicast_StartGame_Implementation()
{
	for (FConstPlayerControllerIterator it = GetWorld()->GetPlayerControllerIterator(); it; ++it)
	{
		AMyPlayerController* my_controller = Cast<AMyPlayerController>(it->Get());
		if (!my_controller)
		{
			continue;
		}

		AMyPlayerState* player_state = my_controller->GetPlayerState<AMyPlayerState>();
		if (!player_state)
		{
			continue;
		}

		player_state->My_State_Initialize();
	}
}

AActor* AsazandoraGameMode::FindPlayerStart_Implementation(AController* player, const FString& IncomingName)
{
	// 次にスポーンするプレイヤーのインデックスを格納
	const int32 CurrentPlayerIndex = NextPlayerIndex;
	NextPlayerIndex++;			// 次のプレイヤーのためにインクリメント

	// 探したいPlayerStartのタグ作成	例）CurrentPlayerIndexが0ならStartPoint_0を作成、1ならStartPoint_1になる
	const FName TargetTag = FName(*FString::Printf(TEXT("StartPoint_%d"), CurrentPlayerIndex));

	// ワールドに存在する全てのPlayerStartを探す	Itはイテレーター（要素にアクセスできるポインタ。++Itをすると次の要素に移動出来たりする）
	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		// StartPointに現在のポインタを格納する
		APlayerStart* StartPoint = *It;

		// StartPointが探しているタグ(TargetTag)と同じなら
		if (StartPoint && StartPoint->ActorHasTag(TargetTag))
		{
			// StartPointを返り値として渡す
			return StartPoint;
		}
	}

	// なかった場合はログを出力
	UE_LOG(LogTemp, Warning, TEXT("PlayerStart with tag %s not found. Usingdefault PlayerStart selection."), *TargetTag.ToString());

	// デフォルトのスポーン地点を返り値として渡す
	return Super::FindPlayerStart_Implementation(player, IncomingName);
}