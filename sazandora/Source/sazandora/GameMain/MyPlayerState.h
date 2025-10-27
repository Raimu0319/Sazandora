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

public:
	
	UPROPERTY()
	TArray<E_ITEM_TYPE>  player_buy_list;

public:
	// コンストラクタ
	AMyPlayerState();

	TArray<E_ITEM_TYPE> Random_Item();

protected:
	virtual void BeginPlay() override;
	
};