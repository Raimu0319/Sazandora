// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "GoalBoxComponent.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = Custom, meta=(BlueprintSpawnableComponent))
class SAZANDORA_API UGoalBoxComponent : public UBoxComponent
{
	GENERATED_BODY()
	
public:
	UGoalBoxComponent();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnOverlapBegin(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
		);
};
