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
	
	UPROPERTY(BlueprintReadOnly,Replicated)
	TArray<E_ITEM_TYPE> player_buy_list;

	UPROPERTY(BlueprintReadOnly,Replicated)
	TArray<bool> buylist_crear;			//買い物達成状況

	void Random_Item();

	void Buy_Item(int i, bool flg);

	bool Is_Cleared() const;

	UPROPERTY()
	class UHUDWidget* wiget_p = nullptr;

	// Blueprintへの通知
	
	void OnItemUpdated();

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
};