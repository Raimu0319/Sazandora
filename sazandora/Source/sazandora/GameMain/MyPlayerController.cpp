// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerController.h"

AMyPlayerController::AMyPlayerController()
{

}

void AMyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (HUDWidget)
	{
		HUDWidget->AddToViewport();

		// PlayerStateの紐づけ
		AMyPlayerState* ps = GetPlayerState<AMyPlayerState>();
		HUDWidget->InitializeWidget(ps);
	}
}


