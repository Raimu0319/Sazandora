// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Item_Type.h"
#include "MyPlayerState.h"
#include "HUDWidget.generated.h"

/**
 * 
 */
UCLASS()
class SAZANDORA_API UHUDWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadOnly)
	AMyPlayerState* player_state;

	UFUNCTION(BlueprintCallable)
	void InitializeWidget(AMyPlayerState* PlayerState);
	
	
	UFUNCTION(BlueprintImplementableEvent)
	void RefreshUI();
};
