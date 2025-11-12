// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameMain/MyGameInstance.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "ServerListWidget.generated.h"

/**
 * 
 */
UCLASS()
class SAZANDORA_API UServerListWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
    UFUNCTION(BlueprintCallable)
    void Setup(const FString& ServerName, const FString& IP, int PlayerCount);

protected:
    // Blueprintè„ÇÃTextBlockÇ∆ButtonÇC++Ç…ïRïtÇØÇÈ
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* ServerNameText;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* ServerIPAddress;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* PlayerCountText;

    UPROPERTY(meta = (BindWidget))
    class UButton* ConnectButton;

    FString ServerIP;

    UFUNCTION()
    void OnConnectClicked();
};
