// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "../Public/NetWork/LogInWidget.h"
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "NetWorkGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class SAZANDORA_API ANetWorkGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

	ANetWorkGameModeBase();

public:

	virtual void BeginPlay() override;

	//サーバー起動（listenサーバー）
	UFUNCTION(BlueprintCallable)
	void StartListenServer();

	//クライアント接続
	UFUNCTION(BlueprintCallable)
	void ConnectToServer(const FString& IPAddress);	

};
