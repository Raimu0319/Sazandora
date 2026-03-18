// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "MyGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class SAZANDORA_API UMyGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:

	UPROPERTY(BlueprintReadWrite)
	FString APIServerIP;

    virtual void Init() override;
    virtual void Shutdown() override;
	void StartAPIServer();
	void StopAPIServer();
	void Set_PlayerCount(int32_t count);
	//起動したサーバーのハンドルをセットするための関数
	void Set_ServerProcess(FProcHandle handle);
private:
	FProcHandle NodeProcessHandle;
	FTimerHandle HeartbeatTimer;
	int32_t PlayerCount;
	UFUNCTION()
	void OnServerPreExit();
	void UpdateServerInfoOnAPI();
	bool CleanedUp = false;
	//サーバーのハンドル保持用変数
	TArray<FProcHandle> ServerProcessHandle;

};
