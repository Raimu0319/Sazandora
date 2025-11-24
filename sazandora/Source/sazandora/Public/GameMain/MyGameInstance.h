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
    virtual void Init() override;
    virtual void Shutdown() override;

private:
	void StartAPIServer();
	void StopAPIServer();
	FProcHandle NodeProcessHandle;
};
