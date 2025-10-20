// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/SphereComponent.h"
#include "NPC_Character.generated.h"

UCLASS()
class SAZANDORA_API ANPC_Character : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ANPC_Character();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// プレイヤーと会話できる範囲を検出するコリジョン
	UPROPERTY(VisibleAnywhere, Category="NPC")
	class USphereComponent* TalkRange;

	// 会話開始関数（プレイヤーが近づいたときなどに呼ぶ）
	UFUNCTION()
	void OnPlayerEnterRange(UPrimitiveComponent* OverlappedComp, AActor* OtherActer,
		UPrimitiveComponent* OtherComo, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// 離れたとき
	UFUNCTION()
	void OnPlayerLeaveRange(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

};
