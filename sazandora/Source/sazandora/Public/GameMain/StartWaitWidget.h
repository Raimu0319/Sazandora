// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StartWaitWidget.generated.h"

/**
 * 
 */
UCLASS()
class SAZANDORA_API UStartWaitWidget : public UUserWidget
{
	GENERATED_BODY()
	

public:
	UFUNCTION()
	void Widget_Initialize();
	UFUNCTION(BlueprintCallable)
	void OnStartButtonClicked();

	UPROPERTY(BlueprintReadOnly)
	bool is_host;

	virtual void NativeConstruct() override;

protected:
	UPROPERTY(meta = (BindWidget))
	class UButton* start_button;
	
};
