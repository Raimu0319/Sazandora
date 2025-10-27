// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerState.h"

// 初期化
AMyPlayerState::AMyPlayerState()
{

}

// BeginPlay
void AMyPlayerState::BeginPlay()
{
	Super::BeginPlay();
}

// ランダムで購入するアイテムを渡す処理
TArray<E_ITEM_TYPE> AMyPlayerState::Random_Item()
{
	player_buy_list.Empty();						// 初期化
	player_buy_list.Reserve(D_MAX_BUY_LISTSIZE);	// メモリの確保

	// アイテムリストのサイズ分追加
	while (player_buy_list.Num() < D_MAX_BUY_LISTSIZE)
	{
		// アイテムタイプ
		E_ITEM_TYPE Item_Type = E_ITEM_TYPE::E_NONE;

		// ランダムで0～アイテムリストの間の数値を取得
		int32 r = FMath::RandRange(1, D_MAX_ITEM_TYPE - 1);

		// rの数値に対応するEnumに変換する
		switch (r)
		{
		case 1:
			Item_Type = E_ITEM_TYPE::E_JUICE;
			break;

		case 2:
			Item_Type = E_ITEM_TYPE::E_HAMBRGER;
			break;

		case 3:
			Item_Type = E_ITEM_TYPE::E_DONUT;
			break;

		case 4:
			Item_Type = E_ITEM_TYPE::E_POPCORN;
			break;

		default:
			break;
		}

		// 同じアイテムが入っているかどうか
		if (!player_buy_list.Contains(Item_Type))
		{
			// なければ配列に追加
			player_buy_list.Add(Item_Type);
		}
	}

	// アイテムリストを渡す
	return player_buy_list;

}