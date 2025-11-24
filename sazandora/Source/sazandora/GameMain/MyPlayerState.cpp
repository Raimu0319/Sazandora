// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerState.h"
#include "HUDWidget.h"
#include "sazandora/sazandoraGameMode.h"
#include "MyPlayerController.h"
#include "Net/UnrealNetwork.h"

// 初期化
AMyPlayerState::AMyPlayerState()
{
	bReplicates = true;
}

// レプリケーションの設定
void AMyPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMyPlayerState, player_buy_list);
	DOREPLIFETIME(AMyPlayerState, buylist_crear);
	DOREPLIFETIME(AMyPlayerState, is_loaded);
	DOREPLIFETIME(AMyPlayerState, is_host);
}

//// ロードが完了したどうか
//void AMyPlayerState::Server_SetLoaded_Implementation(bool load_flg)
//{
//	is_loaded = load_flg;
//	OnRep_IsLoaded();
//}

void AMyPlayerState::OnRep_IsLoaded()
{
	UE_LOG(LogTemp, Log, TEXT("ロード中"));
}

// BeginPlay
void AMyPlayerState::BeginPlay()
{
	Super::BeginPlay();
}

// Playerの初期化
void AMyPlayerState::My_State_Initialize()
{
	/*if (!HasAuthority())
	{
		return;
	}*/

	if (HasAuthority())
	{
		buylist_crear.SetNum(D_MAX_BUY_LISTSIZE);

		// 要素の初期化
		buylist_crear = { false, false, false };

		// アイテムリストの作成
		Random_Item();

		//// 数値をFStringに変換
		for (int32 i = 0; i < 3; i++)
		{
			FString LogMessage = FString::Printf(TEXT("配列の要素: %d"), player_buy_list[i]);
			UE_LOG(LogTemp, Log, TEXT("%s"), *LogMessage);
		}
	}
}

//void AMyPlayerState::Server_Random_Item_Implementation()
//{
//	Random_Item();
//}

// ロードが完了したどうか
void AMyPlayerState::Server_SetLoaded_Implementation(bool load_flg)
{
	AController* OwnerController = Cast<AController>(GetOwner());
	UE_LOG(LogTemp, Log, TEXT("[PS Server_SetLoaded] %s called by %s | load=%d | HasAuthority=%d"),
		*GetName(),
		OwnerController ? *OwnerController->GetName() : TEXT("None"),
		load_flg,
		HasAuthority());
	// do NOT call OnRep manually

	is_loaded = load_flg;

	// サーバーのみ確認
	if (is_loaded && HasAuthority())
	{
		//// クライアント自身に通知
		//Client_OnLoaded();

		// サーバーのみ実行
		// HasAuthorityでサーバーかクライアントかを調べる
		// trueの場合はホストまたはサーバーでの実行
		// falseの場合はクライアントでの実行
		//if (HasAuthority())
		//{
		//	AsazandoraGameMode* game_mode = GetWorld()->GetAuthGameMode<AsazandoraGameMode>();

		//	// nullチェック
		//	if (game_mode)
		//	{
		//		game_mode->CheckAllPlayersLoaded();
		//	}
		//}

			// タイマーハンドルの設定
			// timerを一位に選別し、後で操作（停止、チェックなど）するために使用するハンドル
			FTimerHandle DelayHandle;

			// ゲームワールドのタイマーマネージャーに、新しいtimerを設定するように指示
			GetWorld()->GetTimerManager().SetTimer(DelayHandle, [this]()
				{
					// タイマーが切れたときに実行する関数
					// sazandoraGameModeを参照できるかどうか
					if (AsazandoraGameMode* GM = GetWorld()->GetAuthGameMode<AsazandoraGameMode>())
					{
						// 参照に成功した場合全てのプレイヤーがチェックに成功しているかを確認
						GM->CheckAllPlayersLoaded();
					}
				}, 0.4f, false);		// 0.3 : 実行を遅延する時間		// ループをするかどうか
	}

}

void AMyPlayerState::Client_OnLoaded_Implementation()
{
	UE_LOG(LogTemp, Log, TEXT("ロード中"));

	UE_LOG(LogTemp, Log, TEXT("[PS OnRep_IsLoaded] %s -> %d | NetMode=%d"), *GetName(), is_loaded, (int)GetNetMode());
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
	//OnItemUpdated();
}

void AMyPlayerState::OnRep_BuyList()
{
	if (!IsNetMode(NM_Client))
	{
		return; // サーバー側では UI 更新しない
	}

	// クライアント側だけで走るUI更新処理
	OnItemUpdated();
}

// クリアチェック
bool AMyPlayerState::Is_Cleared() const
{
	// クリアリストの中身にfalseがあるかどうか
	if (!buylist_crear.Contains(false))
	{
		// ないならtrue
		return true;
	}

	// ある場合はfalse
	return false;
}

// ウィジェットの更新
void AMyPlayerState::OnItemUpdated()
{
	// クライアントのみUI更新　ホストにはwidgetが存在しないためクライアントのみの実行
	if (GetNetMode() != NM_Client)
	{
		return;
	}

	AMyPlayerController* pc = Cast<AMyPlayerController>(GetOwner());

	if (pc)
	{
		pc->Refresh_UI();
	}
}