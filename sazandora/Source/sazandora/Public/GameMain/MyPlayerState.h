// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Item_Type.h"
#include "MyPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class SAZANDORA_API AMyPlayerState : public APlayerState
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:
	// コンストラクタ
	AMyPlayerState();
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_BuyList)
	TArray<E_ITEM_TYPE> player_buy_list;

	// サーバーで変数の値を変更し、その変更をネットワーク経由でクライアントにレプリケートする
	// クライアントは、値を受け取り、自身のローカル変数に格納する
	// その直後に、指定された通知関数(RepNotify関数)をクライアント上で自動的に呼び出す
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_BuyList)
	TArray<bool> buylist_crear;			//買い物達成状況

	UPROPERTY(Replicated)
	bool is_host = false;				// ホストかどうか

	UPROPERTY(Replicated)
	int player_number = 0;				// プレイヤーの番号

	UPROPERTY(Replicated)
	bool is_player_clear = false;		// 買い物をすべて達成しているかどうか

	// クライアントからサーバーへ関数の呼び出し
	UFUNCTION(Server, Reliable)
	void Server_SetLoaded(bool  load_flg);			// ロードが完了したどうか

	UFUNCTION()
	void My_State_Initialize();			// PlayerStateの初期化　

	UFUNCTION()
	void Set_PlayerNumber(int32 number);	// プレイヤー番号の格納

	void Random_Item();						// ランダムで購入するアイテムを渡す処理

	void Buy_Item(int i, bool flg);			// アイテム購入処理

	UFUNCTION()
	void Set_Is_PlayerClear(bool flg);		// 買い物を全て終わらせているか

	UFUNCTION()
	void OnRep_BuyList();					// buylist_crearに変更があった場合にUIを更新する通知を送る

	bool Is_Cleared() const;

	// Blueprintへの通知
	void OnItemUpdated();					// ウィジェットの更新

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Client, Reliable)
	void Client_OnLoaded();

	// 変数が変わった場合にOnRep_IsLoadedを実行する
	UPROPERTY(ReplicatedUsing = OnRep_IsLoaded)
	bool is_loaded = false;

protected:
	UFUNCTION()
	void OnRep_IsLoaded();					// ロードが完了したどうか
};