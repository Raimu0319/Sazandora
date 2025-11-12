// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MyGameInstance.generated.h"

USTRUCT(BlueprintType)
struct FServerInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString ServerName;

    UPROPERTY(BlueprintReadWrite)
    FString IPAddress;

    UPROPERTY(BlueprintReadWrite)
    int32 CurrentPlayers;

    UPROPERTY(BlueprintReadWrite)
    int32 MaxPlayers;
};

/**
 * 
 */
UCLASS()
class SAZANDORA_API UMyGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
    void MyGameInstance();

    UPROPERTY(BlueprintReadWrite)
    TArray<FServerInfo> ServerList;

    void AddServerInfo(const FServerInfo& Info);

    const TArray<FServerInfo>& GetServerList() const;
};
