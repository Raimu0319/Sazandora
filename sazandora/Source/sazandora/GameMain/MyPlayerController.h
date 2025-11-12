// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "HUDWidget.h"
#include "MyPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class SAZANDORA_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AMyPlayerController();

	UPROPERTY()
	bool bAlreadyNotified = false;

	// プレイヤーのロード完了をサーバーへ通知する関数
	UFUNCTION(BlueprintCallable)
	void NotifyLoaded();

	UFUNCTION()
	void Create_HUDWidget();

	UFUNCTION(Client, Reliable)
	void Client_StartGame();

	// AControllerクラスまたはAPawnクラスでPlayerStateのポインタが
	// クライアントにレプリケートされた時に呼び出されるコールバック関数
	virtual void OnRep_PlayerState() override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<class  UHUDWidget> HUDWidget_class;

	UHUDWidget* HUDWidget_pointer = nullptr;
	
};
