// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMain/EndWidget.h"
#include "Kismet/GameplayStatics.h"

void UEndWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (TitleBack)
	{
		TitleBack->OnClicked.AddDynamic(this, &UEndWidget::OnTitleBackButtonClicked);
	}

	TitleBack->SetVisibility(ESlateVisibility::Visible);
}

void UEndWidget::OnTitleBackButtonClicked()
{
	FString ServerArgs = FString::Printf(TEXT("/Game/ThirdPerson/Maps/Login_Map?game=/Script/sazandora.LoginMenuGameMode"));
	UGameplayStatics::OpenLevel(GetWorld(), FName(*FString::Printf(TEXT("%s"), *ServerArgs)));
}

void UEndWidget::Set_ClearFlg(bool flg)
{
	this->clear_flg = flg;
	Set_ResltText_Text();
}

void UEndWidget::Set_ResltText_Text()
{
	// nullチェック
	if (ResultText)
	{
		if (clear_flg)
		{
			ResultText->SetText(FText::FromString("Win"));
		}
		else
		{
			ResultText->SetText(FText::FromString("Lose"));
		}
	}
}