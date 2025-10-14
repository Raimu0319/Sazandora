// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LogInWidget.generated.h"

/**
 * 
 */
UCLASS()
class SAZANDORA_API ULogInWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

private:
	UPROPERTY(meta = (BindWidget))
	UButton* ServerButton;

	UPROPERTY(meta = (BindWidget))
	UButton* ClientButton;

	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* TextBoxIPAddress;

	UFUNCTION()
	void OnServerButtonClicked();

	UFUNCTION()
	void OnClientButtonClicked();
};
