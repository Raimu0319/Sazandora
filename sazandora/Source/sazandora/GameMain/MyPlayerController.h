// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "HUDWidget.h"
#include "MyPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class SAZANDORA_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AMyPlayerController();

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditDefaultsOnly, category ="UI")
	TSubclassOf<class  UHUDWidget> HUDWidget_class;

private:
	UPROPERTY()
	UHUDWidget* HUDWidget;
	
	
};
