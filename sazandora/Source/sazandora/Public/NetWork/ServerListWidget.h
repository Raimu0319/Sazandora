// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "GameMain/MyGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "UnableConnectWidget.h"
#include "ServerListWidget.generated.h"

/**
 * 
 */
UCLASS()
class SAZANDORA_API UServerListWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:

    UServerListWidget(const FObjectInitializer& ObjectInitializer);

    UFUNCTION(BlueprintCallable)
    void Setup(const FString& ServerName, const FString& IP, int PlayerCount, bool gameplay, TSubclassOf<class UUnableConnectWidget> point);

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UnableConnectWidget")
    TSubclassOf<class UUnableConnectWidget>UnableConnectWidget;


    FString ServerIP;

    UFUNCTION()
    void OnConnectClicked();

private:
    bool ConnectBlock = false;
    int32 PlayerCount = 0;
    bool GamePlay = false;
};
