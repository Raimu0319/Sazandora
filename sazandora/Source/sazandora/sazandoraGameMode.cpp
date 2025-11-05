// Copyright Epic Games, Inc. All Rights Reserved.

#include "sazandoraGameMode.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/PlayerController.h"
#include "GameMain/Main_Character.h"
#include "GameMain/MyPlayerState.h"
#include "GameMain/MyPlayerController.h"
#include "UObject/ConstructorHelpers.h"

AsazandoraGameMode::AsazandoraGameMode()
{
	DefaultPawnClass = AMain_Character::StaticClass();

	// カスタムのPlayerStateクラスの指定
	PlayerStateClass = AMyPlayerState::StaticClass();

	PlayerControllerClass = AMyPlayerController::StaticClass();
}

// 全てのプレイヤーのロードが完了したかどうか
void AsazandoraGameMode::CheckAllPlayersLoaded()
{
	bool all_ready = true;

	for (FConstPlayerControllerIterator it = GetWorld()->GetPlayerControllerIterator(); it; ++it)
	{
		APlayerController* player_controller = it->Get();

		if (player_controller)
		{
			AMyPlayerState* player_state = player_controller->GetPlayerState<AMyPlayerState>();
			if (!player_state || !player_state->is_loaded)
			{
				all_ready = false;
				break;
			}
		}
	}

	if (all_ready)
	{
		UE_LOG(LogTemp, Log, TEXT("全プレイヤーのロード完了。試合開始！"));
		Multicast_StartGame();
	}

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

		if (HasAuthority())
		{
			player_state->My_State_Initialize();
		}

		if (my_controller->IsLocalController())
		{
			// ホスト（ListenServer）含め、実際に画面を持つ人だけ
			my_controller->Create_HUDWidget();
		}
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