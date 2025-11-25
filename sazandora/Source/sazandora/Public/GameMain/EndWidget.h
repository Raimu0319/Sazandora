// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Blueprint/UserWidget.h"
#include "EndWidget.generated.h"

/**
 * 
 */
UCLASS()
class SAZANDORA_API UEndWidget : public UUserWidget
{
	GENERATED_BODY()
	
private:
	UPROPERTY(meta = (BindWidget))
	UButton* TitleBack;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ResultText;

protected:
	virtual void NativeConstruct() override;

private:
	UFUNCTION()
	void OnTitleBackButtonClicked();
	
};
