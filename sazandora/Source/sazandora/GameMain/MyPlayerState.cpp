// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerState.h"
#include "HUDWidget.h"
#include "Net/UnrealNetwork.h"

// 初期化
AMyPlayerState::AMyPlayerState()
{

}

// ロードが完了したどうか
void AMyPlayerState::Server_SetLoaded_Implementation(bool load_flg)
{
	is_loaded = load_flg;
	OnRep_IsLoaded();
}

void AMyPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMyPlayerState, player_buy_list);
	DOREPLIFETIME(AMyPlayerState,buylist_crear);
	DOREPLIFETIME(AMyPlayerState,is_loaded);
}

void AMyPlayerState::OnRep_IsLoaded()
{
	UE_LOG(LogTemp, Log, TEXT("ロード中"));
}

// BeginPlay
void AMyPlayerState::BeginPlay()
{
	Super::BeginPlay();
}

void AMyPlayerState::My_State_Initialize()
{
	buylist_crear.SetNum(D_MAX_BUY_LISTSIZE);

	buylist_crear = { false, false, false };

	Random_Item();
}

// ランダムで購入するアイテムを渡す処理
void AMyPlayerState::Random_Item()
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
}

// アイテム購入処理
void AMyPlayerState::Buy_Item(int i, bool flg)
{
	buylist_crear[i] = flg;
	OnItemUpdated();
}

bool AMyPlayerState::Is_Cleared() const
{
	if (!buylist_crear.Contains(false))
	{
		return true;
	}

	return false;
}

// ウィジェットの更新
void AMyPlayerState::OnItemUpdated()
{
	wiget_p->RefreshUI();
}