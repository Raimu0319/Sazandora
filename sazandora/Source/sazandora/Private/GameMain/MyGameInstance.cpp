// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMain/MyGameInstance.h"

void UMyGameInstance::MyGameInstance()
{

}

void UMyGameInstance::AddServerInfo(const FServerInfo& Info)
{
	ServerList.Add(Info);
}

const TArray<FServerInfo>& UMyGameInstance::GetServerList() const
{
    return ServerList;
}