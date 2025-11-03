// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerController.h"

AMyPlayerController::AMyPlayerController()
{
	static ConstructorHelpers::FClassFinder<UHUDWidget> widgetclass(TEXT("/Game/ThirdPerson/widget/BP_HUDWidget"));

	if (widgetclass.Succeeded())
	{
		HUDWidget_class = widgetclass.Class;
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("HUDWidget is not find..."));
	}

}

void AMyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (HUDWidget_class)
	{
		UHUDWidget* HUDWidget = CreateWidget<UHUDWidget>(this, HUDWidget_class);
		HUDWidget->AddToViewport();

		// PlayerStateの紐づけ
		AMyPlayerState* ps = GetPlayerState<AMyPlayerState>();
		HUDWidget->InitializeWidget(ps);
	}
}


