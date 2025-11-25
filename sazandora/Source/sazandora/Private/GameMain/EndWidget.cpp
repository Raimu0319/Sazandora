// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMain/EndWidget.h"
#include "Kismet/GameplayStatics.h"

void UEndWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (TitleBack)
	{
		OnTitleBackButtonClicked();
	}
}

void UEndWidget::OnTitleBackButtonClicked()
{
	FString ServerArgs = FString::Printf(TEXT("/Game/ThirdPerson/Maps/Login_Map?game=/Script/sazandora.LoginMenuGameMode"));
	UGameplayStatics::OpenLevel(GetWorld(), FName(*FString::Printf(TEXT("%s"), *ServerArgs)));
}