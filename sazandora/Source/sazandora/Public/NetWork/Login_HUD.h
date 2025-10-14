// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Login_HUD.generated.h"

/**
 * 
 */
UCLASS()
class SAZANDORA_API ALogin_HUD : public AHUD
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> LoginWidgetClass;

private:
	UPROPERTY()
	UUserWidget* LoginWidget;
};
