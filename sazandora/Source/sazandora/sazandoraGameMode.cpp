// Copyright Epic Games, Inc. All Rights Reserved.

#include "sazandoraGameMode.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/PlayerController.h"
#include "GameMain/Main_Character.h"
#include "GameMain/MyPlayerState.h"
#include "UObject/ConstructorHelpers.h"

AsazandoraGameMode::AsazandoraGameMode()
{
	DefaultPawnClass = AMain_Character::StaticClass();

	// カスタムのPlayerStateクラスの指定
	PlayerStateClass = AMyPlayerState::StaticClass();
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