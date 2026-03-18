// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GoalBoxComponent.h"
#include "GoalActor.generated.h"

UCLASS(Blueprintable)
class SAZANDORA_API AGoalActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGoalActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	UGoalBoxComponent* GoalBox;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* GoalMesh;

	// オーバーレイマテリアルの保存
	UPROPERTY(EditAnywhere, Category = "Effects")
	UMaterialInterface* OverlayMaterial;

	UPROPERTY()
	bool is_outline = false;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// アウトラインの表示変更処理
	UFUNCTION()
	void ChangeOutlineVisibility(bool flg);

};
