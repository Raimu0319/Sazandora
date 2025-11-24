// Fill out your copyright notice in the Description page of Project Settings.


#include "NetWork/MyGameViewportClient.h"
#include "Engine/Engine.h"
#include "../../Public/GameMain/MyGameInstance.h"

UMyGameViewportClient::UMyGameViewportClient()
{
    UE_LOG(LogTemp, Warning, TEXT("UMyGameViewportClient Constructor"));
}

void UMyGameViewportClient::CloseRequested(FViewport* InViewport)
{
   
}


