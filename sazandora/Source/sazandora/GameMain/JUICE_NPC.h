// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NPC_Character.h"
#include "JUICE_NPC.generated.h"

/**
 * 
 */
UCLASS()
class SAZANDORA_API AJUICE_NPC : public ANPC_Character
{
	GENERATED_BODY()

public:
	AJUICE_NPC();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// プレイヤーと会話できる範囲を検出するコリジョン
	class USphereComponent* TalkRange;


	// 会話開始関数（プレイヤーが近づいたときなどに呼ぶ）
	virtual void OnPlayerEnterRange(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComo, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

	// 離れたとき
	virtual void OnPlayerLeaveRange(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) override;

	// 会話イベント
	virtual void Talk_Event(AMain_Character* player) override;
};
