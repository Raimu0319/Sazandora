// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h" 
#include "Item_Type.h"
#include "MyPlayerState.h"
#include "HUDWidget.generated.h"

/**
 * Components
 */
UCLASS()
class SAZANDORA_API UHUDWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadOnly)
	AMyPlayerState* player_state;

	UPROPERTY(meta = (BindWidget))
	UImage* A_Button;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ItemBuy_Text;

	UFUNCTION(BlueprintCallable)
	void InitializeWidget(AMyPlayerState* PlayerState);
	
	UFUNCTION()
	void SetVisible(bool is_visible);
	
	UFUNCTION(BlueprintImplementableEvent)
	void RefreshUI();
};
