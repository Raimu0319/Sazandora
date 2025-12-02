// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Blueprint/UserWidget.h"
#include "UnableConnectWidget.generated.h"

/**
 * 
 */
UCLASS()
class SAZANDORA_API UUnableConnectWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:

	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	UButton* OKButton;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* UnableConnectText;

public:
	UFUNCTION()
	void OnOKButtonClicked();

};
