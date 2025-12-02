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
	// その直後に、指定された通知関数(RepNotify関数)を空ライアン途上腕自動的に呼び出す
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_BuyList)
	TArray<bool> buylist_crear;			//買い物達成状況

	//UPROPERTY()
	//class UHUDWidget* wiget_p = nullptr;

	UPROPERTY(Replicated)
	bool is_host = false;

	UPROPERTY(Replicated)
	int player_number = 0;

	//// 変数が変わった場合にOnRep_IsLoadedを実行する
	//UPROPERTY(Replicated)
	//bool is_loaded = false;

	// クライアントからサーバーへ関数の呼び出し
	UFUNCTION(Server, Reliable)
	void Server_SetLoaded(bool  load_flg);

	UFUNCTION()
	void My_State_Initialize();

	UFUNCTION()
	void Set_PlayerNumber(int32 number);

	//UFUNCTION(Server, Reliable)
	//void Server_Random_Item();

	void Random_Item();

	void Buy_Item(int i, bool flg);

	UFUNCTION()
	void OnRep_BuyList();

	bool Is_Cleared() const;

	// Blueprintへの通知
	void OnItemUpdated();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Client, Reliable)
	void Client_OnLoaded();

	// 変数が変わった場合にOnRep_IsLoadedを実行する
	UPROPERTY(ReplicatedUsing = OnRep_IsLoaded)
	bool is_loaded = false;


protected:
	UFUNCTION()
	void OnRep_IsLoaded();

};